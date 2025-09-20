/**
  ******************************************************************************
  * @file    py32f4xx_hal_canfd.h
  * @author  MCU Application Team
  * @brief   Header file of CNAFD HAL module.
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
#ifndef __PY32F4xx_HAL_CANFD_H
#define __PY32F4xx_HAL_CANFD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f4xx_hal_def.h"

/** @addtogroup PY32F4xx_HAL_Driver
  * @{
  */

/** @addtogroup CANFD
  * @{
  */
#if defined ( __CC_ARM   )
#pragma anon_unions
#endif

/* Exported types ------------------------------------------------------------*/
/** @defgroup CANFD_Exported_Types CANFD Exported Types
  * @{
  */

/**
  * @brief HAL State structures definition
  */
typedef enum
{
  HAL_CANFD_STATE_RESET             = 0x00U,  /*!< CANFD not yet initialized or disabled */
  HAL_CANFD_STATE_READY             = 0x01U,  /*!< CANFD initialized and ready for use   */
  HAL_CANFD_STATE_BUSY              = 0x02U,  /*!< CANFD process is ongoing              */
  HAL_CANFD_STATE_LISTENING         = 0x03U,  /*!< CANFD receive process is ongoing      */
  HAL_CANFD_STATE_SLEEP_PENDING     = 0x04U,  /*!< CANFD sleep request is pending        */
  HAL_CANFD_STATE_SLEEP_ACTIVE      = 0x05U,  /*!< CANFD sleep mode is active            */
  HAL_CANFD_STATE_ERROR             = 0x06U   /*!< CANFD error state                     */
} HAL_CANFD_StateTypeDef;

/**
  * @brief CANFD LLC frame format Structure definition
  */
typedef struct
{
  uint32_t DLC:       11;
  uint32_t Reserved1: 5;
  uint32_t IDE:       1;
  uint32_t FDF:       1;
  uint32_t BRS:       1;
  uint32_t Reserved2: 1;
  uint32_t RMF:       1;
  uint32_t Reserved3: 3;
  uint32_t KOER:      3;
  uint32_t ESI:       1;
  uint32_t LBF:       1;
  uint32_t Reserved4: 3;
} CANFD_FormatfieldTypeDef;

/**
  * @brief CANFD Init Structure definition
  */
typedef struct
{
  uint32_t          FrameFormat;                  /*!< Specifies the CANFD frame format.
                                                       This parameter can be a value of @ref CANFD_frame_format     */
    
  uint32_t          Mode;                         /*!< Specifies the CAN operating mode.
                                                         This parameter can be a value of @ref CANFD_operating_mode */

  uint32_t          CANFDProtocol;                /*!< CAN-FD protocol, Bosch CAN-FD or 11898-1:2015 CAN-FD.
                                                         This parameter can be a value of @ref CANFD_FD_protocol */

  uint32_t          Prescaler;                    /*!< Specifies the value by which the oscillator frequency is
                                                         divided for generating the bit time quanta.
                                                         This parameter must be a number between Min_Data = 1 and Max_Data = 32. */

  uint32_t          NominalSyncJumpWidth;         /*!< Specifies the maximum number of time quanta the CANFD
                                                         hardware is allowed to lengthen or shorten a bit to perform
                                                         resynchronization.This parameter must be a number between 1 and 128 */

  uint32_t          NominalTimeSeg1;              /*!< Specifies the number of time quanta in Bit Segment 1.
                                                         This parameter must be a number between 2 and 513 */

  uint32_t          NominalTimeSeg2;              /*!< Specifies the number of time quanta in Bit Segment 2.
                                                         This parameter must be a number between 1 and 128 */

  uint32_t          DataSyncJumpWidth;            /*!< Specifies the maximum number of time quanta the CANFD
                                                         hardware is allowed to lengthen or shorten a data bit to
                                                         perform resynchronization.This parameter must be a number between 1 and 128 */

  uint32_t          DataTimeSeg1;                 /*!< Specifies the number of time quanta in Data Bit Segment 1.
                                                         This parameter must be a number between 2 and 257 */

  uint32_t          DataTimeSeg2;                 /*!< Specifies the number of time quanta in Data Bit Segment 2.
                                                         This parameter must be a number between 1 and 128 */

  uint32_t          SecondSamplePointOffset;      /*!< Specify secondary sample point(SSP) of transmitter delay compensatin(TDC). Number of TQ.
                                                         This parameter must be a number between 0 and 255 */
} CANFD_InitTypeDef;


/**
  * @brief  CANFD Tx header structure definition
  */
typedef struct
{
  uint32_t Identifier;          /*!< Specifies the identifier.
                                     This parameter must be a number between:
                                      - 0 and 0x7FF, if IdType is CANFD_STANDARD_ID
                                      - 0 and 0x1FFFFFFF, if IdType is CANFD_EXTENDED_ID               */

  uint32_t IdType;              /*!< Specifies the identifier type for the message that will be
                                     transmitted.
                                     This parameter can be a value of @ref CANFD_id_type               */

  uint32_t TxFrameType;         /*!< Specifies the frame type of the message that will be transmitted.
                                     This parameter can be a value of @ref CANFD_frame_type            */

  
  uint32_t FrameFormat;         /*!< Specifies whether the Rx frame is received in classic or FD
                                       format.
                                       This parameter can be a value of @ref CANFD_frame_format                */

  uint32_t Handle;              /*!< Specifies handle for frame identifiation.
                                       This parameter must be a number between 0 and 0xFF              */

  uint32_t DataLength;          /*!< Specifies the length of the frame that will be transmitted.
                                      This parameter can be a value of @ref CANFD_data_length_code     */
} CANFD_TxHeaderTypeDef;


/**
  * @brief  CANFD Rx header structure definition
  */
typedef struct
{
  uint32_t Identifier;            /*!< Specifies the identifier.
                                       This parameter must be a number between:
                                        - 0 and 0x7FF, if IdType is CANFD_STANDARD_ID
                                        - 0 and 0x1FFFFFFF, if IdType is CANFD_EXTENDED_ID               */

  uint32_t IdType;                /*!< Specifies the identifier type of the received message.
                                       This parameter can be a value of @ref CANFD_id_type               */

  uint32_t RxFrameType;           /*!< Specifies the the received message frame type.
                                       This parameter can be a value of @ref CANFD_frame_type            */

  uint32_t DataLength;            /*!< Specifies the received frame length.
                                        This parameter can be a value of @ref CANFD_data_length_code     */

  uint32_t ErrorStateIndicator;   /*!< Specifies the error state indicator.
                                       This parameter can be a value of @ref CANFD_error_state_indicator */

  uint32_t ProtocolErrorType;     /*!< Specifies CAN frame error type. 
                                       This parameter can be a value of @ref CANFD_protocol_error_type      */

  uint32_t LoopBackIndicator;     /*!< Specifies CANFD loop back frame
                                       This parameter can be a value of @ref CANFD_loopback_frame         */

  uint32_t FrameFormat;           /*!< Specifies whether the Rx frame is received in classic or FD format.
                                       This parameter can be a value of @ref CANFD_frame_format                */

  uint32_t RxTimestamp;           /*!< Specifies the timestamp counter value captured on start of frame reception.
                                       This parameter must be a number between 0 and 0xFFFF              */

} CANFD_RxHeaderTypeDef;

/**
  * @brief CANFD filter Structure definition
  */
