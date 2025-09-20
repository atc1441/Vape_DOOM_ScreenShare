/**
  ******************************************************************************
  * @file    py32f4xx_hal_esmc.h
  * @author  MCU Application Team
  * @brief   Header file of ESMC HAL module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PY32F4xx_HAL_ESMC_H
#define __PY32F4xx_HAL_ESMC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f4xx_hal_def.h"

/** @addtogroup PY32F4xx_HAL_Driver
  * @{
  */

/** @addtogroup ESMC
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup ESMC_Exported_Types ESMC Exported Types
  * @{
  */

/**
  * @brief  ESMC Init structure definition
  */

typedef struct
{
  uint32_t ClockPrescaler;     /* Specifies the prescaler factor for generating clock based on the AHB clock.
                                  This parameter can be a number between 2 and 255 */

  uint32_t ClockMode;          /* Specifies the Clock Mode. It indicates the level that clock takes between commands.
                                  This parameter can be a value of @ref ESMC_ClockMode */

  uint32_t DualFlash;          /* Specifies the Dual Flash Mode State
                                  This parameter can be a value of @ref ESMC_DualFlash_Mode */

} ESMC_InitTypeDef;

/**
  * @brief HAL ESMC State structures definition
  */
typedef enum
{
  HAL_ESMC_STATE_RESET             = 0x00U,    /*!< Peripheral not initialized                            */
  HAL_ESMC_STATE_READY             = 0x01U,    /*!< Peripheral initialized and ready for use              */
  HAL_ESMC_STATE_BUSY              = 0x02U,    /*!< Peripheral in indirect mode and busy                  */
  HAL_ESMC_STATE_BUSY_INDIRECT_TX  = 0x12U,    /*!< Peripheral in indirect mode with transmission ongoing */
  HAL_ESMC_STATE_BUSY_INDIRECT_RX  = 0x22U,    /*!< Peripheral in indirect mode with reception ongoing    */
  HAL_ESMC_STATE_BUSY_AUTO_POLLING = 0x42U,    /*!< Peripheral in auto polling mode ongoing               */
  HAL_ESMC_STATE_BUSY_MEM_MAPPED   = 0x82U,    /*!< Peripheral in memory mapped mode ongoing              */
  HAL_ESMC_STATE_ABORT             = 0x08U,    /*!< Peripheral with abort request ongoing                 */
  HAL_ESMC_STATE_ERROR             = 0x04U     /*!< Peripheral in error                                   */
} HAL_ESMC_StateTypeDef;

/**
  * @brief  ESMC Handle Structure definition
  */
#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
typedef struct __ESMC_HandleTypeDef
#else
typedef struct
#endif
{
  ESMC_TypeDef               *Instance;        /* ESMC registers base address        */
  ESMC_InitTypeDef           Init;             /* ESMC communication parameters      */
  uint8_t                    *pTxBuffPtr;      /* Pointer to ESMC Tx transfer Buffer */
  __IO uint32_t              TxXferSize;       /* ESMC Tx Transfer size              */
  __IO uint32_t              TxXferCount;      /* ESMC Tx Transfer Counter           */
  uint8_t                    *pRxBuffPtr;      /* Pointer to ESMC Rx transfer Buffer */
  __IO uint32_t              RxXferSize;       /* ESMC Rx Transfer size              */
  __IO uint32_t              RxXferCount;      /* ESMC Rx Transfer Counter           */
  DMA_HandleTypeDef          *hdmarx;          /* ESMC Rx DMA Handle parameters      */
  DMA_HandleTypeDef          *hdmatx;          /* ESMC Tx DMA Handle parameters      */
  __IO HAL_LockTypeDef       Lock;             /* Locking object                     */
  __IO HAL_ESMC_StateTypeDef State;            /* ESMC communication state           */
  __IO uint32_t              ErrorCode;        /* ESMC Error code                    */
  uint32_t                   Timeout;          /* Timeout for the ESMC memory access */
  uint32_t                   Contex;           /* ESMC transfer context */
  uint32_t                   ContexAddr24;     /* Temporarily store the value of register ADDR24 */
#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
  void (* ErrorCallback)        (struct __ESMC_HandleTypeDef *hesmc);
  void (* AbortCpltCallback)    (struct __ESMC_HandleTypeDef *hesmc);
  void (* CmdCpltCallback)      (struct __ESMC_HandleTypeDef *hesmc);
  void (* RxCpltCallback)       (struct __ESMC_HandleTypeDef *hesmc);
  void (* TxCpltCallback)       (struct __ESMC_HandleTypeDef *hesmc);
  void (* RxHalfCpltCallback)   (struct __ESMC_HandleTypeDef *hesmc);
  void (* TxHalfCpltCallback)   (struct __ESMC_HandleTypeDef *hesmc);
  void (* MspInitCallback)      (struct __ESMC_HandleTypeDef *hesmc);
  void (* MspDeInitCallback)    (struct __ESMC_HandleTypeDef *hesmc);
#endif
} ESMC_HandleTypeDef;

