/**
  ******************************************************************************
  * @file    py32f4xx_hal_esmc.c
  * @author  MCU Application Team
  * @brief   ESMC HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the QuadSPI interface (ESMC).
  *           + Initialization and de-initialization functions
  *           + Indirect functional mode management
  *           + Memory-mapped functional mode management
  *           + Auto-polling functional mode management
  *           + Interrupts and flags management
  *           + DMA channel configuration for indirect functional mode
  *           + Errors management and abort functionality
  *
  *
  @verbatim
 ===============================================================================
  @endverbatim
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

/** @defgroup ESMC ESMC
  * @brief HAL ESMC module driver
  * @{
  */
#ifdef HAL_ESMC_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @addtogroup ESMC_Private_Constants
  * @{
  */
#define ESMC_FUNCTIONAL_MODE_INDIRECT_READ_WRITE       0x00000000U   /*!<Indirect write or read mode*/
#define ESMC_FUNCTIONAL_MODE_INDIRECT_INSTRUCT         0x00000001U   /*!<Indirect only send instruction mode*/
#define ESMC_FUNCTIONAL_MODE_MEMORY_MAPPED             0x00000002U   /*!<Memory-mapped mode*/
/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/** @addtogroup ESMC_Private_Macros ESMC Private Macros
  * @{
  */
#define IS_ESMC_FUNCTIONAL_MODE(MODE) (((MODE) == ESMC_FUNCTIONAL_MODE_INDIRECT_READ_WRITE)    || \
                                       ((MODE) == ESMC_FUNCTIONAL_MODE_INDIRECT_INSTRUCT) || \
                                       ((MODE) == ESMC_FUNCTIONAL_MODE_MEMORY_MAPPED))
/**
  * @}
  */

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/** @addtogroup ESMC_Private_Functions ESMC Private Functions
  * @{
  */
static void ESMC_DMARxCplt(DMA_HandleTypeDef *hdma);
static void ESMC_DMATxCplt(DMA_HandleTypeDef *hdma);
static void ESMC_DMARxHalfCplt(DMA_HandleTypeDef *hdma);
static void ESMC_DMATxHalfCplt(DMA_HandleTypeDef *hdma);
static void ESMC_DMAError(DMA_HandleTypeDef *hdma);
static void ESMC_DMAAbortCplt(DMA_HandleTypeDef *hdma);
static HAL_StatusTypeDef ESMC_WaitFlagStateUntilTimeout(ESMC_HandleTypeDef *hesmc, uint32_t Flag, FlagStatus State, uint32_t tickstart, uint32_t Timeout);
static void ESMC_Config(ESMC_HandleTypeDef *hesmc, ESMC_CommandTypeDef *cmd, uint32_t FunctionalMode);
static void ESMC_WriteBuff(ESMC_HandleTypeDef *hesmc, uint8_t *pData, uint8_t dataLen);
static void ESMC_ReadBuff(ESMC_HandleTypeDef *hesmc, uint8_t *pData, uint8_t dataLen);
/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/

/** @defgroup ESMC_Exported_Functions ESMC Exported Functions
  * @{
  */

/** @defgroup ESMC_Exported_Functions_Group1 Initialization/de-initialization functions
  *  @brief    Initialization and Configuration functions
  *
@verbatim
===============================================================================
            ##### Initialization and Configuration functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to :
      (+) Initialize the QuadSPI.
      (+) De-initialize the QuadSPI.

@endverbatim
  * @{
  */

/**
  * @brief Initializes the ESMC mode according to the specified parameters
  *        in the ESMC_InitTypeDef and creates the associated handle.
  * @param hesmc esmc handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ESMC_Init(ESMC_HandleTypeDef *hesmc)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t tickstart = HAL_GetTick();

  /* Check the ESMC handle allocation */
  if(hesmc == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_ESMC_ALL_INSTANCE(hesmc->Instance));
  assert_param(IS_ESMC_CLOCK_PRESCALER(hesmc->Init.ClockPrescaler));
  assert_param(IS_ESMC_CLOCK_MODE(hesmc->Init.ClockMode));
  assert_param(IS_ESMC_DUAL_FLASH_MODE(hesmc->Init.DualFlash));

  /* Process locked */
  __HAL_LOCK(hesmc);

  if(hesmc->State == HAL_ESMC_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hesmc->Lock = HAL_UNLOCKED;

#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
    /* Reset Callback pointers in HAL_ESMC_STATE_RESET only */
    hesmc->ErrorCallback         = HAL_ESMC_ErrorCallback;
    hesmc->AbortCpltCallback     = HAL_ESMC_AbortCpltCallback;
    hesmc->CmdCpltCallback       = HAL_ESMC_CmdCpltCallback;
    hesmc->RxCpltCallback        = HAL_ESMC_RxCpltCallback;
    hesmc->TxCpltCallback        = HAL_ESMC_TxCpltCallback;
    hesmc->RxHalfCpltCallback    = HAL_ESMC_RxHalfCpltCallback;
    hesmc->TxHalfCpltCallback    = HAL_ESMC_TxHalfCpltCallback;

    if(hesmc->MspInitCallback == NULL)
    {
      hesmc->MspInitCallback = HAL_ESMC_MspInit;
    }

    /* Init the low level hardware */
    hesmc->MspInitCallback(hesmc);
#else
    /* Init the low level hardware : GPIO, CLOCK */
    HAL_ESMC_MspInit(hesmc);
#endif

    /* Configure the default timeout for the ESMC memory access */
    HAL_ESMC_SetTimeout(hesmc, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);
  }

  /* Wait till IDLE flag set */
  status = ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_IDLE, SET, tickstart, hesmc->Timeout);

  if(status == HAL_OK)
  {
    /* Set Clock Prescaler */
    MODIFY_REG(hesmc->Instance->BAUD, ESMC_BAUD_BAUD, hesmc->Init.ClockPrescaler);

    /* Set Clock Mode */
    MODIFY_REG(hesmc->Instance->CR2, ESMC_CR2_CPOL| ESMC_CR2_CPHA, hesmc->Init.ClockMode);

    /* Config Dual Flash, Enable ESMC, And reset other bits */
    hesmc->Instance->CR = ESMC_CR_SPIEN | hesmc->Init.DualFlash;

    /* Clear FIFO */
    __HAL_ESMC_CLEAR_FIFO(hesmc);

    /* Initialize the ESMC operation */
    hesmc->Contex = ESMC_CONTEXT_NONE;

    /* Set ESMC error code to none */
    hesmc->ErrorCode = HAL_ESMC_ERROR_NONE;

    /* Initialize the ESMC state */
    hesmc->State = HAL_ESMC_STATE_READY;
  }

  /* Release Lock */
  __HAL_UNLOCK(hesmc);

  /* Return function status */
  return status;
}

/**
  * @brief DeInitializes the ESMC peripheral
  * @param hesmc esmc handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ESMC_DeInit(ESMC_HandleTypeDef *hesmc)
{
  /* Check the ESMC handle allocation */
  if(hesmc == NULL)
  {
    return HAL_ERROR;
  }

  /* Process locked */
  __HAL_LOCK(hesmc);

  /* Disable the slave select */
  __HAL_ESMC_DISABLE_SLAVE(hesmc);

  /* Soft reset ESMC */
  __HAL_ESMC_SOFT_RESET(hesmc);

#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
  if(hesmc->MspDeInitCallback == NULL)
  {
    hesmc->MspDeInitCallback = HAL_ESMC_MspDeInit;
  }

  /* DeInit the low level hardware */
  hesmc->MspDeInitCallback(hesmc);
#else
  /* DeInit the low level hardware: GPIO, CLOCK, NVIC... */
  HAL_ESMC_MspDeInit(hesmc);
#endif

  /* Set ESMC error code to none */
  hesmc->ErrorCode = HAL_ESMC_ERROR_NONE;

  /* Initialize the ESMC state */
  hesmc->State = HAL_ESMC_STATE_RESET;

  /* Release Lock */
  __HAL_UNLOCK(hesmc);

  return HAL_OK;
}

/**
  * @brief ESMC MSP Init
  * @param hesmc ESMC handle
  * @retval None
  */
__weak void HAL_ESMC_MspInit(ESMC_HandleTypeDef *hesmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hesmc);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_ESMC_MspInit can be implemented in the user file
   */
}

/**
  * @brief ESMC MSP DeInit
  * @param hesmc ESMC handle
  * @retval None
  */
__weak void HAL_ESMC_MspDeInit(ESMC_HandleTypeDef *hesmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hesmc);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_ESMC_MspDeInit can be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup ESMC_Exported_Functions_Group2 IO operation functions
  *  @brief ESMC Transmit/Receive functions
  *
@verbatim
 ===============================================================================
                      ##### IO operation functions #####
 ===============================================================================
       [..]
    This subsection provides a set of functions allowing to :
      (+) Handle the interrupts.
      (+) Handle the command sequence.
      (+) Transmit data in blocking, interrupt or DMA mode.
      (+) Receive data in blocking, interrupt or DMA mode.
      (+) Manage the auto-polling functional mode.
      (+) Manage the memory-mapped functional mode.

