/**
  ******************************************************************************
  * @file    py32f4xx_hal.h
  * @author  MCU Application Team
  * @brief   This file contains all the functions prototypes for the HAL
  *          module driver.
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
#ifndef __PY32F4xx_HAL_H
#define __PY32F4xx_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f4xx_hal_conf.h"

/** @addtogroup PY32F4xx_HAL_Driver
  * @{
  */

/** @addtogroup HAL
  * @{
  */

/* Exported constants --------------------------------------------------------*/

/** @defgroup HAL_Exported_Constants HAL Exported Constants
  * @{
  */

/** @defgroup HAL_TICK_FREQ Tick Frequency
  * @{
  */
typedef enum
{
  HAL_TICK_FREQ_10HZ         = 100U,
  HAL_TICK_FREQ_100HZ        = 10U,
  HAL_TICK_FREQ_1KHZ         = 1U,
  HAL_TICK_FREQ_DEFAULT      = HAL_TICK_FREQ_1KHZ
} HAL_TickFreqTypeDef;
/**
  * @}
  */
/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
/** @addtogroup HAL_Exported_Variables
  * @{
  */
extern uint32_t uwTickPrio;
extern HAL_TickFreqTypeDef uwTickFreq;
/**
  * @}
  */

/* SYSCFG Exported Constants -------------------------------------------------*/
/** @defgroup SYSCFG_Exported_Constants SYSCFG Exported Constants
  * @{
  */

/** @defgroup SYSCFG_BootMode Boot Mode
  * @{
  */
#define SYSCFG_BOOT_MAINFLASH          0x00000000U
#define SYSCFG_BOOT_SYSTEMFLASH        SYSCFG_CFGR1_MEM_MODE_0
#define SYSCFG_BOOT_ESMC               SYSCFG_CFGR1_MEM_MODE_1
#define SYSCFG_BOOT_SRAM               (SYSCFG_CFGR1_MEM_MODE_1 | SYSCFG_CFGR1_MEM_MODE_0)
/**
  * @}
  */

/** @defgroup SYSCFG_I2C_ANF I2C Analog Filter
  * @{
  */
#define HAL_SYSCFG_I2C_ANF_PB5            SYSCFG_CFGR1_I2C_PB5
#define HAL_SYSCFG_I2C_ANF_PB6            SYSCFG_CFGR1_I2C_PB6
#define HAL_SYSCFG_I2C_ANF_PB7            SYSCFG_CFGR1_I2C_PB7
#define HAL_SYSCFG_I2C_ANF_PB8            SYSCFG_CFGR1_I2C_PB8
#define HAL_SYSCFG_I2C_ANF_PB9            SYSCFG_CFGR1_I2C_PB9
#define HAL_SYSCFG_I2C_ANF_PB10           SYSCFG_CFGR1_I2C_PB10
#define HAL_SYSCFG_I2C_ANF_PB11           SYSCFG_CFGR1_I2C_PB11
#define HAL_SYSCFG_I2C_ANF_PB12           SYSCFG_CFGR1_I2C_PB12
/**
  * @}
  */

/** @defgroup SYSCFG_GPIO_ENA GPIO Enable Analog Mode
  * @{
  */
#define HAL_SYSCFG_GPIO_ENA_PA0            SYSCFG_GPIOENA_PA_ENA_0
#define HAL_SYSCFG_GPIO_ENA_PA1            SYSCFG_GPIOENA_PA_ENA_1
#define HAL_SYSCFG_GPIO_ENA_PA2            SYSCFG_GPIOENA_PA_ENA_2
#define HAL_SYSCFG_GPIO_ENA_PA3            SYSCFG_GPIOENA_PA_ENA_3
#define HAL_SYSCFG_GPIO_ENA_PA4            SYSCFG_GPIOENA_PA_ENA_4
#define HAL_SYSCFG_GPIO_ENA_PA5            SYSCFG_GPIOENA_PA_ENA_5
#define HAL_SYSCFG_GPIO_ENA_PA6            SYSCFG_GPIOENA_PA_ENA_6
#define HAL_SYSCFG_GPIO_ENA_PA7            SYSCFG_GPIOENA_PA_ENA_7
#define HAL_SYSCFG_GPIO_ENA_PB0            SYSCFG_GPIOENA_PB_ENA_0
#define HAL_SYSCFG_GPIO_ENA_PB1            SYSCFG_GPIOENA_PB_ENA_1
#define HAL_SYSCFG_GPIO_ENA_PC0            SYSCFG_GPIOENA_PC_ENA_0
#define HAL_SYSCFG_GPIO_ENA_PC1            SYSCFG_GPIOENA_PC_ENA_1
#define HAL_SYSCFG_GPIO_ENA_PC2            SYSCFG_GPIOENA_PC_ENA_2
#define HAL_SYSCFG_GPIO_ENA_PC3            SYSCFG_GPIOENA_PC_ENA_3
#define HAL_SYSCFG_GPIO_ENA_PC4            SYSCFG_GPIOENA_PC_ENA_4
#define HAL_SYSCFG_GPIO_ENA_PC5            SYSCFG_GPIOENA_PC_ENA_5
/**
  * @}
  */

