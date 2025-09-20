/**
  ******************************************************************************
  * @file    py32f4xx_hal_gpio_ex.h
  * @author  MCU Application Team
  * @brief   Header file of GPIO HAL Extended module.
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
#ifndef __PY32F4XX_HAL_GPIO_EX_H
#define __PY32F4XX_HAL_GPIO_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f4xx_hal_def.h"

/** @addtogroup PY32F4xx_HAL_Driver
  * @{
  */

/** @defgroup GPIOEx GPIOEx
  * @brief GPIO Extended HAL module driver
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/** @defgroup GPIOEx_Exported_Constants GPIOEx Exported Constants
  * @{
  */

/** @defgroup GPIOEx_Alternate_function_selection GPIOEx Alternate function selection
  * @{
  */





/*------------------------- PY32F403 ------------------------*/
/**
  * @brief   AF 0 selection
  */
#define GPIO_AF0_SWJ           ((uint8_t)0x00)  /*!< SWJ (SWD/JTAG) Alternate Function mapping */
#define GPIO_AF0_MCO           ((uint8_t)0x00)  /*!< MCO Alternate Function mapping */

/**
  * @brief   AF 1 selection
  */
#define GPIO_AF1_I2C1          ((uint8_t)0x01)  /*!< I2C1 Alternate Function mapping */
#define GPIO_AF1_I2C2          ((uint8_t)0x01)  /*!< I2C2 Alternate Function mapping */
#define GPIO_AF1_USART4        ((uint8_t)0x01)  /*!< USART4 Alternate Function mapping */
#define GPIO_AF1_USART5        ((uint8_t)0x01)  /*!< USART5 Alternate Function mapping */

/**
  * @brief   AF 2 selection
  */
#define GPIO_AF2_USART1        ((uint8_t)0x02)  /*!< USART1 Alternate Function mapping */
#define GPIO_AF2_USART2        ((uint8_t)0x02)  /*!< USART2 Alternate Function mapping */
#define GPIO_AF2_USART3        ((uint8_t)0x02)  /*!< USART3 Alternate Function mapping */
#define GPIO_AF2_SPI3          ((uint8_t)0x02)  /*!< SPI3 Alternate Function mapping */

/**
  * @brief   AF 3 selection
  */
#define GPIO_AF3_SPI1          ((uint8_t)0x03U)  /*!< SPI1 Alternate Function mapping */
#define GPIO_AF3_SPI2          ((uint8_t)0x03U)  /*!< SPI2 Alternate Function mapping */

/**
  * @brief   AF 4 selection
  */

#define GPIO_AF4_TIM1          ((uint8_t)0x04)  /*!< TIM1 Alternate Function mapping */

/**
  * @brief   AF 5 selection
  */
#define GPIO_AF5_TIM8          ((uint8_t)0x05)  /*!< TIM8 Alternate Function mapping */

/**
  * @brief   AF 6 selection
  */
#define GPIO_AF6_TIM2          ((uint8_t)0x06)  /*!< TIM2 Alternate Function mapping */
#define GPIO_AF6_TIM3          ((uint8_t)0x06)  /*!< TIM3 Alternate Function mapping */
#define GPIO_AF6_TIM4          ((uint8_t)0x06)  /*!< TIM4 Alternate Function mapping */

/**
  * @brief   AF 7 selection
  */
#define GPIO_AF7_TIM5          ((uint8_t)0x07)  /*!< TIM5 Alternate Function mapping */

/**
  * @brief   AF 8 selection
  */
#define GPIO_AF8_TIM9          ((uint8_t)0x08U)  /*!< TIM9  Alternate Function mapping */
#define GPIO_AF8_TIM10         ((uint8_t)0x08U)  /*!< TIM10 Alternate Function mapping */
#define GPIO_AF8_TIM11         ((uint8_t)0x08U)  /*!< TIM11 Alternate Function mapping */
#define GPIO_AF8_TIM12         ((uint8_t)0x08U)  /*!< TIM12 Alternate Function mapping */
#define GPIO_AF8_TIM13         ((uint8_t)0x08U)  /*!< TIM13 Alternate Function mapping */
#define GPIO_AF8_TIM14         ((uint8_t)0x08U)  /*!< TIM14 Alternate Function mapping */
/**
  * @brief   AF 9 selection
  */

/**
  * @brief   AF 10 selection
  */
#define GPIO_AF10_ESMC         ((uint8_t)0x0A)  /*!< ESMC Alternate Function mapping */
#define GPIO_AF10_CAN          ((uint8_t)0x0A)  /*!< CAN  Alternate Function mapping */
/**
  * @brief   AF 11 selection
  */
#define GPIO_AF11_SDIO         ((uint8_t)0x0B)  /*!< SDIO Alternate Function mapping */
/**
  * @brief   AF 12 selection
  */
  
/**
  * @brief   AF 13 selection
  */
  
/**
  * @brief   AF 14 selection
  */
#define GPIO_AF14_I2S1         ((uint8_t)0x0E)  /*!< I2S1 Alternate Function mapping */      
#define GPIO_AF14_I2S2         ((uint8_t)0x0E)  /*!< I2S2 Alternate Function mapping */
#define GPIO_AF14_I2S3         ((uint8_t)0x0E)  /*!< I2S3 Alternate Function mapping */

/**
  * @brief   AF 15 selection
  */
#define GPIO_AF15_EVENT        ((uint8_t)0x0fU) /*!< EVENT Alternate Function mapping */
#define IS_GPIO_AF(AF)         ((AF) <= (uint8_t)0x0f)
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup GPIOEx_Exported_Macros GPIOEx Exported Macros
  * @{
  */

/** @defgroup GPIOEx_Get_Port_Index GPIOEx Get Port Index
  * @{
  */
#define GPIO_GET_INDEX(__GPIOx__)    (((__GPIOx__) == (GPIOA))? 0UL :\
                                      ((__GPIOx__) == (GPIOB))? 1UL :\
                                      ((__GPIOx__) == (GPIOC))? 2UL :\
                                      ((__GPIOx__) == (GPIOD))? 3UL :\
                                      ((__GPIOx__) == (GPIOE))? 4UL : 7U)

/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __PY32F4xx_HAL_GPIO_EX_H */

/************************ (C) COPYRIGHT Puya Semiconductor *****END OF FILE****/
