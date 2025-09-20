/**
  ******************************************************************************
  * @file    py32f4xx_hal_canfd.c
  * @author  MCU Application Team
  * @brief   CANFD HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the CAN with Flexible Data-rate (CANFD).
  *           + Initialization and de-initialization functions
  *           + IO operation functions
  *           + Peripheral Control functions
  *           + Peripheral State and Errors functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by Puya under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "py32f4xx_hal.h"

/** @addtogroup PY32F4xx_HAL_Driver
  * @{
  */

/** @defgroup CANFD CANFD
  * @brief CANFD driver modules
  * @{
  */

#ifdef  HAL_CANFD_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private defines ------------------------------------------------------------*/
/** @defgroup CANFD_Private_Constants CANFD Private Constants
  * @{
  */
#define CANFD_TIMEOUT_VALUE 10U
#define CANFD_LLC_EXTENDED_ID_MSK              (0x1FFFFFFFU)
#define CANFD_LLC_STANDARD_ID_MSK              (0x7FF<<18)
#define CANFD_LLC_FORMAT_MSK                   (0x1F1707FF)
/**
  * @}
  */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup CANFD_Private_Variables CANFD Private Variables
  * @{
  */
static const uint8_t DLCtoBytes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup CANFD_Private_Functions CANFD Private Functions
  * @{
  */
static void CANFD_CopyMessageToFifo(CANFD_HandleTypeDef *hcanfd, CANFD_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData);
/**
  * @}
  */

/** @defgroup CANFD_Exported_Functions CANFD Exported Functions
  * @{
  */

/** @defgroup CANFD_Exported_Functions_Group1 Initialization and de-initialization functions
 *  @brief    Initialization and Configuration functions
  * @{
  */

/**
  * @brief  CANFD init
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_Init(CANFD_HandleTypeDef *hcanfd)
{
  uint32_t tickstart;

  /* Check CANFD handle */
  if (hcanfd == NULL)
  {
    return HAL_ERROR;
  }

  /* Check function parameters */
  assert_param(IS_CANFD_ALL_INSTANCE(hcanfd->Instance));
  assert_param(IS_CANFD_FRAME_FORMAT(hcanfd->Init.FrameFormat));
  assert_param(IS_CANFD_MODE(hcanfd->Init.Mode));
  assert_param(IS_CANFD_CANFD_PROTOCOL(hcanfd->Init.CANFDProtocol));
  assert_param(IS_CANFD_CKDIV(hcanfd->Init.Prescaler));
  assert_param(IS_CANFD_NOMINAL_SJW(hcanfd->Init.NominalSyncJumpWidth));
  assert_param(IS_CANFD_NOMINAL_TSEG1(hcanfd->Init.NominalTimeSeg1));
  assert_param(IS_CANFD_NOMINAL_TSEG2(hcanfd->Init.NominalTimeSeg2));
  if (hcanfd->Init.FrameFormat == CANFD_FRAME_FD_BRS)
  {
    assert_param(IS_CANFD_DATA_SJW(hcanfd->Init.DataSyncJumpWidth));
    assert_param(IS_CANFD_DATA_TSEG1(hcanfd->Init.DataTimeSeg1));
    assert_param(IS_CANFD_DATA_TSEG2(hcanfd->Init.DataTimeSeg2));
  }
  assert_param(IS_CANFD_SSPOFF(hcanfd->Init.SecondSamplePointOffset));

  if (hcanfd->State == HAL_CANFD_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hcanfd->Lock = HAL_UNLOCKED;

    /* Init the low level hardware: CLOCK, NVIC */
    HAL_CANFD_MspInit(hcanfd);
  }

  /* forces to reset state */
  SET_BIT(hcanfd->Instance->MCR, CANFD_MCR_RESET);

  /* Get tick */
  tickstart = HAL_GetTick();

  /* Wait until the RESET bit into MCR register is set */
  while ((hcanfd->Instance->MCR & CANFD_MCR_RESET) == 0U)
  {
    /* Check for the Timeout */
    if ((HAL_GetTick() - tickstart) > CANFD_TIMEOUT_VALUE)
    {
      /* Update error code */
      hcanfd->ErrorCode |= HAL_CANFD_ERROR_TIMEOUT;

      /* Change CAN state */
      hcanfd->State = HAL_CANFD_STATE_ERROR;

      return HAL_ERROR;
    }
  }

  /* Specifies the number of time quanta in Bit */  
  hcanfd->Instance->ACBTR = (((hcanfd->Init.NominalSyncJumpWidth - 1U) << CANFD_ACBTR_AC_SJW_Pos) + \
                             ((hcanfd->Init.NominalTimeSeg1 - 2U) << CANFD_ACBTR_AC_SEG_1_Pos) + \
                             ((hcanfd->Init.NominalTimeSeg2 - 1U) << CANFD_ACBTR_AC_SEG_2_Pos));

  /* Specifies functions related to FD frames  */
  if (hcanfd->Init.FrameFormat == CANFD_FRAME_CLASSIC)
  {
    hcanfd->Instance->FDBTR = hcanfd->Instance->ACBTR;
  }
  else
  {
    if (hcanfd->Init.FrameFormat == CANFD_FRAME_FD_NO_BRS)
    {
      hcanfd->Instance->FDBTR = hcanfd->Instance->ACBTR;
    }
    else
    {
      hcanfd->Instance->FDBTR = (((hcanfd->Init.DataSyncJumpWidth - 1U) << CANFD_FDBTR_FD_SJW_Pos) + \
                                 ((hcanfd->Init.DataTimeSeg1 - 2U) << CANFD_FDBTR_FD_SEG_1_Pos) + \
                                 ((hcanfd->Init.DataTimeSeg2 - 1U) << CANFD_FDBTR_FD_SEG_2_Pos));
    }
    /* Specifies Bosch CAN-FD or 11898-1:2015 CAN-FD */
    MODIFY_REG(hcanfd->Instance->MCR, CANFD_MCR_FD_ISO, hcanfd->Init.CANFDProtocol);

    /* Specifies secondary sample point(SSP) and  the length of a time quantum(PRESC)*/
    MODIFY_REG(hcanfd->Instance->RLSSP, CANFD_RLSSP_FD_SSPOFF, (hcanfd->Init.SecondSamplePointOffset << CANFD_RLSSP_FD_SSPOFF_Pos));
  }
  
  MODIFY_REG(hcanfd->Instance->RLSSP, CANFD_RLSSP_PRESC, ((hcanfd->Init.Prescaler - 1U) << CANFD_RLSSP_PRESC_Pos));

  /* Initialize the error code */
  hcanfd->ErrorCode = HAL_CANFD_ERROR_NONE;

  /* Initialize the CANFD state */
  hcanfd->State = HAL_CANFD_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  CANFD MSP Init.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_MspInit(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_MspInit could be implemented in the user file
   */
}

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
/**
  * @brief  Register a CANFD CallBack.
  *         To be used instead of the weak predefined callback
  * @param  hcanfd pointer to a CANFD_HandleTypeDef structure that contains
  *         the configuration information for CANFD module
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_CANFD_RX_COMPLETE_CB_ID             CANFD Rx complete callback ID
  *           @arg @ref HAL_CANFD_RX_FIFO_ALMOST_FULL_CB_ID     CANFD Rx fifo almost full callback ID 
  *           @arg @ref HAL_CANFD_RX_FIFO_FULL_CB_ID            CANFD Rx fifo full callback ID 
  *           @arg @ref HAL_CANFD_PTB_TX_COMPLETE_CB_ID         CANFD PTB Tx complete callback ID 
  *           @arg @ref HAL_CANFD_STB_TX_COMPLETE_CB_ID         CANFD STB Tx complete callback ID
  *           @arg @ref HAL_CANFD_TX_ABORT_CB_ID                CANFD Tx abort callback ID         
  *           @arg @ref HAL_CANFD_RX_FIFO_OVERFLOW_CB_ID        CANFD Rx fifo overflow callback ID 
  *           @arg @ref HAL_CANFD_ARBITRATION_LOST_CB_ID        CANFD Arbitration lost callback ID 
  *           @arg @ref HAL_CANFD_PASSIVE_ERROR_CB_ID           CANFD Passive error callback ID
  *           @arg @ref HAL_CANFD_BUS_ERROR_CB_ID               CANFD Bus error callback ID 
  *           @arg @ref HAL_CANFD_ERROR_CHANGE_CB_ID            CANFD Error change callback ID 
  *           @arg @ref HAL_CANFD_TT_RX_TIME_TRIG_CB_ID         CANFD TT Rx time trig callback ID
  *           @arg @ref HAL_CANFD_TT_TX_SINGLE_TRIG_CB_ID       CANFD TT Tx single trig callback ID
  *           @arg @ref HAL_CANFD_TT_TX_START_TRIG_CB_ID        CANFD TT Tx start trig callback ID 
  *           @arg @ref HAL_CANFD_TT_TX_STOP_TRIG_CB_ID         CANFD TT Tx stop trig callback ID
  *           @arg @ref HAL_CANFD_TT_TIMESTAMP_WRAPAROUND_CB_ID CANFD TT timestamp wraparound callback ID
  *           @arg @ref HAL_CANFD_TT_TRIG_ERROR_CB_ID           CANFD TT trig error callback ID
  *           @arg @ref HAL_CANFD_MSPINIT_CB_ID                 MspInit callback ID
  *           @arg @ref HAL_CANFD_MSPDEINIT_CB_ID               MspDeInit callback ID
  * @param  pCallback pointer to the Callback function
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_RegisterCallback(CANFD_HandleTypeDef *hcanfd, HAL_CANFD_CallbackIDTypeDef CallbackID, void (* pCallback)(CANFD_HandleTypeDef *_hCANFD))
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* Update the error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_INVALID_CALLBACK;

    return HAL_ERROR;
  }

  if (hcanfd->State == HAL_CANFD_STATE_READY)
  {
    switch (CallbackID)
    {
      case HAL_CANFD_RX_COMPLETE_CB_ID :
        hfdcan->RxCpltCallback = pCallback;
        break;

      case HAL_CANFD_RX_FIFO_ALMOST_FULL_CB_ID :
        hfdcan->RxFifoAlmostFullCallback = pCallback;
        break;

      case HAL_CANFD_RX_FIFO_FULL_CB_ID :
        hfdcan->RxFifoFullCallback = pCallback;
        break;

      case HAL_CANFD_PTB_TX_COMPLETE_CB_ID :
        hfdcan->PtbTxCpltCallback = pCallback;
        break;

      case HAL_CANFD_STB_TX_COMPLETE_CB_ID :
        hfdcan->StbTxCpltCallback = pCallback;
        break;

      case HAL_CANFD_TX_ABORT_CB_ID :
        hfdcan->TxAbortCallback = pCallback;
        break;

      case HAL_CANFD_RX_FIFO_OVERFLOW_CB_ID :
        hfdcan->RxFifoOverflowCallback = pCallback;
        break;

      case HAL_CANFD_ARBITRATION_LOST_CB_ID :
        hfdcan->ArbitrationLostCallback = pCallback;
        break;

      case HAL_CANFD_PASSIVE_ERROR_CB_ID :
        hfdcan->PassiveErrorCallback = pCallback;
        break;

      case HAL_CANFD_BUS_ERROR_CB_ID :
        hfdcan->BusErrorCallback = pCallback;
        break;

      case HAL_CANFD_ERROR_CHANGE_CB_ID :
        hfdcan->ErrorChangeCallback = pCallback;
        break;

      case HAL_CANFD_TT_RX_TIME_TRIG_CB_ID :
        hfdcan->TT_RxTimeTrigCallback = pCallback;
        break;

      case HAL_CANFD_TT_TX_SINGLE_TRIG_CB_ID :
        hfdcan->TT_TxSingleTrigCallback = pCallback;
        break;

      case HAL_CANFD_TT_TX_START_TRIG_CB_ID :
        hfdcan->TT_TxStartTrigCallback = pCallback;
        break;

      case HAL_CANFD_TT_TX_STOP_TRIG_CB_ID :
        hfdcan->TT_TxStopTrigCallback = pCallback;
        break;

      case HAL_CANFD_TT_TIMESTAMP_WRAPAROUND_CB_ID :
        hfdcan->TT_TimestampWraparoundCallback = pCallback;
        break;

      case HAL_CANFD_TT_TRIG_ERROR_CB_ID :
        hfdcan->TT_TrigErrorCallback = pCallback;
        break;

      default :
        /* Update the error code */
        hfdcan->ErrorCode |= HAL_CANFD_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (hcanfd->State == HAL_CANFD_STATE_RESET)
  {
    switch (CallbackID)
    {
      case HAL_CANFD_MSPINIT_CB_ID :
        hcanfd->MspInitCallback = pCallback;
        break;

      case HAL_CANFD_MSPDEINIT_CB_ID :
        hcanfd->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the error code */
        hcanfd->ErrorCode |= HAL_CANFD_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hfdcan->ErrorCode |= HAL_CANFD_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
  }

  return status;
}
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup CANFD_Exported_Functions_Group2 IO operation functions
 *  @brief    Configuration functions.
  * @{
  */

/**
  * @brief  Config CANFD filter
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  sFilter pointer to an CANFD_FilterTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_ConfigFilter(CANFD_HandleTypeDef *hcanfd, CANFD_FilterTypeDef *sFilter)
{
  HAL_CANFD_StateTypeDef state = hcanfd->State;

  /* Check function parameters */
  assert_param(IS_CANFD_ID_TYPE(sFilter->IdType));
  assert_param(IS_CANFD_FILTER_CHANNEL(sFilter->FilterChannel));
  assert_param(IS_CANFD_RANK(sFilter->Rank));
  assert_param(IS_CANFD_FILTER_ID(sFilter->FilterID));
  assert_param(IS_CANFD_FILTER_FORMAT(sFilter->FilterFormat));
  assert_param(IS_CANFD_MASK_ID(sFilter->MaskID));
  assert_param(IS_CANFD_MASK_FORMAT(sFilter->MaskFormat));
  
  if (state == HAL_CANFD_STATE_READY)
  {
    if (sFilter->Rank == CANFD_FILTER_RANK_NONE)
    {
      /* Disable Filter channel */
      CLEAR_BIT(hcanfd->Instance->ACFCR, (1U << (CANFD_ACFCR_AE_0_Pos + sFilter->FilterChannel)));
    }
    else
    {
      /* Specify acceptance filter address */
      MODIFY_REG(hcanfd->Instance->ACFCR, CANFD_ACFCR_ACFADR, sFilter->FilterChannel);

      /* Specify acceptance code Identifier and mask Identifier */
      if (sFilter->IdType == CANFD_STANDARD_ID)
      {
        hcanfd->Instance->ACFC.ID = sFilter->FilterID << 18;
        hcanfd->Instance->ACFM.ID = (~CANFD_LLC_STANDARD_ID_MSK) | (sFilter->MaskID << 18);
      }
      else
      {
        hcanfd->Instance->ACFC.ID = sFilter->FilterID;
        hcanfd->Instance->ACFM.ID = (~CANFD_LLC_EXTENDED_ID_MSK) | (sFilter->MaskID);
      }

      /* Specify acceptance code Format */
      hcanfd->Instance->ACFC.FORMAT = sFilter->FilterFormat;

      /* Specify acceptance mask Format */
      hcanfd->Instance->ACFM.FORMAT = (~CANFD_LLC_FORMAT_MSK) | (sFilter->MaskFormat);

      /* Specify acceptance mask Type */
      hcanfd->Instance->ACFM.TYPE = 0xFFFFFFFF;

      /* Specify acceptance mask Acceptance Field */
       hcanfd->Instance->ACFM.Reserved = 0xFFFFFFFF;
      /* Enable Filter channel*/
      SET_BIT(hcanfd->Instance->ACFCR, (1U << (CANFD_ACFCR_AE_0_Pos + sFilter->FilterChannel)));
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Add a message to the Tx FIFO
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  pTxHeader pointer to an CANFD_TxHeaderTypeDef structure.
  * @param  pData pointer to a buffer containing the payload of the Tx frame.
  * @param  TxFifoType Tx FIFO type.
  *         This parameter can be a value of @arg CANFD_fifo_type.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_AddMessageToTxFifo(CANFD_HandleTypeDef *hcanfd, CANFD_TxHeaderTypeDef *pTxHeader, uint8_t *pData, uint32_t TxFifoType)
{
  /* Check function parameters */
  assert_param(IS_CANFD_ID_TYPE(pTxHeader->IdType));
  if (pTxHeader->IdType == CANFD_STANDARD_ID)
  {
    assert_param(IS_CANFD_MAX_VALUE(pTxHeader->Identifier, 0x7FFU));
  }
  else /* pTxHeader->IdType == CANFD_EXTENDED_ID */
  {
    assert_param(IS_CANFD_MAX_VALUE(pTxHeader->Identifier, 0x1FFFFFFFU));
  }
  assert_param(IS_CANFD_TX_FRAME_TYPE(pTxHeader->TxFrameType));
  assert_param(IS_CANFD_FRAME_FORMAT(pTxHeader->FrameFormat));
  assert_param(IS_CANFD_Handle(pTxHeader->Handle));
  assert_param(IS_CANFD_DLC(pTxHeader->DataLength));
  assert_param(IS_CANFD_TX_FIFO_TYPE(TxFifoType));
  
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    if (TxFifoType == CANFD_TX_FIFO_PTB)
    {
      /* select PTB FIFO */
      CLEAR_BIT(hcanfd->Instance->MCR, CANFD_MCR_TBSEL);

      /* check that the PTB FIFO is not full */
      if (READ_BIT(hcanfd->Instance->MCR, CANFD_MCR_TPE) == 0)
      {
        /* Write data to the FIFO */
        CANFD_CopyMessageToFifo(hcanfd, pTxHeader, pData);
      }
      else
      {
        /* Update error code */
        hcanfd->ErrorCode |= HAL_CANFD_ERROR_FIFO_FULL;

        /* The PTB is transmitting data, Cannot write data to PTB FIFO */
        return HAL_ERROR;
      }
    }
    else
    {
      /* select STB FIFO */
      SET_BIT(hcanfd->Instance->MCR, CANFD_MCR_TBSEL);

      /* check that the STB FIFO is not full */
      if (__HAL_CANFD_GET_STB_FIFO_FREE_LEVEL(hcanfd) != CANFD_STB_FIFO_FULL)
      {
        /* Write data to the FIFO */
        CANFD_CopyMessageToFifo(hcanfd, pTxHeader, pData);
        
        /* this slot has been filled, CAN-CTRL connects the TBUF registers to the next slot.*/
        SET_BIT(CANFD->MCR, CANFD_MCR_TSNEXT);
      }
      else
      {
        /* Update error code */
        hcanfd->ErrorCode |= HAL_CANFD_ERROR_FIFO_FULL;

        /* The STB FIFO is full and cannot be written */
        return HAL_ERROR;
      }
    }

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Activation the message is sent from Tx FIFO to the bus
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  activeTxMode Select the sending mode
  *         This parameter can be a value of @arg CANFD_transmit_mode.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_ActivateTxRequest(CANFD_HandleTypeDef *hcanfd, uint32_t activeTxMode)
{
  /* Check function parameters */
  assert_param(IS_CANFD_TRANSMIT_MODE(activeTxMode));
  
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    if (activeTxMode == CANFD_TXFIFO_PTB_SEND)
    {
      /* PTB FIFO data start sending */
      SET_BIT(hcanfd->Instance->MCR,CANFD_MCR_TPE);
    }

    if (activeTxMode == CANFD_TXFIFO_STB_SEND_ONE)
    {
      /* Transmit STB FIFO one frame */
      SET_BIT(hcanfd->Instance->MCR,CANFD_MCR_TSONE);

      hcanfd->LastSTBTxType = CANFD_TXFIFO_STB_SEND_ONE;
    }
    else if (activeTxMode == CANFD_TXFIFO_STB_SEND_ALL)
    {
      /* Transmit STB FIFO all frame */
      SET_BIT(hcanfd->Instance->MCR,CANFD_MCR_TSALL);

      hcanfd->LastSTBTxType = CANFD_TXFIFO_STB_SEND_ALL;
    }

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Get Tx FIFO message
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  pRxHeader pointer to an CANFD_RxHeaderTypeDef structure.
  * @param  pRxData pointer to a buffer containing the payload of the Rx frame.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_GetRxMessage(CANFD_HandleTypeDef *hcanfd, CANFD_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData)
{
  uint8_t  *pData;
  uint32_t ByteCounter;
  uint32_t Format;
  
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    if (__HAL_CANFD_GET_RX_FIFO_FILL_LEVEL(hcanfd) == CANFD_RX_FIFO_EMPTY)
    {
      /* Update error code */
      hcanfd->ErrorCode |= HAL_CANFD_ERROR_FIFO_FULL;

      return HAL_ERROR;
    }

    Format = hcanfd->Instance->RBUF.FORMAT;
    pRxHeader->DataLength = Format & CANFD_LLC_FORMAT_DLC;
    pRxHeader->IdType = Format & CANFD_LLC_FORMAT_IDE;
    pRxHeader->RxFrameType = Format & CANFD_LLC_FORMAT_RMF;
    pRxHeader->FrameFormat = Format & (CANFD_LLC_FORMAT_BRS | CANFD_LLC_FORMAT_FDF);
    pRxHeader->ProtocolErrorType = (Format & CANFD_LLC_FORMAT_KOER)>>CANFD_LLC_FORMAT_KOER_Pos;
    pRxHeader->ErrorStateIndicator = Format & CANFD_LLC_FORMAT_ESI;
    pRxHeader->LoopBackIndicator = Format & CANFD_LLC_FORMAT_LBF;

    if (pRxHeader->IdType == CANFD_EXTENDED_ID)
    {
      pRxHeader->Identifier = hcanfd->Instance->RBUF.ID & CANFD_LLC_EXTENDED_ID_MSK;
    }
    else
    {
      pRxHeader->Identifier = (hcanfd->Instance->RBUF.ID & CANFD_LLC_STANDARD_ID_MSK) >> 18;
    }
    
    /* Retrieve Rx payload */
    pData = (uint8_t *)hcanfd->Instance->RBUF.DATA;
    for (ByteCounter = 0; ByteCounter < DLCtoBytes[pRxHeader->DataLength]; ByteCounter++)
    {
      pRxData[ByteCounter] = pData[ByteCounter];
    }
    
    /* The host controller has read the actual RB slot and releases it. Afterwards CAN-CTRL points
          to the next RB slot.*/
    SET_BIT(CANFD->MCR, CANFD_MCR_RREL);
    
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Abort the message is sent from Tx FIFO to the bus
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  TransmitType Select a send mode to abort
  *         This parameter can be a value of @arg CANFD_transmit_mode.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_AbortTxRequest(CANFD_HandleTypeDef *hcanfd, uint32_t TransmitType)
{
  /* Check function parameters */
  assert_param(IS_CANFD_TRANSMIT_MODE(TransmitType));
  
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    if (TransmitType == CANFD_TXFIFO_PTB_SEND)
    {
      /* Abort PTB FIFO data transmission */
      SET_BIT(hcanfd->Instance->MCR,CANFD_MCR_TPA);
    }

    if ((TransmitType == CANFD_TXFIFO_STB_SEND_ONE) || (TransmitType == CANFD_TXFIFO_STB_SEND_ALL))
    {
      /* Abort STB FIFO data transmission */
      SET_BIT(hcanfd->Instance->MCR,CANFD_MCR_TSA);
    }

    hcanfd->LastAbortTxType = TransmitType;

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Config CANFD Re-arbitration limit.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  reaLimit indicates Re-arbitration limit value.
  *         This parameter can be a value of @arg CANFD_rearbitration_Limit.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_ConfigRearbitrationLimit(CANFD_HandleTypeDef *hcanfd, uint32_t reaLimit)
{
  /* Check function parameters */
  assert_param(IS_CANFD_REARBITRATION_LIMIT(reaLimit));

  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {

    MODIFY_REG(hcanfd->Instance->RLSSP, CANFD_RLSSP_REALIM, reaLimit);

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Config CANFD Re-transmission limit.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  retLimit indicates Re-transmission limit value.
  *         This parameter can be a value of @arg CANFD_retransmission_Limit.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_ConfigRetransmissionLimit(CANFD_HandleTypeDef *hcanfd, uint32_t retLimit)
{
  /* Check function parameters */
  assert_param(IS_CANFD_RETRANSMISSION_LIMIT(retLimit));

  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    MODIFY_REG(hcanfd->Instance->RLSSP, CANFD_RLSSP_RETLIM, retLimit);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Config the Priority mode of STB Buffer transmit.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  StbPriority the Priority mode of STB Buffer transmit.
  *         This parameter can be a value of @arg CANFD_STB_Priority_Mode.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_ConfigTxFifoPriority(CANFD_HandleTypeDef *hcanfd, uint32_t StbPriority)
{
  /* Check function parameters */
  assert_param(IS_CANFD_STB_PRIORITY(StbPriority));

  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    MODIFY_REG(hcanfd->Instance->MCR, CANFD_MCR_TSMODE, StbPriority);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure the STB FIFO operation mode.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  OverflowMode CAN Receive Buffer Overflow Mode.
  *         This parameter can be a value of @arg CANFD_RX_FIFO_overflow_mode.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_ConfigRxFifoOverwrite(CANFD_HandleTypeDef *hcanfd, uint32_t OverflowMode)
{
  /* Check function parameters */
  assert_param(IS_CANFD_RX_FIFO_MODE(OverflowMode));

  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    MODIFY_REG(hcanfd->Instance->MCR, CANFD_MCR_ROM, OverflowMode);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure receive buffer almost full warning limit.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  Threshold receive buffer almost full warning limit.
  *         This parameter can be a value of @arg CANFD_RX_FIFO_threshold.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_ConfigRxFifoThreshold(CANFD_HandleTypeDef *hcanfd, uint32_t Threshold)
{
  /* Check function parameters */
  assert_param(IS_CANFD_RX_FIFO_THRESHOLD(Threshold));

  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    MODIFY_REG(hcanfd->Instance->WECR, CANFD_WECR_AFWL, Threshold);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable CANFD receive error data frame.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_EnableReceiveErrorDataFrame(CANFD_HandleTypeDef *hcanfd)
{
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    /* Enable transmitter delay compensation */
    SET_BIT(hcanfd->Instance->MCR, CANFD_MCR_RBALL);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable CANFD receive error data frame.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_DisableReceiveErrorDataFrame(CANFD_HandleTypeDef *hcanfd)
{
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    /* Enable transmitter delay compensation */
    CLEAR_BIT(hcanfd->Instance->MCR, CANFD_MCR_RBALL);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable CANFD receive error data frame.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  pTTConfig pointer to an CANFD_TimeTriggerTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_TT_Config(CANFD_HandleTypeDef *hcanfd, CANFD_TimeTriggerTypeDef *pTTConfig)
{
  /* Check function parameters */
  assert_param(IS_CANFD_TT_FIFO_MODE(pTTConfig->FifoMode));
  assert_param(IS_CANFD_TT_PRESCALER(pTTConfig->Prescaler));
  if (pTTConfig->RefMessageIdType == CANFD_STANDARD_ID)
  {
    assert_param(IS_CANFD_MAX_VALUE(pTTConfig->RefMessageId, 0x7FFU));
  }
  else /* pTxHeader->IdType == CANFD_EXTENDED_ID */
  {
    assert_param(IS_CANFD_MAX_VALUE(pTTConfig->RefMessageId, 0x1FFFFFFFU));
  }
  assert_param(IS_CANFD_TX_ENABLE_WINDOW(pTTConfig->TxEnableWindow));
  assert_param(IS_CANFD_WATCH_TRIG_TIME(pTTConfig->WatchTrigTime));
  
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    MODIFY_REG(hcanfd->Instance->MCR, CANFD_MCR_TTTBM, pTTConfig->FifoMode);

    if (pTTConfig->RefMessageIdType == CANFD_STANDARD_ID)
    {
      WRITE_REG(hcanfd->Instance->REFMSG, (pTTConfig->RefMessageId));
    }
    else
    {
      WRITE_REG(hcanfd->Instance->REFMSG, (0x80000000 | pTTConfig->RefMessageId));
    }

    MODIFY_REG(hcanfd->Instance->TTTR, CANFD_TTTR_TT_WTRIG,(pTTConfig->WatchTrigTime<<CANFD_TTTR_TT_WTRIG_Pos));

    MODIFY_REG(hcanfd->Instance->TTCR, (CANFD_TTCR_T_PRESC | CANFD_TTCR_TEW), (pTTConfig->Prescaler | (pTTConfig->TxEnableWindow << CANFD_TTCR_TEW_Pos)));

    SET_BIT(hcanfd->Instance->TTCR, CANFD_TTCR_TTEN);

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Add a message to the time trig FIFO
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  txHeader pointer to an CANFD_TxHeaderTypeDef structure.
  * @param  pData pointer to a buffer containing the payload of the Tx frame.
  * @param  slotIndex the fifo index
  *         This parameter can be a value of @arg CANFD_TT_FIFO_INDEX.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_TT_AddMessageToFifo(CANFD_HandleTypeDef *hcanfd, CANFD_TxHeaderTypeDef *txHeader, uint8_t *pData, uint32_t slotIndex)
{
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    MODIFY_REG(hcanfd->Instance->TTCR, CANFD_TTCR_TBPTR, (slotIndex<<CANFD_TTCR_TBPTR_Pos));

    if (READ_BIT(hcanfd->Instance->MCR, CANFD_MCR_TSFF) == 0)
    {
      /* Write data to the FIFO */
      CANFD_CopyMessageToFifo(hcanfd, txHeader, pData);

      /* Notifies CANFD that the slot is filled */
      SET_BIT(CANFD->TTCR, CANFD_TTCR_TBF);
    }
    else
    {
      /* Update error code */
      hcanfd->ErrorCode |= HAL_CANFD_ERROR_FIFO_FULL;

      return HAL_ERROR;
    }

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Activate TTCAN Trigger Request.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  triggerType TTCAN trigger type.
  *         This parameter can be a value of @arg CANFD_TT_trigger_type.
  * @param  trigTime TTCAN trigger time.
  * @param  slotIndex the fifo index
  *         This parameter can be a value of @arg CANFD_TT_FIFO_INDEX.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_TT_ActivateTrigRequest(CANFD_HandleTypeDef *hcanfd, uint32_t triggerType, uint32_t trigTime, uint32_t slotIndex)
{
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    MODIFY_REG(CANFD->TTCR, (CANFD_TTCR_TTPTR | CANFD_TTCR_TTYPE), ((slotIndex<<CANFD_TTCR_TTPTR_Pos) | triggerType));

    MODIFY_REG(CANFD->TTTR, CANFD_TTTR_TT_TRIG, (trigTime<<CANFD_TTTR_TT_TRIG_Pos));

    hcanfd->LastTimeTrigType = triggerType;

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  TTCAN mode,set TB slot to empty.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  slotIndex TB SLOT index.
  *         This parameter can be a value of @arg CANFD_TT_FIFO_INDEX.
  * @param  slotState Set TB SLOT state
  *         This parameter can be a value of @arg CANFD_TT_FIFO_State_Set.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_TT_ConfigTxBufferState(CANFD_HandleTypeDef *hcanfd, uint32_t slotIndex, uint32_t slotState)
{
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    MODIFY_REG(hcanfd->Instance->TTCR, CANFD_TTCR_TBPTR, (slotIndex<<CANFD_TTCR_TBPTR_Pos));

    MODIFY_REG(CANFD->TTCR, (CANFD_TTCR_TBE | CANFD_TTCR_TBF), slotState);

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Start the CANFD module.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_Start(CANFD_HandleTypeDef *hcanfd)
{
  uint32_t tickstart;

  if (hcanfd->State == HAL_CANFD_STATE_READY)
  {
    /* Change CANFD peripheral state */
    hcanfd->State = HAL_CANFD_STATE_BUSY;

    /* Request leave initialisation */
    CLEAR_BIT(hcanfd->Instance->MCR, CANFD_MCR_RESET);

    /* Get tick */
    tickstart = HAL_GetTick();

    /* Wait until the RESET bit into MCR register is set */
    while ((hcanfd->Instance->MCR & CANFD_MCR_RESET) != 0)
    {
      /* Check for the Timeout */
      if ((HAL_GetTick() - tickstart) > CANFD_TIMEOUT_VALUE)
      {
        /* Update error code */
        hcanfd->ErrorCode |= HAL_CANFD_ERROR_TIMEOUT;

        /* Change CAN state */
        hcanfd->State = HAL_CANFD_STATE_ERROR;

        return HAL_ERROR;
      }
    }
    
    /* Specifies the CANFD working mode */
    if (hcanfd->Init.Mode == CANFD_MODE_RESTRICTED_OPERATION)
    {
      SET_BIT(hcanfd->Instance->TSNCR, CANFD_TSNCR_ROP);
    }
    else
    {
      MODIFY_REG(hcanfd->Instance->MCR, (CANFD_MCR_LBME | CANFD_MCR_LBMI | CANFD_MCR_SACK | CANFD_MCR_LOM), \
                 hcanfd->Init.Mode);
    }

    /* Reset the CANFD ErrorCode */
    hcanfd->ErrorCode = HAL_CANFD_ERROR_NONE;

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable interrupts.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  activeITs indicates which interrupts will be enabled.
  *         This parameter can be any combination of @arg CANFD_Interrupts.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_ActivateNotification(CANFD_HandleTypeDef *hcanfd, uint32_t activeITs)
{
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    __HAL_CANFD_ENABLE_IT(hcanfd, activeITs);

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable interrupts.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  inactiveITs indicates which interrupts will be disabled.
  *         This parameter can be any combination of @arg CANFD_Interrupts.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_DeactivateNotification(CANFD_HandleTypeDef *hcanfd, uint32_t inactiveITs)
{
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    __HAL_CANFD_DISABLE_IT(hcanfd, inactiveITs);

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  CANFD enter standby mode.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_EnterStandbyMode(CANFD_HandleTypeDef *hcanfd)
{
  uint32_t tickstart;

  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    SET_BIT(hcanfd->Instance->MCR, CANFD_MCR_STBY);

    /* Get tick */
    tickstart = HAL_GetTick();

    while (READ_BIT(hcanfd->Instance->MCR, CANFD_MCR_STBY) == 0)
    {
      /* Check for the Timeout */
      if ((HAL_GetTick() - tickstart) > CANFD_TIMEOUT_VALUE)
      {
        /* Update error code */
        hcanfd->ErrorCode |= HAL_CANFD_ERROR_TIMEOUT;
        
        return HAL_ERROR;
      }
      SET_BIT(hcanfd->Instance->MCR, CANFD_MCR_STBY);
    }

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  CANFD exit standby mode.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_ExitStandbyMode(CANFD_HandleTypeDef *hcanfd)
{
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    CLEAR_BIT(hcanfd->Instance->MCR, CANFD_MCR_STBY);

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Get protocol status.
  * @param  hcanfd pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  ProtocolStatus pointer to an FDCAN_ProtocolStatusTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_GetProtocolStatus(CANFD_HandleTypeDef *hcanfd, CANFD_ProtocolStatusTypeDef *ProtocolStatus)
{
  uint32_t StatusReg;

  /* Read the protocol status register */
  StatusReg = READ_REG(hcanfd->Instance->WECR);

  /* Fill the protocol status structure */
  ProtocolStatus->LastArbLostPos = ((StatusReg & CANFD_WECR_ALC) >> CANFD_WECR_ALC_Pos);
  ProtocolStatus->LastErrorCode = ((StatusReg & CANFD_WECR_KOER) >> CANFD_WECR_KOER_Pos);
  ProtocolStatus->RxErrorCnt = ((StatusReg & CANFD_WECR_RECNT) >> CANFD_WECR_RECNT_Pos);
  ProtocolStatus->TxErrorCnt = ((StatusReg & CANFD_WECR_TECNT) >> CANFD_WECR_TECNT_Pos);
  
  StatusReg = READ_REG(hcanfd->Instance->IFR);
  
  if ((StatusReg & CANFD_IFR_EPASS) != 0)
  {
    ProtocolStatus->ErrorPassive = 1;
  }
  else
  {
    ProtocolStatus->ErrorPassive = 0;
  }
  
  if ((StatusReg & CANFD_IFR_EWARN) != 0)
  {
    ProtocolStatus->Warning = 1;
  }
  else
  {
    ProtocolStatus->Warning = 0;
  }
  
  ProtocolStatus->BusOff = READ_BIT(hcanfd->Instance->MCR, CANFD_MCR_BUSOFF);
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Get transmit status.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  TxStatus pointer to an CANFD_TxStatusTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_GetTransmitStatus(CANFD_HandleTypeDef *hcanfd, CANFD_TxStatusTypeDef *TxStatus)
{
  uint32_t StatusReg;

  /* Read the protocol status register */
  StatusReg = READ_REG(hcanfd->Instance->TSR);

  /* Fill the protocol status structure */
  TxStatus->LastTxHandle = ((StatusReg & CANFD_TSR_HANDLE_H) >> CANFD_TSR_HANDLE_H_Pos);
  TxStatus->LastTxStatus = ((StatusReg & CANFD_TSR_TSTAT_H) >> CANFD_TSR_TSTAT_H_Pos);
  TxStatus->TxHandle = ((StatusReg & CANFD_TSR_HANDLE_L) >> CANFD_TSR_HANDLE_L_Pos);
  TxStatus->TxStatus = ((StatusReg & CANFD_TSR_TSTAT_L) >> CANFD_TSR_TSTAT_L_Pos);
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the CANFD module.
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CANFD_Stop(CANFD_HandleTypeDef *hcanfd)
{
  if (hcanfd->State == HAL_CANFD_STATE_BUSY)
  {
    /* forces to reset state */
    SET_BIT(hcanfd->Instance->MCR, CANFD_MCR_RESET);

    /* Reset the CANFD ErrorCode */
    hcanfd->ErrorCode = HAL_CANFD_ERROR_NONE;

    /* Change CANFD peripheral state */
    hcanfd->State = HAL_CANFD_STATE_READY;

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcanfd->ErrorCode |= HAL_CANFD_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  This function handles CANFD interrupt request.
  * @param  hcanfd  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
void HAL_CANFD_IRQHandler(CANFD_HandleTypeDef *hcanfd)
{
  /* Receive Interrupt */
  if ((__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_RX_COMPLETE) != 0U) && \
      (__HAL_CANFD_GET_IT_SOURCE(hcanfd, CANFD_IT_RX_COMPLETE) != 0U))
  {
    __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_RX_COMPLETE);

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcanfd->RxCpltCallback(hcanfd);
#else
    /* Recieve Complete Callback */
    HAL_CANFD_RxCpltCallback(hcanfd);
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
  }

  /* RB Almost Full Interrupt */
  if ((__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_RX_FIFO_ALMOST_FULL) != 0U) && \
      (__HAL_CANFD_GET_IT_SOURCE(hcanfd, CANFD_IT_RX_FIFO_ALMOST_FULL) != 0U))
  {
    __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_RX_FIFO_ALMOST_FULL);

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcanfd->RxFifoAlmostFullCallback(hcanfd);
#else
    /* Recieve fifo almost full Callback */
    HAL_CANFD_RxFifoAlmostFullCallback(hcanfd);
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
  }

  /* RB Full Interrupt */
  if ((__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_RX_FIFO_FULL) != 0U) && \
      (__HAL_CANFD_GET_IT_SOURCE(hcanfd, CANFD_IT_RX_FIFO_FULL) != 0U))
  {
    __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_RX_FIFO_FULL);

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcanfd->RxFifoFullCallback(hcanfd);
#else
    /* Recieve fifo full Callback */
    HAL_CANFD_RxFifoFullCallback(hcanfd);
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
  }

  /* Transmission Primary Interrupt */
  if ((__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_TX_PTB_COMPLETE) != 0U) && \
      (__HAL_CANFD_GET_IT_SOURCE(hcanfd, CANFD_IT_TX_PTB_COMPLETE) != 0U))
  {
    __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_TX_PTB_COMPLETE);

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcanfd->PtbTxCpltCallback(hcanfd);
#else
    /* PTB Transmit complete Callback */
    HAL_CANFD_PtbTxCpltCallback(hcanfd);
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
  }

  /* Transmission Secondary Interrupt */
  if ((__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_TX_STB_COMPLETE) != 0U) && \
      (__HAL_CANFD_GET_IT_SOURCE(hcanfd, CANFD_IT_TX_STB_COMPLETE) != 0U))
  {
    __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_TX_STB_COMPLETE);

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcanfd->StbTxCpltCallback(hcanfd);
#else
    /* STB Transmit complete Callback */
    HAL_CANFD_StbTxCpltCallback(hcanfd, hcanfd->LastSTBTxType);
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
  }

  /* Abort Interrupt */
  if (__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_TX_ABORT_COMPLETE) != 0U)
  {
    __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_TX_ABORT_COMPLETE);

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcanfd->TxAbortCallback(hcanfd);
#else
    /* Abort Interrupt Callback */
    HAL_CANFD_TxAbortCallback(hcanfd, hcanfd->LastAbortTxType);
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
  }

  /* Time Trigger Interrupt */
  if ((__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_TRIGGER_COMPLETE) != 0U) && \
      (__HAL_CANFD_GET_IT_SOURCE(hcanfd, CANFD_IT_TRIGGER_COMPLETE) != 0U))
  {
    __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_TRIGGER_COMPLETE);

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    if (hcanfd->LastTimeTrigType == CANFD_TT_RX_TIME_TRIG)
    {
      hcanfd->TT_RxTimeTrigCallback(hcanfd);
    }
    else if (hcanfd->LastTimeTrigType == CANFD_TT_TX_SINGLE_TRIG)
    {
      hcanfd->TT_TxSingleTrigCallback(hcanfd);
    }
    else if (hcanfd->LastTimeTrigType == CANFD_TT_TX_START_TRIG)
    {
      hcanfd->TT_TxStartTrigCallback(hcanfd);
    }
    else if (hcanfd->LastTimeTrigType == CANFD_TT_TX_STOP_TRIG)
    {
      hcanfd->TT_TxStopTrigCallback(hcanfd);
    }
#else
    /* TTCAN Callback */
    if (hcanfd->LastTimeTrigType == CANFD_TT_RX_TIME_TRIG)
    {
      HAL_CANFD_TT_RxTimeTrigCallback(hcanfd);
    }
    else if (hcanfd->LastTimeTrigType == CANFD_TT_TX_SINGLE_TRIG)
    {
      HAL_CANFD_TT_TxSingleTrigCallback(hcanfd);
    }
    else if (hcanfd->LastTimeTrigType == CANFD_TT_TX_START_TRIG)
    {
      HAL_CANFD_TT_TxStartTrigCallback(hcanfd);
    }
    else if (hcanfd->LastTimeTrigType == CANFD_TT_TX_STOP_TRIG)
    {
      HAL_CANFD_TT_TxStopTrigCallback(hcanfd);
    }
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
  }

  /* Watch Trigger Interrupt */
  if ((__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_TIMESTAMP_WRAPAROUND) != 0U) && \
      (__HAL_CANFD_GET_IT_SOURCE(hcanfd, CANFD_IT_TIMESTAMP_WRAPAROUND) != 0U))
  {
    /* Clear the Timestamp Wraparound flag */
    __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_TIMESTAMP_WRAPAROUND);
#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcanfd->TimestampWraparoundCallback(hcanfd);
#else
    /* Timestamp Wraparound Callback */
    HAL_CANFD_TT_TimestampWraparoundCallback(hcanfd);
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
  }

  /* Error interrupt */
  if (((__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_ALL_ERROR) != 0U) && \
      (__HAL_CANFD_GET_IT_SOURCE(hcanfd, CANFD_IT_ALL_ERROR) != 0U)) ||\
      (__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_TRIGGER_ERROR) != 0U))
  {
    /* interrupt of Overflow of CANFD Rx FIFO */
    if ((__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_RX_FIFO_OVERFLOW) != 0U) && \
        (__HAL_CANFD_GET_IT_SOURCE(hcanfd, CANFD_IT_RX_FIFO_OVERFLOW) != 0U))
    {
      __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_RX_FIFO_OVERFLOW);

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
      /* Call registered callback*/
      hcanfd->RxFifoOverflowCallback(hcanfd);
#else
      /* Receive fifo overflow Callback */
      HAL_CANFD_RxFifoOverflowCallback(hcanfd);
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
    }

    /* Arbitration lost interrupt*/
    if ((__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_ARB_LOST) != 0U) && \
        (__HAL_CANFD_GET_IT_SOURCE(hcanfd, CANFD_IT_ARB_LOST) != 0U))
    {
      __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_ARB_LOST);

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
      /* Call registered callback*/
      hcanfd->ArbitrationLostCallback(hcanfd);
#else
      /* Arbitration lost Callback */
      HAL_CANFD_ArbitrationLostCallback(hcanfd);
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
    }

    /* Error_Passive interrupt*/
    if ((__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_ERROR_PASSIVE) != 0U) && \
        (__HAL_CANFD_GET_IT_SOURCE(hcanfd, CANFD_IT_ERROR_PASSIVE) != 0U))
    {
      __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_ERROR_PASSIVE);

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
      /* Call registered callback*/
      hcanfd->PassiveErrorCallback(hcanfd);
#else
      /* Passive error Callback */
      HAL_CANFD_PassiveErrorCallback(hcanfd);
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
    }

    /* bus error interrupt */
    if ((__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_BUS_ERROR) != 0U) && \
        (__HAL_CANFD_GET_IT_SOURCE(hcanfd, CANFD_IT_BUS_ERROR) != 0U))
    {
      __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_BUS_ERROR);

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
      /* Call registered callback*/
      hcanfd->BusErrorCallback(hcanfd);
#else
      /* Bus error Callback */
      HAL_CANFD_BusErrorCallback(hcanfd);
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
    }

    /* interrupt of Bus_Off status changed or Error_Passive status changed */
    if ((__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_ERROR_TOGGLE) != 0U) && \
        (__HAL_CANFD_GET_IT_SOURCE(hcanfd, CANFD_IT_ERROR_TOGGLE) != 0U))
    {
      __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_ERROR_TOGGLE);

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
      /* Call registered callback*/
      hcanfd->ErrorChangeCallback(hcanfd);
#else
      /* Error change Callback */
      HAL_CANFD_ErrorChangeCallback(hcanfd);
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
    }

    /* Trigger error Interrupt */
    if (__HAL_CANFD_GET_FLAG(hcanfd, CANFD_FLAG_TRIGGER_ERROR) != 0U)
    {
      __HAL_CANFD_CLEAR_FLAG(hcanfd, CANFD_FLAG_TRIGGER_ERROR);

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
      /* Call registered callback*/
      hcanfd->TT_TrigErrorCallback(hcanfd);
#else
      /* TTCAN trig error Callback */
      HAL_CANFD_TT_TrigErrorCallback(hcanfd);
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
    }
  }
}

/**
  * @brief  Recieve Complete Callback.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_RxCpltCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_RxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Recieve fifo almost full Callback.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_RxFifoAlmostFullCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_RxFifoAlmostFullCallback could be implemented in the user file
   */
}

/**
  * @brief  Recieve fifo full Callback.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_RxFifoFullCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_RxFifoFullCallback could be implemented in the user file
   */
}

/**
  * @brief  PTB Transmit complete Callback.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_PtbTxCpltCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_PtbTxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  STB Transmit complete Callback.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @param  LastSTBTxType  CAN last STB transmit type.
  * @retval None
  */
__weak void HAL_CANFD_StbTxCpltCallback(CANFD_HandleTypeDef *hcan, uint32_t LastSTBTxType)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_StbTxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Receive fifo overflow Callback.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_RxFifoOverflowCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_RxFifoOverflowCallback could be implemented in the user file
   */
}