typedef struct
{
  uint32_t IdType;                            /*!< Specifies the identifier type.
                                                   This parameter can be a value of @ref CANFD_id_type       */
                                              
  uint32_t FilterChannel;                     /*!< Specify acceptance filter channel.
                                                   This parameter can be values of @ref CANFD_filter_channel */
                                              
  uint32_t Rank;                              /*!< Add or remove the channel from filter group.
                                                   This parameter can be a value of @ref CANFD_filter_rank */
                                              
  uint32_t FilterID;                          /*!< Specifies the filter identification.
                                                     This parameter must be a number between:
                                                      - 0 and 0x7FF, if IdType is CANFD_STANDARD_ID
                                                      - 0 and 0x1FFFFFFF, if IdType is CANFD_EXTENDED_ID       */
  union{                                      
    uint32_t FilterFormat;                    /*!< Specifies the filter format.
                                                   This parameter can be a value of @ref CANFD_LLC_FORMAT_BITS */
    CANFD_FormatfieldTypeDef FilterFormat_f;
  };
  
  uint32_t MaskID;                            /*!< Specifies the filter mask identification.
                                                     This parameter must be a number between:
                                                      - 0 and 0x7FF, if IdType is CANFD_STANDARD_ID
                                                      - 0 and 0x1FFFFFFF, if IdType is CANFD_EXTENDED_ID       */
                                              
  union{                                      
    uint32_t MaskFormat;                      /*!< Specifies the mask format. 
                                                   This parameter can be a value of @ref CANFD_LLC_FORMAT_BITS */
    CANFD_FormatfieldTypeDef MaskFormat_f;
  };

} CANFD_FilterTypeDef;

/**
 * @brief CAN time-triggered communication configuration structure.
 */
typedef struct
{

  uint32_t FifoMode;                 /*!< Specifies the TTCAN Mode.
                                           This parameter can be a value of @ref CANFD_TTCAN_FIFO_mode            */

  uint32_t Prescaler;                /*!< Specifies the Timestamp prescaler.
                                           This parameter can be a value of @ref CANFD_timestamp_prescaler   */

  uint32_t RefMessageIdType;         /*!< Specifies the identifier type of the reference message.
                                           This parameter can be a value of @ref CANFD_id_type               */

  uint32_t RefMessageId;             /*!< Specifies the identifier.
                                         This parameter must be a number between:
                                          - 0 and 0x7FF, if IdType is CANFD_STANDARD_ID
                                          - 0 and 0x1FFFFFFF, if IdType is CANFD_EXTENDED_ID               */

  uint32_t TxEnableWindow;           /*!< Specifies the Transmit Enable Window.
                                           This parameter must be a number between 1 and 15           */

  uint32_t WatchTrigTime;            /*!< Specifies the Application Watchdog Limit.
                                           This parameter must be a number between 0 and 0xFFFF              */

} CANFD_TimeTriggerTypeDef;

/**
  * @brief CANFD Protocol Status structure definition
  */
typedef struct
{
  uint32_t TxErrorCnt;        /*!< Specifies the Transmit Error Counter Value.
                                   This parameter can be a number between 0 and 255                                          */

  uint32_t RxErrorCnt;        /*!< Specifies the Receive Error Counter Value.
                                   This parameter can be a number between 0 and 255                                          */
  
  uint32_t LastErrorCode;     /*!< Specifies the type of the last error that occurred on the CANFD bus.
                                   This parameter can be a value of @ref CANFD_protocol_error_type                           */
  
  uint32_t LastArbLostPos;    /*!< Specifies the position of the last arbitration lost on the CANFD bus. 
                                   This parameter can be a number between 0 and 31                                           */

  uint32_t ErrorPassive;      /*!< Specifies the CANFD module error status.
                                   This parameter can be:
                                    - 0 : The CANFD is in Error_Active state
                                    - 1 : The CANFD is in Error_Passive state                                                */

  uint32_t Warning;           /*!< Specifies the CANFD module warning status.
                                   This parameter can be:
                                    - 0 : error counters (RxErrorCnt and TxErrorCnt) are below the Error_Warning limit
                                    - 1 : at least one of error counters has reached the Error_Warning limit          */

  uint32_t BusOff;            /*!< Specifies the CANFD module Bus_Off status.
                                   This parameter can be:
                                    - 0 : The CANFD is not in Bus_Off state
                                    - 1 : The CANFD is in Bus_Off state                                                      */

} CANFD_ProtocolStatusTypeDef;

/**
  * @brief CANFD transmit status Structure definition
  */
typedef struct
{
  uint32_t LastTxHandle;     /*!< Specifies the handle value of the last Tx frame.
                                  This parameter can be a number between 0 and 255                                           */
  
  uint32_t LastTxStatus;     /*!< Specifies the status of the last Tx frame.
                                  This parameter can be a value of @ref CANFD_transmit_status                                */
  
  uint32_t TxHandle;         /*!< Specifies the handle value of the current Tx frame.
                                  This parameter can be a number between 0 and 255                                           */

  uint32_t TxStatus;         /*!< Specifies the status of the current Tx frame.
                                  This parameter can be a value of @ref CANFD_transmit_status                                */
  
} CANFD_TxStatusTypeDef;


#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
typedef struct __CANFD_HandleTypeDef
#else
typedef struct
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
{
  CANFD_TypeDef                 *Instance;                 /*!< Register base address */

  CANFD_InitTypeDef             Init;                      /*!< CANFD required parameters */

  __IO HAL_CANFD_StateTypeDef   State;                     /*!< CANFD communication state */

  HAL_LockTypeDef               Lock;                      /*!< CANFD locking object      */

  __IO uint32_t                 ErrorCode;                 /*!< CANFD Error code. */

  uint32_t                      LastTimeTrigType;          /*!< CANFD last time trig type */

  uint32_t                      LastAbortTxType;           /*!< CANFD last abort transmit type */

  uint32_t                      LastSTBTxType;             /*!< CANFD last STB transmit type */

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
  void (* RxCpltCallback)(__CANFD_HandleTypeDef *hcan);
  void (* RxFifoAlmostFullCallback)(__CANFD_HandleTypeDef *hcan);
  void (* RxFifoFullCallback)(__CANFD_HandleTypeDef *hcan);
  void (* PtbTxCpltCallback)(__CANFD_HandleTypeDef *hcan);
  void (* StbTxCpltCallback)(__CANFD_HandleTypeDef *hcan, uint32_t LastSTBTxType);
  void (* RxFifoOverflowCallback)(__CANFD_HandleTypeDef *hcan);
  void (* ArbitrationLostCallback)(__CANFD_HandleTypeDef *hcan);
  void (* TxAbortCallback)(__CANFD_HandleTypeDef *hcan, uint32_t LastAbortTxType);
  void (* PassiveErrorCallback)(__CANFD_HandleTypeDef *hcan);
  void (* BusErrorCallback)(__CANFD_HandleTypeDef *hcan);
  void (* ErrorChangeCallback)(__CANFD_HandleTypeDef *hcan);
  void (* TT_RxTimeTrigCallback)(__CANFD_HandleTypeDef *hcan);
  void (* TT_TxSingleTrigCallback)(__CANFD_HandleTypeDef *hcan);
  void (* TT_TxStartTrigCallback)(__CANFD_HandleTypeDef *hcan);
  void (* TT_TxStopTrigCallback)(__CANFD_HandleTypeDef *hcan);
  void (* TT_TimestampWraparoundCallback)(__CANFD_HandleTypeDef *hcan);
  void (* TT_TrigErrorCallback)(__CANFD_HandleTypeDef *hcan);  
  
  void (* MspInitCallback)(struct __CANFD_HandleTypeDef *hfdcan);                     /*!< CANFD Msp Init callback              */
  void (* MspDeInitCallback)(struct __CANFD_HandleTypeDef *hfdcan);                   /*!< CANFD Msp DeInit callback            */
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */

} CANFD_HandleTypeDef;

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
/**
  * @brief  HAL CANFD common Callback ID enumeration definition
  */
