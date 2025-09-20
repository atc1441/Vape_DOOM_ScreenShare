/**
  ******************************************************************************
  * @file    py32f4xx_hal_flash.h
  * @author  MCU Application Team
  * @brief   Header file of FLASH HAL module.
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
#ifndef __PY32F4XX_HAL_FLASH_H
#define __PY32F4XX_HAL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f4xx_hal_def.h"

/** @addtogroup PY32F4xx_HAL_Driver
  * @{
  */

/** @addtogroup FLASH
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup FLASH_Exported_Types FLASH Exported Types
  * @{
  */

/**
  * @brief  FLASH Erase structure definition
  */
typedef struct
{
  uint32_t TypeErase;        /*!< Mass erase or page erase.
                                  This parameter can be a value of @ref FLASH_Type_Erase */
  uint32_t PageAddress;      /*!< PageAdress: Initial FLASH page address to erase when mass erase /sector erase /block erase is disabled
                                  This parameter must be a number between Min_Data = FLASH_BASE and Max_Data = FLASH_END */
  uint32_t NbPages;          /*!< Number of pages to be erased.
                                  This parameter must be a value between 1 and (FLASH_PAGE_NB - value of initial page)*/
  uint32_t SectorAddress;    /*!< SectorAddress: Initial FLASH sector address to erase when mass erase /page erase /block erase is disabled
                                  This parameter must be a number between Min_Data = FLASH_BASE and Max_Data = FLASH_END */    
  uint32_t NbSectors;        /*!< Number of sectors to be erased.
                                  This parameter must be a value between 1 and (FLASH_SECTOR_NB - value of initial sector)*/
  uint32_t BlockAddress;     /*!< BlockAddress: Initial FLASH block address to erase when mass erase /page erase /sector erase is disabled
                                  This parameter must be a number between Min_Data = FLASH_BASE and Max_Data = FLASH_END */    
  uint32_t NbBlocks;         /*!< Number of blocks to be erased.
                                  This parameter must be a value between 1 and (FLASH_BLOCK_NB - value of initial block)*/    
} FLASH_EraseInitTypeDef;

/**
  * @brief  FLASH Option Bytes PROGRAM structure definition
  */
typedef struct
{
  uint32_t  OptionType;         /*!< OptionType: Option byte to be configured.
                                  This parameter can be a value of @ref FLASH_Option_Type */

  uint32_t  WRPBlock;           /*!< WRPBlock: This bitfield specifies the block (s) which are write protected.
                                   This parameter can be a combination of @ref FLASH_Option_Bytes_Write_Protection */
      
  uint32_t  RDPLevel;           /*!< RDPLevel: Set the read protection level.
                                   This parameter can be a value of @ref FLASH_OB_Read_Protection */
  
  uint32_t  USERType;           /*!< User option byte(s) to be configured (used for OPTIONBYTE_USER).
                                   This parameter can be a combination of @ref FLASH_OB_USER_Type */

  uint32_t  USERConfig;         /*!< Value of the user option byte (used for OPTIONBYTE_USER).
                                   This parameter can be a combination of 
                                   @ref FLASH_OB_USER_IWDG_SW,
                                   @ref FLASH_OB_USER_NRST_STDBY,
                                   @ref FLASH_OB_USER_NRST_STOP */
  
} FLASH_OBProgramInitTypeDef; 