/**
  * @brief  Arbitration lost Callback.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_ArbitrationLostCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_ArbitrationLostCallback could be implemented in the user file
   */
}

/**
  * @brief  Abort Interrupt Callback.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @param  LastAbortTxType  CAN last abort transmit type.
  * @retval None
  */
__weak void HAL_CANFD_TxAbortCallback(CANFD_HandleTypeDef *hcan, uint32_t LastAbortTxType)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_TxAbortCallback could be implemented in the user file
   */
}

/**
  * @brief  Passive error Callback.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_PassiveErrorCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_PassiveErrorCallback could be implemented in the user file
   */
}

/**
  * @brief  Bus error Callback.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_BusErrorCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_BusErrorCallback could be implemented in the user file
   */
}

/**
  * @brief  Error change Callback.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_ErrorChangeCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_ErrorChangeCallback could be implemented in the user file
   */
}

/**
  * @brief  TTCAN rx timer trigger callbacks.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_TT_RxTimeTrigCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_TT_RxTimeTrigCallback could be implemented in the user file
   */
}

/**
  * @brief  TTCAN tx singger trigger callbacks.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_TT_TxSingleTrigCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_TT_TxSingleTrigCallback could be implemented in the user file
   */
}

/**
  * @brief  TTCAN tx start trigger callbacks.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_TT_TxStartTrigCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_TT_TxStartTrigCallback could be implemented in the user file
   */
}