typedef enum
{
  HAL_CANFD_RX_COMPLETE_CB_ID             = 0x00U,    /*!< CANFD Rx complete callback ID                */
  HAL_CANFD_RX_FIFO_ALMOST_FULL_CB_ID     = 0x01U,    /*!< CANFD Rx fifo almost full callback ID        */
  HAL_CANFD_RX_FIFO_FULL_CB_ID            = 0x02U,    /*!< CANFD Rx fifo full callback ID               */
  HAL_CANFD_PTB_TX_COMPLETE_CB_ID         = 0x03U,    /*!< CANFD PTB Tx complete callback ID            */
  HAL_CANFD_STB_TX_COMPLETE_CB_ID         = 0x04U,    /*!< CANFD STB Tx complete callback ID            */
  HAL_CANFD_TX_ABORT_CB_ID                = 0x05U,    /*!< CANFD Tx abort callback ID                   */
  HAL_CANFD_RX_FIFO_OVERFLOW_CB_ID        = 0x06U,    /*!< CANFD Rx fifo overflow callback ID           */
  HAL_CANFD_ARBITRATION_LOST_CB_ID        = 0x07U,    /*!< CANFD Arbitration lost callback ID           */

  HAL_CANFD_PASSIVE_ERROR_CB_ID           = 0x08U,    /*!< CANFD Passive error callback ID              */
  HAL_CANFD_BUS_ERROR_CB_ID               = 0x09U,    /*!< CANFD Bus error callback ID                  */
  HAL_CANFD_ERROR_CHANGE_CB_ID            = 0x0AU,    /*!< CANFD Error change callback ID               */

  HAL_CANFD_TT_RX_TIME_TRIG_CB_ID         = 0x0BU,    /*!< CANFD TT Rx time trig callback ID            */
  HAL_CANFD_TT_TX_SINGLE_TRIG_CB_ID       = 0x0CU,    /*!< CANFD TT Tx single trig callback ID          */
  HAL_CANFD_TT_TX_START_TRIG_CB_ID        = 0x0DU,    /*!< CANFD TT Tx start trig callback ID           */
  HAL_CANFD_TT_TX_STOP_TRIG_CB_ID         = 0x0EU,    /*!< CANFD TT Tx stop trig callback ID            */
  HAL_CANFD_TT_TIMESTAMP_WRAPAROUND_CB_ID = 0x0FU,    /*!< CANFD TT timestamp wraparound callback ID    */

  HAL_CANFD_TT_TRIG_ERROR_CB_ID           = 0x10U,    /*!< CANFD TT trig error callback ID              */

  HAL_CANFD_MSPINIT_CB_ID                 = 0x11U,    /*!< CANFD MspInit callback ID                    */
  HAL_CANFD_MSPDEINIT_CB_ID               = 0x12U,    /*!< CANFD MspDeInit callback ID                  */

} HAL_CANFD_CallbackIDTypeDef;

/**
  * @brief  HAL CANFD Callback pointer definition
  */
typedef  void (*pCANFD_CallbackTypeDef)(CANFD_HandleTypeDef *hcanfd);     /*!< pointer to a common CANfd callback function */

#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/

/** @defgroup CANFD_Exported_Constants CANFD Exported Constants
  * @{
  */

/** @defgroup HAL_CANFD_Error_Code HAL CANFD Error Code
  * @{
  */
#define HAL_CANFD_ERROR_NONE            ((uint32_t)0x00000000U) /*!< No error                                                               */
#define HAL_CANFD_ERROR_TIMEOUT         ((uint32_t)0x00000001U) /*!< Timeout error                                                          */
#define HAL_CANFD_ERROR_NOT_INITIALIZED ((uint32_t)0x00000002U) /*!< Peripheral not initialized                                             */
#define HAL_CANFD_ERROR_NOT_READY       ((uint32_t)0x00000004U) /*!< Peripheral not ready                                                   */
#define HAL_CANFD_ERROR_NOT_STARTED     ((uint32_t)0x00000008U) /*!< Peripheral not started                                                 */
#define HAL_CANFD_ERROR_NOT_SUPPORTED   ((uint32_t)0x00000010U) /*!< Mode not supported                                                     */
#define HAL_CANFD_ERROR_PARAM           ((uint32_t)0x00000020U) /*!< Parameter error                                                        */
#define HAL_CANFD_ERROR_PENDING         ((uint32_t)0x00000040U) /*!< Pending operation                                                      */
#define HAL_CANFD_ERROR_RAM_ACCESS      ((uint32_t)0x00000080U) /*!< Message RAM Access Failure                                             */
#define HAL_CANFD_ERROR_FIFO_EMPTY      ((uint32_t)0x00000100U) /*!< Get element from empty FIFO                                            */
#define HAL_CANFD_ERROR_FIFO_FULL       ((uint32_t)0x00000200U) /*!< Put element in full FIFO                                               */

#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
#define HAL_CANFD_ERROR_INVALID_CALLBACK ((uint32_t)0x00000400U) /*!< Invalid Callback error                                                */
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
/**
  * @}
  */
  
/** @defgroup CANFD_frame_format CANFD Frame Format
  * @{
  */
#define CANFD_FRAME_CLASSIC   ((uint32_t)0x00000000U)                     /*!< Classic mode                      */
#define CANFD_FRAME_FD_NO_BRS ((uint32_t)0x00020000U)                     /*!< FD mode without BitRate Switching */
#define CANFD_FRAME_FD_BRS    ((uint32_t)(0x00020000U | 0x00040000U))     /*!< FD mode with BitRate Switching    */
/**
  * @}
  */

/** @defgroup CANFD_data_length_code CANFD Data Length Code
  * @{
  */
#define CANFD_DLC_BYTES_0              ((uint32_t)0x00000000U) /*!< 0 bytes data field  */
#define CANFD_DLC_BYTES_1              ((uint32_t)0x00000001U) /*!< 1 bytes data field  */
#define CANFD_DLC_BYTES_2              ((uint32_t)0x00000002U) /*!< 2 bytes data field  */
#define CANFD_DLC_BYTES_3              ((uint32_t)0x00000003U) /*!< 3 bytes data field  */
#define CANFD_DLC_BYTES_4              ((uint32_t)0x00000004U) /*!< 4 bytes data field  */
#define CANFD_DLC_BYTES_5              ((uint32_t)0x00000005U) /*!< 5 bytes data field  */
#define CANFD_DLC_BYTES_6              ((uint32_t)0x00000006U) /*!< 6 bytes data field  */
#define CANFD_DLC_BYTES_7              ((uint32_t)0x00000007U) /*!< 7 bytes data field  */
#define CANFD_DLC_BYTES_8              ((uint32_t)0x00000008U) /*!< 8 bytes data field  */
#define CANFD_DLC_BYTES_12             ((uint32_t)0x00000009U) /*!< 12 bytes data field */
#define CANFD_DLC_BYTES_16             ((uint32_t)0x0000000AU) /*!< 16 bytes data field */
#define CANFD_DLC_BYTES_20             ((uint32_t)0x0000000BU) /*!< 20 bytes data field */
#define CANFD_DLC_BYTES_24             ((uint32_t)0x0000000CU) /*!< 24 bytes data field */
#define CANFD_DLC_BYTES_32             ((uint32_t)0x0000000DU) /*!< 32 bytes data field */
#define CANFD_DLC_BYTES_48             ((uint32_t)0x0000000EU) /*!< 48 bytes data field */
#define CANFD_DLC_BYTES_64             ((uint32_t)0x0000000FU) /*!< 64 bytes data field */
/**
  * @}
  */

/** @defgroup CANFD_LLC_FORMAT_BITS CANFD LLC format bits
  * @{
  */