/**
* @brief  FLASH handle Structure definition
*/
typedef struct
{
  HAL_LockTypeDef   Lock;                           /* FLASH locking object */
  uint32_t          ErrorCode;                      /* FLASH error code */
  uint32_t          ProcedureOnGoing;               /* Internal variable to indicate which procedure is ongoing or not in IT context */
  uint32_t          Address;                        /* Internal variable to save address selected for program in IT context */
  uint32_t          PageOrSectorOrBlock;            /* Internal variable to define the current page or sector or block which is erasing in IT context */
  uint32_t          NbPagesSectorsBlocksToErase;    /* Internal variable to save the remaining pages to erase in IT context */
} FLASH_ProcessTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup FLASH_Exported_Constants FLASH Exported Constants
  * @{
  */

/** @defgroup FLASH_Latency FLASH Latency
  * @{
  */
#define FLASH_LATENCY_0                 0x00000000UL                                                       /*!< FLASH Zero wait state */
#define FLASH_LATENCY_1                 FLASH_ACR_LATENCY_0                                                /*!< FLASH One wait state */
#define FLASH_LATENCY_3                 (FLASH_ACR_LATENCY_0 | FLASH_ACR_LATENCY_1)                        /*!< FLASH Three wait state */
#define FLASH_LATENCY_4                 FLASH_ACR_LATENCY_2                                                /*!< FLASH Four wait state */
#define FLASH_LATENCY_5                 (FLASH_ACR_LATENCY_0 | FLASH_ACR_LATENCY_2)                        /*!< FLASH Five wait state */
#define FLASH_LATENCY_6                 (FLASH_ACR_LATENCY_1 | FLASH_ACR_LATENCY_2)                        /*!< FLASH Six wait state */
/**
  * @}
  */
  
/** @defgroup FLASH_Type_Erase FLASH erase type
  * @{
  */
#define FLASH_TYPEERASE_MASSERASE       (0x01U)  /*!<Flash mass erase activation*/
#define FLASH_TYPEERASE_PAGEERASE       (0x02U)  /*!<Flash page erase activation*/
#define FLASH_TYPEERASE_SECTORERASE     (0x03U)  /*!<Flash sector erase activation*/ 
#define FLASH_TYPEERASE_BLOCKERASE      (0x04U)  /*!<Flash block erase activation*/
/**
  * @}
  */

/** @defgroup FLASH_Flags FLASH Flags Definition
  * @{
  */
#define FLASH_FLAG_BSY                  FLASH_SR_BSY      /*!< FLASH Operation Busy flag */
#define FLASH_FLAG_OPTVERR              FLASH_SR_OPTVERR  /*!< FLASH Option validity error flag */
#define FLASH_FLAG_WRPERR               FLASH_SR_WRPERR   /*!< FLASH Write protection error flag */
#define FLASH_FLAG_EOP                  FLASH_SR_EOP      /*!< FLASH End of operation flag */
#define FLASH_FLAG_ALL_ERRORS           (FLASH_FLAG_WRPERR | FLASH_FLAG_OPTVERR)
/**
  * @}
  */

/** @defgroup FLASH_Interrupt_definition FLASH Interrupts Definition
  * @brief FLASH Interrupt definition
  * @{
  */
#define FLASH_IT_EOP                    FLASH_CR_EOPIE              /*!< End of FLASH Operation Interrupt source */
#define FLASH_IT_OPERR                  FLASH_CR_ERRIE              /*!< Error Interrupt source */
/**
  * @}
  */

/** @defgroup FLASH_Error FLASH Error
  * @{
  */
#define HAL_FLASH_ERROR_NONE            0x00000000U
#define HAL_FLASH_ERROR_WRP             FLASH_FLAG_WRPERR
#define HAL_FLASH_ERROR_OPTV            FLASH_FLAG_OPTVERR
/**
  * @}
  */

/** @defgroup FLASH_Option_Bytes_Write_Protection FLASH Option Bytes Write Protection
  * @{
  */
#define OB_WRP_BLOCK_0               FLASH_WRPR_WRP_0  /* Write protection of Block0 */
#define OB_WRP_BLOCK_1               FLASH_WRPR_WRP_1  /* Write protection of Block1 */
#define OB_WRP_BLOCK_2               FLASH_WRPR_WRP_2  /* Write protection of Block2 */
#define OB_WRP_BLOCK_3               FLASH_WRPR_WRP_3  /* Write protection of Block3 */
#define OB_WRP_BLOCK_4               FLASH_WRPR_WRP_4  /* Write protection of Block4 */
#define OB_WRP_BLOCK_5               FLASH_WRPR_WRP_5  /* Write protection of Block5 */
#define OB_WRP_BLOCK_6               FLASH_WRPR_WRP_6  /* Write protection of Block6 */
#define OB_WRP_BLOCK_7               FLASH_WRPR_WRP_7  /* Write protection of Block7 */
#define OB_WRP_BLOCK_8               FLASH_WRPR_WRP_8  /* Write protection of Block8 */
#define OB_WRP_BLOCK_9               FLASH_WRPR_WRP_9  /* Write protection of Block9 */
#define OB_WRP_BLOCK_10              FLASH_WRPR_WRP_10 /* Write protection of Block10 */
#define OB_WRP_BLOCK_11              FLASH_WRPR_WRP_11 /* Write protection of Block11 */

#define OB_WRP_Pages0to127           OB_WRP_BLOCK_0    /* Write protection of Block0 */
#define OB_WRP_Pages128to255         OB_WRP_BLOCK_1    /* Write protection of Block1 */
#define OB_WRP_Pages256to383         OB_WRP_BLOCK_2    /* Write protection of Block2 */
#define OB_WRP_Pages384to511         OB_WRP_BLOCK_3    /* Write protection of Block3 */
#define OB_WRP_Pages512to639         OB_WRP_BLOCK_4    /* Write protection of Block4 */
#define OB_WRP_Pages640to767         OB_WRP_BLOCK_5    /* Write protection of Block5 */
#define OB_WRP_Pages768to895         OB_WRP_BLOCK_6    /* Write protection of Block6 */
#define OB_WRP_Pages896to1023        OB_WRP_BLOCK_7    /* Write protection of Block7 */
#define OB_WRP_Pages1024to1151       OB_WRP_BLOCK_8    /* Write protection of Block8 */
#define OB_WRP_Pages1152to1279       OB_WRP_BLOCK_9    /* Write protection of Block9 */
#define OB_WRP_Pages1280to1407       OB_WRP_BLOCK_10   /* Write protection of Block10 */
#define OB_WRP_Pages1408to1535       OB_WRP_BLOCK_11   /* Write protection of Block11 */
#define OB_WRP_AllPages              FLASH_WRPR_WRP    /* Write protection of all Blocks */
/**
  * @}
  */
  
/** @defgroup FLASH_OB_Read_Protection FLASH Option Bytes Read Protection
  * @{
  */
#define OB_RDP_LEVEL_0         ((uint8_t)0xAAU)
#define OB_RDP_LEVEL_1         ((uint8_t)0x55U)
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_Type FLASH User Option Type
  * @{
  */
#define OB_USER_IWDG_SW           FLASH_OPTR_IWDG_SW
#define OB_USER_NRST_STOP         FLASH_OPTR_NRST_STOP
#define OB_USER_NRST_STDBY        FLASH_OPTR_NRST_STDBY
#define OB_USER_ALL               (OB_USER_NRST_STOP  | OB_USER_NRST_STDBY   | OB_USER_IWDG_SW)                         
/**
  * @}
  */

/** @defgroup FLASH_Type_Program FLASH type program
  * @{
  */
#define FLASH_TYPEPROGRAM_PAGE       (0x01U)  /*!<Program 256bytes at a specified address.*/  
/**
  * @}
  */

/** @defgroup FLASH_Option_Type FLASH Option Type
  * @{
  */
#define OPTIONBYTE_WRP            ((uint32_t)0x01U)  /*!<WRP option byte configuration*/
#define OPTIONBYTE_RDP            ((uint32_t)0x04U)  /*!<RDP option byte configuration*/
#define OPTIONBYTE_USER           ((uint32_t)0x08U)  /*!<USER option byte configuration*/

#define OPTIONBYTE_ALL            (OPTIONBYTE_WRP  | OPTIONBYTE_RDP  | OPTIONBYTE_USER)                                                                
/**
  * @}
  */

/** @defgroup FLASH_WRP_State FLASH WRP State
  * @{
  */
#define OB_WRPSTATE_DISABLE        ((uint32_t)0x00U)  /*!<Disable the write protection of the desired blocks*/
#define OB_WRPSTATE_ENABLE         ((uint32_t)0x01U)  /*!<Enable the write protection of the desired blocks*/
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_IWDG_SW FLASH Option Bytes IWatchdog
  * @{
  */
#define OB_IWDG_SW                     FLASH_OPTR_IWDG_SW  /*!< Software IWDG selected */
#define OB_IWDG_HW                     ((uint8_t)0x0000U)  /*!< Hardware IWDG selected */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_NRST_STDBY FLASH Option Bytes NRST_STDBY
  * @{
  */
#define OB_STDBY_RST                   ((uint8_t)0x0000U)     /*!< Reset generated when entering Standby mode */
#define OB_STDBY_NORST                 FLASH_OPTR_NRST_STDBY  /*!< No reset generated when entering Standby mode */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_NRST_STOP FLASH Option Bytes NRST_STOP
  * @{
  */
#define OB_STOP_RST                    ((uint8_t)0x0000U)    /*!< Reset generated when entering Stop mode */
#define OB_STOP_NORST                  FLASH_OPTR_NRST_STOP  /*!< No reset generated when entering Stop mode*/
/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup FLASH_Exported_Macros FLASH Exported Macros
  *  @brief macros to control FLASH features
  *  @{
  */

/**
  * @brief  Set the FLASH Latency.
  * @param  __LATENCY__ FLASH Latency
  *         This parameter can be one of the following values :
  *     @arg @ref FLASH_LATENCY_0 FLASH Zero wait state
  *     @arg @ref FLASH_LATENCY_1 FLASH One wait state
  *     @arg @ref FLASH_LATENCY_3 FLASH Three wait states
  *     @arg @ref FLASH_LATENCY_4 FLASH Four wait states
  *     @arg @ref FLASH_LATENCY_5 FLASH Five wait states
  * @retval None
  */
#define __HAL_FLASH_SET_LATENCY(__LATENCY__)    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, (__LATENCY__))

/**
  * @brief  Get the FLASH Latency.
  * @retval FLASH Latency
  *         Returned value can be one of the following values :
  *     @arg @ref FLASH_LATENCY_0 FLASH Zero wait state
  *     @arg @ref FLASH_LATENCY_1 FLASH One wait state
  *     @arg @ref FLASH_LATENCY_3 FLASH Three wait states
  *     @arg @ref FLASH_LATENCY_4 FLASH Four wait states
  *     @arg @ref FLASH_LATENCY_5 FLASH Five wait states 
  */
#define __HAL_FLASH_GET_LATENCY()               READ_BIT(FLASH->ACR, FLASH_ACR_LATENCY)          

                                                                                                                                               
/** @defgroup FLASH_Interrupt FLASH Interrupts Macros
 *  @brief macros to handle FLASH interrupts
 * @{
 */

/**
  * @brief  Enable the specified FLASH interrupt.
  * @param  __INTERRUPT__ FLASH interrupt
  *         This parameter can be any combination of the following values:
  *     @arg @ref FLASH_IT_EOP End of FLASH Operation Interrupt
  *     @arg @ref FLASH_IT_OPERR Error Interrupt
  * @retval none
  */
#define __HAL_FLASH_ENABLE_IT(__INTERRUPT__)   SET_BIT(FLASH->CR, (__INTERRUPT__))

/**
  * @brief  Disable the specified FLASH interrupt.
  * @param  __INTERRUPT__ FLASH interrupt
  *         This parameter can be any combination of the following values:
  *     @arg @ref FLASH_IT_EOP End of FLASH Operation Interrupt
  *     @arg @ref FLASH_IT_OPERR Error Interrupt
  * @retval none
  */
#define __HAL_FLASH_DISABLE_IT(__INTERRUPT__)    CLEAR_BIT(FLASH->CR, (__INTERRUPT__))

/**
  * @brief  Check whether the specified FLASH flag is set or not.
  * @param  __FLAG__ specifies the FLASH flag to check.
  *   This parameter can be one of the following values:
  *     @arg @ref FLASH_FLAG_EOP FLASH End of Operation flag
  *     @arg @ref FLASH_FLAG_WRPERR FLASH Write protection error flag
  *     @arg @ref FLASH_FLAG_OPTVERR FLASH Option validity error flag
  *     @arg @ref FLASH_FLAG_BSY FLASH write/erase operations in progress flag
  *     @arg @ref FLASH_FLAG_ALL_ERRORS FLASH All errors flags
  * @retval The new state of FLASH_FLAG (SET or RESET).
  */
#define __HAL_FLASH_GET_FLAG(__FLAG__)                    (READ_BIT(FLASH->SR,   (__FLAG__)) == (__FLAG__))

/**
  * @brief  Clear the FLASHs pending flags.
  * @param  __FLAG__ specifies the FLASH flags to clear.
  *   This parameter can be any combination of the following values:
  *     @arg @ref FLASH_FLAG_EOP FLASH End of Operation flag
  *     @arg @ref FLASH_FLAG_WRPERR FLASH Write protection error flag
  *     @arg @ref FLASH_FLAG_OPTVERR FLASH Option validity error flag
  *     @arg @ref FLASH_FLAG_ALL_ERRORS FLASH All errors flags
  * @retval None
  */
#define __HAL_FLASH_CLEAR_FLAG(__FLAG__)        do {  WRITE_REG(FLASH->SR, (__FLAG__)); \
                                                   } while(0U)

/**
  * @}
  */

/**
  * @}
  */
/* Include FLASH HAL Extended module */
/* Exported variables --------------------------------------------------------*/
/** @defgroup FLASH_Exported_Variables FLASH Exported Variables
  * @{
  */
                 

extern FLASH_ProcessTypeDef pFlash;
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup FLASH_Exported_Functions
  * @{
  */

/* Program operation functions  ***********************************************/
/** @addtogroup FLASH_Exported_Functions_Group1
  * @{
  */
HAL_StatusTypeDef  HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint32_t *DataAddr );
HAL_StatusTypeDef  HAL_FLASH_Program_IT(uint32_t TypeProgram, uint32_t Address, uint32_t *DataAddr);
HAL_StatusTypeDef  HAL_FLASH_PageProgram(uint32_t Address, uint32_t *DataAddr );
HAL_StatusTypeDef  HAL_FLASH_PageProgram_IT(uint32_t Address, uint32_t *DataAddr);
/* FLASH IRQ handler method */
void               HAL_FLASH_IRQHandler(void);
/* Callbacks in non blocking modes */
void               HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue);
void               HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue);
HAL_StatusTypeDef  HAL_FLASH_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageError);
HAL_StatusTypeDef  HAL_FLASH_Erase_IT(FLASH_EraseInitTypeDef *pEraseInit);

#define HAL_FLASHEx_Erase        HAL_FLASH_Erase
#define HAL_FLASHEx_Erase_IT     HAL_FLASH_Erase_IT
/**
  * @}
  */

/* Peripheral Control functions  **********************************************/
/** @addtogroup FLASH_Exported_Functions_Group2
  * @{
  */
HAL_StatusTypeDef  HAL_FLASH_Unlock(void);
HAL_StatusTypeDef  HAL_FLASH_Lock(void);
/* Option bytes control */
HAL_StatusTypeDef  HAL_FLASH_OB_Unlock(void);
HAL_StatusTypeDef  HAL_FLASH_OB_Lock(void);
HAL_StatusTypeDef  HAL_FLASH_OB_Launch(void);
HAL_StatusTypeDef  HAL_FLASH_OBProgram(FLASH_OBProgramInitTypeDef *pOBInit);
void               HAL_FLASH_OBGetConfig(FLASH_OBProgramInitTypeDef *pOBInit);
HAL_StatusTypeDef  HAL_FLASH_OB_RDP_LevelConfig(uint8_t ReadProtectLevel);


/**
  * @}
  */

/* Peripheral State functions  ************************************************/
/** @addtogroup FLASH_Exported_Functions_Group3
  * @{
  */
uint32_t HAL_FLASH_GetError(void);
/**
  * @}
  */

/**
  * @}
  */

/* Private types --------------------------------------------------------*/
/** @defgroup FLASH_Private_types FLASH Private Types
  * @{
  */
HAL_StatusTypeDef  FLASH_WaitForLastOperation(uint32_t Timeout);

  
/**
  * @}
  */

/* Private constants --------------------------------------------------------*/
/** @defgroup FLASH_Private_Constants FLASH Private Constants
  * @{
  */
#define FLASH_TIMEOUT_VALUE             1000U          /*!< FLASH Execution Timeout, 1 s */

#define FLASH_TYPENONE                  0x00000000U    /*!< No Programmation Procedure On Going */

#define FLASH_FLAG_SR_ERROR             (FLASH_FLAG_OPTVERR  | FLASH_FLAG_WRPERR)     /*!< All SR error flags */

#define FLASH_FLAG_SR_CLEAR             (FLASH_FLAG_SR_ERROR | FLASH_SR_EOP)
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup FLASH_Private_Macros FLASH Private Macros
 *  @{
 */
#define IS_FLASH_MAIN_MEM_ADDRESS(__ADDRESS__)         (((__ADDRESS__) >= (FLASH_BASE)) && ((__ADDRESS__) <= (FLASH_BASE + FLASH_SIZE - 1UL)))

#define IS_FLASH_PROGRAM_MAIN_MEM_ADDRESS(__ADDRESS__) (((__ADDRESS__) >= (FLASH_BASE)) && ((__ADDRESS__) <= (FLASH_BASE + FLASH_SIZE - 8UL)))

#define IS_FLASH_PROGRAM_ADDRESS(__ADDRESS__)          (IS_FLASH_PROGRAM_MAIN_MEM_ADDRESS(__ADDRESS__))

#define IS_FLASH_NB_PAGES(__ADDRESS__, __VALUE__)      (((__ADDRESS__) >= (FLASH_BASE)) && ((__ADDRESS__ + (__VALUE__*FLASH_PAGE_SIZE)) <= (FLASH_BASE + FLASH_SIZE - 1UL))&&(__VALUE__ >0 ))

#define IS_FLASH_NB_SECTORS(__ADDRESS__, __VALUE__)    (((__ADDRESS__) >= (FLASH_BASE)) && ((__ADDRESS__ + (__VALUE__*FLASH_SECTOR_SIZE)) <= (FLASH_BASE + FLASH_SIZE - 1UL))&&(__VALUE__>0 ))

#define IS_FLASH_NB_BLOCKS(__ADDRESS__, __VALUE__)    (((__ADDRESS__) >= (FLASH_BASE)) && ((__ADDRESS__ + (__VALUE__*FLASH_BLOCK_SIZE)) <= (FLASH_BASE + FLASH_SIZE - 1UL)) &&(__VALUE__ >0 ))

#define IS_FLASH_FAST_PROGRAM_ADDRESS(__ADDRESS__)     (((__ADDRESS__) >= (FLASH_BASE)) && ((__ADDRESS__) <= (FLASH_BASE + FLASH_SIZE - 256UL)))

#define IS_FLASH_PAGE(__PAGE__)                        ((__PAGE__) < FLASH_PAGE_NB)

#define IS_FLASH_BANK(__BANK__)                        ((__BANK__) == 0x00UL)

#define IS_FLASH_TYPEERASE(__VALUE__)                  (((__VALUE__) == FLASH_TYPEERASE_PAGEERASE) || \
                                                        ((__VALUE__) == FLASH_TYPEERASE_SECTORERASE) || \
                                                        ((__VALUE__) == FLASH_TYPEERASE_BLOCKERASE) || \
                                                        ((__VALUE__) == FLASH_TYPEERASE_MASSERASE))

#define IS_FLASH_TYPEPROGRAM(__VALUE__)                ((__VALUE__) == FLASH_TYPEPROGRAM_PAGE)

#define IS_OPTIONBYTE(__VALUE__)                       ((((__VALUE__) & OPTIONBYTE_ALL) != 0x00U) && \
                                                       (((__VALUE__) & ~OPTIONBYTE_ALL) == 0x00U))

#define IS_OB_RDP_LEVEL(__LEVEL__)                     (((__LEVEL__) == OB_RDP_LEVEL_0)   ||\
                                                        ((__LEVEL__) == OB_RDP_LEVEL_1))