/**
  * @brief  TTCAN tx stop trigger callbacks.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_TT_TxStopTrigCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_TT_TxStopTrigCallback could be implemented in the user file
   */
}

/**
  * @brief  Timestamp Wraparound Callback.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_TT_TimestampWraparoundCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_TT_TimestampWraparoundCallback could be implemented in the user file
   */
}

/**
  * @brief  TTCAN trig error Callback.
  * @param  hcan  Pointer to a CANFD_HandleTypeDef structure that contains
  *                the configuration information for the specified CANFD module.
  * @retval None
  */
__weak void HAL_CANFD_TT_TrigErrorCallback(CANFD_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CANFD_TT_TrigErrorCallback could be implemented in the user file
   */
}

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup CANFD_Private_Functions
  * @{
  */

/**
  * @brief  Copy message to transmit FIFO
  * @param  hcanfd pointer to an CANFD_HandleTypeDef structure that contains
  *         the configuration information for the specified CANFD.
  * @param  pTxHeader pointer to an CANFD_TxHeaderTypeDef structure.
  * @param  pTxData pointer to a buffer containing the payload of the Tx frame.
  * @retval HAL status
  */
static void CANFD_CopyMessageToFifo(CANFD_HandleTypeDef *hcanfd, CANFD_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData)
{
  uint32_t index;
  uint32_t ByteCounter;
  
  /* Specify TBUF Identifier */
  if (pTxHeader->IdType == CANFD_STANDARD_ID)
  {
    hcanfd->Instance->TBUF.ID = pTxHeader->Identifier << 18;
  }
  else
  {
    hcanfd->Instance->TBUF.ID = pTxHeader->Identifier;
  }

  /* Specify TBUF FORMAT */
  hcanfd->Instance->TBUF.FORMAT = pTxHeader->DataLength | pTxHeader->IdType | pTxHeader->FrameFormat | pTxHeader->TxFrameType;

  /* Specify TBUF HANDLE */
  hcanfd->Instance->TBUF.TYPE = pTxHeader->Handle << 24;
  
  /* Write Tx payload to the message RAM */
  index = 0;
  for (ByteCounter = 0; ByteCounter < DLCtoBytes[pTxHeader->DataLength]; ByteCounter += 4U)
  {
    hcanfd->Instance->TBUF.DATA[index++] = (((uint32_t)pTxData[ByteCounter + 3U] << 24U) |
                                           ((uint32_t)pTxData[ByteCounter + 2U] << 16U)  |
                                           ((uint32_t)pTxData[ByteCounter + 1U] << 8U)   |
                                            (uint32_t)pTxData[ByteCounter]);
  }
}
/**
  * @}
  */

#endif /* HAL_CANFD_MODULE_ENABLED */

/**
  * @}
  */

/**
  * @}
  */
/************************ (C) COPYRIGHT Puya *****END OF FILE****/