#define CANFD_LLC_FORMAT_DLC_0         ((uint32_t)0x00000001U)         /*!< BIT DLC_0  */
#define CANFD_LLC_FORMAT_DLC_1         ((uint32_t)0x00000002U)         /*!< BIT DLC_1  */
#define CANFD_LLC_FORMAT_DLC_2         ((uint32_t)0x00000004U)         /*!< BIT DLC_2  */
#define CANFD_LLC_FORMAT_DLC_3         ((uint32_t)0x00000008U)         /*!< BIT DLC_3  */
#define CANFD_LLC_FORMAT_IDE           ((uint32_t)0x00010000U)         /*!< BIT IDE    */
#define CANFD_LLC_FORMAT_FDF           ((uint32_t)0x00020000U)         /*!< BIT FDF    */
#define CANFD_LLC_FORMAT_BRS           ((uint32_t)0x00040000U)         /*!< BIT BRS    */
#define CANFD_LLC_FORMAT_RMF           ((uint32_t)0x00100000U)         /*!< BIT RMF    */
#define CANFD_LLC_FORMAT_KOER_0        ((uint32_t)0x01000000U)         /*!< BIT KOER_0 */
#define CANFD_LLC_FORMAT_KOER_1        ((uint32_t)0x02000000U)         /*!< BIT KOER_1 */
#define CANFD_LLC_FORMAT_KOER_2        ((uint32_t)0x04000000U)         /*!< BIT KOER_2 */
#define CANFD_LLC_FORMAT_ESI           ((uint32_t)0x08000000U)         /*!< BIT ESI    */
#define CANFD_LLC_FORMAT_LBF           ((uint32_t)0x10000000U)         /*!< BIT LBF    */

#define CANFD_LLC_FORMAT_DLC           ((uint32_t)0x0000000FU)         /*!< BIT DLC    */
#define CANFD_LLC_FORMAT_KOER          ((uint32_t)0x07000000U)         /*!< BIT KOER   */

#define CANFD_LLC_FORMAT_KOER_Pos      (24)                            /*!< BIT KOER Pos   */
/**
  * @}
  */

/** @defgroup CANFD_id_type CANFD id type
  * @{
  */
#define CANFD_STANDARD_ID            ((uint32_t)0x00000000U)         /*!< Standard ID element */
#define CANFD_EXTENDED_ID            ((uint32_t)0x00010000U)         /*!< Extended ID element */
/**
  * @}
  */

/** @defgroup CANFD_format CANFD Frame Format
  * @{
  */
#define CANFD_CLASSIC_CAN            ((uint32_t)0x00000000U)         /*!< Classic frame      */
#define CANFD_FD_CAN                 ((uint32_t)0x00020000U)         /*!< FD frame           */
/**
  * @}
  */

/** @defgroup CANFD_bit_rate_switching CANFD Bit Rate Switching
  * @{
  */
#define CANFD_BRS_OFF        ((uint32_t)0x00000000U)          /*!< FD mode without BitRate Switching */
#define CANFD_BRS_ON         ((uint32_t)0x00040000U)          /*!< FD mode with BitRate Switching    */
/**
  * @}
  */

/** @defgroup CANFD_frame_type CANFD frame type
  * @{
  */
#define CANFD_DATA_FRAME                   ((uint32_t)0x00000000U)         /*!< data frame        */
#define CANFD_REMOTE_FRAME                 ((uint32_t)0x00100000U)         /*!< remote frame      */
/**
  * @}
  */

/** @defgroup CANFD_protocol_error_type CANFD frame error type
  * @{
  */
#define CANFD_PROTOCOL_NONE_ERROR             ((uint32_t)0x00000000U)         /*!< no error                     */
#define CANFD_PROTOCOL_BIT_ERROR              ((uint32_t)0x00000001U)         /*!< frame bit error              */
#define CANFD_PROTOCOL_FORM_ERROR             ((uint32_t)0x00000002U)         /*!< frame form error             */
#define CANFD_PROTOCOL_STUFF_ERROR            ((uint32_t)0x00000003U)         /*!< frame bit sruff error        */
#define CANFD_PROTOCOL_ACK_ERROR              ((uint32_t)0x00000004U)         /*!< frame ack error              */
#define CANFD_PROTOCOL_CRC_ERROR              ((uint32_t)0x00000005U)         /*!< all stored data are invalid  */
#define CANFD_PROTOCOL_OTHER_ERROR            ((uint32_t)0x00000006U)         /*!< OTHER ERROR  */
/**
  * @}
  */

/** @defgroup CANFD_error_state_indicator CANFD Error State Indicator
  * @{
  */
#define CANFD_ESI_ACTIVE            ((uint32_t)0x00000000U)         /*!< CANFD node is error active     */
#define CANFD_ESI_PASSIVE           ((uint32_t)0x08000000U)         /*!< CANFD node is error passive    */
/**
  * @}
  */

/** @defgroup CANFD_loopback_frame CANFD loop back frame
  * @{
  */
#define CANFD_LOOPBACK_FRAME_ON         ((uint32_t)0x00000000U)         /*!< CANFD receive frame is LoopBack frame     */
#define CANFD_LOOPBACK_FRAME_OFF        ((uint32_t)0x10000000U)         /*!< CANFD receive frame is not LoopBack frame */
/**
  * @}
  */

/** @defgroup CANFD_operating_mode CANFD Operating Mode
* @{
*/
#define CANFD_MODE_NORMAL                (0x00000000U)                                  /*!< Normal mode   */
#define CANFD_MODE_RESTRICTED_OPERATION  ((uint32_t)CANFD_TSNCR_ROP)                    /*!< Restricted operation mode */
#define CANFD_MODE_LOOPBACK_EXT_NOACK    ((uint32_t)CANFD_MCR_LBME)                     /*!< Loopback mode */
#define CANFD_MODE_LOOPBACK_EXT_ACK      ((uint32_t)(CANFD_MCR_LBME | CANFD_MCR_SACK))  /*!< Loopback mode */
#define CANFD_MODE_LOOPBACK_INT          ((uint32_t)CANFD_MCR_LBMI)                     /*!< Loopback mode */
#define CANFD_MODE_SILENT                ((uint32_t)CANFD_MCR_LOM)                      /*!< Silent mode   */
#define CANFD_MODE_SILENT_LOOPBACK       ((uint32_t)(CANFD_MCR_LOM | CANFD_MCR_LBME))   /*!< Loopback combined with silent mode */
/**
  * @}
  */

/** @defgroup CANFD_retransmission_Limit CANFD Retransmission Limit
* @{
*/
#define CANFD_AUTO_RETRANSMISSION_1TRANSFER           (0x00000000U)
#define CANFD_AUTO_RETRANSMISSION_2TRANSFERS          ((uint32_t)CANFD_RLSSP_RETLIM_0)
#define CANFD_AUTO_RETRANSMISSION_3TRANSFERS          ((uint32_t)CANFD_RLSSP_RETLIM_1)
#define CANFD_AUTO_RETRANSMISSION_4TRANSFERS          ((uint32_t)(CANFD_RLSSP_RETLIM_0|CANFD_RLSSP_RETLIM_1))
#define CANFD_AUTO_RETRANSMISSION_5TRANSFERS          ((uint32_t)CANFD_RLSSP_RETLIM_2)
#define CANFD_AUTO_RETRANSMISSION_6TRANSFERS          ((uint32_t)(CANFD_RLSSP_RETLIM_0 | CANFD_RLSSP_RETLIM_2))
#define CANFD_AUTO_RETRANSMISSION_7TRANSFERS          ((uint32_t)(CANFD_RLSSP_RETLIM_1 | CANFD_RLSSP_RETLIM_2))
#define CANFD_AUTO_RETRANSMISSION_NO_LIMIT            ((uint32_t)(CANFD_RLSSP_RETLIM_0 | CANFD_RLSSP_RETLIM_1 | CANFD_RLSSP_RETLIM_2))
/**
  * @}
  */

