/**
  ******************************************************************************
  * @file    py32f4xx_hal_pwr_ex.h
  * @author  MCU Application Team
  * @brief   Header file of PWR HAL Extension module.
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
#ifndef __PY32F4xx_HAL_PWR_EX_H
#define __PY32F4xx_HAL_PWR_EX_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f4xx_hal_def.h"

/** @addtogroup PY32F4xx_HAL_Driver
  * @{
  */

/** @addtogroup PWREx
  * @{
  */ 

/* Exported types ------------------------------------------------------------*/ 

/* Exported constants --------------------------------------------------------*/
/** @defgroup PWREx_Exported_Constants PWREx Exported Constants
  * @{
  */
/** @defgroup PWREx_REGULATOR_VOLTAGE_SCALE PWREx Regulator Voltage Scale
  * @{
  */
#define PWR_REGULATOR_VOLTAGE_SCALE1         0x00000000U            /* Scale 1 mode(default value at reset): 1.1v. */
#define PWR_REGULATOR_VOLTAGE_SCALE2         PWR_CR_VOS_0           /* Scale 2 mode: 1.0v. */
#define PWR_REGULATOR_VOLTAGE_SCALE3         PWR_CR_VOS_1           /* Scale 3 mode: 0.9v  */
#define PWR_REGULATOR_VOLTAGE_SCALE4         PWR_CR_VOS             /* Scale 4 mode: 0.8v  */
/**
  * @}
  */
 
/** @defgroup PWREx_BACKUP_VOLTAGE_SCALE PWREx BkUp Voltage Scale
  * @{
  */  
#define PWR_BACKUP_VOLTAGE_SCALE1            0x00000000U            /* Scale 1 mode(default value at reset): 1.1v. */
#define PWR_BACKUP_VOLTAGE_SCALE2            PWR_CR_BKPVR_VOS_0     /* Scale 2 mode: 0.9v. */
#define PWR_BACKUP_VOLTAGE_SCALE3            PWR_CR_BKPVR_VOS_1     /* Scale 3 mode: 0.85v. */
#define PWR_BACKUP_VOLTAGE_SCALE4            PWR_CR_BKPVR_VOS       /* Scale 4 mode: 0.8v. */
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup PWREx_Exported_Macros PWREx Exported Macros
  *  @{
  */

/** @brief  macros configure the main internal regulator output voltage.
  * @param  __REGULATOR__ specifies the regulator output voltage to achieve
  *         a tradeoff between performance and power consumption when the device does
  *         not operate at the maximum frequency (refer to the datasheets for more details).
  *          This parameter can be one of the following values:
  *            @arg PWR_REGULATOR_VOLTAGE_SCALE1: Regulator voltage output Scale 1 mode
  *            @arg PWR_REGULATOR_VOLTAGE_SCALE2: Regulator voltage output Scale 2 mode
  *            @arg PWR_REGULATOR_VOLTAGE_SCALE3: Regulator voltage output Scale 3 mode
  *            @arg PWR_REGULATOR_VOLTAGE_SCALE4: Regulator voltage output Scale 4 mode
  * @retval None
  */
#define __HAL_PWR_VOLTAGESCALING_CONFIG(__REGULATOR__) do {                                                     \
                                                            __IO uint32_t tmpreg = 0x00U;                        \
                                                            MODIFY_REG(PWR->CR, PWR_CR_VOS, (__REGULATOR__));   \
                                                            /* Delay after an RCC peripheral clock enabling */  \
                                                            tmpreg = READ_BIT(PWR->CR, PWR_CR_VOS);             \
                                                            UNUSED(tmpreg);                                     \
                                                          } while(0U)

                                                          

/** @brief  macros configure the backup ldo voltage.
  * @param  __REGULATOR__ specifies the backup ldo voltage to achieve
  *          This parameter can be one of the following values:
  *            @arg PWR_BACKUP_VOLTAGE_SCALE1:  voltage output Scale 1 mode
  *            @arg PWR_BACKUP_VOLTAGE_SCALE2:  voltage output Scale 2 mode
  *            @arg PWR_BACKUP_VOLTAGE_SCALE3:  voltage output Scale 3 mode
  *            @arg PWR_BACKUP_VOLTAGE_SCALE4:  voltage output Scale 4 mode
  * @retval None
  */