/**
  * @brief  ESMC Command structure definition
  */
typedef struct
{
  uint32_t TransferFormat;     /* Specifies the Transfer Format
                                  This parameter can be a value of @ref ESMC_TransferFormat */
  uint32_t Instruction;        /* Specifies the Instruction to be sent
                                  This parameter can be a value (8-bit) between 0x00 and 0xFF */
  uint32_t InstructionMode;    /* Specifies the Instruction Mode
                                  This parameter can be a value of @ref ESMC_InstructionMode */
  uint32_t Address;            /* Specifies the Address to be sent (Size from 1 to 4 bytes according AddressSize)
                                  This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
  uint32_t AddressMode;        /* Specifies the Address Mode
                                  This parameter can be a value of @ref ESMC_AddressMode */
  uint32_t AddressSize;        /* Specifies the Address Size
                                  This parameter can be a value of @ref ESMC_AddressSize */
  uint32_t AlternateByte;      /* Specifies the Alternate Byte to be sent
                                  This parameter can be a value (8-bits) between 0x0 and 0xFF */
  uint32_t AlternateByteMode;  /* Specifies the Alternate Bytes Mode
                                  This parameter can be a value of @ref ESMC_AlternateBytesMode */
  uint32_t DataMode;           /* Specifies the Data Mode (used for dummy cycles and data phases)
                                  This parameter can be a value of @ref ESMC_DataMode */
  uint32_t NbData;             /* Specifies the number of data to transfer.
                                  This parameter can be any value between 1 and 0xFFFFFFFF */
  uint32_t DdrMode;            /* Specifies the double data rate mode for address, and data phase
                                  This parameter can be a value of @ref ESMC_DdrMode */
  uint32_t DummyCycles;        /* Specifies the Number of Dummy Cycles.
                                  This parameter can be a number between 0 and 31 */
  uint32_t CSPinSel;           /* Specifies the activated SSxO pin
                                  This parameter can be a value of @ref ESMC_CS_PIN_SEL */
} ESMC_CommandTypeDef;


#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL ESMC Callback ID enumeration definition
  */
typedef enum
{
  HAL_ESMC_ERROR_CB_ID          = 0x00U,  /*!< ESMC Error Callback ID            */
  HAL_ESMC_ABORT_CB_ID          = 0x01U,  /*!< ESMC Abort Callback ID            */
  HAL_ESMC_CMD_CPLT_CB_ID       = 0x02U,  /*!< ESMC Command Complete Callback ID */
  HAL_ESMC_RX_CPLT_CB_ID        = 0x03U,  /*!< ESMC Rx Complete Callback ID      */
  HAL_ESMC_TX_CPLT_CB_ID        = 0x04U,  /*!< ESMC Tx Complete Callback ID      */
  HAL_ESMC_RX_HALF_CPLT_CB_ID   = 0x05U,  /*!< ESMC Rx Half Complete Callback ID */
  HAL_ESMC_TX_HALF_CPLT_CB_ID   = 0x06U,  /*!< ESMC Tx Half Complete Callback ID */
  HAL_ESMC_MSP_INIT_CB_ID       = 0x07U,  /*!< ESMC MspInit Callback ID          */
  HAL_ESMC_MSP_DEINIT_CB_ID     = 0x08U   /*!< ESMC MspDeInit Callback ID        */
} HAL_ESMC_CallbackIDTypeDef;

/**
  * @brief  HAL ESMC Callback pointer definition
  */
