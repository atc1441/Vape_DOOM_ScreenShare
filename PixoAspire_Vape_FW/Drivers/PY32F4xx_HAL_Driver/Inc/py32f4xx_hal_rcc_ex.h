/**
  ******************************************************************************
  * @file    py32f4xx_hal_rcc_ex.h
  * @author  MCU Application Team
  * @brief   Header file of RCC HAL Extended module.
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
#ifndef __PY32F4XX_HAL_RCC_EX_H
#define __PY32F4XX_HAL_RCC_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f4xx_hal_def.h"

/** @addtogroup PY32F4xx_HAL_Driver
  * @{
  */

/** @addtogroup RCCEx
  * @{
  */

/* Private Constants ---------------------------------------------------------*/
/** @addtogroup RCCEx_Private_Constants
 * @{
 */

//#define CR_REG_INDEX                 ((uint8_t)1)

/**
  * @}
  */

/* Private Macros ------------------------------------------------------------*/
/** @addtogroup RCCEx_Private_Macros
 * @{
 */

#define IS_RCC_PLL_MUL(__MUL__) ((__MUL__) <= RCC_PLL_MUL63)

#define IS_RCC_MCO1SOURCE(__SOURCE__) (((__SOURCE__) == RCC_MCO1SOURCE_SYSCLK)  || ((__SOURCE__) == RCC_MCO1SOURCE_HSI) \
                                    || ((__SOURCE__) == RCC_MCO1SOURCE_HSE)     || ((__SOURCE__) == RCC_MCO1SOURCE_PLLCLK) \
                                    || ((__SOURCE__) == RCC_MCO1SOURCE_NOCLOCK) || ((__SOURCE__) == RCC_MCO1SOURCE_LSI) \
                                    || ((__SOURCE__) == RCC_MCO1SOURCE_LSE)     || ((__SOURCE__) == RCC_MCO1SOURCE_HSI48M))

#define IS_RCC_ADCPLLCLK_DIV(__ADCCLK__) (((__ADCCLK__) == RCC_ADCPCLK2_DIV2)  || ((__ADCCLK__) == RCC_ADCPCLK2_DIV4)   || \
                                          ((__ADCCLK__) == RCC_ADCPCLK2_DIV6)  || ((__ADCCLK__) == RCC_ADCPCLK2_DIV8)   || \
                                          ((__ADCCLK__) == RCC_ADCPCLK2_DIV12) || ((__ADCCLK__) == RCC_ADCPCLK2_DIV16))

#define IS_RCC_USBCLKSOURCE(__USBCLK__)  (((__USBCLK__) == RCC_USBCLKSOURCE_PLL)      || ((__USBCLK__) == RCC_USBCLKSOURCE_PLL_DIV1_5)   || \
                                          ((__USBCLK__) == RCC_USBCLKSOURCE_PLL_DIV2) || ((__USBCLK__) == RCC_USBCLKSOURCE_PLL_DIV2_5)   || \
                                          ((__USBCLK__) == RCC_USBCLKSOURCE_PLL_DIV3) || ((__USBCLK__) == RCC_USBCLKSOURCE_PLL_DIV3_5)   || \
                                          ((__USBCLK__) == RCC_USBCLKSOURCE_PLL_DIV4) || ((__USBCLK__) == RCC_USBCLKSOURCE_HSI48M))

#define IS_RCC_CANCLKSOURCE(__CANCLK__)  (((__CANCLK__) == RCC_CANCLKSOURCE_PLL)      || ((__CANCLK__) == RCC_CANCLKSOURCE_PLL_DIV2)   || \
                                          ((__CANCLK__) == RCC_CANCLKSOURCE_PLL_DIV3) || ((__CANCLK__) == RCC_CANCLKSOURCE_PLL_DIV4)   || \
                                          ((__CANCLK__) == RCC_CANCLKSOURCE_PLL_DIV5) || ((__CANCLK__) == RCC_CANCLKSOURCE_PLL_DIV6)   || \
                                          ((__CANCLK__) == RCC_CANCLKSOURCE_PLL_DIV7) || ((__CANCLK__) == RCC_CANCLKSOURCE_PLL_DIV8)   || \
                                          ((__CANCLK__) == RCC_CANCLKSOURCE_HSE))

#define IS_RCC_PERIPHCLOCK(PERIPHCLOCK)  (((PERIPHCLOCK) == RCC_PERIPHCLK_RTC) || ((PERIPHCLOCK) == RCC_PERIPHCLK_ADC) || \
                                          ((PERIPHCLOCK) == RCC_PERIPHCLK_USB) || ((PERIPHCLOCK) == RCC_PERIPHCLK_CAN))



/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/

/** @defgroup RCCEx_Exported_Types RCCEx Exported Types
  * @{
  */

/**
  * @brief  RCC extended clocks structure definition
  */