@endverbatim
  * @{
  */

/**
  * @brief This function handles ESMC interrupt request.
  * @param hesmc ESMC handle
  * @retval None.
  */
void HAL_ESMC_IRQHandler(ESMC_HandleTypeDef *hesmc)
{
  uint32_t flag = *(__IO uint16_t *)(&hesmc->Instance->SR);
  uint32_t itsource = READ_REG(hesmc->Instance->IER);
  uint32_t len;

  /* ESMC data wait interrupt occurred ----------------------------------*/
  if(((flag & ESMC_FLAG_DATAWAITIF) != 0U) && ((itsource & ESMC_IT_DATAWAIT) != 0U))
  {
    if(hesmc->State == HAL_ESMC_STATE_BUSY_INDIRECT_TX)
    {
      if (hesmc->TxXferCount == 0U)
      {
        if ((hesmc->Instance->CR & ESMC_CR_DMAEN) != 0U)
        {
          /* Disable the DMA transfer by clearing the DMAEN bit in the ESMC CR register */
          CLEAR_BIT(hesmc->Instance->CR, ESMC_CR_DMAEN);

          /* Disable the DMA channel */
          __HAL_DMA_DISABLE(hesmc->hdmatx);
        }
        /* No more data available for the transfer */
        /* Disable the ESMC datawait Interrupt */
        __HAL_ESMC_DISABLE_IT(hesmc, ESMC_IT_DATAWAIT);
        if (hesmc->Contex == ESMC_CONTEXT_WRITE_VARIABLE_LENGTH)
        {
          /* Disable the slave select */
          __HAL_ESMC_DISABLE_SLAVE(hesmc);

          /* Change state of ESMC */
          hesmc->State = HAL_ESMC_STATE_READY;

          /* Restore default values */
          hesmc->Contex = ESMC_CONTEXT_NONE;

          /* Command Complete callback */
#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
          hesmc->TxCpltCallback(hesmc);
#else
          HAL_ESMC_TxCpltCallback(hesmc);
#endif
        }
      } else
      {
        /* Transmission process */
        if ((hesmc->TxXferCount/HAL_ESMC_FIFO_SIZE) > 0)
        {
          /* The amount of data to be sent is greater than the maximum depth of the FIFO */
          ESMC_WriteBuff(hesmc, hesmc->pTxBuffPtr, HAL_ESMC_FIFO_SIZE);
          hesmc->pTxBuffPtr += HAL_ESMC_FIFO_SIZE;
          hesmc->TxXferCount -= HAL_ESMC_FIFO_SIZE;
        } else
        {
          ESMC_WriteBuff(hesmc, hesmc->pTxBuffPtr, hesmc->TxXferCount);
          hesmc->pTxBuffPtr += hesmc->TxXferCount;
          hesmc->TxXferCount = 0;
        }
      }
    } else if(hesmc->State == HAL_ESMC_STATE_BUSY_INDIRECT_RX)
    {
      /* Transmission process */
      len = (hesmc->RxXferCount > HAL_ESMC_FIFO_SIZE) ? HAL_ESMC_FIFO_SIZE : hesmc->RxXferCount;
      ESMC_ReadBuff(hesmc, hesmc->pRxBuffPtr, len);
      hesmc->pRxBuffPtr += len;
      hesmc->RxXferCount -= len;

      if (hesmc->RxXferCount == 0U)
      {
        /* No more data available for the transfer */
        /* Disable the ESMC datawait Interrupt */
        __HAL_ESMC_DISABLE_IT(hesmc, ESMC_IT_DATAWAIT);
        if (hesmc->Contex == ESMC_CONTEXT_READ_VARIABLE_LENGTH)
        {
          /* Disable the slave select */
          __HAL_ESMC_DISABLE_SLAVE(hesmc);

          /* Change state of ESMC */
          hesmc->State = HAL_ESMC_STATE_READY;

          /* Restore default values */
          hesmc->Contex = ESMC_CONTEXT_NONE;

          /* Command Complete callback */
#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
          hesmc->RxCpltCallback(hesmc);
#else
          HAL_ESMC_RxCpltCallback(hesmc);
#endif
        }
      }
    }
    else if(hesmc->State == HAL_ESMC_STATE_BUSY)
    {
      /* Disable the ESMC datawait Interrupt */
      __HAL_ESMC_DISABLE_IT(hesmc, ESMC_IT_DATAWAIT);

      /* Disable the slave select */
      __HAL_ESMC_DISABLE_SLAVE(hesmc);

      /* Change state of ESMC */
      hesmc->State = HAL_ESMC_STATE_READY;

      /* Restore default values */
      hesmc->Contex = ESMC_CONTEXT_NONE;

      /* Command Complete callback */
#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
      hesmc->CmdCpltCallback(hesmc);
#else
      HAL_ESMC_CmdCpltCallback(hesmc);
#endif
    }
  }
  else if(((flag & ESMC_FLAG_IDLEIF) != 0U) && ((itsource & ESMC_IT_IDLE) != 0U))
  {
    /* Clear interrupt */
    __HAL_ESMC_CLEAR_FLAG(hesmc, ESMC_FLAG_IDLEIF);

    /* Disable the ESMC idle interrupts and datawait interrupts */
    __HAL_ESMC_DISABLE_IT(hesmc, (ESMC_IT_IDLE | ESMC_IT_DATAWAIT));

    /* Transfer complete callback */
    if(hesmc->State == HAL_ESMC_STATE_BUSY_INDIRECT_TX)
    {
      if ((hesmc->Instance->CR & ESMC_CR_DMAEN) != 0U)
      {
        /* Disable the DMA transfer by clearing the DMAEN bit in the ESMC CR register */
        CLEAR_BIT(hesmc->Instance->CR, ESMC_CR_DMAEN);

        /* Disable the DMA channel */
        __HAL_DMA_DISABLE(hesmc->hdmatx);
      }

      /* Change state of ESMC */
      hesmc->State = HAL_ESMC_STATE_READY;

      /* Restore default values */
      hesmc->Contex = ESMC_CONTEXT_NONE;

      /* TX Complete callback */
#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
      hesmc->TxCpltCallback(hesmc);
#else
      HAL_ESMC_TxCpltCallback(hesmc);
#endif
    }
    else if(hesmc->State == HAL_ESMC_STATE_BUSY_INDIRECT_RX)
    {
      /* Transmission process */
      ESMC_ReadBuff(hesmc, hesmc->pRxBuffPtr, hesmc->RxXferCount);
      hesmc->pRxBuffPtr += hesmc->RxXferCount;
      hesmc->RxXferCount = 0;

      /* Change state of ESMC */
      hesmc->State = HAL_ESMC_STATE_READY;

      /* Restore default values */
      hesmc->Contex = ESMC_CONTEXT_NONE;

      /* RX Complete callback */
#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
      hesmc->RxCpltCallback(hesmc);
#else
      HAL_ESMC_RxCpltCallback(hesmc);
#endif
    }
    else if(hesmc->State == HAL_ESMC_STATE_BUSY)
    {
      /* Change state of ESMC */
      hesmc->State = HAL_ESMC_STATE_READY;

      /* Restore default values */
      hesmc->Contex = ESMC_CONTEXT_NONE;

      /* Command Complete callback */
#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
      hesmc->CmdCpltCallback(hesmc);
#else
      HAL_ESMC_CmdCpltCallback(hesmc);
#endif
    }
    else
    {
      /* Nothing to do */
    }
  }
}

/**
  * @brief Sets the command configuration.
  * @param hesmc ESMC handle
  * @param cmd  structure that contains the command configuration information
  * @param Timeout  Time out duration
  * @note   This function is used only in Indirect Read or Write Modes
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ESMC_Command(ESMC_HandleTypeDef *hesmc, ESMC_CommandTypeDef *cmd, uint32_t Timeout)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t tickstart = HAL_GetTick();

  assert_param(IS_ESMC_INSTRUCTION_MODE(cmd->InstructionMode));
  if (cmd->InstructionMode != ESMC_INSTRUCTION_NONE)
  {
    assert_param(IS_ESMC_INSTRUCTION(cmd->Instruction));
  }
  assert_param(IS_ESMC_ADDRESS_MODE(cmd->AddressMode));
  if (cmd->AddressMode != ESMC_ADDRESS_NONE)
  {
    assert_param(IS_ESMC_ADDRESS_SIZE(cmd->AddressSize));
  }

  assert_param(IS_ESMC_DUMMY_CYCLES(cmd->DummyCycles));
  assert_param(IS_ESMC_DATA_MODE(cmd->DataMode));

  assert_param(IS_ESMC_DDR_MODE(cmd->DdrMode));

  /* Process locked */
  __HAL_LOCK(hesmc);

  if(hesmc->State == HAL_ESMC_STATE_READY)
  {
    hesmc->ErrorCode = HAL_ESMC_ERROR_NONE;

    /* Update ESMC state */
    hesmc->State = HAL_ESMC_STATE_BUSY;

    /* Wait till idle state set */
    status = ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_IDLE, SET, tickstart, Timeout);

    if (status == HAL_OK)
    {
      if ((cmd->InstructionMode != ESMC_INSTRUCTION_NONE) && \
          (cmd->AddressMode == ESMC_ADDRESS_NONE) && \
          (cmd->AlternateByteMode == ESMC_ALTERNATE_BYTES_DISABLE)&& \
          (cmd->DataMode == ESMC_DATA_NONE))
      {
        /* Call the configuration function  */
        ESMC_Config(hesmc, cmd, ESMC_FUNCTIONAL_MODE_INDIRECT_INSTRUCT);

        /* wait until idle flag is set */
        status = ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_IDLEIF, SET, tickstart, Timeout);

        if (status == HAL_OK)
        {
          /* Update ESMC state */
          hesmc->State = HAL_ESMC_STATE_READY;
        }
      }
      else
      {
        /* Call the configuration function  */
        ESMC_Config(hesmc, cmd, ESMC_FUNCTIONAL_MODE_INDIRECT_READ_WRITE);

        if (cmd->DataMode == ESMC_DATA_NONE)
        {
          /* wait until data wait flag is set */
          status = ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_DATAWAITIF, SET, tickstart, Timeout);

          if (status == HAL_OK)
          {
            /* The slave select outputs to be driven inactive high */
            __HAL_ESMC_DISABLE_SLAVE(hesmc);
            /* Update ESMC state */
            hesmc->State = HAL_ESMC_STATE_READY;
          }
        } else
        {
          /* Update ESMC state */
          hesmc->State = HAL_ESMC_STATE_READY;
        }
      }
    }
  }
  else
  {
    status = HAL_BUSY;
  }

  /* Process unlocked */
  __HAL_UNLOCK(hesmc);

  /* Return function status */
  return status;
}