/** @defgroup SYSCFG_TIMER_CLOCK_2X Timer clock 2X pclk
  * @{
  */
#define HAL_SYSCFG_TIM2_3_4_5_6_7_12_13_14_SEL    SYSCFG_TIM_PCLK1_SEL                  /*!< timer clock 2X APB1 clock */
#define HAL_SYSCFG_TIM1_8_9_10_11_SEL             SYSCFG_TIM_PCLK2_SEL                  /*!< timer clock 2X APB2 clock */
/**
  * @}
  */

/** @defgroup DBGMCU_TRACE DBGMCU TRACE Pin Assignment
  * @{
  */
#define HAL_DBGMCU_TRACE_NONE               0x00000000U                                     /*!< TRACE pins not assigned (default state) */
#define HAL_DBGMCU_TRACE_ASYNCH             DBGMCU_CR_TRACE_IOEN                            /*!< TRACE pin assignment for Asynchronous Mode */
#define HAL_DBGMCU_TRACE_SYNCH_SIZE1        (DBGMCU_CR_TRACE_IOEN | DBGMCU_CR_TRACE_MODE_0) /*!< TRACE pin assignment for Synchronous Mode with a TRACEDATA size of 1 */
#define HAL_DBGMCU_TRACE_SYNCH_SIZE2        (DBGMCU_CR_TRACE_IOEN | DBGMCU_CR_TRACE_MODE_1) /*!< TRACE pin assignment for Synchronous Mode with a TRACEDATA size of 2 */
#define HAL_DBGMCU_TRACE_SYNCH_SIZE4        (DBGMCU_CR_TRACE_IOEN | DBGMCU_CR_TRACE_MODE)   /*!< TRACE pin assignment for Synchronous Mode with a TRACEDATA size of 4 */
/**
  * @}
  */