typedef void (*pESMC_CallbackTypeDef)(ESMC_HandleTypeDef *hesmc);
#endif
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup ESMC_Exported_Constants ESMC Exported Constants
  * @{
  */

/** @defgroup ESMC_FIFO_SIZE ESMC FIFO SIZE
  * @{
  */
#define HAL_ESMC_TCR_MAX_LENGTH       0xFF
#define HAL_ESMC_FIFO_SIZE            0x80
/**
  * @}
  */
  
/** @defgroup ESMC_ErrorCode ESMC Error Code
  * @{
  */
#define HAL_ESMC_ERROR_NONE            ((uint32_t)0x00000000U) /*!< No error           */
#define HAL_ESMC_ERROR_TIMEOUT         ((uint32_t)0x00000001U) /*!< Timeout error      */
#define HAL_ESMC_ERROR_TRANSFER        ((uint32_t)0x00000002U) /*!< Transfer error     */
#define HAL_ESMC_ERROR_DMA             ((uint32_t)0x00000004U) /*!< DMA transfer error */
#define HAL_ESMC_ERROR_INVALID_PARAM   ((uint32_t)0x00000008U) /*!< Invalid parameters error */
#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
#define HAL_ESMC_ERROR_INVALID_CALLBACK 0x00000010U /*!< Invalid callback error   */
#endif
/**
  * @}
  */

/** @defgroup ESMC_CONTEXT ESMC CONTEXT
  * @{
  */
#define ESMC_CONTEXT_NONE                   ((uint32_t)0x00000000U)  /*!< None                             */
#define ESMC_CONTEXT_READ_FIXED_LENGTH      ((uint32_t)0x00000001U)  /*!< Read single block operation      */
#define ESMC_CONTEXT_READ_VARIABLE_LENGTH   ((uint32_t)0x00000002U)  /*!< Read multiple blocks operation   */
#define ESMC_CONTEXT_WRITE_FIXED_LENGTH     ((uint32_t)0x00000010U)  /*!< Write single block operation     */
#define ESMC_CONTEXT_WRITE_VARIABLE_LENGTH  ((uint32_t)0x00000020U)  /*!< Write multiple blocks operation  */
/**
  * @}
  */

//#define ESMC_FUNCTIONAL_MODE_INDIRECT_WRITE          ((uint32_t)0x0) /*!<Indirect read mode*/
//#define ESMC_FUNCTIONAL_MODE_INDIRECT_READ           ((uint32_t)0x1) /*!<Indirect read mode*/

/** @defgroup ESMC_ClockMode ESMC Clock Mode
  * @{
  */
#define ESMC_CLOCK_MODE_0                   ((uint32_t)0x00000000U)                     /*!<Clk stays low while nCS is released, the first data sampling is the first clock edge*/
#define ESMC_CLOCK_MODE_1                   ((uint32_t)(                ESMC_CR2_CPHA)) /*!<Clk stays low while nCS is released, the first data sampling is the second clock edge*/
#define ESMC_CLOCK_MODE_2                   ((uint32_t)(ESMC_CR2_CPOL                )) /*!<Clk goes high while nCS is released, the first data sampling is the first clock edge*/
#define ESMC_CLOCK_MODE_3                   ((uint32_t)(ESMC_CR2_CPOL | ESMC_CR2_CPHA)) /*!<Clk goes high while nCS is released, the first data sampling is the second clock edge*/
/**
  * @}
  */

/** @defgroup ESMC_TransferFormat ESMC Transfer Format
  * @{
  */
#define ESMC_TRANSFER_FORMAT_SINGLE         ((uint32_t)0x00000000U)                                    /*!<Transfer on a single line*/
#define ESMC_TRANSFER_FORMAT_DUAL           ((uint32_t)(ESMC_SOCR_SPIMODE_0))                          /*!<Transfer on two lines*/
#define ESMC_TRANSFER_FORMAT_QUAD           ((uint32_t)(ESMC_SOCR_SPIMODE_1))                          /*!<Transfer on four lines*/
#define ESMC_TRANSFER_FORMAT_OCTAL          ((uint32_t)(ESMC_SOCR_SPIMODE_0 | ESMC_SOCR_SPIMODE_1))    /*!<Transfer on eight lines*/
/**
  * @}
  */

/** @defgroup ESMC_InstructionMode ESMC Instruction Mode
* @{
*/
#define ESMC_INSTRUCTION_NONE          ((uint32_t)0x00U)    /*!<No instruction*/
#define ESMC_INSTRUCTION_SINGLE_LINE   ((uint32_t)0x01U)    /*!<Instruction on a single line*/
#define ESMC_INSTRUCTION_MULTI_LINES   ((uint32_t)0x02U)    /*!<Instruction on multi lines*/
/**
  * @}
  */

/** @defgroup ESMC_AddressMode ESMC Address Mode
* @{
*/
#define ESMC_ADDRESS_NONE              ((uint32_t)0x00U)    /*!<No address*/
#define ESMC_ADDRESS_SINGLE_LINE       ((uint32_t)0x01U)    /*!<Address on a single line*/
#define ESMC_ADDRESS_MULTI_LINES       ((uint32_t)0x02U)    /*!<Address on multi lines*/
/**
  * @}
  */

/** @defgroup ESMC_AddressSize ESMC Address Size
  * @{
  */
#define ESMC_ADDRESS_8_BITS            ((uint32_t)ESMC_CR3_ADDR8BIT)    /*!<8-bit address*/
#define ESMC_ADDRESS_16_BITS           ((uint32_t)ESMC_CR3_ADDR16BIT)   /*!<16-bit address*/
#define ESMC_ADDRESS_24_BITS           ((uint32_t)0x00)                 /*!<24-bit address*/
#define ESMC_ADDRESS_32_BITS           ((uint32_t)ESMC_CR3_ADDR32BIT)   /*!<32-bit address*/
/**
  * @}
  */

/** @defgroup ESMC_DataMode ESMC data mode
  * @{
  */
#define ESMC_DATA_NONE                 ((uint32_t)0x00U)   /*!<No data*/
#define ESMC_DATA_WRITE                ((uint32_t)0x01U)   /*!<data write mode*/
#define ESMC_DATA_READ                 ((uint32_t)0x02U)   /*!<data read mode*/
/**
  * @}
  */

/** @defgroup ESMC_AlternateBytesMode ESMC Alternate Bytes Mode
* @{
*/
#define ESMC_ALTERNATE_BYTES_DISABLE   0x00000000U                      /*!<No send alternate bytes*/
#define ESMC_ALTERNATE_BYTES_ENABLE    ((uint32_t)ESMC_SOCR_SENM)       /*!<Send alternate bytes right after ADDR*/
/**
  * @}
  */

/** @defgroup ESMC_SSxO ESMC CS Select
  * @{
  */
#define ESMC_SSxO_SS0            0x00000001U                      /*!<CS0 select*/
#define ESMC_SSxO_SS1            0x00000002U                      /*!<CS1 select*/
#define ESMC_SSxO_SS2            0x00000004U                      /*!<CS2 select*/
#define ESMC_SSxO_SS3            0x00000008U                      /*!<CS3 select*/
/**
  * @}
  */

/** @defgroup ESMC_DualFlash_Mode  ESMC Dual Flash Mode
* @{
*/
#define ESMC_DUALFLASH_ENABLE         ((uint32_t)ESMC_CR_2QSPI)
#define ESMC_DUALFLASH_DISABLE        ((uint32_t)0x00000000U)
/**
  * @}
  */

/** @defgroup ESMC_CS_PIN_SEL  ESMC CS pin select
* @{
*/
#define ESMC_SELECT_PIN_CS0         ((uint32_t)0x00000001U)      /*!<CS0 select*/
#define ESMC_SELECT_PIN_CS1         ((uint32_t)0x00000002U)      /*!<CS1 select*/
#define ESMC_SELECT_PIN_CS2         ((uint32_t)0x00000004U)      /*!<CS2 select*/
#define ESMC_SELECT_PIN_CS3         ((uint32_t)0x00000008U)      /*!<CS3 select*/
/**
  * @}
  */

/** @defgroup ESMC_DdrMode ESMC Ddr Mode
  * @{
  */
#define ESMC_DDR_DISABLE              ((uint32_t)0x00000000U)       /*!<Double data rate mode disabled*/
#define ESMC_DDR_DATA                 (ESMC_SOCR_DDRDATA)           /*!<Double data rate mode enabled*/
#define ESMC_DDR_ADDR_DATA            (ESMC_SOCR_DDRADDR | ESMC_SOCR_DDRDATA)  /*!<Double data rate mode enabled*/
#define ESMC_DDR_CMD_ADDR_DATA        (ESMC_SOCR_DDRCMD  | ESMC_SOCR_DDRADDR | ESMC_SOCR_DDRDATA)  /*!<Double data rate mode enabled*/
/**
  * @}
  */

/** @defgroup ESMC_Flags ESMC Flags
  * @{
  */
#define ESMC_FLAG_CMDIF                (ESMC_IFR_CMDIF << 8)        /*!<CMD flag: COMMAND Done*/
#define ESMC_FLAG_FIFOEIF              (ESMC_IFR_FIFOEIF << 8)      /*!<FIFO Empty flag: FIFO are completely empty*/
#define ESMC_FLAG_FIFOHIF              (ESMC_IFR_FIFOHIF << 8)      /*!<FIFO HALF Full flag: FIFO memory is half filled*/
#define ESMC_FLAG_FIFOFIF              (ESMC_IFR_FIFOFIF << 8)      /*!<FIFO Full flag: There is no free space in the FIFO Memory*/
#define ESMC_FLAG_IDLEIF               (ESMC_IFR_IDLEIF << 8)       /*!<IDLE flag: ESMC completed operation and went to IDLE state*/
#define ESMC_FLAG_DATAWAITIF           (ESMC_IFR_DATAWAITIF << 8)   /*!<DATA WAIT flag: In write operations FIFO is empty, or In read operations FIFO is full*/
#define ESMC_FLAG_FIFOOIF              (ESMC_IFR_FIFOOIF << 8)      /*!<FIFO Overflow flag: A data write occurred while Buffer was full, too many bytes loaded to the buffer*/
#define ESMC_FLAG_ADDRDONEIF           (ESMC_IFR_ADDRDONEIF << 8)   /*!<ADDRESS Done flag: the Address field has just been sent out*/
#define ESMC_FLAG_SSACTIVE             (ESMC_SR_SSACTIVE)           /*!<SS outputs Active flag: any of the SS outputs is active (low)*/
#define ESMC_FLAG_RXBUSY               (ESMC_SR_RXBUSY)             /*!<RX In progress flag: ESMC is busy with the data reception from Flash*/
#define ESMC_FLAG_TXBUSY               (ESMC_SR_TXBUSY)             /*!<TX In progress flag: ESMC is busy with memory write operations*/
#define ESMC_FLAG_IDLE                 (ESMC_SR_IDLE)               /*!<ESMC is in IDLE state*/
#define ESMC_FLAG_FIFIOOVERFOLOW       (ESMC_SR_FIFIOOVERFOLOW)     /*!<FIFO has operflown flag*/
#define ESMC_FLAG_IT                   (ESMC_SR_SPIF)               /*!<Interrupt request flag: There is at least one interrupt request pending*/
/**
  * @}
  */

/** @defgroup ESMC_Interrupts EMSC Interrupts
  * @{
  */
#define ESMC_IT_CMD                    ESMC_IER_CMDIE       /*!<Interrupt on the command done*/
#define ESMC_IT_FIFOE                  ESMC_IER_FIFOEIE     /*!<Interrupt on the fifo empty*/
#define ESMC_IT_FIFOH                  ESMC_IER_FIFOHIE     /*!<Interrupt on the fifo half full flag*/
#define ESMC_IT_FIFOF                  ESMC_IER_FIFOFIE     /*!<Interrupt on the fifo full flag*/
#define ESMC_IT_IDLE                   ESMC_IER_IDLEIE      /*!<Interrupt on the idle flag*/
#define ESMC_IT_DATAWAIT               ESMC_IER_DATAWAITIE  /*!<Interrupt on the data wait flag*/
#define ESMC_IT_FIFOO                  ESMC_IER_FIFOOIE     /*!<Interrupt on the fifo overflow flag*/
#define ESMC_IT_ADDRDONE               ESMC_IER_ADDRDONEIE  /*!<Interrupt on the address done flag*/

#define ESMC_IT_ALL                    (ESMC_IT_CMD  | ESMC_IT_FIFOE    | ESMC_IT_FIFOH | ESMC_IT_FIFOF | \
                                        ESMC_IT_IDLE | ESMC_IT_DATAWAIT | ESMC_IT_FIFOO | ESMC_IT_ADDRDONE)