/**
  * @brief Set the command configuration in interrupt mode.
  * @param hesmc : ESMC handle
  * @param cmd : structure that contains the command configuration information
  * @note   This function is used only in Indirect Read or Write Modes
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ESMC_Command_IT(ESMC_HandleTypeDef *hesmc, ESMC_CommandTypeDef *cmd)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();

  /* Check the parameters */
  assert_param(IS_ESMC_INSTRUCTION_MODE(cmd->InstructionMode));
  if (cmd->InstructionMode != ESMC_INSTRUCTION_NONE)
  {
    assert_param(IS_ESMC_INSTRUCTION(cmd->Instruction));
  }

  assert_param(IS_ESMC_ADDRESS_MODE(cmd->AddressMode));
  if (cmd->AddressMode != ESMC_ADDRESS_NONE)
  {
    assert_param(IS_ESMC_ADDRESS_SIZE(cmd->AddressSize));
  }

  assert_param(IS_ESMC_ALTERNATE_BYTES_MODE(cmd->AlternateByteMode));
  if (cmd->AlternateByteMode != ESMC_ALTERNATE_BYTES_DISABLE)
  {
    assert_param(IS_ESMC_ALTERNATE_BYTE(cmd->AlternateByte));
  }

  assert_param(IS_ESMC_DUMMY_CYCLES(cmd->DummyCycles));
  assert_param(IS_ESMC_DATA_MODE(cmd->DataMode));

  assert_param(IS_ESMC_DDR_MODE(cmd->DdrMode));

  /* Process locked */
  __HAL_LOCK(hesmc);

  if(hesmc->State == HAL_ESMC_STATE_READY)
  {
    hesmc->ErrorCode = HAL_ESMC_ERROR_NONE;

    /* Update ESMC state */
    hesmc->State = HAL_ESMC_STATE_BUSY;

    /* Wait till idle state set */
    status = ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_IDLE, SET, tickstart, hesmc->Timeout);

    if (status == HAL_OK)
    {
      if ((cmd->InstructionMode != ESMC_INSTRUCTION_NONE) && \
          (cmd->AddressMode == ESMC_ADDRESS_NONE) && \
          (cmd->AlternateByte == ESMC_ALTERNATE_BYTES_DISABLE)&& \
          (cmd->DataMode == ESMC_DATA_NONE))
      {
        /* Clear idle interrupt */
        __HAL_ESMC_CLEAR_FLAG(hesmc, ESMC_FLAG_IDLEIF);

        /* Call the configuration function  */
        ESMC_Config(hesmc, cmd, ESMC_FUNCTIONAL_MODE_INDIRECT_INSTRUCT);

        /* Process unlocked */
        __HAL_UNLOCK(hesmc);

        /* Enable the ESMC idle Interrupt */
        __HAL_ESMC_ENABLE_GLOBAL_IT(hesmc);
        __HAL_ESMC_ENABLE_IT(hesmc, ESMC_IT_IDLE);
      }
      else
      {
        /* Call the configuration function  */
        ESMC_Config(hesmc, cmd, ESMC_FUNCTIONAL_MODE_INDIRECT_READ_WRITE);

        if (cmd->DataMode == ESMC_DATA_NONE)
        {
          /* Process unlocked */
          __HAL_UNLOCK(hesmc);

          /* Enable the ESMC datawait Interrupt */
          __HAL_ESMC_ENABLE_GLOBAL_IT(hesmc);
          __HAL_ESMC_ENABLE_IT(hesmc, ESMC_IT_DATAWAIT);
        } else
        {
          /* Update ESMC state */
          hesmc->State = HAL_ESMC_STATE_READY;

          /* Process unlocked */
          __HAL_UNLOCK(hesmc);
        }
      }
    }
  }
  else
  {
    status = HAL_BUSY;

    /* Process unlocked */
    __HAL_UNLOCK(hesmc);
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Configure the Memory Mapped mode.
  * @param  hesmc : ESMC handle
  * @param  cmd : structure that contains the command configuration information.
  * @note   This function is used only in Memory mapped Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ESMC_MemoryMapped(ESMC_HandleTypeDef *hesmc, ESMC_CommandTypeDef *cmd)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();

  /* Check the parameters */
  assert_param(IS_ESMC_INSTRUCTION_MODE(cmd->InstructionMode));
  if (cmd->InstructionMode != ESMC_INSTRUCTION_NONE)
  {
    assert_param(IS_ESMC_INSTRUCTION(cmd->Instruction));
  }

  assert_param(IS_ESMC_ADDRESS_MODE(cmd->AddressMode));
  if (cmd->AddressMode != ESMC_ADDRESS_NONE)
  {
    assert_param(IS_ESMC_ADDRESS_SIZE(cmd->AddressSize));
  }

  assert_param(IS_ESMC_ALTERNATE_BYTES_MODE(cmd->AlternateByteMode));
  if (cmd->AlternateByteMode != ESMC_ALTERNATE_BYTES_DISABLE)
  {
    assert_param(IS_ESMC_ALTERNATE_BYTE(cmd->AlternateByte));
  }

  assert_param(IS_ESMC_DUMMY_CYCLES(cmd->DummyCycles));
  assert_param(IS_ESMC_DATA_MODE(cmd->DataMode));

  assert_param(IS_ESMC_DDR_MODE(cmd->DdrMode));

  /* Process locked */
  __HAL_LOCK(hesmc);

  if(hesmc->State == HAL_ESMC_STATE_READY)
  {
    hesmc->ErrorCode = HAL_ESMC_ERROR_NONE;

    /* Update state */
    hesmc->State = HAL_ESMC_STATE_BUSY_MEM_MAPPED;

    /* Wait till IDLE flag set */
    status = ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_IDLE, SET, tickstart, hesmc->Timeout);

    if (status == HAL_OK)
    {
      /* Disable XIP mode, DMA, global interrupt */
      CLEAR_BIT(hesmc->Instance->CR, (ESMC_CR_XIPEN | ESMC_CR_DMAEN | ESMC_CR_GIE));

      /* Call the configuration function */
      ESMC_Config(hesmc, cmd, ESMC_FUNCTIONAL_MODE_MEMORY_MAPPED);

      /* Enable XIP mode */
      SET_BIT(hesmc->Instance->CR, ESMC_CR_XIPEN);

      /* Enable the slave select */
      __HAL_ESMC_ENABLE_SLAVE(hesmc);
    }
  }
  else
  {
    status = HAL_BUSY;
  }

  /* Process unlocked */
  __HAL_UNLOCK(hesmc);

  /* Return function status */
  return status;
}

/**
  * @brief Transmit an amount of data in blocking mode.
  * @param hesmc ESMC handle
  * @param pData pointer to data buffer
  * @param Timeout  Time out duration
  * @note   This function is used only in Indirect Write Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ESMC_Transmit(ESMC_HandleTypeDef *hesmc, uint8_t *pData, uint32_t Timeout)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t tickstart = HAL_GetTick();
  uint8_t first_data_flag = 1;

  /* Process locked */
  __HAL_LOCK(hesmc);

  if(hesmc->State == HAL_ESMC_STATE_READY)
  {
    hesmc->ErrorCode = HAL_ESMC_ERROR_NONE;

    if((pData != NULL) && (hesmc->TxXferCount != 0) && (hesmc->TxXferSize != 0))
    {
      /* Update state */
      hesmc->State = HAL_ESMC_STATE_BUSY_INDIRECT_TX;

      hesmc->pTxBuffPtr = pData;

      while(hesmc->TxXferCount != 0U)
      {
        if ((hesmc->TxXferCount/HAL_ESMC_FIFO_SIZE) > 0)
        {
          ESMC_WriteBuff(hesmc, hesmc->pTxBuffPtr, HAL_ESMC_FIFO_SIZE);
          hesmc->pTxBuffPtr += HAL_ESMC_FIFO_SIZE;
          hesmc->TxXferCount -= HAL_ESMC_FIFO_SIZE;
        } else
        {
          ESMC_WriteBuff(hesmc, hesmc->pTxBuffPtr, hesmc->TxXferCount);
          hesmc->pTxBuffPtr += hesmc->TxXferCount;
          hesmc->TxXferCount = 0;
        }

        if (first_data_flag == 1)
        {
          first_data_flag = 0;
          /* send command */
          hesmc->Instance->ADDR24 = hesmc->ContexAddr24;
        }

        /* Wait until fifo empty flag is set */
        status = ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_FIFOEIF, SET, tickstart, Timeout);
        if (status != HAL_OK)
        {
          break;
        }
      }

      if (status == HAL_OK)
      {
        if (hesmc->Contex == ESMC_CONTEXT_WRITE_FIXED_LENGTH)
        {
          /* Wait until IDLE flag is set */
          status = ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_IDLEIF, SET, tickstart, Timeout);
          if (status == HAL_OK)
          {
            /* Clear flag */
            __HAL_ESMC_CLEAR_FLAG(hesmc, ESMC_FLAG_IDLEIF);
          }
        } else
        {
          /* Wait until DATAWAIT flag is set */
          status = ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_DATAWAITIF, SET, tickstart, Timeout);
          if (status == HAL_OK)
          {
            /* Disable the slave select */
            __HAL_ESMC_DISABLE_SLAVE(hesmc);
          }
        }
      }

      /* Update ESMC state */
      hesmc->State = HAL_ESMC_STATE_READY;
    }
    else
    {
      hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_PARAM;
      status = HAL_ERROR;
    }
  }
  else
  {
    status = HAL_BUSY;
  }

  /* Process unlocked */
  __HAL_UNLOCK(hesmc);

  return status;
}