/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup HAL_Exported_Macros HAL Exported Macros
  * @{
  */

/** @defgroup DBGMCU_Freeze_Unfreeze Freeze Unfreeze Peripherals in Debug mode
  * @brief   Freeze/Unfreeze Peripherals in Debug mode
  * Note: On devices PY32F4xx
  *       Debug registers DBGMCU_IDCODE and DBGMCU_CR are accessible only in
  *       debug mode (not accessible by the user software in normal mode).
  *       Refer to errata sheet of these devices for more details.
  * @{
  */

/**
  * @brief  IWDG Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_IWDG_STOP)
#define __HAL_DBGMCU_FREEZE_IWDG()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_IWDG_STOP)
#define __HAL_DBGMCU_UNFREEZE_IWDG()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_IWDG_STOP)
#endif

/**
  * @brief  WWDG Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_WWDG_STOP)
#define __HAL_DBGMCU_FREEZE_WWDG()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_WWDG_STOP)
#define __HAL_DBGMCU_UNFREEZE_WWDG()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_WWDG_STOP)
#endif

/**
  * @brief  TIM1 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM1_STOP)
#define __HAL_DBGMCU_FREEZE_TIM1()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM1_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM1()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM1_STOP)
#endif

/**
  * @brief  TIM2 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM2_STOP)
#define __HAL_DBGMCU_FREEZE_TIM2()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM2_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM2()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM2_STOP)
#endif

/**
  * @brief  TIM3 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM3_STOP)
#define __HAL_DBGMCU_FREEZE_TIM3()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM3_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM3()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM3_STOP)
#endif

/**
  * @brief  TIM4 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM4_STOP)
#define __HAL_DBGMCU_FREEZE_TIM4()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM4_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM4()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM4_STOP)
#endif

/**
  * @brief  CAN Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_CAN_STOP)
#define __HAL_DBGMCU_FREEZE_CAN()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_CAN_STOP)
#define __HAL_DBGMCU_UNFREEZE_CAN()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_CAN_STOP)
#endif

/**
  * @brief  I2C1 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_I2C1_SMBUS_TIMEOUT)
#define __HAL_DBGMCU_FREEZE_I2C1_TIMEOUT()    SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_I2C1_SMBUS_TIMEOUT)
#define __HAL_DBGMCU_UNFREEZE_I2C1_TIMEOUT()  CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_I2C1_SMBUS_TIMEOUT)
#endif

/**
  * @brief  I2C2 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_I2C2_SMBUS_TIMEOUT)
#define __HAL_DBGMCU_FREEZE_I2C2_TIMEOUT()    SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_I2C2_SMBUS_TIMEOUT)
#define __HAL_DBGMCU_UNFREEZE_I2C2_TIMEOUT()  CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_I2C2_SMBUS_TIMEOUT)
#endif

/**
  * @brief  TIM8 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM8_STOP)
#define __HAL_DBGMCU_FREEZE_TIM8()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM8_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM8()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM8_STOP)
#endif

/**
  * @brief  TIM5 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM5_STOP)
#define __HAL_DBGMCU_FREEZE_TIM5()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM5_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM5()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM5_STOP)
#endif

/**
  * @brief  TIM6 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM6_STOP)
#define __HAL_DBGMCU_FREEZE_TIM6()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM6_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM6()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM6_STOP)
#endif

/**
  * @brief  TIM7 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM7_STOP)
#define __HAL_DBGMCU_FREEZE_TIM7()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM7_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM7()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM7_STOP)
#endif

/**
  * @brief  TIM12 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM12_STOP)
#define __HAL_DBGMCU_FREEZE_TIM12()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM12_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM12()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM12_STOP)
#endif

/**
  * @brief  TIM13 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM13_STOP)
#define __HAL_DBGMCU_FREEZE_TIM13()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM13_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM13()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM13_STOP)
#endif

/**
  * @brief  TIM14 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM14_STOP)
#define __HAL_DBGMCU_FREEZE_TIM14()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM14_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM14()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM14_STOP)
#endif

/**
  * @brief  TIM9 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM9_STOP)
#define __HAL_DBGMCU_FREEZE_TIM9()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM9_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM9()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM9_STOP)
#endif

/**
  * @brief  TIM10 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM10_STOP)
#define __HAL_DBGMCU_FREEZE_TIM10()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM10_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM10()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM10_STOP)
#endif

/**
  * @brief  TIM11 Peripherals Debug mode
  */
#if defined (DBGMCU_CR_DBG_TIM11_STOP)
#define __HAL_DBGMCU_FREEZE_TIM11()            SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM11_STOP)
#define __HAL_DBGMCU_UNFREEZE_TIM11()          CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_TIM11_STOP)
#endif

/**
  * @}
  */

/** @defgroup SYSCFG_Exported_Macros SYSCFG Exported Macros
  * @{
  */

/** @brief  SYSCFG Break  Lockup lock.
  *         Enables and locks the connection of  LOCKUP (Hardfault) output to TIM1/8 Break input
  * @note   The selected configuration is locked and can be unlocked only by system reset.
  */
#define __HAL_SYSCFG_BREAK_LOCKUP_LOCK()        SET_BIT(SYSCFG->CFGR[1], SYSCFG_CFGR2_LOCKUP_LOCK)

/** @brief  SYSCFG Break PVD lock.
  *         Enables and locks the PVD connection with Timer1 Break input
  * @note   The selected configuration is locked and can be unlocked only by system reset
  */
#define __HAL_SYSCFG_BREAK_PVD_LOCK()           SET_BIT(SYSCFG->CFGR[1], SYSCFG_CFGR2_PVD_LOCK)

/**
  * @brief Enable the remapping of ADC1_ETRGINJ (ADC 1 External trigger injected conversion).
  * @note  ENABLE: ADC1 External Event injected conversion is connected to TIM8 Channel4.
  * @retval None
  */
#define __HAL_AFIO_REMAP_ADC1_ETRGINJ_ENABLE()           SET_BIT(SYSCFG->CFGR[1], SYSCFG_CFGR2_ADC1_ETRGINJ_REMAP)

/**
  * @brief Disable the remapping of ADC1_ETRGINJ (ADC 1 External trigger injected conversion).
  * @note  DISABLE: ADC1 External trigger injected conversion is connected to EXTI15
  * @retval None
  */
#define __HAL_AFIO_REMAP_ADC1_ETRGINJ_DISABLE()           CLEAR_BIT(SYSCFG->CFGR[1], SYSCFG_CFGR2_ADC1_ETRGINJ_REMAP)

/**
  * @brief Enable the remapping of ADC1_ETRGREG (ADC 1 External trigger regular conversion).
  * @note  ENABLE: ADC1 External Event regular conversion is connected to TIM8 TRGO.
  * @retval None
  */
#define __HAL_AFIO_REMAP_ADC1_ETRGREG_ENABLE()           SET_BIT(SYSCFG->CFGR[1], SYSCFG_CFGR2_ADC1_ETRGREG_REMAP)

/**
  * @brief Disable the remapping of ADC1_ETRGREG (ADC 1 External trigger regular conversion).
  * @note  DISABLE: ADC1 External trigger regular conversion is connected to EXTI11
  * @retval None
  */
#define __HAL_AFIO_REMAP_ADC1_ETRGREG_DISABLE()           CLEAR_BIT(SYSCFG->CFGR[1], SYSCFG_CFGR2_ADC1_ETRGREG_REMAP)

/**
  * @brief Enable the remapping of ADC2_ETRGINJ (ADC 2 External trigger injected conversion).
  * @note  ENABLE: ADC2 External Event injected conversion is connected to TIM8 Channel4.
  * @retval None
  */
#define __HAL_AFIO_REMAP_ADC2_ETRGINJ_ENABLE()           SET_BIT(SYSCFG->CFGR[1], SYSCFG_CFGR2_ADC2_ETRGINJ_REMAP)

/**
  * @brief Disable the remapping of ADC2_ETRGINJ (ADC 2 External trigger injected conversion).
  * @note  DISABLE: ADC2 External trigger injected conversion is connected to EXTI15
  * @retval None
  */
#define __HAL_AFIO_REMAP_ADC2_ETRGINJ_DISABLE()           CLEAR_BIT(SYSCFG->CFGR[1], SYSCFG_CFGR2_ADC2_ETRGINJ_REMAP)

/**
  * @brief Enable the remapping of ADC2_ETRGREG (ADC 2 External trigger regular conversion).
  * @note  ENABLE: ADC2 External Event regular conversion is connected to TIM8 TRGO.
  * @retval None
  */
#define __HAL_AFIO_REMAP_ADC2_ETRGREG_ENABLE()           SET_BIT(SYSCFG->CFGR[1], SYSCFG_CFGR2_ADC2_ETRGREG_REMAP)
                 
/**
  * @brief Disable the remapping of ADC2_ETRGREG (ADC 2 External trigger regular conversion).
  * @note  DISABLE: ADC2 External trigger regular conversion is connected to EXTI11
  * @retval None
  */
#define __HAL_AFIO_REMAP_ADC2_ETRGREG_DISABLE()           CLEAR_BIT(SYSCFG->CFGR[1], SYSCFG_CFGR2_ADC2_ETRGREG_REMAP)

/**
  * @}
  */

/**
  * @}
  */
/* Private macros ------------------------------------------------------------*/
/** @defgroup HAL_Private_Macros HAL Private Macros
  * @{
  */
#define IS_TICKFREQ(FREQ) (((FREQ) == HAL_TICK_FREQ_10HZ)  || \
                           ((FREQ) == HAL_TICK_FREQ_100HZ) || \
                           ((FREQ) == HAL_TICK_FREQ_1KHZ))
                           