#define IS_OB_USER_TYPE(__TYPE__)                      ((((__TYPE__) & OB_USER_ALL) != 0x00U) && \
                                                        (((__TYPE__) & ~OB_USER_ALL) == 0x00U))

#define IS_OB_USER_CONFIG(__TYPE__,__CONFIG__)         ((~(__TYPE__) & (__CONFIG__)) == 0x00U)

#if defined(FLASH_PCROP_SUPPORT)
#define IS_OB_PCROP_CONFIG(__CONFIG__)                 (((__CONFIG__) & ~(OB_PCROP_ZONE_A | OB_PCROP_ZONE_B | OB_PCROP_RDP_ERASE)) == 0x00U)
#endif

#if defined(FLASH_SECURABLE_MEMORY_SUPPORT)
#define IS_OB_SEC_BOOT_LOCK(__VALUE__)                 (((__VALUE__) == OB_BOOT_ENTRY_FORCED_NONE) || ((__VALUE__) == OB_BOOT_ENTRY_FORCED_FLASH))

#define IS_OB_SEC_SIZE(__VALUE__)                      ((__VALUE__) < (FLASH_PAGE_NB + 1U))
#endif

#define IS_FLASH_LATENCY(__LATENCY__)                  (((__LATENCY__) == FLASH_LATENCY_0) || \
                                                        ((__LATENCY__) == FLASH_LATENCY_1) || \
                                                        ((__LATENCY__) == FLASH_LATENCY_3) || \
                                                        ((__LATENCY__) == FLASH_LATENCY_4) || \
                                                        ((__LATENCY__) == FLASH_LATENCY_5))

#define IS_WRPSTATE(__VALUE__)                         (((__VALUE__) == OB_WRPSTATE_DISABLE) || \
                                                        ((__VALUE__) == OB_WRPSTATE_ENABLE))

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

#endif /* __PY32F4xx_HAL_FLASH_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