/**
  * @brief Receive an amount of data in blocking mode
  * @param hesmc ESMC handle
  * @param pData pointer to data buffer
  * @param Timeout  Time out duration
  * @note   This function is used only in Indirect Read Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ESMC_Receive(ESMC_HandleTypeDef *hesmc, uint8_t *pData, uint32_t Timeout)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t tickstart = HAL_GetTick();

  /* Process locked */
  __HAL_LOCK(hesmc);

  if(hesmc->State == HAL_ESMC_STATE_READY)
  {
    hesmc->ErrorCode = HAL_ESMC_ERROR_NONE;

    if((pData != NULL) && (hesmc->RxXferCount != 0) && (hesmc->RxXferSize != 0))
    {
      /* Update state */
      hesmc->State = HAL_ESMC_STATE_BUSY_INDIRECT_RX;

      hesmc->pRxBuffPtr = pData;

      /* send command */
      hesmc->Instance->ADDR24 = hesmc->ContexAddr24;

      while ((hesmc->RxXferCount/HAL_ESMC_FIFO_SIZE) > 0U)
      {
        /* Wait until fifo full flag is set */
        if (ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_FIFOFIF, SET, tickstart, Timeout) != HAL_OK)
        {
          break;
        }
        ESMC_ReadBuff(hesmc, hesmc->pRxBuffPtr, HAL_ESMC_FIFO_SIZE);
        hesmc->pRxBuffPtr += HAL_ESMC_FIFO_SIZE;
        hesmc->RxXferCount -= HAL_ESMC_FIFO_SIZE;
      }
      while (hesmc->RxXferCount != 0)
      {
        if (hesmc->Contex == ESMC_CONTEXT_READ_FIXED_LENGTH)
        {
          /* Wait until idle flag is set */
          if (ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_IDLEIF, SET, tickstart, Timeout) != HAL_OK)
          {
            break;
          }
        } else if (hesmc->Contex == ESMC_CONTEXT_READ_VARIABLE_LENGTH)
        {
          if (hesmc->RxXferCount <= (HAL_ESMC_FIFO_SIZE/2))
          {
            /* Wait until fifo half full flag is set */
            if (ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_FIFOHIF, SET, tickstart, Timeout) != HAL_OK)
            {
              break;
            }
          } else
          {
            /* Wait until fifo full flag is set */
            if (ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_FIFOFIF, SET, tickstart, Timeout) != HAL_OK)
            {
              break;
            }
          }
        }
        ESMC_ReadBuff(hesmc, hesmc->pRxBuffPtr, hesmc->RxXferCount);
        hesmc->pRxBuffPtr += hesmc->RxXferCount;
        hesmc->RxXferCount = 0;
      }

      if (status == HAL_OK)
      {
        if (hesmc->Contex == ESMC_CONTEXT_READ_VARIABLE_LENGTH)
        {
          /* Disable the slave select */
          __HAL_ESMC_DISABLE_SLAVE(hesmc);
        }
      }

      /* Update ESMC state */
      hesmc->State = HAL_ESMC_STATE_READY;
    }
    else
    {
      hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_PARAM;
      status = HAL_ERROR;
    }
  }
  else
  {
    status = HAL_BUSY;
  }

  /* Process unlocked */
  __HAL_UNLOCK(hesmc);

  return status;
}


/**
  * @brief  Send an amount of data in interrupt mode
  * @param  hesmc ESMC handle
  * @param  pData pointer to data buffer
  * @note   This function is used only in Indirect Write Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ESMC_Transmit_IT(ESMC_HandleTypeDef *hesmc, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hesmc);

  if(hesmc->State == HAL_ESMC_STATE_READY)
  {
    hesmc->ErrorCode = HAL_ESMC_ERROR_NONE;

    if((pData != NULL) && (hesmc->TxXferCount != 0) && (hesmc->TxXferSize != 0))
    {
      /* Update state */
      hesmc->State = HAL_ESMC_STATE_BUSY_INDIRECT_TX;

      hesmc->pTxBuffPtr = pData;

      if ((hesmc->TxXferCount/HAL_ESMC_FIFO_SIZE) > 0)
      {
        ESMC_WriteBuff(hesmc, hesmc->pTxBuffPtr, HAL_ESMC_FIFO_SIZE);
        hesmc->pTxBuffPtr += HAL_ESMC_FIFO_SIZE;
        hesmc->TxXferCount -= HAL_ESMC_FIFO_SIZE;
      } else
      {
        ESMC_WriteBuff(hesmc, hesmc->pTxBuffPtr, hesmc->TxXferCount);
        hesmc->pTxBuffPtr += hesmc->TxXferCount;
        hesmc->TxXferCount = 0;
      }

      /* send command */
      hesmc->Instance->ADDR24 = hesmc->ContexAddr24;

      /* Process unlocked */
      __HAL_UNLOCK(hesmc);

      /* Enable the ESMC Interrupt */
      __HAL_ESMC_ENABLE_GLOBAL_IT(hesmc);
      if (hesmc->Contex == ESMC_CONTEXT_WRITE_FIXED_LENGTH)
      {
        __HAL_ESMC_ENABLE_IT(hesmc, (ESMC_IT_DATAWAIT | ESMC_IT_IDLE));
      } else
      {
        __HAL_ESMC_ENABLE_IT(hesmc, ESMC_IT_DATAWAIT);
      }
    }
    else
    {
      hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_PARAM;
      status = HAL_ERROR;

      /* Process unlocked */
      __HAL_UNLOCK(hesmc);
    }
  }
  else
  {
    status = HAL_BUSY;

    /* Process unlocked */
    __HAL_UNLOCK(hesmc);
  }

  return status;
}

/**
  * @brief  Receive an amount of data in no-blocking mode with Interrupt
  * @param  hesmc ESMC handle
  * @param  pData pointer to data buffer
  * @note   This function is used only in Indirect Read Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ESMC_Receive_IT(ESMC_HandleTypeDef *hesmc, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hesmc);

  if(hesmc->State == HAL_ESMC_STATE_READY)
  {
    hesmc->ErrorCode = HAL_ESMC_ERROR_NONE;

    if((pData != NULL) && (hesmc->RxXferCount != 0) && (hesmc->RxXferSize != 0))
    {
      /* Update state */
      hesmc->State = HAL_ESMC_STATE_BUSY_INDIRECT_RX;

      hesmc->pRxBuffPtr = pData;

      /* send command */
      hesmc->Instance->ADDR24 = hesmc->ContexAddr24;

      /* Process unlocked */
      __HAL_UNLOCK(hesmc);

      /* Enable the ESMC Interrupt */
      __HAL_ESMC_ENABLE_GLOBAL_IT(hesmc);
      if (hesmc->Contex == ESMC_CONTEXT_READ_FIXED_LENGTH)
      {
        __HAL_ESMC_ENABLE_IT(hesmc, (ESMC_IT_DATAWAIT | ESMC_IT_IDLE));
      } else
      {
        __HAL_ESMC_ENABLE_IT(hesmc, ESMC_IT_DATAWAIT);
      }
    }
    else
    {
      hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_PARAM;
      status = HAL_ERROR;

      /* Process unlocked */
      __HAL_UNLOCK(hesmc);
    }
  }
  else
  {
    status = HAL_BUSY;

    /* Process unlocked */
    __HAL_UNLOCK(hesmc);
  }

  return status;
}

/**
  * @brief  Sends an amount of data in non blocking mode with DMA.
  * @param  hesmc ESMC handle
  * @param  pData pointer to data buffer
  * @note   This function is used only in Indirect Write Mode
  * @note   If DMA peripheral access is configured as halfword, the number
  *         of data and the fifo threshold should be aligned on halfword
  * @note   If DMA peripheral access is configured as word, the number
  *         of data and the fifo threshold should be aligned on word
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ESMC_Transmit_DMA(ESMC_HandleTypeDef *hesmc, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t *pData32 = (uint32_t *)pData;

  /* Process locked */
  __HAL_LOCK(hesmc);

  if(hesmc->State == HAL_ESMC_STATE_READY)
  {
    /* Clear the error code */
    hesmc->ErrorCode = HAL_ESMC_ERROR_NONE;

    if((pData != NULL) && (hesmc->TxXferCount != 0) && (hesmc->TxXferSize != 0))
    {
      /* Configure counters of the handle */
      if ((hesmc->hdmatx->Init.PeriphDataAlignment == DMA_PDATAALIGN_WORD) && ((hesmc->TxXferSize % 4U) == 0U))
      {
        hesmc->TxXferCount = (hesmc->TxXferSize >> 2U);
      }
      else
      {
        /* The number of data is not aligned on word
        => no transfer possible with DMA peripheral access configured as word */
        hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_PARAM;
        status = HAL_ERROR;

        /* Process unlocked */
        __HAL_UNLOCK(hesmc);
      }

      if (status == HAL_OK)
      {
        /* Update state */
        hesmc->State = HAL_ESMC_STATE_BUSY_INDIRECT_TX;

        /* Configure size and pointer of the handle */
        hesmc->TxXferSize = hesmc->TxXferCount;
        hesmc->pTxBuffPtr = pData;

        /* Set the ESMC DMA transfer complete callback */
        hesmc->hdmatx->XferCpltCallback = ESMC_DMATxCplt;

        /* Set the ESMC DMA Half transfer complete callback */
        hesmc->hdmatx->XferHalfCpltCallback = ESMC_DMATxHalfCplt;

        /* Set the DMA error callback */
        hesmc->hdmatx->XferErrorCallback = ESMC_DMAError;

        /* Clear the DMA abort callback */
        hesmc->hdmatx->XferAbortCallback = NULL;

        /* Configure the direction of the DMA */
        hesmc->hdmatx->Init.Direction = DMA_MEMORY_TO_PERIPH;
        MODIFY_REG(hesmc->hdmatx->Instance->CCR, DMA_CCR_DIR, hesmc->hdmatx->Init.Direction);

        hesmc->Instance->DATA = *pData32++;

        /* Enable the ESMC transmit DMA Channel */
        if (HAL_DMA_Start_IT(hesmc->hdmatx, (uint32_t)pData32, (uint32_t)&hesmc->Instance->DATA, hesmc->TxXferSize-1) == HAL_OK)
        {
          /* Process unlocked */
          __HAL_UNLOCK(hesmc);

          /* Enable the DMA transfer by setting the DMAEN bit in the ESMC CR register */
          SET_BIT(hesmc->Instance->CR, ESMC_CR_DMAEN);

          /* send command */
          hesmc->Instance->ADDR24 = hesmc->ContexAddr24;
        }
        else
        {
          status = HAL_ERROR;
          hesmc->ErrorCode |= HAL_ESMC_ERROR_DMA;
          hesmc->State = HAL_ESMC_STATE_READY;

          /* Process unlocked */
          __HAL_UNLOCK(hesmc);
        }
      }
    }
    else
    {
      hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_PARAM;
      status = HAL_ERROR;

      /* Process unlocked */
      __HAL_UNLOCK(hesmc);
    }
  }
  else
  {
    status = HAL_BUSY;

    /* Process unlocked */
    __HAL_UNLOCK(hesmc);
  }

  return status;
}

