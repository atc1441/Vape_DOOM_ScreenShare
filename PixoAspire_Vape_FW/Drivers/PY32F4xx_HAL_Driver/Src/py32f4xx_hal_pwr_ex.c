/**
  ******************************************************************************
  * @file    py32f4xx_hal_pwr_ex.c
  * @author  MCU Application Team
  * @brief   Extended PWR HAL module driver.
  *          This file provides firmware functions to manage the following 
  *          functionalities of PWR extension peripheral:           
  *           + Peripheral Extended features functions
  *         
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

/** @defgroup PWREx PWREx
  * @brief PWR HAL module driver
  * @{
  */

#ifdef HAL_PWR_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/** @defgroup PWREx_Exported_Functions PWREx Exported Functions
  *  @{
  */

/** @defgroup PWREx_Exported_Functions_Group1 Peripheral Extended features functions 
  *  @brief Peripheral Extended features functions 
  *

  * @{
  */

/**
  * @brief Config wakeup from stop mode, enable flash delay time.
  * @param  DelayTime: Specifies the delay time before FLASH control
  *          This parameter can be one of the following values:
  *            - @arg PWR_WAKEUP_FLASH_DELAY_3US: Wake up from the STOP mode, Delay 3us enable flash
  *            - @arg PWR_WAKEUP_FLASH_DELAY_5US: Wake up from the STOP mode, Delay 5us enable flash
  *            - @arg PWR_WAKEUP_FLASH_DELAY_2US: Wake up from the STOP mode, Delay 2us enable flash
  *            - @arg PWR_WAKEUP_FLASH_DELAY_0US: Wake up from the STOP mode, Enable flash immediately
  * @retval None
  */
void HAL_PWREx_SetWakeupFlashDelay(uint32_t DelayTime)
{
  MODIFY_REG(PWR->CR, PWR_CR_FLS_WUPT, DelayTime);
}

/**
  * @brief Get wakeup from stop mode, enable flash delay time.
  * @retval Config Flash wakeup delay time.
  *            - @arg PWR_WAKEUP_FLASH_DELAY_3US: Wake up from the STOP mode, Delay 3us enable flash
  *            - @arg PWR_WAKEUP_FLASH_DELAY_5US: Wake up from the STOP mode, Delay 5us enable flash
  *            - @arg PWR_WAKEUP_FLASH_DELAY_2US: Wake up from the STOP mode, Delay 2us enable flash
  *            - @arg PWR_WAKEUP_FLASH_DELAY_0US: Wake up from the STOP mode, Enable flash immediately
  */
uint32_t HAL_PWREx_GetWakeupFlashDelay(void)
{
  return (PWR->CR & PWR_CR_FLS_WUPT);
}

/**
  * @brief Config wakeup from STANDBY mode MR Ready delay time.
  * @param  DelayTime: Specifies the delay time afte MR Ready.
  *          This parameter can be one of the following values:
  *            - @arg PWR_WAKEUP_MRREADY_DELAY_5US:   Wake up from the STANDY mode, MR Ready Delay 5us
  *            - @arg PWR_WAKEUP_MRREADY_DELAY_10US:  Wake up from the STANDY mode, MR Ready Delay 10us
  *            - @arg PWR_WAKEUP_MRREADY_DELAY_20US:  Wake up from the STANDY mode, MR Ready Delay 20us
  *            - @arg PWR_WAKEUP_MRREADY_DELAY_30US:  Wake up from the STANDY mode, MR Ready Delay 30us
  * @retval None
  */
void HAL_PWREx_SetWakeupMrReadyDelay(uint32_t DelayTime)        
{
  MODIFY_REG(PWR->CR, PWR_CR_STDBY_MRRDY_WAIT, DelayTime);
}

/**
  * @brief Get wakeup from STANDBY mode MR Ready delay time.
  * @retval Get STANDBY wakeup MR Ready delay time.
  *            - @arg PWR_WAKEUP_MRREADY_DELAY_5US:   Wake up from the STANDY mode, MR Ready Delay 5us
  *            - @arg PWR_WAKEUP_MRREADY_DELAY_10US:  Wake up from the STANDY mode, MR Ready Delay 10us
  *            - @arg PWR_WAKEUP_MRREADY_DELAY_20US:  Wake up from the STANDY mode, MR Ready Delay 20us
  *            - @arg PWR_WAKEUP_MRREADY_DELAY_30US:  Wake up from the STANDY mode, MR Ready Delay 30us
  */
uint32_t HAL_PWREx_GetWakeupMrReadyDelay(void)         
{
  return (PWR->CR & PWR_CR_STDBY_MRRDY_WAIT);
}

/**
  * @brief When wakeup from stop mode,HSI and Main Regulator start at the same time.
  * @retval None
  */
void HAL_PWREx_DisableHSIWakeupWait(void)                            
{
  SET_BIT(PWR->CR, PWR_CR_HSION_CTRL);                      
}

/**
  * @brief When wakeup from stop mode,HSI enable After Main Regulator stable.
  * @retval None
  */
void HAL_PWREx_EnableHSIWakeupWait(void)                            
{
  CLEAR_BIT(PWR->CR, PWR_CR_HSION_CTRL);                      
}

/**
  * @brief Return Voltage Scaling Range.
  * @retval The configured scale for the regulator voltage(VOS bit field).
  *         The returned value can be one of the following:
  *            - @arg PWR_REGULATOR_VOLTAGE_SCALE1: Regulator voltage output Scale 1 mode
  *            - @arg PWR_REGULATOR_VOLTAGE_SCALE2: Regulator voltage output Scale 2 mode
  *            - @arg PWR_REGULATOR_VOLTAGE_SCALE3: Regulator voltage output Scale 3 mode
  *            - @arg PWR_REGULATOR_VOLTAGE_SCALE4: Regulator voltage output Scale 4 mode
  */  
uint32_t HAL_PWREx_GetVoltageRange(void)
{
  return (PWR->CR & PWR_CR_VOS);
}

/**
  * @brief Configures the main internal regulator output voltage.
  * @param  VoltageScaling specifies the regulator output voltage to achieve
  *         a tradeoff between performance and power consumption.
  *          This parameter can be one of the following values:
  *            @arg PWR_REGULATOR_VOLTAGE_SCALE1: Regulator voltage output range 1 mode                                         
  *            @arg PWR_REGULATOR_VOLTAGE_SCALE2: Regulator voltage output range 2 mode
  *            @arg PWR_REGULATOR_VOLTAGE_SCALE3: Regulator voltage output range 3 mode                                         
  *            @arg PWR_REGULATOR_VOLTAGE_SCALE4: Regulator voltage output range 4 mode
  * @retval None
  */
void HAL_PWREx_ControlVoltageScaling(uint32_t VoltageScaling)
{  
  assert_param(IS_PWR_VOLTAGE_SCALING_RANGE(VoltageScaling));
  
  /* Enable PWR RCC Clock Peripheral */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* Set Range */
  __HAL_PWR_VOLTAGESCALING_CONFIG(VoltageScaling);
}

/**
  * @brief Configures the backup output voltage.
  * @param  VoltageScaling specifies the backup voltage.
  *          This parameter can be one of the following values:
  *            @arg PWR_BACKUP_VOLTAGE_SCALE1: Backup voltage output range 1 mode,
  *            @arg PWR_BACKUP_VOLTAGE_SCALE2: Backup voltage output range 2 mode,
  *            @arg PWR_BACKUP_VOLTAGE_SCALE3: Backup voltage output range 3 mode,
  *            @arg PWR_BACKUP_VOLTAGE_SCALE4: Backup voltage output range 4 mode,
  * @retval None
  */
void HAL_PWREx_ControlBkUpVoltageScaling(uint32_t VoltageScaling)       
{  
  assert_param(IS_PWR_BKUPVOLTAGE_SCALING_RANGE(VoltageScaling));
  
  /* Enable PWR RCC Clock Peripheral */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* Set Range */
  __HAL_PWR_BKUPVOLTAGESCALING_CONFIG(VoltageScaling);  
}


/**
  * @brief Return Backup Voltage Scaling Range.
  * @retval The configured scale for the Backup voltage.
  *         The returned value can be one of the following:
  *            - @arg PWR_BACKUP_VOLTAGE_SCALE1: Backup voltage output Scale 1 mode
  *            - @arg PWR_BACKUP_VOLTAGE_SCALE2: Backup voltage output Scale 2 mode
  *            - @arg PWR_BACKUP_VOLTAGE_SCALE3: Backup voltage output Scale 3 mode
  *            - @arg PWR_BACKUP_VOLTAGE_SCALE4: Backup voltage output Scale 4 mode
  */  
uint32_t HAL_PWREx_GetBkUpVoltageRange(void)        
{
  return (PWR->CR & PWR_CR_BKPVR_VOS);
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_PWR_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