/**
  * @}
  */

/* SYSCFG Private macros -----------------------------------------------------*/
/** @defgroup SYSCFG_Private_Macros SYSCFG Private Macros
  * @{
  */
#define IS_SYSCFG_I2C_ANF_IO(IO) (((IO) == HAL_SYSCFG_I2C_ANF_PB5)  || \
                                  ((IO) == HAL_SYSCFG_I2C_ANF_PB6)  || \
                                  ((IO) == HAL_SYSCFG_I2C_ANF_PB7)  || \
                                  ((IO) == HAL_SYSCFG_I2C_ANF_PB8)  || \
                                  ((IO) == HAL_SYSCFG_I2C_ANF_PB9)  || \
                                  ((IO) == HAL_SYSCFG_I2C_ANF_PB10) || \
                                  ((IO) == HAL_SYSCFG_I2C_ANF_PB11) || \
                                  ((IO) == HAL_SYSCFG_I2C_ANF_PB12 ))
                                  
                                  
#define IS_SYSCFG_GPIO_ENA_IO(IO) (((IO) == HAL_SYSCFG_GPIO_ENA_PA0)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PA1)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PA2)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PA3)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PA4)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PA5)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PA6)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PA7)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PB0)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PB1)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PC0)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PC1)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PC2)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PC3)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PC4)  || \
                                   ((IO) == HAL_SYSCFG_GPIO_ENA_PC5 ))
                               
                                  