/**
  * @brief  Receives an amount of data in non blocking mode with DMA.
  * @param  hesmc ESMC handle
  * @param  pData pointer to data buffer.
  * @note   This function is used only in Indirect Read Mode
  * @note   If DMA peripheral access is configured as halfword, the number
  *         of data and the fifo threshold should be aligned on halfword
  * @note   If DMA peripheral access is configured as word, the number
  *         of data and the fifo threshold should be aligned on word
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ESMC_Receive_DMA(ESMC_HandleTypeDef *hesmc, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hesmc);

  if(hesmc->State == HAL_ESMC_STATE_READY)
  {
    /* Clear the error code */
    hesmc->ErrorCode = HAL_ESMC_ERROR_NONE;

    if((pData != NULL) && (hesmc->RxXferCount != 0) && (hesmc->RxXferSize != 0))
    {
      /* Configure counters of the handle */
      if ((hesmc->hdmarx->Init.PeriphDataAlignment == DMA_PDATAALIGN_WORD) && ((hesmc->RxXferSize % 4U) == 0U))
      {
        hesmc->RxXferCount = (hesmc->RxXferSize >> 2U);
      }
      else
      {
        /* The number of data is not aligned on word
             => no transfer possible with DMA peripheral access configured as word */
        hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_PARAM;
        status = HAL_ERROR;

        /* Process unlocked */
        __HAL_UNLOCK(hesmc);
      }

      if (status == HAL_OK)
      {
        /* Update state */
        hesmc->State = HAL_ESMC_STATE_BUSY_INDIRECT_RX;

        /* Configure size and pointer of the handle */
        hesmc->RxXferSize = hesmc->RxXferCount;
        hesmc->pRxBuffPtr = pData;

        /* send command */
        hesmc->Instance->ADDR24 = hesmc->ContexAddr24;

        /* Set the ESMC DMA transfer complete callback */
        hesmc->hdmarx->XferCpltCallback = ESMC_DMARxCplt;

        /* Set the ESMC DMA Half transfer complete callback */
        hesmc->hdmarx->XferHalfCpltCallback = ESMC_DMARxHalfCplt;

        /* Set the DMA error callback */
        hesmc->hdmarx->XferErrorCallback = ESMC_DMAError;

        /* Clear the DMA abort callback */
        hesmc->hdmarx->XferAbortCallback = NULL;

        /* Configure the direction of the DMA */
        hesmc->hdmarx->Init.Direction = DMA_PERIPH_TO_MEMORY;
        MODIFY_REG(hesmc->hdmarx->Instance->CCR, DMA_CCR_DIR, hesmc->hdmarx->Init.Direction);

        /* Enable the DMA Channel */
        if (HAL_DMA_Start_IT(hesmc->hdmarx, (uint32_t)&hesmc->Instance->DATA, (uint32_t)pData, hesmc->RxXferSize) == HAL_OK)
        {
          /* Process unlocked */
          __HAL_UNLOCK(hesmc);

          /* Enable the DMA transfer by setting the DMAEN bit in the ESMC CR register */
          SET_BIT(hesmc->Instance->CR, ESMC_CR_DMAEN);
        }
        else
        {
          status = HAL_ERROR;
          hesmc->ErrorCode |= HAL_ESMC_ERROR_DMA;
          hesmc->State = HAL_ESMC_STATE_READY;

          /* Process unlocked */
          __HAL_UNLOCK(hesmc);
        }
      }
    }
    else
    {
      hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_PARAM;
      status = HAL_ERROR;

      /* Process unlocked */
      __HAL_UNLOCK(hesmc);
    }
  }
  else
  {
    status = HAL_BUSY;

    /* Process unlocked */
    __HAL_UNLOCK(hesmc);
  }

  return status;
}


/**
  * @brief  Transfer Error callbacks
  * @param  hesmc ESMC handle
  * @retval None
  */
__weak void HAL_ESMC_ErrorCallback(ESMC_HandleTypeDef *hesmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hesmc);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_ESMC_ErrorCallback could be implemented in the user file
   */
}

/**
  * @brief  Abort completed callback.
  * @param  hesmc ESMC handle
  * @retval None
  */
__weak void HAL_ESMC_AbortCpltCallback(ESMC_HandleTypeDef *hesmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hesmc);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_ESMC_AbortCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Command completed callback.
  * @param  hesmc ESMC handle
  * @retval None
  */
__weak void HAL_ESMC_CmdCpltCallback(ESMC_HandleTypeDef *hesmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hesmc);

  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_ESMC_CmdCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  hesmc ESMC handle
  * @retval None
  */
__weak void HAL_ESMC_RxCpltCallback(ESMC_HandleTypeDef *hesmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hesmc);

  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_ESMC_RxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Tx Transfer completed callbacks.
  * @param  hesmc ESMC handle
  * @retval None
  */
__weak void HAL_ESMC_TxCpltCallback(ESMC_HandleTypeDef *hesmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hesmc);

  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_ESMC_TxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Rx Half Transfer completed callbacks.
  * @param  hesmc ESMC handle
  * @retval None
  */
__weak void HAL_ESMC_RxHalfCpltCallback(ESMC_HandleTypeDef *hesmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hesmc);

  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_ESMC_RxHalfCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Tx Half Transfer completed callbacks.
  * @param  hesmc ESMC handle
  * @retval None
  */
__weak void HAL_ESMC_TxHalfCpltCallback(ESMC_HandleTypeDef *hesmc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hesmc);

  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_ESMC_TxHalfCpltCallback could be implemented in the user file
   */
}