/** @defgroup CANFD_rearbitration_Limit CANFD Rearbitration Limit
* @{
*/
#define CANFD_AUTO_REARBITRATION_1TRANSFER           (0x00000000U)
#define CANFD_AUTO_REARBITRATION_2TRANSFERS          ((uint32_t)CANFD_RLSSP_REALIM_0)
#define CANFD_AUTO_REARBITRATION_3TRANSFERS          ((uint32_t)CANFD_RLSSP_REALIM_1)
#define CANFD_AUTO_REARBITRATION_4TRANSFERS          ((uint32_t)(CANFD_RLSSP_REALIM_0|CANFD_RLSSP_REALIM_1))
#define CANFD_AUTO_REARBITRATION_5TRANSFERS          ((uint32_t)CANFD_RLSSP_REALIM_2)
#define CANFD_AUTO_REARBITRATION_6TRANSFERS          ((uint32_t)(CANFD_RLSSP_REALIM_0 | CANFD_RLSSP_REALIM_2))
#define CANFD_AUTO_REARBITRATION_7TRANSFERS          ((uint32_t)(CANFD_RLSSP_REALIM_1 | CANFD_RLSSP_REALIM_2))
#define CANFD_AUTO_REARBITRATION_NO_LIMIT            ((uint32_t)(CANFD_RLSSP_REALIM_0 | CANFD_RLSSP_REALIM_1 | CANFD_RLSSP_REALIM_2))
/**
  * @}
  */
  
/** @defgroup CANFD_RX_FIFO_threshold CANFD receive fifo almost full warning limit.
* @{
*/
#define CANFD_RX_FIFO_THRESHOLD_1SLOT          ((uint32_t)CANFD_WECR_AFWL_0)
#define CANFD_RX_FIFO_THRESHOLD_2SLOTS         ((uint32_t)CANFD_WECR_AFWL_1)
#define CANFD_RX_FIFO_THRESHOLD_3SLOTS         ((uint32_t)(CANFD_WECR_AFWL_0 | CANFD_WECR_AFWL_1))
/**
  * @}
  */
  
/**
 * @defgroup CANFD_FD_protocol CANFD FD Protocol
 * @{
 */
#define CANFD_FD_BOSCH               (0x00000000U)
#define CANFD_FD_ISO_11898           (CANFD_MCR_FD_ISO)
/**
 * @}
 */

/**
 * @defgroup CANFD_transmit_mode CANFD transmit Mode
 * @{
 */
#define CANFD_TXFIFO_PTB_SEND            (0x00000001U)
#define CANFD_TXFIFO_STB_SEND_ONE        (0x00000002U)
#define CANFD_TXFIFO_STB_SEND_ALL        (0x00000004U)
/**
 * @}
 */

/**
* @defgroup CANFD_filter_channel CANFD Acceptance Channel
* @{
*/
#define CANFD_FILTER_CHANNEL_0           (0x00000000U)
#define CANFD_FILTER_CHANNEL_1           (0x00000001U)
#define CANFD_FILTER_CHANNEL_2           (0x00000002U)
#define CANFD_FILTER_CHANNEL_3           (0x00000003U)
#define CANFD_FILTER_CHANNEL_4           (0x00000004U)
#define CANFD_FILTER_CHANNEL_5           (0x00000005U)
#define CANFD_FILTER_CHANNEL_6           (0x00000006U)
#define CANFD_FILTER_CHANNEL_7           (0x00000007U)
#define CANFD_FILTER_CHANNEL_8           (0x00000008U)
#define CANFD_FILTER_CHANNEL_9           (0x00000009U)
#define CANFD_FILTER_CHANNEL_10          (0x0000000AU)
#define CANFD_FILTER_CHANNEL_11          (0x0000000BU)
/**
 * @}
 */

/** @defgroup CANFD_filter_rank CANFD Filter rank
  * @{
  */
#define CANFD_FILTER_RANK_CHANNEL_NUMBER (0x00000000U)  /*!< Enable the rank of the selected channels.  */
#define CANFD_FILTER_RANK_NONE           (0x00000001U)  /*!< Disable the selected rank (selected channel).  */
/**
  * @}
  */

/**
 * @defgroup CANFD_RX_FIFO_overflow_mode CANFD Receive Buffer Overflow Mode
 * @{
 */
#define CANFD_RX_FIFO_OVERWRITE          (0x00000000U)    /*!< Rx FIFO overwrite mode */
#define CANFD_RX_FIFO_BLOCKING           (CANFD_MCR_ROM)   /*!< Rx FIFO blocking mode  */
/**
 * @}
 */

/**
 * @defgroup CANFD_receive_fifo_state CANFD Receive Buffer status
 * @{
 */
#define CANFD_RX_FIFO_EMPTY                    (0x00000000U)                           /*!< Rx FIFO empty  */
#define CANFD_RX_FIFO_LE_AFWL                  (CANFD_MCR_RSTAT_0)                     /*!< Rx FIFO more than empty and less than almost full (AFWL) */
#define CANFD_RX_FIFO_GT_AFWL                  (CANFD_MCR_RSTAT_1)                     /*!< Rx FIFO Greater than or equal to almost full */
#define CANFD_RX_FIFO_FULL                     (CANFD_MCR_RSTAT_0 | CANFD_MCR_RSTAT_1) /*!< Rx FIFO full */
/**
 * @}
 */

/**
 * @defgroup CANFD_transmit_fifo_state CANFD Transmit Buffer status
 * @{
 */
#define CANFD_STB_FIFO_EMPTY                  (0x00000000U)   /*!< STB FIFO empty  */
#define CANFD_STB_FIFO_LE_HF                  (CANFD_MCR_TSSTAT_0)   /*!< STB FIFO less than or equal to half full */
#define CANFD_STB_FIFO_GT_HF                  (CANFD_MCR_TSSTAT_1)   /*!< STB FIFO More than half full */
#define CANFD_STB_FIFO_FULL                   (CANFD_MCR_TSSTAT_0 | CANFD_MCR_TSSTAT_1)   /*!< STB FIFO full */
/**
 * @}
 */

/**
 * @defgroup CANFD_fifo_type CANFD Transmit FIFO type
 * @{
 */
#define CANFD_TX_FIFO_PTB                     (0x0U)
#define CANFD_TX_FIFO_STB                     (0x1U)
/**
 * @}
 */

/**
 * @defgroup CANFD_STB_priority_mode CANFD STB Priority Mode
 * @{
 */
#define CANFD_STB_PRIORITY_FIFO               (0x0U)                  /*!< Data first in and first be transmitted. */
#define CANFD_STB_PRIORITY_ID                 (CANFD_MCR_TSMODE)      /*!< Data with smallest ID first be transmitted. */
/**
 * @}
 */

/**
 * @defgroup CANFD_transmit_status CANFD Tx Frame status
 * @{
 */
#define CANFD_TRANSMIT_STATUS_IDLE               (0x00000000U)                  /*!< No transmission in progress. */
#define CANFD_TRANSMIT_STATUS_ONGOING            (0x00000001U)                  /*!< Transmission active without any issues. */
#define CANFD_TRANSMIT_STATUS_LOST_ARBITRATION   (0x00000002U)                  /*!< Arbitration lost. re-arbitration may take place with respect to REALIM. */
#define CANFD_TRANSMIT_STATUS_TRANSMITTED        (0x00000003U)                  /*!< Transmission successfully completed. */
#define CANFD_TRANSMIT_STATUS_ABORTED            (0x00000004U)                  /*!< Transmission aborted (TPA, TSA). */
#define CANFD_TRANSMIT_STATUS_DISTURBED          (0x00000005U)                  /*!< Transmission error. Retransmission may take place with respect to RETLIM. */
#define CANFD_TRANSMIT_STATUS_REJECTED           (0x00000006U)                  /*!< Misconfiguration of the frame format in the LLC frame. */
/**
 * @}
 */
 
