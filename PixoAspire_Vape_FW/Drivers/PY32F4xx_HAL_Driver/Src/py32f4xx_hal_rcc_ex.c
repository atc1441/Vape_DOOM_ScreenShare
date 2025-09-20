/**
  ******************************************************************************
  * @file    py32f4xx_hal_rcc_ex.c
  * @author  MCU Application Team
  * @brief   Extended RCC HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities RCC extended peripheral:
  *           + Extended Peripheral Control functions
  *           + Extended Clock management functions
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

#ifdef HAL_RCC_MODULE_ENABLED

/** @defgroup RCCEx RCCEx
  * @brief RCC Extension HAL module driver.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup RCCEx_Private_Constants RCCEx Private Constants
  * @{
  */
/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/** @defgroup RCCEx_Private_Macros RCCEx Private Macros
  * @{
  */
/**
  * @}
  */

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup RCCEx_Exported_Functions RCCEx Exported Functions
  * @{
  */

/** @defgroup RCCEx_Exported_Functions_Group1 Peripheral Control functions
  *  @brief  Extended Peripheral Control functions
  *
@verbatim
 ===============================================================================
                ##### Extended Peripheral Control functions  #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to control the RCC Clocks
    frequencies.
    [..]
    (@) Important note: Care must be taken when HAL_RCCEx_PeriphCLKConfig() is used to
        select the RTC clock source; in this case the Backup domain will be reset in
        order to modify the RTC Clock source, as consequence RTC registers (including
        the backup registers) are set to their reset values.

@endverbatim
  * @{
  */

/**
  * @brief  Initializes the RCC extended peripherals clocks according to the specified parameters in the
  *         RCC_PeriphCLKInitTypeDef.
  * @param  PeriphClkInit pointer to an RCC_PeriphCLKInitTypeDef structure that
  *         contains the configuration information for the Extended Peripherals clocks(RTC clock).
  *
  * @note   Care must be taken when HAL_RCCEx_PeriphCLKConfig() is used to select
  *         the RTC clock source; in this case the Backup domain will be reset in
  *         order to modify the RTC Clock source, as consequence RTC registers (including
  *         the backup registers) are set to their reset values.
  *
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_RCCEx_PeriphCLKConfig(RCC_PeriphCLKInitTypeDef  *PeriphClkInit)
{
  uint32_t tickstart = 0U, temp_reg = 0U;

  /* Check the parameters */
  assert_param(IS_RCC_PERIPHCLOCK(PeriphClkInit->PeriphClockSelection));

  /*------------------------------- RTC/LCD Configuration ------------------------*/
  if ((((PeriphClkInit->PeriphClockSelection) & RCC_PERIPHCLK_RTC) == RCC_PERIPHCLK_RTC))
  {
    /* check for RTC Parameters used to output RTCCLK */
    assert_param(IS_RCC_RTCCLKSOURCE(PeriphClkInit->RtcClockSelection));

    FlagStatus       pwrclkchanged = RESET;

    /* As soon as function is called to change RTC clock source, activation of the
       power domain is done. */
    /* Requires to enable write access to Backup Domain of necessary */
    if (__HAL_RCC_PWR_IS_CLK_DISABLED())
    {
      __HAL_RCC_PWR_CLK_ENABLE();
      pwrclkchanged = SET;
    }

    if (HAL_IS_BIT_CLR(PWR->CR, PWR_CR_DBP))
    {
      /* Enable write access to Backup domain */
      SET_BIT(PWR->CR, PWR_CR_DBP);

      /* Wait for Backup domain Write protection disable */
      tickstart = HAL_GetTick();

      while (HAL_IS_BIT_CLR(PWR->CR, PWR_CR_DBP))
      {
        if ((HAL_GetTick() - tickstart) > RCC_DBP_TIMEOUT_VALUE)
        {
          return HAL_TIMEOUT;
        }
      }
    }

    /* Reset the Backup domain only if the RTC Clock source selection is modified from reset value */
    temp_reg = (RCC->BDCR & RCC_BDCR_RTCSEL);
    if ((temp_reg != 0x00000000U) && (temp_reg != (PeriphClkInit->RtcClockSelection & RCC_BDCR_RTCSEL)))
    {
      /* Store the content of BDCR register before the reset of Backup Domain */
      temp_reg = (RCC->BDCR & ~(RCC_BDCR_RTCSEL));
      /* RTC Clock selection can be changed only if the Backup Domain is reset */
      __HAL_RCC_BACKUPRESET_FORCE();
      __HAL_RCC_BACKUPRESET_RELEASE();
      /* Restore the Content of BDCR register */
      RCC->BDCR = temp_reg;

      /* Wait for LSERDY if LSE was enabled */
      if (HAL_IS_BIT_SET(temp_reg, RCC_BDCR_LSEON))
      {
        /* Get Start Tick */
        tickstart = HAL_GetTick();

        /* Wait till LSE is ready */
        while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET)
        {
          if ((HAL_GetTick() - tickstart) > RCC_LSE_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
    }
    __HAL_RCC_RTC_CONFIG(PeriphClkInit->RtcClockSelection);

    /* Require to disable power clock if necessary */
    if (pwrclkchanged == SET)
    {
      __HAL_RCC_PWR_CLK_DISABLE();
    }
  }

  /*------------------------------ ADC clock Configuration ------------------*/
  if (((PeriphClkInit->PeriphClockSelection) & RCC_PERIPHCLK_ADC) == RCC_PERIPHCLK_ADC)
  {
    /* Check the parameters */
    assert_param(IS_RCC_ADCPLLCLK_DIV(PeriphClkInit->AdcClockSelection));

    /* Configure the ADC clock source */
    __HAL_RCC_ADC_CONFIG(PeriphClkInit->AdcClockSelection);
  }
  
  /*------------------------------ USB clock Configuration ------------------*/
  if (((PeriphClkInit->PeriphClockSelection) & RCC_PERIPHCLK_USB) == RCC_PERIPHCLK_USB)
  {
    /* Check the parameters */
    assert_param(IS_RCC_USBCLKSOURCE(PeriphClkInit->UsbClockSelection));
    
    if((PeriphClkInit->UsbClockSelection) != RCC_USBCLKSOURCE_HSI48M)
    {
      /* Set USB clock source as PLL frequency */
      CLEAR_BIT(RCC->CFGR1,RCC_CFGR1_USBSELHSI48);
      
      /* Configure the USB clock source */
      __HAL_RCC_USB_CONFIG(PeriphClkInit->UsbClockSelection);
    }
    else
    {
      /* Set USB clock source as HSI48M */
      SET_BIT(RCC->CFGR1,RCC_CFGR1_USBSELHSI48);
    }
  }
  
  /*------------------------------ CAN clock Configuration ------------------*/
  if (((PeriphClkInit->PeriphClockSelection) & RCC_PERIPHCLK_CAN) == RCC_PERIPHCLK_CAN)
  {
    /* Check the parameters */
    assert_param(IS_RCC_CANCLKSOURCE(PeriphClkInit->CanClockSelection));

    /* Configure the CAN clock source */
    __HAL_RCC_CAN_CONFIG(PeriphClkInit->CanClockSelection);
  }

  return HAL_OK;
}

/**
  * @brief  Get the PeriphClkInit according to the internal
  * RCC configuration registers.
  * @param  PeriphClkInit pointer to an RCC_PeriphCLKInitTypeDef structure that
  *         returns the configuration information for the Extended Peripherals clocks(RTC, I2S, ADC clocks).
  * @retval None
  */
void HAL_RCCEx_GetPeriphCLKConfig(RCC_PeriphCLKInitTypeDef  *PeriphClkInit)
{
  uint32_t srcclk = 0U;

  /* Set all possible values for the extended clock type parameter------------*/
  PeriphClkInit->PeriphClockSelection = RCC_PERIPHCLK_RTC;

  /* Get the RTC configuration -----------------------------------------------*/
  srcclk = __HAL_RCC_GET_RTC_SOURCE();
  /* Source clock is LSE or LSI*/
  PeriphClkInit->RtcClockSelection = srcclk;

  /* Get the ADC clock configuration -----------------------------------------*/
  PeriphClkInit->PeriphClockSelection |= RCC_PERIPHCLK_ADC;
  PeriphClkInit->AdcClockSelection = __HAL_RCC_GET_ADC_SOURCE();

  /* Get the USB clock configuration -----------------------------------------*/
  PeriphClkInit->PeriphClockSelection |= RCC_PERIPHCLK_USB;
  if(READ_BIT(RCC->CFGR1,RCC_USBCLKSOURCE_HSI48M) != RCC_USBCLKSOURCE_HSI48M)
  {
    PeriphClkInit->UsbClockSelection = __HAL_RCC_GET_USB_SOURCE();
  }
  else
  {
    PeriphClkInit->UsbClockSelection = RCC_USBCLKSOURCE_HSI48M;
  }

  /* Get the CAN clock configuration -----------------------------------------*/
  PeriphClkInit->PeriphClockSelection |= RCC_PERIPHCLK_CAN;
  PeriphClkInit->CanClockSelection = __HAL_RCC_GET_CAN_SOURCE();
}

/**
  * @brief  Returns the peripheral clock frequency
  * @note   Returns 0 if peripheral clock is unknown
  * @param  PeriphClk Peripheral clock identifier
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_PERIPHCLK_RTC  RTC peripheral clock
  *            @arg @ref RCC_PERIPHCLK_ADC  ADC peripheral clock
  *            @arg @ref RCC_PERIPHCLK_USB  USB peripheral clock
  *            @arg @ref RCC_PERIPHCLK_CAN  CAN peripheral clock
  * @retval Frequency in Hz (0: means that no available frequency for the peripheral)
  */
uint32_t HAL_RCCEx_GetPeriphCLKFreq(uint32_t PeriphClk)
{
  const uint8_t aPLLMULFactorTable[64] = {2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, \
                                        18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, \
                                        34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, \
                                        50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 63, 63};
                                          
  const uint8_t aPredivFactorTable[2] = {1, 2};

  uint32_t tmpreg = 0U, prediv = 0U, pllclk = 0U, pllmul = 0U;

  uint32_t  usbprediv = 0U;
  uint32_t temp_reg = 0U, frequency = 0U;

  /* Check the parameters */
  assert_param(IS_RCC_PERIPHCLOCK(PeriphClk));

  switch (PeriphClk)
  {
    case RCC_PERIPHCLK_RTC:
    {
      /* Get RCC BDCR configuration ------------------------------------------------------*/
      temp_reg = RCC->BDCR;

      /* Check if LSE is ready if RTC clock selection is LSE */
      if (((temp_reg & RCC_BDCR_RTCSEL) == RCC_RTCCLKSOURCE_LSE) && (HAL_IS_BIT_SET(temp_reg, RCC_BDCR_LSERDY)))
      {
        frequency = LSE_VALUE;
      }
      /* Check if LSI is ready if RTC clock selection is LSI */
      else if (((temp_reg & RCC_BDCR_RTCSEL) == RCC_RTCCLKSOURCE_LSI) && (HAL_IS_BIT_SET(RCC->CSR, RCC_CSR_LSIRDY)))
      {
        frequency = LSI_VALUE;
      }
      else if (((temp_reg & RCC_BDCR_RTCSEL) == RCC_RTCCLKSOURCE_HSE_DIV128) && (HAL_IS_BIT_SET(RCC->CR, RCC_CR_HSERDY)))
      {
        frequency = HSE_VALUE / 128U;
      }
      /* Clock not enabled for RTC*/
      else
      {
        /* nothing to do: frequency already initialized to 0U */
      }
      break;
    }
    case RCC_PERIPHCLK_ADC:
    {
      switch(__HAL_RCC_GET_ADC_SOURCE())
      {
        case RCC_ADCPCLK2_DIV2:
        {
          frequency = HAL_RCC_GetPCLK2Freq() / 2;
          break;
        }
        case RCC_ADCPCLK2_DIV4:
        {
          frequency = HAL_RCC_GetPCLK2Freq() / 4;
          break;
        }
        case RCC_ADCPCLK2_DIV6:
        {
          frequency = HAL_RCC_GetPCLK2Freq() / 6;
          break;
        }
        case RCC_ADCPCLK2_DIV8:
        {
          frequency = HAL_RCC_GetPCLK2Freq() / 8;
          break;
        }
        case RCC_ADCPCLK2_DIV12:
        {
          frequency = HAL_RCC_GetPCLK2Freq() / 12;
          break;
        }
        case RCC_ADCPCLK2_DIV16:
        {
          frequency = HAL_RCC_GetPCLK2Freq() / 16;
          break;
        }
        default:
        {
          break;
        }
      }
      break;
    }
    
    case RCC_PERIPHCLK_USB:
    {
      /* Get RCC CFGR1 configuration ------------------------------------------------------*/
      temp_reg = RCC->CFGR1;
      
      if(((temp_reg & RCC_USBCLKSOURCE_HSI48M) == RCC_USBCLKSOURCE_HSI48M) && (HAL_IS_BIT_SET(RCC->CFGR1, RCC_CFGR1_HSI48RDY)))
      {
        frequency = HSI48_VALUE;
      }
      else if(((temp_reg & RCC_USBCLKSOURCE_HSI48M) != RCC_USBCLKSOURCE_HSI48M) && (HAL_IS_BIT_SET(RCC->CR, RCC_CR_PLLRDY)))
      {
        tmpreg = RCC->CFGR;
        pllmul = aPLLMULFactorTable[(((tmpreg&(RCC_CFGR_PLLMULL_4|RCC_CFGR_PLLMULL_5))>>7) | (tmpreg&(RCC_CFGR_PLLMULL_0| \
                                     RCC_CFGR_PLLMULL_1|RCC_CFGR_PLLMULL_2|RCC_CFGR_PLLMULL_3)))>>RCC_CFGR_PLLMULL_Pos];
        if (((tmpreg & (RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE)) == RCC_PLLSOURCE_HSE) || ((tmpreg & (RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE)) == RCC_PLLSOURCE_HSE_DIV2))
        {
          prediv = aPredivFactorTable[(uint32_t)(RCC->CFGR & RCC_CFGR_PLLXTPRE) >> RCC_CFGR_PLLXTPRE_Pos];
          /* HSE used as PLL clock source : PLLCLK = HSE/PREDIV1 * PLLMUL */
          pllclk = (uint32_t)((HSE_VALUE  * pllmul) / prediv);
        }
        else
        {
          /* HSI used as PLL clock source : PLLCLK = HSI/2 * PLLMUL */
          pllclk = (uint32_t)((HSI_VALUE >> 1) * pllmul);
        }

        usbprediv = __HAL_RCC_GET_USB_SOURCE();
        switch(usbprediv)
        {
          case RCC_USBCLKSOURCE_PLL:
          {
            frequency = pllclk;
            break;
          }
          case RCC_USBCLKSOURCE_PLL_DIV1_5:
          {
            frequency = (pllclk * 2) /3;
            break;
          }
          case RCC_USBCLKSOURCE_PLL_DIV2:
          {
            frequency = pllclk / 2;
            break;
          }
          case RCC_USBCLKSOURCE_PLL_DIV2_5:
          {
            frequency = (pllclk * 2) / 5;
            break;
          }
          case RCC_USBCLKSOURCE_PLL_DIV3:
          {
            frequency = pllclk / 3;
            break;
          }
          case RCC_USBCLKSOURCE_PLL_DIV3_5:
          {
            frequency = (pllclk * 2) / 7;
            break;
          }
          case RCC_USBCLKSOURCE_PLL_DIV4:
          {
            frequency = pllclk / 4;
            break;
          }
          default:
          {
            break;
          }
        }
      }
      else
      {
        /* nothing to do: frequency already initialized to 0U */
      }
      break;
    }
    
    case RCC_PERIPHCLK_CAN:
    {
      /* Get RCC CFGR1 configuration ------------------------------------------------------*/
      temp_reg = RCC->CFGR2;
      
      if(((temp_reg & RCC_CFGR2_CANCKSEL) != RCC_CANCLKSOURCE_HSE) && ((temp_reg & RCC_CANCLKSOURCE_HSE) == RCC_CANCLKSOURCE_HSE))
      {
        /* No clock */
      }
      else if(((temp_reg & RCC_CFGR2_CANCKSEL) == RCC_CANCLKSOURCE_HSE) && (HAL_IS_BIT_SET(RCC->CR, RCC_CR_HSERDY)))
      {
        frequency = HSE_VALUE;
      }
      else
      {
        if(HAL_IS_BIT_SET(RCC->CR, RCC_CR_PLLRDY))
        {
          /* PLL / CANPLLDIV */
          tmpreg = RCC->CFGR;
          pllmul = aPLLMULFactorTable[(((tmpreg&(RCC_CFGR_PLLMULL_4|RCC_CFGR_PLLMULL_5))>>7) | (tmpreg&(RCC_CFGR_PLLMULL_0| \
                                       RCC_CFGR_PLLMULL_1|RCC_CFGR_PLLMULL_2|RCC_CFGR_PLLMULL_3)))>>RCC_CFGR_PLLMULL_Pos];
          if (((tmpreg & (RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE)) == RCC_PLLSOURCE_HSE) || ((tmpreg & (RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE)) == RCC_PLLSOURCE_HSE_DIV2))
          {
            prediv = aPredivFactorTable[(uint32_t)(RCC->CFGR & RCC_CFGR_PLLXTPRE) >> RCC_CFGR_PLLXTPRE_Pos];
            /* HSE used as PLL clock source : PLLCLK = HSE/PREDIV1 * PLLMUL */
            pllclk = (uint32_t)((HSE_VALUE  * pllmul) / prediv);
          }
          else
          {
            /* HSI used as PLL clock source : PLLCLK = HSI/2 * PLLMUL */
            pllclk = (uint32_t)((HSI_VALUE >> 1) * pllmul);
          }
          
          frequency = (pllclk / ((__HAL_RCC_GET_CAN_SOURCE() >> RCC_CFGR2_CANCKSEL_Pos) + 1));
        }
      }
      break;
    }    
    
    default:
    {
      break;
    }
  }
  
  return (frequency);
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_RCC_MODULE_ENABLED */

/**
  * @}
  */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/