/**
  * @}
  */

/** @defgroup ESMC_Timeout_definition ESMC Timeout definition
  * @{
  */
#define HAL_QPSI_TIMEOUT_DEFAULT_VALUE ((uint32_t)5000)/* 5 s */
/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup ESMC_Exported_Macros ESMC Exported Macros
  * @{
  */

/** @brief Reset ESMC handle state
  * @param  __HANDLE__ ESMC handle.
  * @retval None
  */
#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
#define __HAL_ESMC_RESET_HANDLE_STATE(__HANDLE__)           do {                                              \
                                                                  (__HANDLE__)->State = HAL_ESMC_STATE_RESET; \
                                                                  (__HANDLE__)->MspInitCallback = NULL;       \
                                                                  (__HANDLE__)->MspDeInitCallback = NULL;     \
                                                               } while(0)
#else
#define __HAL_ESMC_RESET_HANDLE_STATE(__HANDLE__)           ((__HANDLE__)->State = HAL_ESMC_STATE_RESET)
#endif

/** @brief  Enable ESMC
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @retval None
  */
#define __HAL_ESMC_ENABLE(__HANDLE__)                       SET_BIT((__HANDLE__)->Instance->CR, ESMC_CR_SPIEN)

/** @brief  Disable ESMC
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @retval None
  */
