/**
 ******************************************************************************
 * @file    py32f4xx_hal_msp.c
 * @author  MCU Application Team
 * @brief   This file provides code for the MSP Initialization
 *          and de-Initialization codes.
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
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* External functions --------------------------------------------------------*/

/**
 * @brief Initialize global MSP
 */
void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}

/**
 * @brief Initialize SPI related MSP
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  /* Initialize SPI1 */
  if (hspi->Instance == SPI1)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();  /* Enable GPIOB clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();  /* Enable GPIOA clock */
    __HAL_RCC_SYSCFG_CLK_ENABLE(); /* Enable SYSCFG clock */
    __HAL_RCC_SPI1_CLK_ENABLE();   /* Enable SPI1 clock */
    __HAL_RCC_DMA1_CLK_ENABLE();   /* Enable DMA clock */

    /*SCK*/
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    if (hspi->Init.CLKPolarity == SPI_POLARITY_LOW)
    {
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    }
    else
    {
      GPIO_InitStruct.Pull = GPIO_PULLUP;
    }
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF3_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* GPIO configured as SPI：MISO/MOSI*/
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF3_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /* Interrupt configuration */
    HAL_NVIC_SetPriority(SPI1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
    /* DMA_CH1 configuration */
    dma_handle_ch1.Instance = DMA1_Channel1;
    dma_handle_ch1.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma_handle_ch1.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_handle_ch1.Init.MemInc = DMA_MINC_ENABLE;
    if (hspi->Init.DataSize <= SPI_DATASIZE_8BIT)
    {
      dma_handle_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
      dma_handle_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    }
    else
    {
      dma_handle_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      dma_handle_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    }
    dma_handle_ch1.Init.Mode = DMA_NORMAL;
    dma_handle_ch1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    /* Initialize DMA */
    HAL_DMA_Init(&dma_handle_ch1);
    /* DMA handle is associated with SPI handle */
    __HAL_LINKDMA(hspi, hdmatx, dma_handle_ch1);

    /* DMA_CH2 configuration */
    dma_handle_ch2.Instance = DMA1_Channel2;
    dma_handle_ch2.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_handle_ch2.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_handle_ch2.Init.MemInc = DMA_MINC_ENABLE;
    if (hspi->Init.DataSize <= SPI_DATASIZE_8BIT)
    {
      dma_handle_ch2.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
      dma_handle_ch2.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    }
    else
    {
      dma_handle_ch2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      dma_handle_ch2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    }
    dma_handle_ch2.Init.Mode = DMA_NORMAL;
    dma_handle_ch2.Init.Priority = DMA_PRIORITY_LOW;
    /* Initialize DMA*/
    HAL_DMA_Init(&dma_handle_ch2);
    /* DMA handle is associated with SPI handle*/
    __HAL_LINKDMA(hspi, hdmarx, dma_handle_ch2);

    /* Set DMA channel map. */
    HAL_DMA_ChannelMap(&dma_handle_ch1, DMA_CHANNEL_MAP_SPI1_WR); /* SPI1_TX DMA1_CH1 */
    HAL_DMA_ChannelMap(&dma_handle_ch2, DMA_CHANNEL_MAP_SPI1_RD); /* SPI1_RX DMA1_CH2 */

    /* DMA interrupt configuration*/
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  }
}

/**
 * @brief Deinit SPI MSP
 */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI1)
  {
    /* Reset SPI peripheral */
    __HAL_RCC_SPI1_FORCE_RESET();
    __HAL_RCC_SPI1_RELEASE_RESET();

    /* Disable SPI and GPIO clock */
    /* Deinit SPI SCK */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);

    HAL_NVIC_DisableIRQ(SPI1_IRQn);

    HAL_DMA_DeInit(&dma_handle_ch1);
    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
  }
}
/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