/**
 * @defgroup CANFD_TT_trigger_type TTCAN Trigger Type
 * @{
 */
#define CANFD_TT_IMMEDIATE_TRIGGER        (0x0U)
#define CANFD_TT_RX_TIME_TRIG             (CANFD_TTCR_TTYPE_0)
#define CANFD_TT_TX_SINGLE_TRIG           (CANFD_TTCR_TTYPE_1)
#define CANFD_TT_TX_START_TRIG            (CANFD_TTCR_TTYPE_0 | CANFD_TTCR_TTYPE_1)
#define CANFD_TT_TX_STOP_TRIG             (CANFD_TTCR_TTYPE_2)
/**
 * @}
 */

/**
 * @defgroup CANFD_TTCAN_FIFO_mode TTCAN FIFO mode
 * @{
 */
#define CANFD_TT_FIFO_PTB_STB           (0x0U)
#define CANFD_TT_FIFO_MERGE             (CANFD_MCR_TTTBM)
/**
 * @}
 */

/**
 * @defgroup CANFD_TT_FIFO_INDEX TTCAN FIFO index
 * @{
 */
#define CANFD_TT_FIFO_SLOT0             (0x00000000U)
#define CANFD_TT_FIFO_SLOT1             (0x00000001U)
#define CANFD_TT_FIFO_SLOT2             (0x00000002U)
#define CANFD_TT_FIFO_SLOT3             (0x00000003U)
/**
 * @}
 */

/**
 * @defgroup CANFD_TT_FIFO_State_Set TTCAN FIFO State Set
 * @{
 */
#define CANFD_TT_FIFO_SLOT_SET_EMPTY    (CANFD_TTCR_TBE)
#define CANFD_TT_FIFO_SLOT_SET_FULL     (CANFD_TTCR_TBF)
/**
 * @}
 */
 
/** @defgroup CANFD_timestamp_prescaler CANFD timestamp prescaler
  * @{
  */
#define CANFD_TT_PRESC_1                ((uint32_t)0x00000000U)                       /*!< Timestamp counter time unit in equal to CAN bit time                 */
#define CANFD_TT_PRESC_2                (CANFD_TTCR_T_PRESC_0)                        /*!< Timestamp counter time unit in equal to CAN bit time multiplied by 2  */
#define CANFD_TT_PRESC_4                (CANFD_TTCR_T_PRESC_1)                        /*!< Timestamp counter time unit in equal to CAN bit time multiplied by 4  */
#define CANFD_TT_PRESC_8                (CANFD_TTCR_T_PRESC_0 |CANFD_TTCR_T_PRESC_1)  /*!< Timestamp counter time unit in equal to CAN bit time multiplied by 8  */
/**
  * @}
  */

/** @defgroup CANFD_flags CANFD Flags
  * @{
  */
#define CANFD_FLAG_TX_STB_COMPLETE             CANFD_IFR_TSIF             /*!< STB Transmission Completed   */
#define CANFD_FLAG_TX_PTB_COMPLETE             CANFD_IFR_TPIF             /*!< PTB Transmission Completed   */
#define CANFD_FLAG_TX_ABORT_COMPLETE           CANFD_IFR_AIF              /*!< Transmission Cancellation Finished */
#define CANFD_FLAG_RX_COMPLETE                 CANFD_IFR_RIF              /*!< Rx Completed   */
#define CANFD_FLAG_RX_FIFO_ALMOST_FULL         CANFD_IFR_RAFIF            /*!< Rx FIFO almost full */
#define CANFD_FLAG_RX_FIFO_FULL                CANFD_IFR_RFIF             /*!< Rx FIFO full */
#define CANFD_FLAG_RX_FIFO_OVERFLOW            CANFD_IFR_ROIF             /*!< Overflow of CANFD Rx FIFO   */
#define CANFD_FLAG_ARB_LOST                    CANFD_IFR_ALIF             /*!< Arbitration lost detected   */
#define CANFD_FLAG_ERROR_PASSIVE               CANFD_IFR_EPIF             /*!< Error_Passive status changed  */
#define CANFD_FLAG_BUS_ERROR                   CANFD_IFR_BEIF             /*!< Bus_Off status changed       */
#define CANFD_FLAG_ERROR_TOGGLE                CANFD_IFR_EIF              /*!< Bus_Off status changed or Error_Passive status changed    */
#define CANFD_FLAG_TRIGGER_COMPLETE            CANFD_IFR_TTIF             /*!< Trigger Interrupt flag   */
#define CANFD_FLAG_TRIGGER_ERROR               CANFD_IFR_TEIF             /*!< Trigger error Interrupt flag   */
#define CANFD_FLAG_TIMESTAMP_WRAPAROUND        CANFD_IFR_WTIF             /*!< Timestamp counter wrapped around */

#define CANFD_FLAG_ALL_ERROR                   (CANFD_FLAG_RX_FIFO_OVERFLOW | CANFD_FLAG_ARB_LOST  |  \
                                                CANFD_FLAG_ERROR_PASSIVE    | CANFD_FLAG_BUS_ERROR |  \
                                                CANFD_FLAG_ERROR_TOGGLE     | CANFD_FLAG_TRIGGER_ERROR)
/**
  * @}
  */

/** @defgroup CANFD_interrupts CANFD Interrupts
  * @{
  */
#define CANFD_IT_TX_STB_COMPLETE           CANFD_IER_TSIE             /*!< STB Transmission Completed */
#define CANFD_IT_TX_PTB_COMPLETE           CANFD_IER_TPIE             /*!< PTB Transmission Completed */
#define CANFD_IT_ERROR_TOGGLE              CANFD_IER_EIE              /*!< Bus_Off status changed or Error_Passive status changed    */
#define CANFD_IT_RX_FIFO_ALMOST_FULL       CANFD_IER_RAFIE            /*!< Rx FIFO almost full */
#define CANFD_IT_RX_FIFO_FULL              CANFD_IER_RFIE             /*!< Rx FIFO full */
#define CANFD_IT_RX_FIFO_OVERFLOW          CANFD_IER_ROIE             /*!< Overflow of CANFD Rx FIFO   */
#define CANFD_IT_RX_COMPLETE               CANFD_IER_RIE              /*!< Rx Completed   */
#define CANFD_IT_ARB_LOST                  CANFD_IER_ALIE             /*!< Arbitration lost detected   */
#define CANFD_IT_ERROR_PASSIVE             CANFD_IER_EPIE             /*!< Error_Passive status changed  */
#define CANFD_IT_BUS_ERROR                 CANFD_IER_BEIE             /*!< Bus_Off status changed       */
#define CANFD_IT_TRIGGER_COMPLETE          CANFD_IER_TTIE             /*!< Trigger Interrupt flag   */
#define CANFD_IT_TIMESTAMP_WRAPAROUND      CANFD_IER_WTIE             /*!< Timestamp counter wrapped around */