#define __HAL_ESMC_DISABLE(__HANDLE__)                      CLEAR_BIT((__HANDLE__)->Instance->CR, ESMC_CR_SPIEN)

/** @brief  Enable ESMC Global interrupt
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @retval None
  */
#define __HAL_ESMC_ENABLE_GLOBAL_IT(__HANDLE__)             SET_BIT((__HANDLE__)->Instance->CR, ESMC_CR_GIE)

/** @brief  Disable ESMC Global interrupt
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @retval None
  */
#define __HAL_ESMC_DISABLE_GLOBAL_IT(__HANDLE__)            CLEAR_BIT((__HANDLE__)->Instance->CR, ESMC_CR_GIE)

/** @brief  Soft reset ESMC
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @retval None
  */
#define __HAL_ESMC_SOFT_RESET(__HANDLE__)                   SET_BIT((__HANDLE__)->Instance->CR, ESMC_CR_SOFTRST)

/** @brief  Enables the specified ESMC interrupt.
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @param  __INTERRUPT__ specifies the ESMC interrupt source to enable.
  *          This parameter can be one of the following values:
  *            @arg ESMC_IT_CMD     : ESMC Command Done interrupt
  *            @arg ESMC_IT_FIFOE   : ESMC FIFO empty interrupt
  *            @arg ESMC_IT_FIFOH   : ESMC FIFO half full interrupt
  *            @arg ESMC_IT_FIFOF   : ESMC FIFO full interrupt
  *            @arg ESMC_IT_IDLE    : ESMC Idle interrupt
  *            @arg ESMC_IT_DATAWAIT: ESMC Datawait interrupt
  *            @arg ESMC_IT_FIFOO   : ESMC FIFO OverFlow interrupt
  *            @arg ESMC_IT_ADDRDONE: ESMC Address Done interrupt
  * @retval None
  */
#define __HAL_ESMC_ENABLE_IT(__HANDLE__, __INTERRUPT__)     SET_BIT((__HANDLE__)->Instance->IER, (__INTERRUPT__))