#define IS_SYSCFG_TIM_CLK_CFG(PCLKSEL)    (((PCLKSEL) == HAL_SYSCFG_TIM2_3_4_5_6_7_12_13_14_SEL)  || \
                                           ((PCLKSEL) == HAL_SYSCFG_TIM1_8_9_10_11_SEL ))
                                                                   
/**
  * @}
  */

/* DBGMCU Private macros -----------------------------------------------------*/
/** @defgroup DBGMCU_Private_Macros DBGMCU Private Macros
  * @{
  */
#define IS_DBGMCU_TracePinAssignment(PinAssignment) (((PinAssignment) == HAL_DBGMCU_TRACE_NONE)         || \
                                                     ((PinAssignment) == HAL_DBGMCU_TRACE_ASYNCH)       || \
                                                     ((PinAssignment) == HAL_DBGMCU_TRACE_SYNCH_SIZE1)  || \
                                                     ((PinAssignment) == HAL_DBGMCU_TRACE_SYNCH_SIZE2)  || \
                                                     ((PinAssignment) == HAL_DBGMCU_TRACE_SYNCH_SIZE4 ))



/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup HAL_Exported_Functions
  * @{
  */
/** @addtogroup HAL_Exported_Functions_Group1
  * @{
  */
/* Initialization and de-initialization functions  ******************************/
HAL_StatusTypeDef HAL_Init(void);
HAL_StatusTypeDef HAL_DeInit(void);
void HAL_MspInit(void);
void HAL_MspDeInit(void);
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority);
/**
  * @}
  */

/** @addtogroup HAL_Exported_Functions_Group2
  * @{
  */
/* Peripheral Control functions  ************************************************/
void HAL_IncTick(void);
void HAL_Delay(uint32_t Delay);
uint32_t HAL_GetTick(void);
uint32_t HAL_GetTickPrio(void);
HAL_StatusTypeDef HAL_SetTickFreq(HAL_TickFreqTypeDef Freq);
HAL_TickFreqTypeDef HAL_GetTickFreq(void);
void HAL_SuspendTick(void);
void HAL_ResumeTick(void);
uint32_t HAL_GetHalVersion(void);
uint32_t HAL_GetREVID(void);
uint32_t HAL_GetDEVID(void);
uint32_t HAL_GetUIDw0(void);
uint32_t HAL_GetUIDw1(void);
uint32_t HAL_GetUIDw2(void);
void HAL_DBGMCU_EnableDBGSleepMode(void);
void HAL_DBGMCU_DisableDBGSleepMode(void);
void HAL_DBGMCU_EnableDBGStopMode(void);
void HAL_DBGMCU_DisableDBGStopMode(void);
void HAL_DBGMCU_EnableDBGStandbyMode(void);
void HAL_DBGMCU_DisableDBGStandbyMode(void);
void HAL_DBGMCU_SetTracePinAssignment(uint32_t PinAssignment);
uint32_t HAL_DBGMCU_GetTracePinAssignment(void);
void HAL_SYSCFG_SetRemapMemory(uint32_t Memory);
uint32_t HAL_SYSCFG_GetRemapMemory(void);
void HAL_SYSCFG_EnableGPIOAnalogMode(uint32_t GPIOAnalogEnable);
void HAL_SYSCFG_DisableGPIOAnalogMode(uint32_t GPIOAnalogEnable);
void HAL_SYSCFG_EnableGPIONoiseFilter(GPIO_TypeDef *GPIOx,uint16_t GPIO_Pin);
void HAL_SYSCFG_DisableGPIONoiseFilter(GPIO_TypeDef *GPIOx,uint16_t GPIO_Pin);
void HAL_SYSCFG_EnableI2CAnalogFilter(uint32_t I2CAnalogFilter);
void HAL_SYSCFG_DisableI2CAnalogFilter(uint32_t I2CAnalogFilter);
void HAL_SYSCFG_EnableTimerClock2xPclk(uint32_t TimerSelect);
void HAL_SYSCFG_DisableTimerClock2xPclk(uint32_t TimerSelect);

/**
  * @}
  */

/**
  * @}
  */
/* Private types -------------------------------------------------------------*/

/* Private constants ---------------------------------------------------------*/
/** @defgroup HAL_Private_Constants HAL Private Constants
  * @{
  */
/**
  * @}
  */
/* Private macros ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __PY32F4xx_HAL_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