#define __HAL_PWR_BKUPVOLTAGESCALING_CONFIG(__REGULATOR__) do {                                                     \
                                                            __IO uint32_t tmpreg = 0x00U;                        \
                                                            MODIFY_REG(PWR->CR, PWR_CR_BKPVR_VOS, (__REGULATOR__));   \
                                                            /* Delay after an RCC peripheral clock enabling */  \
                                                            tmpreg = READ_BIT(PWR->CR, PWR_CR_BKPVR_VOS);             \
                                                            UNUSED(tmpreg);                                     \
                                                          } while(0U)
                                                          
                                                          
/**
  * @}
  */

/* Exported Functions --------------------------------------------------------*/
/** @defgroup PWREx_Exported_Functions PWREx Exported Functions
  *  @{
  */

/** @addtogroup PWREx_Exported_Functions_Group1
  * @{
  */
void HAL_PWREx_SetWakeupFlashDelay(uint32_t DelayTime);
uint32_t HAL_PWREx_GetWakeupFlashDelay(void); 
void HAL_PWREx_DisableHSIWakeupWait(void); 
uint32_t HAL_PWREx_GetVoltageRange(void);
void HAL_PWREx_ControlVoltageScaling(uint32_t VoltageScaling);

void HAL_PWREx_SetWakeupMrReadyDelay(uint32_t DelayTime);
uint32_t HAL_PWREx_GetWakeupMrReadyDelay(void);
void HAL_PWREx_ControlBkUpVoltageScaling(uint32_t VoltageScaling);                                                        
uint32_t HAL_PWREx_GetBkUpVoltageRange(void);                                                 
                                                          
/**
  * @}
  */
/**
  * @}
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @defgroup PWREx_Private_Constants PWREx Private Constants
  * @{
  */

/** @defgroup PWR_FLASH_WAKEUP_DELAY Flash wakeup delay time
  * @{
  */  
#define PWR_WAKEUP_FLASH_DELAY_3US  0x00000000U
#define PWR_WAKEUP_FLASH_DELAY_5US  PWR_CR_FLS_WUPT_0
#define PWR_WAKEUP_FLASH_DELAY_2US  PWR_CR_FLS_WUPT_1
#define PWR_WAKEUP_FLASH_DELAY_0US  (PWR_CR_FLS_WUPT_0 | PWR_CR_FLS_WUPT_1)

 /**
  * @}
  */

/** @defgroup PWR_MRREADY_WAKEUP_DELAY standby wakeup MR ready delay time
  * @{
  */  
#define PWR_WAKEUP_MRREADY_DELAY_5US   0x00000000U
#define PWR_WAKEUP_MRREADY_DELAY_10US  PWR_CR_STDBY_MRRDY_WAIT_0
#define PWR_WAKEUP_MRREADY_DELAY_20US  PWR_CR_STDBY_MRRDY_WAIT_1
#define PWR_WAKEUP_MRREADY_DELAY_30US  (PWR_CR_STDBY_MRRDY_WAIT_0 | PWR_CR_STDBY_MRRDY_WAIT_1)

 /**
  * @}
  */


/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup PWREx_Private_Macros PWREx Private Macros
  * @{
  */

/** @defgroup PWREx_IS_PWR_Definitions PWREx Private macros to check input parameters
  * @{
  */
#define IS_PWR_VOLTAGE_SCALING_RANGE(VOLTAGE) (((VOLTAGE) == PWR_REGULATOR_VOLTAGE_SCALE1) || \
                                               ((VOLTAGE) == PWR_REGULATOR_VOLTAGE_SCALE2) || \
                                               ((VOLTAGE) == PWR_REGULATOR_VOLTAGE_SCALE3) || \
                                               ((VOLTAGE) == PWR_REGULATOR_VOLTAGE_SCALE4))

#define IS_PWR_BKUPVOLTAGE_SCALING_RANGE(VOLTAGE) (((VOLTAGE) == PWR_BACKUP_VOLTAGE_SCALE1) || \
                                                   ((VOLTAGE) == PWR_BACKUP_VOLTAGE_SCALE2) || \
                                                   ((VOLTAGE) == PWR_BACKUP_VOLTAGE_SCALE3) || \
                                                   ((VOLTAGE) == PWR_BACKUP_VOLTAGE_SCALE4))

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


#endif /* __PY32F4xx_HAL_PWR_EX_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