/** @brief  Disables the specified ESMC interrupt.
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @param  __INTERRUPT__ specifies the ESMC interrupt source to disable.
  *          This parameter can be one of the following values:
  *            @arg ESMC_IT_CMD     : ESMC Command Done interrupt
  *            @arg ESMC_IT_FIFOE   : ESMC FIFO empty interrupt
  *            @arg ESMC_IT_FIFOH   : ESMC FIFO half full interrupt
  *            @arg ESMC_IT_FIFOF   : ESMC FIFO full interrupt
  *            @arg ESMC_IT_IDLE    : ESMC Idle interrupt
  *            @arg ESMC_IT_DATAWAIT: ESMC Datawait interrupt
  *            @arg ESMC_IT_FIFOO   : ESMC FIFO OverFlow interrupt
  *            @arg ESMC_IT_ADDRDONE: ESMC Address Done interrupt
  * @retval None
  */
#define __HAL_ESMC_DISABLE_IT(__HANDLE__, __INTERRUPT__)    CLEAR_BIT((__HANDLE__)->Instance->IER, (__INTERRUPT__))

/** @brief  Checks whether the specified ESMC interrupt source is enabled.
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @param  __INTERRUPT__ specifies the ESMC interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg ESMC_IT_CMD     : ESMC Command Done interrupt
  *            @arg ESMC_IT_FIFOE   : ESMC FIFO empty interrupt
  *            @arg ESMC_IT_FIFOH   : ESMC FIFO half full interrupt
  *            @arg ESMC_IT_FIFOF   : ESMC FIFO full interrupt
  *            @arg ESMC_IT_IDLE    : ESMC Idle interrupt
  *            @arg ESMC_IT_DATAWAIT: ESMC Datawait interrupt
  *            @arg ESMC_IT_FIFOO   : ESMC FIFO OverFlow interrupt
  *            @arg ESMC_IT_ADDRDONE: ESMC Address Done interrupt
  * @retval The new state of __INTERRUPT__ (TRUE or FALSE).
  */
#define __HAL_ESMC_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) (READ_BIT((__HANDLE__)->Instance->IER, (__INTERRUPT__)) == (__INTERRUPT__))

/**
  * @brief  Get the selected ESMC's flag status.
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @param  __FLAG__ specifies the ESMC flag to check.
  *          This parameter can be one of the following values:
  *            @arg ESMC_FLAG_CMDIF         : COMMAND Done flag
  *            @arg ESMC_FLAG_FIFOEIF       : FIFO Empty flag
  *            @arg ESMC_FLAG_FIFOHIF       : FIFO HALF Full flag
  *            @arg ESMC_FLAG_FIFOFIF       : FIFO Full flag
  *            @arg ESMC_FLAG_IDLEIF        : IDLE flag
  *            @arg ESMC_FLAG_DATAWAITIF    : DATA WAIT flag
  *            @arg ESMC_FLAG_FIFOOIF       : FIFO Overflow flag
  *            @arg ESMC_FLAG_ADDRDONEIF    : ADDRESS Done flag
  *            @arg ESMC_FLAG_SSACTIVE      : SS outputs Active flag
  *            @arg ESMC_FLAG_RXBUSY        : RX In progress flag
  *            @arg ESMC_FLAG_TXBUSY        : TX In progress flag
  *            @arg ESMC_FLAG_IDLE          : ESMC is in IDLE state
  *            @arg ESMC_FLAG_FIFIOOVERFOLOW: FIFO has operflown flag
  *            @arg ESMC_FLAG_IT            : Interrupt request flag:
  * @retval None
  */
#define __HAL_ESMC_GET_FLAG(__HANDLE__, __FLAG__)           (((*(__IO uint16_t *)(&(__HANDLE__)->Instance->SR) & (__FLAG__)) != 0) ? SET : RESET)

/** @brief  Clears the specified ESMC's flag status.
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @param  __FLAG__ specifies the ESMC clear register flag that needs to be set
  *          This parameter can be one of the following values:
  *            @arg ESMC_FLAG_CMDIF         : COMMAND Done flag
  *            @arg ESMC_FLAG_IDLEIF        : IDLE flag
  *            @arg ESMC_FLAG_ADDRDONEIF    : ADDRESS Done flag
  * @retval None
  */
#define __HAL_ESMC_CLEAR_FLAG(__HANDLE__, __FLAG__)         (*(__IO uint16_t *)(&(__HANDLE__)->Instance->SR) = (__FLAG__))

/** @brief  configure which slave select output should be driven during no xip mode.
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @param  __SSX__ specifies the CS pin
  *          The parameter can take the following values:
  *            @arg ESMC_SELECT_PIN_CS0
  *            @arg ESMC_SELECT_PIN_CS1
  *            @arg ESMC_SELECT_PIN_CS2
  *            @arg ESMC_SELECT_PIN_CS3
  * @retval None
  */