#define CANFD_IT_ALL_ERROR                 (CANFD_IT_ERROR_TOGGLE | CANFD_IT_RX_FIFO_OVERFLOW | \
                                            CANFD_IT_ARB_LOST     | CANFD_IT_ERROR_PASSIVE    | \
                                            CANFD_IT_BUS_ERROR)
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup CANFD_Exported_Macros CANFD Exported Macros
  * @{
  */

/**
  * @brief  Enable the specified CANFD interrupts.
  * @param  __HANDLE__ CANFD handle.
  * @param  __INTERRUPT__ CANFD interrupt.
  *         This parameter can be any combination of @arg CANFD_Interrupts
  * @retval None
  */
#define __HAL_CANFD_ENABLE_IT(__HANDLE__, __INTERRUPT__)             \
  (__HANDLE__)->Instance->IER |= (__INTERRUPT__)

/**
  * @brief  Disable the specified CANFD interrupts.
  * @param  __HANDLE__ CANFD handle.
  * @param  __INTERRUPT__ CANFD interrupt.
  *         This parameter can be any combination of @arg CANFD_Interrupts
  * @retval None
  */
#define __HAL_CANFD_DISABLE_IT(__HANDLE__, __INTERRUPT__)               \
  ((__HANDLE__)->Instance->IER) &= ~(__INTERRUPT__)
  
/** @brief  Check if the specified CANFD interrupt source is enabled or disabled.
  * @param  __HANDLE__ CANFD handle.
  * @param  __INTERRUPT__ specifies the CANFD interrupt source to check.
  *         This parameter can be a value of @arg CANFD_Interrupts
  * @retval ITStatus
  */
#define __HAL_CANFD_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) ((__HANDLE__)->Instance->IER & (__INTERRUPT__))

/**
  * @brief  Check whether the specified CANFD flag is set or not.
  * @param  __HANDLE__ CANFD handle.
  * @param  __FLAG__ CANFD flag.
  *         This parameter can be one of @arg CANFD_flags
  * @retval FlagStatus
  */
#define __HAL_CANFD_GET_FLAG(__HANDLE__, __FLAG__) ((__HANDLE__)->Instance->IFR & (__FLAG__))

/**
  * @brief  Clear the specified CANFD flags.
  * @param  __HANDLE__ CANFD handle.
  * @param  __FLAG__ specifies the flags to clear.
  *         This parameter can be any combination of @arg CANFD_flags
  * @retval None
  */
#define __HAL_CANFD_CLEAR_FLAG(__HANDLE__, __FLAG__)             \
  ((__HANDLE__)->Instance->IFR) = (__FLAG__)

/** @brief  Return Rx FIFO fill level.
  * @param  __HANDLE__ CANFD handle.
  * @retval Rx FIFO fill level.
  */
#define __HAL_CANFD_GET_RX_FIFO_FILL_LEVEL(__HANDLE__) ((__HANDLE__)->Instance->MCR & CANFD_MCR_RSTAT)

/** @brief  Return STB FIFO free level.
  * @param  __HANDLE__ CANFD handle.
  * @retval STB FIFO free level.
  */
#define __HAL_CANFD_GET_STB_FIFO_FREE_LEVEL(__HANDLE__) ((__HANDLE__)->Instance->MCR & CANFD_MCR_TSSTAT)

/**
  * @brief  Enable canbus OFF.
  * @param  __HANDLE__ CANFD handle.
  * @retval None
  */
#define __HAL_CANFD_EnableCanBusOff(__HANDLE__)  (SET_BIT((__HANDLE__)->Instance->MCR, CANFD_MCR_BUSOFF))

/**
  * @brief  Disable canbus OFF.
  * @param  __HANDLE__ CANFD handle.
  * @retval None
  */
#define __HAL_CANFD_DisableCanBusOff(__HANDLE__)  (CLEAR_BIT((__HANDLE__)->Instance->MCR, CANFD_MCR_BUSOFF))
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup CNAFD_Private_Macros CANFD Private Macros
  * @{
  */

#define IS_CANFD_FRAME_FORMAT(FORMAT)   (((FORMAT) == CANFD_FRAME_CLASSIC) || \
                                         ((FORMAT) == CANFD_FRAME_FD_NO_BRS) || \
                                         ((FORMAT) == CANFD_FRAME_FD_BRS))
#define IS_CANFD_MODE(MODE)  (((MODE) == CANFD_MODE_NORMAL) || \
                              ((MODE) == CANFD_MODE_RESTRICTED_OPERATION) || \
                              ((MODE) == CANFD_MODE_LOOPBACK_EXT_NOACK) || \
                              ((MODE) == CANFD_MODE_LOOPBACK_EXT_ACK) || \
                              ((MODE) == CANFD_MODE_LOOPBACK_INT) || \
                              ((MODE) == CANFD_MODE_SILENT) || \
                              ((MODE) == CANFD_MODE_SILENT_LOOPBACK))
#define IS_CANFD_CANFD_PROTOCOL(PROTOCOL)  (((PROTOCOL) == CANFD_FD_BOSCH) || \
                                            ((PROTOCOL) == CANFD_FD_ISO_11898))
#define IS_CANFD_CKDIV(DIV)  (((DIV) >= 1U) && ((DIV) <= 32U))
#define IS_CANFD_NOMINAL_SJW(SJW)  (((SJW) >= 1U) && ((SJW) <= 128U))
#define IS_CANFD_NOMINAL_TSEG1(TSEG1)  (((TSEG1) >= 2U) && ((TSEG1) <= 513U))
#define IS_CANFD_NOMINAL_TSEG2(TSEG2)  (((TSEG2) >= 1U) && ((TSEG2) <= 128U))
#define IS_CANFD_DATA_SJW(SJW)  (((SJW) >= 1U) && ((SJW) <= 128U))
#define IS_CANFD_DATA_TSEG1(TSEG1)  (((TSEG1) >= 2U) && ((TSEG1) <= 257U))
#define IS_CANFD_DATA_TSEG2(TSEG2)  (((TSEG2) >= 1U) && ((TSEG2) <= 128U))
#define IS_CANFD_SSPOFF(SSPOFF)  ((SSPOFF) <= 255U)

#define IS_CANFD_ID_TYPE(TYPE)   (((TYPE) == CANFD_STANDARD_ID) || \
                                  ((TYPE) == CANFD_EXTENDED_ID))
#define IS_CANFD_FILTER_CHANNEL(CHANNEL)  ((CHANNEL) <= CANFD_FILTER_CHANNEL_11)
#define IS_CANFD_RANK(RANK)   (((RANK) == CANFD_FILTER_RANK_CHANNEL_NUMBER) || \
                               ((RANK) == CANFD_FILTER_RANK_NONE))
#define IS_CANFD_FILTER_ID(ID)  ((ID) <= 0x1FFFFFFFU)
#define IS_CANFD_FILTER_FORMAT(FORMAT)  ((FORMAT) <= 0x1F1707FFU)
#define IS_CANFD_MASK_ID(ID)  ((ID) <= 0x1FFFFFFFU)
#define IS_CANFD_MASK_FORMAT(FORMAT)  ((FORMAT) <= 0x1F1707FFU)

#define IS_CANFD_MAX_VALUE(ID, VALUE)  ((ID) <= VALUE)
#define IS_CANFD_TX_FRAME_TYPE(TYPE)   (((TYPE) == CANFD_DATA_FRAME) || \
                                        ((TYPE) == CANFD_REMOTE_FRAME))
#define IS_CANFD_Handle(ID)  ((ID) <= 0xFFU)
#define IS_CANFD_DLC(DLC)  ((DLC) <= CANFD_DLC_BYTES_64)
#define IS_CANFD_TX_FIFO_TYPE(TYPE)   (((TYPE) == CANFD_TX_FIFO_PTB) || \
                                        ((TYPE) == CANFD_TX_FIFO_STB))

#define IS_CANFD_TRANSMIT_MODE(MODE)   (((MODE) == CANFD_TXFIFO_PTB_SEND) || \
                                        ((MODE) == CANFD_TXFIFO_STB_SEND_ONE) || \
                                        ((MODE) == CANFD_TXFIFO_STB_SEND_ALL))

#define IS_CANFD_REARBITRATION_LIMIT(LIMIT)  ((LIMIT) <= CANFD_AUTO_REARBITRATION_NO_LIMIT)

#define IS_CANFD_RETRANSMISSION_LIMIT(LIMIT)  ((LIMIT) <= CANFD_AUTO_RETRANSMISSION_NO_LIMIT)