#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User ESMC Callback
  *         To be used instead of the weak (surcharged) predefined callback
  * @param hesmc : ESMC handle
  * @param CallbackId : ID of the callback to be registered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_ESMC_ERROR_CB_ID          ESMC Error Callback ID
  *          @arg @ref HAL_ESMC_ABORT_CB_ID          ESMC Abort Callback ID
  *          @arg @ref HAL_ESMC_CMD_CPLT_CB_ID       ESMC Command Complete Callback ID
  *          @arg @ref HAL_ESMC_RX_CPLT_CB_ID        ESMC Rx Complete Callback ID
  *          @arg @ref HAL_ESMC_TX_CPLT_CB_ID        ESMC Tx Complete Callback ID
  *          @arg @ref HAL_ESMC_RX_HALF_CPLT_CB_ID   ESMC Rx Half Complete Callback ID
  *          @arg @ref HAL_ESMC_TX_HALF_CPLT_CB_ID   ESMC Tx Half Complete Callback ID
  *          @arg @ref HAL_ESMC_MSP_INIT_CB_ID       ESMC MspInit callback ID
  *          @arg @ref HAL_ESMC_MSP_DEINIT_CB_ID     ESMC MspDeInit callback ID
  * @param pCallback : pointer to the Callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_ESMC_RegisterCallback (ESMC_HandleTypeDef *hesmc, HAL_ESMC_CallbackIDTypeDef CallbackId, pESMC_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if(pCallback == NULL)
  {
    /* Update the error code */
    hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_CALLBACK;
    return HAL_ERROR;
  }

  /* Process locked */
  __HAL_LOCK(hesmc);

  if(hesmc->State == HAL_ESMC_STATE_READY)
  {
    switch (CallbackId)
    {
    case  HAL_ESMC_ERROR_CB_ID :
      hesmc->ErrorCallback = pCallback;
      break;
    case HAL_ESMC_ABORT_CB_ID :
      hesmc->AbortCpltCallback = pCallback;
      break;
    case HAL_ESMC_CMD_CPLT_CB_ID :
      hesmc->CmdCpltCallback = pCallback;
      break;
    case HAL_ESMC_RX_CPLT_CB_ID :
      hesmc->RxCpltCallback = pCallback;
      break;
    case HAL_ESMC_TX_CPLT_CB_ID :
      hesmc->TxCpltCallback = pCallback;
      break;
    case HAL_ESMC_RX_HALF_CPLT_CB_ID :
      hesmc->RxHalfCpltCallback = pCallback;
      break;
    case HAL_ESMC_TX_HALF_CPLT_CB_ID :
      hesmc->TxHalfCpltCallback = pCallback;
      break;
    case HAL_ESMC_MSP_INIT_CB_ID :
      hesmc->MspInitCallback = pCallback;
      break;
    case HAL_ESMC_MSP_DEINIT_CB_ID :
      hesmc->MspDeInitCallback = pCallback;
      break;
    default :
      /* Update the error code */
      hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if (hesmc->State == HAL_ESMC_STATE_RESET)
  {
    switch (CallbackId)
    {
    case HAL_ESMC_MSP_INIT_CB_ID :
      hesmc->MspInitCallback = pCallback;
      break;
    case HAL_ESMC_MSP_DEINIT_CB_ID :
      hesmc->MspDeInitCallback = pCallback;
      break;
    default :
      /* Update the error code */
      hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* Update the error code */
    hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_CALLBACK;
    /* update return status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hesmc);
  return status;
}

/**
  * @brief  Unregister a User ESMC Callback
  *         ESMC Callback is redirected to the weak (surcharged) predefined callback
  * @param hesmc : ESMC handle
  * @param CallbackId : ID of the callback to be unregistered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_ESMC_ERROR_CB_ID          ESMC Error Callback ID
  *          @arg @ref HAL_ESMC_ABORT_CB_ID          ESMC Abort Callback ID
  *          @arg @ref HAL_ESMC_CMD_CPLT_CB_ID       ESMC Command Complete Callback ID
  *          @arg @ref HAL_ESMC_RX_CPLT_CB_ID        ESMC Rx Complete Callback ID
  *          @arg @ref HAL_ESMC_TX_CPLT_CB_ID        ESMC Tx Complete Callback ID
  *          @arg @ref HAL_ESMC_RX_HALF_CPLT_CB_ID   ESMC Rx Half Complete Callback ID
  *          @arg @ref HAL_ESMC_TX_HALF_CPLT_CB_ID   ESMC Tx Half Complete Callback ID
  *          @arg @ref HAL_ESMC_MSP_INIT_CB_ID       ESMC MspInit callback ID
  *          @arg @ref HAL_ESMC_MSP_DEINIT_CB_ID     ESMC MspDeInit callback ID
  * @retval status
  */
HAL_StatusTypeDef HAL_ESMC_UnRegisterCallback (ESMC_HandleTypeDef *hesmc, HAL_ESMC_CallbackIDTypeDef CallbackId)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hesmc);

  if(hesmc->State == HAL_ESMC_STATE_READY)
  {
    switch (CallbackId)
    {
    case  HAL_ESMC_ERROR_CB_ID :
      hesmc->ErrorCallback = HAL_ESMC_ErrorCallback;
      break;
    case HAL_ESMC_ABORT_CB_ID :
      hesmc->AbortCpltCallback = HAL_ESMC_AbortCpltCallback;
      break;
    case HAL_ESMC_CMD_CPLT_CB_ID :
      hesmc->CmdCpltCallback = HAL_ESMC_CmdCpltCallback;
      break;
    case HAL_ESMC_RX_CPLT_CB_ID :
      hesmc->RxCpltCallback = HAL_ESMC_RxCpltCallback;
      break;
    case HAL_ESMC_TX_CPLT_CB_ID :
      hesmc->TxCpltCallback = HAL_ESMC_TxCpltCallback;
      break;
    case HAL_ESMC_RX_HALF_CPLT_CB_ID :
      hesmc->RxHalfCpltCallback = HAL_ESMC_RxHalfCpltCallback;
      break;
    case HAL_ESMC_TX_HALF_CPLT_CB_ID :
      hesmc->TxHalfCpltCallback = HAL_ESMC_TxHalfCpltCallback;
      break;
    case HAL_ESMC_MSP_INIT_CB_ID :
      hesmc->MspInitCallback = HAL_ESMC_MspInit;
      break;
    case HAL_ESMC_MSP_DEINIT_CB_ID :
      hesmc->MspDeInitCallback = HAL_ESMC_MspDeInit;
      break;
    default :
      /* Update the error code */
      hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if (hesmc->State == HAL_ESMC_STATE_RESET)
  {
    switch (CallbackId)
    {
    case HAL_ESMC_MSP_INIT_CB_ID :
      hesmc->MspInitCallback = HAL_ESMC_MspInit;
      break;
    case HAL_ESMC_MSP_DEINIT_CB_ID :
      hesmc->MspDeInitCallback = HAL_ESMC_MspDeInit;
      break;
    default :
      /* Update the error code */
      hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_CALLBACK;
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* Update the error code */
    hesmc->ErrorCode |= HAL_ESMC_ERROR_INVALID_CALLBACK;
    /* update return status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hesmc);
  return status;
}
#endif

/**
  * @}
  */

/** @defgroup ESMC_Exported_Functions_Group3 Peripheral Control and State functions
  *  @brief   ESMC control and State functions
  *
@verbatim
 ===============================================================================
                  ##### Peripheral Control and State functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to :
      (+) Check in run-time the state of the driver.
      (+) Check the error code set during last operation.
      (+) Abort any operation.
.....
@endverbatim
  * @{
  */

/**
  * @brief  Return the ESMC handle state.
  * @param  hesmc ESMC handle
  * @retval HAL state
  */
HAL_ESMC_StateTypeDef HAL_ESMC_GetState(ESMC_HandleTypeDef *hesmc)
{
  /* Return ESMC handle state */
  return hesmc->State;
}

/**
* @brief  Return the ESMC error code
* @param  hesmc ESMC handle
* @retval ESMC Error Code
*/
uint32_t HAL_ESMC_GetError(ESMC_HandleTypeDef *hesmc)
{
  return hesmc->ErrorCode;
}

/**
* @brief  Abort the current transmission
* @param  hesmc ESMC handle
* @retval HAL status
*/
HAL_StatusTypeDef HAL_ESMC_Abort(ESMC_HandleTypeDef *hesmc)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t tickstart = HAL_GetTick();

  /* Check if the state is in one of the busy states */
  if ((hesmc->State & 0x2) != 0)
  {
    /* Process unlocked */
    __HAL_UNLOCK(hesmc);

    if ((hesmc->Instance->CR & ESMC_CR_DMAEN)!= RESET)
    {
      /* Disable the DMA transfer by clearing the DMAEN bit in the ESMC CR register */
      CLEAR_BIT(hesmc->Instance->CR, ESMC_CR_DMAEN);

      /* Abort DMA channel */
      if (hesmc->State == HAL_ESMC_STATE_BUSY_INDIRECT_RX)
      {
        status = HAL_DMA_Abort(hesmc->hdmarx);
      }
      else if (hesmc->State == HAL_ESMC_STATE_BUSY_INDIRECT_TX)
      {
        status = HAL_DMA_Abort(hesmc->hdmatx);
      }

      if(status != HAL_OK)
      {
        hesmc->ErrorCode |= HAL_ESMC_ERROR_DMA;
      }
    }

    /* Disable the slave select, Abort communication */
    __HAL_ESMC_DISABLE_SLAVE(hesmc);

    /* Wait until IDLE flag is set to go back in idle state */
    status = ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_IDLE, SET, tickstart, hesmc->Timeout);

    if(status == HAL_OK)
    {
      /* Update state */
      hesmc->State = HAL_ESMC_STATE_READY;
    }
  }

  return status;
}

/**
* @brief  Abort the current transmission (non-blocking function)
* @param  hesmc ESMC handle
* @retval HAL status
*/
HAL_StatusTypeDef HAL_ESMC_Abort_IT(ESMC_HandleTypeDef *hesmc)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t tickstart = HAL_GetTick();

  /* Check if the state is in one of the busy states */
  if ((hesmc->State & 0x2) != 0)
  {
    /* Process unlocked */
    __HAL_UNLOCK(hesmc);

    /* Disable global interrupts */
    __HAL_ESMC_DISABLE_GLOBAL_IT(hesmc);
    /* Disable all interrupts */
    __HAL_ESMC_DISABLE_IT(hesmc, ESMC_IT_ALL);

    if ((hesmc->Instance->CR & ESMC_CR_DMAEN)!= RESET)
    {
      /* Disable the DMA transfer by clearing the DMAEN bit in the ESMC CR register */
      CLEAR_BIT(hesmc->Instance->CR, ESMC_CR_DMAEN);

      /* Abort DMA channel */
      hesmc->hdmarx->XferAbortCallback = ESMC_DMAAbortCplt;
      if (hesmc->State == HAL_ESMC_STATE_BUSY_INDIRECT_RX)
      {
        status = HAL_DMA_Abort_IT(hesmc->hdmarx);
      }
      else if (hesmc->State == HAL_ESMC_STATE_BUSY_INDIRECT_TX)
      {
        status = HAL_DMA_Abort_IT(hesmc->hdmarx);
      }
      if (status != HAL_OK)
      {
        /* Change state of ESMC */
        hesmc->State = HAL_ESMC_STATE_READY;

        /* Abort Complete callback */
#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
        hesmc->AbortCpltCallback(hesmc);
#else
        HAL_ESMC_AbortCpltCallback(hesmc);
#endif
      }
    }

    /* Disable the slave select, Abort communication */
    __HAL_ESMC_DISABLE_SLAVE(hesmc);

    /* Wait until IDLE flag is set to go back in idle state */
    status = ESMC_WaitFlagStateUntilTimeout(hesmc, ESMC_FLAG_IDLE, SET, tickstart, hesmc->Timeout);

    if(status == HAL_OK)
    {
      /* Update state */
      hesmc->State = HAL_ESMC_STATE_READY;
    }
  }

  return status;
}

/** @brief Set ESMC timeout
  * @param  hesmc ESMC handle.
  * @param  Timeout Timeout for the ESMC memory access.
  * @retval None
  */
void HAL_ESMC_SetTimeout(ESMC_HandleTypeDef *hesmc, uint32_t Timeout)
{
  hesmc->Timeout = Timeout;
}


/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  DMA ESMC receive process complete callback.
  * @param  hdma DMA handle
  * @retval None
  */
static void ESMC_DMARxCplt(DMA_HandleTypeDef *hdma)
{
  ESMC_HandleTypeDef* hesmc = ( ESMC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
  hesmc->RxXferCount = 0;

  /* Disable the DMA transfer by clearing the DMAEN bit in the ESMC CR register */
  CLEAR_BIT(hesmc->Instance->CR, ESMC_CR_DMAEN);

  /* Disable the DMA channel */
  __HAL_DMA_DISABLE(hesmc->hdmarx);

  if (hesmc->Contex == ESMC_CONTEXT_READ_VARIABLE_LENGTH)
  {
    /* Disable the slave select */
    __HAL_ESMC_DISABLE_SLAVE(hesmc);
  }

  /* Change state of ESMC */
  hesmc->State = HAL_ESMC_STATE_READY;

  /* Restore default values */
  hesmc->Contex = ESMC_CONTEXT_NONE;
}

/**
  * @brief  DMA ESMC transmit process complete callback.
  * @param  hdma DMA handle
  * @retval None
  */
static void ESMC_DMATxCplt(DMA_HandleTypeDef *hdma)
{
  ESMC_HandleTypeDef* hesmc = ( ESMC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
  hesmc->TxXferCount = 0;

  /* Enable the ESMC Interrupt */
  __HAL_ESMC_ENABLE_GLOBAL_IT(hesmc);
  if (hesmc->Contex == ESMC_CONTEXT_WRITE_FIXED_LENGTH)
  {
    __HAL_ESMC_ENABLE_IT(hesmc, ESMC_IT_IDLE);
  } else
  {
    __HAL_ESMC_ENABLE_IT(hesmc, ESMC_IT_DATAWAIT);
  }
}

/**
  * @brief  DMA ESMC receive process half complete callback
  * @param  hdma  DMA handle
  * @retval None
  */
static void ESMC_DMARxHalfCplt(DMA_HandleTypeDef *hdma)
{
  ESMC_HandleTypeDef* hesmc = (ESMC_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;

#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
  hesmc->RxHalfCpltCallback(hesmc);
#else
  HAL_ESMC_RxHalfCpltCallback(hesmc);
#endif
}

/**
  * @brief  DMA ESMC transmit process half complete callback
  * @param  hdma  DMA handle
  * @retval None
  */
static void ESMC_DMATxHalfCplt(DMA_HandleTypeDef *hdma)
{
  ESMC_HandleTypeDef* hesmc = (ESMC_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;

#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
  hesmc->TxHalfCpltCallback(hesmc);
#else
  HAL_ESMC_TxHalfCpltCallback(hesmc);
#endif
}

/**
  * @brief  DMA ESMC communication error callback.
  * @param  hdma DMA handle
  * @retval None
  */
static void ESMC_DMAError(DMA_HandleTypeDef *hdma)
{
  ESMC_HandleTypeDef* hesmc = ( ESMC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;

  hesmc->RxXferCount = 0;
  hesmc->TxXferCount = 0;
  hesmc->ErrorCode   |= HAL_ESMC_ERROR_DMA;

  /* Disable the DMA transfer by clearing the DMAEN bit in the ESMC CR register */
  CLEAR_BIT(hesmc->Instance->CR, ESMC_CR_DMAEN);

  /* Abort the ESMC */
  HAL_ESMC_Abort_IT(hesmc);

}

/**
  * @brief  DMA ESMC abort complete callback.
  * @param  hdma DMA handle
  * @retval None
  */
static void ESMC_DMAAbortCplt(DMA_HandleTypeDef *hdma)
{
  ESMC_HandleTypeDef* hesmc = ( ESMC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;

  hesmc->RxXferCount = 0;
  hesmc->TxXferCount = 0;

  if(hesmc->State == HAL_ESMC_STATE_ABORT)
  {

  }
  else
  {
    /* DMA Abort called due to a transfer error interrupt */
    /* Change state of ESMC */
    hesmc->State = HAL_ESMC_STATE_READY;

    /* Error callback */
#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
    hesmc->ErrorCallback(hesmc);
#else
    HAL_ESMC_ErrorCallback(hesmc);
#endif
  }
}

/**
  * @brief  Wait for a flag state until timeout.
  * @param  hesmc ESMC handle
  * @param  Flag Flag checked
  * @param  State Value of the flag expected
  * @param  tickstart Start tick value
  * @param  Timeout Duration of the time out
  * @retval HAL status
  */
static HAL_StatusTypeDef ESMC_WaitFlagStateUntilTimeout(ESMC_HandleTypeDef *hesmc, uint32_t Flag,
    FlagStatus State, uint32_t tickstart, uint32_t Timeout)
{
  /* Wait until flag is in expected state */
  while((__HAL_ESMC_GET_FLAG(hesmc, Flag)) != State)
  {
    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if((Timeout == 0) || ((HAL_GetTick() - tickstart) > Timeout))
      {
        hesmc->State     = HAL_ESMC_STATE_ERROR;
        hesmc->ErrorCode |= HAL_ESMC_ERROR_TIMEOUT;

        return HAL_ERROR;
      }
    }
  }
  return HAL_OK;
}

/**
  * @brief  Configure the communication registers.
  * @param  hesmc ESMC handle
  * @param  cmd structure that contains the command configuration information
  * @param  FunctionalMode functional mode to configured
  *           This parameter can be one of the following values:
  *            @arg ESMC_FUNCTIONAL_MODE_WRITE: write mode
  *            @arg ESMC_FUNCTIONAL_MODE_READ: read mode
  * @retval None
  */
static void ESMC_Config(ESMC_HandleTypeDef *hesmc, ESMC_CommandTypeDef *cmd, uint32_t FunctionalMode)
{
  uint32_t TmpSOCR = 0;
  uint32_t TmpDCR = 0;
  uint32_t TmpADDR24 = 0;
  uint32_t TmpADDR32 = 0;

  assert_param(IS_ESMC_FUNCTIONAL_MODE(FunctionalMode));

  if (FunctionalMode == ESMC_FUNCTIONAL_MODE_INDIRECT_INSTRUCT)
  {
    TmpSOCR = hesmc->Instance->SOCR;

    /* configure slave select output */
    __HAL_ESMC_SET_SSXO(hesmc,cmd->CSPinSel);

    /* Disable the slave select by default */
    __HAL_ESMC_DISABLE_SLAVE(hesmc);

    /* configure spi transfer format, instruction operation mode and DDR mode */
    MODIFY_REG(TmpSOCR, (ESMC_SOCR_SPIMODE | ESMC_SOCR_DDRADDR | ESMC_SOCR_DDRCMD | ESMC_SOCR_DDRDATA), \
               (cmd->TransferFormat | cmd->DdrMode));
    if (cmd->InstructionMode  == ESMC_INSTRUCTION_MULTI_LINES)
    {
      SET_BIT(TmpSOCR, ESMC_SOCR_MULTICMD);
    } else
    {
      CLEAR_BIT(TmpSOCR, ESMC_SOCR_MULTICMD);
    }

    /* write config values to register */
    WRITE_REG(hesmc->Instance->SOCR, TmpSOCR);

    /*Make sure the command can be sent*/
    CLEAR_BIT(hesmc->Instance->DCR, ESMC_DCR_NOCMD);

    /* configure command value */
    WRITE_REG(hesmc->Instance->SFCR, cmd->Instruction);

  }	else if (FunctionalMode == ESMC_FUNCTIONAL_MODE_INDIRECT_READ_WRITE)
  {
    TmpSOCR = hesmc->Instance->SOCR;
    TmpDCR  = hesmc->Instance->DCR;
    TmpADDR24 = hesmc->Instance->ADDR24;
    TmpADDR32 = hesmc->Instance->ADDR32;

    /* configure slave select output */
    MODIFY_REG(TmpADDR32, ESMC_ADDR32_SS, (cmd->CSPinSel) << ESMC_ADDR32_SS_Pos);

    /* configure spi transfer format, instruction operation mode and DDR mode */
    MODIFY_REG(TmpSOCR, (ESMC_SOCR_SPIMODE | ESMC_SOCR_DDRADDR | ESMC_SOCR_DDRCMD | ESMC_SOCR_DDRDATA), \
               (cmd->TransferFormat | cmd->DdrMode));

    /* config command region */
    if (cmd->InstructionMode != ESMC_INSTRUCTION_NONE)
    {
      CLEAR_BIT(TmpDCR, ESMC_DCR_NOCMD);

      if (cmd->InstructionMode  == ESMC_INSTRUCTION_MULTI_LINES)
      {
        SET_BIT(TmpSOCR, ESMC_SOCR_MULTICMD);
      } else
      {
        CLEAR_BIT(TmpSOCR, ESMC_SOCR_MULTICMD);
      }
      /* set command value */
      MODIFY_REG(TmpADDR24, ESMC_ADDR24_INS, (uint8_t)cmd->Instruction);
    } else
    {
      /* Omit instruction and begin with Address */
      SET_BIT(TmpDCR, ESMC_DCR_NOCMD);
    }

    /* config address region */
    if (cmd->AddressMode != ESMC_ADDRESS_NONE)
    {
      CLEAR_BIT(TmpDCR, ESMC_DCR_NOADDR);
      if ((cmd->AddressMode == ESMC_ADDRESS_MULTI_LINES) || \
          (cmd->InstructionMode  == ESMC_INSTRUCTION_MULTI_LINES))
      {
        SET_BIT(TmpSOCR, ESMC_SOCR_MULTIADDR);
      } else
      {
        CLEAR_BIT(TmpSOCR, ESMC_SOCR_MULTIADDR);
      }
      /* set address value */
      if (cmd->AddressSize == ESMC_ADDRESS_32_BITS)
      {
        MODIFY_REG(TmpADDR32, ESMC_ADDR32_ADDR3, ((cmd->Address >> 24) & 0x000000FF));
      }
      MODIFY_REG(TmpADDR24, (ESMC_ADDR24_ADDR0 | ESMC_ADDR24_ADDR1 | ESMC_ADDR24_ADDR2), \
                 ((cmd->Address & 0x00FFFFFF) << ESMC_ADDR24_ADDR0_Pos));
      /* set address size */
      MODIFY_REG(hesmc->Instance->CR3, (ESMC_CR3_ADDR8BIT | ESMC_CR3_ADDR16BIT | ESMC_CR3_ADDR32BIT), cmd->AddressSize);
    } else
    {
      /* Do not send out ADDRESS bytes */
      SET_BIT(TmpDCR, ESMC_DCR_NOADDR);
    }

    /* config alternate region */
    if (cmd->AlternateByteMode == ESMC_ALTERNATE_BYTES_ENABLE)
    {
      /* configure alternate byte */
      MODIFY_REG(TmpADDR32, ESMC_ADDR32_MREG, cmd->AlternateByte);
      SET_BIT(TmpSOCR, ESMC_SOCR_SENM);
    } else
    {
      CLEAR_BIT(TmpSOCR, ESMC_SOCR_SENM);
    }

    /* config dummy region */
    MODIFY_REG(TmpDCR, ESMC_DCR_DUMMY, cmd->DummyCycles);

    /* config data region */
    if ((cmd->DataMode == ESMC_DATA_WRITE) && (cmd->NbData != 0))
    {
      /* set write mode */
      CLEAR_BIT(TmpDCR, ESMC_DCR_REC);
      if (cmd->NbData > HAL_ESMC_TCR_MAX_LENGTH)
      {
        /* Read/write of undefined number of data bytes from/to FLASH */
        WRITE_REG(hesmc->Instance->TCR, 0);

        hesmc->Contex = ESMC_CONTEXT_WRITE_VARIABLE_LENGTH;
      } else
      {
        /* Read/write specified number of data bytes from/to FLASH */
        WRITE_REG(hesmc->Instance->TCR, cmd->NbData);

        hesmc->Contex = ESMC_CONTEXT_WRITE_FIXED_LENGTH;
      }

      /* Configure counters and size of the handle */
      hesmc->TxXferCount = cmd->NbData;
      hesmc->TxXferSize  = cmd->NbData;
    } else if ((cmd->DataMode == ESMC_DATA_READ) && (cmd->NbData != 0))
    {
      /* set read mode */
      SET_BIT(TmpDCR, ESMC_DCR_REC);
      if (cmd->NbData > HAL_ESMC_TCR_MAX_LENGTH)
      {
        /* Read/write of undefined number of data bytes from/to FLASH */
        WRITE_REG(hesmc->Instance->TCR, 0);

        hesmc->Contex = ESMC_CONTEXT_READ_VARIABLE_LENGTH;
      } else
      {
        /* Read/write specified number of data bytes from/to FLASH */
        WRITE_REG(hesmc->Instance->TCR, cmd->NbData);

        hesmc->Contex = ESMC_CONTEXT_READ_FIXED_LENGTH;
      }

      /* Configure counters and size of the handle */
      hesmc->RxXferCount = cmd->NbData;
      hesmc->RxXferSize  = cmd->NbData;
    } else
    {
      CLEAR_BIT(TmpDCR, ESMC_DCR_REC);

      WRITE_REG(hesmc->Instance->TCR, 0);

      /* Restore default values */
      hesmc->Contex = ESMC_CONTEXT_NONE;
    }

    /* write register */
    WRITE_REG(hesmc->Instance->ADDR32, TmpADDR32);
    /* Disable the slave select by default */
    __HAL_ESMC_DISABLE_SLAVE(hesmc);

    /* clear fifo */
    __HAL_ESMC_CLEAR_FIFO(hesmc);

    WRITE_REG(hesmc->Instance->SOCR, TmpSOCR);
    WRITE_REG(hesmc->Instance->DCR, TmpDCR);

    /* set command value and address value  */
    if (cmd->DataMode != ESMC_DATA_NONE)
    {
      hesmc->ContexAddr24 = TmpADDR24;
    } else
    {
      WRITE_REG(hesmc->Instance->ADDR24, TmpADDR24);
    }

  } else if (FunctionalMode == ESMC_FUNCTIONAL_MODE_MEMORY_MAPPED)
  {
    TmpSOCR = hesmc->Instance->XSOCR;
    TmpDCR  = hesmc->Instance->XDCR;
    TmpADDR32 = hesmc->Instance->ADDR32;

    /* configure slave select output */
    MODIFY_REG(TmpADDR32, ESMC_ADDR32_XSS, (cmd->CSPinSel) << ESMC_ADDR32_XSS_Pos);

    /* config transmit format */
    MODIFY_REG(TmpSOCR, ESMC_SOCR_SPIMODE, cmd->TransferFormat);

    /* config command region */
    if (cmd->InstructionMode != ESMC_INSTRUCTION_NONE)
    {
      WRITE_REG(hesmc->Instance->XSFCR, cmd->Instruction);

      CLEAR_BIT(TmpDCR, ESMC_DCR_NOCMD);

      if (cmd->InstructionMode  == ESMC_INSTRUCTION_MULTI_LINES)
      {
        SET_BIT(TmpSOCR, ESMC_SOCR_MULTICMD);
      } else
      {
        CLEAR_BIT(TmpSOCR, ESMC_SOCR_MULTICMD);
      }
    } else
    {
      /* Omit instruction and begin with Address */
      SET_BIT(TmpDCR, ESMC_DCR_NOCMD);
    }

    /* config address region */
    CLEAR_BIT(TmpDCR, ESMC_DCR_NOADDR);
    if ((cmd->AddressMode == ESMC_ADDRESS_MULTI_LINES) || \
        (cmd->InstructionMode  == ESMC_INSTRUCTION_MULTI_LINES))
    {
      SET_BIT(TmpSOCR, ESMC_SOCR_MULTIADDR);
    } else
    {
      CLEAR_BIT(TmpSOCR, ESMC_SOCR_MULTIADDR);
    }

    /* set address size */
    MODIFY_REG(hesmc->Instance->XCR3, (ESMC_CR3_ADDR8BIT | ESMC_CR3_ADDR16BIT | ESMC_CR3_ADDR32BIT), cmd->AddressSize);

    /* config alternate region */
    if (cmd->AlternateByteMode == ESMC_ALTERNATE_BYTES_ENABLE)
    {
      /* configure alternate byte */
      MODIFY_REG(TmpADDR32, ESMC_ADDR32_MREG, cmd->AlternateByte);
      SET_BIT(TmpSOCR, ESMC_SOCR_SENM);
    } else
    {
      CLEAR_BIT(TmpSOCR, ESMC_SOCR_SENM);
    }

    /* config dummy region */
    MODIFY_REG(TmpDCR, ESMC_DCR_DUMMY, cmd->DummyCycles);

    /* config data region */
    /* set read mode */
    SET_BIT(TmpDCR, ESMC_DCR_REC);

    /* write register */
    WRITE_REG(hesmc->Instance->ADDR32, TmpADDR32);

    /* Disable the slave select by default */
    __HAL_ESMC_DISABLE_SLAVE(hesmc);

    /* clear fifo */
    __HAL_ESMC_CLEAR_FIFO(hesmc);

    /* write register */
    WRITE_REG(hesmc->Instance->XSOCR, TmpSOCR);
    WRITE_REG(hesmc->Instance->XDCR, TmpDCR);
  }
}

/**
  * @brief  Write data to FIFO.
  * @param  hesmc ESMC handle
  * @param  pData Data buffer pointer
  * @param  dataLen Number of bytes to be written
  * @retval None
  */
void ESMC_WriteBuff(ESMC_HandleTypeDef *hesmc, uint8_t *pData, uint8_t dataLen)
{
  uint8_t len32 = (dataLen & 0xFC) >> 2;
  uint8_t len8 = dataLen & 0x03;
  uint8_t i;
  uint32_t data;
  __IO uint8_t *data_reg8 = (uint8_t *)(&hesmc->Instance->DATA);

  for(i=0; i<len32; i++)
  {
    data = (uint32_t)(*pData++);
    data |= ((uint32_t)(*pData++) << 8);
    data |= ((uint32_t)(*pData++) << 16);
    data |= ((uint32_t)(*pData++) << 24);
    hesmc->Instance->DATA = data;
  }

  for(i=0; i<len8; i++)
  {
    *data_reg8 = *pData++;
  }
}

/**
  * @brief  Write data to FIFO.
  * @param  hesmc ESMC handle
  * @param  pData Data buffer pointer
  * @param  dataLen Number of bytes to be written
  * @retval None
  */
void ESMC_ReadBuff(ESMC_HandleTypeDef *hesmc, uint8_t *pData, uint8_t dataLen)
{
  uint8_t len32 = (dataLen & 0xFC) >> 2;
  uint8_t len8 = dataLen & 0x03;
  uint8_t i;
  uint32_t data;

  for(i=0; i<len32; i++)
  {
    data = hesmc->Instance->DATA;
    *pData++ = (uint8_t)(data & 0xFFU);
    *pData++ = (uint8_t)((data >> 8U) & 0xFFU);
    *pData++ = (uint8_t)((data >> 16U) & 0xFFU);
    *pData++ = (uint8_t)((data >> 24U) & 0xFFU);
  }

  if (len8 != 0)
  {
    data = hesmc->Instance->DATA;
    while (len8--)
    {
      *pData++ = (uint8_t)(data & 0xFFU);
      data >>= 8;
    }
  }
}
/**
  * @}
  */

#endif /* HAL_ESMC_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