#define __HAL_ESMC_SET_SSXO(__HANDLE__, __SSX__)             SET_BIT((__HANDLE__)->Instance->ADDR32, (__SSX__)<<ESMC_ADDR32_SS_Pos)

/** @brief  configure which slave select output should be driven during xip mode.
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @param  __SSX__ specifies the CS pin
  *          The parameter can take the following values:
  *            @arg ESMC_SELECT_PIN_CS0
  *            @arg ESMC_SELECT_PIN_CS1
  *            @arg ESMC_SELECT_PIN_CS2
  *            @arg ESMC_SELECT_PIN_CS3
  * @retval None
  */
#define __HAL_ESMC_SET_XIP_SSXO(__HANDLE__, __SSX__)         SET_BIT((__HANDLE__)->Instance->ADDR32, (__SSX__)<<ESMC_ADDR32_XSS_Pos)

/** @brief  Clears the FIFO.
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @retval None
  */
#define __HAL_ESMC_CLEAR_FIFO(__HANDLE__)                    SET_BIT((__HANDLE__)->Instance->CR3, ESMC_CR3_FIFOCLR)

/** @brief  Enable the slave select.
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @retval None
  */
#define __HAL_ESMC_ENABLE_SLAVE(__HANDLE__)                  SET_BIT((__HANDLE__)->Instance->CR3, ESMC_CR3_SSSETRQ)

/** @brief  Disable the slave select.
  * @param  __HANDLE__ specifies the ESMC Handle.
  * @retval None
  */
#define __HAL_ESMC_DISABLE_SLAVE(__HANDLE__)                 SET_BIT((__HANDLE__)->Instance->CR3, ESMC_CR3_SSCLRRQ)

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup ESMC_Exported_Functions
  * @{
  */

/** @addtogroup ESMC_Exported_Functions_Group1
  * @{
  */
/* Initialization/de-initialization functions  ********************************/
HAL_StatusTypeDef     HAL_ESMC_Init     (ESMC_HandleTypeDef *hesmc);
HAL_StatusTypeDef     HAL_ESMC_DeInit   (ESMC_HandleTypeDef *hesmc);
void                  HAL_ESMC_MspInit  (ESMC_HandleTypeDef *hesmc);
void                  HAL_ESMC_MspDeInit(ESMC_HandleTypeDef *hesmc);
/**
  * @}
  */

/** @addtogroup ESMC_Exported_Functions_Group2
  * @{
  */
/* IO operation functions *****************************************************/
/* ESMC IRQ handler method */
void                  HAL_ESMC_IRQHandler(ESMC_HandleTypeDef *hesmc);

/* ESMC indirect mode */
HAL_StatusTypeDef     HAL_ESMC_Command      (ESMC_HandleTypeDef *hesmc, ESMC_CommandTypeDef *cmd, uint32_t Timeout);
HAL_StatusTypeDef     HAL_ESMC_Transmit     (ESMC_HandleTypeDef *hesmc, uint8_t *pData, uint32_t Timeout);
HAL_StatusTypeDef     HAL_ESMC_Receive      (ESMC_HandleTypeDef *hesmc, uint8_t *pData, uint32_t Timeout);
HAL_StatusTypeDef     HAL_ESMC_Command_IT   (ESMC_HandleTypeDef *hesmc, ESMC_CommandTypeDef *cmd);
HAL_StatusTypeDef     HAL_ESMC_Transmit_IT  (ESMC_HandleTypeDef *hesmc, uint8_t *pData);
HAL_StatusTypeDef     HAL_ESMC_Receive_IT   (ESMC_HandleTypeDef *hesmc, uint8_t *pData);
HAL_StatusTypeDef     HAL_ESMC_Transmit_DMA (ESMC_HandleTypeDef *hesmc, uint8_t *pData);
HAL_StatusTypeDef     HAL_ESMC_Receive_DMA  (ESMC_HandleTypeDef *hesmc, uint8_t *pData);

/* QSPI memory-mapped mode */
HAL_StatusTypeDef HAL_ESMC_MemoryMapped(ESMC_HandleTypeDef *hesmc, ESMC_CommandTypeDef *cmd);


/* Callback functions in non-blocking modes ***********************************/
void                  HAL_ESMC_ErrorCallback        (ESMC_HandleTypeDef *hesmc);
void                  HAL_ESMC_AbortCpltCallback    (ESMC_HandleTypeDef *hesmc);
void                  HAL_ESMC_SetSSxO(ESMC_HandleTypeDef *hesmc, uint32_t SSxO);

/* ESMC indirect mode */
void                  HAL_ESMC_CmdCpltCallback      (ESMC_HandleTypeDef *hesmc);
void                  HAL_ESMC_RxCpltCallback       (ESMC_HandleTypeDef *hesmc);
void                  HAL_ESMC_TxCpltCallback       (ESMC_HandleTypeDef *hesmc);
void                  HAL_ESMC_RxHalfCpltCallback   (ESMC_HandleTypeDef *hesmc);
void                  HAL_ESMC_TxHalfCpltCallback   (ESMC_HandleTypeDef *hesmc);

#if (USE_HAL_ESMC_REGISTER_CALLBACKS == 1)
/* ESMC callback registering/unregistering */
HAL_StatusTypeDef     HAL_ESMC_RegisterCallback     (ESMC_HandleTypeDef *hesmc, HAL_ESMC_CallbackIDTypeDef CallbackId, pESMC_CallbackTypeDef pCallback);
HAL_StatusTypeDef     HAL_ESMC_UnRegisterCallback   (ESMC_HandleTypeDef *hesmc, HAL_ESMC_CallbackIDTypeDef CallbackId);
#endif
/**
  * @}
  */

/** @addtogroup ESMC_Exported_Functions_Group3
  * @{
  */
/* Peripheral Control and State functions  ************************************/
HAL_ESMC_StateTypeDef HAL_ESMC_GetState        (ESMC_HandleTypeDef *hesmc);
uint32_t              HAL_ESMC_GetError        (ESMC_HandleTypeDef *hesmc);
HAL_StatusTypeDef     HAL_ESMC_Abort           (ESMC_HandleTypeDef *hesmc);
HAL_StatusTypeDef     HAL_ESMC_Abort_IT        (ESMC_HandleTypeDef *hesmc);
void                  HAL_ESMC_SetTimeout      (ESMC_HandleTypeDef *hesmc, uint32_t Timeout);
/**
  * @}
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup ESMC_Private_Macros ESMC Private Macros
  * @{
  */

#define IS_ESMC_CLOCK_PRESCALER(PRESCALER)  ((PRESCALER) <= 0xFF && (PRESCALER) >= 0x2)


#define IS_ESMC_CLOCK_MODE(CLKMODE)         (((CLKMODE) == ESMC_CLOCK_MODE_0) || \
                                             ((CLKMODE) == ESMC_CLOCK_MODE_1) || \
                                             ((CLKMODE) == ESMC_CLOCK_MODE_2) || \
                                             ((CLKMODE) == ESMC_CLOCK_MODE_3))