typedef struct
{
  uint32_t PeriphClockSelection;            /*!< The Extended Clock to be configured.
                                            This parameter can be a value of @ref RCCEx_Periph_Clock_Selection */

  uint32_t RtcClockSelection;              /*!< specifies the RTC clock source.
                                             This parameter can be a value of @ref RCC_RTC_Clock_Source */

  uint32_t AdcClockSelection;              /*!< ADC clock source
                                            This parameter can be a value of @ref RCCEx_ADC_Prescaler */

  uint32_t UsbClockSelection;              /*!< USB clock source
                                            This parameter can be a value of @ref RCCEx_USB_Prescaler */

  uint32_t CanClockSelection;              /*!< CAN clock source
                                            This parameter can be a value of @ref RCCEx_CAN_Clock_Source */
  
} RCC_PeriphCLKInitTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup RCCEx_Exported_Constants RCCEx Exported Constants
  * @{
  */

/** @defgroup RCCEx_Periph_Clock_Selection Periph Clock Selection
  * @{
  */
#define RCC_PERIPHCLK_RTC           0x00000001U
#define RCC_PERIPHCLK_ADC           0x00000002U
#define RCC_PERIPHCLK_USB           0x00000010U
#define RCC_PERIPHCLK_CAN           0x00000020U

/**
  * @}
  */

/** @defgroup RCCEx_ADC_Prescaler ADC Prescaler
  * @{
  */
#define RCC_ADCPCLK2_DIV2              RCC_CFGR_ADCPRE_DIV2
#define RCC_ADCPCLK2_DIV4              RCC_CFGR_ADCPRE_DIV4
#define RCC_ADCPCLK2_DIV6              RCC_CFGR_ADCPRE_DIV6
#define RCC_ADCPCLK2_DIV8              RCC_CFGR_ADCPRE_DIV8
#define RCC_ADCPCLK2_DIV12             RCC_CFGR_ADCPRE_DIV12
#define RCC_ADCPCLK2_DIV16             RCC_CFGR_ADCPRE_DIV16

/**
  * @}
  */
  
/** @defgroup RCCEx_USB_Prescaler USB Clock Source
  * @{
  */
#define RCC_USBCLKSOURCE_PLL              RCC_CFGR_USBPRE_0
#define RCC_USBCLKSOURCE_PLL_DIV1_5       0x00000000U
#define RCC_USBCLKSOURCE_PLL_DIV2         (RCC_CFGR_USBPRE_1 | RCC_CFGR_USBPRE_0)
#define RCC_USBCLKSOURCE_PLL_DIV2_5       (RCC_CFGR_USBPRE_1)
#define RCC_USBCLKSOURCE_PLL_DIV3         (RCC_CFGR_USBPRE_2)
#define RCC_USBCLKSOURCE_PLL_DIV3_5       (RCC_CFGR_USBPRE_2 | RCC_CFGR_USBPRE_0)
#define RCC_USBCLKSOURCE_PLL_DIV4         (RCC_CFGR_USBPRE_2 | RCC_CFGR_USBPRE_1)
#define RCC_USBCLKSOURCE_HSI48M           RCC_CFGR1_USBSELHSI48

/**
  * @}
  */

/** @defgroup RCCEx_CAN_Clock_Source CAN Clock Source
  * @{
  */
#define RCC_CANCLKSOURCE_PLL           0x00000000U
#define RCC_CANCLKSOURCE_PLL_DIV2      RCC_CFGR2_CANCKSEL_0
#define RCC_CANCLKSOURCE_PLL_DIV3      RCC_CFGR2_CANCKSEL_1
#define RCC_CANCLKSOURCE_PLL_DIV4      (RCC_CFGR2_CANCKSEL_1 | RCC_CFGR2_CANCKSEL_0)
#define RCC_CANCLKSOURCE_PLL_DIV5      RCC_CFGR2_CANCKSEL_2
#define RCC_CANCLKSOURCE_PLL_DIV6      (RCC_CFGR2_CANCKSEL_2 | RCC_CFGR2_CANCKSEL_0)
#define RCC_CANCLKSOURCE_PLL_DIV7      (RCC_CFGR2_CANCKSEL_2 | RCC_CFGR2_CANCKSEL_1)
#define RCC_CANCLKSOURCE_PLL_DIV8      (RCC_CFGR2_CANCKSEL_2 | RCC_CFGR2_CANCKSEL_1 | RCC_CFGR2_CANCKSEL_0)
#define RCC_CANCLKSOURCE_HSE           RCC_CFGR2_CANCKSEL_3

/**
  * @}
  */

/** @defgroup RCCEx_Prediv1_Factor HSE Prediv1 Factor
  * @{
  */

#define RCC_HSE_PREDIV_DIV1              0x00000000U
#define RCC_HSE_PREDIV_DIV2              RCC_CFGR_PLLXTPRE

/**
  * @}
  */

/** @defgroup RCCEx_PLL_Multiplication_Factor PLL Multiplication Factor
  * @{
  */
#define RCC_PLL_MUL2                    RCC_CFGR_PLLMULL2
#define RCC_PLL_MUL3                    RCC_CFGR_PLLMULL3
#define RCC_PLL_MUL4                    RCC_CFGR_PLLMULL4
#define RCC_PLL_MUL5                    RCC_CFGR_PLLMULL5
#define RCC_PLL_MUL6                    RCC_CFGR_PLLMULL6
#define RCC_PLL_MUL7                    RCC_CFGR_PLLMULL7
#define RCC_PLL_MUL8                    RCC_CFGR_PLLMULL8
#define RCC_PLL_MUL9                    RCC_CFGR_PLLMULL9
#define RCC_PLL_MUL10                   RCC_CFGR_PLLMULL10
#define RCC_PLL_MUL11                   RCC_CFGR_PLLMULL11
#define RCC_PLL_MUL12                   RCC_CFGR_PLLMULL12
#define RCC_PLL_MUL13                   RCC_CFGR_PLLMULL13
#define RCC_PLL_MUL14                   RCC_CFGR_PLLMULL14
#define RCC_PLL_MUL15                   RCC_CFGR_PLLMULL15
#define RCC_PLL_MUL16                   RCC_CFGR_PLLMULL16
#define RCC_PLL_MUL17                   RCC_CFGR_PLLMULL17
#define RCC_PLL_MUL18                   RCC_CFGR_PLLMULL18
#define RCC_PLL_MUL19                   RCC_CFGR_PLLMULL19
#define RCC_PLL_MUL20                   RCC_CFGR_PLLMULL20
#define RCC_PLL_MUL21                   RCC_CFGR_PLLMULL21
#define RCC_PLL_MUL22                   RCC_CFGR_PLLMULL22
#define RCC_PLL_MUL23                   RCC_CFGR_PLLMULL23
#define RCC_PLL_MUL24                   RCC_CFGR_PLLMULL24
#define RCC_PLL_MUL25                   RCC_CFGR_PLLMULL25
#define RCC_PLL_MUL26                   RCC_CFGR_PLLMULL26
#define RCC_PLL_MUL27                   RCC_CFGR_PLLMULL27
#define RCC_PLL_MUL28                   RCC_CFGR_PLLMULL28
#define RCC_PLL_MUL29                   RCC_CFGR_PLLMULL29
#define RCC_PLL_MUL30                   RCC_CFGR_PLLMULL30
#define RCC_PLL_MUL31                   RCC_CFGR_PLLMULL31
#define RCC_PLL_MUL32                   RCC_CFGR_PLLMULL32
#define RCC_PLL_MUL33                   RCC_CFGR_PLLMULL33
#define RCC_PLL_MUL34                   RCC_CFGR_PLLMULL34
#define RCC_PLL_MUL35                   RCC_CFGR_PLLMULL35
#define RCC_PLL_MUL36                   RCC_CFGR_PLLMULL36
#define RCC_PLL_MUL37                   RCC_CFGR_PLLMULL37
#define RCC_PLL_MUL38                   RCC_CFGR_PLLMULL38
#define RCC_PLL_MUL39                   RCC_CFGR_PLLMULL39
#define RCC_PLL_MUL40                   RCC_CFGR_PLLMULL40
#define RCC_PLL_MUL41                   RCC_CFGR_PLLMULL41
#define RCC_PLL_MUL42                   RCC_CFGR_PLLMULL42
#define RCC_PLL_MUL43                   RCC_CFGR_PLLMULL43
#define RCC_PLL_MUL44                   RCC_CFGR_PLLMULL44
#define RCC_PLL_MUL45                   RCC_CFGR_PLLMULL45
#define RCC_PLL_MUL46                   RCC_CFGR_PLLMULL46
#define RCC_PLL_MUL47                   RCC_CFGR_PLLMULL47
#define RCC_PLL_MUL48                   RCC_CFGR_PLLMULL48
#define RCC_PLL_MUL49                   RCC_CFGR_PLLMULL49
#define RCC_PLL_MUL50                   RCC_CFGR_PLLMULL50
#define RCC_PLL_MUL51                   RCC_CFGR_PLLMULL51
#define RCC_PLL_MUL52                   RCC_CFGR_PLLMULL52
#define RCC_PLL_MUL53                   RCC_CFGR_PLLMULL53
#define RCC_PLL_MUL54                   RCC_CFGR_PLLMULL54
#define RCC_PLL_MUL55                   RCC_CFGR_PLLMULL55
#define RCC_PLL_MUL56                   RCC_CFGR_PLLMULL56
#define RCC_PLL_MUL57                   RCC_CFGR_PLLMULL57
#define RCC_PLL_MUL58                   RCC_CFGR_PLLMULL58
#define RCC_PLL_MUL59                   RCC_CFGR_PLLMULL59
#define RCC_PLL_MUL60                   RCC_CFGR_PLLMULL60
#define RCC_PLL_MUL61                   RCC_CFGR_PLLMULL61
#define RCC_PLL_MUL62                   RCC_CFGR_PLLMULL62
#define RCC_PLL_MUL63                   RCC_CFGR_PLLMULL63

/**
  * @}
  */

/** @defgroup RCCEx_MCO1_Clock_Source MCO1 Clock Source
  * @{
  */
#define RCC_MCO1SOURCE_NOCLOCK           ((uint32_t)RCC_CFGR_MCO_NOCLOCK)
#define RCC_MCO1SOURCE_SYSCLK            ((uint32_t)RCC_CFGR_MCO_SYSCLK)
#define RCC_MCO1SOURCE_HSI               ((uint32_t)RCC_CFGR_MCO_HSI8M)
#define RCC_MCO1SOURCE_LSI               ((uint32_t)RCC_CFGR_MCO_LSI)
#define RCC_MCO1SOURCE_HSE               ((uint32_t)RCC_CFGR_MCO_HSE)
#define RCC_MCO1SOURCE_LSE               ((uint32_t)RCC_CFGR_MCO_LSE)
#define RCC_MCO1SOURCE_PLLCLK            ((uint32_t)RCC_CFGR_MCO_PLL)
#define RCC_MCO1SOURCE_HSI48M            ((uint32_t)RCC_CFGR_MCO_HSI48M)
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup RCCEx_Exported_Macros RCCEx Exported Macros
 * @{
 */

/** @defgroup RCCEx_Peripheral_Clock_Enable_Disable Peripheral Clock Enable Disable
  * @brief  Enable or disable the AHB1 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */
#define __HAL_RCC_DMA2_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_SDIO_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_SDIOEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_SDIOEN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)


#define __HAL_RCC_SDIO_CLK_DISABLE()        (RCC->AHB1ENR &= ~(RCC_AHB1ENR_SDIOEN))
#define __HAL_RCC_DMA2_CLK_DISABLE()        (RCC->AHB1ENR &= ~(RCC_AHB1ENR_DMA2EN))

/**
  * @}
  */

/** @defgroup RCCEx_AHB1_Peripheral_Clock_Enable_Disable_Status AHB1 Peripheral Clock Enable Disable Status
  * @brief  Get the enable or disable status of the AHB1 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_DMA2_IS_CLK_ENABLED()       ((RCC->AHB1ENR & (RCC_AHB1ENR_DMA2EN)) != RESET)
#define __HAL_RCC_DMA2_IS_CLK_DISABLED()      ((RCC->AHB1ENR & (RCC_AHB1ENR_DMA2EN)) == RESET)

#define __HAL_RCC_SDIO_IS_CLK_ENABLED()       ((RCC->AHB1ENR & (RCC_AHB1ENR_SDIOEN)) != RESET)
#define __HAL_RCC_SDIO_IS_CLK_DISABLED()      ((RCC->AHB1ENR & (RCC_AHB1ENR_SDIOEN)) == RESET)

/**
  * @}
  */

/** @defgroup RCCEx_APB1_Clock_Enable_Disable APB1 Clock Enable Disable
  * @brief  Enable or disable the Low Speed APB (APB1) peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */
#define __HAL_RCC_TIM5_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM5EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM5EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_TIM6_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM6EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM6EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_TIM7_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM7EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM7EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)



#define __HAL_RCC_TIM12_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM12EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM12EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_TIM13_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM13EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM13EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_TIM14_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM14EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM14EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_SPI3_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_SPI3EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_SPI3EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_USART3_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USART3EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_USART3EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_USART4_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USART4EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_USART4EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_USART5_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USART5EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_USART5EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)
#if defined(DAC1)
#define __HAL_RCC_DAC_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_DACEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_DACEN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)
#endif /* DAC1 */

#define __HAL_RCC_TIM5_CLK_DISABLE()        (RCC->APB1ENR &= ~(RCC_APB1ENR_TIM5EN))
#define __HAL_RCC_TIM6_CLK_DISABLE()        (RCC->APB1ENR &= ~(RCC_APB1ENR_TIM6EN))
#define __HAL_RCC_TIM7_CLK_DISABLE()        (RCC->APB1ENR &= ~(RCC_APB1ENR_TIM7EN))
#define __HAL_RCC_TIM12_CLK_DISABLE()       (RCC->APB1ENR &= ~(RCC_APB1ENR_TIM12EN))
#define __HAL_RCC_TIM13_CLK_DISABLE()       (RCC->APB1ENR &= ~(RCC_APB1ENR_TIM13EN))
#define __HAL_RCC_TIM14_CLK_DISABLE()       (RCC->APB1ENR &= ~(RCC_APB1ENR_TIM14EN))
#define __HAL_RCC_SPI3_CLK_DISABLE()        (RCC->APB1ENR &= ~(RCC_APB1ENR_SPI3EN))
#define __HAL_RCC_USART3_CLK_DISABLE()      (RCC->APB1ENR &= ~(RCC_APB1ENR_USART3EN))
#define __HAL_RCC_USART4_CLK_DISABLE()      (RCC->APB1ENR &= ~(RCC_APB1ENR_USART4EN))
#define __HAL_RCC_USART5_CLK_DISABLE()      (RCC->APB1ENR &= ~(RCC_APB1ENR_USART5EN))
#if defined(DAC1)
#define __HAL_RCC_DAC_CLK_DISABLE()         (RCC->APB1ENR &= ~(RCC_APB1ENR_DACEN))
#endif /* DAC1 */

/**
  * @}
  */

/** @defgroup RCCEx_APB1_Peripheral_Clock_Enable_Disable_Status APB1 Peripheral Clock Enable Disable Status
  * @brief  Get the enable or disable status of the APB1 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */
#define __HAL_RCC_TIM5_IS_CLK_ENABLED()         ((RCC->APB1ENR & (RCC_APB1ENR_TIM5EN)) != RESET)
#define __HAL_RCC_TIM5_IS_CLK_DISABLED()        ((RCC->APB1ENR & (RCC_APB1ENR_TIM5EN)) == RESET)
#define __HAL_RCC_TIM6_IS_CLK_ENABLED()         ((RCC->APB1ENR & (RCC_APB1ENR_TIM6EN)) != RESET)
#define __HAL_RCC_TIM6_IS_CLK_DISABLED()        ((RCC->APB1ENR & (RCC_APB1ENR_TIM6EN)) == RESET)
#define __HAL_RCC_TIM7_IS_CLK_ENABLED()         ((RCC->APB1ENR & (RCC_APB1ENR_TIM7EN)) != RESET)
#define __HAL_RCC_TIM7_IS_CLK_DISABLED()        ((RCC->APB1ENR & (RCC_APB1ENR_TIM7EN)) == RESET)
#define __HAL_RCC_TIM12_IS_CLK_ENABLED()        ((RCC->APB1ENR & (RCC_APB1ENR_TIM12EN)) != RESET)
#define __HAL_RCC_TIM12_IS_CLK_DISABLED()       ((RCC->APB1ENR & (RCC_APB1ENR_TIM12EN)) == RESET)
#define __HAL_RCC_TIM13_IS_CLK_ENABLED()        ((RCC->APB1ENR & (RCC_APB1ENR_TIM13EN)) != RESET)
#define __HAL_RCC_TIM13_IS_CLK_DISABLED()       ((RCC->APB1ENR & (RCC_APB1ENR_TIM13EN)) == RESET)
#define __HAL_RCC_TIM14_IS_CLK_ENABLED()        ((RCC->APB1ENR & (RCC_APB1ENR_TIM14EN)) != RESET)
#define __HAL_RCC_TIM14_IS_CLK_DISABLED()       ((RCC->APB1ENR & (RCC_APB1ENR_TIM14EN)) == RESET)
#define __HAL_RCC_SPI3_IS_CLK_ENABLED()         ((RCC->APB1ENR & (RCC_APB1ENR_SPI3EN)) != RESET)
#define __HAL_RCC_SPI3_IS_CLK_DISABLED()        ((RCC->APB1ENR & (RCC_APB1ENR_SPI3EN)) == RESET)
#define __HAL_RCC_USART3_IS_CLK_ENABLED()       ((RCC->APB1ENR & (RCC_APB1ENR_USART3EN)) != RESET)
#define __HAL_RCC_USART3_IS_CLK_DISABLED()      ((RCC->APB1ENR & (RCC_APB1ENR_USART3EN)) == RESET)
#define __HAL_RCC_USART4_IS_CLK_ENABLED()       ((RCC->APB1ENR & (RCC_APB1ENR_USART4EN)) != RESET)
#define __HAL_RCC_USART4_IS_CLK_DISABLED()      ((RCC->APB1ENR & (RCC_APB1ENR_USART4EN)) == RESET)
#define __HAL_RCC_USART5_IS_CLK_ENABLED()       ((RCC->APB1ENR & (RCC_APB1ENR_USART5EN)) != RESET)
#define __HAL_RCC_USART5_IS_CLK_DISABLED()      ((RCC->APB1ENR & (RCC_APB1ENR_USART5EN)) == RESET)
#if defined(DAC1)
#define __HAL_RCC_DAC_IS_CLK_ENABLED()          ((RCC->APB1ENR & (RCC_APB1ENR_DACEN)) != RESET)
#define __HAL_RCC_DAC_IS_CLK_DISABLED()         ((RCC->APB1ENR & (RCC_APB1ENR_DACEN)) == RESET)
#endif /* DAC1 */

/**
  * @}
  */

/** @defgroup RCCEx_APB2_Clock_Enable_Disable APB2 Clock Enable Disable
  * @brief  Enable or disable the High Speed APB (APB2) peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_ADC2_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB2ENR, RCC_APB2ENR_ADC2EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_ADC2EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_TIM8_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM8EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM8EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_ADC3_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB2ENR, RCC_APB2ENR_ADC3EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_ADC3EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_TIM9_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM9EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM9EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_TIM10_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM10EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM10EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_TIM11_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM11EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM11EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

#define __HAL_RCC_ADC2_CLK_DISABLE()         (RCC->APB2ENR &= ~(RCC_APB2ENR_ADC2EN))
#define __HAL_RCC_TIM8_CLK_DISABLE()         (RCC->APB2ENR &= ~(RCC_APB2ENR_TIM8EN))
#define __HAL_RCC_ADC3_CLK_DISABLE()         (RCC->APB2ENR &= ~(RCC_APB2ENR_ADC3EN))
#define __HAL_RCC_TIM9_CLK_DISABLE()         (RCC->APB2ENR &= ~(RCC_APB2ENR_TIM9EN))
#define __HAL_RCC_TIM10_CLK_DISABLE()        (RCC->APB2ENR &= ~(RCC_APB2ENR_TIM10EN))
#define __HAL_RCC_TIM11_CLK_DISABLE()        (RCC->APB2ENR &= ~(RCC_APB2ENR_TIM11EN))

/**
  * @}
  */

/** @defgroup RCCEx_APB2_Peripheral_Clock_Enable_Disable_Status APB2 Peripheral Clock Enable Disable Status
  * @brief  Get the enable or disable status of the APB2 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_ADC2_IS_CLK_ENABLED()        ((RCC->APB2ENR & (RCC_APB2ENR_ADC2EN)) != RESET)
#define __HAL_RCC_ADC2_IS_CLK_DISABLED()       ((RCC->APB2ENR & (RCC_APB2ENR_ADC2EN)) == RESET)
#define __HAL_RCC_TIM8_IS_CLK_ENABLED()        ((RCC->APB2ENR & (RCC_APB2ENR_TIM8EN)) != RESET)
#define __HAL_RCC_TIM8_IS_CLK_DISABLED()       ((RCC->APB2ENR & (RCC_APB2ENR_TIM8EN)) == RESET)
#define __HAL_RCC_ADC3_IS_CLK_ENABLED()        ((RCC->APB2ENR & (RCC_APB2ENR_ADC3EN)) != RESET)
#define __HAL_RCC_ADC3_IS_CLK_DISABLED()       ((RCC->APB2ENR & (RCC_APB2ENR_ADC3EN)) == RESET)
#define __HAL_RCC_TIM9_IS_CLK_ENABLED()        ((RCC->APB2ENR & (RCC_APB2ENR_TIM9EN)) != RESET)
#define __HAL_RCC_TIM9_IS_CLK_DISABLED()       ((RCC->APB2ENR & (RCC_APB2ENR_TIM9EN)) == RESET)
#define __HAL_RCC_TIM10_IS_CLK_ENABLED()       ((RCC->APB2ENR & (RCC_APB2ENR_TIM10EN)) != RESET)
#define __HAL_RCC_TIM10_IS_CLK_DISABLED()      ((RCC->APB2ENR & (RCC_APB2ENR_TIM10EN)) == RESET)
#define __HAL_RCC_TIM11_IS_CLK_ENABLED()       ((RCC->APB2ENR & (RCC_APB2ENR_TIM11EN)) != RESET)
#define __HAL_RCC_TIM11_IS_CLK_DISABLED()      ((RCC->APB2ENR & (RCC_APB2ENR_TIM11EN)) == RESET)

/**
  * @}
  */

/** @defgroup RCCEx_Peripheral_Clock_Force_Release Peripheral Clock Force Release
  * @brief  Force or release AHB1 peripheral reset.
  * @{
  */
#define __HAL_RCC_AHB1_FORCE_RESET()          (RCC->AHB1RSTR = 0xFFFFFFFFU)
#define __HAL_RCC_DMA1_FORCE_RESET()         (RCC->AHB1RSTR |= (RCC_AHB1RSTR_DMA1RST))
#define __HAL_RCC_DMA2_FORCE_RESET()         (RCC->AHB1RSTR |= (RCC_AHB1RSTR_DMA2RST))
#define __HAL_RCC_CRC_FORCE_RESET()          (RCC->AHB1RSTR |= (RCC_AHB1RSTR_CRCRST))
#define __HAL_RCC_SDIO_FORCE_RESET()         (RCC->AHB1RSTR |= (RCC_AHB1RSTR_SDIORST))
#define __HAL_RCC_ESMC_FORCE_RESET()         (RCC->AHB1RSTR |= (RCC_AHB1RSTR_ESMCRST))

#define __HAL_RCC_AHB1_RELEASE_RESET()        (RCC->AHB1RSTR = 0x00)
#define __HAL_RCC_DMA1_RELEASE_RESET()       (RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_DMA1RST))
#define __HAL_RCC_DMA2_RELEASE_RESET()       (RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_DMA2RST))
#define __HAL_RCC_CRC_RELEASE_RESET()        (RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_CRCRST))
#define __HAL_RCC_SDIO_RELEASE_RESET()       (RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_SDIORST))
#define __HAL_RCC_ESMC_RELEASE_RESET()       (RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_ESMCRST))

/**
  * @}
  */

/** @defgroup RCCEx_APB1_Force_Release_Reset APB1 Force Release Reset
  * @brief  Force or release APB1 peripheral reset.
  * @{
  */
#define __HAL_RCC_TIM5_FORCE_RESET()         (RCC->APB1RSTR |= (RCC_APB1RSTR_TIM5RST))
#define __HAL_RCC_TIM6_FORCE_RESET()         (RCC->APB1RSTR |= (RCC_APB1RSTR_TIM6RST))
#define __HAL_RCC_TIM7_FORCE_RESET()         (RCC->APB1RSTR |= (RCC_APB1RSTR_TIM7RST))
#define __HAL_RCC_TIM12_FORCE_RESET()        (RCC->APB1RSTR |= (RCC_APB1RSTR_TIM12RST))
#define __HAL_RCC_TIM13_FORCE_RESET()        (RCC->APB1RSTR |= (RCC_APB1RSTR_TIM13RST))
#define __HAL_RCC_TIM14_FORCE_RESET()        (RCC->APB1RSTR |= (RCC_APB1RSTR_TIM14RST))
#define __HAL_RCC_SPI3_FORCE_RESET()         (RCC->APB1RSTR |= (RCC_APB1RSTR_SPI3RST))
#define __HAL_RCC_USART3_FORCE_RESET()       (RCC->APB1RSTR |= (RCC_APB1RSTR_USART3RST))
#define __HAL_RCC_USART4_FORCE_RESET()       (RCC->APB1RSTR |= (RCC_APB1RSTR_USART4RST))
#define __HAL_RCC_USART5_FORCE_RESET()       (RCC->APB1RSTR |= (RCC_APB1RSTR_USART5RST))
#if defined(DAC1)
#define __HAL_RCC_DAC_FORCE_RESET()          (RCC->APB1RSTR |= (RCC_APB1RSTR_DACRST))
#endif /* DAC1 */

#define __HAL_RCC_TIM5_RELEASE_RESET()       (RCC->APB1RSTR &= ~(RCC_APB1RSTR_TIM5RST))
#define __HAL_RCC_TIM6_RELEASE_RESET()       (RCC->APB1RSTR &= ~(RCC_APB1RSTR_TIM6RST))
#define __HAL_RCC_TIM7_RELEASE_RESET()       (RCC->APB1RSTR &= ~(RCC_APB1RSTR_TIM7RST))
#define __HAL_RCC_TIM12_RELEASE_RESET()      (RCC->APB1RSTR &= ~(RCC_APB1RSTR_TIM12RST))
#define __HAL_RCC_TIM13_RELEASE_RESET()      (RCC->APB1RSTR &= ~(RCC_APB1RSTR_TIM13RST))
#define __HAL_RCC_TIM14_RELEASE_RESET()      (RCC->APB1RSTR &= ~(RCC_APB1RSTR_TIM14RST))
#define __HAL_RCC_SPI3_RELEASE_RESET()       (RCC->APB1RSTR &= ~(RCC_APB1RSTR_SPI3RST))
#define __HAL_RCC_USART3_RELEASE_RESET()     (RCC->APB1RSTR &= ~(RCC_APB1RSTR_USART3RST))
#define __HAL_RCC_USART4_RELEASE_RESET()     (RCC->APB1RSTR &= ~(RCC_APB1RSTR_USART4RST))
#define __HAL_RCC_USART5_RELEASE_RESET()     (RCC->APB1RSTR &= ~(RCC_APB1RSTR_USART5RST))
#if defined(DAC1)
#define __HAL_RCC_DAC_RELEASE_RESET()        (RCC->APB1RSTR &= ~(RCC_APB1RSTR_DACRST))
#endif /* DAC1 */

/**
  * @}
  */

/** @defgroup RCCEx_APB2_Force_Release_Reset APB2 Force Release Reset
  * @brief  Force or release APB2 peripheral reset.
  * @{
  */
#define __HAL_RCC_ADC2_FORCE_RESET()        (RCC->APB2RSTR |= (RCC_APB2RSTR_ADC2RST))
#define __HAL_RCC_TIM8_FORCE_RESET()        (RCC->APB2RSTR |= (RCC_APB2RSTR_TIM8RST))
#define __HAL_RCC_ADC3_FORCE_RESET()        (RCC->APB2RSTR |= (RCC_APB2RSTR_ADC3RST))
#define __HAL_RCC_TIM9_FORCE_RESET()        (RCC->APB2RSTR |= (RCC_APB2RSTR_TIM9RST))
#define __HAL_RCC_TIM10_FORCE_RESET()       (RCC->APB2RSTR |= (RCC_APB2RSTR_TIM10RST))
#define __HAL_RCC_TIM11_FORCE_RESET()       (RCC->APB2RSTR |= (RCC_APB2RSTR_TIM11RST))

#define __HAL_RCC_ADC2_RELEASE_RESET()      (RCC->APB2RSTR &= ~(RCC_APB2RSTR_ADC2RST))
#define __HAL_RCC_TIM8_RELEASE_RESET()      (RCC->APB2RSTR &= ~(RCC_APB2RSTR_TIM8RST))
#define __HAL_RCC_ADC3_RELEASE_RESET()      (RCC->APB2RSTR &= ~(RCC_APB2RSTR_ADC3RST))
#define __HAL_RCC_TIM9_RELEASE_RESET()      (RCC->APB2RSTR &= ~(RCC_APB2RSTR_TIM9RST))
#define __HAL_RCC_TIM10_RELEASE_RESET()     (RCC->APB2RSTR &= ~(RCC_APB2RSTR_TIM10RST))
#define __HAL_RCC_TIM11_RELEASE_RESET()     (RCC->APB2RSTR &= ~(RCC_APB2RSTR_TIM11RST))

/**
  * @}
  */

/** @defgroup RCCEx_HSE_Configuration HSE Configuration
  * @{
  */

/**
  * @brief  Macro to configure the External High Speed oscillator (HSE) Predivision factor for PLL.
  * @note   Predivision factor can not be changed if PLL is used as system clock
  *         In this case, you have to select another source of the system clock, disable the PLL and
  *         then change the HSE predivision factor.
  * @param  __HSE_PREDIV_VALUE__ specifies the division value applied to HSE.
  *         This parameter must be a number between RCC_HSE_PREDIV_DIV1 and RCC_HSE_PREDIV_DIV2.
  */
#define __HAL_RCC_HSE_PREDIV_CONFIG(__HSE_PREDIV_VALUE__) \
                  MODIFY_REG(RCC->CFGR,RCC_CFGR_PLLXTPRE, (uint32_t)(__HSE_PREDIV_VALUE__))

/**
  * @brief  Macro to get prediv1 factor for PLL.
  */
#define __HAL_RCC_HSE_GET_PREDIV() READ_BIT(RCC->CFGR, RCC_CFGR_PLLXTPRE)


/**
  * @}
  */

/** @defgroup RCCEx_Peripheral_Configuration Peripheral Configuration
  * @brief  Macros to configure clock source of different peripherals.
  * @{
  */

/** @brief  Macro to configure the ADCx clock (x=1 to 3 depending on devices).
  * @param  __ADCCLKSOURCE__ specifies the ADC clock source.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_ADCPCLK2_DIV2  PCLK2 clock divided by 2  selected as ADC clock
  *            @arg @ref RCC_ADCPCLK2_DIV4  PCLK2 clock divided by 4  selected as ADC clock
  *            @arg @ref RCC_ADCPCLK2_DIV6  PCLK2 clock divided by 6  selected as ADC clock
  *            @arg @ref RCC_ADCPCLK2_DIV8  PCLK2 clock divided by 8  selected as ADC clock
  *            @arg @ref RCC_ADCPCLK2_DIV12 PCLK2 clock divided by 12 selected as ADC clock
  *            @arg @ref RCC_ADCPCLK2_DIV16 PCLK2 clock divided by 16 selected as ADC clock
  */
#define __HAL_RCC_ADC_CONFIG(__ADCCLKSOURCE__) \
                  MODIFY_REG(RCC->CFGR, RCC_CFGR_ADCPRE, (uint32_t)(__ADCCLKSOURCE__))

/** @brief  Macro to get the ADC clock (ADCxCLK, x=1 to 3 depending on devices).
  * @retval The clock source can be one of the following values:
  *            @arg @ref RCC_ADCPCLK2_DIV2  PCLK2 clock divided by 2  selected as ADC clock
  *            @arg @ref RCC_ADCPCLK2_DIV4  PCLK2 clock divided by 4  selected as ADC clock
  *            @arg @ref RCC_ADCPCLK2_DIV6  PCLK2 clock divided by 6  selected as ADC clock
  *            @arg @ref RCC_ADCPCLK2_DIV8  PCLK2 clock divided by 8  selected as ADC clock
  *            @arg @ref RCC_ADCPCLK2_DIV12 PCLK2 clock divided by 12 selected as ADC clock
  *            @arg @ref RCC_ADCPCLK2_DIV16 PCLK2 clock divided by 16 selected as ADC clock
  */
#define __HAL_RCC_GET_ADC_SOURCE() ((uint32_t)(READ_BIT(RCC->CFGR, RCC_CFGR_ADCPRE)))

/** @brief  Macro to configure the USB clock.
  * @param  __USBCLKSOURCE__ specifies the USB clock source.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_USBCLKSOURCE_PLL PLL clock divided by 1 selected as USB clock
  *            @arg @ref RCC_USBCLKSOURCE_PLL_DIV1_5 PLL clock divided by 1.5 selected as USB clock
  *            @arg @ref RCC_USBCLKSOURCE_PLL_DIV2   PLL clock divided by 2 selected as USB clock
  *            @arg @ref RCC_USBCLKSOURCE_PLL_DIV2_5 PLL clock divided by 2.5 selected as USB clock
  *            @arg @ref RCC_USBCLKSOURCE_PLL_DIV3   PLL clock divided by 3 selected as USB clock
  *            @arg @ref RCC_USBCLKSOURCE_PLL_DIV3_5 PLL clock divided by 3.5 selected as USB clock
  *            @arg @ref RCC_USBCLKSOURCE_PLL_DIV4   PLL clock divided by 4 selected as USB clock
*/
#define __HAL_RCC_USB_CONFIG(__USBCLKSOURCE__) \
                  MODIFY_REG(RCC->CFGR, RCC_CFGR_USBPRE, (uint32_t)(__USBCLKSOURCE__))

/** @brief  Macro to get the USB clock.
  * @retval The clock source can be one of the following values:
  *            @arg @ref RCC_USBCLKSOURCE_PLL PLL clock divided by 1 selected as USB clock
  *            @arg @ref RCC_USBCLKSOURCE_PLL_DIV1_5 PLL clock divided by 1.5 selected as USB clock
  *            @arg @ref RCC_USBCLKSOURCE_PLL_DIV2   PLL clock divided by 2 selected as USB clock
  *            @arg @ref RCC_USBCLKSOURCE_PLL_DIV2_5 PLL clock divided by 2.5 selected as USB clock
  *            @arg @ref RCC_USBCLKSOURCE_PLL_DIV3   PLL clock divided by 3 selected as USB clock
  *            @arg @ref RCC_USBCLKSOURCE_PLL_DIV3_5 PLL clock divided by 3.5 selected as USB clock
  *            @arg @ref RCC_USBCLKSOURCE_PLL_DIV4   PLL clock divided by 4 selected as USB clock
  */
#define __HAL_RCC_GET_USB_SOURCE() ((uint32_t)(READ_BIT(RCC->CFGR, RCC_CFGR_USBPRE)))

/** @brief  Macro to configure the CAN clock.
  * @param  __CANCLKSOURCE__ specifies the CAN clock source.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_CANCLKSOURCE_PLL      PLL clock divided by 1 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV2 PLL clock divided by 2 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV3 PLL clock divided by 3 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV4 PLL clock divided by 4 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV5 PLL clock divided by 5 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV6 PLL clock divided by 6 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV7 PLL clock divided by 7 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV8 PLL clock divided by 8 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_HSE      HSE selected as CAN clock
*/
#define __HAL_RCC_CAN_CONFIG(__CANCLKSOURCE__) \
                  MODIFY_REG(RCC->CFGR2, RCC_CFGR2_CANCKSEL, (uint32_t)(__CANCLKSOURCE__))

/** @brief  Macro to get the CAN clock.
  * @retval The clock source can be one of the following values:
  *            @arg @ref RCC_CANCLKSOURCE_PLL      PLL clock divided by 1 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV2 PLL clock divided by 2 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV3 PLL clock divided by 3 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV4 PLL clock divided by 4 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV5 PLL clock divided by 5 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV6 PLL clock divided by 6 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV7 PLL clock divided by 7 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_PLL_DIV8 PLL clock divided by 8 selected as CAN clock
  *            @arg @ref RCC_CANCLKSOURCE_HSE      HSE selected as CAN clock
  */
#define __HAL_RCC_GET_CAN_SOURCE() ((uint32_t)(READ_BIT(RCC->CFGR2, RCC_CFGR2_CANCKSEL)))

/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup RCCEx_Exported_Functions
  * @{
  */

/** @addtogroup RCCEx_Exported_Functions_Group1
  * @{
  */

HAL_StatusTypeDef HAL_RCCEx_PeriphCLKConfig(RCC_PeriphCLKInitTypeDef  *PeriphClkInit);
void              HAL_RCCEx_GetPeriphCLKConfig(RCC_PeriphCLKInitTypeDef  *PeriphClkInit);
uint32_t          HAL_RCCEx_GetPeriphCLKFreq(uint32_t PeriphClk);

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

#endif /* __PY32F4XX_HAL_RCC_EX_H */


/************************ (C) COPYRIGHT Puya *****END OF FILE******************/