#define IS_CANFD_STB_PRIORITY(PRIORITY)   (((PRIORITY) == CANFD_STB_PRIORITY_FIFO) || \
                                           ((PRIORITY) == CANFD_STB_PRIORITY_ID))

#define IS_CANFD_RX_FIFO_MODE(MODE)   (((MODE) == CANFD_RX_FIFO_OVERWRITE) || \
                                       ((MODE) == CANFD_RX_FIFO_BLOCKING))

#define IS_CANFD_RX_FIFO_THRESHOLD(THRESHOLD)   (((THRESHOLD) == CANFD_RX_FIFO_THRESHOLD_1SLOT) || \
                                                 ((THRESHOLD) == CANFD_RX_FIFO_THRESHOLD_2SLOTS) || \
                                                 ((THRESHOLD) == CANFD_RX_FIFO_THRESHOLD_3SLOTS))

#define IS_CANFD_TT_FIFO_MODE(MODE)   (((MODE) == CANFD_TT_FIFO_PTB_STB) || \
                                       ((MODE) == CANFD_TT_FIFO_MERGE))
#define IS_CANFD_TT_PRESCALER(PRESCALER)   (((PRESCALER) == CANFD_TT_PRESC_1) || \
                                            ((PRESCALER) == CANFD_TT_PRESC_2) ||\
                                            ((PRESCALER) == CANFD_TT_PRESC_4) ||\
                                            ((PRESCALER) == CANFD_TT_PRESC_8))
#define IS_CANFD_TX_ENABLE_WINDOW(WINDOW)  (((WINDOW) >= 1U) && ((WINDOW) <= 15U))
#define IS_CANFD_WATCH_TRIG_TIME(TIME)  ((TIME) <= 0xFFFFU)
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup CANFD_Exported_Functions CANFD Exported Functions
  * @{
  */

/** @addtogroup CANFD_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */

/* Initialization/de-initialization functions  **********************************/
HAL_StatusTypeDef HAL_CANFD_Init(CANFD_HandleTypeDef *hcanfd);
void HAL_CANFD_MspInit(CANFD_HandleTypeDef *hcan);

/* Callbacks Register/UnRegister functions  ***********************************/
#if USE_HAL_CANFD_REGISTER_CALLBACKS == 1
HAL_StatusTypeDef HAL_CANFD_RegisterCallback(CANFD_HandleTypeDef *hcanfd, HAL_CANFD_CallbackIDTypeDef CallbackID, void (* pCallback)(CANFD_HandleTypeDef *_hCANFD));
#endif /* USE_HAL_CANFD_REGISTER_CALLBACKS */
/**
  * @}
  */

/** @addtogroup CANFD_Exported_Functions_Group2 IO operation functions
  * @{
  */

/* IO operation functions *******************************************************/
HAL_StatusTypeDef HAL_CANFD_ConfigFilter(CANFD_HandleTypeDef *hcanfd, CANFD_FilterTypeDef *sFilter);
HAL_StatusTypeDef HAL_CANFD_AddMessageToTxFifo(CANFD_HandleTypeDef *hcanfd, CANFD_TxHeaderTypeDef *pTxHeader, uint8_t *pData, uint32_t TxFifoType);
HAL_StatusTypeDef HAL_CANFD_ActivateTxRequest(CANFD_HandleTypeDef *hcanfd, uint32_t activeTxMode);
HAL_StatusTypeDef HAL_CANFD_GetRxMessage(CANFD_HandleTypeDef *hcanfd, CANFD_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData);
HAL_StatusTypeDef HAL_CANFD_AbortTxRequest(CANFD_HandleTypeDef *hcanfd, uint32_t TransmitType);
HAL_StatusTypeDef HAL_CANFD_ConfigRearbitrationLimit(CANFD_HandleTypeDef *hcanfd, uint32_t reaLimit);
HAL_StatusTypeDef HAL_CANFD_ConfigRetransmissionLimit(CANFD_HandleTypeDef *hcanfd, uint32_t retLimit);
HAL_StatusTypeDef HAL_CANFD_ConfigTxFifoPriority(CANFD_HandleTypeDef *hcanfd, uint32_t StbPriority);
HAL_StatusTypeDef HAL_CANFD_ConfigRxFifoOverwrite(CANFD_HandleTypeDef *hcanfd, uint32_t OverflowMode);
HAL_StatusTypeDef HAL_CANFD_ConfigRxFifoThreshold(CANFD_HandleTypeDef *hcanfd, uint32_t Threshold);
HAL_StatusTypeDef HAL_CANFD_EnableReceiveErrorDataFrame(CANFD_HandleTypeDef *hcanfd);
HAL_StatusTypeDef HAL_CANFD_DisableReceiveErrorDataFrame(CANFD_HandleTypeDef *hcanfd);
HAL_StatusTypeDef HAL_CANFD_TT_Config(CANFD_HandleTypeDef *hcanfd, CANFD_TimeTriggerTypeDef *pTTConfig);
HAL_StatusTypeDef HAL_CANFD_TT_AddMessageToFifo(CANFD_HandleTypeDef *hcanfd, CANFD_TxHeaderTypeDef *txHeader, uint8_t *pData, uint32_t slotIndex);
HAL_StatusTypeDef HAL_CANFD_TT_ActivateTrigRequest(CANFD_HandleTypeDef *hcanfd, uint32_t triggerType, uint32_t trigTime, uint32_t slotIndex);
HAL_StatusTypeDef HAL_CANFD_TT_ConfigTxBufferState(CANFD_HandleTypeDef *hcanfd, uint32_t slotIndex, uint32_t slotState);
HAL_StatusTypeDef HAL_CANFD_Start(CANFD_HandleTypeDef *hcanfd);
HAL_StatusTypeDef HAL_CANFD_ActivateNotification(CANFD_HandleTypeDef *hcanfd, uint32_t activeITs);
HAL_StatusTypeDef HAL_CANFD_DeactivateNotification(CANFD_HandleTypeDef *hcanfd, uint32_t inactiveITs);
HAL_StatusTypeDef HAL_CANFD_EnterStandbyMode(CANFD_HandleTypeDef *hcanfd);
HAL_StatusTypeDef HAL_CANFD_ExitStandbyMode(CANFD_HandleTypeDef *hcanfd);
HAL_StatusTypeDef HAL_CANFD_GetProtocolStatus(CANFD_HandleTypeDef *hcanfd, CANFD_ProtocolStatusTypeDef *ProtocolStatus);
HAL_StatusTypeDef HAL_CANFD_GetTransmitStatus(CANFD_HandleTypeDef *hcanfd, CANFD_TxStatusTypeDef *TxStatus);
HAL_StatusTypeDef HAL_CANFD_Stop(CANFD_HandleTypeDef *hcanfd);

void HAL_CANFD_IRQHandler(CANFD_HandleTypeDef *hcanfd);
void HAL_CANFD_RxCpltCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_RxFifoAlmostFullCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_RxFifoFullCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_PtbTxCpltCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_StbTxCpltCallback(CANFD_HandleTypeDef *hcan, uint32_t LastSTBTxType);
void HAL_CANFD_RxFifoOverflowCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_ArbitrationLostCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_TxAbortCallback(CANFD_HandleTypeDef *hcan, uint32_t LastAbortTxType);
void HAL_CANFD_PassiveErrorCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_BusErrorCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_ErrorChangeCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_TT_RxTimeTrigCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_TT_TxSingleTrigCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_TT_TxStartTrigCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_TT_TxStopTrigCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_TT_TimestampWraparoundCallback(CANFD_HandleTypeDef *hcan);
void HAL_CANFD_TT_TrigErrorCallback(CANFD_HandleTypeDef *hcan);
/**
  * @}
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

#endif /* __PY32F4xx_HAL_CANFD_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