#define IS_ESMC_DUAL_FLASH_MODE(MODE)    (((MODE) == ESMC_DUALFLASH_ENABLE) || \
                                          ((MODE) == ESMC_DUALFLASH_DISABLE))

#define IS_ESMC_INSTRUCTION(INSTRUCTION)    ((INSTRUCTION) <= 0xFF)

#define IS_ESMC_ADDRESS_SIZE(ADDR_SIZE)     (((ADDR_SIZE) == ESMC_ADDRESS_8_BITS)  || \
                                             ((ADDR_SIZE) == ESMC_ADDRESS_16_BITS) || \
                                             ((ADDR_SIZE) == ESMC_ADDRESS_24_BITS) || \
                                             ((ADDR_SIZE) == ESMC_ADDRESS_32_BITS))

#define IS_ESMC_DUMMY_CYCLES(DCY)           ((DCY) <= 31)

#define IS_ESMC_INSTRUCTION_MODE(MODE)      (((MODE) == ESMC_INSTRUCTION_NONE)         || \
                                             ((MODE) == ESMC_INSTRUCTION_SINGLE_LINE)  || \
                                             ((MODE) == ESMC_INSTRUCTION_MULTI_LINES))

#define IS_ESMC_ADDRESS_MODE(MODE)          (((MODE) == ESMC_ADDRESS_NONE)         || \
                                             ((MODE) == ESMC_ADDRESS_SINGLE_LINE)  || \
                                             ((MODE) == ESMC_ADDRESS_MULTI_LINES))

#define IS_ESMC_ALTERNATE_BYTES_MODE(MODE)  (((MODE) == ESMC_ALTERNATE_BYTES_DISABLE) || \
                                             ((MODE) == ESMC_ALTERNATE_BYTES_ENABLE))

#define IS_ESMC_ALTERNATE_BYTE(ALT)         ((ALT) <= 0xFF)

#define IS_ESMC_DATA_MODE(MODE)             (((MODE) == ESMC_DATA_NONE)    || \
                                             ((MODE) == ESMC_DATA_WRITE)   || \
                                             ((MODE) == ESMC_DATA_READ))

#define IS_ESMC_DDR_MODE(DDR_MODE)          (((DDR_MODE) == ESMC_DDR_DISABLE)    || \
                                             ((DDR_MODE) == ESMC_DDR_DATA)       || \
                                             ((DDR_MODE) == ESMC_DDR_ADDR_DATA)  || \
                                             ((DDR_MODE) == ESMC_DDR_CMD_ADDR_DATA))

/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/
/** @defgroup ESMC_Private_Functions ESMC Private Functions
  * @{
  */

/**
  * @}
  */

/**
  * @}
  */
  
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __PY32F4xx_HAL_ESMC_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
