/**
******************************************************************************
* @file    PY32F403xC.h
* @brief   CMSIS Cortex-M4 Device Peripheral Access Layer Header File.
*          This file contains all the peripheral register's definitions, bits
*          definitions and memory mapping for PY32F4xx devices.
* @version v1.0.0
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

#ifndef __PY32F403XC_H
#define __PY32F403XC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @brief Configuration of the Cortex-M4 Processor and Core Peripherals
*/
#define __CM4_REV                 0x0001U  /*!< Core revision r0p1                            */
#define __MPU_PRESENT             0U       /*!< PY32F4xx provides an MPU                      */
#define __NVIC_PRIO_BITS          3U       /*!< PY32F4xx uses 3 Bits for the Priority Levels  */
#define __Vendor_SysTickConfig    0U       /*!< Set to 1 if different SysTick Config is used  */
#define __FPU_PRESENT             1U       /*!< FPU present                                   */

/**
 * @brief PY32F4xx Interrupt Number Definition, according to the selected device
 *        in @ref Library_configuration_section
 */
typedef enum
{
    /******  Cortex-M4 Processor Exceptions Numbers **************************************************************/
    NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                                          */
    MemoryManagement_IRQn       = -12,    /*!< 4 Cortex-M4 Memory Management Interrupt                           */
    BusFault_IRQn               = -11,    /*!< 5 Cortex-M4 Bus Fault Interrupt                                   */
    UsageFault_IRQn             = -10,    /*!< 6 Cortex-M4 Usage Fault Interrupt                                 */
    SVCall_IRQn                 = -5,     /*!< 11 Cortex-M4 SV Call Interrupt                                    */
    DebugMonitor_IRQn           = -4,     /*!< 12 Cortex-M4 Debug Monitor Interrupt                              */
    PendSV_IRQn                 = -2,     /*!< 14 Cortex-M4 Pend SV Interrupt                                    */
    SysTick_IRQn                = -1,     /*!< 15 Cortex-M4 System Tick Interrupt                                */
    WWDG_IRQn                   = 0,      /*!< Window WatchDog Interrupt                                         */
    PVD_IRQn                    = 1,      /*!< PVD through EXTI Line detection Interrupt                         */
    TAMPER_IRQn                 = 2,      /*!< Tamper Interrupt                                                  */
    RTC_IRQn                    = 3,      /*!< RTC global Interrupt                                              */
    FLASH_IRQn                  = 4,      /*!< FLASH global Interrupt                                            */
    RCC_CTC_IRQn                = 5,      /*!< RCC and CTC global Interrupt                                      */
    EXTI0_IRQn                  = 6,      /*!< EXTI Line0 Interrupt                                              */
    EXTI1_IRQn                  = 7,      /*!< EXTI Line1 Interrupt                                              */
    EXTI2_IRQn                  = 8,      /*!< EXTI Line2 Interrupt                                              */
    EXTI3_IRQn                  = 9,      /*!< EXTI Line3 Interrupt                                              */
    EXTI4_IRQn                  = 10,     /*!< EXTI Line4 Interrupt                                              */
    DMA1_Channel1_IRQn          = 11,     /*!< DMA1 Channel 1 global Interrupt                                   */
    DMA1_Channel2_IRQn          = 12,     /*!< DMA1 Channel 2 global Interrupt                                   */
    DMA1_Channel3_IRQn          = 13,     /*!< DMA1 Channel 3 global Interrupt                                   */
    DMA1_Channel4_IRQn          = 14,     /*!< DMA1 Channel 4 global Interrupt                                   */
    DMA1_Channel5_IRQn          = 15,     /*!< DMA1 Channel 5 global Interrupt                                   */
    DMA1_Channel6_IRQn          = 16,     /*!< DMA1 Channel 6 global Interrupt                                   */
    DMA1_Channel7_IRQn          = 17,     /*!< DMA1 Channel 7 global Interrupt                                   */
    ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                                    */
    USB_IRQn                    = 19,     /*!< USB Interrupt                                                     */
    CAN_IRQn                    = 20,     /*!< CAN Interrupt                                                     */
    EXTI9_5_IRQn                = 23,     /*!< External Line[9:5] Interrupts                                     */
    TIM1_BRK_TIM9_IRQn          = 24,     /*!< TIM1 Break Interrupt and TIM9 global Interrupt                    */
    TIM1_UP_TIM10_IRQn          = 25,     /*!< TIM1 Update Interrupt and TIM10 global Interrupt                  */
    TIM1_TRG_COM_TIM11_IRQn     = 26,     /*!< TIM1 Trigger and Commutation Interrupt and TIM11 global interrupt */
    TIM1_CC_IRQn                = 27,     /*!< TIM1 Capture Compare Interrupt                                    */
    TIM2_IRQn                   = 28,     /*!< TIM2 global Interrupt                                             */
    TIM3_IRQn                   = 29,     /*!< TIM3 global Interrupt                                             */
    TIM4_IRQn                   = 30,     /*!< TIM4 global Interrupt                                             */
    I2C1_EV_IRQn                = 31,     /*!< I2C1 Event Interrupt                                              */
    I2C1_ER_IRQn                = 32,     /*!< I2C1 Error Interrupt                                              */
    I2C2_EV_IRQn                = 33,     /*!< I2C2 Event Interrupt                                              */
    I2C2_ER_IRQn                = 34,     /*!< I2C2 Error Interrupt                                              */
    SPI1_IRQn                   = 35,     /*!< SPI1 global Interrupt                                             */
    SPI2_IRQn                   = 36,     /*!< SPI2 global Interrupt                                             */
    USART1_IRQn                 = 37,     /*!< USART1 global Interrupt                                           */
    USART2_IRQn                 = 38,     /*!< USART2 global Interrupt                                           */
    USART3_IRQn                 = 39,     /*!< USART3 global Interrupt                                           */
    EXTI15_10_IRQn              = 40,     /*!< External Line[15:10] Interrupts                                   */
    RTC_Alarm_IRQn              = 41,     /*!< RTC Alarm through EXTI Line Interrupt                             */
    TIM8_BRK_TIM12_IRQn         = 43,     /*!< TIM8 Break Interrupt and TIM12 global Interrupt                   */
    TIM8_UP_TIM13_IRQn          = 44,     /*!< TIM8 Update Interrupt and TIM13 global Interrupt                  */
    TIM8_TRG_COM_TIM14_IRQn     = 45,     /*!< TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt */
    TIM8_CC_IRQn                = 46,     /*!< TIM8 Capture Compare Interrupt                                    */
    ADC3_IRQn                   = 47,     /*!< ADC3 global Interrupt                                             */
    ESMC_IRQn                   = 48,     /*!< ESMC global Interrupt                                             */
    SDIO_IRQn                   = 49,     /*!< SDIO global Interrupt                                             */
    TIM5_IRQn                   = 50,     /*!< TIM5 global Interrupt                                             */
    SPI3_IRQn                   = 51,     /*!< SPI3 global Interrupt                                             */
    USART4_IRQn                 = 52,     /*!< USART4 global Interrupt                                           */
    USART5_IRQn                 = 53,     /*!< USART5 global Interrupt                                           */
    TIM6_IRQn                   = 54,     /*!< TIM6 global Interrupt                                             */
    TIM7_IRQn                   = 55,     /*!< TIM7 global Interrupt                                             */
    DMA2_Channel1_IRQn          = 56,     /*!< DMA2 Channel 1 global Interrupt                                   */
    DMA2_Channel2_IRQn          = 57,     /*!< DMA2 Channel 2 global Interrupt                                   */
    DMA2_Channel3_IRQn          = 58,     /*!< DMA2 Channel 3 global Interrupt                                   */
    DMA2_Channel4_5_IRQn        = 59,     /*!< DMA2 Channel 4 and Channel 5 global Interrupt                     */
} IRQn_Type;

#include "core_cm4.h"             /* Cortex-M4 processor and core peripherals */
#include "system_py32f4xx.h"
#include <stdint.h>

/**
* @brief ADC Registers
  */
typedef struct
{
    __IO uint32_t SR;          /*!< ADC desc SR,      Address offset: 0x00 */
    __IO uint32_t CR1;         /*!< ADC desc CR1,     Address offset: 0x04 */
    __IO uint32_t CR2;         /*!< ADC desc CR2,     Address offset: 0x08 */
    __IO uint32_t SMPR1;       /*!< ADC desc SMPR1,   Address offset: 0x0C */
    __IO uint32_t SMPR2;       /*!< ADC desc SMPR2,   Address offset: 0x10 */
    __IO uint32_t JOFR1;       /*!< ADC desc JOFR1,   Address offset: 0x14 */
    __IO uint32_t JOFR2;       /*!< ADC desc JOFR2,   Address offset: 0x18 */
    __IO uint32_t JOFR3;       /*!< ADC desc JOFR3,   Address offset: 0x1C */
    __IO uint32_t JOFR4;       /*!< ADC desc JOFR4,   Address offset: 0x20 */
    __IO uint32_t HTR;         /*!< ADC desc HTR,     Address offset: 0x24 */
    __IO uint32_t LTR;         /*!< ADC desc LTR,     Address offset: 0x28 */
    __IO uint32_t SQR1;        /*!< ADC desc SQR1,    Address offset: 0x2C */
    __IO uint32_t SQR2;        /*!< ADC desc SQR2,    Address offset: 0x30 */
    __IO uint32_t SQR3;        /*!< ADC desc SQR3,    Address offset: 0x34 */
    __IO uint32_t JSQR;        /*!< ADC desc JSQR,    Address offset: 0x38 */
    __IO uint32_t JDR1;        /*!< ADC desc JDR1,    Address offset: 0x3C */
    __IO uint32_t JDR2;        /*!< ADC desc JDR2,    Address offset: 0x40 */
    __IO uint32_t JDR3;        /*!< ADC desc JDR3,    Address offset: 0x44 */
    __IO uint32_t JDR4;        /*!< ADC desc JDR4,    Address offset: 0x48 */
    __IO uint32_t DR;          /*!< ADC desc DR,      Address offset: 0x4C */
    __IO uint32_t CCSR;        /*!< ADC desc CCSR,    Address offset: 0x50 */
    __IO uint32_t CALRR1;      /*!< ADC desc CALRR1,  Address offset: 0x54 */
    __IO uint32_t CALRR2;      /*!< ADC desc CALRR2,  Address offset: 0x58 */
    __IO uint32_t CALFIR1;     /*!< ADC desc CALFIR1, Address offset: 0x5C */
    __IO uint32_t CALFIR2;     /*!< ADC desc CALFIR2, Address offset: 0x60 */
}ADC_TypeDef;

/**
* @brief BKP Registers
*/
typedef struct
{
    uint8_t RESERVED0[4];      /*!< Reserved, Address offset: 0x00 */
    __IO uint32_t DR1;         /*!< BKP desc DR1,   Address offset: 0x04 */
    __IO uint32_t DR2;         /*!< BKP desc DR2,   Address offset: 0x08 */
    __IO uint32_t DR3;         /*!< BKP desc DR3,   Address offset: 0x0C */
    __IO uint32_t DR4;         /*!< BKP desc DR4,   Address offset: 0x10 */
    __IO uint32_t DR5;         /*!< BKP desc DR5,   Address offset: 0x14 */
    __IO uint32_t DR6;         /*!< BKP desc DR6,   Address offset: 0x18 */
    __IO uint32_t DR7;         /*!< BKP desc DR7,   Address offset: 0x1C */
    __IO uint32_t DR8;         /*!< BKP desc DR8,   Address offset: 0x20 */
    __IO uint32_t DR9;         /*!< BKP desc DR9,   Address offset: 0x24 */
    __IO uint32_t DR10;        /*!< BKP desc DR10,  Address offset: 0x28 */
    __IO uint32_t RTCCR;       /*!< BKP desc RTCCR, Address offset: 0x2C */
    __IO uint32_t CR;          /*!< BKP desc CR,    Address offset: 0x30 */
    __IO uint32_t CSR;         /*!< BKP desc CSR,   Address offset: 0x34 */
    uint8_t RESERVED13[8];     /*!< Reserved, Address offset: 0x38 - 0x3F */
    __IO uint32_t DR11;        /*!< BKP desc DR11,  Address offset: 0x40 */
    __IO uint32_t DR12;        /*!< BKP desc DR12,  Address offset: 0x44 */
    __IO uint32_t DR13;        /*!< BKP desc DR13,  Address offset: 0x48 */
    __IO uint32_t DR14;        /*!< BKP desc DR14,  Address offset: 0x4C */
    __IO uint32_t DR15;        /*!< BKP desc DR15,  Address offset: 0x50 */
    __IO uint32_t DR16;        /*!< BKP desc DR16,  Address offset: 0x54 */
    __IO uint32_t DR17;        /*!< BKP desc DR17,  Address offset: 0x58 */
    __IO uint32_t DR18;        /*!< BKP desc DR18,  Address offset: 0x5C */
    __IO uint32_t DR19;        /*!< BKP desc DR19,  Address offset: 0x60 */
    __IO uint32_t DR20;        /*!< BKP desc DR20,  Address offset: 0x64 */
    __IO uint32_t DR21;        /*!< BKP desc DR21,  Address offset: 0x68 */
    __IO uint32_t DR22;        /*!< BKP desc DR22,  Address offset: 0x6C */
    __IO uint32_t DR23;        /*!< BKP desc DR23,  Address offset: 0x70 */
    __IO uint32_t DR24;        /*!< BKP desc DR24,  Address offset: 0x74 */
    __IO uint32_t DR25;        /*!< BKP desc DR25,  Address offset: 0x78 */
    __IO uint32_t DR26;        /*!< BKP desc DR26,  Address offset: 0x7C */
    __IO uint32_t DR27;        /*!< BKP desc DR27,  Address offset: 0x80 */
    __IO uint32_t DR28;        /*!< BKP desc DR28,  Address offset: 0x84 */
    __IO uint32_t DR29;        /*!< BKP desc DR29,  Address offset: 0x88 */
    __IO uint32_t DR30;        /*!< BKP desc DR30,  Address offset: 0x8C */
    __IO uint32_t DR31;        /*!< BKP desc DR31,  Address offset: 0x90 */
    __IO uint32_t DR32;        /*!< BKP desc DR32,  Address offset: 0x94 */
    __IO uint32_t DR33;        /*!< BKP desc DR33,  Address offset: 0x98 */
    __IO uint32_t DR34;        /*!< BKP desc DR34,  Address offset: 0x9C */
    __IO uint32_t DR35;        /*!< BKP desc DR35,  Address offset: 0xA0 */
    __IO uint32_t DR36;        /*!< BKP desc DR36,  Address offset: 0xA4 */
    __IO uint32_t DR37;        /*!< BKP desc DR37,  Address offset: 0xA8 */
    __IO uint32_t DR38;        /*!< BKP desc DR38,  Address offset: 0xAC */
    __IO uint32_t DR39;        /*!< BKP desc DR39,  Address offset: 0xB0 */
    __IO uint32_t DR40;        /*!< BKP desc DR40,  Address offset: 0xB4 */
    __IO uint32_t DR41;        /*!< BKP desc DR41,  Address offset: 0xB8 */
    __IO uint32_t DR42;        /*!< BKP desc DR42,  Address offset: 0xBC */
}BKP_TypeDef;


/**
* @brief CANFD LLC Acceptance filters Registers
*/
typedef struct
{
    __IO uint32_t ID;
    __IO uint32_t FORMAT;
    __IO uint32_t TYPE;
    // __IO uint32_t AF;
    __IO uint32_t Reserved;
}CANFD_LLC_AC_TypeDef;

/**
* @brief CANFD LLC Acceptance filters Registers
*/
typedef struct
{
    __IO uint32_t ID;
    __IO uint32_t FORMAT;
    __IO uint32_t TYPE;
    // __IO uint32_t AF;
    __IO uint32_t Reserved1;
    // __IO uint32_t RTSL;
    __IO uint32_t Reserved2;
    // __IO uint32_t RTSH;
    __IO uint32_t Reserved3;
    __IO uint32_t TTCAN;
    __IO uint32_t DATA[16];
}CANFD_LLC_TypeDef;

/**
* @brief CANFD Registers
*/
typedef struct
{
    __IO uint32_t           TSNCR;       /*!< CANFD desc TSNCR,   Address offset: 0x00 */
    __IO uint32_t           ACBTR;       /*!< CANFD desc ACBTR,   Address offset: 0x04 */
    __IO uint32_t           FDBTR;       /*!< CANFD desc FDBTR,   Address offset: 0x08 */
    // __IO uint32_t           XLBTR;       /*!< CANFD desc XLBTR,   Address offset: 0x0C */
    __IO uint32_t           Reserved1;
    __IO uint32_t           RLSSP;       /*!< CANFD desc RLSSP,   Address offset: 0x10 */
    __IO uint32_t           IFR;         /*!< CANFD desc IFR,     Address offset: 0x14 */
    __IO uint32_t           IER;         /*!< CANFD desc IER,     Address offset: 0x18 */
    __IO uint32_t           TSR;         /*!< CANFD desc TSR,     Address offset: 0x1C */
    // __IO uint32_t           TTSL;        /*!< CANFD desc TTSL,    Address offset: 0x20 */
    // __IO uint32_t           TTSH;        /*!< CANFD desc TTSH,    Address offset: 0x24 */
    __IO uint32_t Reserved2;
    __IO uint32_t Reserved3;
    __IO uint32_t           MCR;         /*!< CANFD desc MCR,     Address offset: 0x28 */
    __IO uint32_t           WECR;        /*!< CANFD desc WECR,    Address offset: 0x2C */
    __IO uint32_t           REFMSG;      /*!< CANFD desc REFMSG,  Address offset: 0x30 */
    __IO uint32_t           TTCR;        /*!< CANFD desc TTCR,    Address offset: 0x34 */
    __IO uint32_t           TTTR;        /*!< CANFD desc TTTR,    Address offset: 0x38 */
    __IO uint32_t           SCMS;        /*!< CANFD desc SCMS,    Address offset: 0x3C */
    // __IO uint32_t           MESR;        /*!< CANFD desc MESR,    Address offset: 0x40 */
    __IO uint32_t Reserved4;
    __IO uint32_t           ACFCR;       /*!< CANFD desc ACFCR,   Address offset: 0x44 */
    CANFD_LLC_AC_TypeDef    ACFC;        /*!< CANFD desc ACFC,    Address offset: 0x48 - 0x57 */
    CANFD_LLC_AC_TypeDef    ACFM;        /*!< CANFD desc ACFM,    Address offset: 0x58 - 0x67 */
    uint8_t        RESERVED18[8];        /*!< Reserved,           Address offset: 0x68 - 0x6F */
    CANFD_LLC_TypeDef       RBUF;        /*!< CANFD desc RBUF,    Address offset: 0x70 - 0xCB */
    uint8_t        RESERVED19[1988];     /*!< Reserved,           Address offset: 0xCC - 0x88F */
    CANFD_LLC_TypeDef       TBUF;        /*!< CANFD desc TBUF,    Address offset: 0x890 - 0x8EB */
    uint8_t        RESERVED20[1984];     /*!< Reserved,           Address offset: 0x8EC - 0x10AB */
    // __IO uint32_t           PWMCR;       /*!< CANFD desc PWMCR,   Address offset: 0x10AC */
    __IO uint32_t Reserved5;
}CANFD_TypeDef;
/**
* @brief CRC Registers
*/
typedef struct
{
    __IO uint32_t DR;          /*!< CRC desc DR,  Address offset: 0x00 */
    __IO uint32_t IDR;         /*!< CRC desc IDR, Address offset: 0x04 */
    __IO uint32_t CR;          /*!< CRC desc CR,  Address offset: 0x08 */
}CRC_TypeDef;

/**
* @brief CTC Registers
*/
typedef struct
{
    __IO uint32_t CTL0;        /*!< CTC desc CTL0, Address offset: 0x00 */
    __IO uint32_t CTL1;        /*!< CTC desc CTL1, Address offset: 0x04 */
    __IO uint32_t SR;          /*!< CTC desc SR,   Address offset: 0x08 */
    __IO uint32_t INTC;        /*!< CTC desc INTC, Address offset: 0x0C */
}CTC_TypeDef;

/**
* @brief DBGMCU Registers
*/
typedef struct
{
    __IO uint32_t IDCODE;      /*!< DBGMCU desc IDCODE, Address offset: 0x00 */
    __IO uint32_t CR;          /*!< DBGMCU desc CR,     Address offset: 0x04 */
}DBGMCU_TypeDef;


/**
* @brief DMA Controller Registers
*/
typedef struct
{
    __IO uint32_t ISR;
    __IO uint32_t IFCR;
} DMA_TypeDef;

typedef struct
{
    __IO uint32_t CCR;
    __IO uint32_t CNDTR;
    __IO uint32_t CPAR;
    __IO uint32_t CMAR;
} DMA_Channel_TypeDef;
/**
* @brief EXTI Registers
*/
typedef struct
{
    __IO uint32_t IMR;         /*!< EXTI desc IMR,   Address offset: 0x00 */
    __IO uint32_t EMR;         /*!< EXTI desc EMR,   Address offset: 0x04 */
    __IO uint32_t RTSR;        /*!< EXTI desc RTSR,  Address offset: 0x08 */
    __IO uint32_t FTSR;        /*!< EXTI desc FTSR,  Address offset: 0x0C */
    __IO uint32_t SWIER;       /*!< EXTI desc SWIER, Address offset: 0x10 */
    __IO uint32_t PR;          /*!< EXTI desc PR,    Address offset: 0x14 */
}EXTI_TypeDef;

/**
* @brief FLASH Registers
*/
typedef struct
{
    __IO uint32_t ACR;         /*!< FLASH desc ACR,     Address offset: 0x00 */
    uint8_t RESERVED1[4];      /*!< Reserved, Address offset: 0x04 */
    __IO uint32_t KEYR;        /*!< FLASH desc KEYR,    Address offset: 0x08 */
    __IO uint32_t OPTKEYR;     /*!< FLASH desc OPTKEYR, Address offset: 0x0C */
    __IO uint32_t SR;          /*!< FLASH desc SR,      Address offset: 0x10 */
    __IO uint32_t CR;          /*!< FLASH desc CR,      Address offset: 0x14 */
    uint8_t RESERVED5[8];      /*!< Reserved, Address offset: 0x18 - 0x1F */
    __IO uint32_t OPTR;        /*!< FLASH desc OPTR,    Address offset: 0x20 */
    // __IO uint32_t SDKR;        /*!< FLASH desc SDKR,    Address offset: 0x24 */
    uint8_t RESERVED7[8];      /*!< Reserved, Address offset: 0x24 - 0x28 */
    __IO uint32_t WRPR;        /*!< FLASH desc WRPR,    Address offset: 0x2C */
}FLASH_TypeDef;

/**
* @brief General Purpose I/O
*/
typedef struct
{
    __IO uint32_t MODER;       /*!< GPIO port mode register,               Address offset: 0x00      */
    __IO uint32_t OTYPER;      /*!< GPIO port output type register,        Address offset: 0x04      */
    __IO uint32_t OSPEEDR;     /*!< GPIO port output speed register,       Address offset: 0x08      */
    __IO uint32_t PUPDR;       /*!< GPIO port pull-up/pull-down register,  Address offset: 0x0C      */
    __IO uint32_t IDR;         /*!< GPIO port input data register,         Address offset: 0x10      */
    __IO uint32_t ODR;         /*!< GPIO port output data register,        Address offset: 0x14      */
    __IO uint32_t BSRR;        /*!< GPIO port bit set/reset  register,     Address offset: 0x18      */
    __IO uint32_t LCKR;        /*!< GPIO port configuration lock register, Address offset: 0x1C      */
    __IO uint32_t AFR[2];      /*!< GPIO alternate function registers,     Address offset: 0x20-0x24 */
    __IO uint32_t BRR;         /*!< GPIO Bit Reset register,               Address offset: 0x28      */
} GPIO_TypeDef;

/**
* @brief I2C Registers
*/
typedef struct
{
    __IO uint32_t CR1;         /*!< I2C desc CR1,   Address offset: 0x00 */
    __IO uint32_t CR2;         /*!< I2C desc CR2,   Address offset: 0x04 */
    __IO uint32_t OAR1;        /*!< I2C desc OAR1,  Address offset: 0x08 */
    __IO uint32_t OAR2;        /*!< I2C desc OAR2,  Address offset: 0x0C */
    __IO uint32_t DR;          /*!< I2C desc DR,    Address offset: 0x10 */
    __IO uint32_t SR1;         /*!< I2C desc SR1,   Address offset: 0x14 */
    __IO uint32_t SR2;         /*!< I2C desc SR2,   Address offset: 0x18 */
    __IO uint32_t CCR;         /*!< I2C desc CCR,   Address offset: 0x1C */
    __IO uint32_t TRISE;       /*!< I2C desc TRISE, Address offset: 0x20 */
}I2C_TypeDef;

/**
* @brief IWDG Registers
*/
typedef struct
{
    __IO uint32_t KR;          /*!< IWDG desc KR,  Address offset: 0x00 */
    __IO uint32_t PR;          /*!< IWDG desc PR,  Address offset: 0x04 */
    __IO uint32_t RLR;         /*!< IWDG desc RLR, Address offset: 0x08 */
    __IO uint32_t SR;          /*!< IWDG desc SR,  Address offset: 0x0C */
}IWDG_TypeDef;

/**
* @brief PWR Registers
*/
typedef struct
{
    __IO uint32_t CR;          /*!< PWR desc CR,  Address offset: 0x00 */
    __IO uint32_t CSR;         /*!< PWR desc CSR, Address offset: 0x04 */
}PWR_TypeDef;

/**
* @brief ESMC Registers
*/
typedef struct
{
    __IO uint8_t CR;             /*!< Address offset: 0x00 */
    __IO uint8_t CR2;            /*!< Address offset: 0x01 */
    __IO uint8_t TCR;            /*!< Address offset: 0x02 */
    __IO uint8_t BAUD;           /*!< Address offset: 0x03 */
    __IO uint8_t SFCR;           /*!< Address offset: 0x04 */
    __IO uint8_t SOCR;           /*!< Address offset: 0x05 */
    __IO uint8_t DCR;            /*!< Address offset: 0x06 */
    __IO uint8_t CR3;            /*!< Address offset: 0x07 */
    __IO uint32_t ADDR24;        /*!< Address offset: 0x08 */
    __IO uint32_t ADDR32;        /*!< Address offset: 0x0C */
    __IO uint32_t DATA;          /*!< Address offset: 0x10 */
    __IO uint8_t SR;             /*!< Address offset: 0x14 */
    __IO uint8_t IFR;            /*!< Address offset: 0x15 */
    __IO uint8_t IER;            /*!< Address offset: 0x16 */
         uint8_t RESERVED1;
         uint8_t RESERVED2[4];
    __IO uint8_t XSFCR;          /*!< Address offset: 0x1C */
    __IO uint8_t XSOCR;          /*!< Address offset: 0x1D */
    __IO uint8_t XDCR;           /*!< Address offset: 0x1E */
    __IO uint8_t XCR3;           /*!< Address offset: 0x1F */
}ESMC_TypeDef;

/**
* @brief RCC Registers
*/
typedef struct
{
    __IO uint32_t CR;          /*!< RCC clock control register,                Address offset: 0x00 */
    __IO uint32_t CFGR;        /*!< RCC clock configuration register,          Address offset: 0x04 */
    __IO uint32_t CIR;         /*!< RCC clock interrupt register,              Address offset: 0x08 */
    __IO uint32_t APB2RSTR;    /*!< RCC APB2 peripheral reset register,        Address offset: 0x0C */
    __IO uint32_t APB1RSTR;    /*!< RCC APB1 peripheral reset register,        Address offset: 0x10 */
    __IO uint32_t AHB1ENR;     /*!< RCC AHB1 peripheral clock register,        Address offset: 0x14 */
    __IO uint32_t APB2ENR;     /*!< RCC APB2 peripheral clock enable register, Address offset: 0x18 */
    __IO uint32_t APB1ENR;     /*!< RCC APB1 peripheral clock enable register, Address offset: 0x1C */
    __IO uint32_t BDCR;        /*!< RCC Backup domain control register,        Address offset: 0x20 */
    __IO uint32_t CSR;         /*!< RCC clock control & status register,       Address offset: 0x24 */
    __IO uint32_t CFGR1;       /*!< RCC clock configuration register 1,        Address offset: 0x28 */
    __IO uint32_t AHB1RSTR;    /*!< RCC AHB1 peripheral reset register,        Address offset: 0x2C */
    __IO uint32_t AHB2RSTR;    /*!< RCC AHB2 peripheral reset register,        Address offset: 0x30 */
    __IO uint32_t AHB2ENR;     /*!< RCC AHB2 peripheral clock register,        Address offset: 0x34 */
    __IO uint32_t CFGR2;       /*!< RCC desc CFGR2,                            Address offset: 0x38 */
}RCC_TypeDef;

/**
* @brief RTC Registers
*/
typedef struct
{
    __IO uint32_t CRH;         /*!< RTC desc CRH,  Address offset: 0x00 */
    __IO uint32_t CRL;         /*!< RTC desc CRL,  Address offset: 0x04 */
    __IO uint32_t PRLH;        /*!< RTC desc PRLH, Address offset: 0x08 */
    __IO uint32_t PRLL;        /*!< RTC desc PRLL, Address offset: 0x0C */
    __IO uint32_t DIVH;        /*!< RTC desc DIVH, Address offset: 0x10 */
    __IO uint32_t DIVL;        /*!< RTC desc DIVL, Address offset: 0x14 */
    __IO uint32_t CNTH;        /*!< RTC desc CNTH, Address offset: 0x18 */
    __IO uint32_t CNTL;        /*!< RTC desc CNTL, Address offset: 0x1C */
    __IO uint32_t ALRH;        /*!< RTC desc ALRH, Address offset: 0x20 */
    __IO uint32_t ALRL;        /*!< RTC desc ALRL, Address offset: 0x24 */
}RTC_TypeDef;

/**
* @brief SDIO Registers
*/
typedef struct
{
    __IO uint32_t POWER;       /*!< SDIO desc POWER,    Address offset: 0x00 */
    __IO uint32_t CLKCR;       /*!< SDIO desc CLKCR,    Address offset: 0x04 */
    __IO uint32_t ARG;         /*!< SDIO desc ARG,      Address offset: 0x08 */
    __IO uint32_t CMD;         /*!< SDIO desc CMD,      Address offset: 0x0C */
    __IO uint32_t RESPCMD;     /*!< SDIO desc RESPCMD,  Address offset: 0x10 */
    __IO uint32_t RESP0;       /*!< SDIO desc RESP0,    Address offset: 0x14 */
    __IO uint32_t RESP1;       /*!< SDIO desc RESP1,    Address offset: 0x18 */
    __IO uint32_t RESP2;       /*!< SDIO desc RESP2,    Address offset: 0x1C */
    __IO uint32_t RESP3;       /*!< SDIO desc RESP3,    Address offset: 0x20 */
    __IO uint32_t TMOUT;       /*!< SDIO desc TMOUT,    Address offset: 0x24 */
    __IO uint32_t BLKSIZ;      /*!< SDIO desc BLKSIZ,   Address offset: 0x28 */
    __IO uint32_t DLEN;        /*!< SDIO desc DLEN,     Address offset: 0x2C */
    __IO uint32_t CTRL;        /*!< SDIO desc CTRL,     Address offset: 0x30 */
    __IO uint32_t STATUS;      /*!< SDIO desc STATUS,   Address offset: 0x34 */
    __IO uint32_t INTSTS;      /*!< SDIO desc INTSTS,   Address offset: 0x38 */
    __IO uint32_t INTMASK;     /*!< SDIO desc INTMASK,  Address offset: 0x3C */
    __IO uint32_t FIFOTH;      /*!< SDIO desc FIFOTH,   Address offset: 0x40 */
    __IO uint32_t TCBCNT;      /*!< SDIO desc TCBCNT,   Address offset: 0x44 */
    __IO uint32_t TBBCNT;      /*!< SDIO desc TBBCNT,   Address offset: 0x48 */
    uint8_t RESERVED19[436];   /*!< Reserved, Address offset: 0x4C - 0x1FF */
    __IO uint32_t FIFODATA;    /*!< SDIO desc FIFODATA, Address offset: 0x200 */
}SDIO_TypeDef;

/**
* @brief SPI Registers
*/
typedef struct
{
    __IO uint32_t CR1;         /*!< SPI desc CR1,     Address offset: 0x00 */
    __IO uint32_t CR2;         /*!< SPI desc CR2,     Address offset: 0x04 */
    __IO uint32_t SR;          /*!< SPI desc SR,      Address offset: 0x08 */
    __IO uint32_t DR;          /*!< SPI desc DR,      Address offset: 0x0C */
    __IO uint32_t CRCPR;       /*!< SPI desc CRCPR,   Address offset: 0x10 */
    __IO uint32_t RXCRCR;      /*!< SPI desc RXCRCR,  Address offset: 0x14 */
    __IO uint32_t TXCRCR;      /*!< SPI desc TXCRCR,  Address offset: 0x18 */
    __IO uint32_t I2SCFGR;     /*!< SPI desc I2SCFGR, Address offset: 0x1C */
    __IO uint32_t I2SPR;       /*!< SPI desc I2SPR,   Address offset: 0x20 */
}SPI_TypeDef;


/**
* @brief SYSCFG Registers
*/
typedef struct
{
    __IO uint32_t CFGR[5];     /*!< SYSCFG configuration register,   Address offset: 0x00 ~ 0x13 */
    __IO uint32_t EXTICR[4];   /*!< SYSCFG external interrupt configuration registers,   Address offset: 0x14-0x23 */
    __IO uint32_t PAENS;       /*!< SYSCFG desc PAENS,   Address offset: 0x24 */
    __IO uint32_t PBENS;       /*!< SYSCFG desc PBENS,   Address offset: 0x28 */
    __IO uint32_t PCENS;       /*!< SYSCFG desc PCENS,   Address offset: 0x2C */
    __IO uint32_t PDENS;       /*!< SYSCFG desc PDENS,   Address offset: 0x30 */
    __IO uint32_t PEENS;       /*!< SYSCFG desc PEENS,   Address offset: 0x34 */
    __IO uint32_t GPIOENA;     /*!< SYSCFG desc GPIOENA, Address offset: 0x38 */
    __IO uint32_t Reserved[80];
    __IO uint32_t TIM_CLK_EXT;
}SYSCFG_TypeDef;

/**
* @brief TIM Registers
*/
typedef struct
{
    __IO uint32_t CR1;         /*!< TIM desc CR1,          Address offset: 0x00 */
    __IO uint32_t CR2;         /*!< TIM desc CR2,          Address offset: 0x04 */
    __IO uint32_t SMCR;        /*!< TIM desc SMCR,         Address offset: 0x08 */
    __IO uint32_t DIER;        /*!< TIM desc DIER,         Address offset: 0x0C */
    __IO uint32_t SR;          /*!< TIM desc SR,           Address offset: 0x10 */
    __IO uint32_t EGR;         /*!< TIM desc EGR,          Address offset: 0x14 */
    __IO uint32_t CCMR1;       /*!< TIM desc CCMR1:OUTPUT, Address offset: 0x18 */
    __IO uint32_t CCMR2;       /*!< TIM desc CCMR2:OUTPUT, Address offset: 0x1C */
    __IO uint32_t CCER;        /*!< TIM desc CCER,         Address offset: 0x20 */
    __IO uint32_t CNT;         /*!< TIM desc CNT,          Address offset: 0x24 */
    __IO uint32_t PSC;         /*!< TIM desc PSC,          Address offset: 0x28 */
    __IO uint32_t ARR;         /*!< TIM desc ARR,          Address offset: 0x2C */
    __IO uint32_t RCR;         /*!< TIM desc RCR,          Address offset: 0x30 */
    __IO uint32_t CCR1;        /*!< TIM desc CCR1,         Address offset: 0x34 */
    __IO uint32_t CCR2;        /*!< TIM desc CCR2,         Address offset: 0x38 */
    __IO uint32_t CCR3;        /*!< TIM desc CCR3,         Address offset: 0x3C */
    __IO uint32_t CCR4;        /*!< TIM desc CCR4,         Address offset: 0x40 */
    __IO uint32_t BDTR;        /*!< TIM desc BDTR,         Address offset: 0x44 */
    __IO uint32_t DCR;         /*!< TIM desc DCR,          Address offset: 0x48 */
    __IO uint32_t DMAR;        /*!< TIM desc DMAR,         Address offset: 0x4C */
    __IO uint32_t OR;          /*!< TIM desc OR,           Address offset: 0x50 */
}TIM_TypeDef;

/**
* @brief USART Registers
*/
typedef struct
{
    __IO uint32_t SR;          /*!< USART desc SR,   Address offset: 0x00 */
    __IO uint32_t DR;          /*!< USART desc DR,   Address offset: 0x04 */
    __IO uint32_t BRR;         /*!< USART desc BRR,  Address offset: 0x08 */
    __IO uint32_t CR1;         /*!< USART desc CR1,  Address offset: 0x0C */
    __IO uint32_t CR2;         /*!< USART desc CR2,  Address offset: 0x10 */
    __IO uint32_t CR3;         /*!< USART desc CR3,  Address offset: 0x14 */
    __IO uint32_t GTPR;        /*!< USART desc GTPR, Address offset: 0x18 */
}USART_TypeDef;

/**
* @brief WWDG Registers
*/
typedef struct
{
    __IO uint32_t CR;          /*!< WWDG desc CR,  Address offset: 0x00 */
    __IO uint32_t CFR;         /*!< WWDG desc CFR, Address offset: 0x04 */
    __IO uint32_t SR;          /*!< WWDG desc SR,  Address offset: 0x08 */
}WWDG_TypeDef;

/*!< Peripheral memory map */
#define FLASH_BASE            (0x08000000UL)                         /*!< FLASH base address */
#define FLASH_END             (0x0803FFFFUL)                         /*!< FLASH end address */
#define FLASH_SIZE            (FLASH_END - FLASH_BASE + 1)
#define FLASH_PAGE_SIZE       (0x00000100UL)                         /*!< FLASH Page Size, 256 Bytes */
#define FLASH_PAGE_NB         (FLASH_SIZE / FLASH_PAGE_SIZE)
#define FLASH_SECTOR_SIZE     (0x00000800UL)                         /*!< FLASH Sector Size, 2048 Bytes */
#define FLASH_SECTOR_NB       (FLASH_SIZE / FLASH_SECTOR_SIZE)
#define FLASH_BLOCK_SIZE      (0x00008000UL)                         /*!< FLASH Block  Size, 32768 Bytes */
#define FLASH_BLOCK_NB        (FLASH_SIZE / FLASH_BLOCK_SIZE)
#define SRAM_BASE             (0x20000000UL)                         /*!< SRAM base address */
#define SRAM_END              (0x2000BFFFUL)                         /*!< SRAM end address */
#define PERIPH_BASE           (0x40000000UL)                         /*!< Peripheral base address */
#define SRAM_BB_BASE          (0x22000000UL)                         /*!< SRAM base address in the bit-band region */
#define PERIPH_BB_BASE        (0x42000000UL)                         /*!< Peripheral base address in the bit-band region */
#define IOPORT_BASE           (0x50000000UL)                         /*!< IOPORT base address */
#define UID_BASE              (0x1FFF5800UL)                         /*!< Unique device ID register base address */

#define APB1PERIPH_BASE                        PERIPH_BASE
#define APB2PERIPH_BASE                        (PERIPH_BASE + 0x00010000UL)

#define AHBPERIPH_BASE                         (0xA0000000UL)
#define AHB1PERIPH_BASE                        (PERIPH_BASE + 0x00020000UL)
#define AHB2PERIPH_BASE                        (PERIPH_BASE + 0x08000000UL)

/*!< APB peripherals */
#define ADC1_BASE                              (0x40012400UL)
#define ADC2_BASE                              (0x40012800UL)
#define ADC3_BASE                              (0x40013C00UL)
#define BKP_BASE                               (0x40006C00UL)
#define CANFD_BASE                             (0x40008000UL)
#define CRC_BASE                               (0x40023000UL)
#define CTC_BASE                               (0x4000C800UL)
#define DBGMCU_BASE                            (0xE0042000UL)
#define DMA1_BASE                              (AHB1PERIPH_BASE + 0x00000000UL)
#define DMA1_Channel1_BASE                     (AHB1PERIPH_BASE + 0x00000008UL)
#define DMA1_Channel2_BASE                     (AHB1PERIPH_BASE + 0x0000001CUL)
#define DMA1_Channel3_BASE                     (AHB1PERIPH_BASE + 0x00000030UL)
#define DMA1_Channel4_BASE                     (AHB1PERIPH_BASE + 0x00000044UL)
#define DMA1_Channel5_BASE                     (AHB1PERIPH_BASE + 0x00000058UL)
#define DMA1_Channel6_BASE                     (AHB1PERIPH_BASE + 0x0000006CUL)
#define DMA1_Channel7_BASE                     (AHB1PERIPH_BASE + 0x00000080UL)
#define DMA2_BASE                              (AHB1PERIPH_BASE + 0x00000400UL)
#define DMA2_Channel1_BASE                     (AHB1PERIPH_BASE + 0x00000408UL)
#define DMA2_Channel2_BASE                     (AHB1PERIPH_BASE + 0x0000041CUL)
#define DMA2_Channel3_BASE                     (AHB1PERIPH_BASE + 0x00000430UL)
#define DMA2_Channel4_BASE                     (AHB1PERIPH_BASE + 0x00000444UL)
#define DMA2_Channel5_BASE                     (AHB1PERIPH_BASE + 0x00000458UL)
#define EXTI_BASE                              (0x40010400UL)
#define FLASH_R_BASE                           (AHB1PERIPH_BASE + 0x2000) /*!< Flash registers base address */
#define OB_BASE                                (0x1FFF5000UL)    /*!< Flash Option Bytes base address */
#define GPIOA_BASE                             (0x48000000UL)
#define GPIOB_BASE                             (0x48000400UL)
#define GPIOC_BASE                             (0x48000800UL)
#define GPIOD_BASE                             (0x48000C00UL)
#define GPIOE_BASE                             (0x48001000UL)
#define I2C1_BASE                              (0x40005400UL)
#define I2C2_BASE                              (0x40005800UL)
#define IWDG_BASE                              (0x40003000UL)
#define PWR_BASE                               (0x40007000UL)
#define RCC_BASE                               (0x40021000UL)
#define RTC_BASE                               (0x40002800UL)
#define SDIO_BASE                              (0x40018000UL)
#define SPI1_BASE                              (0x40013000UL)
#define SPI2_BASE                              (0x40003800UL)
#define SPI3_BASE                              (0x40003C00UL)
#define SYSCFG_BASE                            (0x40010000UL)
#define TIM1_BASE                              (0x40012C00UL)
#define TIM2_BASE                              (0x40000000UL)
#define TIM3_BASE                              (0x40000400UL)
#define TIM4_BASE                              (0x40000800UL)
#define TIM5_BASE                              (0x40000C00UL)
#define TIM6_BASE                              (0x40001000UL)
#define TIM7_BASE                              (0x40001400UL)
#define TIM8_BASE                              (0x40013400UL)
#define TIM9_BASE                              (0x40014C00UL)
#define TIM10_BASE                             (0x40015000UL)
#define TIM11_BASE                             (0x40015400UL)
#define TIM12_BASE                             (0x40001800UL)
#define TIM13_BASE                             (0x40001C00UL)
#define TIM14_BASE                             (0x40002000UL)
#define USART1_BASE                            (0x40013800UL)
#define USART2_BASE                            (0x40004400UL)
#define USART3_BASE                            (0x40004800UL)
#define USART4_BASE                            (0x40004C00UL)
#define USART5_BASE                            (0x40005000UL)
#define WWDG_BASE                              (0x40002C00UL)
#define ESMC_BASE                              (AHBPERIPH_BASE + 0x00001000UL)


#define ADC1                                   ((ADC_TypeDef *) ADC1_BASE)
#define ADC2                                   ((ADC_TypeDef *) ADC2_BASE)
#define ADC3                                   ((ADC_TypeDef *) ADC3_BASE)
#define BKP                                    ((BKP_TypeDef *) BKP_BASE)
#define CANFD                                  ((CANFD_TypeDef *) CANFD_BASE)
#define CRC                                    ((CRC_TypeDef *) CRC_BASE)
#define CTC                                    ((CTC_TypeDef *) CTC_BASE)
#define DBGMCU                                 ((DBGMCU_TypeDef *) DBGMCU_BASE)
#define DMA1                                   ((DMA_TypeDef *)DMA1_BASE)
#define DMA2                                   ((DMA_TypeDef *)DMA2_BASE)
#define DMA1_Channel1                          ((DMA_Channel_TypeDef *)DMA1_Channel1_BASE)
#define DMA1_Channel2                          ((DMA_Channel_TypeDef *)DMA1_Channel2_BASE)
#define DMA1_Channel3                          ((DMA_Channel_TypeDef *)DMA1_Channel3_BASE)
#define DMA1_Channel4                          ((DMA_Channel_TypeDef *)DMA1_Channel4_BASE)
#define DMA1_Channel5                          ((DMA_Channel_TypeDef *)DMA1_Channel5_BASE)
#define DMA1_Channel6                          ((DMA_Channel_TypeDef *)DMA1_Channel6_BASE)
#define DMA1_Channel7                          ((DMA_Channel_TypeDef *)DMA1_Channel7_BASE)
#define DMA2_Channel1                          ((DMA_Channel_TypeDef *)DMA2_Channel1_BASE)
#define DMA2_Channel2                          ((DMA_Channel_TypeDef *)DMA2_Channel2_BASE)
#define DMA2_Channel3                          ((DMA_Channel_TypeDef *)DMA2_Channel3_BASE)
#define DMA2_Channel4                          ((DMA_Channel_TypeDef *)DMA2_Channel4_BASE)
#define DMA2_Channel5                          ((DMA_Channel_TypeDef *)DMA2_Channel5_BASE)
#define EXTI                                   ((EXTI_TypeDef *) EXTI_BASE)
#define FLASH                                  ((FLASH_TypeDef *) FLASH_R_BASE)
#define GPIOA                                  ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB                                  ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC                                  ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD                                  ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE                                  ((GPIO_TypeDef *) GPIOE_BASE)
#define I2C1                                   ((I2C_TypeDef *) I2C1_BASE)
#define I2C2                                   ((I2C_TypeDef *) I2C2_BASE)
#define IWDG                                   ((IWDG_TypeDef *) IWDG_BASE)
#define PWR                                    ((PWR_TypeDef *) PWR_BASE)
#define RCC                                    ((RCC_TypeDef *) RCC_BASE)
#define RTC                                    ((RTC_TypeDef *) RTC_BASE)
#define SDIO                                   ((SDIO_TypeDef *) SDIO_BASE)
#define SPI1                                   ((SPI_TypeDef *) SPI1_BASE)
#define SPI2                                   ((SPI_TypeDef *) SPI2_BASE)
#define SPI3                                   ((SPI_TypeDef *) SPI3_BASE)
#define SYSCFG                                 ((SYSCFG_TypeDef *) SYSCFG_BASE)
#define TIM1                                   ((TIM_TypeDef *) TIM1_BASE)
#define TIM2                                   ((TIM_TypeDef *) TIM2_BASE)
#define TIM3                                   ((TIM_TypeDef *) TIM3_BASE)
#define TIM4                                   ((TIM_TypeDef *) TIM4_BASE)
#define TIM5                                   ((TIM_TypeDef *) TIM5_BASE)
#define TIM6                                   ((TIM_TypeDef *) TIM6_BASE)
#define TIM7                                   ((TIM_TypeDef *) TIM7_BASE)
#define TIM8                                   ((TIM_TypeDef *) TIM8_BASE)
#define TIM9                                   ((TIM_TypeDef *) TIM9_BASE)
#define TIM10                                  ((TIM_TypeDef *) TIM10_BASE)
#define TIM11                                  ((TIM_TypeDef *) TIM11_BASE)
#define TIM12                                  ((TIM_TypeDef *) TIM12_BASE)
#define TIM13                                  ((TIM_TypeDef *) TIM13_BASE)
#define TIM14                                  ((TIM_TypeDef *) TIM14_BASE)
#define USART1                                 ((USART_TypeDef *) USART1_BASE)
#define USART2                                 ((USART_TypeDef *) USART2_BASE)
#define USART3                                 ((USART_TypeDef *) USART3_BASE)
#define USART4                                 ((USART_TypeDef *) USART4_BASE)
#define USART5                                 ((USART_TypeDef *) USART5_BASE)
#define WWDG                                   ((WWDG_TypeDef *) WWDG_BASE)
#define ESMC                                   ((ESMC_TypeDef *) ESMC_BASE)



/*********************  Bits Define For Peripheral ADC  *********************/
/*!< ADC_SR */
#define ADC_SR_AWD_Pos                            (0U)
#define ADC_SR_AWD_Msk                            (0x1UL << ADC_SR_AWD_Pos)                         /*!< 0x00000001 */
#define ADC_SR_AWD                                ADC_SR_AWD_Msk                                    /*!< desc AWD */
#define ADC_SR_EOC_Pos                            (1U)
#define ADC_SR_EOC_Msk                            (0x1UL << ADC_SR_EOC_Pos)                         /*!< 0x00000002 */
#define ADC_SR_EOC                                ADC_SR_EOC_Msk                                    /*!< desc EOC */
#define ADC_SR_JEOC_Pos                           (2U)
#define ADC_SR_JEOC_Msk                           (0x1UL << ADC_SR_JEOC_Pos)                        /*!< 0x00000004 */
#define ADC_SR_JEOC                               ADC_SR_JEOC_Msk                                   /*!< desc JEOC */
#define ADC_SR_JSTRT_Pos                          (3U)
#define ADC_SR_JSTRT_Msk                          (0x1UL << ADC_SR_JSTRT_Pos)                       /*!< 0x00000008 */
#define ADC_SR_JSTRT                              ADC_SR_JSTRT_Msk                                  /*!< desc JSTRT */
#define ADC_SR_STRT_Pos                           (4U)
#define ADC_SR_STRT_Msk                           (0x1UL << ADC_SR_STRT_Pos)                        /*!< 0x00000010 */
#define ADC_SR_STRT                               ADC_SR_STRT_Msk                                   /*!< desc STRT */

/*!< ADC_CR1 */
#define ADC_CR1_AWDCH_Pos                         (0U)
#define ADC_CR1_AWDCH_Msk                         (0x1FUL << ADC_CR1_AWDCH_Pos)                     /*!< 0x0000001F */
#define ADC_CR1_AWDCH                             ADC_CR1_AWDCH_Msk                                 /*!< AWDCH[4:0] bits (desc AWDCH) */
#define ADC_CR1_AWDCH_0                           (0x1UL << ADC_CR1_AWDCH_Pos)                      /*!< 0x00000001 */
#define ADC_CR1_AWDCH_1                           (0x2UL << ADC_CR1_AWDCH_Pos)                      /*!< 0x00000002 */
#define ADC_CR1_AWDCH_2                           (0x4UL << ADC_CR1_AWDCH_Pos)                      /*!< 0x00000004 */
#define ADC_CR1_AWDCH_3                           (0x8UL << ADC_CR1_AWDCH_Pos)                      /*!< 0x00000008 */
#define ADC_CR1_AWDCH_4                           (0x10UL << ADC_CR1_AWDCH_Pos)                     /*!< 0x00000010 */

#define ADC_CR1_EOCIE_Pos                         (5U)
#define ADC_CR1_EOCIE_Msk                         (0x1UL << ADC_CR1_EOCIE_Pos)                      /*!< 0x00000020 */
#define ADC_CR1_EOCIE                             ADC_CR1_EOCIE_Msk                                 /*!< desc EOCIE */
#define ADC_CR1_AWDIE_Pos                         (6U)
#define ADC_CR1_AWDIE_Msk                         (0x1UL << ADC_CR1_AWDIE_Pos)                      /*!< 0x00000040 */
#define ADC_CR1_AWDIE                             ADC_CR1_AWDIE_Msk                                 /*!< desc AWDIE */
#define ADC_CR1_JEOCIE_Pos                        (7U)
#define ADC_CR1_JEOCIE_Msk                        (0x1UL << ADC_CR1_JEOCIE_Pos)                     /*!< 0x00000080 */
#define ADC_CR1_JEOCIE                            ADC_CR1_JEOCIE_Msk                                /*!< desc JEOCIE */
#define ADC_CR1_SCAN_Pos                          (8U)
#define ADC_CR1_SCAN_Msk                          (0x1UL << ADC_CR1_SCAN_Pos)                       /*!< 0x00000100 */
#define ADC_CR1_SCAN                              ADC_CR1_SCAN_Msk                                  /*!< desc SCAN */
#define ADC_CR1_AWDSGL_Pos                        (9U)
#define ADC_CR1_AWDSGL_Msk                        (0x1UL << ADC_CR1_AWDSGL_Pos)                     /*!< 0x00000200 */
#define ADC_CR1_AWDSGL                            ADC_CR1_AWDSGL_Msk                                /*!< desc AWDSGL */
#define ADC_CR1_JAUTO_Pos                         (10U)
#define ADC_CR1_JAUTO_Msk                         (0x1UL << ADC_CR1_JAUTO_Pos)                      /*!< 0x00000400 */
#define ADC_CR1_JAUTO                             ADC_CR1_JAUTO_Msk                                 /*!< desc JAUTO */
#define ADC_CR1_DISCEN_Pos                        (11U)
#define ADC_CR1_DISCEN_Msk                        (0x1UL << ADC_CR1_DISCEN_Pos)                     /*!< 0x00000800 */
#define ADC_CR1_DISCEN                            ADC_CR1_DISCEN_Msk                                /*!< desc DISCEN */
#define ADC_CR1_JDISCEN_Pos                       (12U)
#define ADC_CR1_JDISCEN_Msk                       (0x1UL << ADC_CR1_JDISCEN_Pos)                    /*!< 0x00001000 */
#define ADC_CR1_JDISCEN                           ADC_CR1_JDISCEN_Msk                               /*!< desc JDISCEN */
#define ADC_CR1_DISCNUM_Pos                       (13U)
#define ADC_CR1_DISCNUM_Msk                       (0x7UL << ADC_CR1_DISCNUM_Pos)                    /*!< 0x0000E000 */
#define ADC_CR1_DISCNUM                           ADC_CR1_DISCNUM_Msk                               /*!< DISCNUM[15:13] bits (desc DISCNUM) */
#define ADC_CR1_DISCNUM_0                         (0x1UL << ADC_CR1_DISCNUM_Pos)                    /*!< 0x00002000 */
#define ADC_CR1_DISCNUM_1                         (0x2UL << ADC_CR1_DISCNUM_Pos)                    /*!< 0x00004000 */
#define ADC_CR1_DISCNUM_2                         (0x4UL << ADC_CR1_DISCNUM_Pos)                    /*!< 0x00008000 */

#define ADC_CR1_DUALMOD_Pos                       (16U)
#define ADC_CR1_DUALMOD_Msk                       (0xFUL << ADC_CR1_DUALMOD_Pos)                    /*!< 0x000F0000 */
#define ADC_CR1_DUALMOD                           ADC_CR1_DUALMOD_Msk                               /*!< DUALMOD[19:16] bits (desc DUALMOD) */
#define ADC_CR1_DUALMOD_0                         (0x1UL << ADC_CR1_DUALMOD_Pos)                    /*!< 0x00010000 */
#define ADC_CR1_DUALMOD_1                         (0x2UL << ADC_CR1_DUALMOD_Pos)                    /*!< 0x00020000 */
#define ADC_CR1_DUALMOD_2                         (0x4UL << ADC_CR1_DUALMOD_Pos)                    /*!< 0x00040000 */
#define ADC_CR1_DUALMOD_3                         (0x8UL << ADC_CR1_DUALMOD_Pos)                    /*!< 0x00080000 */

#define ADC_CR1_JAWDEN_Pos                        (22U)
#define ADC_CR1_JAWDEN_Msk                        (0x1UL << ADC_CR1_JAWDEN_Pos)                     /*!< 0x00400000 */
#define ADC_CR1_JAWDEN                            ADC_CR1_JAWDEN_Msk                                /*!< desc JAWDEN */
#define ADC_CR1_AWDEN_Pos                         (23U)
#define ADC_CR1_AWDEN_Msk                         (0x1UL << ADC_CR1_AWDEN_Pos)                      /*!< 0x00800000 */
#define ADC_CR1_AWDEN                             ADC_CR1_AWDEN_Msk                                 /*!< desc AWDEN */
#define ADC_CR1_RESSEL_Pos                        (24U)
#define ADC_CR1_RESSEL_Msk                        (0x3UL << ADC_CR1_RESSEL_Pos)                     /*!< 0x03000000 */
#define ADC_CR1_RESSEL                            ADC_CR1_RESSEL_Msk                                /*!< RESSEL[25:24] bits (desc RESSEL) */
#define ADC_CR1_RESSEL_0                          (0x1UL << ADC_CR1_RESSEL_Pos)                     /*!< 0x01000000 */
#define ADC_CR1_RESSEL_1                          (0x2UL << ADC_CR1_RESSEL_Pos)                     /*!< 0x02000000 */

// #define ADC_CR1_WAIT_Pos                          (26U)
// #define ADC_CR1_WAIT_Msk                          (0x1UL << ADC_CR1_WAIT_Pos)                       /*!< 0x04000000 */
// #define ADC_CR1_WAIT                              ADC_CR1_WAIT_Msk                                  /*!< desc WAIT */
#define ADC_CR1_ADSTP_Pos                         (27U)
#define ADC_CR1_ADSTP_Msk                         (0x1UL << ADC_CR1_ADSTP_Pos)                      /*!< 0x08000000 */
#define ADC_CR1_ADSTP                             ADC_CR1_ADSTP_Msk                                 /*!< desc ADSTP */
// #define ADC_CR1_MSBSEL_Pos                        (28U)
// #define ADC_CR1_MSBSEL_Msk                        (0x1UL << ADC_CR1_MSBSEL_Pos)                     /*!< 0x10000000 */
// #define ADC_CR1_MSBSEL                            ADC_CR1_MSBSEL_Msk                                /*!< desc MSBSEL */

/*!< ADC_CR2 */
#define ADC_CR2_ADON_Pos                          (0U)
#define ADC_CR2_ADON_Msk                          (0x1UL << ADC_CR2_ADON_Pos)                       /*!< 0x00000001 */
#define ADC_CR2_ADON                              ADC_CR2_ADON_Msk                                  /*!< desc ADON */
#define ADC_CR2_CONT_Pos                          (1U)
#define ADC_CR2_CONT_Msk                          (0x1UL << ADC_CR2_CONT_Pos)                       /*!< 0x00000002 */
#define ADC_CR2_CONT                              ADC_CR2_CONT_Msk                                  /*!< desc CONT */
#define ADC_CR2_CAL_Pos                           (2U)
#define ADC_CR2_CAL_Msk                           (0x1UL << ADC_CR2_CAL_Pos)                        /*!< 0x00000004 */
#define ADC_CR2_CAL                               ADC_CR2_CAL_Msk                                   /*!< desc CAL */
#define ADC_CR2_RSTCAL_Pos                        (3U)
#define ADC_CR2_RSTCAL_Msk                        (0x1UL << ADC_CR2_RSTCAL_Pos)                     /*!< 0x00000008 */
#define ADC_CR2_RSTCAL                            ADC_CR2_RSTCAL_Msk                                /*!< desc RSTCAL */
#define ADC_CR2_DMA_Pos                           (8U)
#define ADC_CR2_DMA_Msk                           (0x1UL << ADC_CR2_DMA_Pos)                        /*!< 0x00000100 */
#define ADC_CR2_DMA                               ADC_CR2_DMA_Msk                                   /*!< desc DMA */
#define ADC_CR2_ALIGN_Pos                         (11U)
#define ADC_CR2_ALIGN_Msk                         (0x1UL << ADC_CR2_ALIGN_Pos)                      /*!< 0x00000800 */
#define ADC_CR2_ALIGN                             ADC_CR2_ALIGN_Msk                                 /*!< desc ALIGN */
#define ADC_CR2_JEXTSEL_Pos                       (12U)
#define ADC_CR2_JEXTSEL_Msk                       (0x7UL << ADC_CR2_JEXTSEL_Pos)                    /*!< 0x00007000 */
#define ADC_CR2_JEXTSEL                           ADC_CR2_JEXTSEL_Msk                               /*!< JEXTSEL[14:12] bits (desc JEXTSEL) */
#define ADC_CR2_JEXTSEL_0                         (0x1UL << ADC_CR2_JEXTSEL_Pos)                    /*!< 0x00001000 */
#define ADC_CR2_JEXTSEL_1                         (0x2UL << ADC_CR2_JEXTSEL_Pos)                    /*!< 0x00002000 */
#define ADC_CR2_JEXTSEL_2                         (0x4UL << ADC_CR2_JEXTSEL_Pos)                    /*!< 0x00004000 */

#define ADC_CR2_JEXTTRIG_Pos                      (15U)
#define ADC_CR2_JEXTTRIG_Msk                      (0x1UL << ADC_CR2_JEXTTRIG_Pos)                   /*!< 0x00008000 */
#define ADC_CR2_JEXTTRIG                          ADC_CR2_JEXTTRIG_Msk                              /*!< desc JEXTTRIG */
#define ADC_CR2_EXTSEL_Pos                        (17U)
#define ADC_CR2_EXTSEL_Msk                        (0x7UL << ADC_CR2_EXTSEL_Pos)                     /*!< 0x000E0000 */
#define ADC_CR2_EXTSEL                            ADC_CR2_EXTSEL_Msk                                /*!< EXTSEL[19:17] bits (desc EXTSEL) */
#define ADC_CR2_EXTSEL_0                          (0x1UL << ADC_CR2_EXTSEL_Pos)                     /*!< 0x00020000 */
#define ADC_CR2_EXTSEL_1                          (0x2UL << ADC_CR2_EXTSEL_Pos)                     /*!< 0x00040000 */
#define ADC_CR2_EXTSEL_2                          (0x4UL << ADC_CR2_EXTSEL_Pos)                     /*!< 0x00080000 */

#define ADC_CR2_EXTTRIG_Pos                       (20U)
#define ADC_CR2_EXTTRIG_Msk                       (0x1UL << ADC_CR2_EXTTRIG_Pos)                    /*!< 0x00100000 */
#define ADC_CR2_EXTTRIG                           ADC_CR2_EXTTRIG_Msk                               /*!< desc EXTTRIG */
#define ADC_CR2_JSWSTART_Pos                      (21U)
#define ADC_CR2_JSWSTART_Msk                      (0x1UL << ADC_CR2_JSWSTART_Pos)                   /*!< 0x00200000 */
#define ADC_CR2_JSWSTART                          ADC_CR2_JSWSTART_Msk                              /*!< desc JSWSTART */
#define ADC_CR2_SWSTART_Pos                       (22U)
#define ADC_CR2_SWSTART_Msk                       (0x1UL << ADC_CR2_SWSTART_Pos)                    /*!< 0x00400000 */
#define ADC_CR2_SWSTART                           ADC_CR2_SWSTART_Msk                               /*!< desc SWSTART */
#define ADC_CR2_TSVREFE_Pos                       (23U)
#define ADC_CR2_TSVREFE_Msk                       (0x1UL << ADC_CR2_TSVREFE_Pos)                    /*!< 0x00800000 */
#define ADC_CR2_TSVREFE                           ADC_CR2_TSVREFE_Msk                               /*!< desc TSVREFE */
// #define ADC_CR2_VBAT_Pos                          (24U)
// #define ADC_CR2_VBAT_Msk                          (0x1UL << ADC_CR2_VBAT_Pos)                       /*!< 0x01000000 */
// #define ADC_CR2_VBAT                              ADC_CR2_VBAT_Msk                                  /*!< desc VBAT */

/*!< ADC_SMPR1 */
#define ADC_SMPR1_SMP10_Pos                       (0U)
#define ADC_SMPR1_SMP10_Msk                       (0x7UL << ADC_SMPR1_SMP10_Pos)                    /*!< 0x00000007 */
#define ADC_SMPR1_SMP10                           ADC_SMPR1_SMP10_Msk                               /*!< SMP10[2:0] bits (desc SMP10) */
#define ADC_SMPR1_SMP10_0                         (0x1UL << ADC_SMPR1_SMP10_Pos)                    /*!< 0x00000001 */
#define ADC_SMPR1_SMP10_1                         (0x2UL << ADC_SMPR1_SMP10_Pos)                    /*!< 0x00000002 */
#define ADC_SMPR1_SMP10_2                         (0x4UL << ADC_SMPR1_SMP10_Pos)                    /*!< 0x00000004 */

#define ADC_SMPR1_SMP11_Pos                       (3U)
#define ADC_SMPR1_SMP11_Msk                       (0x7UL << ADC_SMPR1_SMP11_Pos)                    /*!< 0x00000038 */
#define ADC_SMPR1_SMP11                           ADC_SMPR1_SMP11_Msk                               /*!< SMP11[5:3] bits (desc SMP11) */
#define ADC_SMPR1_SMP11_0                         (0x1UL << ADC_SMPR1_SMP11_Pos)                    /*!< 0x00000008 */
#define ADC_SMPR1_SMP11_1                         (0x2UL << ADC_SMPR1_SMP11_Pos)                    /*!< 0x00000010 */
#define ADC_SMPR1_SMP11_2                         (0x4UL << ADC_SMPR1_SMP11_Pos)                    /*!< 0x00000020 */

#define ADC_SMPR1_SMP12_Pos                       (6U)
#define ADC_SMPR1_SMP12_Msk                       (0x7UL << ADC_SMPR1_SMP12_Pos)                    /*!< 0x000001C0 */
#define ADC_SMPR1_SMP12                           ADC_SMPR1_SMP12_Msk                               /*!< SMP12[8:6] bits (desc SMP12) */
#define ADC_SMPR1_SMP12_0                         (0x1UL << ADC_SMPR1_SMP12_Pos)                    /*!< 0x00000040 */
#define ADC_SMPR1_SMP12_1                         (0x2UL << ADC_SMPR1_SMP12_Pos)                    /*!< 0x00000080 */
#define ADC_SMPR1_SMP12_2                         (0x4UL << ADC_SMPR1_SMP12_Pos)                    /*!< 0x00000100 */

#define ADC_SMPR1_SMP13_Pos                       (9U)
#define ADC_SMPR1_SMP13_Msk                       (0x7UL << ADC_SMPR1_SMP13_Pos)                    /*!< 0x00000E00 */
#define ADC_SMPR1_SMP13                           ADC_SMPR1_SMP13_Msk                               /*!< SMP13[11:9] bits (desc SMP13) */
#define ADC_SMPR1_SMP13_0                         (0x1UL << ADC_SMPR1_SMP13_Pos)                    /*!< 0x00000200 */
#define ADC_SMPR1_SMP13_1                         (0x2UL << ADC_SMPR1_SMP13_Pos)                    /*!< 0x00000400 */
#define ADC_SMPR1_SMP13_2                         (0x4UL << ADC_SMPR1_SMP13_Pos)                    /*!< 0x00000800 */

#define ADC_SMPR1_SMP14_Pos                       (12U)
#define ADC_SMPR1_SMP14_Msk                       (0x7UL << ADC_SMPR1_SMP14_Pos)                    /*!< 0x00007000 */
#define ADC_SMPR1_SMP14                           ADC_SMPR1_SMP14_Msk                               /*!< SMP14[14:12] bits (desc SMP14) */
#define ADC_SMPR1_SMP14_0                         (0x1UL << ADC_SMPR1_SMP14_Pos)                    /*!< 0x00001000 */
#define ADC_SMPR1_SMP14_1                         (0x2UL << ADC_SMPR1_SMP14_Pos)                    /*!< 0x00002000 */
#define ADC_SMPR1_SMP14_2                         (0x4UL << ADC_SMPR1_SMP14_Pos)                    /*!< 0x00004000 */

#define ADC_SMPR1_SMP15_Pos                       (15U)
#define ADC_SMPR1_SMP15_Msk                       (0x7UL << ADC_SMPR1_SMP15_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR1_SMP15                           ADC_SMPR1_SMP15_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR1_SMP15_0                         (0x1UL << ADC_SMPR1_SMP15_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR1_SMP15_1                         (0x2UL << ADC_SMPR1_SMP15_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR1_SMP15_2                         (0x4UL << ADC_SMPR1_SMP15_Pos)                    /*!< 0x00020000 */

#define ADC_SMPR1_SMP16_Pos                       (18U)
#define ADC_SMPR1_SMP16_Msk                       (0x7UL << ADC_SMPR1_SMP16_Pos)                    /*!< 0x001C0000 */
#define ADC_SMPR1_SMP16                           ADC_SMPR1_SMP16_Msk                               /*!< SMP16[20:18] bits (desc SMP16) */
#define ADC_SMPR1_SMP16_0                         (0x1UL << ADC_SMPR1_SMP16_Pos)                    /*!< 0x00040000 */
#define ADC_SMPR1_SMP16_1                         (0x2UL << ADC_SMPR1_SMP16_Pos)                    /*!< 0x00080000 */
#define ADC_SMPR1_SMP16_2                         (0x4UL << ADC_SMPR1_SMP16_Pos)                    /*!< 0x00100000 */

#define ADC_SMPR1_SMP17_Pos                       (21U)
#define ADC_SMPR1_SMP17_Msk                       (0x7UL << ADC_SMPR1_SMP17_Pos)                    /*!< 0x00E00000 */
#define ADC_SMPR1_SMP17                           ADC_SMPR1_SMP17_Msk                               /*!< SMP17[23:21] bits (desc SMP17) */
#define ADC_SMPR1_SMP17_0                         (0x1UL << ADC_SMPR1_SMP17_Pos)                    /*!< 0x00200000 */
#define ADC_SMPR1_SMP17_1                         (0x2UL << ADC_SMPR1_SMP17_Pos)                    /*!< 0x00400000 */
#define ADC_SMPR1_SMP17_2                         (0x4UL << ADC_SMPR1_SMP17_Pos)                    /*!< 0x00800000 */


/*!< ADC_SMPR2 */
#define ADC_SMPR2_SMP0_Pos                        (0U)
#define ADC_SMPR2_SMP0_Msk                        (0x7UL << ADC_SMPR2_SMP0_Pos)                     /*!< 0x00000007 */
#define ADC_SMPR2_SMP0                            ADC_SMPR2_SMP0_Msk                                /*!< SMP0[2:0] bits (desc SMP0) */
#define ADC_SMPR2_SMP0_0                          (0x1UL << ADC_SMPR2_SMP0_Pos)                     /*!< 0x00000001 */
#define ADC_SMPR2_SMP0_1                          (0x2UL << ADC_SMPR2_SMP0_Pos)                     /*!< 0x00000002 */
#define ADC_SMPR2_SMP0_2                          (0x4UL << ADC_SMPR2_SMP0_Pos)                     /*!< 0x00000004 */

#define ADC_SMPR2_SMP1_Pos                        (3U)
#define ADC_SMPR2_SMP1_Msk                        (0x7UL << ADC_SMPR2_SMP1_Pos)                     /*!< 0x00000038 */
#define ADC_SMPR2_SMP1                            ADC_SMPR2_SMP1_Msk                                /*!< SMP1[5:3] bits (desc SMP1) */
#define ADC_SMPR2_SMP1_0                          (0x1UL << ADC_SMPR2_SMP1_Pos)                     /*!< 0x00000008 */
#define ADC_SMPR2_SMP1_1                          (0x2UL << ADC_SMPR2_SMP1_Pos)                     /*!< 0x00000010 */
#define ADC_SMPR2_SMP1_2                          (0x4UL << ADC_SMPR2_SMP1_Pos)                     /*!< 0x00000020 */

#define ADC_SMPR2_SMP2_Pos                        (6U)
#define ADC_SMPR2_SMP2_Msk                        (0x7UL << ADC_SMPR2_SMP2_Pos)                     /*!< 0x000001C0 */
#define ADC_SMPR2_SMP2                            ADC_SMPR2_SMP2_Msk                                /*!< SMP2[8:6] bits (desc SMP2) */
#define ADC_SMPR2_SMP2_0                          (0x1UL << ADC_SMPR2_SMP2_Pos)                     /*!< 0x00000040 */
#define ADC_SMPR2_SMP2_1                          (0x2UL << ADC_SMPR2_SMP2_Pos)                     /*!< 0x00000080 */
#define ADC_SMPR2_SMP2_2                          (0x4UL << ADC_SMPR2_SMP2_Pos)                     /*!< 0x00000100 */

#define ADC_SMPR2_SMP3_Pos                        (9U)
#define ADC_SMPR2_SMP3_Msk                        (0x7UL << ADC_SMPR2_SMP3_Pos)                     /*!< 0x00000E00 */
#define ADC_SMPR2_SMP3                            ADC_SMPR2_SMP3_Msk                                /*!< SMP3[11:9] bits (desc SMP3) */
#define ADC_SMPR2_SMP3_0                          (0x1UL << ADC_SMPR2_SMP3_Pos)                     /*!< 0x00000200 */
#define ADC_SMPR2_SMP3_1                          (0x2UL << ADC_SMPR2_SMP3_Pos)                     /*!< 0x00000400 */
#define ADC_SMPR2_SMP3_2                          (0x4UL << ADC_SMPR2_SMP3_Pos)                     /*!< 0x00000800 */

#define ADC_SMPR2_SMP4_Pos                        (12U)
#define ADC_SMPR2_SMP4_Msk                        (0x7UL << ADC_SMPR2_SMP4_Pos)                     /*!< 0x00007000 */
#define ADC_SMPR2_SMP4                            ADC_SMPR2_SMP4_Msk                                /*!< SMP4[14:12] bits (desc SMP4) */
#define ADC_SMPR2_SMP4_0                          (0x1UL << ADC_SMPR2_SMP4_Pos)                     /*!< 0x00001000 */
#define ADC_SMPR2_SMP4_1                          (0x2UL << ADC_SMPR2_SMP4_Pos)                     /*!< 0x00002000 */
#define ADC_SMPR2_SMP4_2                          (0x4UL << ADC_SMPR2_SMP4_Pos)                     /*!< 0x00004000 */

#define ADC_SMPR2_SMP5_Pos                        (15U)
#define ADC_SMPR2_SMP5_Msk                        (0x7UL << ADC_SMPR2_SMP5_Pos)                     /*!< 0x00038000 */
#define ADC_SMPR2_SMP5                            ADC_SMPR2_SMP5_Msk                                /*!< SMP5[17:15] bits (desc SMP5) */
#define ADC_SMPR2_SMP5_0                          (0x1UL << ADC_SMPR2_SMP5_Pos)                     /*!< 0x00008000 */
#define ADC_SMPR2_SMP5_1                          (0x2UL << ADC_SMPR2_SMP5_Pos)                     /*!< 0x00010000 */
#define ADC_SMPR2_SMP5_2                          (0x4UL << ADC_SMPR2_SMP5_Pos)                     /*!< 0x00020000 */

#define ADC_SMPR2_SMP6_Pos                        (18U)
#define ADC_SMPR2_SMP6_Msk                        (0x7UL << ADC_SMPR2_SMP6_Pos)                     /*!< 0x001C0000 */
#define ADC_SMPR2_SMP6                            ADC_SMPR2_SMP6_Msk                                /*!< SMP6[20:18] bits (desc SMP6) */
#define ADC_SMPR2_SMP6_0                          (0x1UL << ADC_SMPR2_SMP6_Pos)                     /*!< 0x00040000 */
#define ADC_SMPR2_SMP6_1                          (0x2UL << ADC_SMPR2_SMP6_Pos)                     /*!< 0x00080000 */
#define ADC_SMPR2_SMP6_2                          (0x4UL << ADC_SMPR2_SMP6_Pos)                     /*!< 0x00100000 */

#define ADC_SMPR2_SMP7_Pos                        (21U)
#define ADC_SMPR2_SMP7_Msk                        (0x7UL << ADC_SMPR2_SMP7_Pos)                     /*!< 0x00E00000 */
#define ADC_SMPR2_SMP7                            ADC_SMPR2_SMP7_Msk                                /*!< SMP7[23:21] bits (desc SMP7) */
#define ADC_SMPR2_SMP7_0                          (0x1UL << ADC_SMPR2_SMP7_Pos)                     /*!< 0x00200000 */
#define ADC_SMPR2_SMP7_1                          (0x2UL << ADC_SMPR2_SMP7_Pos)                     /*!< 0x00400000 */
#define ADC_SMPR2_SMP7_2                          (0x4UL << ADC_SMPR2_SMP7_Pos)                     /*!< 0x00800000 */

#define ADC_SMPR2_SMP8_Pos                        (24U)
#define ADC_SMPR2_SMP8_Msk                        (0x7UL << ADC_SMPR2_SMP8_Pos)                     /*!< 0x07000000 */
#define ADC_SMPR2_SMP8                            ADC_SMPR2_SMP8_Msk                                /*!< SMP8[26:24] bits (desc SMP8) */
#define ADC_SMPR2_SMP8_0                          (0x1UL << ADC_SMPR2_SMP8_Pos)                     /*!< 0x01000000 */
#define ADC_SMPR2_SMP8_1                          (0x2UL << ADC_SMPR2_SMP8_Pos)                     /*!< 0x02000000 */
#define ADC_SMPR2_SMP8_2                          (0x4UL << ADC_SMPR2_SMP8_Pos)                     /*!< 0x04000000 */

#define ADC_SMPR2_SMP9_Pos                        (27U)
#define ADC_SMPR2_SMP9_Msk                        (0x7UL << ADC_SMPR2_SMP9_Pos)                     /*!< 0x38000000 */
#define ADC_SMPR2_SMP9                            ADC_SMPR2_SMP9_Msk                                /*!< SMP9[29:27] bits (desc SMP9) */
#define ADC_SMPR2_SMP9_0                          (0x1UL << ADC_SMPR2_SMP9_Pos)                     /*!< 0x08000000 */
#define ADC_SMPR2_SMP9_1                          (0x2UL << ADC_SMPR2_SMP9_Pos)                     /*!< 0x10000000 */
#define ADC_SMPR2_SMP9_2                          (0x4UL << ADC_SMPR2_SMP9_Pos)                     /*!< 0x20000000 */


/*!< ADC_JOFR1 */
#define ADC_JOFR1_JOFFSET1_Pos                    (0U)
#define ADC_JOFR1_JOFFSET1_Msk                    (0xFFFUL << ADC_JOFR1_JOFFSET1_Pos)               /*!< 0x00000FFF */
#define ADC_JOFR1_JOFFSET1                        ADC_JOFR1_JOFFSET1_Msk                            /*!< JOFFSET1[11:0] bits (desc JOFFSET1) */

/*!< ADC_JOFR2 */
#define ADC_JOFR2_JOFFSET2_Pos                    (0U)
#define ADC_JOFR2_JOFFSET2_Msk                    (0xFFFUL << ADC_JOFR2_JOFFSET2_Pos)               /*!< 0x00000FFF */
#define ADC_JOFR2_JOFFSET2                        ADC_JOFR2_JOFFSET2_Msk                            /*!< JOFFSET2[11:0] bits (desc JOFFSET2) */

/*!< ADC_JOFR3 */
#define ADC_JOFR3_JOFFSET3_Pos                    (0U)
#define ADC_JOFR3_JOFFSET3_Msk                    (0xFFFUL << ADC_JOFR3_JOFFSET3_Pos)               /*!< 0x00000FFF */
#define ADC_JOFR3_JOFFSET3                        ADC_JOFR3_JOFFSET3_Msk                            /*!< JOFFSET3[11:0] bits (desc JOFFSET3) */

/*!< ADC_JOFR4 */
#define ADC_JOFR4_JOFFSET4_Pos                    (0U)
#define ADC_JOFR4_JOFFSET4_Msk                    (0xFFFUL << ADC_JOFR4_JOFFSET4_Pos)               /*!< 0x00000FFF */
#define ADC_JOFR4_JOFFSET4                        ADC_JOFR4_JOFFSET4_Msk                            /*!< JOFFSET4[11:0] bits (desc JOFFSET4) */

/*!< ADC_HTR */
#define ADC_HTR_HT_Pos                            (0U)
#define ADC_HTR_HT_Msk                            (0xFFFUL << ADC_HTR_HT_Pos)                       /*!< 0x00000FFF */
#define ADC_HTR_HT                                ADC_HTR_HT_Msk                                    /*!< HT[11:0] bits (desc HT) */

/*!< ADC_LTR */
#define ADC_LTR_LT_Pos                            (0U)
#define ADC_LTR_LT_Msk                            (0xFFFUL << ADC_LTR_LT_Pos)                       /*!< 0x00000FFF */
#define ADC_LTR_LT                                ADC_LTR_LT_Msk                                    /*!< LT[11:0] bits (desc LT) */

/*!< ADC_SQR1 */
#define ADC_SQR1_SQ13_Pos                         (0U)
#define ADC_SQR1_SQ13_Msk                         (0x1FUL << ADC_SQR1_SQ13_Pos)                     /*!< 0x0000001F */
#define ADC_SQR1_SQ13                             ADC_SQR1_SQ13_Msk                                 /*!< SQ13[4:0] bits (desc SQ13) */
#define ADC_SQR1_SQ13_0                           (0x1UL << ADC_SQR1_SQ13_Pos)                      /*!< 0x00000001 */
#define ADC_SQR1_SQ13_1                           (0x2UL << ADC_SQR1_SQ13_Pos)                      /*!< 0x00000002 */
#define ADC_SQR1_SQ13_2                           (0x4UL << ADC_SQR1_SQ13_Pos)                      /*!< 0x00000004 */
#define ADC_SQR1_SQ13_3                           (0x8UL << ADC_SQR1_SQ13_Pos)                      /*!< 0x00000008 */
#define ADC_SQR1_SQ13_4                           (0x10UL << ADC_SQR1_SQ13_Pos)                     /*!< 0x00000010 */

#define ADC_SQR1_SQ14_Pos                         (5U)
#define ADC_SQR1_SQ14_Msk                         (0x1FUL << ADC_SQR1_SQ14_Pos)                     /*!< 0x000003E0 */
#define ADC_SQR1_SQ14                             ADC_SQR1_SQ14_Msk                                 /*!< SQ14[9:5] bits (desc SQ14) */
#define ADC_SQR1_SQ14_0                           (0x1UL << ADC_SQR1_SQ14_Pos)                      /*!< 0x00000020 */
#define ADC_SQR1_SQ14_1                           (0x2UL << ADC_SQR1_SQ14_Pos)                      /*!< 0x00000040 */
#define ADC_SQR1_SQ14_2                           (0x4UL << ADC_SQR1_SQ14_Pos)                      /*!< 0x00000080 */
#define ADC_SQR1_SQ14_3                           (0x8UL << ADC_SQR1_SQ14_Pos)                      /*!< 0x00000100 */
#define ADC_SQR1_SQ14_4                           (0x10UL << ADC_SQR1_SQ14_Pos)                     /*!< 0x00000200 */

#define ADC_SQR1_SQ15_Pos                         (10U)
#define ADC_SQR1_SQ15_Msk                         (0x1FUL << ADC_SQR1_SQ15_Pos)                     /*!< 0x00007C00 */
#define ADC_SQR1_SQ15                             ADC_SQR1_SQ15_Msk                                 /*!< SQ15[14:10] bits (desc SQ15) */
#define ADC_SQR1_SQ15_0                           (0x1UL << ADC_SQR1_SQ15_Pos)                      /*!< 0x00000400 */
#define ADC_SQR1_SQ15_1                           (0x2UL << ADC_SQR1_SQ15_Pos)                      /*!< 0x00000800 */
#define ADC_SQR1_SQ15_2                           (0x4UL << ADC_SQR1_SQ15_Pos)                      /*!< 0x00001000 */
#define ADC_SQR1_SQ15_3                           (0x8UL << ADC_SQR1_SQ15_Pos)                      /*!< 0x00002000 */
#define ADC_SQR1_SQ15_4                           (0x10UL << ADC_SQR1_SQ15_Pos)                     /*!< 0x00004000 */

#define ADC_SQR1_SQ16_Pos                         (15U)
#define ADC_SQR1_SQ16_Msk                         (0x1FUL << ADC_SQR1_SQ16_Pos)                     /*!< 0x000F8000 */
#define ADC_SQR1_SQ16                             ADC_SQR1_SQ16_Msk                                 /*!< SQ16[19:15] bits (desc SQ16) */
#define ADC_SQR1_SQ16_0                           (0x1UL << ADC_SQR1_SQ16_Pos)                      /*!< 0x00008000 */
#define ADC_SQR1_SQ16_1                           (0x2UL << ADC_SQR1_SQ16_Pos)                      /*!< 0x00010000 */
#define ADC_SQR1_SQ16_2                           (0x4UL << ADC_SQR1_SQ16_Pos)                      /*!< 0x00020000 */
#define ADC_SQR1_SQ16_3                           (0x8UL << ADC_SQR1_SQ16_Pos)                      /*!< 0x00040000 */
#define ADC_SQR1_SQ16_4                           (0x10UL << ADC_SQR1_SQ16_Pos)                     /*!< 0x00080000 */

#define ADC_SQR1_L_Pos                            (20U)
#define ADC_SQR1_L_Msk                            (0xFUL << ADC_SQR1_L_Pos)                         /*!< 0x00F00000 */
#define ADC_SQR1_L                                ADC_SQR1_L_Msk                                    /*!< L[23:20] bits (desc L) */
#define ADC_SQR1_L_0                              (0x1UL << ADC_SQR1_L_Pos)                         /*!< 0x00100000 */
#define ADC_SQR1_L_1                              (0x2UL << ADC_SQR1_L_Pos)                         /*!< 0x00200000 */
#define ADC_SQR1_L_2                              (0x4UL << ADC_SQR1_L_Pos)                         /*!< 0x00400000 */
#define ADC_SQR1_L_3                              (0x8UL << ADC_SQR1_L_Pos)                         /*!< 0x00800000 */


/*!< ADC_SQR2 */
#define ADC_SQR2_SQ7_Pos                          (0U)
#define ADC_SQR2_SQ7_Msk                          (0x1FUL << ADC_SQR2_SQ7_Pos)                      /*!< 0x0000001F */
#define ADC_SQR2_SQ7                              ADC_SQR2_SQ7_Msk                                  /*!< SQ7[4:0] bits (desc SQ7) */
#define ADC_SQR2_SQ7_0                            (0x1UL << ADC_SQR2_SQ7_Pos)                       /*!< 0x00000001 */
#define ADC_SQR2_SQ7_1                            (0x2UL << ADC_SQR2_SQ7_Pos)                       /*!< 0x00000002 */
#define ADC_SQR2_SQ7_2                            (0x4UL << ADC_SQR2_SQ7_Pos)                       /*!< 0x00000004 */
#define ADC_SQR2_SQ7_3                            (0x8UL << ADC_SQR2_SQ7_Pos)                       /*!< 0x00000008 */
#define ADC_SQR2_SQ7_4                            (0x10UL << ADC_SQR2_SQ7_Pos)                      /*!< 0x00000010 */

#define ADC_SQR2_SQ8_Pos                          (5U)
#define ADC_SQR2_SQ8_Msk                          (0x1FUL << ADC_SQR2_SQ8_Pos)                      /*!< 0x000003E0 */
#define ADC_SQR2_SQ8                              ADC_SQR2_SQ8_Msk                                  /*!< SQ8[9:5] bits (desc SQ8) */
#define ADC_SQR2_SQ8_0                            (0x1UL << ADC_SQR2_SQ8_Pos)                       /*!< 0x00000020 */
#define ADC_SQR2_SQ8_1                            (0x2UL << ADC_SQR2_SQ8_Pos)                       /*!< 0x00000040 */
#define ADC_SQR2_SQ8_2                            (0x4UL << ADC_SQR2_SQ8_Pos)                       /*!< 0x00000080 */
#define ADC_SQR2_SQ8_3                            (0x8UL << ADC_SQR2_SQ8_Pos)                       /*!< 0x00000100 */
#define ADC_SQR2_SQ8_4                            (0x10UL << ADC_SQR2_SQ8_Pos)                      /*!< 0x00000200 */

#define ADC_SQR2_SQ9_Pos                          (10U)
#define ADC_SQR2_SQ9_Msk                          (0x1FUL << ADC_SQR2_SQ9_Pos)                      /*!< 0x00007C00 */
#define ADC_SQR2_SQ9                              ADC_SQR2_SQ9_Msk                                  /*!< SQ9[14:10] bits (desc SQ9) */
#define ADC_SQR2_SQ9_0                            (0x1UL << ADC_SQR2_SQ9_Pos)                       /*!< 0x00000400 */
#define ADC_SQR2_SQ9_1                            (0x2UL << ADC_SQR2_SQ9_Pos)                       /*!< 0x00000800 */
#define ADC_SQR2_SQ9_2                            (0x4UL << ADC_SQR2_SQ9_Pos)                       /*!< 0x00001000 */
#define ADC_SQR2_SQ9_3                            (0x8UL << ADC_SQR2_SQ9_Pos)                       /*!< 0x00002000 */
#define ADC_SQR2_SQ9_4                            (0x10UL << ADC_SQR2_SQ9_Pos)                      /*!< 0x00004000 */

#define ADC_SQR2_SQ10_Pos                         (15U)
#define ADC_SQR2_SQ10_Msk                         (0x1FUL << ADC_SQR2_SQ10_Pos)                     /*!< 0x000F8000 */
#define ADC_SQR2_SQ10                             ADC_SQR2_SQ10_Msk                                 /*!< SQ10[19:15] bits (desc SQ10) */
#define ADC_SQR2_SQ10_0                           (0x1UL << ADC_SQR2_SQ10_Pos)                      /*!< 0x00008000 */
#define ADC_SQR2_SQ10_1                           (0x2UL << ADC_SQR2_SQ10_Pos)                      /*!< 0x00010000 */
#define ADC_SQR2_SQ10_2                           (0x4UL << ADC_SQR2_SQ10_Pos)                      /*!< 0x00020000 */
#define ADC_SQR2_SQ10_3                           (0x8UL << ADC_SQR2_SQ10_Pos)                      /*!< 0x00040000 */
#define ADC_SQR2_SQ10_4                           (0x10UL << ADC_SQR2_SQ10_Pos)                     /*!< 0x00080000 */

#define ADC_SQR2_SQ11_Pos                         (20U)
#define ADC_SQR2_SQ11_Msk                         (0x1FUL << ADC_SQR2_SQ11_Pos)                     /*!< 0x01F00000 */
#define ADC_SQR2_SQ11                             ADC_SQR2_SQ11_Msk                                 /*!< SQ11[24:20] bits (desc SQ11) */
#define ADC_SQR2_SQ11_0                           (0x1UL << ADC_SQR2_SQ11_Pos)                      /*!< 0x00100000 */
#define ADC_SQR2_SQ11_1                           (0x2UL << ADC_SQR2_SQ11_Pos)                      /*!< 0x00200000 */
#define ADC_SQR2_SQ11_2                           (0x4UL << ADC_SQR2_SQ11_Pos)                      /*!< 0x00400000 */
#define ADC_SQR2_SQ11_3                           (0x8UL << ADC_SQR2_SQ11_Pos)                      /*!< 0x00800000 */
#define ADC_SQR2_SQ11_4                           (0x10UL << ADC_SQR2_SQ11_Pos)                     /*!< 0x01000000 */

#define ADC_SQR2_SQ12_Pos                         (25U)
#define ADC_SQR2_SQ12_Msk                         (0x1FUL << ADC_SQR2_SQ12_Pos)                     /*!< 0x3E000000 */
#define ADC_SQR2_SQ12                             ADC_SQR2_SQ12_Msk                                 /*!< SQ12[29:25] bits (desc SQ12) */
#define ADC_SQR2_SQ12_0                           (0x1UL << ADC_SQR2_SQ12_Pos)                      /*!< 0x02000000 */
#define ADC_SQR2_SQ12_1                           (0x2UL << ADC_SQR2_SQ12_Pos)                      /*!< 0x04000000 */
#define ADC_SQR2_SQ12_2                           (0x4UL << ADC_SQR2_SQ12_Pos)                      /*!< 0x08000000 */
#define ADC_SQR2_SQ12_3                           (0x8UL << ADC_SQR2_SQ12_Pos)                      /*!< 0x10000000 */
#define ADC_SQR2_SQ12_4                           (0x10UL << ADC_SQR2_SQ12_Pos)                     /*!< 0x20000000 */


/*!< ADC_SQR3 */
#define ADC_SQR3_SQ1_Pos                          (0U)
#define ADC_SQR3_SQ1_Msk                          (0x1FUL << ADC_SQR3_SQ1_Pos)                      /*!< 0x0000001F */
#define ADC_SQR3_SQ1                              ADC_SQR3_SQ1_Msk                                  /*!< SQ1[4:0] bits (desc SQ1) */
#define ADC_SQR3_SQ1_0                            (0x1UL << ADC_SQR3_SQ1_Pos)                       /*!< 0x00000001 */
#define ADC_SQR3_SQ1_1                            (0x2UL << ADC_SQR3_SQ1_Pos)                       /*!< 0x00000002 */
#define ADC_SQR3_SQ1_2                            (0x4UL << ADC_SQR3_SQ1_Pos)                       /*!< 0x00000004 */
#define ADC_SQR3_SQ1_3                            (0x8UL << ADC_SQR3_SQ1_Pos)                       /*!< 0x00000008 */
#define ADC_SQR3_SQ1_4                            (0x10UL << ADC_SQR3_SQ1_Pos)                      /*!< 0x00000010 */

#define ADC_SQR3_SQ2_Pos                          (5U)
#define ADC_SQR3_SQ2_Msk                          (0x1FUL << ADC_SQR3_SQ2_Pos)                      /*!< 0x000003E0 */
#define ADC_SQR3_SQ2                              ADC_SQR3_SQ2_Msk                                  /*!< SQ2[9:5] bits (desc SQ2) */
#define ADC_SQR3_SQ2_0                            (0x1UL << ADC_SQR3_SQ2_Pos)                       /*!< 0x00000020 */
#define ADC_SQR3_SQ2_1                            (0x2UL << ADC_SQR3_SQ2_Pos)                       /*!< 0x00000040 */
#define ADC_SQR3_SQ2_2                            (0x4UL << ADC_SQR3_SQ2_Pos)                       /*!< 0x00000080 */
#define ADC_SQR3_SQ2_3                            (0x8UL << ADC_SQR3_SQ2_Pos)                       /*!< 0x00000100 */
#define ADC_SQR3_SQ2_4                            (0x10UL << ADC_SQR3_SQ2_Pos)                      /*!< 0x00000200 */

#define ADC_SQR3_SQ3_Pos                          (10U)
#define ADC_SQR3_SQ3_Msk                          (0x1FUL << ADC_SQR3_SQ3_Pos)                      /*!< 0x00007C00 */
#define ADC_SQR3_SQ3                              ADC_SQR3_SQ3_Msk                                  /*!< SQ3[14:10] bits (desc SQ3) */
#define ADC_SQR3_SQ3_0                            (0x1UL << ADC_SQR3_SQ3_Pos)                       /*!< 0x00000400 */
#define ADC_SQR3_SQ3_1                            (0x2UL << ADC_SQR3_SQ3_Pos)                       /*!< 0x00000800 */
#define ADC_SQR3_SQ3_2                            (0x4UL << ADC_SQR3_SQ3_Pos)                       /*!< 0x00001000 */
#define ADC_SQR3_SQ3_3                            (0x8UL << ADC_SQR3_SQ3_Pos)                       /*!< 0x00002000 */
#define ADC_SQR3_SQ3_4                            (0x10UL << ADC_SQR3_SQ3_Pos)                      /*!< 0x00004000 */

#define ADC_SQR3_SQ4_Pos                          (15U)
#define ADC_SQR3_SQ4_Msk                          (0x1FUL << ADC_SQR3_SQ4_Pos)                      /*!< 0x000F8000 */
#define ADC_SQR3_SQ4                              ADC_SQR3_SQ4_Msk                                  /*!< SQ4[19:15] bits (desc SQ4) */
#define ADC_SQR3_SQ4_0                            (0x1UL << ADC_SQR3_SQ4_Pos)                       /*!< 0x00008000 */
#define ADC_SQR3_SQ4_1                            (0x2UL << ADC_SQR3_SQ4_Pos)                       /*!< 0x00010000 */
#define ADC_SQR3_SQ4_2                            (0x4UL << ADC_SQR3_SQ4_Pos)                       /*!< 0x00020000 */
#define ADC_SQR3_SQ4_3                            (0x8UL << ADC_SQR3_SQ4_Pos)                       /*!< 0x00040000 */
#define ADC_SQR3_SQ4_4                            (0x10UL << ADC_SQR3_SQ4_Pos)                      /*!< 0x00080000 */

#define ADC_SQR3_SQ5_Pos                          (20U)
#define ADC_SQR3_SQ5_Msk                          (0x1FUL << ADC_SQR3_SQ5_Pos)                      /*!< 0x000F8000 */
#define ADC_SQR3_SQ5                              ADC_SQR3_SQ5_Msk                                  /*!< SQ4[24:20] bits (desc SQ5) */
#define ADC_SQR3_SQ5_0                            (0x1UL << ADC_SQR3_SQ5_Pos)                       /*!< 0x00008000 */
#define ADC_SQR3_SQ5_1                            (0x2UL << ADC_SQR3_SQ5_Pos)                       /*!< 0x00010000 */
#define ADC_SQR3_SQ5_2                            (0x4UL << ADC_SQR3_SQ5_Pos)                       /*!< 0x00020000 */
#define ADC_SQR3_SQ5_3                            (0x8UL << ADC_SQR3_SQ5_Pos)                       /*!< 0x00040000 */
#define ADC_SQR3_SQ5_4                            (0x10UL << ADC_SQR3_SQ5_Pos)                      /*!< 0x00080000 */

#define ADC_SQR3_SQ6_Pos                          (25U)
#define ADC_SQR3_SQ6_Msk                          (0x1FUL << ADC_SQR3_SQ6_Pos)                      /*!< 0x3E000000 */
#define ADC_SQR3_SQ6                              ADC_SQR3_SQ6_Msk                                  /*!< SQ6[29:25] bits (desc SQ6) */
#define ADC_SQR3_SQ6_0                            (0x1UL << ADC_SQR3_SQ6_Pos)                       /*!< 0x02000000 */
#define ADC_SQR3_SQ6_1                            (0x2UL << ADC_SQR3_SQ6_Pos)                       /*!< 0x04000000 */
#define ADC_SQR3_SQ6_2                            (0x4UL << ADC_SQR3_SQ6_Pos)                       /*!< 0x08000000 */
#define ADC_SQR3_SQ6_3                            (0x8UL << ADC_SQR3_SQ6_Pos)                       /*!< 0x10000000 */
#define ADC_SQR3_SQ6_4                            (0x10UL << ADC_SQR3_SQ6_Pos)                      /*!< 0x20000000 */


/*!< ADC_JSQR */
#define ADC_JSQR_JSQ1_Pos                         (0U)
#define ADC_JSQR_JSQ1_Msk                         (0x1FUL << ADC_JSQR_JSQ1_Pos)                     /*!< 0x0000001F */
#define ADC_JSQR_JSQ1                             ADC_JSQR_JSQ1_Msk                                 /*!< JSQ1[4:0] bits (desc JSQ1) */
#define ADC_JSQR_JSQ1_0                           (0x1UL << ADC_JSQR_JSQ1_Pos)                      /*!< 0x00000001 */
#define ADC_JSQR_JSQ1_1                           (0x2UL << ADC_JSQR_JSQ1_Pos)                      /*!< 0x00000002 */
#define ADC_JSQR_JSQ1_2                           (0x4UL << ADC_JSQR_JSQ1_Pos)                      /*!< 0x00000004 */
#define ADC_JSQR_JSQ1_3                           (0x8UL << ADC_JSQR_JSQ1_Pos)                      /*!< 0x00000008 */
#define ADC_JSQR_JSQ1_4                           (0x10UL << ADC_JSQR_JSQ1_Pos)                     /*!< 0x00000010 */

#define ADC_JSQR_JSQ2_Pos                         (5U)
#define ADC_JSQR_JSQ2_Msk                         (0x1FUL << ADC_JSQR_JSQ2_Pos)                     /*!< 0x000003E0 */
#define ADC_JSQR_JSQ2                             ADC_JSQR_JSQ2_Msk                                 /*!< JSQ2[9:5] bits (desc JSQ2) */
#define ADC_JSQR_JSQ2_0                           (0x1UL << ADC_JSQR_JSQ2_Pos)                      /*!< 0x00000020 */
#define ADC_JSQR_JSQ2_1                           (0x2UL << ADC_JSQR_JSQ2_Pos)                      /*!< 0x00000040 */
#define ADC_JSQR_JSQ2_2                           (0x4UL << ADC_JSQR_JSQ2_Pos)                      /*!< 0x00000080 */
#define ADC_JSQR_JSQ2_3                           (0x8UL << ADC_JSQR_JSQ2_Pos)                      /*!< 0x00000100 */
#define ADC_JSQR_JSQ2_4                           (0x10UL << ADC_JSQR_JSQ2_Pos)                     /*!< 0x00000200 */

#define ADC_JSQR_JSQ3_Pos                         (10U)
#define ADC_JSQR_JSQ3_Msk                         (0x1FUL << ADC_JSQR_JSQ3_Pos)                     /*!< 0x00007C00 */
#define ADC_JSQR_JSQ3                             ADC_JSQR_JSQ3_Msk                                 /*!< JSQ3[14:10] bits (desc JSQ3) */
#define ADC_JSQR_JSQ3_0                           (0x1UL << ADC_JSQR_JSQ3_Pos)                      /*!< 0x00000400 */
#define ADC_JSQR_JSQ3_1                           (0x2UL << ADC_JSQR_JSQ3_Pos)                      /*!< 0x00000800 */
#define ADC_JSQR_JSQ3_2                           (0x4UL << ADC_JSQR_JSQ3_Pos)                      /*!< 0x00001000 */
#define ADC_JSQR_JSQ3_3                           (0x8UL << ADC_JSQR_JSQ3_Pos)                      /*!< 0x00002000 */
#define ADC_JSQR_JSQ3_4                           (0x10UL << ADC_JSQR_JSQ3_Pos)                     /*!< 0x00004000 */

#define ADC_JSQR_JSQ4_Pos                         (15U)
#define ADC_JSQR_JSQ4_Msk                         (0x1FUL << ADC_JSQR_JSQ4_Pos)                     /*!< 0x000F8000 */
#define ADC_JSQR_JSQ4                             ADC_JSQR_JSQ4_Msk                                 /*!< JSQ4[19:15] bits (desc JSQ4) */
#define ADC_JSQR_JSQ4_0                           (0x1UL << ADC_JSQR_JSQ4_Pos)                      /*!< 0x00008000 */
#define ADC_JSQR_JSQ4_1                           (0x2UL << ADC_JSQR_JSQ4_Pos)                      /*!< 0x00010000 */
#define ADC_JSQR_JSQ4_2                           (0x4UL << ADC_JSQR_JSQ4_Pos)                      /*!< 0x00020000 */
#define ADC_JSQR_JSQ4_3                           (0x8UL << ADC_JSQR_JSQ4_Pos)                      /*!< 0x00040000 */
#define ADC_JSQR_JSQ4_4                           (0x10UL << ADC_JSQR_JSQ4_Pos)                     /*!< 0x00080000 */

#define ADC_JSQR_JL_Pos                           (20U)
#define ADC_JSQR_JL_Msk                           (0x3UL << ADC_JSQR_JL_Pos)                        /*!< 0x00300000 */
#define ADC_JSQR_JL                               ADC_JSQR_JL_Msk                                   /*!< JL[21:20] bits (desc JL) */
#define ADC_JSQR_JL_0                             (0x1UL << ADC_JSQR_JL_Pos)                        /*!< 0x00100000 */
#define ADC_JSQR_JL_1                             (0x2UL << ADC_JSQR_JL_Pos)                        /*!< 0x00200000 */


/*!< ADC_JDR1 */
#define ADC_JDR1_JDR1_Pos                         (0U)
#define ADC_JDR1_JDR1_Msk                         (0xFFFFUL << ADC_JDR1_JDR1_Pos)                   /*!< 0x0000FFFF */
#define ADC_JDR1_JDR1                             ADC_JDR1_JDR1_Msk                                 /*!< JDR1[15:0] bits (desc JDR1) */

/*!< ADC_JDR2 */
#define ADC_JDR2_JDR2_Pos                         (0U)
#define ADC_JDR2_JDR2_Msk                         (0xFFFFUL << ADC_JDR2_JDR2_Pos)                   /*!< 0x0000FFFF */
#define ADC_JDR2_JDR2                             ADC_JDR2_JDR2_Msk                                 /*!< JDR2[15:0] bits (desc JDR2) */

/*!< ADC_JDR3 */
#define ADC_JDR3_JDR3_Pos                         (0U)
#define ADC_JDR3_JDR3_Msk                         (0xFFFFUL << ADC_JDR3_JDR3_Pos)                   /*!< 0x0000FFFF */
#define ADC_JDR3_JDR3                             ADC_JDR3_JDR3_Msk                                 /*!< JDR3[15:0] bits (desc JDR3) */

/*!< ADC_JDR4 */
#define ADC_JDR4_JDR4_Pos                         (0U)
#define ADC_JDR4_JDR4_Msk                         (0xFFFFUL << ADC_JDR4_JDR4_Pos)                   /*!< 0x0000FFFF */
#define ADC_JDR4_JDR4                             ADC_JDR4_JDR4_Msk                                 /*!< JDR4[15:0] bits (desc JDR4) */

/*!< ADC_DR */
#define ADC_DR_DATA_Pos                           (0U)
#define ADC_DR_DATA_Msk                           (0xFFFFUL << ADC_DR_DATA_Pos)                     /*!< 0x0000FFFF */
#define ADC_DR_DATA                               ADC_DR_DATA_Msk                                   /*!< DATA[15:0] bits (desc DATA) */
#define ADC_DR_ADC2DATA_Pos                       (16U)
#define ADC_DR_ADC2DATA_Msk                       (0xFFFFUL << ADC_DR_ADC2DATA_Pos)                 /*!< 0xFFFF0000 */
#define ADC_DR_ADC2DATA                           ADC_DR_ADC2DATA_Msk                               /*!< ADC2DATA[31:16] bits (desc ADC2DATA) */

/*!< ADC_CCSR */
#define ADC_CCSR_CALSEL_Pos                       (11U)
#define ADC_CCSR_CALSEL_Msk                       (0x1UL << ADC_CCSR_CALSEL_Pos)                    /*!< 0x00000800 */
#define ADC_CCSR_CALSEL                           ADC_CCSR_CALSEL_Msk                               /*!< desc CALSEL */

#define ADC_CCSR_CALSMP_Pos                       (12U)
#define ADC_CCSR_CALSMP_Msk                       (0x3UL << ADC_CCSR_CALSMP_Pos)                    /*!< 0x00003000 */
#define ADC_CCSR_CALSMP                           ADC_CCSR_CALSMP_Msk                               /*!< CALSMP[13:12] bits (desc CALSMP) */
#define ADC_CCSR_CALSMP_0                         (0x1UL << ADC_CCSR_CALSMP_Pos)                    /*!< 0x00001000 */
#define ADC_CCSR_CALSMP_1                         (0x2UL << ADC_CCSR_CALSMP_Pos)                    /*!< 0x00002000 */

// #define ADC_CCSR_CALBYP_Pos                       (14U)
// #define ADC_CCSR_CALBYP_Msk                       (0x1UL << ADC_CCSR_CALBYP_Pos)                    /*!< 0x00004000 */
// #define ADC_CCSR_CALBYP                           ADC_CCSR_CALBYP_Msk                               /*!< desc CALBYP */

// #define ADC_CCSR_CALSET_Pos                       (15U)
// #define ADC_CCSR_CALSET_Msk                       (0x1UL << ADC_CCSR_CALSET_Pos)                    /*!< 0x00008000 */
// #define ADC_CCSR_CALSET                           ADC_CCSR_CALSET_Msk                               /*!< desc CALSET */

#define ADC_CCSR_OFFSUC_Pos                       (29U)
#define ADC_CCSR_OFFSUC_Msk                       (0x1UL << ADC_CCSR_OFFSUC_Pos)                    /*!< 0x20000000 */
#define ADC_CCSR_OFFSUC                           ADC_CCSR_OFFSUC_Msk                               /*!< desc OFFSUC */

#define ADC_CCSR_CAPSUC_Pos                       (30U)
#define ADC_CCSR_CAPSUC_Msk                       (0x1UL << ADC_CCSR_CAPSUC_Pos)                    /*!< 0x40000000 */
#define ADC_CCSR_CAPSUC                           ADC_CCSR_CAPSUC_Msk                               /*!< desc CAPSUC */

#define ADC_CCSR_CALON_Pos                        (31U)
#define ADC_CCSR_CALON_Msk                        (0x1UL << ADC_CCSR_CALON_Pos)                     /*!< 0x80000000 */
#define ADC_CCSR_CALON                            ADC_CCSR_CALON_Msk                                /*!< desc CALON */

// /*!< ADC_CALRR1 */
// #define ADC_CALRR1_CALC10OUT_Pos                  (0U)
// #define ADC_CALRR1_CALC10OUT_Msk                  (0xFFUL << ADC_CALRR1_CALC10OUT_Pos)              /*!< 0x000000FF */
// #define ADC_CALRR1_CALC10OUT                      ADC_CALRR1_CALC10OUT_Msk                          /*!< CALC10OUT[7:0] bits (desc CALC10OUT) */
// #define ADC_CALRR1_CALC11OUT_Pos                  (8U)
// #define ADC_CALRR1_CALC11OUT_Msk                  (0xFFUL << ADC_CALRR1_CALC11OUT_Pos)              /*!< 0x0000FF00 */
// #define ADC_CALRR1_CALC11OUT                      ADC_CALRR1_CALC11OUT_Msk                          /*!< CALC11OUT[15:8] bits (desc CALC11OUT) */
// #define ADC_CALRR1_CALBOUT_Pos                    (16U)
// #define ADC_CALRR1_CALBOUT_Msk                    (0xFFUL << ADC_CALRR1_CALBOUT_Pos)                /*!< 0x00FF0000 */
// #define ADC_CALRR1_CALBOUT                        ADC_CALRR1_CALBOUT_Msk                            /*!< CALBOUT[23:16] bits (desc CALBOUT) */

// /*!< ADC_CALRR2 */
// #define ADC_CALRR2_CALC6OUT_Pos                   (0U)
// #define ADC_CALRR2_CALC6OUT_Msk                   (0xFFUL << ADC_CALRR2_CALC6OUT_Pos)               /*!< 0x000000FF */
// #define ADC_CALRR2_CALC6OUT                       ADC_CALRR2_CALC6OUT_Msk                           /*!< CALC6OUT[7:0] bits (desc CALC6OUT) */
// #define ADC_CALRR2_CALC7OUT_Pos                   (8U)
// #define ADC_CALRR2_CALC7OUT_Msk                   (0xFFUL << ADC_CALRR2_CALC7OUT_Pos)               /*!< 0x0000FF00 */
// #define ADC_CALRR2_CALC7OUT                       ADC_CALRR2_CALC7OUT_Msk                           /*!< CALC7OUT[15:8] bits (desc CALC7OUT) */
// #define ADC_CALRR2_CALC8OUT_Pos                   (16U)
// #define ADC_CALRR2_CALC8OUT_Msk                   (0xFFUL << ADC_CALRR2_CALC8OUT_Pos)               /*!< 0x00FF0000 */
// #define ADC_CALRR2_CALC8OUT                       ADC_CALRR2_CALC8OUT_Msk                           /*!< CALC8OUT[23:16] bits (desc CALC8OUT) */
// #define ADC_CALRR2_CALC9OUT_Pos                   (24U)
// #define ADC_CALRR2_CALC9OUT_Msk                   (0xFFUL << ADC_CALRR2_CALC9OUT_Pos)               /*!< 0xFF000000 */
// #define ADC_CALRR2_CALC9OUT                       ADC_CALRR2_CALC9OUT_Msk                           /*!< CALC9OUT[31:24] bits (desc CALC9OUT) */

// /*!< ADC_CALFIR1 */
// #define ADC_CALFIR1_CALC10IO_Pos                  (0U)
// #define ADC_CALFIR1_CALC10IO_Msk                  (0xFFUL << ADC_CALFIR1_CALC10IO_Pos)              /*!< 0x000000FF */
// #define ADC_CALFIR1_CALC10IO                      ADC_CALFIR1_CALC10IO_Msk                          /*!< CALC10IO[7:0] bits (desc CALC10IO) */
// #define ADC_CALFIR1_CALC11IO_Pos                  (8U)
// #define ADC_CALFIR1_CALC11IO_Msk                  (0xFFUL << ADC_CALFIR1_CALC11IO_Pos)              /*!< 0x0000FF00 */
// #define ADC_CALFIR1_CALC11IO                      ADC_CALFIR1_CALC11IO_Msk                          /*!< CALC11IO[15:8] bits (desc CALC11IO) */
// #define ADC_CALFIR1_CALBIO_Pos                    (16U)
// #define ADC_CALFIR1_CALBIO_Msk                    (0xFFUL << ADC_CALFIR1_CALBIO_Pos)                /*!< 0x00FF0000 */
// #define ADC_CALFIR1_CALBIO                        ADC_CALFIR1_CALBIO_Msk                            /*!< CALBIO[23:16] bits (desc CALBIO) */

// /*!< ADC_CALFIR2 */
// #define ADC_CALFIR2_CALC6IO_Pos                   (0U)
// #define ADC_CALFIR2_CALC6IO_Msk                   (0xFFUL << ADC_CALFIR2_CALC6IO_Pos)               /*!< 0x000000FF */
// #define ADC_CALFIR2_CALC6IO                       ADC_CALFIR2_CALC6IO_Msk                           /*!< CALC6IO[7:0] bits (desc CALC6IO) */
// #define ADC_CALFIR2_CALC7IO_Pos                   (8U)
// #define ADC_CALFIR2_CALC7IO_Msk                   (0xFFUL << ADC_CALFIR2_CALC7IO_Pos)               /*!< 0x0000FF00 */
// #define ADC_CALFIR2_CALC7IO                       ADC_CALFIR2_CALC7IO_Msk                           /*!< CALC7IO[15:8] bits (desc CALC7IO) */
// #define ADC_CALFIR2_CALC8IO_Pos                   (16U)
// #define ADC_CALFIR2_CALC8IO_Msk                   (0xFFUL << ADC_CALFIR2_CALC8IO_Pos)               /*!< 0x00FF0000 */
// #define ADC_CALFIR2_CALC8IO                       ADC_CALFIR2_CALC8IO_Msk                           /*!< CALC8IO[23:16] bits (desc CALC8IO) */
// #define ADC_CALFIR2_CALC9IO_Pos                   (24U)
// #define ADC_CALFIR2_CALC9IO_Msk                   (0xFFUL << ADC_CALFIR2_CALC9IO_Pos)               /*!< 0xFF000000 */
// #define ADC_CALFIR2_CALC9IO                       ADC_CALFIR2_CALC9IO_Msk                           /*!< CALC9IO[31:24] bits (desc CALC9IO) */

/*********************  Bits Define For Peripheral BKP  *********************/
/*!< BKP_DR1 */
#define BKP_DR1_D_Pos                             (0U)
#define BKP_DR1_D_Msk                             (0xFFFFUL << BKP_DR1_D_Pos)                       /*!< 0x0000FFFF */
#define BKP_DR1_D                                 BKP_DR1_D_Msk                                     /*!< D[15:0] bits (desc D) */

/*!< BKP_DR2 */
#define BKP_DR2_D_Pos                             (0U)
#define BKP_DR2_D_Msk                             (0xFFFFUL << BKP_DR2_D_Pos)                       /*!< 0x0000FFFF */
#define BKP_DR2_D                                 BKP_DR2_D_Msk                                     /*!< D[15:0] bits (desc D) */

/*!< BKP_DR3 */
#define BKP_DR3_D_Pos                             (0U)
#define BKP_DR3_D_Msk                             (0xFFFFUL << BKP_DR3_D_Pos)                       /*!< 0x0000FFFF */
#define BKP_DR3_D                                 BKP_DR3_D_Msk                                     /*!< D[15:0] bits (desc D) */

/*!< BKP_DR4 */
#define BKP_DR4_D_Pos                             (0U)
#define BKP_DR4_D_Msk                             (0xFFFFUL << BKP_DR4_D_Pos)                       /*!< 0x0000FFFF */
#define BKP_DR4_D                                 BKP_DR4_D_Msk                                     /*!< D[15:0] bits (desc D) */

/*!< BKP_DR5 */
#define BKP_DR5_D_Pos                             (0U)
#define BKP_DR5_D_Msk                             (0xFFFFUL << BKP_DR5_D_Pos)                       /*!< 0x0000FFFF */
#define BKP_DR5_D                                 BKP_DR5_D_Msk                                     /*!< D[15:0] bits (desc D) */

/*!< BKP_DR6 */
#define BKP_DR6_D_Pos                             (0U)
#define BKP_DR6_D_Msk                             (0xFFFFUL << BKP_DR6_D_Pos)                       /*!< 0x0000FFFF */
#define BKP_DR6_D                                 BKP_DR6_D_Msk                                     /*!< D[15:0] bits (desc D) */

/*!< BKP_DR7 */
#define BKP_DR7_D_Pos                             (0U)
#define BKP_DR7_D_Msk                             (0xFFFFUL << BKP_DR7_D_Pos)                       /*!< 0x0000FFFF */
#define BKP_DR7_D                                 BKP_DR7_D_Msk                                     /*!< D[15:0] bits (desc D) */

/*!< BKP_DR8 */
#define BKP_DR8_D_Pos                             (0U)
#define BKP_DR8_D_Msk                             (0xFFFFUL << BKP_DR8_D_Pos)                       /*!< 0x0000FFFF */
#define BKP_DR8_D                                 BKP_DR8_D_Msk                                     /*!< D[15:0] bits (desc D) */

/*!< BKP_DR9 */
#define BKP_DR9_D_Pos                             (0U)
#define BKP_DR9_D_Msk                             (0xFFFFUL << BKP_DR9_D_Pos)                       /*!< 0x0000FFFF */
#define BKP_DR9_D                                 BKP_DR9_D_Msk                                     /*!< D[15:0] bits (desc D) */

/*!< BKP_DR10 */
#define BKP_DR10_D_Pos                            (0U)
#define BKP_DR10_D_Msk                            (0xFFFFUL << BKP_DR10_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR10_D                                BKP_DR10_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_RTCCR */
#define BKP_RTCCR_CAL_Pos                         (0U)
#define BKP_RTCCR_CAL_Msk                         (0x7FUL << BKP_RTCCR_CAL_Pos)                     /*!< 0x0000007F */
#define BKP_RTCCR_CAL                             BKP_RTCCR_CAL_Msk                                 /*!< CAL[6:0] bits (desc CAL) */
#define BKP_RTCCR_CAL_0                           (0x1UL << BKP_RTCCR_CAL_Pos)                      /*!< 0x00000001 */
#define BKP_RTCCR_CAL_1                           (0x2UL << BKP_RTCCR_CAL_Pos)                      /*!< 0x00000002 */
#define BKP_RTCCR_CAL_2                           (0x4UL << BKP_RTCCR_CAL_Pos)                      /*!< 0x00000004 */
#define BKP_RTCCR_CAL_3                           (0x8UL << BKP_RTCCR_CAL_Pos)                      /*!< 0x00000008 */
#define BKP_RTCCR_CAL_4                           (0x10UL << BKP_RTCCR_CAL_Pos)                     /*!< 0x00000010 */
#define BKP_RTCCR_CAL_5                           (0x20UL << BKP_RTCCR_CAL_Pos)                     /*!< 0x00000020 */
#define BKP_RTCCR_CAL_6                           (0x40UL << BKP_RTCCR_CAL_Pos)                     /*!< 0x00000040 */

#define BKP_RTCCR_CCO_Pos                         (7U)
#define BKP_RTCCR_CCO_Msk                         (0x1UL << BKP_RTCCR_CCO_Pos)                      /*!< 0x00000080 */
#define BKP_RTCCR_CCO                             BKP_RTCCR_CCO_Msk                                 /*!< desc CCO */
#define BKP_RTCCR_ASOE_Pos                        (8U)
#define BKP_RTCCR_ASOE_Msk                        (0x1UL << BKP_RTCCR_ASOE_Pos)                     /*!< 0x00000100 */
#define BKP_RTCCR_ASOE                            BKP_RTCCR_ASOE_Msk                                /*!< desc ASOE */
#define BKP_RTCCR_ASOS_Pos                        (9U)
#define BKP_RTCCR_ASOS_Msk                        (0x1UL << BKP_RTCCR_ASOS_Pos)                     /*!< 0x00000200 */
#define BKP_RTCCR_ASOS                            BKP_RTCCR_ASOS_Msk                                /*!< desc ASOS */

/*!< BKP_CR */
#define BKP_CR_TPE_Pos                            (0U)
#define BKP_CR_TPE_Msk                            (0x1UL << BKP_CR_TPE_Pos)                         /*!< 0x00000001 */
#define BKP_CR_TPE                                BKP_CR_TPE_Msk                                    /*!< desc TPE */
#define BKP_CR_TPAL_Pos                           (1U)
#define BKP_CR_TPAL_Msk                           (0x1UL << BKP_CR_TPAL_Pos)                        /*!< 0x00000002 */
#define BKP_CR_TPAL                               BKP_CR_TPAL_Msk                                   /*!< desc TPAL */

/*!< BKP_CSR */
#define BKP_CSR_CTE_Pos                           (0U)
#define BKP_CSR_CTE_Msk                           (0x1UL << BKP_CSR_CTE_Pos)                        /*!< 0x00000001 */
#define BKP_CSR_CTE                               BKP_CSR_CTE_Msk                                   /*!< desc CTE */
#define BKP_CSR_CTI_Pos                           (1U)
#define BKP_CSR_CTI_Msk                           (0x1UL << BKP_CSR_CTI_Pos)                        /*!< 0x00000002 */
#define BKP_CSR_CTI                               BKP_CSR_CTI_Msk                                   /*!< desc CTI */
#define BKP_CSR_TPIE_Pos                          (2U)
#define BKP_CSR_TPIE_Msk                          (0x1UL << BKP_CSR_TPIE_Pos)                       /*!< 0x00000004 */
#define BKP_CSR_TPIE                              BKP_CSR_TPIE_Msk                                  /*!< desc TPIE */
#define BKP_CSR_TEF_Pos                           (8U)
#define BKP_CSR_TEF_Msk                           (0x1UL << BKP_CSR_TEF_Pos)                        /*!< 0x00000100 */
#define BKP_CSR_TEF                               BKP_CSR_TEF_Msk                                   /*!< desc TEF */
#define BKP_CSR_TIF_Pos                           (9U)
#define BKP_CSR_TIF_Msk                           (0x1UL << BKP_CSR_TIF_Pos)                        /*!< 0x00000200 */
#define BKP_CSR_TIF                               BKP_CSR_TIF_Msk                                   /*!< desc TIF */

/*!< BKP_DR11 */
#define BKP_DR11_D_Pos                            (0U)
#define BKP_DR11_D_Msk                            (0xFFFFUL << BKP_DR11_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR11_D                                BKP_DR11_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR12 */
#define BKP_DR12_D_Pos                            (0U)
#define BKP_DR12_D_Msk                            (0xFFFFUL << BKP_DR12_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR12_D                                BKP_DR12_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR13 */
#define BKP_DR13_D_Pos                            (0U)
#define BKP_DR13_D_Msk                            (0xFFFFUL << BKP_DR13_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR13_D                                BKP_DR13_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR14 */
#define BKP_DR14_D_Pos                            (0U)
#define BKP_DR14_D_Msk                            (0xFFFFUL << BKP_DR14_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR14_D                                BKP_DR14_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR15 */
#define BKP_DR15_D_Pos                            (0U)
#define BKP_DR15_D_Msk                            (0xFFFFUL << BKP_DR15_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR15_D                                BKP_DR15_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR16 */
#define BKP_DR16_D_Pos                            (0U)
#define BKP_DR16_D_Msk                            (0xFFFFUL << BKP_DR16_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR16_D                                BKP_DR16_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR17 */
#define BKP_DR17_D_Pos                            (0U)
#define BKP_DR17_D_Msk                            (0xFFFFUL << BKP_DR17_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR17_D                                BKP_DR17_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR18 */
#define BKP_DR18_D_Pos                            (0U)
#define BKP_DR18_D_Msk                            (0xFFFFUL << BKP_DR18_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR18_D                                BKP_DR18_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR19 */
#define BKP_DR19_D_Pos                            (0U)
#define BKP_DR19_D_Msk                            (0xFFFFUL << BKP_DR19_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR19_D                                BKP_DR19_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR20 */
#define BKP_DR20_D_Pos                            (0U)
#define BKP_DR20_D_Msk                            (0xFFFFUL << BKP_DR20_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR20_D                                BKP_DR20_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR21 */
#define BKP_DR21_D_Pos                            (0U)
#define BKP_DR21_D_Msk                            (0xFFFFUL << BKP_DR21_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR21_D                                BKP_DR21_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR22 */
#define BKP_DR22_D_Pos                            (0U)
#define BKP_DR22_D_Msk                            (0xFFFFUL << BKP_DR22_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR22_D                                BKP_DR22_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR23 */
#define BKP_DR23_D_Pos                            (0U)
#define BKP_DR23_D_Msk                            (0xFFFFUL << BKP_DR23_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR23_D                                BKP_DR23_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR24 */
#define BKP_DR24_D_Pos                            (0U)
#define BKP_DR24_D_Msk                            (0xFFFFUL << BKP_DR24_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR24_D                                BKP_DR24_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR25 */
#define BKP_DR25_D_Pos                            (0U)
#define BKP_DR25_D_Msk                            (0xFFFFUL << BKP_DR25_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR25_D                                BKP_DR25_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR26 */
#define BKP_DR26_D_Pos                            (0U)
#define BKP_DR26_D_Msk                            (0xFFFFUL << BKP_DR26_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR26_D                                BKP_DR26_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR27 */
#define BKP_DR27_D_Pos                            (0U)
#define BKP_DR27_D_Msk                            (0xFFFFUL << BKP_DR27_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR27_D                                BKP_DR27_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR28 */
#define BKP_DR28_D_Pos                            (0U)
#define BKP_DR28_D_Msk                            (0xFFFFUL << BKP_DR28_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR28_D                                BKP_DR28_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR29 */
#define BKP_DR29_D_Pos                            (0U)
#define BKP_DR29_D_Msk                            (0xFFFFUL << BKP_DR29_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR29_D                                BKP_DR29_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR30 */
#define BKP_DR30_D_Pos                            (0U)
#define BKP_DR30_D_Msk                            (0xFFFFUL << BKP_DR30_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR30_D                                BKP_DR30_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR31 */
#define BKP_DR31_D_Pos                            (0U)
#define BKP_DR31_D_Msk                            (0xFFFFUL << BKP_DR31_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR31_D                                BKP_DR31_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR32 */
#define BKP_DR32_D_Pos                            (0U)
#define BKP_DR32_D_Msk                            (0xFFFFUL << BKP_DR32_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR32_D                                BKP_DR32_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR33 */
#define BKP_DR33_D_Pos                            (0U)
#define BKP_DR33_D_Msk                            (0xFFFFUL << BKP_DR33_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR33_D                                BKP_DR33_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR34 */
#define BKP_DR34_D_Pos                            (0U)
#define BKP_DR34_D_Msk                            (0xFFFFUL << BKP_DR34_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR34_D                                BKP_DR34_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR35 */
#define BKP_DR35_D_Pos                            (0U)
#define BKP_DR35_D_Msk                            (0xFFFFUL << BKP_DR35_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR35_D                                BKP_DR35_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR36 */
#define BKP_DR36_D_Pos                            (0U)
#define BKP_DR36_D_Msk                            (0xFFFFUL << BKP_DR36_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR36_D                                BKP_DR36_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR37 */
#define BKP_DR37_D_Pos                            (0U)
#define BKP_DR37_D_Msk                            (0xFFFFUL << BKP_DR37_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR37_D                                BKP_DR37_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR38 */
#define BKP_DR38_D_Pos                            (0U)
#define BKP_DR38_D_Msk                            (0xFFFFUL << BKP_DR38_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR38_D                                BKP_DR38_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR39 */
#define BKP_DR39_D_Pos                            (0U)
#define BKP_DR39_D_Msk                            (0xFFFFUL << BKP_DR39_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR39_D                                BKP_DR39_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR40 */
#define BKP_DR40_D_Pos                            (0U)
#define BKP_DR40_D_Msk                            (0xFFFFUL << BKP_DR40_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR40_D                                BKP_DR40_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR41 */
#define BKP_DR41_D_Pos                            (0U)
#define BKP_DR41_D_Msk                            (0xFFFFUL << BKP_DR41_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR41_D                                BKP_DR41_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*!< BKP_DR42 */
#define BKP_DR42_D_Pos                            (0U)
#define BKP_DR42_D_Msk                            (0xFFFFUL << BKP_DR42_D_Pos)                      /*!< 0x0000FFFF */
#define BKP_DR42_D                                BKP_DR42_D_Msk                                    /*!< D[15:0] bits (desc D) */

/*********************  Bits Define For Peripheral CANFD  *********************/
/*!< CANFD_TSNCR */
#define CANFD_TSNCR_VERSION_Pos                   (0U)
#define CANFD_TSNCR_VERSION_Msk                   (0xFFFFUL << CANFD_TSNCR_VERSION_Pos)             /*!< 0x0000FFFF */
#define CANFD_TSNCR_VERSION                       CANFD_TSNCR_VERSION_Msk                           /*!< VERSION[15:0] bits (desc VERSION) */
#define CANFD_TSNCR_CES_Pos                       (16U)
#define CANFD_TSNCR_CES_Msk                       (0x1UL << CANFD_TSNCR_CES_Pos)                    /*!< 0x00010000 */
#define CANFD_TSNCR_CES                           CANFD_TSNCR_CES_Msk                               /*!< desc CES */
#define CANFD_TSNCR_ROP_Pos                       (17U)
#define CANFD_TSNCR_ROP_Msk                       (0x1UL << CANFD_TSNCR_ROP_Pos)                    /*!< 0x00020000 */
#define CANFD_TSNCR_ROP                           CANFD_TSNCR_ROP_Msk                               /*!< desc ROP */
// #define CANFD_TSNCR_TMSE_Pos                      (18U)
// #define CANFD_TSNCR_TMSE_Msk                      (0x1UL << CANFD_TSNCR_TMSE_Pos)                   /*!< 0x00040000 */
// #define CANFD_TSNCR_TMSE                          CANFD_TSNCR_TMSE_Msk                              /*!< desc TMSE */
// #define CANFD_TSNCR_TSEN_Pos                      (24U)
// #define CANFD_TSNCR_TSEN_Msk                      (0x1UL << CANFD_TSNCR_TSEN_Pos)                   /*!< 0x01000000 */
// #define CANFD_TSNCR_TSEN                          CANFD_TSNCR_TSEN_Msk                              /*!< desc TSEN */
// #define CANFD_TSNCR_TSPOS_Pos                     (25U)
// #define CANFD_TSNCR_TSPOS_Msk                     (0x1UL << CANFD_TSNCR_TSPOS_Pos)                  /*!< 0x02000000 */
// #define CANFD_TSNCR_TSPOS                         CANFD_TSNCR_TSPOS_Msk                             /*!< desc TSPOS */

/*!< CANFD_ACBTR */
#define CANFD_ACBTR_AC_SEG_1_Pos                  (0U)
#define CANFD_ACBTR_AC_SEG_1_Msk                  (0x1FFUL << CANFD_ACBTR_AC_SEG_1_Pos)             /*!< 0x000001FF */
#define CANFD_ACBTR_AC_SEG_1                      CANFD_ACBTR_AC_SEG_1_Msk                          /*!< AC_SEG_1[8:0] bits (desc AC_SEG_1) */
#define CANFD_ACBTR_AC_SEG_2_Pos                  (16U)
#define CANFD_ACBTR_AC_SEG_2_Msk                  (0x7FUL << CANFD_ACBTR_AC_SEG_2_Pos)              /*!< 0x007F0000 */
#define CANFD_ACBTR_AC_SEG_2                      CANFD_ACBTR_AC_SEG_2_Msk                          /*!< AC_SEG_2[22:16] bits (desc AC_SEG_2) */
#define CANFD_ACBTR_AC_SEG_2_0                    (0x1UL << CANFD_ACBTR_AC_SEG_2_Pos)               /*!< 0x00010000 */
#define CANFD_ACBTR_AC_SEG_2_1                    (0x2UL << CANFD_ACBTR_AC_SEG_2_Pos)               /*!< 0x00020000 */
#define CANFD_ACBTR_AC_SEG_2_2                    (0x4UL << CANFD_ACBTR_AC_SEG_2_Pos)               /*!< 0x00040000 */
#define CANFD_ACBTR_AC_SEG_2_3                    (0x8UL << CANFD_ACBTR_AC_SEG_2_Pos)               /*!< 0x00080000 */
#define CANFD_ACBTR_AC_SEG_2_4                    (0x10UL << CANFD_ACBTR_AC_SEG_2_Pos)              /*!< 0x00100000 */
#define CANFD_ACBTR_AC_SEG_2_5                    (0x20UL << CANFD_ACBTR_AC_SEG_2_Pos)              /*!< 0x00200000 */
#define CANFD_ACBTR_AC_SEG_2_6                    (0x40UL << CANFD_ACBTR_AC_SEG_2_Pos)              /*!< 0x00400000 */

#define CANFD_ACBTR_AC_SJW_Pos                    (24U)
#define CANFD_ACBTR_AC_SJW_Msk                    (0x7FUL << CANFD_ACBTR_AC_SJW_Pos)                /*!< 0x7F000000 */
#define CANFD_ACBTR_AC_SJW                        CANFD_ACBTR_AC_SJW_Msk                            /*!< AC_SJW[30:24] bits (desc AC_SJW) */
#define CANFD_ACBTR_AC_SJW_0                      (0x1UL << CANFD_ACBTR_AC_SJW_Pos)                 /*!< 0x01000000 */
#define CANFD_ACBTR_AC_SJW_1                      (0x2UL << CANFD_ACBTR_AC_SJW_Pos)                 /*!< 0x02000000 */
#define CANFD_ACBTR_AC_SJW_2                      (0x4UL << CANFD_ACBTR_AC_SJW_Pos)                 /*!< 0x04000000 */
#define CANFD_ACBTR_AC_SJW_3                      (0x8UL << CANFD_ACBTR_AC_SJW_Pos)                 /*!< 0x08000000 */
#define CANFD_ACBTR_AC_SJW_4                      (0x10UL << CANFD_ACBTR_AC_SJW_Pos)                /*!< 0x10000000 */
#define CANFD_ACBTR_AC_SJW_5                      (0x20UL << CANFD_ACBTR_AC_SJW_Pos)                /*!< 0x20000000 */
#define CANFD_ACBTR_AC_SJW_6                      (0x40UL << CANFD_ACBTR_AC_SJW_Pos)                /*!< 0x40000000 */


/*!< CANFD_FDBTR */
#define CANFD_FDBTR_FD_SEG_1_Pos                  (0U)
#define CANFD_FDBTR_FD_SEG_1_Msk                  (0xFFUL << CANFD_FDBTR_FD_SEG_1_Pos)              /*!< 0x000000FF */
#define CANFD_FDBTR_FD_SEG_1                      CANFD_FDBTR_FD_SEG_1_Msk                          /*!< FD_SEG_1[7:0] bits (desc FD_SEG_1) */
#define CANFD_FDBTR_FD_SEG_2_Pos                  (16U)
#define CANFD_FDBTR_FD_SEG_2_Msk                  (0x7FUL << CANFD_FDBTR_FD_SEG_2_Pos)              /*!< 0x007F0000 */
#define CANFD_FDBTR_FD_SEG_2                      CANFD_FDBTR_FD_SEG_2_Msk                          /*!< FD_SEG_2[22:16] bits (desc FD_SEG_2) */
#define CANFD_FDBTR_FD_SEG_2_0                    (0x1UL << CANFD_FDBTR_FD_SEG_2_Pos)               /*!< 0x00010000 */
#define CANFD_FDBTR_FD_SEG_2_1                    (0x2UL << CANFD_FDBTR_FD_SEG_2_Pos)               /*!< 0x00020000 */
#define CANFD_FDBTR_FD_SEG_2_2                    (0x4UL << CANFD_FDBTR_FD_SEG_2_Pos)               /*!< 0x00040000 */
#define CANFD_FDBTR_FD_SEG_2_3                    (0x8UL << CANFD_FDBTR_FD_SEG_2_Pos)               /*!< 0x00080000 */
#define CANFD_FDBTR_FD_SEG_2_4                    (0x10UL << CANFD_FDBTR_FD_SEG_2_Pos)              /*!< 0x00100000 */
#define CANFD_FDBTR_FD_SEG_2_5                    (0x20UL << CANFD_FDBTR_FD_SEG_2_Pos)              /*!< 0x00200000 */
#define CANFD_FDBTR_FD_SEG_2_6                    (0x40UL << CANFD_FDBTR_FD_SEG_2_Pos)              /*!< 0x00400000 */

#define CANFD_FDBTR_FD_SJW_Pos                    (24U)
#define CANFD_FDBTR_FD_SJW_Msk                    (0x7FUL << CANFD_FDBTR_FD_SJW_Pos)                /*!< 0x7F000000 */
#define CANFD_FDBTR_FD_SJW                        CANFD_FDBTR_FD_SJW_Msk                            /*!< FD_SJW[30:24] bits (desc FD_SJW) */
#define CANFD_FDBTR_FD_SJW_0                      (0x1UL << CANFD_FDBTR_FD_SJW_Pos)                 /*!< 0x01000000 */
#define CANFD_FDBTR_FD_SJW_1                      (0x2UL << CANFD_FDBTR_FD_SJW_Pos)                 /*!< 0x02000000 */
#define CANFD_FDBTR_FD_SJW_2                      (0x4UL << CANFD_FDBTR_FD_SJW_Pos)                 /*!< 0x04000000 */
#define CANFD_FDBTR_FD_SJW_3                      (0x8UL << CANFD_FDBTR_FD_SJW_Pos)                 /*!< 0x08000000 */
#define CANFD_FDBTR_FD_SJW_4                      (0x10UL << CANFD_FDBTR_FD_SJW_Pos)                /*!< 0x10000000 */
#define CANFD_FDBTR_FD_SJW_5                      (0x20UL << CANFD_FDBTR_FD_SJW_Pos)                /*!< 0x20000000 */
#define CANFD_FDBTR_FD_SJW_6                      (0x40UL << CANFD_FDBTR_FD_SJW_Pos)                /*!< 0x40000000 */


// /*!< CANFD_XLBTR */
// #define CANFD_XLBTR_XL_SEG_1_Pos                  (0U)
// #define CANFD_XLBTR_XL_SEG_1_Msk                  (0xFFUL << CANFD_XLBTR_XL_SEG_1_Pos)              /*!< 0x000000FF */
// #define CANFD_XLBTR_XL_SEG_1                      CANFD_XLBTR_XL_SEG_1_Msk                          /*!< XL_SEG_1[7:0] bits (desc XL_SEG_1) */
// #define CANFD_XLBTR_XL_SEG_2_Pos                  (16U)
// #define CANFD_XLBTR_XL_SEG_2_Msk                  (0x7FUL << CANFD_XLBTR_XL_SEG_2_Pos)              /*!< 0x007F0000 */
// #define CANFD_XLBTR_XL_SEG_2                      CANFD_XLBTR_XL_SEG_2_Msk                          /*!< XL_SEG_2[22:16] bits (desc XL_SEG_2) */
// #define CANFD_XLBTR_XL_SEG_2_0                    (0x1UL << CANFD_XLBTR_XL_SEG_2_Pos)               /*!< 0x00010000 */
// #define CANFD_XLBTR_XL_SEG_2_1                    (0x2UL << CANFD_XLBTR_XL_SEG_2_Pos)               /*!< 0x00020000 */
// #define CANFD_XLBTR_XL_SEG_2_2                    (0x4UL << CANFD_XLBTR_XL_SEG_2_Pos)               /*!< 0x00040000 */
// #define CANFD_XLBTR_XL_SEG_2_3                    (0x8UL << CANFD_XLBTR_XL_SEG_2_Pos)               /*!< 0x00080000 */
// #define CANFD_XLBTR_XL_SEG_2_4                    (0x10UL << CANFD_XLBTR_XL_SEG_2_Pos)              /*!< 0x00100000 */
// #define CANFD_XLBTR_XL_SEG_2_5                    (0x20UL << CANFD_XLBTR_XL_SEG_2_Pos)              /*!< 0x00200000 */
// #define CANFD_XLBTR_XL_SEG_2_6                    (0x40UL << CANFD_XLBTR_XL_SEG_2_Pos)              /*!< 0x00400000 */

// #define CANFD_XLBTR_XL_SJW_Pos                    (24U)
// #define CANFD_XLBTR_XL_SJW_Msk                    (0x7FUL << CANFD_XLBTR_XL_SJW_Pos)                /*!< 0x7F000000 */
// #define CANFD_XLBTR_XL_SJW                        CANFD_XLBTR_XL_SJW_Msk                            /*!< XL_SJW[30:24] bits (desc XL_SJW) */
// #define CANFD_XLBTR_XL_SJW_0                      (0x1UL << CANFD_XLBTR_XL_SJW_Pos)                 /*!< 0x01000000 */
// #define CANFD_XLBTR_XL_SJW_1                      (0x2UL << CANFD_XLBTR_XL_SJW_Pos)                 /*!< 0x02000000 */
// #define CANFD_XLBTR_XL_SJW_2                      (0x4UL << CANFD_XLBTR_XL_SJW_Pos)                 /*!< 0x04000000 */
// #define CANFD_XLBTR_XL_SJW_3                      (0x8UL << CANFD_XLBTR_XL_SJW_Pos)                 /*!< 0x08000000 */
// #define CANFD_XLBTR_XL_SJW_4                      (0x10UL << CANFD_XLBTR_XL_SJW_Pos)                /*!< 0x10000000 */
// #define CANFD_XLBTR_XL_SJW_5                      (0x20UL << CANFD_XLBTR_XL_SJW_Pos)                /*!< 0x20000000 */
// #define CANFD_XLBTR_XL_SJW_6                      (0x40UL << CANFD_XLBTR_XL_SJW_Pos)                /*!< 0x40000000 */


/*!< CANFD_RLSSP */
#define CANFD_RLSSP_PRESC_Pos                     (0U)
#define CANFD_RLSSP_PRESC_Msk                     (0x1FUL << CANFD_RLSSP_PRESC_Pos)                 /*!< 0x0000001F */
#define CANFD_RLSSP_PRESC                         CANFD_RLSSP_PRESC_Msk                             /*!< PRESC[4:0] bits (desc PRESC) */
#define CANFD_RLSSP_PRESC_0                       (0x1UL << CANFD_RLSSP_PRESC_Pos)                  /*!< 0x00000001 */
#define CANFD_RLSSP_PRESC_1                       (0x2UL << CANFD_RLSSP_PRESC_Pos)                  /*!< 0x00000002 */
#define CANFD_RLSSP_PRESC_2                       (0x4UL << CANFD_RLSSP_PRESC_Pos)                  /*!< 0x00000004 */
#define CANFD_RLSSP_PRESC_3                       (0x8UL << CANFD_RLSSP_PRESC_Pos)                  /*!< 0x00000008 */
#define CANFD_RLSSP_PRESC_4                       (0x10UL << CANFD_RLSSP_PRESC_Pos)                 /*!< 0x00000010 */

#define CANFD_RLSSP_FD_SSPOFF_Pos                 (8U)
#define CANFD_RLSSP_FD_SSPOFF_Msk                 (0xFFUL << CANFD_RLSSP_FD_SSPOFF_Pos)             /*!< 0x0000FF00 */
#define CANFD_RLSSP_FD_SSPOFF                     CANFD_RLSSP_FD_SSPOFF_Msk                         /*!< FD_SSPOFF[15:8] bits (desc FD_SSPOFF) */
// #define CANFD_RLSSP_XL_SSPOFF_Pos                 (16U)
// #define CANFD_RLSSP_XL_SSPOFF_Msk                 (0xFFUL << CANFD_RLSSP_XL_SSPOFF_Pos)             /*!< 0x00FF0000 */
// #define CANFD_RLSSP_XL_SSPOFF                     CANFD_RLSSP_XL_SSPOFF_Msk                         /*!< XL_SSPOFF[23:16] bits (desc XL_SSPOFF) */
#define CANFD_RLSSP_REALIM_Pos                    (24U)
#define CANFD_RLSSP_REALIM_Msk                    (0x7UL << CANFD_RLSSP_REALIM_Pos)                 /*!< 0x07000000 */
#define CANFD_RLSSP_REALIM                        CANFD_RLSSP_REALIM_Msk                            /*!< REALIM[26:24] bits (desc REALIM) */
#define CANFD_RLSSP_REALIM_0                      (0x1UL << CANFD_RLSSP_REALIM_Pos)                 /*!< 0x01000000 */
#define CANFD_RLSSP_REALIM_1                      (0x2UL << CANFD_RLSSP_REALIM_Pos)                 /*!< 0x02000000 */
#define CANFD_RLSSP_REALIM_2                      (0x4UL << CANFD_RLSSP_REALIM_Pos)                 /*!< 0x04000000 */

#define CANFD_RLSSP_RETLIM_Pos                    (28U)
#define CANFD_RLSSP_RETLIM_Msk                    (0x7UL << CANFD_RLSSP_RETLIM_Pos)                 /*!< 0x70000000 */
#define CANFD_RLSSP_RETLIM                        CANFD_RLSSP_RETLIM_Msk                            /*!< RETLIM[30:28] bits (desc RETLIM) */
#define CANFD_RLSSP_RETLIM_0                      (0x1UL << CANFD_RLSSP_RETLIM_Pos)                 /*!< 0x10000000 */
#define CANFD_RLSSP_RETLIM_1                      (0x2UL << CANFD_RLSSP_RETLIM_Pos)                 /*!< 0x20000000 */
#define CANFD_RLSSP_RETLIM_2                      (0x4UL << CANFD_RLSSP_RETLIM_Pos)                 /*!< 0x40000000 */


/*!< CANFD_IFR */
#define CANFD_IFR_AIF_Pos                         (0U)
#define CANFD_IFR_AIF_Msk                         (0x1UL << CANFD_IFR_AIF_Pos)                      /*!< 0x00000001 */
#define CANFD_IFR_AIF                             CANFD_IFR_AIF_Msk                                 /*!< desc AIF */
#define CANFD_IFR_EIF_Pos                         (1U)
#define CANFD_IFR_EIF_Msk                         (0x1UL << CANFD_IFR_EIF_Pos)                      /*!< 0x00000002 */
#define CANFD_IFR_EIF                             CANFD_IFR_EIF_Msk                                 /*!< desc EIF */
#define CANFD_IFR_TSIF_Pos                        (2U)
#define CANFD_IFR_TSIF_Msk                        (0x1UL << CANFD_IFR_TSIF_Pos)                     /*!< 0x00000004 */
#define CANFD_IFR_TSIF                            CANFD_IFR_TSIF_Msk                                /*!< desc TSIF */
#define CANFD_IFR_TPIF_Pos                        (3U)
#define CANFD_IFR_TPIF_Msk                        (0x1UL << CANFD_IFR_TPIF_Pos)                     /*!< 0x00000008 */
#define CANFD_IFR_TPIF                            CANFD_IFR_TPIF_Msk                                /*!< desc TPIF */
#define CANFD_IFR_RAFIF_Pos                       (4U)
#define CANFD_IFR_RAFIF_Msk                       (0x1UL << CANFD_IFR_RAFIF_Pos)                    /*!< 0x00000010 */
#define CANFD_IFR_RAFIF                           CANFD_IFR_RAFIF_Msk                               /*!< desc RAFIF */
#define CANFD_IFR_RFIF_Pos                        (5U)
#define CANFD_IFR_RFIF_Msk                        (0x1UL << CANFD_IFR_RFIF_Pos)                     /*!< 0x00000020 */
#define CANFD_IFR_RFIF                            CANFD_IFR_RFIF_Msk                                /*!< desc RFIF */
#define CANFD_IFR_ROIF_Pos                        (6U)
#define CANFD_IFR_ROIF_Msk                        (0x1UL << CANFD_IFR_ROIF_Pos)                     /*!< 0x00000040 */
#define CANFD_IFR_ROIF                            CANFD_IFR_ROIF_Msk                                /*!< desc ROIF */
#define CANFD_IFR_RIF_Pos                         (7U)
#define CANFD_IFR_RIF_Msk                         (0x1UL << CANFD_IFR_RIF_Pos)                      /*!< 0x00000080 */
#define CANFD_IFR_RIF                             CANFD_IFR_RIF_Msk                                 /*!< desc RIF */
#define CANFD_IFR_BEIF_Pos                        (8U)
#define CANFD_IFR_BEIF_Msk                        (0x1UL << CANFD_IFR_BEIF_Pos)                     /*!< 0x00000100 */
#define CANFD_IFR_BEIF                            CANFD_IFR_BEIF_Msk                                /*!< desc BEIF */
#define CANFD_IFR_ALIF_Pos                        (9U)
#define CANFD_IFR_ALIF_Msk                        (0x1UL << CANFD_IFR_ALIF_Pos)                     /*!< 0x00000200 */
#define CANFD_IFR_ALIF                            CANFD_IFR_ALIF_Msk                                /*!< desc ALIF */
#define CANFD_IFR_EPIF_Pos                        (10U)
#define CANFD_IFR_EPIF_Msk                        (0x1UL << CANFD_IFR_EPIF_Pos)                     /*!< 0x00000400 */
#define CANFD_IFR_EPIF                            CANFD_IFR_EPIF_Msk                                /*!< desc EPIF */
#define CANFD_IFR_TTIF_Pos                        (11U)
#define CANFD_IFR_TTIF_Msk                        (0x1UL << CANFD_IFR_TTIF_Pos)                     /*!< 0x00000800 */
#define CANFD_IFR_TTIF                            CANFD_IFR_TTIF_Msk                                /*!< desc TTIF */
#define CANFD_IFR_TEIF_Pos                        (12U)
#define CANFD_IFR_TEIF_Msk                        (0x1UL << CANFD_IFR_TEIF_Pos)                     /*!< 0x00001000 */
#define CANFD_IFR_TEIF                            CANFD_IFR_TEIF_Msk                                /*!< desc TEIF */
#define CANFD_IFR_WTIF_Pos                        (13U)
#define CANFD_IFR_WTIF_Msk                        (0x1UL << CANFD_IFR_WTIF_Pos)                     /*!< 0x00002000 */
#define CANFD_IFR_WTIF                            CANFD_IFR_WTIF_Msk                                /*!< desc WTIF */
// #define CANFD_IFR_MDWIF_Pos                       (14U)
// #define CANFD_IFR_MDWIF_Msk                       (0x1UL << CANFD_IFR_MDWIF_Pos)                    /*!< 0x00004000 */
// #define CANFD_IFR_MDWIF                           CANFD_IFR_MDWIF_Msk                               /*!< desc MDWIF */
// #define CANFD_IFR_MDEIF_Pos                       (15U)
// #define CANFD_IFR_MDEIF_Msk                       (0x1UL << CANFD_IFR_MDEIF_Pos)                    /*!< 0x00008000 */
// #define CANFD_IFR_MDEIF                           CANFD_IFR_MDEIF_Msk                               /*!< desc MDEIF */
// #define CANFD_IFR_MAEIF_Pos                       (16U)
// #define CANFD_IFR_MAEIF_Msk                       (0x1UL << CANFD_IFR_MAEIF_Pos)                    /*!< 0x00010000 */
// #define CANFD_IFR_MAEIF                           CANFD_IFR_MAEIF_Msk                               /*!< desc MAEIF */
// #define CANFD_IFR_SEIF_Pos                        (17U)
// #define CANFD_IFR_SEIF_Msk                        (0x1UL << CANFD_IFR_SEIF_Pos)                     /*!< 0x00020000 */
// #define CANFD_IFR_SEIF                            CANFD_IFR_SEIF_Msk                                /*!< desc SEIF */
// #define CANFD_IFR_SWIF_Pos                        (18U)
// #define CANFD_IFR_SWIF_Msk                        (0x1UL << CANFD_IFR_SWIF_Pos)                     /*!< 0x00040000 */
// #define CANFD_IFR_SWIF                            CANFD_IFR_SWIF_Msk                                /*!< desc SWIF */
#define CANFD_IFR_EPASS_Pos                       (30U)
#define CANFD_IFR_EPASS_Msk                       (0x1UL << CANFD_IFR_EPASS_Pos)                    /*!< 0x40000000 */
#define CANFD_IFR_EPASS                           CANFD_IFR_EPASS_Msk                               /*!< desc EPASS */
#define CANFD_IFR_EWARN_Pos                       (31U)
#define CANFD_IFR_EWARN_Msk                       (0x1UL << CANFD_IFR_EWARN_Pos)                    /*!< 0x80000000 */
#define CANFD_IFR_EWARN                           CANFD_IFR_EWARN_Msk                               /*!< desc EWARN */

/*!< CANFD_IER */
#define CANFD_IER_EIE_Pos                         (1U)
#define CANFD_IER_EIE_Msk                         (0x1UL << CANFD_IER_EIE_Pos)                      /*!< 0x00000002 */
#define CANFD_IER_EIE                             CANFD_IER_EIE_Msk                                 /*!< desc EIE */
#define CANFD_IER_TSIE_Pos                        (2U)
#define CANFD_IER_TSIE_Msk                        (0x1UL << CANFD_IER_TSIE_Pos)                     /*!< 0x00000004 */
#define CANFD_IER_TSIE                            CANFD_IER_TSIE_Msk                                /*!< desc TSIE */
#define CANFD_IER_TPIE_Pos                        (3U)
#define CANFD_IER_TPIE_Msk                        (0x1UL << CANFD_IER_TPIE_Pos)                     /*!< 0x00000008 */
#define CANFD_IER_TPIE                            CANFD_IER_TPIE_Msk                                /*!< desc TPIE */
#define CANFD_IER_RAFIE_Pos                       (4U)
#define CANFD_IER_RAFIE_Msk                       (0x1UL << CANFD_IER_RAFIE_Pos)                    /*!< 0x00000010 */
#define CANFD_IER_RAFIE                           CANFD_IER_RAFIE_Msk                               /*!< desc RAFIE */
#define CANFD_IER_RFIE_Pos                        (5U)
#define CANFD_IER_RFIE_Msk                        (0x1UL << CANFD_IER_RFIE_Pos)                     /*!< 0x00000020 */
#define CANFD_IER_RFIE                            CANFD_IER_RFIE_Msk                                /*!< desc RFIE */
#define CANFD_IER_ROIE_Pos                        (6U)
#define CANFD_IER_ROIE_Msk                        (0x1UL << CANFD_IER_ROIE_Pos)                     /*!< 0x00000040 */
#define CANFD_IER_ROIE                            CANFD_IER_ROIE_Msk                                /*!< desc ROIE */
#define CANFD_IER_RIE_Pos                         (7U)
#define CANFD_IER_RIE_Msk                         (0x1UL << CANFD_IER_RIE_Pos)                      /*!< 0x00000080 */
#define CANFD_IER_RIE                             CANFD_IER_RIE_Msk                                 /*!< desc RIE */
#define CANFD_IER_BEIE_Pos                        (8U)
#define CANFD_IER_BEIE_Msk                        (0x1UL << CANFD_IER_BEIE_Pos)                     /*!< 0x00000100 */
#define CANFD_IER_BEIE                            CANFD_IER_BEIE_Msk                                /*!< desc BEIE */
#define CANFD_IER_ALIE_Pos                        (9U)
#define CANFD_IER_ALIE_Msk                        (0x1UL << CANFD_IER_ALIE_Pos)                     /*!< 0x00000200 */
#define CANFD_IER_ALIE                            CANFD_IER_ALIE_Msk                                /*!< desc ALIE */
#define CANFD_IER_EPIE_Pos                        (10U)
#define CANFD_IER_EPIE_Msk                        (0x1UL << CANFD_IER_EPIE_Pos)                     /*!< 0x00000400 */
#define CANFD_IER_EPIE                            CANFD_IER_EPIE_Msk                                /*!< desc EPIE */
#define CANFD_IER_TTIE_Pos                        (11U)
#define CANFD_IER_TTIE_Msk                        (0x1UL << CANFD_IER_TTIE_Pos)                     /*!< 0x00000800 */
#define CANFD_IER_TTIE                            CANFD_IER_TTIE_Msk                                /*!< desc TTIE */
#define CANFD_IER_WTIE_Pos                        (13U)
#define CANFD_IER_WTIE_Msk                        (0x1UL << CANFD_IER_WTIE_Pos)                     /*!< 0x00002000 */
#define CANFD_IER_WTIE                            CANFD_IER_WTIE_Msk                                /*!< desc WTIE */
// #define CANFD_IER_MDWIE_Pos                       (14U)
// #define CANFD_IER_MDWIE_Msk                       (0x1UL << CANFD_IER_MDWIE_Pos)                    /*!< 0x00004000 */
// #define CANFD_IER_MDWIE                           CANFD_IER_MDWIE_Msk                               /*!< desc MDWIE */
// #define CANFD_IER_SWIE_Pos                        (18U)
// #define CANFD_IER_SWIE_Msk                        (0x1UL << CANFD_IER_SWIE_Pos)                     /*!< 0x00040000 */
// #define CANFD_IER_SWIE                            CANFD_IER_SWIE_Msk                                /*!< desc SWIE */

/*!< CANFD_TSR */
#define CANFD_TSR_HANDLE_L_Pos                    (0U)
#define CANFD_TSR_HANDLE_L_Msk                    (0xFFUL << CANFD_TSR_HANDLE_L_Pos)                /*!< 0x000000FF */
#define CANFD_TSR_HANDLE_L                        CANFD_TSR_HANDLE_L_Msk                            /*!< HANDLE_L[7:0] bits (desc HANDLE_L) */
#define CANFD_TSR_TSTAT_L_Pos                     (8U)
#define CANFD_TSR_TSTAT_L_Msk                     (0x7UL << CANFD_TSR_TSTAT_L_Pos)                  /*!< 0x00000700 */
#define CANFD_TSR_TSTAT_L                         CANFD_TSR_TSTAT_L_Msk                             /*!< TSTAT_L[10:8] bits (desc TSTAT_L) */
#define CANFD_TSR_TSTAT_L_0                       (0x1UL << CANFD_TSR_TSTAT_L_Pos)                  /*!< 0x00000100 */
#define CANFD_TSR_TSTAT_L_1                       (0x2UL << CANFD_TSR_TSTAT_L_Pos)                  /*!< 0x00000200 */
#define CANFD_TSR_TSTAT_L_2                       (0x4UL << CANFD_TSR_TSTAT_L_Pos)                  /*!< 0x00000400 */

#define CANFD_TSR_HANDLE_H_Pos                    (16U)
#define CANFD_TSR_HANDLE_H_Msk                    (0xFFUL << CANFD_TSR_HANDLE_H_Pos)                /*!< 0x00FF0000 */
#define CANFD_TSR_HANDLE_H                        CANFD_TSR_HANDLE_H_Msk                            /*!< HANDLE_H[23:16] bits (desc HANDLE_H) */
#define CANFD_TSR_TSTAT_H_Pos                     (24U)
#define CANFD_TSR_TSTAT_H_Msk                     (0x7UL << CANFD_TSR_TSTAT_H_Pos)                  /*!< 0x07000000 */
#define CANFD_TSR_TSTAT_H                         CANFD_TSR_TSTAT_H_Msk                             /*!< TSTAT_H[26:24] bits (desc TSTAT_H) */
#define CANFD_TSR_TSTAT_H_0                       (0x1UL << CANFD_TSR_TSTAT_H_Pos)                  /*!< 0x01000000 */
#define CANFD_TSR_TSTAT_H_1                       (0x2UL << CANFD_TSR_TSTAT_H_Pos)                  /*!< 0x02000000 */
#define CANFD_TSR_TSTAT_H_2                       (0x4UL << CANFD_TSR_TSTAT_H_Pos)                  /*!< 0x04000000 */


// /*!< CANFD_TTSL */
// #define CANFD_TTSL_TTS_Pos                        (0U)
// #define CANFD_TTSL_TTS_Msk                        (0xFFFFFFFFUL << CANFD_TTSL_TTS_Pos)              /*!< 0xFFFFFFFF */
// #define CANFD_TTSL_TTS                            CANFD_TTSL_TTS_Msk                                /*!< TTS[31:0] bits (desc TTS) */

// /*!< CANFD_TTSH */
// #define CANFD_TTSH_TTS_Pos                        (0U)
// #define CANFD_TTSH_TTS_Msk                        (0xFFFFFFFFUL << CANFD_TTSH_TTS_Pos)              /*!< 0xFFFFFFFF */
// #define CANFD_TTSH_TTS                            CANFD_TTSH_TTS_Msk                                /*!< TTS[31:0] bits (desc TTS) */

/*!< CANFD_MCR */
#define CANFD_MCR_BUSOFF_Pos                      (0U)
#define CANFD_MCR_BUSOFF_Msk                      (0x1UL << CANFD_MCR_BUSOFF_Pos)                   /*!< 0x00000001 */
#define CANFD_MCR_BUSOFF                          CANFD_MCR_BUSOFF_Msk                              /*!< desc BUSOFF */
#define CANFD_MCR_LBMI_Pos                        (5U)
#define CANFD_MCR_LBMI_Msk                        (0x1UL << CANFD_MCR_LBMI_Pos)                     /*!< 0x00000020 */
#define CANFD_MCR_LBMI                            CANFD_MCR_LBMI_Msk                                /*!< desc LBMI */
#define CANFD_MCR_LBME_Pos                        (6U)
#define CANFD_MCR_LBME_Msk                        (0x1UL << CANFD_MCR_LBME_Pos)                     /*!< 0x00000040 */
#define CANFD_MCR_LBME                            CANFD_MCR_LBME_Msk                                /*!< desc LBME */
#define CANFD_MCR_RESET_Pos                       (7U)
#define CANFD_MCR_RESET_Msk                       (0x1UL << CANFD_MCR_RESET_Pos)                    /*!< 0x00000080 */
#define CANFD_MCR_RESET                           CANFD_MCR_RESET_Msk                               /*!< desc RESET */
#define CANFD_MCR_TSA_Pos                         (8U)
#define CANFD_MCR_TSA_Msk                         (0x1UL << CANFD_MCR_TSA_Pos)                      /*!< 0x00000100 */
#define CANFD_MCR_TSA                             CANFD_MCR_TSA_Msk                                 /*!< desc TSA */
#define CANFD_MCR_TSALL_Pos                       (9U)
#define CANFD_MCR_TSALL_Msk                       (0x1UL << CANFD_MCR_TSALL_Pos)                    /*!< 0x00000200 */
#define CANFD_MCR_TSALL                           CANFD_MCR_TSALL_Msk                               /*!< desc TSALL */
#define CANFD_MCR_TSONE_Pos                       (10U)
#define CANFD_MCR_TSONE_Msk                       (0x1UL << CANFD_MCR_TSONE_Pos)                    /*!< 0x00000400 */
#define CANFD_MCR_TSONE                           CANFD_MCR_TSONE_Msk                               /*!< desc TSONE */
#define CANFD_MCR_TPA_Pos                         (11U)
#define CANFD_MCR_TPA_Msk                         (0x1UL << CANFD_MCR_TPA_Pos)                      /*!< 0x00000800 */
#define CANFD_MCR_TPA                             CANFD_MCR_TPA_Msk                                 /*!< desc TPA */
#define CANFD_MCR_TPE_Pos                         (12U)
#define CANFD_MCR_TPE_Msk                         (0x1UL << CANFD_MCR_TPE_Pos)                      /*!< 0x00001000 */
#define CANFD_MCR_TPE                             CANFD_MCR_TPE_Msk                                 /*!< desc TPE */
#define CANFD_MCR_STBY_Pos                        (13U)
#define CANFD_MCR_STBY_Msk                        (0x1UL << CANFD_MCR_STBY_Pos)                     /*!< 0x00002000 */
#define CANFD_MCR_STBY                            CANFD_MCR_STBY_Msk                                /*!< desc STBY */
#define CANFD_MCR_LOM_Pos                         (14U)
#define CANFD_MCR_LOM_Msk                         (0x1UL << CANFD_MCR_LOM_Pos)                      /*!< 0x00004000 */
#define CANFD_MCR_LOM                             CANFD_MCR_LOM_Msk                                 /*!< desc LOM */
#define CANFD_MCR_TBSEL_Pos                       (15U)
#define CANFD_MCR_TBSEL_Msk                       (0x1UL << CANFD_MCR_TBSEL_Pos)                    /*!< 0x00008000 */
#define CANFD_MCR_TBSEL                           CANFD_MCR_TBSEL_Msk                               /*!< desc TBSEL */
#define CANFD_MCR_TSSTAT_Pos                      (16U)
#define CANFD_MCR_TSSTAT_Msk                      (0x3UL << CANFD_MCR_TSSTAT_Pos)                   /*!< 0x00030000 */
#define CANFD_MCR_TSSTAT                          CANFD_MCR_TSSTAT_Msk                              /*!< TSSTAT[17:16] bits (desc TSSTAT) */
#define CANFD_MCR_TSSTAT_0                        (0x1UL << CANFD_MCR_TSSTAT_Pos)                   /*!< 0x00010000 */
#define CANFD_MCR_TSSTAT_1                        (0x2UL << CANFD_MCR_TSSTAT_Pos)                   /*!< 0x00020000 */

#define CANFD_MCR_TSFF_Pos                        (18U)
#define CANFD_MCR_TSFF_Msk                        (0x1UL << CANFD_MCR_TSFF_Pos)                     /*!< 0x00040000 */
#define CANFD_MCR_TSFF                            CANFD_MCR_TSFF_Msk                                /*!< desc TSFF */
#define CANFD_MCR_TTTBM_Pos                       (20U)
#define CANFD_MCR_TTTBM_Msk                       (0x1UL << CANFD_MCR_TTTBM_Pos)                    /*!< 0x00100000 */
#define CANFD_MCR_TTTBM                           CANFD_MCR_TTTBM_Msk                               /*!< desc TTTBM */
#define CANFD_MCR_TSMODE_Pos                      (21U)
#define CANFD_MCR_TSMODE_Msk                      (0x1UL << CANFD_MCR_TSMODE_Pos)                   /*!< 0x00200000 */
#define CANFD_MCR_TSMODE                          CANFD_MCR_TSMODE_Msk                              /*!< desc TSMODE */
#define CANFD_MCR_TSNEXT_Pos                      (22U)
#define CANFD_MCR_TSNEXT_Msk                      (0x1UL << CANFD_MCR_TSNEXT_Pos)                   /*!< 0x00400000 */
#define CANFD_MCR_TSNEXT                          CANFD_MCR_TSNEXT_Msk                              /*!< desc TSNEXT */
#define CANFD_MCR_FD_ISO_Pos                      (23U)
#define CANFD_MCR_FD_ISO_Msk                      (0x1UL << CANFD_MCR_FD_ISO_Pos)                   /*!< 0x00800000 */
#define CANFD_MCR_FD_ISO                          CANFD_MCR_FD_ISO_Msk                              /*!< desc FD_ISO */
#define CANFD_MCR_RSTAT_Pos                       (24U)
#define CANFD_MCR_RSTAT_Msk                       (0x3UL << CANFD_MCR_RSTAT_Pos)                    /*!< 0x03000000 */
#define CANFD_MCR_RSTAT                           CANFD_MCR_RSTAT_Msk                               /*!< RSTAT[25:24] bits (desc RSTAT) */
#define CANFD_MCR_RSTAT_0                         (0x1UL << CANFD_MCR_RSTAT_Pos)                    /*!< 0x01000000 */
#define CANFD_MCR_RSTAT_1                         (0x2UL << CANFD_MCR_RSTAT_Pos)                    /*!< 0x02000000 */

#define CANFD_MCR_RBALL_Pos                       (27U)
#define CANFD_MCR_RBALL_Msk                       (0x1UL << CANFD_MCR_RBALL_Pos)                    /*!< 0x08000000 */
#define CANFD_MCR_RBALL                           CANFD_MCR_RBALL_Msk                               /*!< desc RBALL */
#define CANFD_MCR_RREL_Pos                        (28U)
#define CANFD_MCR_RREL_Msk                        (0x1UL << CANFD_MCR_RREL_Pos)                     /*!< 0x10000000 */
#define CANFD_MCR_RREL                            CANFD_MCR_RREL_Msk                                /*!< desc RREL */
#define CANFD_MCR_ROV_Pos                         (29U)
#define CANFD_MCR_ROV_Msk                         (0x1UL << CANFD_MCR_ROV_Pos)                      /*!< 0x20000000 */
#define CANFD_MCR_ROV                             CANFD_MCR_ROV_Msk                                 /*!< desc ROV */
#define CANFD_MCR_ROM_Pos                         (30U)
#define CANFD_MCR_ROM_Msk                         (0x1UL << CANFD_MCR_ROM_Pos)                      /*!< 0x40000000 */
#define CANFD_MCR_ROM                             CANFD_MCR_ROM_Msk                                 /*!< desc ROM */
#define CANFD_MCR_SACK_Pos                        (31U)
#define CANFD_MCR_SACK_Msk                        (0x1UL << CANFD_MCR_SACK_Pos)                     /*!< 0x80000000 */
#define CANFD_MCR_SACK                            CANFD_MCR_SACK_Msk                                /*!< desc SACK */

/*!< CANFD_WECR */
#define CANFD_WECR_EWL_Pos                        (0U)
#define CANFD_WECR_EWL_Msk                        (0xFUL << CANFD_WECR_EWL_Pos)                     /*!< 0x0000000F */
#define CANFD_WECR_EWL                            CANFD_WECR_EWL_Msk                                /*!< EWL[3:0] bits (desc EWL) */
#define CANFD_WECR_EWL_0                          (0x1UL << CANFD_WECR_EWL_Pos)                     /*!< 0x00000001 */
#define CANFD_WECR_EWL_1                          (0x2UL << CANFD_WECR_EWL_Pos)                     /*!< 0x00000002 */
#define CANFD_WECR_EWL_2                          (0x4UL << CANFD_WECR_EWL_Pos)                     /*!< 0x00000004 */
#define CANFD_WECR_EWL_3                          (0x8UL << CANFD_WECR_EWL_Pos)                     /*!< 0x00000008 */

#define CANFD_WECR_AFWL_Pos                       (4U)
#define CANFD_WECR_AFWL_Msk                       (0xFUL << CANFD_WECR_AFWL_Pos)                    /*!< 0x000000F0 */
#define CANFD_WECR_AFWL                           CANFD_WECR_AFWL_Msk                               /*!< AFWL[7:4] bits (desc AFWL) */
#define CANFD_WECR_AFWL_0                         (0x1UL << CANFD_WECR_AFWL_Pos)                    /*!< 0x00000010 */
#define CANFD_WECR_AFWL_1                         (0x2UL << CANFD_WECR_AFWL_Pos)                    /*!< 0x00000020 */
#define CANFD_WECR_AFWL_2                         (0x4UL << CANFD_WECR_AFWL_Pos)                    /*!< 0x00000040 */
#define CANFD_WECR_AFWL_3                         (0x8UL << CANFD_WECR_AFWL_Pos)                    /*!< 0x00000080 */

#define CANFD_WECR_ALC_Pos                        (8U)
#define CANFD_WECR_ALC_Msk                        (0x1FUL << CANFD_WECR_ALC_Pos)                    /*!< 0x00001F00 */
#define CANFD_WECR_ALC                            CANFD_WECR_ALC_Msk                                /*!< ALC[12:8] bits (desc ALC) */
#define CANFD_WECR_ALC_0                          (0x1UL << CANFD_WECR_ALC_Pos)                     /*!< 0x00000100 */
#define CANFD_WECR_ALC_1                          (0x2UL << CANFD_WECR_ALC_Pos)                     /*!< 0x00000200 */
#define CANFD_WECR_ALC_2                          (0x4UL << CANFD_WECR_ALC_Pos)                     /*!< 0x00000400 */
#define CANFD_WECR_ALC_3                          (0x8UL << CANFD_WECR_ALC_Pos)                     /*!< 0x00000800 */
#define CANFD_WECR_ALC_4                          (0x10UL << CANFD_WECR_ALC_Pos)                    /*!< 0x00001000 */

#define CANFD_WECR_KOER_Pos                       (13U)
#define CANFD_WECR_KOER_Msk                       (0x7UL << CANFD_WECR_KOER_Pos)                    /*!< 0x0000E000 */
#define CANFD_WECR_KOER                           CANFD_WECR_KOER_Msk                               /*!< KOER[15:13] bits (desc KOER) */
#define CANFD_WECR_KOER_0                         (0x1UL << CANFD_WECR_KOER_Pos)                    /*!< 0x00002000 */
#define CANFD_WECR_KOER_1                         (0x2UL << CANFD_WECR_KOER_Pos)                    /*!< 0x00004000 */
#define CANFD_WECR_KOER_2                         (0x4UL << CANFD_WECR_KOER_Pos)                    /*!< 0x00008000 */

#define CANFD_WECR_RECNT_Pos                      (16U)
#define CANFD_WECR_RECNT_Msk                      (0xFFUL << CANFD_WECR_RECNT_Pos)                  /*!< 0x00FF0000 */
#define CANFD_WECR_RECNT                          CANFD_WECR_RECNT_Msk                              /*!< RECNT[23:16] bits (desc RECNT) */
#define CANFD_WECR_TECNT_Pos                      (24U)
#define CANFD_WECR_TECNT_Msk                      (0xFFUL << CANFD_WECR_TECNT_Pos)                  /*!< 0xFF000000 */
#define CANFD_WECR_TECNT                          CANFD_WECR_TECNT_Msk                              /*!< TECNT[31:24] bits (desc TECNT) */

/*!< CANFD_REFMSG */
#define CANFD_REFMSG_REF_ID_Pos                   (0U)
#define CANFD_REFMSG_REF_ID_Msk                   (0x1FFFFFFFUL << CANFD_REFMSG_REF_ID_Pos)         /*!< 0x1FFFFFFF */
#define CANFD_REFMSG_REF_ID                       CANFD_REFMSG_REF_ID_Msk                           /*!< REF_ID[28:0] bits (desc REF_ID) */
#define CANFD_REFMSG_REF_IDE_Pos                  (31U)
#define CANFD_REFMSG_REF_IDE_Msk                  (0x1UL << CANFD_REFMSG_REF_IDE_Pos)               /*!< 0x80000000 */
#define CANFD_REFMSG_REF_IDE                      CANFD_REFMSG_REF_IDE_Msk                          /*!< desc REF_IDE */

/*!< CANFD_TTCR */
#define CANFD_TTCR_TTPTR_Pos                      (0U)
#define CANFD_TTCR_TTPTR_Msk                      (0x3FUL << CANFD_TTCR_TTPTR_Pos)                  /*!< 0x0000003F */
#define CANFD_TTCR_TTPTR                          CANFD_TTCR_TTPTR_Msk                              /*!< TTPTR[5:0] bits (desc TTPTR) */
#define CANFD_TTCR_TTPTR_0                        (0x1UL << CANFD_TTCR_TTPTR_Pos)                   /*!< 0x00000001 */
#define CANFD_TTCR_TTPTR_1                        (0x2UL << CANFD_TTCR_TTPTR_Pos)                   /*!< 0x00000002 */
#define CANFD_TTCR_TTPTR_2                        (0x4UL << CANFD_TTCR_TTPTR_Pos)                   /*!< 0x00000004 */
#define CANFD_TTCR_TTPTR_3                        (0x8UL << CANFD_TTCR_TTPTR_Pos)                   /*!< 0x00000008 */
#define CANFD_TTCR_TTPTR_4                        (0x10UL << CANFD_TTCR_TTPTR_Pos)                  /*!< 0x00000010 */
#define CANFD_TTCR_TTPTR_5                        (0x20UL << CANFD_TTCR_TTPTR_Pos)                  /*!< 0x00000020 */

#define CANFD_TTCR_TTYPE_Pos                      (8U)
#define CANFD_TTCR_TTYPE_Msk                      (0x7UL << CANFD_TTCR_TTYPE_Pos)                   /*!< 0x00000700 */
#define CANFD_TTCR_TTYPE                          CANFD_TTCR_TTYPE_Msk                              /*!< TTYPE[10:8] bits (desc TTYPE) */
#define CANFD_TTCR_TTYPE_0                        (0x1UL << CANFD_TTCR_TTYPE_Pos)                   /*!< 0x00000100 */
#define CANFD_TTCR_TTYPE_1                        (0x2UL << CANFD_TTCR_TTYPE_Pos)                   /*!< 0x00000200 */
#define CANFD_TTCR_TTYPE_2                        (0x4UL << CANFD_TTCR_TTYPE_Pos)                   /*!< 0x00000400 */

#define CANFD_TTCR_TEW_Pos                        (12U)
#define CANFD_TTCR_TEW_Msk                        (0xFUL << CANFD_TTCR_TEW_Pos)                     /*!< 0x0000F000 */
#define CANFD_TTCR_TEW                            CANFD_TTCR_TEW_Msk                                /*!< TEW[15:12] bits (desc TEW) */
#define CANFD_TTCR_TEW_0                          (0x1UL << CANFD_TTCR_TEW_Pos)                     /*!< 0x00001000 */
#define CANFD_TTCR_TEW_1                          (0x2UL << CANFD_TTCR_TEW_Pos)                     /*!< 0x00002000 */
#define CANFD_TTCR_TEW_2                          (0x4UL << CANFD_TTCR_TEW_Pos)                     /*!< 0x00004000 */
#define CANFD_TTCR_TEW_3                          (0x8UL << CANFD_TTCR_TEW_Pos)                     /*!< 0x00008000 */

#define CANFD_TTCR_TBPTR_Pos                      (16U)
#define CANFD_TTCR_TBPTR_Msk                      (0x3FUL << CANFD_TTCR_TBPTR_Pos)                  /*!< 0x003F0000 */
#define CANFD_TTCR_TBPTR                          CANFD_TTCR_TBPTR_Msk                              /*!< TBPTR[21:16] bits (desc TBPTR) */
#define CANFD_TTCR_TBPTR_0                        (0x1UL << CANFD_TTCR_TBPTR_Pos)                   /*!< 0x00010000 */
#define CANFD_TTCR_TBPTR_1                        (0x2UL << CANFD_TTCR_TBPTR_Pos)                   /*!< 0x00020000 */
#define CANFD_TTCR_TBPTR_2                        (0x4UL << CANFD_TTCR_TBPTR_Pos)                   /*!< 0x00040000 */
#define CANFD_TTCR_TBPTR_3                        (0x8UL << CANFD_TTCR_TBPTR_Pos)                   /*!< 0x00080000 */
#define CANFD_TTCR_TBPTR_4                        (0x10UL << CANFD_TTCR_TBPTR_Pos)                  /*!< 0x00100000 */
#define CANFD_TTCR_TBPTR_5                        (0x20UL << CANFD_TTCR_TBPTR_Pos)                  /*!< 0x00200000 */

#define CANFD_TTCR_TBF_Pos                        (22U)
#define CANFD_TTCR_TBF_Msk                        (0x1UL << CANFD_TTCR_TBF_Pos)                     /*!< 0x00400000 */
#define CANFD_TTCR_TBF                            CANFD_TTCR_TBF_Msk                                /*!< desc TBF */
#define CANFD_TTCR_TBE_Pos                        (23U)
#define CANFD_TTCR_TBE_Msk                        (0x1UL << CANFD_TTCR_TBE_Pos)                     /*!< 0x00800000 */
#define CANFD_TTCR_TBE                            CANFD_TTCR_TBE_Msk                                /*!< desc TBE */
#define CANFD_TTCR_TTEN_Pos                       (24U)
#define CANFD_TTCR_TTEN_Msk                       (0x1UL << CANFD_TTCR_TTEN_Pos)                    /*!< 0x01000000 */
#define CANFD_TTCR_TTEN                           CANFD_TTCR_TTEN_Msk                               /*!< desc TTEN */
#define CANFD_TTCR_T_PRESC_Pos                    (25U)
#define CANFD_TTCR_T_PRESC_Msk                    (0x3UL << CANFD_TTCR_T_PRESC_Pos)                 /*!< 0x06000000 */
#define CANFD_TTCR_T_PRESC                        CANFD_TTCR_T_PRESC_Msk                            /*!< T_PRESC[26:25] bits (desc T_PRESC) */
#define CANFD_TTCR_T_PRESC_0                      (0x1UL << CANFD_TTCR_T_PRESC_Pos)                 /*!< 0x02000000 */
#define CANFD_TTCR_T_PRESC_1                      (0x2UL << CANFD_TTCR_T_PRESC_Pos)                 /*!< 0x04000000 */


/*!< CANFD_TTTR */
#define CANFD_TTTR_TT_TRIG_Pos                    (0U)
#define CANFD_TTTR_TT_TRIG_Msk                    (0xFFFFUL << CANFD_TTTR_TT_TRIG_Pos)              /*!< 0x0000FFFF */
#define CANFD_TTTR_TT_TRIG                        CANFD_TTTR_TT_TRIG_Msk                            /*!< TT_TRIG[15:0] bits (desc TT_TRIG) */
#define CANFD_TTTR_TT_WTRIG_Pos                   (16U)
#define CANFD_TTTR_TT_WTRIG_Msk                   (0xFFFFUL << CANFD_TTTR_TT_WTRIG_Pos)             /*!< 0xFFFF0000 */
#define CANFD_TTTR_TT_WTRIG                       CANFD_TTTR_TT_WTRIG_Msk                           /*!< TT_WTRIG[31:16] bits (desc TT_WTRIG) */

/*!< CANFD_SCMS */
// #define CANFD_SCMS_XMREN_Pos                      (0U)
// #define CANFD_SCMS_XMREN_Msk                      (0x1UL << CANFD_SCMS_XMREN_Pos)                   /*!< 0x00000001 */
// #define CANFD_SCMS_XMREN                          CANFD_SCMS_XMREN_Msk                              /*!< desc XMREN */

#define CANFD_SCMS_FSTIM_Pos                      (1U)
#define CANFD_SCMS_FSTIM_Msk                      (0x7UL << CANFD_SCMS_FSTIM_Pos)                   /*!< 0x0000000E */
#define CANFD_SCMS_FSTIM                          CANFD_SCMS_FSTIM_Msk                              /*!< FSTIM[3:1] bits (desc FSTIM) */
#define CANFD_SCMS_FSTIM_0                        (0x1UL << CANFD_SCMS_FSTIM_Pos)                   /*!< 0x00000002 */
#define CANFD_SCMS_FSTIM_1                        (0x2UL << CANFD_SCMS_FSTIM_Pos)                   /*!< 0x00000004 */
#define CANFD_SCMS_FSTIM_2                        (0x4UL << CANFD_SCMS_FSTIM_Pos)                   /*!< 0x00000008 */

#define CANFD_SCMS_ACFA_Pos                       (24U)
#define CANFD_SCMS_ACFA_Msk                       (0x1UL << CANFD_SCMS_ACFA_Pos)                    /*!< 0x01000000 */
#define CANFD_SCMS_ACFA                           CANFD_SCMS_ACFA_Msk                               /*!< desc ACFA */
#define CANFD_SCMS_TXS_Pos                        (25U)
#define CANFD_SCMS_TXS_Msk                        (0x1UL << CANFD_SCMS_TXS_Pos)                     /*!< 0x02000000 */
#define CANFD_SCMS_TXS                            CANFD_SCMS_TXS_Msk                                /*!< desc TXS */
#define CANFD_SCMS_TXB_Pos                        (26U)
#define CANFD_SCMS_TXB_Msk                        (0x1UL << CANFD_SCMS_TXB_Pos)                     /*!< 0x04000000 */
#define CANFD_SCMS_TXB                            CANFD_SCMS_TXB_Msk                                /*!< desc TXB */
#define CANFD_SCMS_HELOC_Pos                      (27U)
#define CANFD_SCMS_HELOC_Msk                      (0x3UL << CANFD_SCMS_HELOC_Pos)                   /*!< 0x18000000 */
#define CANFD_SCMS_HELOC                          CANFD_SCMS_HELOC_Msk                              /*!< HELOC[28:27] bits (desc HELOC) */
#define CANFD_SCMS_HELOC_0                        (0x1UL << CANFD_SCMS_HELOC_Pos)                   /*!< 0x08000000 */
#define CANFD_SCMS_HELOC_1                        (0x2UL << CANFD_SCMS_HELOC_Pos)                   /*!< 0x10000000 */

// #define CANFD_SCMS_MPEN_Pos                       (31U)
// #define CANFD_SCMS_MPEN_Msk                       (0x1UL << CANFD_SCMS_MPEN_Pos)                    /*!< 0x80000000 */
// #define CANFD_SCMS_MPEN                           CANFD_SCMS_MPEN_Msk                               /*!< desc MPEN */

// /*!< CANFD_MESR */
// #define CANFD_MESR_MEBP1_Pos                      (0U)
// #define CANFD_MESR_MEBP1_Msk                      (0x3FUL << CANFD_MESR_MEBP1_Pos)                  /*!< 0x0000003F */
// #define CANFD_MESR_MEBP1                          CANFD_MESR_MEBP1_Msk                              /*!< MEBP1[5:0] bits (desc MEBP1) */
// #define CANFD_MESR_MEBP1_0                        (0x1UL << CANFD_MESR_MEBP1_Pos)                   /*!< 0x00000001 */
// #define CANFD_MESR_MEBP1_1                        (0x2UL << CANFD_MESR_MEBP1_Pos)                   /*!< 0x00000002 */
// #define CANFD_MESR_MEBP1_2                        (0x4UL << CANFD_MESR_MEBP1_Pos)                   /*!< 0x00000004 */
// #define CANFD_MESR_MEBP1_3                        (0x8UL << CANFD_MESR_MEBP1_Pos)                   /*!< 0x00000008 */
// #define CANFD_MESR_MEBP1_4                        (0x10UL << CANFD_MESR_MEBP1_Pos)                  /*!< 0x00000010 */
// #define CANFD_MESR_MEBP1_5                        (0x20UL << CANFD_MESR_MEBP1_Pos)                  /*!< 0x00000020 */

// #define CANFD_MESR_ME1EE_Pos                      (6U)
// #define CANFD_MESR_ME1EE_Msk                      (0x1UL << CANFD_MESR_ME1EE_Pos)                   /*!< 0x00000040 */
// #define CANFD_MESR_ME1EE                          CANFD_MESR_ME1EE_Msk                              /*!< desc ME1EE */
// #define CANFD_MESR_MEAEE_Pos                      (7U)
// #define CANFD_MESR_MEAEE_Msk                      (0x1UL << CANFD_MESR_MEAEE_Pos)                   /*!< 0x00000080 */
// #define CANFD_MESR_MEAEE                          CANFD_MESR_MEAEE_Msk                              /*!< desc MEAEE */
// #define CANFD_MESR_MEBP2_Pos                      (8U)
// #define CANFD_MESR_MEBP2_Msk                      (0x3FUL << CANFD_MESR_MEBP2_Pos)                  /*!< 0x00003F00 */
// #define CANFD_MESR_MEBP2                          CANFD_MESR_MEBP2_Msk                              /*!< MEBP2[13:8] bits (desc MEBP2) */
// #define CANFD_MESR_MEBP2_0                        (0x1UL << CANFD_MESR_MEBP2_Pos)                   /*!< 0x00000100 */
// #define CANFD_MESR_MEBP2_1                        (0x2UL << CANFD_MESR_MEBP2_Pos)                   /*!< 0x00000200 */
// #define CANFD_MESR_MEBP2_2                        (0x4UL << CANFD_MESR_MEBP2_Pos)                   /*!< 0x00000400 */
// #define CANFD_MESR_MEBP2_3                        (0x8UL << CANFD_MESR_MEBP2_Pos)                   /*!< 0x00000800 */
// #define CANFD_MESR_MEBP2_4                        (0x10UL << CANFD_MESR_MEBP2_Pos)                  /*!< 0x00001000 */
// #define CANFD_MESR_MEBP2_5                        (0x20UL << CANFD_MESR_MEBP2_Pos)                  /*!< 0x00002000 */

// #define CANFD_MESR_ME2EE_Pos                      (14U)
// #define CANFD_MESR_ME2EE_Msk                      (0x1UL << CANFD_MESR_ME2EE_Pos)                   /*!< 0x00004000 */
// #define CANFD_MESR_ME2EE                          CANFD_MESR_ME2EE_Msk                              /*!< desc ME2EE */
// #define CANFD_MESR_MEEEC_Pos                      (16U)
// #define CANFD_MESR_MEEEC_Msk                      (0xFUL << CANFD_MESR_MEEEC_Pos)                   /*!< 0x000F0000 */
// #define CANFD_MESR_MEEEC                          CANFD_MESR_MEEEC_Msk                              /*!< MEEEC[19:16] bits (desc MEEEC) */
// #define CANFD_MESR_MEEEC_0                        (0x1UL << CANFD_MESR_MEEEC_Pos)                   /*!< 0x00010000 */
// #define CANFD_MESR_MEEEC_1                        (0x2UL << CANFD_MESR_MEEEC_Pos)                   /*!< 0x00020000 */
// #define CANFD_MESR_MEEEC_2                        (0x4UL << CANFD_MESR_MEEEC_Pos)                   /*!< 0x00040000 */
// #define CANFD_MESR_MEEEC_3                        (0x8UL << CANFD_MESR_MEEEC_Pos)                   /*!< 0x00080000 */

// #define CANFD_MESR_MENEC_Pos                      (20U)
// #define CANFD_MESR_MENEC_Msk                      (0xFUL << CANFD_MESR_MENEC_Pos)                   /*!< 0x00F00000 */
// #define CANFD_MESR_MENEC                          CANFD_MESR_MENEC_Msk                              /*!< MENEC[23:20] bits (desc MENEC) */
// #define CANFD_MESR_MENEC_0                        (0x1UL << CANFD_MESR_MENEC_Pos)                   /*!< 0x00100000 */
// #define CANFD_MESR_MENEC_1                        (0x2UL << CANFD_MESR_MENEC_Pos)                   /*!< 0x00200000 */
// #define CANFD_MESR_MENEC_2                        (0x4UL << CANFD_MESR_MENEC_Pos)                   /*!< 0x00400000 */
// #define CANFD_MESR_MENEC_3                        (0x8UL << CANFD_MESR_MENEC_Pos)                   /*!< 0x00800000 */

// #define CANFD_MESR_MEL_Pos                        (24U)
// #define CANFD_MESR_MEL_Msk                        (0x3UL << CANFD_MESR_MEL_Pos)                     /*!< 0x03000000 */
// #define CANFD_MESR_MEL                            CANFD_MESR_MEL_Msk                                /*!< MEL[25:24] bits (desc MEL) */
// #define CANFD_MESR_MEL_0                          (0x1UL << CANFD_MESR_MEL_Pos)                     /*!< 0x01000000 */
// #define CANFD_MESR_MEL_1                          (0x2UL << CANFD_MESR_MEL_Pos)                     /*!< 0x02000000 */

// #define CANFD_MESR_MES_Pos                        (26U)
// #define CANFD_MESR_MES_Msk                        (0x1UL << CANFD_MESR_MES_Pos)                     /*!< 0x04000000 */
// #define CANFD_MESR_MES                            CANFD_MESR_MES_Msk                                /*!< desc MES */

/*!< CANFD_ACFCR */
#define CANFD_ACFCR_ACFADR_Pos                    (0U)
#define CANFD_ACFCR_ACFADR_Msk                    (0xFUL << CANFD_ACFCR_ACFADR_Pos)                 /*!< 0x0000000F */
#define CANFD_ACFCR_ACFADR                        CANFD_ACFCR_ACFADR_Msk                            /*!< ACFADR[3:0] bits (desc ACFADR) */
#define CANFD_ACFCR_ACFADR_0                      (0x1UL << CANFD_ACFCR_ACFADR_Pos)                 /*!< 0x00000001 */
#define CANFD_ACFCR_ACFADR_1                      (0x2UL << CANFD_ACFCR_ACFADR_Pos)                 /*!< 0x00000002 */
#define CANFD_ACFCR_ACFADR_2                      (0x4UL << CANFD_ACFCR_ACFADR_Pos)                 /*!< 0x00000004 */
#define CANFD_ACFCR_ACFADR_3                      (0x8UL << CANFD_ACFCR_ACFADR_Pos)                 /*!< 0x00000008 */

#define CANFD_ACFCR_AE_0_Pos                      (16U)
#define CANFD_ACFCR_AE_0_Msk                      (0x1UL << CANFD_ACFCR_AE_0_Pos)                   /*!< 0x00010000 */
#define CANFD_ACFCR_AE_0                          CANFD_ACFCR_AE_0_Msk                              /*!< desc AE_0 */
#define CANFD_ACFCR_AE_1_Pos                      (17U)
#define CANFD_ACFCR_AE_1_Msk                      (0x1UL << CANFD_ACFCR_AE_1_Pos)                   /*!< 0x00020000 */
#define CANFD_ACFCR_AE_1                          CANFD_ACFCR_AE_1_Msk                              /*!< desc AE_1 */
#define CANFD_ACFCR_AE_2_Pos                      (18U)
#define CANFD_ACFCR_AE_2_Msk                      (0x1UL << CANFD_ACFCR_AE_2_Pos)                   /*!< 0x00040000 */
#define CANFD_ACFCR_AE_2                          CANFD_ACFCR_AE_2_Msk                              /*!< desc AE_2 */
#define CANFD_ACFCR_AE_3_Pos                      (19U)
#define CANFD_ACFCR_AE_3_Msk                      (0x1UL << CANFD_ACFCR_AE_3_Pos)                   /*!< 0x00080000 */
#define CANFD_ACFCR_AE_3                          CANFD_ACFCR_AE_3_Msk                              /*!< desc AE_3 */
#define CANFD_ACFCR_AE_4_Pos                      (20U)
#define CANFD_ACFCR_AE_4_Msk                      (0x1UL << CANFD_ACFCR_AE_4_Pos)                   /*!< 0x00100000 */
#define CANFD_ACFCR_AE_4                          CANFD_ACFCR_AE_4_Msk                              /*!< desc AE_4 */
#define CANFD_ACFCR_AE_5_Pos                      (21U)
#define CANFD_ACFCR_AE_5_Msk                      (0x1UL << CANFD_ACFCR_AE_5_Pos)                   /*!< 0x00200000 */
#define CANFD_ACFCR_AE_5                          CANFD_ACFCR_AE_5_Msk                              /*!< desc AE_5 */
#define CANFD_ACFCR_AE_6_Pos                      (22U)
#define CANFD_ACFCR_AE_6_Msk                      (0x1UL << CANFD_ACFCR_AE_6_Pos)                   /*!< 0x00400000 */
#define CANFD_ACFCR_AE_6                          CANFD_ACFCR_AE_6_Msk                              /*!< desc AE_6 */
#define CANFD_ACFCR_AE_7_Pos                      (23U)
#define CANFD_ACFCR_AE_7_Msk                      (0x1UL << CANFD_ACFCR_AE_7_Pos)                   /*!< 0x00800000 */
#define CANFD_ACFCR_AE_7                          CANFD_ACFCR_AE_7_Msk                              /*!< desc AE_7 */
#define CANFD_ACFCR_AE_8_Pos                      (24U)
#define CANFD_ACFCR_AE_8_Msk                      (0x1UL << CANFD_ACFCR_AE_8_Pos)                   /*!< 0x01000000 */
#define CANFD_ACFCR_AE_8                          CANFD_ACFCR_AE_8_Msk                              /*!< desc AE_8 */
#define CANFD_ACFCR_AE_9_Pos                      (25U)
#define CANFD_ACFCR_AE_9_Msk                      (0x1UL << CANFD_ACFCR_AE_9_Pos)                   /*!< 0x02000000 */
#define CANFD_ACFCR_AE_9                          CANFD_ACFCR_AE_9_Msk                              /*!< desc AE_9 */
#define CANFD_ACFCR_AE_10_Pos                     (26U)
#define CANFD_ACFCR_AE_10_Msk                     (0x1UL << CANFD_ACFCR_AE_10_Pos)                  /*!< 0x04000000 */
#define CANFD_ACFCR_AE_10                         CANFD_ACFCR_AE_10_Msk                             /*!< desc AE_10 */
#define CANFD_ACFCR_AE_11_Pos                     (27U)
#define CANFD_ACFCR_AE_11_Msk                     (0x1UL << CANFD_ACFCR_AE_11_Pos)                  /*!< 0x08000000 */
#define CANFD_ACFCR_AE_11                         CANFD_ACFCR_AE_11_Msk                             /*!< desc AE_11 */
// #define CANFD_ACFCR_AE_12_Pos                     (28U)
// #define CANFD_ACFCR_AE_12_Msk                     (0x1UL << CANFD_ACFCR_AE_12_Pos)                  /*!< 0x10000000 */
// #define CANFD_ACFCR_AE_12                         CANFD_ACFCR_AE_12_Msk                             /*!< desc AE_12 */
// #define CANFD_ACFCR_AE_13_Pos                     (29U)
// #define CANFD_ACFCR_AE_13_Msk                     (0x1UL << CANFD_ACFCR_AE_13_Pos)                  /*!< 0x20000000 */
// #define CANFD_ACFCR_AE_13                         CANFD_ACFCR_AE_13_Msk                             /*!< desc AE_13 */
// #define CANFD_ACFCR_AE_14_Pos                     (30U)
// #define CANFD_ACFCR_AE_14_Msk                     (0x1UL << CANFD_ACFCR_AE_14_Pos)                  /*!< 0x40000000 */
// #define CANFD_ACFCR_AE_14                         CANFD_ACFCR_AE_14_Msk                             /*!< desc AE_14 */
// #define CANFD_ACFCR_AE_15_Pos                     (31U)
// #define CANFD_ACFCR_AE_15_Msk                     (0x1UL << CANFD_ACFCR_AE_15_Pos)                  /*!< 0x80000000 */
// #define CANFD_ACFCR_AE_15                         CANFD_ACFCR_AE_15_Msk                             /*!< desc AE_15 */

// /*!< CANFD_PWMCR */
// #define CANFD_PWMCR_PWMO_Pos                      (0U)
// #define CANFD_PWMCR_PWMO_Msk                      (0x3FUL << CANFD_PWMCR_PWMO_Pos)                  /*!< 0x0000003F */
// #define CANFD_PWMCR_PWMO                          CANFD_PWMCR_PWMO_Msk                              /*!< PWMO[5:0] bits (desc PWMO) */
// #define CANFD_PWMCR_PWMO_0                        (0x1UL << CANFD_PWMCR_PWMO_Pos)                   /*!< 0x00000001 */
// #define CANFD_PWMCR_PWMO_1                        (0x2UL << CANFD_PWMCR_PWMO_Pos)                   /*!< 0x00000002 */
// #define CANFD_PWMCR_PWMO_2                        (0x4UL << CANFD_PWMCR_PWMO_Pos)                   /*!< 0x00000004 */
// #define CANFD_PWMCR_PWMO_3                        (0x8UL << CANFD_PWMCR_PWMO_Pos)                   /*!< 0x00000008 */
// #define CANFD_PWMCR_PWMO_4                        (0x10UL << CANFD_PWMCR_PWMO_Pos)                  /*!< 0x00000010 */
// #define CANFD_PWMCR_PWMO_5                        (0x20UL << CANFD_PWMCR_PWMO_Pos)                  /*!< 0x00000020 */

// #define CANFD_PWMCR_PWMS_Pos                      (8U)
// #define CANFD_PWMCR_PWMS_Msk                      (0x3FUL << CANFD_PWMCR_PWMS_Pos)                  /*!< 0x00003F00 */
// #define CANFD_PWMCR_PWMS                          CANFD_PWMCR_PWMS_Msk                              /*!< PWMS[13:8] bits (desc PWMS) */
// #define CANFD_PWMCR_PWMS_0                        (0x1UL << CANFD_PWMCR_PWMS_Pos)                   /*!< 0x00000100 */
// #define CANFD_PWMCR_PWMS_1                        (0x2UL << CANFD_PWMCR_PWMS_Pos)                   /*!< 0x00000200 */
// #define CANFD_PWMCR_PWMS_2                        (0x4UL << CANFD_PWMCR_PWMS_Pos)                   /*!< 0x00000400 */
// #define CANFD_PWMCR_PWMS_3                        (0x8UL << CANFD_PWMCR_PWMS_Pos)                   /*!< 0x00000800 */
// #define CANFD_PWMCR_PWMS_4                        (0x10UL << CANFD_PWMCR_PWMS_Pos)                  /*!< 0x00001000 */
// #define CANFD_PWMCR_PWMS_5                        (0x20UL << CANFD_PWMCR_PWMS_Pos)                  /*!< 0x00002000 */

// #define CANFD_PWMCR_PWML_Pos                      (16U)
// #define CANFD_PWMCR_PWML_Msk                      (0x3FUL << CANFD_PWMCR_PWML_Pos)                  /*!< 0x003F0000 */
// #define CANFD_PWMCR_PWML                          CANFD_PWMCR_PWML_Msk                              /*!< PWML[21:16] bits (desc PWML) */
// #define CANFD_PWMCR_PWML_0                        (0x1UL << CANFD_PWMCR_PWML_Pos)                   /*!< 0x00010000 */
// #define CANFD_PWMCR_PWML_1                        (0x2UL << CANFD_PWMCR_PWML_Pos)                   /*!< 0x00020000 */
// #define CANFD_PWMCR_PWML_2                        (0x4UL << CANFD_PWMCR_PWML_Pos)                   /*!< 0x00040000 */
// #define CANFD_PWMCR_PWML_3                        (0x8UL << CANFD_PWMCR_PWML_Pos)                   /*!< 0x00080000 */
// #define CANFD_PWMCR_PWML_4                        (0x10UL << CANFD_PWMCR_PWML_Pos)                  /*!< 0x00100000 */
// #define CANFD_PWMCR_PWML_5                        (0x20UL << CANFD_PWMCR_PWML_Pos)                  /*!< 0x00200000 */


/*********************  Bits Define For Peripheral CRC  *********************/
/*!< CRC_DR */
#define CRC_DR_DR_Pos                             (0U)
#define CRC_DR_DR_Msk                             (0xFFFFFFFFUL << CRC_DR_DR_Pos)                   /*!< 0xFFFFFFFF */
#define CRC_DR_DR                                 CRC_DR_DR_Msk                                     /*!< DR[31:0] bits (desc DR) */

/*!< CRC_IDR */
#define CRC_IDR_IDR_Pos                           (0U)
#define CRC_IDR_IDR_Msk                           (0xFFUL << CRC_IDR_IDR_Pos)                       /*!< 0x000000FF */
#define CRC_IDR_IDR                               CRC_IDR_IDR_Msk                                   /*!< IDR[7:0] bits (desc IDR) */

/*!< CRC_CR */
#define CRC_CR_RESET_Pos                          (0U)
#define CRC_CR_RESET_Msk                          (0x1UL << CRC_CR_RESET_Pos)                       /*!< 0x00000001 */
#define CRC_CR_RESET                              CRC_CR_RESET_Msk                                  /*!< desc RESET */

/*********************  Bits Define For Peripheral CTC  *********************/
/*!< CTC_CTL0 */
#define CTC_CTL0_CKOKIE_Pos                       (0U)
#define CTC_CTL0_CKOKIE_Msk                       (0x1UL << CTC_CTL0_CKOKIE_Pos)                    /*!< 0x00000001 */
#define CTC_CTL0_CKOKIE                           CTC_CTL0_CKOKIE_Msk                               /*!< desc CKOKIE */
#define CTC_CTL0_CKWARNIE_Pos                     (1U)
#define CTC_CTL0_CKWARNIE_Msk                     (0x1UL << CTC_CTL0_CKWARNIE_Pos)                  /*!< 0x00000002 */
#define CTC_CTL0_CKWARNIE                         CTC_CTL0_CKWARNIE_Msk                             /*!< desc CKWARNIE */
#define CTC_CTL0_ERRIE_Pos                        (2U)
#define CTC_CTL0_ERRIE_Msk                        (0x1UL << CTC_CTL0_ERRIE_Pos)                     /*!< 0x00000004 */
#define CTC_CTL0_ERRIE                            CTC_CTL0_ERRIE_Msk                                /*!< desc ERRIE */
#define CTC_CTL0_EREFIE_Pos                       (3U)
#define CTC_CTL0_EREFIE_Msk                       (0x1UL << CTC_CTL0_EREFIE_Pos)                    /*!< 0x00000008 */
#define CTC_CTL0_EREFIE                           CTC_CTL0_EREFIE_Msk                               /*!< desc EREFIE */
#define CTC_CTL0_CNTEN_Pos                        (5U)
#define CTC_CTL0_CNTEN_Msk                        (0x1UL << CTC_CTL0_CNTEN_Pos)                     /*!< 0x00000020 */
#define CTC_CTL0_CNTEN                            CTC_CTL0_CNTEN_Msk                                /*!< desc CNTEN */
#define CTC_CTL0_AUTOTRIM_Pos                     (6U)
#define CTC_CTL0_AUTOTRIM_Msk                     (0x1UL << CTC_CTL0_AUTOTRIM_Pos)                  /*!< 0x00000040 */
#define CTC_CTL0_AUTOTRIM                         CTC_CTL0_AUTOTRIM_Msk                             /*!< desc AUTOTRIM */
#define CTC_CTL0_SWREFPUL_Pos                     (7U)
#define CTC_CTL0_SWREFPUL_Msk                     (0x1UL << CTC_CTL0_SWREFPUL_Pos)                  /*!< 0x00000080 */
#define CTC_CTL0_SWREFPUL                         CTC_CTL0_SWREFPUL_Msk                             /*!< desc SWREFPUL */
#define CTC_CTL0_TRIMVALUE_Pos                    (8U)
#define CTC_CTL0_TRIMVALUE_Msk                    (0x7FUL << CTC_CTL0_TRIMVALUE_Pos)                /*!< 0x00007F00 */
#define CTC_CTL0_TRIMVALUE                        CTC_CTL0_TRIMVALUE_Msk                            /*!< TRIMVALUE[13:8] bits (desc TRIMVALUE) */
#define CTC_CTL0_TRIMVALUE_0                      (0x1UL << CTC_CTL0_TRIMVALUE_Pos)                 /*!< 0x00000100 */
#define CTC_CTL0_TRIMVALUE_1                      (0x2UL << CTC_CTL0_TRIMVALUE_Pos)                 /*!< 0x00000200 */
#define CTC_CTL0_TRIMVALUE_2                      (0x4UL << CTC_CTL0_TRIMVALUE_Pos)                 /*!< 0x00000400 */
#define CTC_CTL0_TRIMVALUE_3                      (0x8UL << CTC_CTL0_TRIMVALUE_Pos)                 /*!< 0x00000800 */
#define CTC_CTL0_TRIMVALUE_4                      (0x10UL << CTC_CTL0_TRIMVALUE_Pos)                /*!< 0x00001000 */
#define CTC_CTL0_TRIMVALUE_5                      (0x20UL << CTC_CTL0_TRIMVALUE_Pos)                /*!< 0x00002000 */
#define CTC_CTL0_TRIMVALUE_6                      (0x40UL << CTC_CTL0_TRIMVALUE_Pos)                /*!< 0x00004000 */


/*!< CTC_CTL1 */
#define CTC_CTL1_RLVALUE_Pos                      (0U)
#define CTC_CTL1_RLVALUE_Msk                      (0xFFFFUL << CTC_CTL1_RLVALUE_Pos)                /*!< 0x0000FFFF */
#define CTC_CTL1_RLVALUE                          CTC_CTL1_RLVALUE_Msk                              /*!< RLVALUE[15:0] bits (desc RLVALUE) */
#define CTC_CTL1_CKLIM_Pos                        (16U)
#define CTC_CTL1_CKLIM_Msk                        (0xFFUL << CTC_CTL1_CKLIM_Pos)                    /*!< 0x00FF0000 */
#define CTC_CTL1_CKLIM                            CTC_CTL1_CKLIM_Msk                                /*!< CKLIM[23:16] bits (desc CKLIM) */
#define CTC_CTL1_REFPSC_Pos                       (24U)
#define CTC_CTL1_REFPSC_Msk                       (0x7UL << CTC_CTL1_REFPSC_Pos)                    /*!< 0x07000000 */
#define CTC_CTL1_REFPSC                           CTC_CTL1_REFPSC_Msk                               /*!< REFPSC[26:24] bits (desc REFPSC) */
#define CTC_CTL1_REFPSC_0                         (0x1UL << CTC_CTL1_REFPSC_Pos)                    /*!< 0x01000000 */
#define CTC_CTL1_REFPSC_1                         (0x2UL << CTC_CTL1_REFPSC_Pos)                    /*!< 0x02000000 */
#define CTC_CTL1_REFPSC_2                         (0x4UL << CTC_CTL1_REFPSC_Pos)                    /*!< 0x04000000 */

#define CTC_CTL1_REFSEL_Pos                       (28U)
#define CTC_CTL1_REFSEL_Msk                       (0x3UL << CTC_CTL1_REFSEL_Pos)                    /*!< 0x30000000 */
#define CTC_CTL1_REFSEL                           CTC_CTL1_REFSEL_Msk                               /*!< REFSEL[29:28] bits (desc REFSEL) */
#define CTC_CTL1_REFSEL_0                         (0x1UL << CTC_CTL1_REFSEL_Pos)                    /*!< 0x10000000 */
#define CTC_CTL1_REFSEL_1                         (0x2UL << CTC_CTL1_REFSEL_Pos)                    /*!< 0x20000000 */

#define CTC_CTL1_REFPOL_Pos                       (31U)
#define CTC_CTL1_REFPOL_Msk                       (0x1UL << CTC_CTL1_REFPOL_Pos)                    /*!< 0x80000000 */
#define CTC_CTL1_REFPOL                           CTC_CTL1_REFPOL_Msk                               /*!< desc REFPOL */

/*!< CTC_SR */
#define CTC_SR_CKOKIF_Pos                         (0U)
#define CTC_SR_CKOKIF_Msk                         (0x1UL << CTC_SR_CKOKIF_Pos)                      /*!< 0x00000001 */
#define CTC_SR_CKOKIF                             CTC_SR_CKOKIF_Msk                                 /*!< desc CKOKIF */
#define CTC_SR_CKWARNIF_Pos                       (1U)
#define CTC_SR_CKWARNIF_Msk                       (0x1UL << CTC_SR_CKWARNIF_Pos)                    /*!< 0x00000002 */
#define CTC_SR_CKWARNIF                           CTC_SR_CKWARNIF_Msk                               /*!< desc CKWARNIF */
#define CTC_SR_ERRIF_Pos                          (2U)
#define CTC_SR_ERRIF_Msk                          (0x1UL << CTC_SR_ERRIF_Pos)                       /*!< 0x00000004 */
#define CTC_SR_ERRIF                              CTC_SR_ERRIF_Msk                                  /*!< desc ERRIF */
#define CTC_SR_EREFIF_Pos                         (3U)
#define CTC_SR_EREFIF_Msk                         (0x1UL << CTC_SR_EREFIF_Pos)                      /*!< 0x00000008 */
#define CTC_SR_EREFIF                             CTC_SR_EREFIF_Msk                                 /*!< desc EREFIF */
#define CTC_SR_CKERR_Pos                          (8U)
#define CTC_SR_CKERR_Msk                          (0x1UL << CTC_SR_CKERR_Pos)                       /*!< 0x00000100 */
#define CTC_SR_CKERR                              CTC_SR_CKERR_Msk                                  /*!< desc CKERR */
#define CTC_SR_REFMISS_Pos                        (9U)
#define CTC_SR_REFMISS_Msk                        (0x1UL << CTC_SR_REFMISS_Pos)                     /*!< 0x00000200 */
#define CTC_SR_REFMISS                            CTC_SR_REFMISS_Msk                                /*!< desc REFMISS */
#define CTC_SR_TRIMERR_Pos                        (10U)
#define CTC_SR_TRIMERR_Msk                        (0x1UL << CTC_SR_TRIMERR_Pos)                     /*!< 0x00000400 */
#define CTC_SR_TRIMERR                            CTC_SR_TRIMERR_Msk                                /*!< desc TRIMERR */
#define CTC_SR_REFDIR_Pos                         (15U)
#define CTC_SR_REFDIR_Msk                         (0x1UL << CTC_SR_REFDIR_Pos)                      /*!< 0x00008000 */
#define CTC_SR_REFDIR                             CTC_SR_REFDIR_Msk                                 /*!< desc REFDIR */
#define CTC_SR_REFCAP_Pos                         (16U)
#define CTC_SR_REFCAP_Msk                         (0xFFFFUL << CTC_SR_REFCAP_Pos)                   /*!< 0xFFFF0000 */
#define CTC_SR_REFCAP                             CTC_SR_REFCAP_Msk                                 /*!< REFCAP[31:16] bits (desc REFCAP) */

/*!< CTC_INTC */
#define CTC_INTC_CKOKIC_Pos                       (0U)
#define CTC_INTC_CKOKIC_Msk                       (0x1UL << CTC_INTC_CKOKIC_Pos)                    /*!< 0x00000001 */
#define CTC_INTC_CKOKIC                           CTC_INTC_CKOKIC_Msk                               /*!< desc CKOKIC */
#define CTC_INTC_CKWARNIC_Pos                     (1U)
#define CTC_INTC_CKWARNIC_Msk                     (0x1UL << CTC_INTC_CKWARNIC_Pos)                  /*!< 0x00000002 */
#define CTC_INTC_CKWARNIC                         CTC_INTC_CKWARNIC_Msk                             /*!< desc CKWARNIC */
#define CTC_INTC_ERRIC_Pos                        (2U)
#define CTC_INTC_ERRIC_Msk                        (0x1UL << CTC_INTC_ERRIC_Pos)                     /*!< 0x00000004 */
#define CTC_INTC_ERRIC                            CTC_INTC_ERRIC_Msk                                /*!< desc ERRIC */
#define CTC_INTC_EREFIC_Pos                       (3U)
#define CTC_INTC_EREFIC_Msk                       (0x1UL << CTC_INTC_EREFIC_Pos)                    /*!< 0x00000008 */
#define CTC_INTC_EREFIC                           CTC_INTC_EREFIC_Msk                               /*!< desc EREFIC */

/*********************  Bits Define For Peripheral DBGMCU  *********************/
/*!< DBGMCU_IDCODE */
#define DBGMCU_IDCODE_REV_ID_Pos                  (0U)
#define DBGMCU_IDCODE_REV_ID_Msk                  (0xFFFFFFFFUL << DBGMCU_IDCODE_REV_ID_Pos)        /*!< 0xFFFFFFFF */
#define DBGMCU_IDCODE_REV_ID                      DBGMCU_IDCODE_REV_ID_Msk                          /*!< REV_ID[31:0] bits (desc REV_ID) */

/*!< DBGMCU_CR */
#define DBGMCU_CR_DBG_SLEEP_Pos                   (0U)
#define DBGMCU_CR_DBG_SLEEP_Msk                   (0x1UL << DBGMCU_CR_DBG_SLEEP_Pos)                /*!< 0x00000001 */
#define DBGMCU_CR_DBG_SLEEP                       DBGMCU_CR_DBG_SLEEP_Msk                           /*!< desc DBG_SLEEP */
#define DBGMCU_CR_DBG_STOP_Pos                    (1U)
#define DBGMCU_CR_DBG_STOP_Msk                    (0x1UL << DBGMCU_CR_DBG_STOP_Pos)                 /*!< 0x00000002 */
#define DBGMCU_CR_DBG_STOP                        DBGMCU_CR_DBG_STOP_Msk                            /*!< desc DBG_STOP */
#define DBGMCU_CR_DBG_STDBY_Pos                   (2U)
#define DBGMCU_CR_DBG_STDBY_Msk                   (0x1UL << DBGMCU_CR_DBG_STDBY_Pos)                /*!< 0x00000004 */
#define DBGMCU_CR_DBG_STDBY                       DBGMCU_CR_DBG_STDBY_Msk                           /*!< desc DBG_STDBY */
#define DBGMCU_CR_TRACE_IOEN_Pos                  (5U)
#define DBGMCU_CR_TRACE_IOEN_Msk                  (0x1UL << DBGMCU_CR_TRACE_IOEN_Pos)               /*!< 0x00000020 */
#define DBGMCU_CR_TRACE_IOEN                      DBGMCU_CR_TRACE_IOEN_Msk                          /*!< desc TRACE_IOEN */
#define DBGMCU_CR_TRACE_MODE_Pos                  (6U)
#define DBGMCU_CR_TRACE_MODE_Msk                  (0x3UL << DBGMCU_CR_TRACE_MODE_Pos)               /*!< 0x000000C0 */
#define DBGMCU_CR_TRACE_MODE                      DBGMCU_CR_TRACE_MODE_Msk                          /*!< TRACE_MODE[7:6] bits (desc TRACE_MODE) */
#define DBGMCU_CR_TRACE_MODE_0                    (0x1UL << DBGMCU_CR_TRACE_MODE_Pos)               /*!< 0x00000040 */
#define DBGMCU_CR_TRACE_MODE_1                    (0x2UL << DBGMCU_CR_TRACE_MODE_Pos)               /*!< 0x00000080 */

#define DBGMCU_CR_DBG_IWDG_STOP_Pos               (8U)
#define DBGMCU_CR_DBG_IWDG_STOP_Msk               (0x1UL << DBGMCU_CR_DBG_IWDG_STOP_Pos)            /*!< 0x00000100 */
#define DBGMCU_CR_DBG_IWDG_STOP                   DBGMCU_CR_DBG_IWDG_STOP_Msk                       /*!< desc DBG_IWDG_STOP */
#define DBGMCU_CR_DBG_WWDG_STOP_Pos               (9U)
#define DBGMCU_CR_DBG_WWDG_STOP_Msk               (0x1UL << DBGMCU_CR_DBG_WWDG_STOP_Pos)            /*!< 0x00000200 */
#define DBGMCU_CR_DBG_WWDG_STOP                   DBGMCU_CR_DBG_WWDG_STOP_Msk                       /*!< desc DBG_WWDG_STOP */
#define DBGMCU_CR_DBG_TIM1_STOP_Pos               (10U)
#define DBGMCU_CR_DBG_TIM1_STOP_Msk               (0x1UL << DBGMCU_CR_DBG_TIM1_STOP_Pos)            /*!< 0x00000400 */
#define DBGMCU_CR_DBG_TIM1_STOP                   DBGMCU_CR_DBG_TIM1_STOP_Msk                       /*!< desc DBG_TIM1_STOP */
#define DBGMCU_CR_DBG_TIM2_STOP_Pos               (11U)
#define DBGMCU_CR_DBG_TIM2_STOP_Msk               (0x1UL << DBGMCU_CR_DBG_TIM2_STOP_Pos)            /*!< 0x00000800 */
#define DBGMCU_CR_DBG_TIM2_STOP                   DBGMCU_CR_DBG_TIM2_STOP_Msk                       /*!< desc DBG_TIM2_STOP */
#define DBGMCU_CR_DBG_TIM3_STOP_Pos               (12U)
#define DBGMCU_CR_DBG_TIM3_STOP_Msk               (0x1UL << DBGMCU_CR_DBG_TIM3_STOP_Pos)            /*!< 0x00001000 */
#define DBGMCU_CR_DBG_TIM3_STOP                   DBGMCU_CR_DBG_TIM3_STOP_Msk                       /*!< desc DBG_TIM3_STOP */
#define DBGMCU_CR_DBG_TIM4_STOP_Pos               (13U)
#define DBGMCU_CR_DBG_TIM4_STOP_Msk               (0x1UL << DBGMCU_CR_DBG_TIM4_STOP_Pos)            /*!< 0x00002000 */
#define DBGMCU_CR_DBG_TIM4_STOP                   DBGMCU_CR_DBG_TIM4_STOP_Msk                       /*!< desc DBG_TIM4_STOP */
#define DBGMCU_CR_DBG_CAN_STOP_Pos                (14U)
#define DBGMCU_CR_DBG_CAN_STOP_Msk                (0x1UL << DBGMCU_CR_DBG_CAN_STOP_Pos)             /*!< 0x00004000 */
#define DBGMCU_CR_DBG_CAN_STOP                    DBGMCU_CR_DBG_CAN_STOP_Msk                        /*!< desc DBG_CAN_STOP */
#define DBGMCU_CR_DBG_I2C1_SMBUS_TIMEOUT_Pos      (15U)
#define DBGMCU_CR_DBG_I2C1_SMBUS_TIMEOUT_Msk      (0x1UL << DBGMCU_CR_DBG_I2C1_SMBUS_TIMEOUT_Pos)   /*!< 0x00008000 */
#define DBGMCU_CR_DBG_I2C1_SMBUS_TIMEOUT          DBGMCU_CR_DBG_I2C1_SMBUS_TIMEOUT_Msk              /*!< desc DBG_I2C1_SMBUS_TIMEOUT */
#define DBGMCU_CR_DBG_I2C2_SMBUS_TIMEOUT_Pos      (16U)
#define DBGMCU_CR_DBG_I2C2_SMBUS_TIMEOUT_Msk      (0x1UL << DBGMCU_CR_DBG_I2C2_SMBUS_TIMEOUT_Pos)   /*!< 0x00010000 */
#define DBGMCU_CR_DBG_I2C2_SMBUS_TIMEOUT          DBGMCU_CR_DBG_I2C2_SMBUS_TIMEOUT_Msk              /*!< desc DBG_I2C2_SMBUS_TIMEOUT */
#define DBGMCU_CR_DBG_TIM8_STOP_Pos               (17U)
#define DBGMCU_CR_DBG_TIM8_STOP_Msk               (0x1UL << DBGMCU_CR_DBG_TIM8_STOP_Pos)            /*!< 0x00020000 */
#define DBGMCU_CR_DBG_TIM8_STOP                   DBGMCU_CR_DBG_TIM8_STOP_Msk                       /*!< desc DBG_TIM8_STOP */
#define DBGMCU_CR_DBG_TIM5_STOP_Pos               (18U)
#define DBGMCU_CR_DBG_TIM5_STOP_Msk               (0x1UL << DBGMCU_CR_DBG_TIM5_STOP_Pos)            /*!< 0x00040000 */
#define DBGMCU_CR_DBG_TIM5_STOP                   DBGMCU_CR_DBG_TIM5_STOP_Msk                       /*!< desc DBG_TIM5_STOP */
#define DBGMCU_CR_DBG_TIM6_STOP_Pos               (19U)
#define DBGMCU_CR_DBG_TIM6_STOP_Msk               (0x1UL << DBGMCU_CR_DBG_TIM6_STOP_Pos)            /*!< 0x00080000 */
#define DBGMCU_CR_DBG_TIM6_STOP                   DBGMCU_CR_DBG_TIM6_STOP_Msk                       /*!< desc DBG_TIM6_STOP */
#define DBGMCU_CR_DBG_TIM7_STOP_Pos               (20U)
#define DBGMCU_CR_DBG_TIM7_STOP_Msk               (0x1UL << DBGMCU_CR_DBG_TIM7_STOP_Pos)            /*!< 0x00100000 */
#define DBGMCU_CR_DBG_TIM7_STOP                   DBGMCU_CR_DBG_TIM7_STOP_Msk                       /*!< desc DBG_TIM7_STOP */
#define DBGMCU_CR_DBG_TIM12_STOP_Pos              (25U)
#define DBGMCU_CR_DBG_TIM12_STOP_Msk              (0x1UL << DBGMCU_CR_DBG_TIM12_STOP_Pos)           /*!< 0x02000000 */
#define DBGMCU_CR_DBG_TIM12_STOP                  DBGMCU_CR_DBG_TIM12_STOP_Msk                      /*!< desc DBG_TIM12_STOP */
#define DBGMCU_CR_DBG_TIM13_STOP_Pos              (26U)
#define DBGMCU_CR_DBG_TIM13_STOP_Msk              (0x1UL << DBGMCU_CR_DBG_TIM13_STOP_Pos)           /*!< 0x04000000 */
#define DBGMCU_CR_DBG_TIM13_STOP                  DBGMCU_CR_DBG_TIM13_STOP_Msk                      /*!< desc DBG_TIM13_STOP */
#define DBGMCU_CR_DBG_TIM14_STOP_Pos              (27U)
#define DBGMCU_CR_DBG_TIM14_STOP_Msk              (0x1UL << DBGMCU_CR_DBG_TIM14_STOP_Pos)           /*!< 0x08000000 */
#define DBGMCU_CR_DBG_TIM14_STOP                  DBGMCU_CR_DBG_TIM14_STOP_Msk                      /*!< desc DBG_TIM14_STOP */
#define DBGMCU_CR_DBG_TIM9_STOP_Pos               (28U)
#define DBGMCU_CR_DBG_TIM9_STOP_Msk               (0x1UL << DBGMCU_CR_DBG_TIM9_STOP_Pos)            /*!< 0x10000000 */
#define DBGMCU_CR_DBG_TIM9_STOP                   DBGMCU_CR_DBG_TIM9_STOP_Msk                       /*!< desc DBG_TIM9_STOP */
#define DBGMCU_CR_DBG_TIM10_STOP_Pos              (29U)
#define DBGMCU_CR_DBG_TIM10_STOP_Msk              (0x1UL << DBGMCU_CR_DBG_TIM10_STOP_Pos)           /*!< 0x20000000 */
#define DBGMCU_CR_DBG_TIM10_STOP                  DBGMCU_CR_DBG_TIM10_STOP_Msk                      /*!< desc DBG_TIM10_STOP */
#define DBGMCU_CR_DBG_TIM11_STOP_Pos              (30U)
#define DBGMCU_CR_DBG_TIM11_STOP_Msk              (0x1UL << DBGMCU_CR_DBG_TIM11_STOP_Pos)           /*!< 0x40000000 */
#define DBGMCU_CR_DBG_TIM11_STOP                  DBGMCU_CR_DBG_TIM11_STOP_Msk                      /*!< desc DBG_TIM11_STOP */

/******************************************************************************/
/*                                                                            */
/*                             DMA Controller                                 */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for DMA_ISR register  ********************/
#define DMA_ISR_GIF1_Pos                          (0U)
#define DMA_ISR_GIF1_Msk                          (0x1UL << DMA_ISR_GIF1_Pos)                       /*!< 0x00000001 */
#define DMA_ISR_GIF1                              DMA_ISR_GIF1_Msk                                  /*!< Channel 1 Global interrupt flag */
#define DMA_ISR_TCIF1_Pos                         (1U)
#define DMA_ISR_TCIF1_Msk                         (0x1UL << DMA_ISR_TCIF1_Pos)                      /*!< 0x00000002 */
#define DMA_ISR_TCIF1                             DMA_ISR_TCIF1_Msk                                 /*!< Channel 1 Transfer Complete flag */
#define DMA_ISR_HTIF1_Pos                         (2U)
#define DMA_ISR_HTIF1_Msk                         (0x1UL << DMA_ISR_HTIF1_Pos)                      /*!< 0x00000004 */
#define DMA_ISR_HTIF1                             DMA_ISR_HTIF1_Msk                                 /*!< Channel 1 Half Transfer flag */
#define DMA_ISR_TEIF1_Pos                         (3U)
#define DMA_ISR_TEIF1_Msk                         (0x1UL << DMA_ISR_TEIF1_Pos)                      /*!< 0x00000008 */
#define DMA_ISR_TEIF1                             DMA_ISR_TEIF1_Msk                                 /*!< Channel 1 Transfer Error flag */
#define DMA_ISR_GIF2_Pos                          (4U)
#define DMA_ISR_GIF2_Msk                          (0x1UL << DMA_ISR_GIF2_Pos)                       /*!< 0x00000010 */
#define DMA_ISR_GIF2                              DMA_ISR_GIF2_Msk                                  /*!< Channel 2 Global interrupt flag */
#define DMA_ISR_TCIF2_Pos                         (5U)
#define DMA_ISR_TCIF2_Msk                         (0x1UL << DMA_ISR_TCIF2_Pos)                      /*!< 0x00000020 */
#define DMA_ISR_TCIF2                             DMA_ISR_TCIF2_Msk                                 /*!< Channel 2 Transfer Complete flag */
#define DMA_ISR_HTIF2_Pos                         (6U)
#define DMA_ISR_HTIF2_Msk                         (0x1UL << DMA_ISR_HTIF2_Pos)                      /*!< 0x00000040 */
#define DMA_ISR_HTIF2                             DMA_ISR_HTIF2_Msk                                 /*!< Channel 2 Half Transfer flag */
#define DMA_ISR_TEIF2_Pos                         (7U)
#define DMA_ISR_TEIF2_Msk                         (0x1UL << DMA_ISR_TEIF2_Pos)                      /*!< 0x00000080 */
#define DMA_ISR_TEIF2                             DMA_ISR_TEIF2_Msk                                 /*!< Channel 2 Transfer Error flag */
#define DMA_ISR_GIF3_Pos                          (8U)
#define DMA_ISR_GIF3_Msk                          (0x1UL << DMA_ISR_GIF3_Pos)                       /*!< 0x00000100 */
#define DMA_ISR_GIF3                              DMA_ISR_GIF3_Msk                                  /*!< Channel 3 Global interrupt flag */
#define DMA_ISR_TCIF3_Pos                         (9U)
#define DMA_ISR_TCIF3_Msk                         (0x1UL << DMA_ISR_TCIF3_Pos)                      /*!< 0x00000200 */
#define DMA_ISR_TCIF3                             DMA_ISR_TCIF3_Msk                                 /*!< Channel 3 Transfer Complete flag */
#define DMA_ISR_HTIF3_Pos                         (10U)
#define DMA_ISR_HTIF3_Msk                         (0x1UL << DMA_ISR_HTIF3_Pos)                      /*!< 0x00000400 */
#define DMA_ISR_HTIF3                             DMA_ISR_HTIF3_Msk                                 /*!< Channel 3 Half Transfer flag */
#define DMA_ISR_TEIF3_Pos                         (11U)
#define DMA_ISR_TEIF3_Msk                         (0x1UL << DMA_ISR_TEIF3_Pos)                      /*!< 0x00000800 */
#define DMA_ISR_TEIF3                             DMA_ISR_TEIF3_Msk                                 /*!< Channel 3 Transfer Error flag */
#define DMA_ISR_GIF4_Pos                          (12U)
#define DMA_ISR_GIF4_Msk                          (0x1UL << DMA_ISR_GIF4_Pos)                       /*!< 0x00001000 */
#define DMA_ISR_GIF4                              DMA_ISR_GIF4_Msk                                  /*!< Channel 4 Global interrupt flag */
#define DMA_ISR_TCIF4_Pos                         (13U)
#define DMA_ISR_TCIF4_Msk                         (0x1UL << DMA_ISR_TCIF4_Pos)                      /*!< 0x00002000 */
#define DMA_ISR_TCIF4                             DMA_ISR_TCIF4_Msk                                 /*!< Channel 4 Transfer Complete flag */
#define DMA_ISR_HTIF4_Pos                         (14U)
#define DMA_ISR_HTIF4_Msk                         (0x1UL << DMA_ISR_HTIF4_Pos)                      /*!< 0x00004000 */
#define DMA_ISR_HTIF4                             DMA_ISR_HTIF4_Msk                                 /*!< Channel 4 Half Transfer flag */
#define DMA_ISR_TEIF4_Pos                         (15U)
#define DMA_ISR_TEIF4_Msk                         (0x1UL << DMA_ISR_TEIF4_Pos)                      /*!< 0x00008000 */
#define DMA_ISR_TEIF4                             DMA_ISR_TEIF4_Msk                                 /*!< Channel 4 Transfer Error flag */
#define DMA_ISR_GIF5_Pos                          (16U)
#define DMA_ISR_GIF5_Msk                          (0x1UL << DMA_ISR_GIF5_Pos)                       /*!< 0x00010000 */
#define DMA_ISR_GIF5                              DMA_ISR_GIF5_Msk                                  /*!< Channel 5 Global interrupt flag */
#define DMA_ISR_TCIF5_Pos                         (17U)
#define DMA_ISR_TCIF5_Msk                         (0x1UL << DMA_ISR_TCIF5_Pos)                      /*!< 0x00020000 */
#define DMA_ISR_TCIF5                             DMA_ISR_TCIF5_Msk                                 /*!< Channel 5 Transfer Complete flag */
#define DMA_ISR_HTIF5_Pos                         (18U)
#define DMA_ISR_HTIF5_Msk                         (0x1UL << DMA_ISR_HTIF5_Pos)                      /*!< 0x00040000 */
#define DMA_ISR_HTIF5                             DMA_ISR_HTIF5_Msk                                 /*!< Channel 5 Half Transfer flag */
#define DMA_ISR_TEIF5_Pos                         (19U)
#define DMA_ISR_TEIF5_Msk                         (0x1UL << DMA_ISR_TEIF5_Pos)                      /*!< 0x00080000 */
#define DMA_ISR_TEIF5                             DMA_ISR_TEIF5_Msk                                 /*!< Channel 5 Transfer Error flag */
#define DMA_ISR_GIF6_Pos                          (20U)
#define DMA_ISR_GIF6_Msk                          (0x1UL << DMA_ISR_GIF6_Pos)                       /*!< 0x00100000 */
#define DMA_ISR_GIF6                              DMA_ISR_GIF6_Msk                                  /*!< Channel 6 Global interrupt flag */
#define DMA_ISR_TCIF6_Pos                         (21U)
#define DMA_ISR_TCIF6_Msk                         (0x1UL << DMA_ISR_TCIF6_Pos)                      /*!< 0x00200000 */
#define DMA_ISR_TCIF6                             DMA_ISR_TCIF6_Msk                                 /*!< Channel 6 Transfer Complete flag */
#define DMA_ISR_HTIF6_Pos                         (22U)
#define DMA_ISR_HTIF6_Msk                         (0x1UL << DMA_ISR_HTIF6_Pos)                      /*!< 0x00400000 */
#define DMA_ISR_HTIF6                             DMA_ISR_HTIF6_Msk                                 /*!< Channel 6 Half Transfer flag */
#define DMA_ISR_TEIF6_Pos                         (23U)
#define DMA_ISR_TEIF6_Msk                         (0x1UL << DMA_ISR_TEIF6_Pos)                      /*!< 0x00800000 */
#define DMA_ISR_TEIF6                             DMA_ISR_TEIF6_Msk                                 /*!< Channel 6 Transfer Error flag */
#define DMA_ISR_GIF7_Pos                          (24U)
#define DMA_ISR_GIF7_Msk                          (0x1UL << DMA_ISR_GIF7_Pos)                       /*!< 0x01000000 */
#define DMA_ISR_GIF7                              DMA_ISR_GIF7_Msk                                  /*!< Channel 7 Global interrupt flag */
#define DMA_ISR_TCIF7_Pos                         (25U)
#define DMA_ISR_TCIF7_Msk                         (0x1UL << DMA_ISR_TCIF7_Pos)                      /*!< 0x02000000 */
#define DMA_ISR_TCIF7                             DMA_ISR_TCIF7_Msk                                 /*!< Channel 7 Transfer Complete flag */
#define DMA_ISR_HTIF7_Pos                         (26U)
#define DMA_ISR_HTIF7_Msk                         (0x1UL << DMA_ISR_HTIF7_Pos)                      /*!< 0x04000000 */
#define DMA_ISR_HTIF7                             DMA_ISR_HTIF7_Msk                                 /*!< Channel 7 Half Transfer flag */
#define DMA_ISR_TEIF7_Pos                         (27U)
#define DMA_ISR_TEIF7_Msk                         (0x1UL << DMA_ISR_TEIF7_Pos)                      /*!< 0x08000000 */
#define DMA_ISR_TEIF7                             DMA_ISR_TEIF7_Msk                                 /*!< Channel 7 Transfer Error flag */

/*******************  Bit definition for DMA_IFCR register  *******************/
#define DMA_IFCR_CGIF1_Pos                        (0U)
#define DMA_IFCR_CGIF1_Msk                        (0x1UL << DMA_IFCR_CGIF1_Pos)                     /*!< 0x00000001 */
#define DMA_IFCR_CGIF1                            DMA_IFCR_CGIF1_Msk                                /*!< Channel 1 Global interrupt clear */
#define DMA_IFCR_CTCIF1_Pos                       (1U)
#define DMA_IFCR_CTCIF1_Msk                       (0x1UL << DMA_IFCR_CTCIF1_Pos)                    /*!< 0x00000002 */
#define DMA_IFCR_CTCIF1                           DMA_IFCR_CTCIF1_Msk                               /*!< Channel 1 Transfer Complete clear */
#define DMA_IFCR_CHTIF1_Pos                       (2U)
#define DMA_IFCR_CHTIF1_Msk                       (0x1UL << DMA_IFCR_CHTIF1_Pos)                    /*!< 0x00000004 */
#define DMA_IFCR_CHTIF1                           DMA_IFCR_CHTIF1_Msk                               /*!< Channel 1 Half Transfer clear */
#define DMA_IFCR_CTEIF1_Pos                       (3U)
#define DMA_IFCR_CTEIF1_Msk                       (0x1UL << DMA_IFCR_CTEIF1_Pos)                    /*!< 0x00000008 */
#define DMA_IFCR_CTEIF1                           DMA_IFCR_CTEIF1_Msk                               /*!< Channel 1 Transfer Error clear */
#define DMA_IFCR_CGIF2_Pos                        (4U)
#define DMA_IFCR_CGIF2_Msk                        (0x1UL << DMA_IFCR_CGIF2_Pos)                     /*!< 0x00000010 */
#define DMA_IFCR_CGIF2                            DMA_IFCR_CGIF2_Msk                                /*!< Channel 2 Global interrupt clear */
#define DMA_IFCR_CTCIF2_Pos                       (5U)
#define DMA_IFCR_CTCIF2_Msk                       (0x1UL << DMA_IFCR_CTCIF2_Pos)                    /*!< 0x00000020 */
#define DMA_IFCR_CTCIF2                           DMA_IFCR_CTCIF2_Msk                               /*!< Channel 2 Transfer Complete clear */
#define DMA_IFCR_CHTIF2_Pos                       (6U)
#define DMA_IFCR_CHTIF2_Msk                       (0x1UL << DMA_IFCR_CHTIF2_Pos)                    /*!< 0x00000040 */
#define DMA_IFCR_CHTIF2                           DMA_IFCR_CHTIF2_Msk                               /*!< Channel 2 Half Transfer clear */
#define DMA_IFCR_CTEIF2_Pos                       (7U)
#define DMA_IFCR_CTEIF2_Msk                       (0x1UL << DMA_IFCR_CTEIF2_Pos)                    /*!< 0x00000080 */
#define DMA_IFCR_CTEIF2                           DMA_IFCR_CTEIF2_Msk                               /*!< Channel 2 Transfer Error clear */
#define DMA_IFCR_CGIF3_Pos                        (8U)
#define DMA_IFCR_CGIF3_Msk                        (0x1UL << DMA_IFCR_CGIF3_Pos)                     /*!< 0x00000100 */
#define DMA_IFCR_CGIF3                            DMA_IFCR_CGIF3_Msk                                /*!< Channel 3 Global interrupt clear */
#define DMA_IFCR_CTCIF3_Pos                       (9U)
#define DMA_IFCR_CTCIF3_Msk                       (0x1UL << DMA_IFCR_CTCIF3_Pos)                    /*!< 0x00000200 */
#define DMA_IFCR_CTCIF3                           DMA_IFCR_CTCIF3_Msk                               /*!< Channel 3 Transfer Complete clear */
#define DMA_IFCR_CHTIF3_Pos                       (10U)
#define DMA_IFCR_CHTIF3_Msk                       (0x1UL << DMA_IFCR_CHTIF3_Pos)                    /*!< 0x00000400 */
#define DMA_IFCR_CHTIF3                           DMA_IFCR_CHTIF3_Msk                               /*!< Channel 3 Half Transfer clear */
#define DMA_IFCR_CTEIF3_Pos                       (11U)
#define DMA_IFCR_CTEIF3_Msk                       (0x1UL << DMA_IFCR_CTEIF3_Pos)                    /*!< 0x00000800 */
#define DMA_IFCR_CTEIF3                           DMA_IFCR_CTEIF3_Msk                               /*!< Channel 3 Transfer Error clear */
#define DMA_IFCR_CGIF4_Pos                        (12U)
#define DMA_IFCR_CGIF4_Msk                        (0x1UL << DMA_IFCR_CGIF4_Pos)                     /*!< 0x00001000 */
#define DMA_IFCR_CGIF4                            DMA_IFCR_CGIF4_Msk                                /*!< Channel 4 Global interrupt clear */
#define DMA_IFCR_CTCIF4_Pos                       (13U)
#define DMA_IFCR_CTCIF4_Msk                       (0x1UL << DMA_IFCR_CTCIF4_Pos)                    /*!< 0x00002000 */
#define DMA_IFCR_CTCIF4                           DMA_IFCR_CTCIF4_Msk                               /*!< Channel 4 Transfer Complete clear */
#define DMA_IFCR_CHTIF4_Pos                       (14U)
#define DMA_IFCR_CHTIF4_Msk                       (0x1UL << DMA_IFCR_CHTIF4_Pos)                    /*!< 0x00004000 */
#define DMA_IFCR_CHTIF4                           DMA_IFCR_CHTIF4_Msk                               /*!< Channel 4 Half Transfer clear */
#define DMA_IFCR_CTEIF4_Pos                       (15U)
#define DMA_IFCR_CTEIF4_Msk                       (0x1UL << DMA_IFCR_CTEIF4_Pos)                    /*!< 0x00008000 */
#define DMA_IFCR_CTEIF4                           DMA_IFCR_CTEIF4_Msk                               /*!< Channel 4 Transfer Error clear */
#define DMA_IFCR_CGIF5_Pos                        (16U)
#define DMA_IFCR_CGIF5_Msk                        (0x1UL << DMA_IFCR_CGIF5_Pos)                     /*!< 0x00010000 */
#define DMA_IFCR_CGIF5                            DMA_IFCR_CGIF5_Msk                                /*!< Channel 5 Global interrupt clear */
#define DMA_IFCR_CTCIF5_Pos                       (17U)
#define DMA_IFCR_CTCIF5_Msk                       (0x1UL << DMA_IFCR_CTCIF5_Pos)                    /*!< 0x00020000 */
#define DMA_IFCR_CTCIF5                           DMA_IFCR_CTCIF5_Msk                               /*!< Channel 5 Transfer Complete clear */
#define DMA_IFCR_CHTIF5_Pos                       (18U)
#define DMA_IFCR_CHTIF5_Msk                       (0x1UL << DMA_IFCR_CHTIF5_Pos)                    /*!< 0x00040000 */
#define DMA_IFCR_CHTIF5                           DMA_IFCR_CHTIF5_Msk                               /*!< Channel 5 Half Transfer clear */
#define DMA_IFCR_CTEIF5_Pos                       (19U)
#define DMA_IFCR_CTEIF5_Msk                       (0x1UL << DMA_IFCR_CTEIF5_Pos)                    /*!< 0x00080000 */
#define DMA_IFCR_CTEIF5                           DMA_IFCR_CTEIF5_Msk                               /*!< Channel 5 Transfer Error clear */
#define DMA_IFCR_CGIF6_Pos                        (20U)
#define DMA_IFCR_CGIF6_Msk                        (0x1UL << DMA_IFCR_CGIF6_Pos)                     /*!< 0x00100000 */
#define DMA_IFCR_CGIF6                            DMA_IFCR_CGIF6_Msk                                /*!< Channel 6 Global interrupt clear */
#define DMA_IFCR_CTCIF6_Pos                       (21U)
#define DMA_IFCR_CTCIF6_Msk                       (0x1UL << DMA_IFCR_CTCIF6_Pos)                    /*!< 0x00200000 */
#define DMA_IFCR_CTCIF6                           DMA_IFCR_CTCIF6_Msk                               /*!< Channel 6 Transfer Complete clear */
#define DMA_IFCR_CHTIF6_Pos                       (22U)
#define DMA_IFCR_CHTIF6_Msk                       (0x1UL << DMA_IFCR_CHTIF6_Pos)                    /*!< 0x00400000 */
#define DMA_IFCR_CHTIF6                           DMA_IFCR_CHTIF6_Msk                               /*!< Channel 6 Half Transfer clear */
#define DMA_IFCR_CTEIF6_Pos                       (23U)
#define DMA_IFCR_CTEIF6_Msk                       (0x1UL << DMA_IFCR_CTEIF6_Pos)                    /*!< 0x00800000 */
#define DMA_IFCR_CTEIF6                           DMA_IFCR_CTEIF6_Msk                               /*!< Channel 6 Transfer Error clear */
#define DMA_IFCR_CGIF7_Pos                        (24U)
#define DMA_IFCR_CGIF7_Msk                        (0x1UL << DMA_IFCR_CGIF7_Pos)                     /*!< 0x01000000 */
#define DMA_IFCR_CGIF7                            DMA_IFCR_CGIF7_Msk                                /*!< Channel 7 Global interrupt clear */
#define DMA_IFCR_CTCIF7_Pos                       (25U)
#define DMA_IFCR_CTCIF7_Msk                       (0x1UL << DMA_IFCR_CTCIF7_Pos)                    /*!< 0x02000000 */
#define DMA_IFCR_CTCIF7                           DMA_IFCR_CTCIF7_Msk                               /*!< Channel 7 Transfer Complete clear */
#define DMA_IFCR_CHTIF7_Pos                       (26U)
#define DMA_IFCR_CHTIF7_Msk                       (0x1UL << DMA_IFCR_CHTIF7_Pos)                    /*!< 0x04000000 */
#define DMA_IFCR_CHTIF7                           DMA_IFCR_CHTIF7_Msk                               /*!< Channel 7 Half Transfer clear */
#define DMA_IFCR_CTEIF7_Pos                       (27U)
#define DMA_IFCR_CTEIF7_Msk                       (0x1UL << DMA_IFCR_CTEIF7_Pos)                    /*!< 0x08000000 */
#define DMA_IFCR_CTEIF7                           DMA_IFCR_CTEIF7_Msk                               /*!< Channel 7 Transfer Error clear */

/*******************  Bit definition for DMA_CCR register   *******************/
#define DMA_CCR_EN_Pos                            (0U)
#define DMA_CCR_EN_Msk                            (0x1UL << DMA_CCR_EN_Pos)                         /*!< 0x00000001 */
#define DMA_CCR_EN                                DMA_CCR_EN_Msk                                    /*!< Channel enable */
#define DMA_CCR_TCIE_Pos                          (1U)
#define DMA_CCR_TCIE_Msk                          (0x1UL << DMA_CCR_TCIE_Pos)                       /*!< 0x00000002 */
#define DMA_CCR_TCIE                              DMA_CCR_TCIE_Msk                                  /*!< Transfer complete interrupt enable */
#define DMA_CCR_HTIE_Pos                          (2U)
#define DMA_CCR_HTIE_Msk                          (0x1UL << DMA_CCR_HTIE_Pos)                       /*!< 0x00000004 */
#define DMA_CCR_HTIE                              DMA_CCR_HTIE_Msk                                  /*!< Half Transfer interrupt enable */
#define DMA_CCR_TEIE_Pos                          (3U)
#define DMA_CCR_TEIE_Msk                          (0x1UL << DMA_CCR_TEIE_Pos)                       /*!< 0x00000008 */
#define DMA_CCR_TEIE                              DMA_CCR_TEIE_Msk                                  /*!< Transfer error interrupt enable */
#define DMA_CCR_DIR_Pos                           (4U)
#define DMA_CCR_DIR_Msk                           (0x1UL << DMA_CCR_DIR_Pos)                        /*!< 0x00000010 */
#define DMA_CCR_DIR                               DMA_CCR_DIR_Msk                                   /*!< Data transfer direction */
#define DMA_CCR_CIRC_Pos                          (5U)
#define DMA_CCR_CIRC_Msk                          (0x1UL << DMA_CCR_CIRC_Pos)                       /*!< 0x00000020 */
#define DMA_CCR_CIRC                              DMA_CCR_CIRC_Msk                                  /*!< Circular mode */
#define DMA_CCR_PINC_Pos                          (6U)
#define DMA_CCR_PINC_Msk                          (0x1UL << DMA_CCR_PINC_Pos)                       /*!< 0x00000040 */
#define DMA_CCR_PINC                              DMA_CCR_PINC_Msk                                  /*!< Peripheral increment mode */
#define DMA_CCR_MINC_Pos                          (7U)
#define DMA_CCR_MINC_Msk                          (0x1UL << DMA_CCR_MINC_Pos)                       /*!< 0x00000080 */
#define DMA_CCR_MINC                              DMA_CCR_MINC_Msk                                  /*!< Memory increment mode */

#define DMA_CCR_PSIZE_Pos                         (8U)
#define DMA_CCR_PSIZE_Msk                         (0x3UL << DMA_CCR_PSIZE_Pos)                      /*!< 0x00000300 */
#define DMA_CCR_PSIZE                             DMA_CCR_PSIZE_Msk                                 /*!< PSIZE[1:0] bits (Peripheral size) */
#define DMA_CCR_PSIZE_0                           (0x1UL << DMA_CCR_PSIZE_Pos)                      /*!< 0x00000100 */
#define DMA_CCR_PSIZE_1                           (0x2UL << DMA_CCR_PSIZE_Pos)                      /*!< 0x00000200 */

#define DMA_CCR_MSIZE_Pos                         (10U)
#define DMA_CCR_MSIZE_Msk                         (0x3UL << DMA_CCR_MSIZE_Pos)                      /*!< 0x00000C00 */
#define DMA_CCR_MSIZE                             DMA_CCR_MSIZE_Msk                                 /*!< MSIZE[1:0] bits (Memory size) */
#define DMA_CCR_MSIZE_0                           (0x1UL << DMA_CCR_MSIZE_Pos)                      /*!< 0x00000400 */
#define DMA_CCR_MSIZE_1                           (0x2UL << DMA_CCR_MSIZE_Pos)                      /*!< 0x00000800 */

#define DMA_CCR_PL_Pos                            (12U)
#define DMA_CCR_PL_Msk                            (0x3UL << DMA_CCR_PL_Pos)                         /*!< 0x00003000 */
#define DMA_CCR_PL                                DMA_CCR_PL_Msk                                    /*!< PL[1:0] bits(Channel Priority level) */
#define DMA_CCR_PL_0                              (0x1UL << DMA_CCR_PL_Pos)                         /*!< 0x00001000 */
#define DMA_CCR_PL_1                              (0x2UL << DMA_CCR_PL_Pos)                         /*!< 0x00002000 */

#define DMA_CCR_MEM2MEM_Pos                       (14U)
#define DMA_CCR_MEM2MEM_Msk                       (0x1UL << DMA_CCR_MEM2MEM_Pos)                    /*!< 0x00004000 */
#define DMA_CCR_MEM2MEM                           DMA_CCR_MEM2MEM_Msk                               /*!< Memory to memory mode */

/******************  Bit definition for DMA_CNDTR  register  ******************/
#define DMA_CNDTR_NDT_Pos                         (0U)
#define DMA_CNDTR_NDT_Msk                         (0xFFFFUL << DMA_CNDTR_NDT_Pos)                   /*!< 0x0000FFFF */
#define DMA_CNDTR_NDT                             DMA_CNDTR_NDT_Msk                                 /*!< Number of data to Transfer */

/******************  Bit definition for DMA_CPAR  register  *******************/
#define DMA_CPAR_PA_Pos                           (0U)
#define DMA_CPAR_PA_Msk                           (0xFFFFFFFFUL << DMA_CPAR_PA_Pos)                 /*!< 0xFFFFFFFF */
#define DMA_CPAR_PA                               DMA_CPAR_PA_Msk                                   /*!< Peripheral Address */

/******************  Bit definition for DMA_CMAR  register  *******************/
#define DMA_CMAR_MA_Pos                           (0U)
#define DMA_CMAR_MA_Msk                           (0xFFFFFFFFUL << DMA_CMAR_MA_Pos)                 /*!< 0xFFFFFFFF */
#define DMA_CMAR_MA                               DMA_CMAR_MA_Msk                                   /*!< Memory Address */

/******************************************************************************/
/*                                                                            */
/*                    External Interrupt/Event Controller                     */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for EXTI_IMR register  *******************/
#define EXTI_IMR_MR0_Pos                          (0U)
#define EXTI_IMR_MR0_Msk                          (0x1UL << EXTI_IMR_MR0_Pos)         /*!< 0x00000001 */
#define EXTI_IMR_MR0                              EXTI_IMR_MR0_Msk                   /*!< Interrupt Mask on line 0 */
#define EXTI_IMR_MR1_Pos                          (1U)
#define EXTI_IMR_MR1_Msk                          (0x1UL << EXTI_IMR_MR1_Pos)         /*!< 0x00000002 */
#define EXTI_IMR_MR1                              EXTI_IMR_MR1_Msk                   /*!< Interrupt Mask on line 1 */
#define EXTI_IMR_MR2_Pos                          (2U)
#define EXTI_IMR_MR2_Msk                          (0x1UL << EXTI_IMR_MR2_Pos)         /*!< 0x00000004 */
#define EXTI_IMR_MR2                              EXTI_IMR_MR2_Msk                   /*!< Interrupt Mask on line 2 */
#define EXTI_IMR_MR3_Pos                          (3U)
#define EXTI_IMR_MR3_Msk                          (0x1UL << EXTI_IMR_MR3_Pos)         /*!< 0x00000008 */
#define EXTI_IMR_MR3                              EXTI_IMR_MR3_Msk                   /*!< Interrupt Mask on line 3 */
#define EXTI_IMR_MR4_Pos                          (4U)
#define EXTI_IMR_MR4_Msk                          (0x1UL << EXTI_IMR_MR4_Pos)         /*!< 0x00000010 */
#define EXTI_IMR_MR4                              EXTI_IMR_MR4_Msk                   /*!< Interrupt Mask on line 4 */
#define EXTI_IMR_MR5_Pos                          (5U)
#define EXTI_IMR_MR5_Msk                          (0x1UL << EXTI_IMR_MR5_Pos)         /*!< 0x00000020 */
#define EXTI_IMR_MR5                              EXTI_IMR_MR5_Msk                   /*!< Interrupt Mask on line 5 */
#define EXTI_IMR_MR6_Pos                          (6U)
#define EXTI_IMR_MR6_Msk                          (0x1UL << EXTI_IMR_MR6_Pos)         /*!< 0x00000040 */
#define EXTI_IMR_MR6                              EXTI_IMR_MR6_Msk                   /*!< Interrupt Mask on line 6 */
#define EXTI_IMR_MR7_Pos                          (7U)
#define EXTI_IMR_MR7_Msk                          (0x1UL << EXTI_IMR_MR7_Pos)         /*!< 0x00000080 */
#define EXTI_IMR_MR7                              EXTI_IMR_MR7_Msk                   /*!< Interrupt Mask on line 7 */
#define EXTI_IMR_MR8_Pos                          (8U)
#define EXTI_IMR_MR8_Msk                          (0x1UL << EXTI_IMR_MR8_Pos)         /*!< 0x00000100 */
#define EXTI_IMR_MR8                              EXTI_IMR_MR8_Msk                   /*!< Interrupt Mask on line 8 */
#define EXTI_IMR_MR9_Pos                          (9U)
#define EXTI_IMR_MR9_Msk                          (0x1UL << EXTI_IMR_MR9_Pos)         /*!< 0x00000200 */
#define EXTI_IMR_MR9                              EXTI_IMR_MR9_Msk                   /*!< Interrupt Mask on line 9 */
#define EXTI_IMR_MR10_Pos                         (10U)
#define EXTI_IMR_MR10_Msk                         (0x1UL << EXTI_IMR_MR10_Pos)        /*!< 0x00000400 */
#define EXTI_IMR_MR10                             EXTI_IMR_MR10_Msk                  /*!< Interrupt Mask on line 10 */
#define EXTI_IMR_MR11_Pos                         (11U)
#define EXTI_IMR_MR11_Msk                         (0x1UL << EXTI_IMR_MR11_Pos)        /*!< 0x00000800 */
#define EXTI_IMR_MR11                             EXTI_IMR_MR11_Msk                  /*!< Interrupt Mask on line 11 */
#define EXTI_IMR_MR12_Pos                         (12U)
#define EXTI_IMR_MR12_Msk                         (0x1UL << EXTI_IMR_MR12_Pos)        /*!< 0x00001000 */
#define EXTI_IMR_MR12                             EXTI_IMR_MR12_Msk                  /*!< Interrupt Mask on line 12 */
#define EXTI_IMR_MR13_Pos                         (13U)
#define EXTI_IMR_MR13_Msk                         (0x1UL << EXTI_IMR_MR13_Pos)        /*!< 0x00002000 */
#define EXTI_IMR_MR13                             EXTI_IMR_MR13_Msk                  /*!< Interrupt Mask on line 13 */
#define EXTI_IMR_MR14_Pos                         (14U)
#define EXTI_IMR_MR14_Msk                         (0x1UL << EXTI_IMR_MR14_Pos)        /*!< 0x00004000 */
#define EXTI_IMR_MR14                             EXTI_IMR_MR14_Msk                  /*!< Interrupt Mask on line 14 */
#define EXTI_IMR_MR15_Pos                         (15U)
#define EXTI_IMR_MR15_Msk                         (0x1UL << EXTI_IMR_MR15_Pos)        /*!< 0x00008000 */
#define EXTI_IMR_MR15                             EXTI_IMR_MR15_Msk                  /*!< Interrupt Mask on line 15 */
#define EXTI_IMR_MR16_Pos                         (16U)
#define EXTI_IMR_MR16_Msk                         (0x1UL << EXTI_IMR_MR16_Pos)        /*!< 0x00010000 */
#define EXTI_IMR_MR16                             EXTI_IMR_MR16_Msk                  /*!< Interrupt Mask on line 16 */
#define EXTI_IMR_MR17_Pos                         (17U)
#define EXTI_IMR_MR17_Msk                         (0x1UL << EXTI_IMR_MR17_Pos)        /*!< 0x00020000 */
#define EXTI_IMR_MR17                             EXTI_IMR_MR17_Msk                  /*!< Interrupt Mask on line 17 */

/* References Defines */
#define  EXTI_IMR_IM0 EXTI_IMR_MR0
#define  EXTI_IMR_IM1 EXTI_IMR_MR1
#define  EXTI_IMR_IM2 EXTI_IMR_MR2
#define  EXTI_IMR_IM3 EXTI_IMR_MR3
#define  EXTI_IMR_IM4 EXTI_IMR_MR4
#define  EXTI_IMR_IM5 EXTI_IMR_MR5
#define  EXTI_IMR_IM6 EXTI_IMR_MR6
#define  EXTI_IMR_IM7 EXTI_IMR_MR7
#define  EXTI_IMR_IM8 EXTI_IMR_MR8
#define  EXTI_IMR_IM9 EXTI_IMR_MR9
#define  EXTI_IMR_IM10 EXTI_IMR_MR10
#define  EXTI_IMR_IM11 EXTI_IMR_MR11
#define  EXTI_IMR_IM12 EXTI_IMR_MR12
#define  EXTI_IMR_IM13 EXTI_IMR_MR13
#define  EXTI_IMR_IM14 EXTI_IMR_MR14
#define  EXTI_IMR_IM15 EXTI_IMR_MR15
#define  EXTI_IMR_IM16 EXTI_IMR_MR16
#define  EXTI_IMR_IM17 EXTI_IMR_MR17
#define  EXTI_IMR_IM   0x0003FFFFU        /*!< Interrupt Mask All */

/*******************  Bit definition for EXTI_EMR register  *******************/
#define EXTI_EMR_MR0_Pos                          (0U)
#define EXTI_EMR_MR0_Msk                          (0x1UL << EXTI_EMR_MR0_Pos)         /*!< 0x00000001 */
#define EXTI_EMR_MR0                              EXTI_EMR_MR0_Msk                   /*!< Event Mask on line 0 */
#define EXTI_EMR_MR1_Pos                          (1U)
#define EXTI_EMR_MR1_Msk                          (0x1UL << EXTI_EMR_MR1_Pos)         /*!< 0x00000002 */
#define EXTI_EMR_MR1                              EXTI_EMR_MR1_Msk                   /*!< Event Mask on line 1 */
#define EXTI_EMR_MR2_Pos                          (2U)
#define EXTI_EMR_MR2_Msk                          (0x1UL << EXTI_EMR_MR2_Pos)         /*!< 0x00000004 */
#define EXTI_EMR_MR2                              EXTI_EMR_MR2_Msk                   /*!< Event Mask on line 2 */
#define EXTI_EMR_MR3_Pos                          (3U)
#define EXTI_EMR_MR3_Msk                          (0x1UL << EXTI_EMR_MR3_Pos)         /*!< 0x00000008 */
#define EXTI_EMR_MR3                              EXTI_EMR_MR3_Msk                   /*!< Event Mask on line 3 */
#define EXTI_EMR_MR4_Pos                          (4U)
#define EXTI_EMR_MR4_Msk                          (0x1UL << EXTI_EMR_MR4_Pos)         /*!< 0x00000010 */
#define EXTI_EMR_MR4                              EXTI_EMR_MR4_Msk                   /*!< Event Mask on line 4 */
#define EXTI_EMR_MR5_Pos                          (5U)
#define EXTI_EMR_MR5_Msk                          (0x1UL << EXTI_EMR_MR5_Pos)         /*!< 0x00000020 */
#define EXTI_EMR_MR5                              EXTI_EMR_MR5_Msk                   /*!< Event Mask on line 5 */
#define EXTI_EMR_MR6_Pos                          (6U)
#define EXTI_EMR_MR6_Msk                          (0x1UL << EXTI_EMR_MR6_Pos)         /*!< 0x00000040 */
#define EXTI_EMR_MR6                              EXTI_EMR_MR6_Msk                   /*!< Event Mask on line 6 */
#define EXTI_EMR_MR7_Pos                          (7U)
#define EXTI_EMR_MR7_Msk                          (0x1UL << EXTI_EMR_MR7_Pos)         /*!< 0x00000080 */
#define EXTI_EMR_MR7                              EXTI_EMR_MR7_Msk                   /*!< Event Mask on line 7 */
#define EXTI_EMR_MR8_Pos                          (8U)
#define EXTI_EMR_MR8_Msk                          (0x1UL << EXTI_EMR_MR8_Pos)         /*!< 0x00000100 */
#define EXTI_EMR_MR8                              EXTI_EMR_MR8_Msk                   /*!< Event Mask on line 8 */
#define EXTI_EMR_MR9_Pos                          (9U)
#define EXTI_EMR_MR9_Msk                          (0x1UL << EXTI_EMR_MR9_Pos)         /*!< 0x00000200 */
#define EXTI_EMR_MR9                              EXTI_EMR_MR9_Msk                   /*!< Event Mask on line 9 */
#define EXTI_EMR_MR10_Pos                         (10U)
#define EXTI_EMR_MR10_Msk                         (0x1UL << EXTI_EMR_MR10_Pos)        /*!< 0x00000400 */
#define EXTI_EMR_MR10                             EXTI_EMR_MR10_Msk                  /*!< Event Mask on line 10 */
#define EXTI_EMR_MR11_Pos                         (11U)
#define EXTI_EMR_MR11_Msk                         (0x1UL << EXTI_EMR_MR11_Pos)        /*!< 0x00000800 */
#define EXTI_EMR_MR11                             EXTI_EMR_MR11_Msk                  /*!< Event Mask on line 11 */
#define EXTI_EMR_MR12_Pos                         (12U)
#define EXTI_EMR_MR12_Msk                         (0x1UL << EXTI_EMR_MR12_Pos)        /*!< 0x00001000 */
#define EXTI_EMR_MR12                             EXTI_EMR_MR12_Msk                  /*!< Event Mask on line 12 */
#define EXTI_EMR_MR13_Pos                         (13U)
#define EXTI_EMR_MR13_Msk                         (0x1UL << EXTI_EMR_MR13_Pos)        /*!< 0x00002000 */
#define EXTI_EMR_MR13                             EXTI_EMR_MR13_Msk                  /*!< Event Mask on line 13 */
#define EXTI_EMR_MR14_Pos                         (14U)
#define EXTI_EMR_MR14_Msk                         (0x1UL << EXTI_EMR_MR14_Pos)        /*!< 0x00004000 */
#define EXTI_EMR_MR14                             EXTI_EMR_MR14_Msk                  /*!< Event Mask on line 14 */
#define EXTI_EMR_MR15_Pos                         (15U)
#define EXTI_EMR_MR15_Msk                         (0x1UL << EXTI_EMR_MR15_Pos)        /*!< 0x00008000 */
#define EXTI_EMR_MR15                             EXTI_EMR_MR15_Msk                  /*!< Event Mask on line 15 */
#define EXTI_EMR_MR16_Pos                         (16U)
#define EXTI_EMR_MR16_Msk                         (0x1UL << EXTI_EMR_MR16_Pos)        /*!< 0x00010000 */
#define EXTI_EMR_MR16                             EXTI_EMR_MR16_Msk                  /*!< Event Mask on line 16 */
#define EXTI_EMR_MR17_Pos                         (17U)
#define EXTI_EMR_MR17_Msk                         (0x1UL << EXTI_EMR_MR17_Pos)        /*!< 0x00020000 */
#define EXTI_EMR_MR17                             EXTI_EMR_MR17_Msk                  /*!< Event Mask on line 17 */

/* References Defines */
#define  EXTI_EMR_EM0 EXTI_EMR_MR0
#define  EXTI_EMR_EM1 EXTI_EMR_MR1
#define  EXTI_EMR_EM2 EXTI_EMR_MR2
#define  EXTI_EMR_EM3 EXTI_EMR_MR3
#define  EXTI_EMR_EM4 EXTI_EMR_MR4
#define  EXTI_EMR_EM5 EXTI_EMR_MR5
#define  EXTI_EMR_EM6 EXTI_EMR_MR6
#define  EXTI_EMR_EM7 EXTI_EMR_MR7
#define  EXTI_EMR_EM8 EXTI_EMR_MR8
#define  EXTI_EMR_EM9 EXTI_EMR_MR9
#define  EXTI_EMR_EM10 EXTI_EMR_MR10
#define  EXTI_EMR_EM11 EXTI_EMR_MR11
#define  EXTI_EMR_EM12 EXTI_EMR_MR12
#define  EXTI_EMR_EM13 EXTI_EMR_MR13
#define  EXTI_EMR_EM14 EXTI_EMR_MR14
#define  EXTI_EMR_EM15 EXTI_EMR_MR15
#define  EXTI_EMR_EM16 EXTI_EMR_MR16
#define  EXTI_EMR_EM17 EXTI_EMR_MR17

/******************  Bit definition for EXTI_RTSR register  *******************/
#define EXTI_RTSR_TR0_Pos                         (0U)
#define EXTI_RTSR_TR0_Msk                         (0x1UL << EXTI_RTSR_TR0_Pos)        /*!< 0x00000001 */
#define EXTI_RTSR_TR0                             EXTI_RTSR_TR0_Msk                  /*!< Rising trigger event configuration bit of line 0 */
#define EXTI_RTSR_TR1_Pos                         (1U)
#define EXTI_RTSR_TR1_Msk                         (0x1UL << EXTI_RTSR_TR1_Pos)        /*!< 0x00000002 */
#define EXTI_RTSR_TR1                             EXTI_RTSR_TR1_Msk                  /*!< Rising trigger event configuration bit of line 1 */
#define EXTI_RTSR_TR2_Pos                         (2U)
#define EXTI_RTSR_TR2_Msk                         (0x1UL << EXTI_RTSR_TR2_Pos)        /*!< 0x00000004 */
#define EXTI_RTSR_TR2                             EXTI_RTSR_TR2_Msk                  /*!< Rising trigger event configuration bit of line 2 */
#define EXTI_RTSR_TR3_Pos                         (3U)
#define EXTI_RTSR_TR3_Msk                         (0x1UL << EXTI_RTSR_TR3_Pos)        /*!< 0x00000008 */
#define EXTI_RTSR_TR3                             EXTI_RTSR_TR3_Msk                  /*!< Rising trigger event configuration bit of line 3 */
#define EXTI_RTSR_TR4_Pos                         (4U)
#define EXTI_RTSR_TR4_Msk                         (0x1UL << EXTI_RTSR_TR4_Pos)        /*!< 0x00000010 */
#define EXTI_RTSR_TR4                             EXTI_RTSR_TR4_Msk                  /*!< Rising trigger event configuration bit of line 4 */
#define EXTI_RTSR_TR5_Pos                         (5U)
#define EXTI_RTSR_TR5_Msk                         (0x1UL << EXTI_RTSR_TR5_Pos)        /*!< 0x00000020 */
#define EXTI_RTSR_TR5                             EXTI_RTSR_TR5_Msk                  /*!< Rising trigger event configuration bit of line 5 */
#define EXTI_RTSR_TR6_Pos                         (6U)
#define EXTI_RTSR_TR6_Msk                         (0x1UL << EXTI_RTSR_TR6_Pos)        /*!< 0x00000040 */
#define EXTI_RTSR_TR6                             EXTI_RTSR_TR6_Msk                  /*!< Rising trigger event configuration bit of line 6 */
#define EXTI_RTSR_TR7_Pos                         (7U)
#define EXTI_RTSR_TR7_Msk                         (0x1UL << EXTI_RTSR_TR7_Pos)        /*!< 0x00000080 */
#define EXTI_RTSR_TR7                             EXTI_RTSR_TR7_Msk                  /*!< Rising trigger event configuration bit of line 7 */
#define EXTI_RTSR_TR8_Pos                         (8U)
#define EXTI_RTSR_TR8_Msk                         (0x1UL << EXTI_RTSR_TR8_Pos)        /*!< 0x00000100 */
#define EXTI_RTSR_TR8                             EXTI_RTSR_TR8_Msk                  /*!< Rising trigger event configuration bit of line 8 */
#define EXTI_RTSR_TR9_Pos                         (9U)
#define EXTI_RTSR_TR9_Msk                         (0x1UL << EXTI_RTSR_TR9_Pos)        /*!< 0x00000200 */
#define EXTI_RTSR_TR9                             EXTI_RTSR_TR9_Msk                  /*!< Rising trigger event configuration bit of line 9 */
#define EXTI_RTSR_TR10_Pos                        (10U)
#define EXTI_RTSR_TR10_Msk                        (0x1UL << EXTI_RTSR_TR10_Pos)       /*!< 0x00000400 */
#define EXTI_RTSR_TR10                            EXTI_RTSR_TR10_Msk                 /*!< Rising trigger event configuration bit of line 10 */
#define EXTI_RTSR_TR11_Pos                        (11U)
#define EXTI_RTSR_TR11_Msk                        (0x1UL << EXTI_RTSR_TR11_Pos)       /*!< 0x00000800 */
#define EXTI_RTSR_TR11                            EXTI_RTSR_TR11_Msk                 /*!< Rising trigger event configuration bit of line 11 */
#define EXTI_RTSR_TR12_Pos                        (12U)
#define EXTI_RTSR_TR12_Msk                        (0x1UL << EXTI_RTSR_TR12_Pos)       /*!< 0x00001000 */
#define EXTI_RTSR_TR12                            EXTI_RTSR_TR12_Msk                 /*!< Rising trigger event configuration bit of line 12 */
#define EXTI_RTSR_TR13_Pos                        (13U)
#define EXTI_RTSR_TR13_Msk                        (0x1UL << EXTI_RTSR_TR13_Pos)       /*!< 0x00002000 */
#define EXTI_RTSR_TR13                            EXTI_RTSR_TR13_Msk                 /*!< Rising trigger event configuration bit of line 13 */
#define EXTI_RTSR_TR14_Pos                        (14U)
#define EXTI_RTSR_TR14_Msk                        (0x1UL << EXTI_RTSR_TR14_Pos)       /*!< 0x00004000 */
#define EXTI_RTSR_TR14                            EXTI_RTSR_TR14_Msk                 /*!< Rising trigger event configuration bit of line 14 */
#define EXTI_RTSR_TR15_Pos                        (15U)
#define EXTI_RTSR_TR15_Msk                        (0x1UL << EXTI_RTSR_TR15_Pos)       /*!< 0x00008000 */
#define EXTI_RTSR_TR15                            EXTI_RTSR_TR15_Msk                 /*!< Rising trigger event configuration bit of line 15 */
#define EXTI_RTSR_TR16_Pos                        (16U)
#define EXTI_RTSR_TR16_Msk                        (0x1UL << EXTI_RTSR_TR16_Pos)       /*!< 0x00010000 */
#define EXTI_RTSR_TR16                            EXTI_RTSR_TR16_Msk                 /*!< Rising trigger event configuration bit of line 16 */
#define EXTI_RTSR_TR17_Pos                        (17U)
#define EXTI_RTSR_TR17_Msk                        (0x1UL << EXTI_RTSR_TR17_Pos)       /*!< 0x00020000 */
#define EXTI_RTSR_TR17                            EXTI_RTSR_TR17_Msk                 /*!< Rising trigger event configuration bit of line 17 */

/* References Defines */
#define  EXTI_RTSR_RT0 EXTI_RTSR_TR0
#define  EXTI_RTSR_RT1 EXTI_RTSR_TR1
#define  EXTI_RTSR_RT2 EXTI_RTSR_TR2
#define  EXTI_RTSR_RT3 EXTI_RTSR_TR3
#define  EXTI_RTSR_RT4 EXTI_RTSR_TR4
#define  EXTI_RTSR_RT5 EXTI_RTSR_TR5
#define  EXTI_RTSR_RT6 EXTI_RTSR_TR6
#define  EXTI_RTSR_RT7 EXTI_RTSR_TR7
#define  EXTI_RTSR_RT8 EXTI_RTSR_TR8
#define  EXTI_RTSR_RT9 EXTI_RTSR_TR9
#define  EXTI_RTSR_RT10 EXTI_RTSR_TR10
#define  EXTI_RTSR_RT11 EXTI_RTSR_TR11
#define  EXTI_RTSR_RT12 EXTI_RTSR_TR12
#define  EXTI_RTSR_RT13 EXTI_RTSR_TR13
#define  EXTI_RTSR_RT14 EXTI_RTSR_TR14
#define  EXTI_RTSR_RT15 EXTI_RTSR_TR15
#define  EXTI_RTSR_RT16 EXTI_RTSR_TR16
#define  EXTI_RTSR_RT17 EXTI_RTSR_TR17

/******************  Bit definition for EXTI_FTSR register  *******************/
#define EXTI_FTSR_TR0_Pos                         (0U)
#define EXTI_FTSR_TR0_Msk                         (0x1UL << EXTI_FTSR_TR0_Pos)        /*!< 0x00000001 */
#define EXTI_FTSR_TR0                             EXTI_FTSR_TR0_Msk                  /*!< Falling trigger event configuration bit of line 0 */
#define EXTI_FTSR_TR1_Pos                         (1U)
#define EXTI_FTSR_TR1_Msk                         (0x1UL << EXTI_FTSR_TR1_Pos)        /*!< 0x00000002 */
#define EXTI_FTSR_TR1                             EXTI_FTSR_TR1_Msk                  /*!< Falling trigger event configuration bit of line 1 */
#define EXTI_FTSR_TR2_Pos                         (2U)
#define EXTI_FTSR_TR2_Msk                         (0x1UL << EXTI_FTSR_TR2_Pos)        /*!< 0x00000004 */
#define EXTI_FTSR_TR2                             EXTI_FTSR_TR2_Msk                  /*!< Falling trigger event configuration bit of line 2 */
#define EXTI_FTSR_TR3_Pos                         (3U)
#define EXTI_FTSR_TR3_Msk                         (0x1UL << EXTI_FTSR_TR3_Pos)        /*!< 0x00000008 */
#define EXTI_FTSR_TR3                             EXTI_FTSR_TR3_Msk                  /*!< Falling trigger event configuration bit of line 3 */
#define EXTI_FTSR_TR4_Pos                         (4U)
#define EXTI_FTSR_TR4_Msk                         (0x1UL << EXTI_FTSR_TR4_Pos)        /*!< 0x00000010 */
#define EXTI_FTSR_TR4                             EXTI_FTSR_TR4_Msk                  /*!< Falling trigger event configuration bit of line 4 */
#define EXTI_FTSR_TR5_Pos                         (5U)
#define EXTI_FTSR_TR5_Msk                         (0x1UL << EXTI_FTSR_TR5_Pos)        /*!< 0x00000020 */
#define EXTI_FTSR_TR5                             EXTI_FTSR_TR5_Msk                  /*!< Falling trigger event configuration bit of line 5 */
#define EXTI_FTSR_TR6_Pos                         (6U)
#define EXTI_FTSR_TR6_Msk                         (0x1UL << EXTI_FTSR_TR6_Pos)        /*!< 0x00000040 */
#define EXTI_FTSR_TR6                             EXTI_FTSR_TR6_Msk                  /*!< Falling trigger event configuration bit of line 6 */
#define EXTI_FTSR_TR7_Pos                         (7U)
#define EXTI_FTSR_TR7_Msk                         (0x1UL << EXTI_FTSR_TR7_Pos)        /*!< 0x00000080 */
#define EXTI_FTSR_TR7                             EXTI_FTSR_TR7_Msk                  /*!< Falling trigger event configuration bit of line 7 */
#define EXTI_FTSR_TR8_Pos                         (8U)
#define EXTI_FTSR_TR8_Msk                         (0x1UL << EXTI_FTSR_TR8_Pos)        /*!< 0x00000100 */
#define EXTI_FTSR_TR8                             EXTI_FTSR_TR8_Msk                  /*!< Falling trigger event configuration bit of line 8 */
#define EXTI_FTSR_TR9_Pos                         (9U)
#define EXTI_FTSR_TR9_Msk                         (0x1UL << EXTI_FTSR_TR9_Pos)        /*!< 0x00000200 */
#define EXTI_FTSR_TR9                             EXTI_FTSR_TR9_Msk                  /*!< Falling trigger event configuration bit of line 9 */
#define EXTI_FTSR_TR10_Pos                        (10U)
#define EXTI_FTSR_TR10_Msk                        (0x1UL << EXTI_FTSR_TR10_Pos)       /*!< 0x00000400 */
#define EXTI_FTSR_TR10                            EXTI_FTSR_TR10_Msk                 /*!< Falling trigger event configuration bit of line 10 */
#define EXTI_FTSR_TR11_Pos                        (11U)
#define EXTI_FTSR_TR11_Msk                        (0x1UL << EXTI_FTSR_TR11_Pos)       /*!< 0x00000800 */
#define EXTI_FTSR_TR11                            EXTI_FTSR_TR11_Msk                 /*!< Falling trigger event configuration bit of line 11 */
#define EXTI_FTSR_TR12_Pos                        (12U)
#define EXTI_FTSR_TR12_Msk                        (0x1UL << EXTI_FTSR_TR12_Pos)       /*!< 0x00001000 */
#define EXTI_FTSR_TR12                            EXTI_FTSR_TR12_Msk                 /*!< Falling trigger event configuration bit of line 12 */
#define EXTI_FTSR_TR13_Pos                        (13U)
#define EXTI_FTSR_TR13_Msk                        (0x1UL << EXTI_FTSR_TR13_Pos)       /*!< 0x00002000 */
#define EXTI_FTSR_TR13                            EXTI_FTSR_TR13_Msk                 /*!< Falling trigger event configuration bit of line 13 */
#define EXTI_FTSR_TR14_Pos                        (14U)
#define EXTI_FTSR_TR14_Msk                        (0x1UL << EXTI_FTSR_TR14_Pos)       /*!< 0x00004000 */
#define EXTI_FTSR_TR14                            EXTI_FTSR_TR14_Msk                 /*!< Falling trigger event configuration bit of line 14 */
#define EXTI_FTSR_TR15_Pos                        (15U)
#define EXTI_FTSR_TR15_Msk                        (0x1UL << EXTI_FTSR_TR15_Pos)       /*!< 0x00008000 */
#define EXTI_FTSR_TR15                            EXTI_FTSR_TR15_Msk                 /*!< Falling trigger event configuration bit of line 15 */
#define EXTI_FTSR_TR16_Pos                        (16U)
#define EXTI_FTSR_TR16_Msk                        (0x1UL << EXTI_FTSR_TR16_Pos)       /*!< 0x00010000 */
#define EXTI_FTSR_TR16                            EXTI_FTSR_TR16_Msk                 /*!< Falling trigger event configuration bit of line 16 */
#define EXTI_FTSR_TR17_Pos                        (17U)
#define EXTI_FTSR_TR17_Msk                        (0x1UL << EXTI_FTSR_TR17_Pos)       /*!< 0x00020000 */
#define EXTI_FTSR_TR17                            EXTI_FTSR_TR17_Msk                 /*!< Falling trigger event configuration bit of line 17 */

/* References Defines */
#define  EXTI_FTSR_FT0 EXTI_FTSR_TR0
#define  EXTI_FTSR_FT1 EXTI_FTSR_TR1
#define  EXTI_FTSR_FT2 EXTI_FTSR_TR2
#define  EXTI_FTSR_FT3 EXTI_FTSR_TR3
#define  EXTI_FTSR_FT4 EXTI_FTSR_TR4
#define  EXTI_FTSR_FT5 EXTI_FTSR_TR5
#define  EXTI_FTSR_FT6 EXTI_FTSR_TR6
#define  EXTI_FTSR_FT7 EXTI_FTSR_TR7
#define  EXTI_FTSR_FT8 EXTI_FTSR_TR8
#define  EXTI_FTSR_FT9 EXTI_FTSR_TR9
#define  EXTI_FTSR_FT10 EXTI_FTSR_TR10
#define  EXTI_FTSR_FT11 EXTI_FTSR_TR11
#define  EXTI_FTSR_FT12 EXTI_FTSR_TR12
#define  EXTI_FTSR_FT13 EXTI_FTSR_TR13
#define  EXTI_FTSR_FT14 EXTI_FTSR_TR14
#define  EXTI_FTSR_FT15 EXTI_FTSR_TR15
#define  EXTI_FTSR_FT16 EXTI_FTSR_TR16
#define  EXTI_FTSR_FT17 EXTI_FTSR_TR17

/******************  Bit definition for EXTI_SWIER register  ******************/
#define EXTI_SWIER_SWIER0_Pos                     (0U)
#define EXTI_SWIER_SWIER0_Msk                     (0x1UL << EXTI_SWIER_SWIER0_Pos)    /*!< 0x00000001 */
#define EXTI_SWIER_SWIER0                         EXTI_SWIER_SWIER0_Msk              /*!< Software Interrupt on line 0 */
#define EXTI_SWIER_SWIER1_Pos                     (1U)
#define EXTI_SWIER_SWIER1_Msk                     (0x1UL << EXTI_SWIER_SWIER1_Pos)    /*!< 0x00000002 */
#define EXTI_SWIER_SWIER1                         EXTI_SWIER_SWIER1_Msk              /*!< Software Interrupt on line 1 */
#define EXTI_SWIER_SWIER2_Pos                     (2U)
#define EXTI_SWIER_SWIER2_Msk                     (0x1UL << EXTI_SWIER_SWIER2_Pos)    /*!< 0x00000004 */
#define EXTI_SWIER_SWIER2                         EXTI_SWIER_SWIER2_Msk              /*!< Software Interrupt on line 2 */
#define EXTI_SWIER_SWIER3_Pos                     (3U)
#define EXTI_SWIER_SWIER3_Msk                     (0x1UL << EXTI_SWIER_SWIER3_Pos)    /*!< 0x00000008 */
#define EXTI_SWIER_SWIER3                         EXTI_SWIER_SWIER3_Msk              /*!< Software Interrupt on line 3 */
#define EXTI_SWIER_SWIER4_Pos                     (4U)
#define EXTI_SWIER_SWIER4_Msk                     (0x1UL << EXTI_SWIER_SWIER4_Pos)    /*!< 0x00000010 */
#define EXTI_SWIER_SWIER4                         EXTI_SWIER_SWIER4_Msk              /*!< Software Interrupt on line 4 */
#define EXTI_SWIER_SWIER5_Pos                     (5U)
#define EXTI_SWIER_SWIER5_Msk                     (0x1UL << EXTI_SWIER_SWIER5_Pos)    /*!< 0x00000020 */
#define EXTI_SWIER_SWIER5                         EXTI_SWIER_SWIER5_Msk              /*!< Software Interrupt on line 5 */
#define EXTI_SWIER_SWIER6_Pos                     (6U)
#define EXTI_SWIER_SWIER6_Msk                     (0x1UL << EXTI_SWIER_SWIER6_Pos)    /*!< 0x00000040 */
#define EXTI_SWIER_SWIER6                         EXTI_SWIER_SWIER6_Msk              /*!< Software Interrupt on line 6 */
#define EXTI_SWIER_SWIER7_Pos                     (7U)
#define EXTI_SWIER_SWIER7_Msk                     (0x1UL << EXTI_SWIER_SWIER7_Pos)    /*!< 0x00000080 */
#define EXTI_SWIER_SWIER7                         EXTI_SWIER_SWIER7_Msk              /*!< Software Interrupt on line 7 */
#define EXTI_SWIER_SWIER8_Pos                     (8U)
#define EXTI_SWIER_SWIER8_Msk                     (0x1UL << EXTI_SWIER_SWIER8_Pos)    /*!< 0x00000100 */
#define EXTI_SWIER_SWIER8                         EXTI_SWIER_SWIER8_Msk              /*!< Software Interrupt on line 8 */
#define EXTI_SWIER_SWIER9_Pos                     (9U)
#define EXTI_SWIER_SWIER9_Msk                     (0x1UL << EXTI_SWIER_SWIER9_Pos)    /*!< 0x00000200 */
#define EXTI_SWIER_SWIER9                         EXTI_SWIER_SWIER9_Msk              /*!< Software Interrupt on line 9 */
#define EXTI_SWIER_SWIER10_Pos                    (10U)
#define EXTI_SWIER_SWIER10_Msk                    (0x1UL << EXTI_SWIER_SWIER10_Pos)   /*!< 0x00000400 */
#define EXTI_SWIER_SWIER10                        EXTI_SWIER_SWIER10_Msk             /*!< Software Interrupt on line 10 */
#define EXTI_SWIER_SWIER11_Pos                    (11U)
#define EXTI_SWIER_SWIER11_Msk                    (0x1UL << EXTI_SWIER_SWIER11_Pos)   /*!< 0x00000800 */
#define EXTI_SWIER_SWIER11                        EXTI_SWIER_SWIER11_Msk             /*!< Software Interrupt on line 11 */
#define EXTI_SWIER_SWIER12_Pos                    (12U)
#define EXTI_SWIER_SWIER12_Msk                    (0x1UL << EXTI_SWIER_SWIER12_Pos)   /*!< 0x00001000 */
#define EXTI_SWIER_SWIER12                        EXTI_SWIER_SWIER12_Msk             /*!< Software Interrupt on line 12 */
#define EXTI_SWIER_SWIER13_Pos                    (13U)
#define EXTI_SWIER_SWIER13_Msk                    (0x1UL << EXTI_SWIER_SWIER13_Pos)   /*!< 0x00002000 */
#define EXTI_SWIER_SWIER13                        EXTI_SWIER_SWIER13_Msk             /*!< Software Interrupt on line 13 */
#define EXTI_SWIER_SWIER14_Pos                    (14U)
#define EXTI_SWIER_SWIER14_Msk                    (0x1UL << EXTI_SWIER_SWIER14_Pos)   /*!< 0x00004000 */
#define EXTI_SWIER_SWIER14                        EXTI_SWIER_SWIER14_Msk             /*!< Software Interrupt on line 14 */
#define EXTI_SWIER_SWIER15_Pos                    (15U)
#define EXTI_SWIER_SWIER15_Msk                    (0x1UL << EXTI_SWIER_SWIER15_Pos)   /*!< 0x00008000 */
#define EXTI_SWIER_SWIER15                        EXTI_SWIER_SWIER15_Msk             /*!< Software Interrupt on line 15 */
#define EXTI_SWIER_SWIER16_Pos                    (16U)
#define EXTI_SWIER_SWIER16_Msk                    (0x1UL << EXTI_SWIER_SWIER16_Pos)   /*!< 0x00010000 */
#define EXTI_SWIER_SWIER16                        EXTI_SWIER_SWIER16_Msk             /*!< Software Interrupt on line 16 */
#define EXTI_SWIER_SWIER17_Pos                    (17U)
#define EXTI_SWIER_SWIER17_Msk                    (0x1UL << EXTI_SWIER_SWIER17_Pos)   /*!< 0x00020000 */
#define EXTI_SWIER_SWIER17                        EXTI_SWIER_SWIER17_Msk             /*!< Software Interrupt on line 17 */

/* References Defines */
#define  EXTI_SWIER_SWI0 EXTI_SWIER_SWIER0
#define  EXTI_SWIER_SWI1 EXTI_SWIER_SWIER1
#define  EXTI_SWIER_SWI2 EXTI_SWIER_SWIER2
#define  EXTI_SWIER_SWI3 EXTI_SWIER_SWIER3
#define  EXTI_SWIER_SWI4 EXTI_SWIER_SWIER4
#define  EXTI_SWIER_SWI5 EXTI_SWIER_SWIER5
#define  EXTI_SWIER_SWI6 EXTI_SWIER_SWIER6
#define  EXTI_SWIER_SWI7 EXTI_SWIER_SWIER7
#define  EXTI_SWIER_SWI8 EXTI_SWIER_SWIER8
#define  EXTI_SWIER_SWI9 EXTI_SWIER_SWIER9
#define  EXTI_SWIER_SWI10 EXTI_SWIER_SWIER10
#define  EXTI_SWIER_SWI11 EXTI_SWIER_SWIER11
#define  EXTI_SWIER_SWI12 EXTI_SWIER_SWIER12
#define  EXTI_SWIER_SWI13 EXTI_SWIER_SWIER13
#define  EXTI_SWIER_SWI14 EXTI_SWIER_SWIER14
#define  EXTI_SWIER_SWI15 EXTI_SWIER_SWIER15
#define  EXTI_SWIER_SWI16 EXTI_SWIER_SWIER16
#define  EXTI_SWIER_SWI17 EXTI_SWIER_SWIER17

/*******************  Bit definition for EXTI_PR register  ********************/
#define EXTI_PR_PR0_Pos                           (0U)
#define EXTI_PR_PR0_Msk                           (0x1UL << EXTI_PR_PR0_Pos)          /*!< 0x00000001 */
#define EXTI_PR_PR0                               EXTI_PR_PR0_Msk                    /*!< Pending bit for line 0 */
#define EXTI_PR_PR1_Pos                           (1U)
#define EXTI_PR_PR1_Msk                           (0x1UL << EXTI_PR_PR1_Pos)          /*!< 0x00000002 */
#define EXTI_PR_PR1                               EXTI_PR_PR1_Msk                    /*!< Pending bit for line 1 */
#define EXTI_PR_PR2_Pos                           (2U)
#define EXTI_PR_PR2_Msk                           (0x1UL << EXTI_PR_PR2_Pos)          /*!< 0x00000004 */
#define EXTI_PR_PR2                               EXTI_PR_PR2_Msk                    /*!< Pending bit for line 2 */
#define EXTI_PR_PR3_Pos                           (3U)
#define EXTI_PR_PR3_Msk                           (0x1UL << EXTI_PR_PR3_Pos)          /*!< 0x00000008 */
#define EXTI_PR_PR3                               EXTI_PR_PR3_Msk                    /*!< Pending bit for line 3 */
#define EXTI_PR_PR4_Pos                           (4U)
#define EXTI_PR_PR4_Msk                           (0x1UL << EXTI_PR_PR4_Pos)          /*!< 0x00000010 */
#define EXTI_PR_PR4                               EXTI_PR_PR4_Msk                    /*!< Pending bit for line 4 */
#define EXTI_PR_PR5_Pos                           (5U)
#define EXTI_PR_PR5_Msk                           (0x1UL << EXTI_PR_PR5_Pos)          /*!< 0x00000020 */
#define EXTI_PR_PR5                               EXTI_PR_PR5_Msk                    /*!< Pending bit for line 5 */
#define EXTI_PR_PR6_Pos                           (6U)
#define EXTI_PR_PR6_Msk                           (0x1UL << EXTI_PR_PR6_Pos)          /*!< 0x00000040 */
#define EXTI_PR_PR6                               EXTI_PR_PR6_Msk                    /*!< Pending bit for line 6 */
#define EXTI_PR_PR7_Pos                           (7U)
#define EXTI_PR_PR7_Msk                           (0x1UL << EXTI_PR_PR7_Pos)          /*!< 0x00000080 */
#define EXTI_PR_PR7                               EXTI_PR_PR7_Msk                    /*!< Pending bit for line 7 */
#define EXTI_PR_PR8_Pos                           (8U)
#define EXTI_PR_PR8_Msk                           (0x1UL << EXTI_PR_PR8_Pos)          /*!< 0x00000100 */
#define EXTI_PR_PR8                               EXTI_PR_PR8_Msk                    /*!< Pending bit for line 8 */
#define EXTI_PR_PR9_Pos                           (9U)
#define EXTI_PR_PR9_Msk                           (0x1UL << EXTI_PR_PR9_Pos)          /*!< 0x00000200 */
#define EXTI_PR_PR9                               EXTI_PR_PR9_Msk                    /*!< Pending bit for line 9 */
#define EXTI_PR_PR10_Pos                          (10U)
#define EXTI_PR_PR10_Msk                          (0x1UL << EXTI_PR_PR10_Pos)         /*!< 0x00000400 */
#define EXTI_PR_PR10                              EXTI_PR_PR10_Msk                   /*!< Pending bit for line 10 */
#define EXTI_PR_PR11_Pos                          (11U)
#define EXTI_PR_PR11_Msk                          (0x1UL << EXTI_PR_PR11_Pos)         /*!< 0x00000800 */
#define EXTI_PR_PR11                              EXTI_PR_PR11_Msk                   /*!< Pending bit for line 11 */
#define EXTI_PR_PR12_Pos                          (12U)
#define EXTI_PR_PR12_Msk                          (0x1UL << EXTI_PR_PR12_Pos)         /*!< 0x00001000 */
#define EXTI_PR_PR12                              EXTI_PR_PR12_Msk                   /*!< Pending bit for line 12 */
#define EXTI_PR_PR13_Pos                          (13U)
#define EXTI_PR_PR13_Msk                          (0x1UL << EXTI_PR_PR13_Pos)         /*!< 0x00002000 */
#define EXTI_PR_PR13                              EXTI_PR_PR13_Msk                   /*!< Pending bit for line 13 */
#define EXTI_PR_PR14_Pos                          (14U)
#define EXTI_PR_PR14_Msk                          (0x1UL << EXTI_PR_PR14_Pos)         /*!< 0x00004000 */
#define EXTI_PR_PR14                              EXTI_PR_PR14_Msk                   /*!< Pending bit for line 14 */
#define EXTI_PR_PR15_Pos                          (15U)
#define EXTI_PR_PR15_Msk                          (0x1UL << EXTI_PR_PR15_Pos)         /*!< 0x00008000 */
#define EXTI_PR_PR15                              EXTI_PR_PR15_Msk                   /*!< Pending bit for line 15 */
#define EXTI_PR_PR16_Pos                          (16U)
#define EXTI_PR_PR16_Msk                          (0x1UL << EXTI_PR_PR16_Pos)         /*!< 0x00010000 */
#define EXTI_PR_PR16                              EXTI_PR_PR16_Msk                   /*!< Pending bit for line 16 */
#define EXTI_PR_PR17_Pos                          (17U)
#define EXTI_PR_PR17_Msk                          (0x1UL << EXTI_PR_PR17_Pos)         /*!< 0x00020000 */
#define EXTI_PR_PR17                              EXTI_PR_PR17_Msk                   /*!< Pending bit for line 17 */

/* References Defines */
#define  EXTI_PR_PIF0 EXTI_PR_PR0
#define  EXTI_PR_PIF1 EXTI_PR_PR1
#define  EXTI_PR_PIF2 EXTI_PR_PR2
#define  EXTI_PR_PIF3 EXTI_PR_PR3
#define  EXTI_PR_PIF4 EXTI_PR_PR4
#define  EXTI_PR_PIF5 EXTI_PR_PR5
#define  EXTI_PR_PIF6 EXTI_PR_PR6
#define  EXTI_PR_PIF7 EXTI_PR_PR7
#define  EXTI_PR_PIF8 EXTI_PR_PR8
#define  EXTI_PR_PIF9 EXTI_PR_PR9
#define  EXTI_PR_PIF10 EXTI_PR_PR10
#define  EXTI_PR_PIF11 EXTI_PR_PR11
#define  EXTI_PR_PIF12 EXTI_PR_PR12
#define  EXTI_PR_PIF13 EXTI_PR_PR13
#define  EXTI_PR_PIF14 EXTI_PR_PR14
#define  EXTI_PR_PIF15 EXTI_PR_PR15
#define  EXTI_PR_PIF16 EXTI_PR_PR16
#define  EXTI_PR_PIF17 EXTI_PR_PR17

/*********************  Bits Define For Peripheral FLASH  *********************/
/*!< FLASH_ACR */
#define FLASH_ACR_LATENCY_Pos                     (0U)
#define FLASH_ACR_LATENCY_Msk                     (0xFUL << FLASH_ACR_LATENCY_Pos)                  /*!< 0x0000000F */
#define FLASH_ACR_LATENCY                         FLASH_ACR_LATENCY_Msk                             /*!< LATENCY[3:0] bits (desc LATENCY) */
#define FLASH_ACR_LATENCY_0                       (0x1UL << FLASH_ACR_LATENCY_Pos)                  /*!< 0x00000001 */
#define FLASH_ACR_LATENCY_1                       (0x2UL << FLASH_ACR_LATENCY_Pos)                  /*!< 0x00000002 */
#define FLASH_ACR_LATENCY_2                       (0x4UL << FLASH_ACR_LATENCY_Pos)                  /*!< 0x00000004 */
#define FLASH_ACR_LATENCY_3                       (0x8UL << FLASH_ACR_LATENCY_Pos)                  /*!< 0x00000008 */

/*!< FLASH_KEYR */
#define FLASH_KEYR_KEY_Pos                        (0U)
#define FLASH_KEYR_KEY_Msk                        (0xFFFFFFFFUL << FLASH_KEYR_KEY_Pos)              /*!< 0xFFFFFFFF */
#define FLASH_KEYR_KEY                            FLASH_KEYR_KEY_Msk                                /*!< KEY[31:0] bits (desc KEY) */

/*!< FLASH_OPTKEYR */
#define FLASH_OPTKEYR_OPTKEY_Pos                  (0U)
#define FLASH_OPTKEYR_OPTKEY_Msk                  (0xFFFFFFFFUL << FLASH_OPTKEYR_OPTKEY_Pos)        /*!< 0xFFFFFFFF */
#define FLASH_OPTKEYR_OPTKEY                      FLASH_OPTKEYR_OPTKEY_Msk                          /*!< OPTKEY[31:0] bits (desc OPTKEY) */

/*!< FLASH_SR */
#define FLASH_SR_EOP_Pos                          (0U)
#define FLASH_SR_EOP_Msk                          (0x1UL << FLASH_SR_EOP_Pos)                       /*!< 0x00000001 */
#define FLASH_SR_EOP                              FLASH_SR_EOP_Msk                                  /*!< desc EOP */
#define FLASH_SR_WRPERR_Pos                       (4U)
#define FLASH_SR_WRPERR_Msk                       (0x1UL << FLASH_SR_WRPERR_Pos)                    /*!< 0x00000010 */
#define FLASH_SR_WRPERR                           FLASH_SR_WRPERR_Msk                               /*!< desc WRPERR */
#define FLASH_SR_OPTVERR_Pos                      (15U)
#define FLASH_SR_OPTVERR_Msk                      (0x1UL << FLASH_SR_OPTVERR_Pos)                   /*!< 0x00008000 */
#define FLASH_SR_OPTVERR                          FLASH_SR_OPTVERR_Msk                              /*!< desc OPTVERR */
#define FLASH_SR_BSY_Pos                          (16U)
#define FLASH_SR_BSY_Msk                          (0x1UL << FLASH_SR_BSY_Pos)                       /*!< 0x00010000 */
#define FLASH_SR_BSY                              FLASH_SR_BSY_Msk                                  /*!< desc BSY */

/*!< FLASH_CR */
#define FLASH_CR_PG_Pos                           (0U)
#define FLASH_CR_PG_Msk                           (0x1UL << FLASH_CR_PG_Pos)                        /*!< 0x00000001 */
#define FLASH_CR_PG                               FLASH_CR_PG_Msk                                   /*!< desc PG */
#define FLASH_CR_PER_Pos                          (1U)
#define FLASH_CR_PER_Msk                          (0x1UL << FLASH_CR_PER_Pos)                       /*!< 0x00000002 */
#define FLASH_CR_PER                              FLASH_CR_PER_Msk                                  /*!< desc PER */
#define FLASH_CR_MER_Pos                          (2U)
#define FLASH_CR_MER_Msk                          (0x1UL << FLASH_CR_MER_Pos)                       /*!< 0x00000004 */
#define FLASH_CR_MER                              FLASH_CR_MER_Msk                                  /*!< desc MER */
#define FLASH_CR_SER_Pos                          (11U)
#define FLASH_CR_SER_Msk                          (0x1UL << FLASH_CR_SER_Pos)                       /*!< 0x00000800 */
#define FLASH_CR_SER                              FLASH_CR_SER_Msk                                  /*!< desc SER */
#define FLASH_CR_BER_Pos                          (12U)
#define FLASH_CR_BER_Msk                          (0x1UL << FLASH_CR_BER_Pos)                       /*!< 0x00001000 */
#define FLASH_CR_BER                              FLASH_CR_BER_Msk                                  /*!< desc BER */
#define FLASH_CR_OPTSTRT_Pos                      (17U)
#define FLASH_CR_OPTSTRT_Msk                      (0x1UL << FLASH_CR_OPTSTRT_Pos)                   /*!< 0x00020000 */
#define FLASH_CR_OPTSTRT                          FLASH_CR_OPTSTRT_Msk                              /*!< desc OPTSTRT */
#define FLASH_CR_PGSTRT_Pos                       (19U)
#define FLASH_CR_PGSTRT_Msk                       (0x1UL << FLASH_CR_PGSTRT_Pos)                    /*!< 0x00080000 */
#define FLASH_CR_PGSTRT                           FLASH_CR_PGSTRT_Msk                               /*!< desc PGSTRT */
#define FLASH_CR_EOPIE_Pos                        (24U)
#define FLASH_CR_EOPIE_Msk                        (0x1UL << FLASH_CR_EOPIE_Pos)                     /*!< 0x01000000 */
#define FLASH_CR_EOPIE                            FLASH_CR_EOPIE_Msk                                /*!< desc EOPIE */
#define FLASH_CR_ERRIE_Pos                        (25U)
#define FLASH_CR_ERRIE_Msk                        (0x1UL << FLASH_CR_ERRIE_Pos)                     /*!< 0x02000000 */
#define FLASH_CR_ERRIE                            FLASH_CR_ERRIE_Msk                                /*!< desc ERRIE */
#define FLASH_CR_OBL_LAUNCH_Pos                   (27U)
#define FLASH_CR_OBL_LAUNCH_Msk                   (0x1UL << FLASH_CR_OBL_LAUNCH_Pos)                /*!< 0x08000000 */
#define FLASH_CR_OBL_LAUNCH                       FLASH_CR_OBL_LAUNCH_Msk                           /*!< desc OBL_LAUNCH */
#define FLASH_CR_OPTLOCK_Pos                      (30U)
#define FLASH_CR_OPTLOCK_Msk                      (0x1UL << FLASH_CR_OPTLOCK_Pos)                   /*!< 0x40000000 */
#define FLASH_CR_OPTLOCK                          FLASH_CR_OPTLOCK_Msk                              /*!< desc OPTLOCK */
#define FLASH_CR_LOCK_Pos                         (31U)
#define FLASH_CR_LOCK_Msk                         (0x1UL << FLASH_CR_LOCK_Pos)                      /*!< 0x80000000 */
#define FLASH_CR_LOCK                             FLASH_CR_LOCK_Msk                                 /*!< desc LOCK */

/*!< FLASH_OPTR */
#define FLASH_OPTR_RDP_Pos                        (0U)
#define FLASH_OPTR_RDP_Msk                        (0xFFUL << FLASH_OPTR_RDP_Pos)                    /*!< 0x000000FF */
#define FLASH_OPTR_RDP                            FLASH_OPTR_RDP_Msk                                /*!< RDP[7:0] bits (desc RDP) */

#define FLASH_OPTR_IWDG_SW_Pos                    (12U)
#define FLASH_OPTR_IWDG_SW_Msk                    (0x1UL << FLASH_OPTR_IWDG_SW_Pos)                 /*!< 0x00001000 */
#define FLASH_OPTR_IWDG_SW                        FLASH_OPTR_IWDG_SW_Msk                            /*!< desc IWDG_SW */

#define FLASH_OPTR_NRST_STOP_Pos                  (13U)
#define FLASH_OPTR_NRST_STOP_Msk                  (0x1UL << FLASH_OPTR_NRST_STOP_Pos)               /*!< 0x00002000 */
#define FLASH_OPTR_NRST_STOP                      FLASH_OPTR_NRST_STOP_Msk                          /*!< desc NRST_STOP */

#define FLASH_OPTR_NRST_STDBY_Pos                 (14U)
#define FLASH_OPTR_NRST_STDBY_Msk                 (0x1UL << FLASH_OPTR_NRST_STDBY_Pos)              /*!< 0x00004000 */
#define FLASH_OPTR_NRST_STDBY                     FLASH_OPTR_NRST_STDBY_Msk                         /*!< desc NRST_STDBY */

/*!< FLASH_SDKR */
// #define FLASH_SDKR_SDK_STRT_Pos                   (0U)
// #define FLASH_SDKR_SDK_STRT_Msk                   (0x3FUL << FLASH_SDKR_SDK_STRT_Pos)               /*!< 0x0000003F */
// #define FLASH_SDKR_SDK_STRT                       FLASH_SDKR_SDK_STRT_Msk                           /*!< SDK_STRT[5:0] bits (desc SDK_STRT) */
// #define FLASH_SDKR_SDK_STRT_0                     (0x1UL << FLASH_SDKR_SDK_STRT_Pos)                /*!< 0x00000001 */
// #define FLASH_SDKR_SDK_STRT_1                     (0x2UL << FLASH_SDKR_SDK_STRT_Pos)                /*!< 0x00000002 */
// #define FLASH_SDKR_SDK_STRT_2                     (0x4UL << FLASH_SDKR_SDK_STRT_Pos)                /*!< 0x00000004 */
// #define FLASH_SDKR_SDK_STRT_3                     (0x8UL << FLASH_SDKR_SDK_STRT_Pos)                /*!< 0x00000008 */
// #define FLASH_SDKR_SDK_STRT_4                     (0x10UL << FLASH_SDKR_SDK_STRT_Pos)               /*!< 0x00000010 */
// #define FLASH_SDKR_SDK_STRT_5                     (0x20UL << FLASH_SDKR_SDK_STRT_Pos)               /*!< 0x00000020 */

// #define FLASH_SDKR_SDK_END_Pos                    (8U)
// #define FLASH_SDKR_SDK_END_Msk                    (0x3FUL << FLASH_SDKR_SDK_END_Pos)                /*!< 0x00003F00 */
// #define FLASH_SDKR_SDK_END                        FLASH_SDKR_SDK_END_Msk                            /*!< SDK_END[13:8] bits (desc SDK_END) */
// #define FLASH_SDKR_SDK_END_0                      (0x1UL << FLASH_SDKR_SDK_END_Pos)                 /*!< 0x00000100 */
// #define FLASH_SDKR_SDK_END_1                      (0x2UL << FLASH_SDKR_SDK_END_Pos)                 /*!< 0x00000200 */
// #define FLASH_SDKR_SDK_END_2                      (0x4UL << FLASH_SDKR_SDK_END_Pos)                 /*!< 0x00000400 */
// #define FLASH_SDKR_SDK_END_3                      (0x8UL << FLASH_SDKR_SDK_END_Pos)                 /*!< 0x00000800 */
// #define FLASH_SDKR_SDK_END_4                      (0x10UL << FLASH_SDKR_SDK_END_Pos)                /*!< 0x00001000 */
// #define FLASH_SDKR_SDK_END_5                      (0x20UL << FLASH_SDKR_SDK_END_Pos)                /*!< 0x00002000 */


/*!< FLASH_WRPR */
#define FLASH_WRPR_WRP_Pos                        (0U)
#define FLASH_WRPR_WRP_Msk                        (0xFFFUL << FLASH_WRPR_WRP_Pos)                   /*!< 0x00000FFF */
#define FLASH_WRPR_WRP                            FLASH_WRPR_WRP_Msk                                /*!< WRP[11:0] bits (desc WRP) */
#define FLASH_WRPR_WRP_0                          (0x0001UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_1                          (0x0002UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_2                          (0x0004UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_3                          (0x0008UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_4                          (0x0010UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_5                          (0x0020UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_6                          (0x0040UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_7                          (0x0080UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_8                          (0x0100UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_9                          (0x0200UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_10                         (0x0400UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_11                         (0x0800UL << FLASH_WRPR_WRP_Pos)

#define GPIO_NRST_CONFIG_SUPPORT         /*!< GPIO feature available only on specific devices: Configure NRST pin */
#define FLASH_SECURABLE_MEMORY_SUPPORT   /*!< Flash feature available only on specific devices: allow to secure memory */
#define FLASH_PCROP_SUPPORT              /*!< Flash feature available only on specific devices: proprietary code read protection areas selected by option */

/******************  FLASH Keys  **********************************************/
#define FLASH_KEY1_Pos                            (0U)
#define FLASH_KEY1_Msk                            (0x45670123UL << FLASH_KEY1_Pos)                  /*!< 0x45670123 */
#define FLASH_KEY1                                FLASH_KEY1_Msk                                    /*!< Flash program erase key1 */
#define FLASH_KEY2_Pos                            (0U)
#define FLASH_KEY2_Msk                            (0xCDEF89ABUL << FLASH_KEY2_Pos)                  /*!< 0xCDEF89AB */
#define FLASH_KEY2                                FLASH_KEY2_Msk                                    /*!< Flash program erase key2: used with FLASH_PEKEY1
to unlock the write access to the FPEC. */

#define FLASH_OPTKEY1_Pos                         (0U)
#define FLASH_OPTKEY1_Msk                         (0x08192A3BUL << FLASH_OPTKEY1_Pos)               /*!< 0x08192A3B */
#define FLASH_OPTKEY1                             FLASH_OPTKEY1_Msk                                 /*!< Flash option key1 */
#define FLASH_OPTKEY2_Pos                         (0U)
#define FLASH_OPTKEY2_Msk                         (0x4C5D6E7FUL << FLASH_OPTKEY2_Pos)               /*!< 0x4C5D6E7F */
#define FLASH_OPTKEY2                             FLASH_OPTKEY2_Msk                                 /*!< Flash option key2: used with FLASH_OPTKEY1 to
unlock the write access to the option byte block */
/*********************  Bits Define For Peripheral GPIO  *********************/
/*!< GPIO_MODER */
#define GPIO_MODER_MODE0_Pos                      (0U)
#define GPIO_MODER_MODE0_Msk                      (0x3UL << GPIO_MODER_MODE0_Pos)                   /*!< 0x00000003 */
#define GPIO_MODER_MODE0                          GPIO_MODER_MODE0_Msk                              /*!< MODE0[1:0] bits (desc MODE0) */
#define GPIO_MODER_MODE0_0                        (0x1UL << GPIO_MODER_MODE0_Pos)                   /*!< 0x00000001 */
#define GPIO_MODER_MODE0_1                        (0x2UL << GPIO_MODER_MODE0_Pos)                   /*!< 0x00000002 */

#define GPIO_MODER_MODE1_Pos                      (2U)
#define GPIO_MODER_MODE1_Msk                      (0x3UL << GPIO_MODER_MODE1_Pos)                   /*!< 0x0000000C */
#define GPIO_MODER_MODE1                          GPIO_MODER_MODE1_Msk                              /*!< MODE1[3:2] bits (desc MODE1) */
#define GPIO_MODER_MODE1_0                        (0x1UL << GPIO_MODER_MODE1_Pos)                   /*!< 0x00000004 */
#define GPIO_MODER_MODE1_1                        (0x2UL << GPIO_MODER_MODE1_Pos)                   /*!< 0x00000008 */

#define GPIO_MODER_MODE2_Pos                      (4U)
#define GPIO_MODER_MODE2_Msk                      (0x3UL << GPIO_MODER_MODE2_Pos)                   /*!< 0x00000030 */
#define GPIO_MODER_MODE2                          GPIO_MODER_MODE2_Msk                              /*!< MODE2[5:4] bits (desc MODE2) */
#define GPIO_MODER_MODE2_0                        (0x1UL << GPIO_MODER_MODE2_Pos)                   /*!< 0x00000010 */
#define GPIO_MODER_MODE2_1                        (0x2UL << GPIO_MODER_MODE2_Pos)                   /*!< 0x00000020 */

#define GPIO_MODER_MODE3_Pos                      (6U)
#define GPIO_MODER_MODE3_Msk                      (0x3UL << GPIO_MODER_MODE3_Pos)                   /*!< 0x000000C0 */
#define GPIO_MODER_MODE3                          GPIO_MODER_MODE3_Msk                              /*!< MODE3[7:6] bits (desc MODE3) */
#define GPIO_MODER_MODE3_0                        (0x1UL << GPIO_MODER_MODE3_Pos)                   /*!< 0x00000040 */
#define GPIO_MODER_MODE3_1                        (0x2UL << GPIO_MODER_MODE3_Pos)                   /*!< 0x00000080 */

#define GPIO_MODER_MODE4_Pos                      (8U)
#define GPIO_MODER_MODE4_Msk                      (0x3UL << GPIO_MODER_MODE4_Pos)                   /*!< 0x00000300 */
#define GPIO_MODER_MODE4                          GPIO_MODER_MODE4_Msk                              /*!< MODE4[9:8] bits (desc MODE4) */
#define GPIO_MODER_MODE4_0                        (0x1UL << GPIO_MODER_MODE4_Pos)                   /*!< 0x00000100 */
#define GPIO_MODER_MODE4_1                        (0x2UL << GPIO_MODER_MODE4_Pos)                   /*!< 0x00000200 */

#define GPIO_MODER_MODE5_Pos                      (10U)
#define GPIO_MODER_MODE5_Msk                      (0x3UL << GPIO_MODER_MODE5_Pos)                   /*!< 0x00000C00 */
#define GPIO_MODER_MODE5                          GPIO_MODER_MODE5_Msk                              /*!< MODE5[11:10] bits (desc MODE5) */
#define GPIO_MODER_MODE5_0                        (0x1UL << GPIO_MODER_MODE5_Pos)                   /*!< 0x00000400 */
#define GPIO_MODER_MODE5_1                        (0x2UL << GPIO_MODER_MODE5_Pos)                   /*!< 0x00000800 */

#define GPIO_MODER_MODE6_Pos                      (12U)
#define GPIO_MODER_MODE6_Msk                      (0x3UL << GPIO_MODER_MODE6_Pos)                   /*!< 0x00003000 */
#define GPIO_MODER_MODE6                          GPIO_MODER_MODE6_Msk                              /*!< MODE6[13:12] bits (desc MODE6) */
#define GPIO_MODER_MODE6_0                        (0x1UL << GPIO_MODER_MODE6_Pos)                   /*!< 0x00001000 */
#define GPIO_MODER_MODE6_1                        (0x2UL << GPIO_MODER_MODE6_Pos)                   /*!< 0x00002000 */

#define GPIO_MODER_MODE7_Pos                      (14U)
#define GPIO_MODER_MODE7_Msk                      (0x3UL << GPIO_MODER_MODE7_Pos)                   /*!< 0x0000C000 */
#define GPIO_MODER_MODE7                          GPIO_MODER_MODE7_Msk                              /*!< MODE7[15:14] bits (desc MODE7) */
#define GPIO_MODER_MODE7_0                        (0x1UL << GPIO_MODER_MODE7_Pos)                   /*!< 0x00004000 */
#define GPIO_MODER_MODE7_1                        (0x2UL << GPIO_MODER_MODE7_Pos)                   /*!< 0x00008000 */

#define GPIO_MODER_MODE8_Pos                      (16U)
#define GPIO_MODER_MODE8_Msk                      (0x3UL << GPIO_MODER_MODE8_Pos)                   /*!< 0x00030000 */
#define GPIO_MODER_MODE8                          GPIO_MODER_MODE8_Msk                              /*!< MODE8[17:16] bits (desc MODE8) */
#define GPIO_MODER_MODE8_0                        (0x1UL << GPIO_MODER_MODE8_Pos)                   /*!< 0x00010000 */
#define GPIO_MODER_MODE8_1                        (0x2UL << GPIO_MODER_MODE8_Pos)                   /*!< 0x00020000 */

#define GPIO_MODER_MODE9_Pos                      (18U)
#define GPIO_MODER_MODE9_Msk                      (0x3UL << GPIO_MODER_MODE9_Pos)                   /*!< 0x000C0000 */
#define GPIO_MODER_MODE9                          GPIO_MODER_MODE9_Msk                              /*!< MODE9[19:18] bits (desc MODE9) */
#define GPIO_MODER_MODE9_0                        (0x1UL << GPIO_MODER_MODE9_Pos)                   /*!< 0x00040000 */
#define GPIO_MODER_MODE9_1                        (0x2UL << GPIO_MODER_MODE9_Pos)                   /*!< 0x00080000 */

#define GPIO_MODER_MODE10_Pos                     (20U)
#define GPIO_MODER_MODE10_Msk                     (0x3UL << GPIO_MODER_MODE10_Pos)                  /*!< 0x00300000 */
#define GPIO_MODER_MODE10                         GPIO_MODER_MODE10_Msk                             /*!< MODE10[21:20] bits (desc MODE10) */
#define GPIO_MODER_MODE10_0                       (0x1UL << GPIO_MODER_MODE10_Pos)                  /*!< 0x00100000 */
#define GPIO_MODER_MODE10_1                       (0x2UL << GPIO_MODER_MODE10_Pos)                  /*!< 0x00200000 */

#define GPIO_MODER_MODE11_Pos                     (22U)
#define GPIO_MODER_MODE11_Msk                     (0x3UL << GPIO_MODER_MODE11_Pos)                  /*!< 0x00C00000 */
#define GPIO_MODER_MODE11                         GPIO_MODER_MODE11_Msk                             /*!< MODE11[23:22] bits (desc MODE11) */
#define GPIO_MODER_MODE11_0                       (0x1UL << GPIO_MODER_MODE11_Pos)                  /*!< 0x00400000 */
#define GPIO_MODER_MODE11_1                       (0x2UL << GPIO_MODER_MODE11_Pos)                  /*!< 0x00800000 */

#define GPIO_MODER_MODE12_Pos                     (24U)
#define GPIO_MODER_MODE12_Msk                     (0x3UL << GPIO_MODER_MODE12_Pos)                  /*!< 0x03000000 */
#define GPIO_MODER_MODE12                         GPIO_MODER_MODE12_Msk                             /*!< MODE12[25:24] bits (desc MODE12) */
#define GPIO_MODER_MODE12_0                       (0x1UL << GPIO_MODER_MODE12_Pos)                  /*!< 0x01000000 */
#define GPIO_MODER_MODE12_1                       (0x2UL << GPIO_MODER_MODE12_Pos)                  /*!< 0x02000000 */

#define GPIO_MODER_MODE13_Pos                     (26U)
#define GPIO_MODER_MODE13_Msk                     (0x3UL << GPIO_MODER_MODE13_Pos)                  /*!< 0x0C000000 */
#define GPIO_MODER_MODE13                         GPIO_MODER_MODE13_Msk                             /*!< MODE13[27:26] bits (desc MODE13) */
#define GPIO_MODER_MODE13_0                       (0x1UL << GPIO_MODER_MODE13_Pos)                  /*!< 0x04000000 */
#define GPIO_MODER_MODE13_1                       (0x2UL << GPIO_MODER_MODE13_Pos)                  /*!< 0x08000000 */

#define GPIO_MODER_MODE14_Pos                     (28U)
#define GPIO_MODER_MODE14_Msk                     (0x3UL << GPIO_MODER_MODE14_Pos)                  /*!< 0x30000000 */
#define GPIO_MODER_MODE14                         GPIO_MODER_MODE14_Msk                             /*!< MODE14[29:28] bits (desc MODE14) */
#define GPIO_MODER_MODE14_0                       (0x1UL << GPIO_MODER_MODE14_Pos)                  /*!< 0x10000000 */
#define GPIO_MODER_MODE14_1                       (0x2UL << GPIO_MODER_MODE14_Pos)                  /*!< 0x20000000 */

#define GPIO_MODER_MODE15_Pos                     (30U)
#define GPIO_MODER_MODE15_Msk                     (0x3UL << GPIO_MODER_MODE15_Pos)                  /*!< 0xC0000000 */
#define GPIO_MODER_MODE15                         GPIO_MODER_MODE15_Msk                             /*!< MODE15[31:30] bits (desc MODE15) */
#define GPIO_MODER_MODE15_0                       (0x1UL << GPIO_MODER_MODE15_Pos)                  /*!< 0x40000000 */
#define GPIO_MODER_MODE15_1                       (0x2UL << GPIO_MODER_MODE15_Pos)                  /*!< 0x80000000 */


/*!< GPIO_OTYPER */
#define GPIO_OTYPER_OT0_Pos                       (0U)
#define GPIO_OTYPER_OT0_Msk                       (0x1UL << GPIO_OTYPER_OT0_Pos)                    /*!< 0x00000001 */
#define GPIO_OTYPER_OT0                           GPIO_OTYPER_OT0_Msk                               /*!< desc OT0 */
#define GPIO_OTYPER_OT1_Pos                       (1U)
#define GPIO_OTYPER_OT1_Msk                       (0x1UL << GPIO_OTYPER_OT1_Pos)                    /*!< 0x00000002 */
#define GPIO_OTYPER_OT1                           GPIO_OTYPER_OT1_Msk                               /*!< desc OT1 */
#define GPIO_OTYPER_OT2_Pos                       (2U)
#define GPIO_OTYPER_OT2_Msk                       (0x1UL << GPIO_OTYPER_OT2_Pos)                    /*!< 0x00000004 */
#define GPIO_OTYPER_OT2                           GPIO_OTYPER_OT2_Msk                               /*!< desc OT2 */
#define GPIO_OTYPER_OT3_Pos                       (3U)
#define GPIO_OTYPER_OT3_Msk                       (0x1UL << GPIO_OTYPER_OT3_Pos)                    /*!< 0x00000008 */
#define GPIO_OTYPER_OT3                           GPIO_OTYPER_OT3_Msk                               /*!< desc OT3 */
#define GPIO_OTYPER_OT4_Pos                       (4U)
#define GPIO_OTYPER_OT4_Msk                       (0x1UL << GPIO_OTYPER_OT4_Pos)                    /*!< 0x00000010 */
#define GPIO_OTYPER_OT4                           GPIO_OTYPER_OT4_Msk                               /*!< desc OT4 */
#define GPIO_OTYPER_OT5_Pos                       (5U)
#define GPIO_OTYPER_OT5_Msk                       (0x1UL << GPIO_OTYPER_OT5_Pos)                    /*!< 0x00000020 */
#define GPIO_OTYPER_OT5                           GPIO_OTYPER_OT5_Msk                               /*!< desc OT5 */
#define GPIO_OTYPER_OT6_Pos                       (6U)
#define GPIO_OTYPER_OT6_Msk                       (0x1UL << GPIO_OTYPER_OT6_Pos)                    /*!< 0x00000040 */
#define GPIO_OTYPER_OT6                           GPIO_OTYPER_OT6_Msk                               /*!< desc OT6 */
#define GPIO_OTYPER_OT7_Pos                       (7U)
#define GPIO_OTYPER_OT7_Msk                       (0x1UL << GPIO_OTYPER_OT7_Pos)                    /*!< 0x00000080 */
#define GPIO_OTYPER_OT7                           GPIO_OTYPER_OT7_Msk                               /*!< desc OT7 */
#define GPIO_OTYPER_OT8_Pos                       (8U)
#define GPIO_OTYPER_OT8_Msk                       (0x1UL << GPIO_OTYPER_OT8_Pos)                    /*!< 0x00000100 */
#define GPIO_OTYPER_OT8                           GPIO_OTYPER_OT8_Msk                               /*!< desc OT8 */
#define GPIO_OTYPER_OT9_Pos                       (9U)
#define GPIO_OTYPER_OT9_Msk                       (0x1UL << GPIO_OTYPER_OT9_Pos)                    /*!< 0x00000200 */
#define GPIO_OTYPER_OT9                           GPIO_OTYPER_OT9_Msk                               /*!< desc OT9 */
#define GPIO_OTYPER_OT10_Pos                      (10U)
#define GPIO_OTYPER_OT10_Msk                      (0x1UL << GPIO_OTYPER_OT10_Pos)                   /*!< 0x00000400 */
#define GPIO_OTYPER_OT10                          GPIO_OTYPER_OT10_Msk                              /*!< desc OT10 */
#define GPIO_OTYPER_OT11_Pos                      (11U)
#define GPIO_OTYPER_OT11_Msk                      (0x1UL << GPIO_OTYPER_OT11_Pos)                   /*!< 0x00000800 */
#define GPIO_OTYPER_OT11                          GPIO_OTYPER_OT11_Msk                              /*!< desc OT11 */
#define GPIO_OTYPER_OT12_Pos                      (12U)
#define GPIO_OTYPER_OT12_Msk                      (0x1UL << GPIO_OTYPER_OT12_Pos)                   /*!< 0x00001000 */
#define GPIO_OTYPER_OT12                          GPIO_OTYPER_OT12_Msk                              /*!< desc OT12 */
#define GPIO_OTYPER_OT13_Pos                      (13U)
#define GPIO_OTYPER_OT13_Msk                      (0x1UL << GPIO_OTYPER_OT13_Pos)                   /*!< 0x00002000 */
#define GPIO_OTYPER_OT13                          GPIO_OTYPER_OT13_Msk                              /*!< desc OT13 */
#define GPIO_OTYPER_OT14_Pos                      (14U)
#define GPIO_OTYPER_OT14_Msk                      (0x1UL << GPIO_OTYPER_OT14_Pos)                   /*!< 0x00004000 */
#define GPIO_OTYPER_OT14                          GPIO_OTYPER_OT14_Msk                              /*!< desc OT14 */
#define GPIO_OTYPER_OT15_Pos                      (15U)
#define GPIO_OTYPER_OT15_Msk                      (0x1UL << GPIO_OTYPER_OT15_Pos)                   /*!< 0x00008000 */
#define GPIO_OTYPER_OT15                          GPIO_OTYPER_OT15_Msk                              /*!< desc OT15 */

/*!< GPIO_OSPEEDR */
#define GPIO_OSPEEDR_OSPEED0_Pos                  (0U)
#define GPIO_OSPEEDR_OSPEED0_Msk                  (0x3UL << GPIO_OSPEEDR_OSPEED0_Pos)               /*!< 0x00000003 */
#define GPIO_OSPEEDR_OSPEED0                      GPIO_OSPEEDR_OSPEED0_Msk                          /*!< OSPEED0[1:0] bits (desc OSPEED0) */
#define GPIO_OSPEEDR_OSPEED0_0                    (0x1UL << GPIO_OSPEEDR_OSPEED0_Pos)               /*!< 0x00000001 */
#define GPIO_OSPEEDR_OSPEED0_1                    (0x2UL << GPIO_OSPEEDR_OSPEED0_Pos)               /*!< 0x00000002 */

#define GPIO_OSPEEDR_OSPEED1_Pos                  (2U)
#define GPIO_OSPEEDR_OSPEED1_Msk                  (0x3UL << GPIO_OSPEEDR_OSPEED1_Pos)               /*!< 0x0000000C */
#define GPIO_OSPEEDR_OSPEED1                      GPIO_OSPEEDR_OSPEED1_Msk                          /*!< OSPEED1[3:2] bits (desc OSPEED1) */
#define GPIO_OSPEEDR_OSPEED1_0                    (0x1UL << GPIO_OSPEEDR_OSPEED1_Pos)               /*!< 0x00000004 */
#define GPIO_OSPEEDR_OSPEED1_1                    (0x2UL << GPIO_OSPEEDR_OSPEED1_Pos)               /*!< 0x00000008 */

#define GPIO_OSPEEDR_OSPEED2_Pos                  (4U)
#define GPIO_OSPEEDR_OSPEED2_Msk                  (0x3UL << GPIO_OSPEEDR_OSPEED2_Pos)               /*!< 0x00000030 */
#define GPIO_OSPEEDR_OSPEED2                      GPIO_OSPEEDR_OSPEED2_Msk                          /*!< OSPEED2[5:4] bits (desc OSPEED2) */
#define GPIO_OSPEEDR_OSPEED2_0                    (0x1UL << GPIO_OSPEEDR_OSPEED2_Pos)               /*!< 0x00000010 */
#define GPIO_OSPEEDR_OSPEED2_1                    (0x2UL << GPIO_OSPEEDR_OSPEED2_Pos)               /*!< 0x00000020 */

#define GPIO_OSPEEDR_OSPEED3_Pos                  (6U)
#define GPIO_OSPEEDR_OSPEED3_Msk                  (0x3UL << GPIO_OSPEEDR_OSPEED3_Pos)               /*!< 0x000000C0 */
#define GPIO_OSPEEDR_OSPEED3                      GPIO_OSPEEDR_OSPEED3_Msk                          /*!< OSPEED3[7:6] bits (desc OSPEED3) */
#define GPIO_OSPEEDR_OSPEED3_0                    (0x1UL << GPIO_OSPEEDR_OSPEED3_Pos)               /*!< 0x00000040 */
#define GPIO_OSPEEDR_OSPEED3_1                    (0x2UL << GPIO_OSPEEDR_OSPEED3_Pos)               /*!< 0x00000080 */

#define GPIO_OSPEEDR_OSPEED4_Pos                  (8U)
#define GPIO_OSPEEDR_OSPEED4_Msk                  (0x3UL << GPIO_OSPEEDR_OSPEED4_Pos)               /*!< 0x00000300 */
#define GPIO_OSPEEDR_OSPEED4                      GPIO_OSPEEDR_OSPEED4_Msk                          /*!< OSPEED4[9:8] bits (desc OSPEED4) */
#define GPIO_OSPEEDR_OSPEED4_0                    (0x1UL << GPIO_OSPEEDR_OSPEED4_Pos)               /*!< 0x00000100 */
#define GPIO_OSPEEDR_OSPEED4_1                    (0x2UL << GPIO_OSPEEDR_OSPEED4_Pos)               /*!< 0x00000200 */

#define GPIO_OSPEEDR_OSPEED5_Pos                  (10U)
#define GPIO_OSPEEDR_OSPEED5_Msk                  (0x3UL << GPIO_OSPEEDR_OSPEED5_Pos)               /*!< 0x00000C00 */
#define GPIO_OSPEEDR_OSPEED5                      GPIO_OSPEEDR_OSPEED5_Msk                          /*!< OSPEED5[11:10] bits (desc OSPEED5) */
#define GPIO_OSPEEDR_OSPEED5_0                    (0x1UL << GPIO_OSPEEDR_OSPEED5_Pos)               /*!< 0x00000400 */
#define GPIO_OSPEEDR_OSPEED5_1                    (0x2UL << GPIO_OSPEEDR_OSPEED5_Pos)               /*!< 0x00000800 */

#define GPIO_OSPEEDR_OSPEED6_Pos                  (12U)
#define GPIO_OSPEEDR_OSPEED6_Msk                  (0x3UL << GPIO_OSPEEDR_OSPEED6_Pos)               /*!< 0x00003000 */
#define GPIO_OSPEEDR_OSPEED6                      GPIO_OSPEEDR_OSPEED6_Msk                          /*!< OSPEED6[13:12] bits (desc OSPEED6) */
#define GPIO_OSPEEDR_OSPEED6_0                    (0x1UL << GPIO_OSPEEDR_OSPEED6_Pos)               /*!< 0x00001000 */
#define GPIO_OSPEEDR_OSPEED6_1                    (0x2UL << GPIO_OSPEEDR_OSPEED6_Pos)               /*!< 0x00002000 */

#define GPIO_OSPEEDR_OSPEED7_Pos                  (14U)
#define GPIO_OSPEEDR_OSPEED7_Msk                  (0x3UL << GPIO_OSPEEDR_OSPEED7_Pos)               /*!< 0x0000C000 */
#define GPIO_OSPEEDR_OSPEED7                      GPIO_OSPEEDR_OSPEED7_Msk                          /*!< OSPEED7[15:14] bits (desc OSPEED7) */
#define GPIO_OSPEEDR_OSPEED7_0                    (0x1UL << GPIO_OSPEEDR_OSPEED7_Pos)               /*!< 0x00004000 */
#define GPIO_OSPEEDR_OSPEED7_1                    (0x2UL << GPIO_OSPEEDR_OSPEED7_Pos)               /*!< 0x00008000 */

#define GPIO_OSPEEDR_OSPEED8_Pos                  (16U)
#define GPIO_OSPEEDR_OSPEED8_Msk                  (0x3UL << GPIO_OSPEEDR_OSPEED8_Pos)               /*!< 0x00030000 */
#define GPIO_OSPEEDR_OSPEED8                      GPIO_OSPEEDR_OSPEED8_Msk                          /*!< OSPEED8[17:16] bits (desc OSPEED8) */
#define GPIO_OSPEEDR_OSPEED8_0                    (0x1UL << GPIO_OSPEEDR_OSPEED8_Pos)               /*!< 0x00010000 */
#define GPIO_OSPEEDR_OSPEED8_1                    (0x2UL << GPIO_OSPEEDR_OSPEED8_Pos)               /*!< 0x00020000 */

#define GPIO_OSPEEDR_OSPEED9_Pos                  (18U)
#define GPIO_OSPEEDR_OSPEED9_Msk                  (0x3UL << GPIO_OSPEEDR_OSPEED9_Pos)               /*!< 0x000C0000 */
#define GPIO_OSPEEDR_OSPEED9                      GPIO_OSPEEDR_OSPEED9_Msk                          /*!< OSPEED9[19:18] bits (desc OSPEED9) */
#define GPIO_OSPEEDR_OSPEED9_0                    (0x1UL << GPIO_OSPEEDR_OSPEED9_Pos)               /*!< 0x00040000 */
#define GPIO_OSPEEDR_OSPEED9_1                    (0x2UL << GPIO_OSPEEDR_OSPEED9_Pos)               /*!< 0x00080000 */

#define GPIO_OSPEEDR_OSPEED10_Pos                 (20U)
#define GPIO_OSPEEDR_OSPEED10_Msk                 (0x3UL << GPIO_OSPEEDR_OSPEED10_Pos)              /*!< 0x00300000 */
#define GPIO_OSPEEDR_OSPEED10                     GPIO_OSPEEDR_OSPEED10_Msk                         /*!< OSPEED10[21:20] bits (desc OSPEED10) */
#define GPIO_OSPEEDR_OSPEED10_0                   (0x1UL << GPIO_OSPEEDR_OSPEED10_Pos)              /*!< 0x00100000 */
#define GPIO_OSPEEDR_OSPEED10_1                   (0x2UL << GPIO_OSPEEDR_OSPEED10_Pos)              /*!< 0x00200000 */

#define GPIO_OSPEEDR_OSPEED11_Pos                 (22U)
#define GPIO_OSPEEDR_OSPEED11_Msk                 (0x3UL << GPIO_OSPEEDR_OSPEED11_Pos)              /*!< 0x00C00000 */
#define GPIO_OSPEEDR_OSPEED11                     GPIO_OSPEEDR_OSPEED11_Msk                         /*!< OSPEED11[23:22] bits (desc OSPEED11) */
#define GPIO_OSPEEDR_OSPEED11_0                   (0x1UL << GPIO_OSPEEDR_OSPEED11_Pos)              /*!< 0x00400000 */
#define GPIO_OSPEEDR_OSPEED11_1                   (0x2UL << GPIO_OSPEEDR_OSPEED11_Pos)              /*!< 0x00800000 */

#define GPIO_OSPEEDR_OSPEED12_Pos                 (24U)
#define GPIO_OSPEEDR_OSPEED12_Msk                 (0x3UL << GPIO_OSPEEDR_OSPEED12_Pos)              /*!< 0x03000000 */
#define GPIO_OSPEEDR_OSPEED12                     GPIO_OSPEEDR_OSPEED12_Msk                         /*!< OSPEED12[25:24] bits (desc OSPEED12) */
#define GPIO_OSPEEDR_OSPEED12_0                   (0x1UL << GPIO_OSPEEDR_OSPEED12_Pos)              /*!< 0x01000000 */
#define GPIO_OSPEEDR_OSPEED12_1                   (0x2UL << GPIO_OSPEEDR_OSPEED12_Pos)              /*!< 0x02000000 */

#define GPIO_OSPEEDR_OSPEED13_Pos                 (26U)
#define GPIO_OSPEEDR_OSPEED13_Msk                 (0x3UL << GPIO_OSPEEDR_OSPEED13_Pos)              /*!< 0x0C000000 */
#define GPIO_OSPEEDR_OSPEED13                     GPIO_OSPEEDR_OSPEED13_Msk                         /*!< OSPEED13[27:26] bits (desc OSPEED13) */
#define GPIO_OSPEEDR_OSPEED13_0                   (0x1UL << GPIO_OSPEEDR_OSPEED13_Pos)              /*!< 0x04000000 */
#define GPIO_OSPEEDR_OSPEED13_1                   (0x2UL << GPIO_OSPEEDR_OSPEED13_Pos)              /*!< 0x08000000 */

#define GPIO_OSPEEDR_OSPEED14_Pos                 (28U)
#define GPIO_OSPEEDR_OSPEED14_Msk                 (0x3UL << GPIO_OSPEEDR_OSPEED14_Pos)              /*!< 0x30000000 */
#define GPIO_OSPEEDR_OSPEED14                     GPIO_OSPEEDR_OSPEED14_Msk                         /*!< OSPEED14[29:28] bits (desc OSPEED14) */
#define GPIO_OSPEEDR_OSPEED14_0                   (0x1UL << GPIO_OSPEEDR_OSPEED14_Pos)              /*!< 0x10000000 */
#define GPIO_OSPEEDR_OSPEED14_1                   (0x2UL << GPIO_OSPEEDR_OSPEED14_Pos)              /*!< 0x20000000 */

#define GPIO_OSPEEDR_OSPEED15_Pos                 (30U)
#define GPIO_OSPEEDR_OSPEED15_Msk                 (0x3UL << GPIO_OSPEEDR_OSPEED15_Pos)              /*!< 0xC0000000 */
#define GPIO_OSPEEDR_OSPEED15                     GPIO_OSPEEDR_OSPEED15_Msk                         /*!< OSPEED15[31:30] bits (desc OSPEED15) */
#define GPIO_OSPEEDR_OSPEED15_0                   (0x1UL << GPIO_OSPEEDR_OSPEED15_Pos)              /*!< 0x40000000 */
#define GPIO_OSPEEDR_OSPEED15_1                   (0x2UL << GPIO_OSPEEDR_OSPEED15_Pos)              /*!< 0x80000000 */


/*!< GPIO_PUPDR */
#define GPIO_PUPDR_PUPD0_Pos                      (0U)
#define GPIO_PUPDR_PUPD0_Msk                      (0x3UL << GPIO_PUPDR_PUPD0_Pos)                   /*!< 0x00000003 */
#define GPIO_PUPDR_PUPD0                          GPIO_PUPDR_PUPD0_Msk                              /*!< PUPD0[1:0] bits (desc PUPD0) */
#define GPIO_PUPDR_PUPD0_0                        (0x1UL << GPIO_PUPDR_PUPD0_Pos)                   /*!< 0x00000001 */
#define GPIO_PUPDR_PUPD0_1                        (0x2UL << GPIO_PUPDR_PUPD0_Pos)                   /*!< 0x00000002 */

#define GPIO_PUPDR_PUPD1_Pos                      (2U)
#define GPIO_PUPDR_PUPD1_Msk                      (0x3UL << GPIO_PUPDR_PUPD1_Pos)                   /*!< 0x0000000C */
#define GPIO_PUPDR_PUPD1                          GPIO_PUPDR_PUPD1_Msk                              /*!< PUPD1[3:2] bits (desc PUPD1) */
#define GPIO_PUPDR_PUPD1_0                        (0x1UL << GPIO_PUPDR_PUPD1_Pos)                   /*!< 0x00000004 */
#define GPIO_PUPDR_PUPD1_1                        (0x2UL << GPIO_PUPDR_PUPD1_Pos)                   /*!< 0x00000008 */

#define GPIO_PUPDR_PUPD2_Pos                      (4U)
#define GPIO_PUPDR_PUPD2_Msk                      (0x3UL << GPIO_PUPDR_PUPD2_Pos)                   /*!< 0x00000030 */
#define GPIO_PUPDR_PUPD2                          GPIO_PUPDR_PUPD2_Msk                              /*!< PUPD2[5:4] bits (desc PUPD2) */
#define GPIO_PUPDR_PUPD2_0                        (0x1UL << GPIO_PUPDR_PUPD2_Pos)                   /*!< 0x00000010 */
#define GPIO_PUPDR_PUPD2_1                        (0x2UL << GPIO_PUPDR_PUPD2_Pos)                   /*!< 0x00000020 */

#define GPIO_PUPDR_PUPD3_Pos                      (6U)
#define GPIO_PUPDR_PUPD3_Msk                      (0x3UL << GPIO_PUPDR_PUPD3_Pos)                   /*!< 0x000000C0 */
#define GPIO_PUPDR_PUPD3                          GPIO_PUPDR_PUPD3_Msk                              /*!< PUPD3[7:6] bits (desc PUPD3) */
#define GPIO_PUPDR_PUPD3_0                        (0x1UL << GPIO_PUPDR_PUPD3_Pos)                   /*!< 0x00000040 */
#define GPIO_PUPDR_PUPD3_1                        (0x2UL << GPIO_PUPDR_PUPD3_Pos)                   /*!< 0x00000080 */

#define GPIO_PUPDR_PUPD4_Pos                      (8U)
#define GPIO_PUPDR_PUPD4_Msk                      (0x3UL << GPIO_PUPDR_PUPD4_Pos)                   /*!< 0x00000300 */
#define GPIO_PUPDR_PUPD4                          GPIO_PUPDR_PUPD4_Msk                              /*!< PUPD4[9:8] bits (desc PUPD4) */
#define GPIO_PUPDR_PUPD4_0                        (0x1UL << GPIO_PUPDR_PUPD4_Pos)                   /*!< 0x00000100 */
#define GPIO_PUPDR_PUPD4_1                        (0x2UL << GPIO_PUPDR_PUPD4_Pos)                   /*!< 0x00000200 */

#define GPIO_PUPDR_PUPD5_Pos                      (10U)
#define GPIO_PUPDR_PUPD5_Msk                      (0x3UL << GPIO_PUPDR_PUPD5_Pos)                   /*!< 0x00000C00 */
#define GPIO_PUPDR_PUPD5                          GPIO_PUPDR_PUPD5_Msk                              /*!< PUPD5[11:10] bits (desc PUPD5) */
#define GPIO_PUPDR_PUPD5_0                        (0x1UL << GPIO_PUPDR_PUPD5_Pos)                   /*!< 0x00000400 */
#define GPIO_PUPDR_PUPD5_1                        (0x2UL << GPIO_PUPDR_PUPD5_Pos)                   /*!< 0x00000800 */

#define GPIO_PUPDR_PUPD6_Pos                      (12U)
#define GPIO_PUPDR_PUPD6_Msk                      (0x3UL << GPIO_PUPDR_PUPD6_Pos)                   /*!< 0x00003000 */
#define GPIO_PUPDR_PUPD6                          GPIO_PUPDR_PUPD6_Msk                              /*!< PUPD6[13:12] bits (desc PUPD6) */
#define GPIO_PUPDR_PUPD6_0                        (0x1UL << GPIO_PUPDR_PUPD6_Pos)                   /*!< 0x00001000 */
#define GPIO_PUPDR_PUPD6_1                        (0x2UL << GPIO_PUPDR_PUPD6_Pos)                   /*!< 0x00002000 */

#define GPIO_PUPDR_PUPD7_Pos                      (14U)
#define GPIO_PUPDR_PUPD7_Msk                      (0x3UL << GPIO_PUPDR_PUPD7_Pos)                   /*!< 0x0000C000 */
#define GPIO_PUPDR_PUPD7                          GPIO_PUPDR_PUPD7_Msk                              /*!< PUPD7[15:14] bits (desc PUPD7) */
#define GPIO_PUPDR_PUPD7_0                        (0x1UL << GPIO_PUPDR_PUPD7_Pos)                   /*!< 0x00004000 */
#define GPIO_PUPDR_PUPD7_1                        (0x2UL << GPIO_PUPDR_PUPD7_Pos)                   /*!< 0x00008000 */

#define GPIO_PUPDR_PUPD8_Pos                      (16U)
#define GPIO_PUPDR_PUPD8_Msk                      (0x3UL << GPIO_PUPDR_PUPD8_Pos)                   /*!< 0x00030000 */
#define GPIO_PUPDR_PUPD8                          GPIO_PUPDR_PUPD8_Msk                              /*!< PUPD8[17:16] bits (desc PUPD8) */
#define GPIO_PUPDR_PUPD8_0                        (0x1UL << GPIO_PUPDR_PUPD8_Pos)                   /*!< 0x00010000 */
#define GPIO_PUPDR_PUPD8_1                        (0x2UL << GPIO_PUPDR_PUPD8_Pos)                   /*!< 0x00020000 */

#define GPIO_PUPDR_PUPD9_Pos                      (18U)
#define GPIO_PUPDR_PUPD9_Msk                      (0x3UL << GPIO_PUPDR_PUPD9_Pos)                   /*!< 0x000C0000 */
#define GPIO_PUPDR_PUPD9                          GPIO_PUPDR_PUPD9_Msk                              /*!< PUPD9[19:18] bits (desc PUPD9) */
#define GPIO_PUPDR_PUPD9_0                        (0x1UL << GPIO_PUPDR_PUPD9_Pos)                   /*!< 0x00040000 */
#define GPIO_PUPDR_PUPD9_1                        (0x2UL << GPIO_PUPDR_PUPD9_Pos)                   /*!< 0x00080000 */

#define GPIO_PUPDR_PUPD10_Pos                     (20U)
#define GPIO_PUPDR_PUPD10_Msk                     (0x3UL << GPIO_PUPDR_PUPD10_Pos)                  /*!< 0x00300000 */
#define GPIO_PUPDR_PUPD10                         GPIO_PUPDR_PUPD10_Msk                             /*!< PUPD10[21:20] bits (desc PUPD10) */
#define GPIO_PUPDR_PUPD10_0                       (0x1UL << GPIO_PUPDR_PUPD10_Pos)                  /*!< 0x00100000 */
#define GPIO_PUPDR_PUPD10_1                       (0x2UL << GPIO_PUPDR_PUPD10_Pos)                  /*!< 0x00200000 */

#define GPIO_PUPDR_PUPD11_Pos                     (22U)
#define GPIO_PUPDR_PUPD11_Msk                     (0x3UL << GPIO_PUPDR_PUPD11_Pos)                  /*!< 0x00C00000 */
#define GPIO_PUPDR_PUPD11                         GPIO_PUPDR_PUPD11_Msk                             /*!< PUPD11[23:22] bits (desc PUPD11) */
#define GPIO_PUPDR_PUPD11_0                       (0x1UL << GPIO_PUPDR_PUPD11_Pos)                  /*!< 0x00400000 */
#define GPIO_PUPDR_PUPD11_1                       (0x2UL << GPIO_PUPDR_PUPD11_Pos)                  /*!< 0x00800000 */

#define GPIO_PUPDR_PUPD12_Pos                     (24U)
#define GPIO_PUPDR_PUPD12_Msk                     (0x3UL << GPIO_PUPDR_PUPD12_Pos)                  /*!< 0x03000000 */
#define GPIO_PUPDR_PUPD12                         GPIO_PUPDR_PUPD12_Msk                             /*!< PUPD12[25:24] bits (desc PUPD12) */
#define GPIO_PUPDR_PUPD12_0                       (0x1UL << GPIO_PUPDR_PUPD12_Pos)                  /*!< 0x01000000 */
#define GPIO_PUPDR_PUPD12_1                       (0x2UL << GPIO_PUPDR_PUPD12_Pos)                  /*!< 0x02000000 */

#define GPIO_PUPDR_PUPD13_Pos                     (26U)
#define GPIO_PUPDR_PUPD13_Msk                     (0x3UL << GPIO_PUPDR_PUPD13_Pos)                  /*!< 0x0C000000 */
#define GPIO_PUPDR_PUPD13                         GPIO_PUPDR_PUPD13_Msk                             /*!< PUPD13[27:26] bits (desc PUPD13) */
#define GPIO_PUPDR_PUPD13_0                       (0x1UL << GPIO_PUPDR_PUPD13_Pos)                  /*!< 0x04000000 */
#define GPIO_PUPDR_PUPD13_1                       (0x2UL << GPIO_PUPDR_PUPD13_Pos)                  /*!< 0x08000000 */

#define GPIO_PUPDR_PUPD14_Pos                     (28U)
#define GPIO_PUPDR_PUPD14_Msk                     (0x3UL << GPIO_PUPDR_PUPD14_Pos)                  /*!< 0x30000000 */
#define GPIO_PUPDR_PUPD14                         GPIO_PUPDR_PUPD14_Msk                             /*!< PUPD14[29:28] bits (desc PUPD14) */
#define GPIO_PUPDR_PUPD14_0                       (0x1UL << GPIO_PUPDR_PUPD14_Pos)                  /*!< 0x10000000 */
#define GPIO_PUPDR_PUPD14_1                       (0x2UL << GPIO_PUPDR_PUPD14_Pos)                  /*!< 0x20000000 */

#define GPIO_PUPDR_PUPD15_Pos                     (30U)
#define GPIO_PUPDR_PUPD15_Msk                     (0x3UL << GPIO_PUPDR_PUPD15_Pos)                  /*!< 0xC0000000 */
#define GPIO_PUPDR_PUPD15                         GPIO_PUPDR_PUPD15_Msk                             /*!< PUPD15[31:30] bits (desc PUPD15) */
#define GPIO_PUPDR_PUPD15_0                       (0x1UL << GPIO_PUPDR_PUPD15_Pos)                  /*!< 0x40000000 */
#define GPIO_PUPDR_PUPD15_1                       (0x2UL << GPIO_PUPDR_PUPD15_Pos)                  /*!< 0x80000000 */


/*!< GPIO_IDR */
#define GPIO_IDR_ID0_Pos                          (0U)
#define GPIO_IDR_ID0_Msk                          (0x1UL << GPIO_IDR_ID0_Pos)                       /*!< 0x00000001 */
#define GPIO_IDR_ID0                              GPIO_IDR_ID0_Msk                                  /*!< desc ID0 */
#define GPIO_IDR_ID1_Pos                          (1U)
#define GPIO_IDR_ID1_Msk                          (0x1UL << GPIO_IDR_ID1_Pos)                       /*!< 0x00000002 */
#define GPIO_IDR_ID1                              GPIO_IDR_ID1_Msk                                  /*!< desc ID1 */
#define GPIO_IDR_ID2_Pos                          (2U)
#define GPIO_IDR_ID2_Msk                          (0x1UL << GPIO_IDR_ID2_Pos)                       /*!< 0x00000004 */
#define GPIO_IDR_ID2                              GPIO_IDR_ID2_Msk                                  /*!< desc ID2 */
#define GPIO_IDR_ID3_Pos                          (3U)
#define GPIO_IDR_ID3_Msk                          (0x1UL << GPIO_IDR_ID3_Pos)                       /*!< 0x00000008 */
#define GPIO_IDR_ID3                              GPIO_IDR_ID3_Msk                                  /*!< desc ID3 */
#define GPIO_IDR_ID4_Pos                          (4U)
#define GPIO_IDR_ID4_Msk                          (0x1UL << GPIO_IDR_ID4_Pos)                       /*!< 0x00000010 */
#define GPIO_IDR_ID4                              GPIO_IDR_ID4_Msk                                  /*!< desc ID4 */
#define GPIO_IDR_ID5_Pos                          (5U)
#define GPIO_IDR_ID5_Msk                          (0x1UL << GPIO_IDR_ID5_Pos)                       /*!< 0x00000020 */
#define GPIO_IDR_ID5                              GPIO_IDR_ID5_Msk                                  /*!< desc ID5 */
#define GPIO_IDR_ID6_Pos                          (6U)
#define GPIO_IDR_ID6_Msk                          (0x1UL << GPIO_IDR_ID6_Pos)                       /*!< 0x00000040 */
#define GPIO_IDR_ID6                              GPIO_IDR_ID6_Msk                                  /*!< desc ID6 */
#define GPIO_IDR_ID7_Pos                          (7U)
#define GPIO_IDR_ID7_Msk                          (0x1UL << GPIO_IDR_ID7_Pos)                       /*!< 0x00000080 */
#define GPIO_IDR_ID7                              GPIO_IDR_ID7_Msk                                  /*!< desc ID7 */
#define GPIO_IDR_ID8_Pos                          (8U)
#define GPIO_IDR_ID8_Msk                          (0x1UL << GPIO_IDR_ID8_Pos)                       /*!< 0x00000100 */
#define GPIO_IDR_ID8                              GPIO_IDR_ID8_Msk                                  /*!< desc ID8 */
#define GPIO_IDR_ID9_Pos                          (9U)
#define GPIO_IDR_ID9_Msk                          (0x1UL << GPIO_IDR_ID9_Pos)                       /*!< 0x00000200 */
#define GPIO_IDR_ID9                              GPIO_IDR_ID9_Msk                                  /*!< desc ID9 */
#define GPIO_IDR_ID10_Pos                         (10U)
#define GPIO_IDR_ID10_Msk                         (0x1UL << GPIO_IDR_ID10_Pos)                      /*!< 0x00000400 */
#define GPIO_IDR_ID10                             GPIO_IDR_ID10_Msk                                 /*!< desc ID10 */
#define GPIO_IDR_ID11_Pos                         (11U)
#define GPIO_IDR_ID11_Msk                         (0x1UL << GPIO_IDR_ID11_Pos)                      /*!< 0x00000800 */
#define GPIO_IDR_ID11                             GPIO_IDR_ID11_Msk                                 /*!< desc ID11 */
#define GPIO_IDR_ID12_Pos                         (12U)
#define GPIO_IDR_ID12_Msk                         (0x1UL << GPIO_IDR_ID12_Pos)                      /*!< 0x00001000 */
#define GPIO_IDR_ID12                             GPIO_IDR_ID12_Msk                                 /*!< desc ID12 */
#define GPIO_IDR_ID13_Pos                         (13U)
#define GPIO_IDR_ID13_Msk                         (0x1UL << GPIO_IDR_ID13_Pos)                      /*!< 0x00002000 */
#define GPIO_IDR_ID13                             GPIO_IDR_ID13_Msk                                 /*!< desc ID13 */
#define GPIO_IDR_ID14_Pos                         (14U)
#define GPIO_IDR_ID14_Msk                         (0x1UL << GPIO_IDR_ID14_Pos)                      /*!< 0x00004000 */
#define GPIO_IDR_ID14                             GPIO_IDR_ID14_Msk                                 /*!< desc ID14 */
#define GPIO_IDR_ID15_Pos                         (15U)
#define GPIO_IDR_ID15_Msk                         (0x1UL << GPIO_IDR_ID15_Pos)                      /*!< 0x00008000 */
#define GPIO_IDR_ID15                             GPIO_IDR_ID15_Msk                                 /*!< desc ID15 */

/*!< GPIO_ODR */
#define GPIO_ODR_OD0_Pos                          (0U)
#define GPIO_ODR_OD0_Msk                          (0x1UL << GPIO_ODR_OD0_Pos)                       /*!< 0x00000001 */
#define GPIO_ODR_OD0                              GPIO_ODR_OD0_Msk                                  /*!< desc OD0 */
#define GPIO_ODR_OD1_Pos                          (1U)
#define GPIO_ODR_OD1_Msk                          (0x1UL << GPIO_ODR_OD1_Pos)                       /*!< 0x00000002 */
#define GPIO_ODR_OD1                              GPIO_ODR_OD1_Msk                                  /*!< desc OD1 */
#define GPIO_ODR_OD2_Pos                          (2U)
#define GPIO_ODR_OD2_Msk                          (0x1UL << GPIO_ODR_OD2_Pos)                       /*!< 0x00000004 */
#define GPIO_ODR_OD2                              GPIO_ODR_OD2_Msk                                  /*!< desc OD2 */
#define GPIO_ODR_OD3_Pos                          (3U)
#define GPIO_ODR_OD3_Msk                          (0x1UL << GPIO_ODR_OD3_Pos)                       /*!< 0x00000008 */
#define GPIO_ODR_OD3                              GPIO_ODR_OD3_Msk                                  /*!< desc OD3 */
#define GPIO_ODR_OD4_Pos                          (4U)
#define GPIO_ODR_OD4_Msk                          (0x1UL << GPIO_ODR_OD4_Pos)                       /*!< 0x00000010 */
#define GPIO_ODR_OD4                              GPIO_ODR_OD4_Msk                                  /*!< desc OD4 */
#define GPIO_ODR_OD5_Pos                          (5U)
#define GPIO_ODR_OD5_Msk                          (0x1UL << GPIO_ODR_OD5_Pos)                       /*!< 0x00000020 */
#define GPIO_ODR_OD5                              GPIO_ODR_OD5_Msk                                  /*!< desc OD5 */
#define GPIO_ODR_OD6_Pos                          (6U)
#define GPIO_ODR_OD6_Msk                          (0x1UL << GPIO_ODR_OD6_Pos)                       /*!< 0x00000040 */
#define GPIO_ODR_OD6                              GPIO_ODR_OD6_Msk                                  /*!< desc OD6 */
#define GPIO_ODR_OD7_Pos                          (7U)
#define GPIO_ODR_OD7_Msk                          (0x1UL << GPIO_ODR_OD7_Pos)                       /*!< 0x00000080 */
#define GPIO_ODR_OD7                              GPIO_ODR_OD7_Msk                                  /*!< desc OD7 */
#define GPIO_ODR_OD8_Pos                          (8U)
#define GPIO_ODR_OD8_Msk                          (0x1UL << GPIO_ODR_OD8_Pos)                       /*!< 0x00000100 */
#define GPIO_ODR_OD8                              GPIO_ODR_OD8_Msk                                  /*!< desc OD8 */
#define GPIO_ODR_OD9_Pos                          (9U)
#define GPIO_ODR_OD9_Msk                          (0x1UL << GPIO_ODR_OD9_Pos)                       /*!< 0x00000200 */
#define GPIO_ODR_OD9                              GPIO_ODR_OD9_Msk                                  /*!< desc OD9 */
#define GPIO_ODR_OD10_Pos                         (10U)
#define GPIO_ODR_OD10_Msk                         (0x1UL << GPIO_ODR_OD10_Pos)                      /*!< 0x00000400 */
#define GPIO_ODR_OD10                             GPIO_ODR_OD10_Msk                                 /*!< desc OD10 */
#define GPIO_ODR_OD11_Pos                         (11U)
#define GPIO_ODR_OD11_Msk                         (0x1UL << GPIO_ODR_OD11_Pos)                      /*!< 0x00000800 */
#define GPIO_ODR_OD11                             GPIO_ODR_OD11_Msk                                 /*!< desc OD11 */
#define GPIO_ODR_OD12_Pos                         (12U)
#define GPIO_ODR_OD12_Msk                         (0x1UL << GPIO_ODR_OD12_Pos)                      /*!< 0x00001000 */
#define GPIO_ODR_OD12                             GPIO_ODR_OD12_Msk                                 /*!< desc OD12 */
#define GPIO_ODR_OD13_Pos                         (13U)
#define GPIO_ODR_OD13_Msk                         (0x1UL << GPIO_ODR_OD13_Pos)                      /*!< 0x00002000 */
#define GPIO_ODR_OD13                             GPIO_ODR_OD13_Msk                                 /*!< desc OD13 */
#define GPIO_ODR_OD14_Pos                         (14U)
#define GPIO_ODR_OD14_Msk                         (0x1UL << GPIO_ODR_OD14_Pos)                      /*!< 0x00004000 */
#define GPIO_ODR_OD14                             GPIO_ODR_OD14_Msk                                 /*!< desc OD14 */
#define GPIO_ODR_OD15_Pos                         (15U)
#define GPIO_ODR_OD15_Msk                         (0x1UL << GPIO_ODR_OD15_Pos)                      /*!< 0x00008000 */
#define GPIO_ODR_OD15                             GPIO_ODR_OD15_Msk                                 /*!< desc OD15 */

/*!< GPIO_BSRR */
#define GPIO_BSRR_BS0_Pos                         (0U)
#define GPIO_BSRR_BS0_Msk                         (0x1UL << GPIO_BSRR_BS0_Pos)                      /*!< 0x00000001 */
#define GPIO_BSRR_BS0                             GPIO_BSRR_BS0_Msk                                 /*!< desc BS0 */
#define GPIO_BSRR_BS1_Pos                         (1U)
#define GPIO_BSRR_BS1_Msk                         (0x1UL << GPIO_BSRR_BS1_Pos)                      /*!< 0x00000002 */
#define GPIO_BSRR_BS1                             GPIO_BSRR_BS1_Msk                                 /*!< desc BS1 */
#define GPIO_BSRR_BS2_Pos                         (2U)
#define GPIO_BSRR_BS2_Msk                         (0x1UL << GPIO_BSRR_BS2_Pos)                      /*!< 0x00000004 */
#define GPIO_BSRR_BS2                             GPIO_BSRR_BS2_Msk                                 /*!< desc BS2 */
#define GPIO_BSRR_BS3_Pos                         (3U)
#define GPIO_BSRR_BS3_Msk                         (0x1UL << GPIO_BSRR_BS3_Pos)                      /*!< 0x00000008 */
#define GPIO_BSRR_BS3                             GPIO_BSRR_BS3_Msk                                 /*!< desc BS3 */
#define GPIO_BSRR_BS4_Pos                         (4U)
#define GPIO_BSRR_BS4_Msk                         (0x1UL << GPIO_BSRR_BS4_Pos)                      /*!< 0x00000010 */
#define GPIO_BSRR_BS4                             GPIO_BSRR_BS4_Msk                                 /*!< desc BS4 */
#define GPIO_BSRR_BS5_Pos                         (5U)
#define GPIO_BSRR_BS5_Msk                         (0x1UL << GPIO_BSRR_BS5_Pos)                      /*!< 0x00000020 */
#define GPIO_BSRR_BS5                             GPIO_BSRR_BS5_Msk                                 /*!< desc BS5 */
#define GPIO_BSRR_BS6_Pos                         (6U)
#define GPIO_BSRR_BS6_Msk                         (0x1UL << GPIO_BSRR_BS6_Pos)                      /*!< 0x00000040 */
#define GPIO_BSRR_BS6                             GPIO_BSRR_BS6_Msk                                 /*!< desc BS6 */
#define GPIO_BSRR_BS7_Pos                         (7U)
#define GPIO_BSRR_BS7_Msk                         (0x1UL << GPIO_BSRR_BS7_Pos)                      /*!< 0x00000080 */
#define GPIO_BSRR_BS7                             GPIO_BSRR_BS7_Msk                                 /*!< desc BS7 */
#define GPIO_BSRR_BS8_Pos                         (8U)
#define GPIO_BSRR_BS8_Msk                         (0x1UL << GPIO_BSRR_BS8_Pos)                      /*!< 0x00000100 */
#define GPIO_BSRR_BS8                             GPIO_BSRR_BS8_Msk                                 /*!< desc BS8 */
#define GPIO_BSRR_BS9_Pos                         (9U)
#define GPIO_BSRR_BS9_Msk                         (0x1UL << GPIO_BSRR_BS9_Pos)                      /*!< 0x00000200 */
#define GPIO_BSRR_BS9                             GPIO_BSRR_BS9_Msk                                 /*!< desc BS9 */
#define GPIO_BSRR_BS10_Pos                        (10U)
#define GPIO_BSRR_BS10_Msk                        (0x1UL << GPIO_BSRR_BS10_Pos)                     /*!< 0x00000400 */
#define GPIO_BSRR_BS10                            GPIO_BSRR_BS10_Msk                                /*!< desc BS10 */
#define GPIO_BSRR_BS11_Pos                        (11U)
#define GPIO_BSRR_BS11_Msk                        (0x1UL << GPIO_BSRR_BS11_Pos)                     /*!< 0x00000800 */
#define GPIO_BSRR_BS11                            GPIO_BSRR_BS11_Msk                                /*!< desc BS11 */
#define GPIO_BSRR_BS12_Pos                        (12U)
#define GPIO_BSRR_BS12_Msk                        (0x1UL << GPIO_BSRR_BS12_Pos)                     /*!< 0x00001000 */
#define GPIO_BSRR_BS12                            GPIO_BSRR_BS12_Msk                                /*!< desc BS12 */
#define GPIO_BSRR_BS13_Pos                        (13U)
#define GPIO_BSRR_BS13_Msk                        (0x1UL << GPIO_BSRR_BS13_Pos)                     /*!< 0x00002000 */
#define GPIO_BSRR_BS13                            GPIO_BSRR_BS13_Msk                                /*!< desc BS13 */
#define GPIO_BSRR_BS14_Pos                        (14U)
#define GPIO_BSRR_BS14_Msk                        (0x1UL << GPIO_BSRR_BS14_Pos)                     /*!< 0x00004000 */
#define GPIO_BSRR_BS14                            GPIO_BSRR_BS14_Msk                                /*!< desc BS14 */
#define GPIO_BSRR_BS15_Pos                        (15U)
#define GPIO_BSRR_BS15_Msk                        (0x1UL << GPIO_BSRR_BS15_Pos)                     /*!< 0x00008000 */
#define GPIO_BSRR_BS15                            GPIO_BSRR_BS15_Msk                                /*!< desc BS15 */
#define GPIO_BSRR_BR0_Pos                         (16U)
#define GPIO_BSRR_BR0_Msk                         (0x1UL << GPIO_BSRR_BR0_Pos)                      /*!< 0x00010000 */
#define GPIO_BSRR_BR0                             GPIO_BSRR_BR0_Msk                                 /*!< desc BR0 */
#define GPIO_BSRR_BR1_Pos                         (17U)
#define GPIO_BSRR_BR1_Msk                         (0x1UL << GPIO_BSRR_BR1_Pos)                      /*!< 0x00020000 */
#define GPIO_BSRR_BR1                             GPIO_BSRR_BR1_Msk                                 /*!< desc BR1 */
#define GPIO_BSRR_BR2_Pos                         (18U)
#define GPIO_BSRR_BR2_Msk                         (0x1UL << GPIO_BSRR_BR2_Pos)                      /*!< 0x00040000 */
#define GPIO_BSRR_BR2                             GPIO_BSRR_BR2_Msk                                 /*!< desc BR2 */
#define GPIO_BSRR_BR3_Pos                         (19U)
#define GPIO_BSRR_BR3_Msk                         (0x1UL << GPIO_BSRR_BR3_Pos)                      /*!< 0x00080000 */
#define GPIO_BSRR_BR3                             GPIO_BSRR_BR3_Msk                                 /*!< desc BR3 */
#define GPIO_BSRR_BR4_Pos                         (20U)
#define GPIO_BSRR_BR4_Msk                         (0x1UL << GPIO_BSRR_BR4_Pos)                      /*!< 0x00100000 */
#define GPIO_BSRR_BR4                             GPIO_BSRR_BR4_Msk                                 /*!< desc BR4 */
#define GPIO_BSRR_BR5_Pos                         (21U)
#define GPIO_BSRR_BR5_Msk                         (0x1UL << GPIO_BSRR_BR5_Pos)                      /*!< 0x00200000 */
#define GPIO_BSRR_BR5                             GPIO_BSRR_BR5_Msk                                 /*!< desc BR5 */
#define GPIO_BSRR_BR6_Pos                         (22U)
#define GPIO_BSRR_BR6_Msk                         (0x1UL << GPIO_BSRR_BR6_Pos)                      /*!< 0x00400000 */
#define GPIO_BSRR_BR6                             GPIO_BSRR_BR6_Msk                                 /*!< desc BR6 */
#define GPIO_BSRR_BR7_Pos                         (23U)
#define GPIO_BSRR_BR7_Msk                         (0x1UL << GPIO_BSRR_BR7_Pos)                      /*!< 0x00800000 */
#define GPIO_BSRR_BR7                             GPIO_BSRR_BR7_Msk                                 /*!< desc BR7 */
#define GPIO_BSRR_BR8_Pos                         (24U)
#define GPIO_BSRR_BR8_Msk                         (0x1UL << GPIO_BSRR_BR8_Pos)                      /*!< 0x01000000 */
#define GPIO_BSRR_BR8                             GPIO_BSRR_BR8_Msk                                 /*!< desc BR8 */
#define GPIO_BSRR_BR9_Pos                         (25U)
#define GPIO_BSRR_BR9_Msk                         (0x1UL << GPIO_BSRR_BR9_Pos)                      /*!< 0x02000000 */
#define GPIO_BSRR_BR9                             GPIO_BSRR_BR9_Msk                                 /*!< desc BR9 */
#define GPIO_BSRR_BR10_Pos                        (26U)
#define GPIO_BSRR_BR10_Msk                        (0x1UL << GPIO_BSRR_BR10_Pos)                     /*!< 0x04000000 */
#define GPIO_BSRR_BR10                            GPIO_BSRR_BR10_Msk                                /*!< desc BR10 */
#define GPIO_BSRR_BR11_Pos                        (27U)
#define GPIO_BSRR_BR11_Msk                        (0x1UL << GPIO_BSRR_BR11_Pos)                     /*!< 0x08000000 */
#define GPIO_BSRR_BR11                            GPIO_BSRR_BR11_Msk                                /*!< desc BR11 */
#define GPIO_BSRR_BR12_Pos                        (28U)
#define GPIO_BSRR_BR12_Msk                        (0x1UL << GPIO_BSRR_BR12_Pos)                     /*!< 0x10000000 */
#define GPIO_BSRR_BR12                            GPIO_BSRR_BR12_Msk                                /*!< desc BR12 */
#define GPIO_BSRR_BR13_Pos                        (29U)
#define GPIO_BSRR_BR13_Msk                        (0x1UL << GPIO_BSRR_BR13_Pos)                     /*!< 0x20000000 */
#define GPIO_BSRR_BR13                            GPIO_BSRR_BR13_Msk                                /*!< desc BR13 */
#define GPIO_BSRR_BR14_Pos                        (30U)
#define GPIO_BSRR_BR14_Msk                        (0x1UL << GPIO_BSRR_BR14_Pos)                     /*!< 0x40000000 */
#define GPIO_BSRR_BR14                            GPIO_BSRR_BR14_Msk                                /*!< desc BR14 */
#define GPIO_BSRR_BR15_Pos                        (31U)
#define GPIO_BSRR_BR15_Msk                        (0x1UL << GPIO_BSRR_BR15_Pos)                     /*!< 0x80000000 */
#define GPIO_BSRR_BR15                            GPIO_BSRR_BR15_Msk                                /*!< desc BR15 */

/*!< GPIO_LCKR */
#define GPIO_LCKR_LCK0_Pos                        (0U)
#define GPIO_LCKR_LCK0_Msk                        (0x1UL << GPIO_LCKR_LCK0_Pos)                     /*!< 0x00000001 */
#define GPIO_LCKR_LCK0                            GPIO_LCKR_LCK0_Msk                                /*!< desc LCK0 */
#define GPIO_LCKR_LCK1_Pos                        (1U)
#define GPIO_LCKR_LCK1_Msk                        (0x1UL << GPIO_LCKR_LCK1_Pos)                     /*!< 0x00000002 */
#define GPIO_LCKR_LCK1                            GPIO_LCKR_LCK1_Msk                                /*!< desc LCK1 */
#define GPIO_LCKR_LCK2_Pos                        (2U)
#define GPIO_LCKR_LCK2_Msk                        (0x1UL << GPIO_LCKR_LCK2_Pos)                     /*!< 0x00000004 */
#define GPIO_LCKR_LCK2                            GPIO_LCKR_LCK2_Msk                                /*!< desc LCK2 */
#define GPIO_LCKR_LCK3_Pos                        (3U)
#define GPIO_LCKR_LCK3_Msk                        (0x1UL << GPIO_LCKR_LCK3_Pos)                     /*!< 0x00000008 */
#define GPIO_LCKR_LCK3                            GPIO_LCKR_LCK3_Msk                                /*!< desc LCK3 */
#define GPIO_LCKR_LCK4_Pos                        (4U)
#define GPIO_LCKR_LCK4_Msk                        (0x1UL << GPIO_LCKR_LCK4_Pos)                     /*!< 0x00000010 */
#define GPIO_LCKR_LCK4                            GPIO_LCKR_LCK4_Msk                                /*!< desc LCK4 */
#define GPIO_LCKR_LCK5_Pos                        (5U)
#define GPIO_LCKR_LCK5_Msk                        (0x1UL << GPIO_LCKR_LCK5_Pos)                     /*!< 0x00000020 */
#define GPIO_LCKR_LCK5                            GPIO_LCKR_LCK5_Msk                                /*!< desc LCK5 */
#define GPIO_LCKR_LCK6_Pos                        (6U)
#define GPIO_LCKR_LCK6_Msk                        (0x1UL << GPIO_LCKR_LCK6_Pos)                     /*!< 0x00000040 */
#define GPIO_LCKR_LCK6                            GPIO_LCKR_LCK6_Msk                                /*!< desc LCK6 */
#define GPIO_LCKR_LCK7_Pos                        (7U)
#define GPIO_LCKR_LCK7_Msk                        (0x1UL << GPIO_LCKR_LCK7_Pos)                     /*!< 0x00000080 */
#define GPIO_LCKR_LCK7                            GPIO_LCKR_LCK7_Msk                                /*!< desc LCK7 */
#define GPIO_LCKR_LCK8_Pos                        (8U)
#define GPIO_LCKR_LCK8_Msk                        (0x1UL << GPIO_LCKR_LCK8_Pos)                     /*!< 0x00000100 */
#define GPIO_LCKR_LCK8                            GPIO_LCKR_LCK8_Msk                                /*!< desc LCK8 */
#define GPIO_LCKR_LCK9_Pos                        (9U)
#define GPIO_LCKR_LCK9_Msk                        (0x1UL << GPIO_LCKR_LCK9_Pos)                     /*!< 0x00000200 */
#define GPIO_LCKR_LCK9                            GPIO_LCKR_LCK9_Msk                                /*!< desc LCK9 */
#define GPIO_LCKR_LCK10_Pos                       (10U)
#define GPIO_LCKR_LCK10_Msk                       (0x1UL << GPIO_LCKR_LCK10_Pos)                    /*!< 0x00000400 */
#define GPIO_LCKR_LCK10                           GPIO_LCKR_LCK10_Msk                               /*!< desc LCK10 */
#define GPIO_LCKR_LCK11_Pos                       (11U)
#define GPIO_LCKR_LCK11_Msk                       (0x1UL << GPIO_LCKR_LCK11_Pos)                    /*!< 0x00000800 */
#define GPIO_LCKR_LCK11                           GPIO_LCKR_LCK11_Msk                               /*!< desc LCK11 */
#define GPIO_LCKR_LCK12_Pos                       (12U)
#define GPIO_LCKR_LCK12_Msk                       (0x1UL << GPIO_LCKR_LCK12_Pos)                    /*!< 0x00001000 */
#define GPIO_LCKR_LCK12                           GPIO_LCKR_LCK12_Msk                               /*!< desc LCK12 */
#define GPIO_LCKR_LCK13_Pos                       (13U)
#define GPIO_LCKR_LCK13_Msk                       (0x1UL << GPIO_LCKR_LCK13_Pos)                    /*!< 0x00002000 */
#define GPIO_LCKR_LCK13                           GPIO_LCKR_LCK13_Msk                               /*!< desc LCK13 */
#define GPIO_LCKR_LCK14_Pos                       (14U)
#define GPIO_LCKR_LCK14_Msk                       (0x1UL << GPIO_LCKR_LCK14_Pos)                    /*!< 0x00004000 */
#define GPIO_LCKR_LCK14                           GPIO_LCKR_LCK14_Msk                               /*!< desc LCK14 */
#define GPIO_LCKR_LCK15_Pos                       (15U)
#define GPIO_LCKR_LCK15_Msk                       (0x1UL << GPIO_LCKR_LCK15_Pos)                    /*!< 0x00008000 */
#define GPIO_LCKR_LCK15                           GPIO_LCKR_LCK15_Msk                               /*!< desc LCK15 */
#define GPIO_LCKR_LCKK_Pos                        (16U)
#define GPIO_LCKR_LCKK_Msk                        (0x1UL << GPIO_LCKR_LCKK_Pos)                     /*!< 0x00010000 */
#define GPIO_LCKR_LCKK                            GPIO_LCKR_LCKK_Msk                                /*!< desc LCKK */

/*!< GPIO_AFRL */
#define GPIO_AFRL_AFSEL0_Pos                      (0U)
#define GPIO_AFRL_AFSEL0_Msk                      (0xFUL << GPIO_AFRL_AFSEL0_Pos)                   /*!< 0x0000000F */
#define GPIO_AFRL_AFSEL0                          GPIO_AFRL_AFSEL0_Msk                              /*!< AFSEL0[3:0] bits (desc AFSEL0) */
#define GPIO_AFRL_AFSEL0_0                        (0x1UL << GPIO_AFRL_AFSEL0_Pos)                   /*!< 0x00000001 */
#define GPIO_AFRL_AFSEL0_1                        (0x2UL << GPIO_AFRL_AFSEL0_Pos)                   /*!< 0x00000002 */
#define GPIO_AFRL_AFSEL0_2                        (0x4UL << GPIO_AFRL_AFSEL0_Pos)                   /*!< 0x00000004 */
#define GPIO_AFRL_AFSEL0_3                        (0x8UL << GPIO_AFRL_AFSEL0_Pos)                   /*!< 0x00000008 */

#define GPIO_AFRL_AFSEL1_Pos                      (4U)
#define GPIO_AFRL_AFSEL1_Msk                      (0xFUL << GPIO_AFRL_AFSEL1_Pos)                   /*!< 0x000000F0 */
#define GPIO_AFRL_AFSEL1                          GPIO_AFRL_AFSEL1_Msk                              /*!< AFSEL1[7:4] bits (desc AFSEL1) */
#define GPIO_AFRL_AFSEL1_0                        (0x1UL << GPIO_AFRL_AFSEL1_Pos)                   /*!< 0x00000010 */
#define GPIO_AFRL_AFSEL1_1                        (0x2UL << GPIO_AFRL_AFSEL1_Pos)                   /*!< 0x00000020 */
#define GPIO_AFRL_AFSEL1_2                        (0x4UL << GPIO_AFRL_AFSEL1_Pos)                   /*!< 0x00000040 */
#define GPIO_AFRL_AFSEL1_3                        (0x8UL << GPIO_AFRL_AFSEL1_Pos)                   /*!< 0x00000080 */

#define GPIO_AFRL_AFSEL2_Pos                      (8U)
#define GPIO_AFRL_AFSEL2_Msk                      (0xFUL << GPIO_AFRL_AFSEL2_Pos)                   /*!< 0x00000F00 */
#define GPIO_AFRL_AFSEL2                          GPIO_AFRL_AFSEL2_Msk                              /*!< AFSEL2[11:8] bits (desc AFSEL2) */
#define GPIO_AFRL_AFSEL2_0                        (0x1UL << GPIO_AFRL_AFSEL2_Pos)                   /*!< 0x00000100 */
#define GPIO_AFRL_AFSEL2_1                        (0x2UL << GPIO_AFRL_AFSEL2_Pos)                   /*!< 0x00000200 */
#define GPIO_AFRL_AFSEL2_2                        (0x4UL << GPIO_AFRL_AFSEL2_Pos)                   /*!< 0x00000400 */
#define GPIO_AFRL_AFSEL2_3                        (0x8UL << GPIO_AFRL_AFSEL2_Pos)                   /*!< 0x00000800 */

#define GPIO_AFRL_AFSEL3_Pos                      (12U)
#define GPIO_AFRL_AFSEL3_Msk                      (0xFUL << GPIO_AFRL_AFSEL3_Pos)                   /*!< 0x0000F000 */
#define GPIO_AFRL_AFSEL3                          GPIO_AFRL_AFSEL3_Msk                              /*!< AFSEL3[15:12] bits (desc AFSEL3) */
#define GPIO_AFRL_AFSEL3_0                        (0x1UL << GPIO_AFRL_AFSEL3_Pos)                   /*!< 0x00001000 */
#define GPIO_AFRL_AFSEL3_1                        (0x2UL << GPIO_AFRL_AFSEL3_Pos)                   /*!< 0x00002000 */
#define GPIO_AFRL_AFSEL3_2                        (0x4UL << GPIO_AFRL_AFSEL3_Pos)                   /*!< 0x00004000 */
#define GPIO_AFRL_AFSEL3_3                        (0x8UL << GPIO_AFRL_AFSEL3_Pos)                   /*!< 0x00008000 */

#define GPIO_AFRL_AFSEL4_Pos                      (16U)
#define GPIO_AFRL_AFSEL4_Msk                      (0xFUL << GPIO_AFRL_AFSEL4_Pos)                   /*!< 0x000F0000 */
#define GPIO_AFRL_AFSEL4                          GPIO_AFRL_AFSEL4_Msk                              /*!< AFSEL4[19:16] bits (desc AFSEL4) */
#define GPIO_AFRL_AFSEL4_0                        (0x1UL << GPIO_AFRL_AFSEL4_Pos)                   /*!< 0x00010000 */
#define GPIO_AFRL_AFSEL4_1                        (0x2UL << GPIO_AFRL_AFSEL4_Pos)                   /*!< 0x00020000 */
#define GPIO_AFRL_AFSEL4_2                        (0x4UL << GPIO_AFRL_AFSEL4_Pos)                   /*!< 0x00040000 */
#define GPIO_AFRL_AFSEL4_3                        (0x8UL << GPIO_AFRL_AFSEL4_Pos)                   /*!< 0x00080000 */

#define GPIO_AFRL_AFSEL5_Pos                      (20U)
#define GPIO_AFRL_AFSEL5_Msk                      (0xFUL << GPIO_AFRL_AFSEL5_Pos)                   /*!< 0x00F00000 */
#define GPIO_AFRL_AFSEL5                          GPIO_AFRL_AFSEL5_Msk                              /*!< AFSEL5[23:20] bits (desc AFSEL5) */
#define GPIO_AFRL_AFSEL5_0                        (0x1UL << GPIO_AFRL_AFSEL5_Pos)                   /*!< 0x00100000 */
#define GPIO_AFRL_AFSEL5_1                        (0x2UL << GPIO_AFRL_AFSEL5_Pos)                   /*!< 0x00200000 */
#define GPIO_AFRL_AFSEL5_2                        (0x4UL << GPIO_AFRL_AFSEL5_Pos)                   /*!< 0x00400000 */
#define GPIO_AFRL_AFSEL5_3                        (0x8UL << GPIO_AFRL_AFSEL5_Pos)                   /*!< 0x00800000 */

#define GPIO_AFRL_AFSEL6_Pos                      (24U)
#define GPIO_AFRL_AFSEL6_Msk                      (0xFUL << GPIO_AFRL_AFSEL6_Pos)                   /*!< 0x0F000000 */
#define GPIO_AFRL_AFSEL6                          GPIO_AFRL_AFSEL6_Msk                              /*!< AFSEL6[27:24] bits (desc AFSEL6) */
#define GPIO_AFRL_AFSEL6_0                        (0x1UL << GPIO_AFRL_AFSEL6_Pos)                   /*!< 0x01000000 */
#define GPIO_AFRL_AFSEL6_1                        (0x2UL << GPIO_AFRL_AFSEL6_Pos)                   /*!< 0x02000000 */
#define GPIO_AFRL_AFSEL6_2                        (0x4UL << GPIO_AFRL_AFSEL6_Pos)                   /*!< 0x04000000 */
#define GPIO_AFRL_AFSEL6_3                        (0x8UL << GPIO_AFRL_AFSEL6_Pos)                   /*!< 0x08000000 */

#define GPIO_AFRL_AFSEL7_Pos                      (28U)
#define GPIO_AFRL_AFSEL7_Msk                      (0xFUL << GPIO_AFRL_AFSEL7_Pos)                   /*!< 0xF0000000 */
#define GPIO_AFRL_AFSEL7                          GPIO_AFRL_AFSEL7_Msk                              /*!< AFSEL7[31:28] bits (desc AFSEL7) */
#define GPIO_AFRL_AFSEL7_0                        (0x1UL << GPIO_AFRL_AFSEL7_Pos)                   /*!< 0x10000000 */
#define GPIO_AFRL_AFSEL7_1                        (0x2UL << GPIO_AFRL_AFSEL7_Pos)                   /*!< 0x20000000 */
#define GPIO_AFRL_AFSEL7_2                        (0x4UL << GPIO_AFRL_AFSEL7_Pos)                   /*!< 0x40000000 */
#define GPIO_AFRL_AFSEL7_3                        (0x8UL << GPIO_AFRL_AFSEL7_Pos)                   /*!< 0x80000000 */


/*!< GPIO_AFRH */
#define GPIO_AFRH_AFSEL8_Pos                      (0U)
#define GPIO_AFRH_AFSEL8_Msk                      (0xFUL << GPIO_AFRH_AFSEL8_Pos)                   /*!< 0x0000000F */
#define GPIO_AFRH_AFSEL8                          GPIO_AFRH_AFSEL8_Msk                              /*!< AFSEL8[3:0] bits (desc AFSEL8) */
#define GPIO_AFRH_AFSEL8_0                        (0x1UL << GPIO_AFRH_AFSEL8_Pos)                   /*!< 0x00000001 */
#define GPIO_AFRH_AFSEL8_1                        (0x2UL << GPIO_AFRH_AFSEL8_Pos)                   /*!< 0x00000002 */
#define GPIO_AFRH_AFSEL8_2                        (0x4UL << GPIO_AFRH_AFSEL8_Pos)                   /*!< 0x00000004 */
#define GPIO_AFRH_AFSEL8_3                        (0x8UL << GPIO_AFRH_AFSEL8_Pos)                   /*!< 0x00000008 */

#define GPIO_AFRH_AFSEL9_Pos                      (4U)
#define GPIO_AFRH_AFSEL9_Msk                      (0xFUL << GPIO_AFRH_AFSEL9_Pos)                   /*!< 0x000000F0 */
#define GPIO_AFRH_AFSEL9                          GPIO_AFRH_AFSEL9_Msk                              /*!< AFSEL9[7:4] bits (desc AFSEL9) */
#define GPIO_AFRH_AFSEL9_0                        (0x1UL << GPIO_AFRH_AFSEL9_Pos)                   /*!< 0x00000010 */
#define GPIO_AFRH_AFSEL9_1                        (0x2UL << GPIO_AFRH_AFSEL9_Pos)                   /*!< 0x00000020 */
#define GPIO_AFRH_AFSEL9_2                        (0x4UL << GPIO_AFRH_AFSEL9_Pos)                   /*!< 0x00000040 */
#define GPIO_AFRH_AFSEL9_3                        (0x8UL << GPIO_AFRH_AFSEL9_Pos)                   /*!< 0x00000080 */

#define GPIO_AFRH_AFSEL10_Pos                     (8U)
#define GPIO_AFRH_AFSEL10_Msk                     (0xFUL << GPIO_AFRH_AFSEL10_Pos)                  /*!< 0x00000F00 */
#define GPIO_AFRH_AFSEL10                         GPIO_AFRH_AFSEL10_Msk                             /*!< AFSEL10[11:8] bits (desc AFSEL10) */
#define GPIO_AFRH_AFSEL10_0                       (0x1UL << GPIO_AFRH_AFSEL10_Pos)                  /*!< 0x00000100 */
#define GPIO_AFRH_AFSEL10_1                       (0x2UL << GPIO_AFRH_AFSEL10_Pos)                  /*!< 0x00000200 */
#define GPIO_AFRH_AFSEL10_2                       (0x4UL << GPIO_AFRH_AFSEL10_Pos)                  /*!< 0x00000400 */
#define GPIO_AFRH_AFSEL10_3                       (0x8UL << GPIO_AFRH_AFSEL10_Pos)                  /*!< 0x00000800 */

#define GPIO_AFRH_AFSEL11_Pos                     (12U)
#define GPIO_AFRH_AFSEL11_Msk                     (0xFUL << GPIO_AFRH_AFSEL11_Pos)                  /*!< 0x0000F000 */
#define GPIO_AFRH_AFSEL11                         GPIO_AFRH_AFSEL11_Msk                             /*!< AFSEL11[15:12] bits (desc AFSEL11) */
#define GPIO_AFRH_AFSEL11_0                       (0x1UL << GPIO_AFRH_AFSEL11_Pos)                  /*!< 0x00001000 */
#define GPIO_AFRH_AFSEL11_1                       (0x2UL << GPIO_AFRH_AFSEL11_Pos)                  /*!< 0x00002000 */
#define GPIO_AFRH_AFSEL11_2                       (0x4UL << GPIO_AFRH_AFSEL11_Pos)                  /*!< 0x00004000 */
#define GPIO_AFRH_AFSEL11_3                       (0x8UL << GPIO_AFRH_AFSEL11_Pos)                  /*!< 0x00008000 */

#define GPIO_AFRH_AFSEL12_Pos                     (16U)
#define GPIO_AFRH_AFSEL12_Msk                     (0xFUL << GPIO_AFRH_AFSEL12_Pos)                  /*!< 0x000F0000 */
#define GPIO_AFRH_AFSEL12                         GPIO_AFRH_AFSEL12_Msk                             /*!< AFSEL12[19:16] bits (desc AFSEL12) */
#define GPIO_AFRH_AFSEL12_0                       (0x1UL << GPIO_AFRH_AFSEL12_Pos)                  /*!< 0x00010000 */
#define GPIO_AFRH_AFSEL12_1                       (0x2UL << GPIO_AFRH_AFSEL12_Pos)                  /*!< 0x00020000 */
#define GPIO_AFRH_AFSEL12_2                       (0x4UL << GPIO_AFRH_AFSEL12_Pos)                  /*!< 0x00040000 */
#define GPIO_AFRH_AFSEL12_3                       (0x8UL << GPIO_AFRH_AFSEL12_Pos)                  /*!< 0x00080000 */

#define GPIO_AFRH_AFSEL13_Pos                     (20U)
#define GPIO_AFRH_AFSEL13_Msk                     (0xFUL << GPIO_AFRH_AFSEL13_Pos)                  /*!< 0x00F00000 */
#define GPIO_AFRH_AFSEL13                         GPIO_AFRH_AFSEL13_Msk                             /*!< AFSEL13[23:20] bits (desc AFSEL13) */
#define GPIO_AFRH_AFSEL13_0                       (0x1UL << GPIO_AFRH_AFSEL13_Pos)                  /*!< 0x00100000 */
#define GPIO_AFRH_AFSEL13_1                       (0x2UL << GPIO_AFRH_AFSEL13_Pos)                  /*!< 0x00200000 */
#define GPIO_AFRH_AFSEL13_2                       (0x4UL << GPIO_AFRH_AFSEL13_Pos)                  /*!< 0x00400000 */
#define GPIO_AFRH_AFSEL13_3                       (0x8UL << GPIO_AFRH_AFSEL13_Pos)                  /*!< 0x00800000 */

#define GPIO_AFRH_AFSEL14_Pos                     (24U)
#define GPIO_AFRH_AFSEL14_Msk                     (0xFUL << GPIO_AFRH_AFSEL14_Pos)                  /*!< 0x0F000000 */
#define GPIO_AFRH_AFSEL14                         GPIO_AFRH_AFSEL14_Msk                             /*!< AFSEL14[27:24] bits (desc AFSEL14) */
#define GPIO_AFRH_AFSEL14_0                       (0x1UL << GPIO_AFRH_AFSEL14_Pos)                  /*!< 0x01000000 */
#define GPIO_AFRH_AFSEL14_1                       (0x2UL << GPIO_AFRH_AFSEL14_Pos)                  /*!< 0x02000000 */
#define GPIO_AFRH_AFSEL14_2                       (0x4UL << GPIO_AFRH_AFSEL14_Pos)                  /*!< 0x04000000 */
#define GPIO_AFRH_AFSEL14_3                       (0x8UL << GPIO_AFRH_AFSEL14_Pos)                  /*!< 0x08000000 */

#define GPIO_AFRH_AFSEL15_Pos                     (28U)
#define GPIO_AFRH_AFSEL15_Msk                     (0xFUL << GPIO_AFRH_AFSEL15_Pos)                  /*!< 0xF0000000 */
#define GPIO_AFRH_AFSEL15                         GPIO_AFRH_AFSEL15_Msk                             /*!< AFSEL15[31:28] bits (desc AFSEL15) */
#define GPIO_AFRH_AFSEL15_0                       (0x1UL << GPIO_AFRH_AFSEL15_Pos)                  /*!< 0x10000000 */
#define GPIO_AFRH_AFSEL15_1                       (0x2UL << GPIO_AFRH_AFSEL15_Pos)                  /*!< 0x20000000 */
#define GPIO_AFRH_AFSEL15_2                       (0x4UL << GPIO_AFRH_AFSEL15_Pos)                  /*!< 0x40000000 */
#define GPIO_AFRH_AFSEL15_3                       (0x8UL << GPIO_AFRH_AFSEL15_Pos)                  /*!< 0x80000000 */


/*!< GPIO_BRR */
#define GPIO_BRR_BR0_Pos                          (0U)
#define GPIO_BRR_BR0_Msk                          (0x1UL << GPIO_BRR_BR0_Pos)                       /*!< 0x00000001 */
#define GPIO_BRR_BR0                              GPIO_BRR_BR0_Msk                                  /*!< desc BR0 */
#define GPIO_BRR_BR1_Pos                          (1U)
#define GPIO_BRR_BR1_Msk                          (0x1UL << GPIO_BRR_BR1_Pos)                       /*!< 0x00000002 */
#define GPIO_BRR_BR1                              GPIO_BRR_BR1_Msk                                  /*!< desc BR1 */
#define GPIO_BRR_BR2_Pos                          (2U)
#define GPIO_BRR_BR2_Msk                          (0x1UL << GPIO_BRR_BR2_Pos)                       /*!< 0x00000004 */
#define GPIO_BRR_BR2                              GPIO_BRR_BR2_Msk                                  /*!< desc BR2 */
#define GPIO_BRR_BR3_Pos                          (3U)
#define GPIO_BRR_BR3_Msk                          (0x1UL << GPIO_BRR_BR3_Pos)                       /*!< 0x00000008 */
#define GPIO_BRR_BR3                              GPIO_BRR_BR3_Msk                                  /*!< desc BR3 */
#define GPIO_BRR_BR4_Pos                          (4U)
#define GPIO_BRR_BR4_Msk                          (0x1UL << GPIO_BRR_BR4_Pos)                       /*!< 0x00000010 */
#define GPIO_BRR_BR4                              GPIO_BRR_BR4_Msk                                  /*!< desc BR4 */
#define GPIO_BRR_BR5_Pos                          (5U)
#define GPIO_BRR_BR5_Msk                          (0x1UL << GPIO_BRR_BR5_Pos)                       /*!< 0x00000020 */
#define GPIO_BRR_BR5                              GPIO_BRR_BR5_Msk                                  /*!< desc BR5 */
#define GPIO_BRR_BR6_Pos                          (6U)
#define GPIO_BRR_BR6_Msk                          (0x1UL << GPIO_BRR_BR6_Pos)                       /*!< 0x00000040 */
#define GPIO_BRR_BR6                              GPIO_BRR_BR6_Msk                                  /*!< desc BR6 */
#define GPIO_BRR_BR7_Pos                          (7U)
#define GPIO_BRR_BR7_Msk                          (0x1UL << GPIO_BRR_BR7_Pos)                       /*!< 0x00000080 */
#define GPIO_BRR_BR7                              GPIO_BRR_BR7_Msk                                  /*!< desc BR7 */
#define GPIO_BRR_BR8_Pos                          (8U)
#define GPIO_BRR_BR8_Msk                          (0x1UL << GPIO_BRR_BR8_Pos)                       /*!< 0x00000100 */
#define GPIO_BRR_BR8                              GPIO_BRR_BR8_Msk                                  /*!< desc BR8 */
#define GPIO_BRR_BR9_Pos                          (9U)
#define GPIO_BRR_BR9_Msk                          (0x1UL << GPIO_BRR_BR9_Pos)                       /*!< 0x00000200 */
#define GPIO_BRR_BR9                              GPIO_BRR_BR9_Msk                                  /*!< desc BR9 */
#define GPIO_BRR_BR10_Pos                         (10U)
#define GPIO_BRR_BR10_Msk                         (0x1UL << GPIO_BRR_BR10_Pos)                      /*!< 0x00000400 */
#define GPIO_BRR_BR10                             GPIO_BRR_BR10_Msk                                 /*!< desc BR10 */
#define GPIO_BRR_BR11_Pos                         (11U)
#define GPIO_BRR_BR11_Msk                         (0x1UL << GPIO_BRR_BR11_Pos)                      /*!< 0x00000800 */
#define GPIO_BRR_BR11                             GPIO_BRR_BR11_Msk                                 /*!< desc BR11 */
#define GPIO_BRR_BR12_Pos                         (12U)
#define GPIO_BRR_BR12_Msk                         (0x1UL << GPIO_BRR_BR12_Pos)                      /*!< 0x00001000 */
#define GPIO_BRR_BR12                             GPIO_BRR_BR12_Msk                                 /*!< desc BR12 */
#define GPIO_BRR_BR13_Pos                         (13U)
#define GPIO_BRR_BR13_Msk                         (0x1UL << GPIO_BRR_BR13_Pos)                      /*!< 0x00002000 */
#define GPIO_BRR_BR13                             GPIO_BRR_BR13_Msk                                 /*!< desc BR13 */
#define GPIO_BRR_BR14_Pos                         (14U)
#define GPIO_BRR_BR14_Msk                         (0x1UL << GPIO_BRR_BR14_Pos)                      /*!< 0x00004000 */
#define GPIO_BRR_BR14                             GPIO_BRR_BR14_Msk                                 /*!< desc BR14 */
#define GPIO_BRR_BR15_Pos                         (15U)
#define GPIO_BRR_BR15_Msk                         (0x1UL << GPIO_BRR_BR15_Pos)                      /*!< 0x00008000 */
#define GPIO_BRR_BR15                             GPIO_BRR_BR15_Msk                                 /*!< desc BR15 */

/*********************  Bits Define For Peripheral I2C  *********************/
/*!< I2C_CR1 */
#define I2C_CR1_PE_Pos                            (0U)
#define I2C_CR1_PE_Msk                            (0x1UL << I2C_CR1_PE_Pos)                         /*!< 0x00000001 */
#define I2C_CR1_PE                                I2C_CR1_PE_Msk                                    /*!< desc PE */
#define I2C_CR1_SMBUS_Pos                         (1U)
#define I2C_CR1_SMBUS_Msk                         (0x1UL << I2C_CR1_SMBUS_Pos)                      /*!< 0x00000002 */
#define I2C_CR1_SMBUS                             I2C_CR1_SMBUS_Msk                                 /*!< desc SMBUS */
#define I2C_CR1_SMBTYPE_Pos                       (3U)
#define I2C_CR1_SMBTYPE_Msk                       (0x1UL << I2C_CR1_SMBTYPE_Pos)                    /*!< 0x00000008 */
#define I2C_CR1_SMBTYPE                           I2C_CR1_SMBTYPE_Msk                               /*!< desc SMBTYPE */
#define I2C_CR1_ENARP_Pos                         (4U)
#define I2C_CR1_ENARP_Msk                         (0x1UL << I2C_CR1_ENARP_Pos)                      /*!< 0x00000010 */
#define I2C_CR1_ENARP                             I2C_CR1_ENARP_Msk                                 /*!< desc ENARP */
#define I2C_CR1_ENPEC_Pos                         (5U)
#define I2C_CR1_ENPEC_Msk                         (0x1UL << I2C_CR1_ENPEC_Pos)                      /*!< 0x00000020 */
#define I2C_CR1_ENPEC                             I2C_CR1_ENPEC_Msk                                 /*!< desc ENPEC */
#define I2C_CR1_ENGC_Pos                          (6U)
#define I2C_CR1_ENGC_Msk                          (0x1UL << I2C_CR1_ENGC_Pos)                       /*!< 0x00000040 */
#define I2C_CR1_ENGC                              I2C_CR1_ENGC_Msk                                  /*!< desc ENGC */
#define I2C_CR1_NOSTRETCH_Pos                     (7U)
#define I2C_CR1_NOSTRETCH_Msk                     (0x1UL << I2C_CR1_NOSTRETCH_Pos)                  /*!< 0x00000080 */
#define I2C_CR1_NOSTRETCH                         I2C_CR1_NOSTRETCH_Msk                             /*!< desc NOSTRETCH */
#define I2C_CR1_START_Pos                         (8U)
#define I2C_CR1_START_Msk                         (0x1UL << I2C_CR1_START_Pos)                      /*!< 0x00000100 */
#define I2C_CR1_START                             I2C_CR1_START_Msk                                 /*!< desc START */
#define I2C_CR1_STOP_Pos                          (9U)
#define I2C_CR1_STOP_Msk                          (0x1UL << I2C_CR1_STOP_Pos)                       /*!< 0x00000200 */
#define I2C_CR1_STOP                              I2C_CR1_STOP_Msk                                  /*!< desc STOP */
#define I2C_CR1_ACK_Pos                           (10U)
#define I2C_CR1_ACK_Msk                           (0x1UL << I2C_CR1_ACK_Pos)                        /*!< 0x00000400 */
#define I2C_CR1_ACK                               I2C_CR1_ACK_Msk                                   /*!< desc ACK */
#define I2C_CR1_POS_Pos                           (11U)
#define I2C_CR1_POS_Msk                           (0x1UL << I2C_CR1_POS_Pos)                        /*!< 0x00000800 */
#define I2C_CR1_POS                               I2C_CR1_POS_Msk                                   /*!< desc POS */
#define I2C_CR1_PEC_Pos                           (12U)
#define I2C_CR1_PEC_Msk                           (0x1UL << I2C_CR1_PEC_Pos)                        /*!< 0x00001000 */
#define I2C_CR1_PEC                               I2C_CR1_PEC_Msk                                   /*!< desc PEC */
#define I2C_CR1_ALERT_Pos                         (13U)
#define I2C_CR1_ALERT_Msk                         (0x1UL << I2C_CR1_ALERT_Pos)                      /*!< 0x00002000 */
#define I2C_CR1_ALERT                             I2C_CR1_ALERT_Msk                                 /*!< desc ALERT */
#define I2C_CR1_SWRST_Pos                         (15U)
#define I2C_CR1_SWRST_Msk                         (0x1UL << I2C_CR1_SWRST_Pos)                      /*!< 0x00008000 */
#define I2C_CR1_SWRST                             I2C_CR1_SWRST_Msk                                 /*!< desc SWRST */

/*!< I2C_CR2 */
#define I2C_CR2_FREQ_Pos                          (0U)
#define I2C_CR2_FREQ_Msk                          (0x3FUL << I2C_CR2_FREQ_Pos)                      /*!< 0x0000003F */
#define I2C_CR2_FREQ                              I2C_CR2_FREQ_Msk                                  /*!< FREQ[5:0] bits (desc FREQ) */
#define I2C_CR2_FREQ_0                            (0x1UL << I2C_CR2_FREQ_Pos)                       /*!< 0x00000001 */
#define I2C_CR2_FREQ_1                            (0x2UL << I2C_CR2_FREQ_Pos)                       /*!< 0x00000002 */
#define I2C_CR2_FREQ_2                            (0x4UL << I2C_CR2_FREQ_Pos)                       /*!< 0x00000004 */
#define I2C_CR2_FREQ_3                            (0x8UL << I2C_CR2_FREQ_Pos)                       /*!< 0x00000008 */
#define I2C_CR2_FREQ_4                            (0x10UL << I2C_CR2_FREQ_Pos)                      /*!< 0x00000010 */
#define I2C_CR2_FREQ_5                            (0x20UL << I2C_CR2_FREQ_Pos)                      /*!< 0x00000020 */

#define I2C_CR2_ITERREN_Pos                       (8U)
#define I2C_CR2_ITERREN_Msk                       (0x1UL << I2C_CR2_ITERREN_Pos)                    /*!< 0x00000100 */
#define I2C_CR2_ITERREN                           I2C_CR2_ITERREN_Msk                               /*!< desc ITERREN */
#define I2C_CR2_ITEVTEN_Pos                       (9U)
#define I2C_CR2_ITEVTEN_Msk                       (0x1UL << I2C_CR2_ITEVTEN_Pos)                    /*!< 0x00000200 */
#define I2C_CR2_ITEVTEN                           I2C_CR2_ITEVTEN_Msk                               /*!< desc ITEVTEN */
#define I2C_CR2_ITBUFEN_Pos                       (10U)
#define I2C_CR2_ITBUFEN_Msk                       (0x1UL << I2C_CR2_ITBUFEN_Pos)                    /*!< 0x00000400 */
#define I2C_CR2_ITBUFEN                           I2C_CR2_ITBUFEN_Msk                               /*!< desc ITBUFEN */
#define I2C_CR2_DMAEN_Pos                         (11U)
#define I2C_CR2_DMAEN_Msk                         (0x1UL << I2C_CR2_DMAEN_Pos)                      /*!< 0x00000800 */
#define I2C_CR2_DMAEN                             I2C_CR2_DMAEN_Msk                                 /*!< desc DMAEN */
#define I2C_CR2_LAST_Pos                          (12U)
#define I2C_CR2_LAST_Msk                          (0x1UL << I2C_CR2_LAST_Pos)                       /*!< 0x00001000 */
#define I2C_CR2_LAST                              I2C_CR2_LAST_Msk                                  /*!< desc LAST */

/*!< I2C_OAR1 */
#define I2C_OAR1_ADD0_Pos                         (0U)
#define I2C_OAR1_ADD0_Msk                         (0x1UL << I2C_OAR1_ADD0_Pos)                      /*!< 0x00000001 */
#define I2C_OAR1_ADD0                             I2C_OAR1_ADD0_Msk                                 /*!< desc ADD0 */

#define I2C_OAR1_ADD1_Pos                         (1U)
#define I2C_OAR1_ADD1_Msk                         (0x1UL << I2C_OAR1_ADD1_Pos)                      /*!< 0x00000002 */
#define I2C_OAR1_ADD1                             I2C_OAR1_ADD1_Msk                                 /*!< Bit 1 */
#define I2C_OAR1_ADD2_Pos                         (2U)
#define I2C_OAR1_ADD2_Msk                         (0x1UL << I2C_OAR1_ADD2_Pos)                      /*!< 0x00000004 */
#define I2C_OAR1_ADD2                             I2C_OAR1_ADD2_Msk                                 /*!< Bit 2 */
#define I2C_OAR1_ADD3_Pos                         (3U)
#define I2C_OAR1_ADD3_Msk                         (0x1UL << I2C_OAR1_ADD3_Pos)                      /*!< 0x00000008 */
#define I2C_OAR1_ADD3                             I2C_OAR1_ADD3_Msk                                 /*!< Bit 3 */
#define I2C_OAR1_ADD4_Pos                         (4U)
#define I2C_OAR1_ADD4_Msk                         (0x1UL << I2C_OAR1_ADD4_Pos)                      /*!< 0x00000010 */
#define I2C_OAR1_ADD4                             I2C_OAR1_ADD4_Msk                                 /*!< Bit 4 */
#define I2C_OAR1_ADD5_Pos                         (5U)
#define I2C_OAR1_ADD5_Msk                         (0x1UL << I2C_OAR1_ADD5_Pos)                      /*!< 0x00000020 */
#define I2C_OAR1_ADD5                             I2C_OAR1_ADD5_Msk                                 /*!< Bit 5 */
#define I2C_OAR1_ADD6_Pos                         (6U)
#define I2C_OAR1_ADD6_Msk                         (0x1UL << I2C_OAR1_ADD6_Pos)                      /*!< 0x00000040 */
#define I2C_OAR1_ADD6                             I2C_OAR1_ADD6_Msk                                 /*!< Bit 6 */
#define I2C_OAR1_ADD7_Pos                         (7U)
#define I2C_OAR1_ADD7_Msk                         (0x1UL << I2C_OAR1_ADD7_Pos)                      /*!< 0x00000080 */
#define I2C_OAR1_ADD7                             I2C_OAR1_ADD7_Msk                                 /*!< Bit 7 */
#define I2C_OAR1_ADD8_Pos                         (8U)
#define I2C_OAR1_ADD8_Msk                         (0x1UL << I2C_OAR1_ADD8_Pos)                      /*!< 0x00000100 */
#define I2C_OAR1_ADD8                             I2C_OAR1_ADD8_Msk                                 /*!< Bit 8 */
#define I2C_OAR1_ADD9_Pos                         (9U)
#define I2C_OAR1_ADD9_Msk                         (0x1UL << I2C_OAR1_ADD9_Pos)                      /*!< 0x00000200 */
#define I2C_OAR1_ADD9                             I2C_OAR1_ADD9_Msk                                 /*!< Bit 9 */

#define I2C_OAR1_ADD1_7                           0x000000FEU                                       /*!< Interface Address */
#define I2C_OAR1_ADD8_9                           0x00000300U                                       /*!< Interface Address */
#define I2C_OAR1_ADD_Pos                          (1U)
#define I2C_OAR1_ADD_Msk                          (0x1FFUL << I2C_OAR1_ADD_Pos)                     /*!< 0x000003FE */
#define I2C_OAR1_ADD                              I2C_OAR1_ADD_Msk                                  /*!< ADD[7:1] bits (desc ADD) */
#define I2C_OAR1_ADDMODE_Pos                      (15U)
#define I2C_OAR1_ADDMODE_Msk                      (0x1UL << I2C_OAR1_ADDMODE_Pos)                   /*!< 0x00008000 */
#define I2C_OAR1_ADDMODE                          I2C_OAR1_ADDMODE_Msk                              /*!< desc ADDMODE */

/*!< I2C_OAR2 */
#define I2C_OAR2_ENDUAL_Pos                       (0U)
#define I2C_OAR2_ENDUAL_Msk                       (0x1UL << I2C_OAR2_ENDUAL_Pos)                    /*!< 0x00000001 */
#define I2C_OAR2_ENDUAL                           I2C_OAR2_ENDUAL_Msk                               /*!< desc ENDUAL */
#define I2C_OAR2_ADD2_Pos                         (1U)
#define I2C_OAR2_ADD2_Msk                         (0x7FUL << I2C_OAR2_ADD2_Pos)                     /*!< 0x000000FE */
#define I2C_OAR2_ADD2                             I2C_OAR2_ADD2_Msk                                 /*!< ADD2[7:1] bits (desc ADD2) */

/*!< I2C_DR */
#define I2C_DR_DR_Pos                             (0U)
#define I2C_DR_DR_Msk                             (0xFFUL << I2C_DR_DR_Pos)                         /*!< 0x000000FF */
#define I2C_DR_DR                                 I2C_DR_DR_Msk                                     /*!< DR[7:0] bits (desc DR) */

/*!< I2C_SR1 */
#define I2C_SR1_SB_Pos                            (0U)
#define I2C_SR1_SB_Msk                            (0x1UL << I2C_SR1_SB_Pos)                         /*!< 0x00000001 */
#define I2C_SR1_SB                                I2C_SR1_SB_Msk                                    /*!< desc SB */
#define I2C_SR1_ADDR_Pos                          (1U)
#define I2C_SR1_ADDR_Msk                          (0x1UL << I2C_SR1_ADDR_Pos)                       /*!< 0x00000002 */
#define I2C_SR1_ADDR                              I2C_SR1_ADDR_Msk                                  /*!< desc ADDR */
#define I2C_SR1_BTF_Pos                           (2U)
#define I2C_SR1_BTF_Msk                           (0x1UL << I2C_SR1_BTF_Pos)                        /*!< 0x00000004 */
#define I2C_SR1_BTF                               I2C_SR1_BTF_Msk                                   /*!< desc BTF */
#define I2C_SR1_ADD10_Pos                         (3U)
#define I2C_SR1_ADD10_Msk                         (0x1UL << I2C_SR1_ADD10_Pos)                      /*!< 0x00000008 */
#define I2C_SR1_ADD10                             I2C_SR1_ADD10_Msk                                 /*!< desc ADD10 */
#define I2C_SR1_STOPF_Pos                         (4U)
#define I2C_SR1_STOPF_Msk                         (0x1UL << I2C_SR1_STOPF_Pos)                      /*!< 0x00000010 */
#define I2C_SR1_STOPF                             I2C_SR1_STOPF_Msk                                 /*!< desc STOPF */
#define I2C_SR1_RXNE_Pos                          (6U)
#define I2C_SR1_RXNE_Msk                          (0x1UL << I2C_SR1_RXNE_Pos)                       /*!< 0x00000040 */
#define I2C_SR1_RXNE                              I2C_SR1_RXNE_Msk                                  /*!< desc RXNE */
#define I2C_SR1_TXE_Pos                           (7U)
#define I2C_SR1_TXE_Msk                           (0x1UL << I2C_SR1_TXE_Pos)                        /*!< 0x00000080 */
#define I2C_SR1_TXE                               I2C_SR1_TXE_Msk                                   /*!< desc TXE */
#define I2C_SR1_BERR_Pos                          (8U)
#define I2C_SR1_BERR_Msk                          (0x1UL << I2C_SR1_BERR_Pos)                       /*!< 0x00000100 */
#define I2C_SR1_BERR                              I2C_SR1_BERR_Msk                                  /*!< desc BERR */
#define I2C_SR1_ARLO_Pos                          (9U)
#define I2C_SR1_ARLO_Msk                          (0x1UL << I2C_SR1_ARLO_Pos)                       /*!< 0x00000200 */
#define I2C_SR1_ARLO                              I2C_SR1_ARLO_Msk                                  /*!< desc ARLO */
#define I2C_SR1_AF_Pos                            (10U)
#define I2C_SR1_AF_Msk                            (0x1UL << I2C_SR1_AF_Pos)                         /*!< 0x00000400 */
#define I2C_SR1_AF                                I2C_SR1_AF_Msk                                    /*!< desc AF */
#define I2C_SR1_OVR_Pos                           (11U)
#define I2C_SR1_OVR_Msk                           (0x1UL << I2C_SR1_OVR_Pos)                        /*!< 0x00000800 */
#define I2C_SR1_OVR                               I2C_SR1_OVR_Msk                                   /*!< desc OVR */
#define I2C_SR1_PECERR_Pos                        (12U)
#define I2C_SR1_PECERR_Msk                        (0x1UL << I2C_SR1_PECERR_Pos)                     /*!< 0x00001000 */
#define I2C_SR1_PECERR                            I2C_SR1_PECERR_Msk                                /*!< desc PECERR */
#define I2C_SR1_TIMEOUT_Pos                       (14U)
#define I2C_SR1_TIMEOUT_Msk                       (0x1UL << I2C_SR1_TIMEOUT_Pos)                    /*!< 0x00004000 */
#define I2C_SR1_TIMEOUT                           I2C_SR1_TIMEOUT_Msk                               /*!< desc TIMEOUT */
#define I2C_SR1_SMBALERT_Pos                      (15U)
#define I2C_SR1_SMBALERT_Msk                      (0x1UL << I2C_SR1_SMBALERT_Pos)                   /*!< 0x00008000 */
#define I2C_SR1_SMBALERT                          I2C_SR1_SMBALERT_Msk                              /*!< desc SMBALERT */

/*!< I2C_SR2 */
#define I2C_SR2_MSL_Pos                           (0U)
#define I2C_SR2_MSL_Msk                           (0x1UL << I2C_SR2_MSL_Pos)                        /*!< 0x00000001 */
#define I2C_SR2_MSL                               I2C_SR2_MSL_Msk                                   /*!< desc MSL */
#define I2C_SR2_BUSY_Pos                          (1U)
#define I2C_SR2_BUSY_Msk                          (0x1UL << I2C_SR2_BUSY_Pos)                       /*!< 0x00000002 */
#define I2C_SR2_BUSY                              I2C_SR2_BUSY_Msk                                  /*!< desc BUSY */
#define I2C_SR2_TRA_Pos                           (2U)
#define I2C_SR2_TRA_Msk                           (0x1UL << I2C_SR2_TRA_Pos)                        /*!< 0x00000004 */
#define I2C_SR2_TRA                               I2C_SR2_TRA_Msk                                   /*!< desc TRA */
#define I2C_SR2_GENCALL_Pos                       (4U)
#define I2C_SR2_GENCALL_Msk                       (0x1UL << I2C_SR2_GENCALL_Pos)                    /*!< 0x00000010 */
#define I2C_SR2_GENCALL                           I2C_SR2_GENCALL_Msk                               /*!< desc GENCALL */
#define I2C_SR2_SMBDEFAULT_Pos                    (5U)
#define I2C_SR2_SMBDEFAULT_Msk                    (0x1UL << I2C_SR2_SMBDEFAULT_Pos)                 /*!< 0x00000020 */
#define I2C_SR2_SMBDEFAULT                        I2C_SR2_SMBDEFAULT_Msk                            /*!< desc SMBDEFAULT */
#define I2C_SR2_SMBHOST_Pos                       (6U)
#define I2C_SR2_SMBHOST_Msk                       (0x1UL << I2C_SR2_SMBHOST_Pos)                    /*!< 0x00000040 */
#define I2C_SR2_SMBHOST                           I2C_SR2_SMBHOST_Msk                               /*!< desc SMBHOST */
#define I2C_SR2_DUALF_Pos                         (7U)
#define I2C_SR2_DUALF_Msk                         (0x1UL << I2C_SR2_DUALF_Pos)                      /*!< 0x00000080 */
#define I2C_SR2_DUALF                             I2C_SR2_DUALF_Msk                                 /*!< desc DUALF */
#define I2C_SR2_PEC_Pos                           (8U)
#define I2C_SR2_PEC_Msk                           (0xFFUL << I2C_SR2_PEC_Pos)                       /*!< 0x0000FF00 */
#define I2C_SR2_PEC                               I2C_SR2_PEC_Msk                                   /*!< PEC[15:8] bits (desc PEC) */

/*!< I2C_CCR */
#define I2C_CCR_CCR_Pos                           (0U)
#define I2C_CCR_CCR_Msk                           (0xFFFUL << I2C_CCR_CCR_Pos)                      /*!< 0x00000FFF */
#define I2C_CCR_CCR                               I2C_CCR_CCR_Msk                                   /*!< CCR[11:0] bits (desc CCR) */
#define I2C_CCR_DUTY_Pos                          (14U)
#define I2C_CCR_DUTY_Msk                          (0x1UL << I2C_CCR_DUTY_Pos)                       /*!< 0x00004000 */
#define I2C_CCR_DUTY                              I2C_CCR_DUTY_Msk                                  /*!< desc DUTY */
#define I2C_CCR_FS_Pos                            (15U)
#define I2C_CCR_FS_Msk                            (0x1UL << I2C_CCR_FS_Pos)                         /*!< 0x00008000 */
#define I2C_CCR_FS                                I2C_CCR_FS_Msk                                    /*!< desc FS */

/*!< I2C_TRISE */
#define I2C_TRISE_TRISE_Pos                       (0U)
#define I2C_TRISE_TRISE_Msk                       (0x3FUL << I2C_TRISE_TRISE_Pos)                   /*!< 0x0000003F */
#define I2C_TRISE_TRISE                           I2C_TRISE_TRISE_Msk                               /*!< TRISE[5:0] bits (desc TRISE) */

/******************************************************************************/
/*                                                                            */
/*                        Independent WATCHDOG (IWDG)                         */
/*                                                                            */
/******************************************************************************/
/*********************  Bits Define For Peripheral IWDG  *********************/
/*!< IWDG_KR */
#define IWDG_KR_KEY_Pos                           (0U)
#define IWDG_KR_KEY_Msk                           (0xFFFFUL << IWDG_KR_KEY_Pos)                     /*!< 0x0000FFFF */
#define IWDG_KR_KEY                               IWDG_KR_KEY_Msk                                   /*!< KEY[15:0] bits (desc KEY) */

/*!< IWDG_PR */
#define IWDG_PR_PR_Pos                            (0U)
#define IWDG_PR_PR_Msk                            (0x7UL << IWDG_PR_PR_Pos)                         /*!< 0x00000007 */
#define IWDG_PR_PR                                IWDG_PR_PR_Msk                                    /*!< PR[2:0] bits (desc PR) */
#define IWDG_PR_PR_0                              (0x1UL << IWDG_PR_PR_Pos)                         /*!< 0x00000001 */
#define IWDG_PR_PR_1                              (0x2UL << IWDG_PR_PR_Pos)                         /*!< 0x00000002 */
#define IWDG_PR_PR_2                              (0x4UL << IWDG_PR_PR_Pos)                         /*!< 0x00000004 */


/*!< IWDG_RLR */
#define IWDG_RLR_RL_Pos                           (0U)
#define IWDG_RLR_RL_Msk                           (0xFFFUL << IWDG_RLR_RL_Pos)                      /*!< 0x00000FFF */
#define IWDG_RLR_RL                               IWDG_RLR_RL_Msk                                   /*!< RL[11:0] bits (desc RL) */

/*!< IWDG_SR */
#define IWDG_SR_PVU_Pos                           (0U)
#define IWDG_SR_PVU_Msk                           (0x1UL << IWDG_SR_PVU_Pos)                        /*!< 0x00000001 */
#define IWDG_SR_PVU                               IWDG_SR_PVU_Msk                                   /*!< desc PVU */
#define IWDG_SR_RVU_Pos                           (1U)
#define IWDG_SR_RVU_Msk                           (0x1UL << IWDG_SR_RVU_Pos)                        /*!< 0x00000002 */
#define IWDG_SR_RVU                               IWDG_SR_RVU_Msk                                   /*!< desc RVU */

/*********************  Bits Define For Peripheral PWR  *********************/
/*!< PWR_CR */
#define PWR_CR_LPDS_Pos                           (0U)
#define PWR_CR_LPDS_Msk                           (0x1UL << PWR_CR_LPDS_Pos)                        /*!< 0x00000001 */
#define PWR_CR_LPDS                               PWR_CR_LPDS_Msk                                   /*!< desc LPDS */

#define PWR_CR_PDDS_Pos                           (1U)
#define PWR_CR_PDDS_Msk                           (0x1UL << PWR_CR_PDDS_Pos)                        /*!< 0x00000002 */
#define PWR_CR_PDDS                               PWR_CR_PDDS_Msk                                   /*!< desc PDDS */

#define PWR_CR_CWUF_Pos                           (2U)
#define PWR_CR_CWUF_Msk                           (0x1UL << PWR_CR_CWUF_Pos)                        /*!< 0x00000004 */
#define PWR_CR_CWUF                               PWR_CR_CWUF_Msk                                   /*!< desc CWUF */

#define PWR_CR_CSBF_Pos                           (3U)
#define PWR_CR_CSBF_Msk                           (0x1UL << PWR_CR_CSBF_Pos)                        /*!< 0x00000008 */
#define PWR_CR_CSBF                               PWR_CR_CSBF_Msk                                   /*!< desc CSBF */

#define PWR_CR_PVDE_Pos                           (4U)
#define PWR_CR_PVDE_Msk                           (0x1UL << PWR_CR_PVDE_Pos)                        /*!< 0x00000010 */
#define PWR_CR_PVDE                               PWR_CR_PVDE_Msk                                   /*!< desc PVDE */

#define PWR_CR_PLS_Pos                            (5U)
#define PWR_CR_PLS_Msk                            (0x7UL << PWR_CR_PLS_Pos)                         /*!< 0x000000E0 */
#define PWR_CR_PLS                                PWR_CR_PLS_Msk                                    /*!< PLS[7:5] bits (desc PLS) */
#define PWR_CR_PLS_0                              (0x1UL << PWR_CR_PLS_Pos)                         /*!< 0x00000020 */
#define PWR_CR_PLS_1                              (0x2UL << PWR_CR_PLS_Pos)                         /*!< 0x00000040 */
#define PWR_CR_PLS_2                              (0x4UL << PWR_CR_PLS_Pos)                         /*!< 0x00000080 */

/*!< PVD level configuration */
#define PWR_CR_PLS_LEV0                           0x00000000U                                       /*!< PVD level 1.8V */
#define PWR_CR_PLS_LEV1                           0x00000020U                                       /*!< PVD level 2.0V */
#define PWR_CR_PLS_LEV2                           0x00000040U                                       /*!< PVD level 2.2V */
#define PWR_CR_PLS_LEV3                           0x00000060U                                       /*!< PVD level 2.4V */
#define PWR_CR_PLS_LEV4                           0x00000080U                                       /*!< PVD level 2.6V */
#define PWR_CR_PLS_LEV5                           0x000000A0U                                       /*!< PVD level 2.8V */
#define PWR_CR_PLS_LEV6                           0x000000C0U                                       /*!< PVD level 3.0V */
#define PWR_CR_PLS_LEV7                           0x000000E0U                                       /*!< PVD level 3.2V */
/* Legacy defines */
#define PWR_CR_PLS_1V8                            PWR_CR_PLS_LEV0
#define PWR_CR_PLS_2V0                            PWR_CR_PLS_LEV1
#define PWR_CR_PLS_2V2                            PWR_CR_PLS_LEV2
#define PWR_CR_PLS_2V4                            PWR_CR_PLS_LEV3
#define PWR_CR_PLS_2V6                            PWR_CR_PLS_LEV4
#define PWR_CR_PLS_2V8                            PWR_CR_PLS_LEV5
#define PWR_CR_PLS_3V0                            PWR_CR_PLS_LEV6
#define PWR_CR_PLS_3V2                            PWR_CR_PLS_LEV7

#define PWR_CR_DBP_Pos                            (8U)
#define PWR_CR_DBP_Msk                            (0x1UL << PWR_CR_DBP_Pos)                         /*!< 0x00000100 */
#define PWR_CR_DBP                                PWR_CR_DBP_Msk                                    /*!< desc DBP */

#define PWR_CR_BKPVR_VOS_Pos                      (11U)
#define PWR_CR_BKPVR_VOS_Msk                      (0x3UL << PWR_CR_BKPVR_VOS_Pos)                   /*!< 0x00018000 */
#define PWR_CR_BKPVR_VOS                          PWR_CR_BKPVR_VOS_Msk                              /*!< desc BKPVR_VOS */
#define PWR_CR_BKPVR_VOS_0                        (0x1UL << PWR_CR_BKPVR_VOS_Pos)
#define PWR_CR_BKPVR_VOS_1                        (0x2UL << PWR_CR_BKPVR_VOS_Pos)

#define PWR_CR_VOS_Pos                            (13U)
#define PWR_CR_VOS_Msk                            (0x3UL << PWR_CR_VOS_Pos)                         /*!< 0x00006000 */
#define PWR_CR_VOS                                PWR_CR_VOS_Msk                                    /*!< VOS[14:13] bits (desc VOS) */
#define PWR_CR_VOS_0                              (0x1UL << PWR_CR_VOS_Pos)                         /*!< 0x00002000 */
#define PWR_CR_VOS_1                              (0x2UL << PWR_CR_VOS_Pos)                         /*!< 0x00004000 */

#define PWR_CR_HSION_CTRL_Pos                     (16U)
#define PWR_CR_HSION_CTRL_Msk                     (0x1UL << PWR_CR_HSION_CTRL_Pos)                  /*!< 0x00010000 */
#define PWR_CR_HSION_CTRL                         PWR_CR_HSION_CTRL_Msk                             /*!< desc HSION_CTRL */

#define PWR_CR_FLS_WUPT_Pos                       (17U)
#define PWR_CR_FLS_WUPT_Msk                       (0x3UL << PWR_CR_FLS_WUPT_Pos)                    /*!< 0x00060000 */
#define PWR_CR_FLS_WUPT                           PWR_CR_FLS_WUPT_Msk                               /*!< FLS_WUPT[18:17] bits (desc FLS_WUPT) */
#define PWR_CR_FLS_WUPT_0                         (0x1UL << PWR_CR_FLS_WUPT_Pos)                    /*!< 0x00020000 */
#define PWR_CR_FLS_WUPT_1                         (0x2UL << PWR_CR_FLS_WUPT_Pos)                    /*!< 0x00040000 */

#define PWR_CR_STDBY_MRRDY_WAIT_Pos               (20U)
#define PWR_CR_STDBY_MRRDY_WAIT_Msk               (0x3UL << PWR_CR_STDBY_MRRDY_WAIT_Pos)            /*!< 0x00300000 */
#define PWR_CR_STDBY_MRRDY_WAIT                   PWR_CR_STDBY_MRRDY_WAIT_Msk                       /*!< STDBY_MRRDY_WAIT[21:20] bits (desc STDBY_MRRDY_WAIT) */
#define PWR_CR_STDBY_MRRDY_WAIT_0                 (0x1UL << PWR_CR_STDBY_MRRDY_WAIT_Pos)            /*!< 0x00100000 */
#define PWR_CR_STDBY_MRRDY_WAIT_1                 (0x2UL << PWR_CR_STDBY_MRRDY_WAIT_Pos)            /*!< 0x00200000 */

/*!< PWR_CSR */
#define PWR_CSR_WUF_Pos                           (0U)
#define PWR_CSR_WUF_Msk                           (0x1UL << PWR_CSR_WUF_Pos)                        /*!< 0x00000001 */
#define PWR_CSR_WUF                               PWR_CSR_WUF_Msk                                   /*!< desc WUF */

#define PWR_CSR_SBF_Pos                           (1U)
#define PWR_CSR_SBF_Msk                           (0x1UL << PWR_CSR_SBF_Pos)                        /*!< 0x00000002 */
#define PWR_CSR_SBF                               PWR_CSR_SBF_Msk                                   /*!< desc SBF */

#define PWR_CSR_PVDO_Pos                          (2U)
#define PWR_CSR_PVDO_Msk                          (0x1UL << PWR_CSR_PVDO_Pos)                       /*!< 0x00000004 */
#define PWR_CSR_PVDO                              PWR_CSR_PVDO_Msk                                  /*!< desc PVDO */

#define PWR_CSR_EWUP1_Pos                         (8U)
#define PWR_CSR_EWUP1_Msk                         (0x1UL << PWR_CSR_EWUP1_Pos)                      /*!< 0x00000100 */
#define PWR_CSR_EWUP1                             PWR_CSR_EWUP1_Msk                                 /*!< desc EWUP1 */

#define PWR_CSR_EWUP2_Pos                         (9U)
#define PWR_CSR_EWUP2_Msk                         (0x1UL << PWR_CSR_EWUP2_Pos)                      /*!< 0x00000200 */
#define PWR_CSR_EWUP2                             PWR_CSR_EWUP2_Msk                                 /*!< desc EWUP2 */

#define PWR_CSR_EWUP3_Pos                         (10U)
#define PWR_CSR_EWUP3_Msk                         (0x1UL << PWR_CSR_EWUP3_Pos)                      /*!< 0x00000400 */
#define PWR_CSR_EWUP3                             PWR_CSR_EWUP3_Msk                                 /*!< desc EWUP3 */

#define PWR_CSR_EWUP4_Pos                         (11U)
#define PWR_CSR_EWUP4_Msk                         (0x1UL << PWR_CSR_EWUP4_Pos)                      /*!< 0x00000800 */
#define PWR_CSR_EWUP4                             PWR_CSR_EWUP4_Msk                                 /*!< desc EWUP4 */

#define PWR_CSR_EWUP5_Pos                         (12U)
#define PWR_CSR_EWUP5_Msk                         (0x1UL << PWR_CSR_EWUP5_Pos)                      /*!< 0x00001000 */
#define PWR_CSR_EWUP5                             PWR_CSR_EWUP5_Msk                                 /*!< desc EWUP5 */

#define PWR_CSR_FLTEN_Pos                         (16U)
#define PWR_CSR_FLTEN_Msk                         (0x1UL << PWR_CSR_FLTEN_Pos)                      /*!< 0x00010000 */
#define PWR_CSR_FLTEN                             PWR_CSR_FLTEN_Msk                                 /*!< desc FLTEN */

#define PWR_CSR_FLT_CTRL_Pos                      (17U)
#define PWR_CSR_FLT_CTRL_Msk                      (0x7UL << PWR_CSR_FLT_CTRL_Pos)                   /*!< 0x000E0000 */
#define PWR_CSR_FLT_CTRL                          PWR_CSR_FLT_CTRL_Msk                              /*!< FLT_CTRL[19:17] bits (desc FLT_CTRL) */
#define PWR_CSR_FLT_CTRL_0                        (0x1UL << PWR_CSR_FLT_CTRL_Pos)                   /*!< 0x00020000 */
#define PWR_CSR_FLT_CTRL_1                        (0x2UL << PWR_CSR_FLT_CTRL_Pos)                   /*!< 0x00040000 */
#define PWR_CSR_FLT_CTRL_2                        (0x4UL << PWR_CSR_FLT_CTRL_Pos)                   /*!< 0x00080000 */

/*!< ESMC CR */
#define ESMC_CR_SOFTRST_Pos                       (0U)
#define ESMC_CR_SOFTRST_Msk                       (0x1UL << ESMC_CR_SOFTRST_Pos)
#define ESMC_CR_SOFTRST                           ESMC_CR_SOFTRST_Msk

#define ESMC_CR_2QSPI_Pos                         (2U)
#define ESMC_CR_2QSPI_Msk                         (0x1UL << ESMC_CR_2QSPI_Pos)
#define ESMC_CR_2QSPI                             ESMC_CR_2QSPI_Msk

#define ESMC_CR_DMAEN_Pos                         (3U)
#define ESMC_CR_DMAEN_Msk                         (0x1UL << ESMC_CR_DMAEN_Pos)
#define ESMC_CR_DMAEN                             ESMC_CR_DMAEN_Msk

#define ESMC_CR_GIE_Pos                           (4U)
#define ESMC_CR_GIE_Msk                           (0x1UL << ESMC_CR_GIE_Pos)
#define ESMC_CR_GIE                               ESMC_CR_GIE_Msk

#define ESMC_CR_DECEN_Pos                         (5U)
#define ESMC_CR_DECEN_Msk                         (0x1UL << ESMC_CR_DECEN_Pos)
#define ESMC_CR_DECEN                             ESMC_CR_DECEN_Msk

#define ESMC_CR_XIPEN_Pos                         (6U)
#define ESMC_CR_XIPEN_Msk                         (0x1UL << ESMC_CR_XIPEN_Pos)
#define ESMC_CR_XIPEN                             ESMC_CR_XIPEN_Msk

#define ESMC_CR_SPIEN_Pos                         (7U)
#define ESMC_CR_SPIEN_Msk                         (0x1UL << ESMC_CR_SPIEN_Pos)
#define ESMC_CR_SPIEN                             ESMC_CR_SPIEN_Msk

/*!< ESMC CR2 */
#define ESMC_CR2_CPHA_Pos                         (0U)
#define ESMC_CR2_CPHA_Msk                         (0x1UL << ESMC_CR2_CPHA_Pos)
#define ESMC_CR2_CPHA                             ESMC_CR2_CPHA_Msk

#define ESMC_CR2_CPOL_Pos                         (1U)
#define ESMC_CR2_CPOL_Msk                         (0x1UL << ESMC_CR2_CPOL_Pos)
#define ESMC_CR2_CPOL                             ESMC_CR2_CPOL_Msk

#define ESMC_CR2_SLAVEEN_Pos                      (4U)
#define ESMC_CR2_SLAVEEN_Msk                      (0x1UL << ESMC_CR2_SLAVEEN_Pos)
#define ESMC_CR2_SLAVEEN                          ESMC_CR2_SLAVEEN_Msk

#define ESMC_CR2_WREN_Pos                         (5U)
#define ESMC_CR2_WREN_Msk                         (0x1UL << ESMC_CR2_WREN_Pos)
#define ESMC_CR2_WREN                             ESMC_CR2_WREN_Msk

/*!< ESMC TCR */
#define ESMC_TCR_TCR_Pos                     (0U)
#define ESMC_TCR_TCR_Msk                     (0xFFUL << ESMC_TCR_TCR_Pos)
#define ESMC_TCR_TCR                          ESMC_TCR_TCR_Msk
#define ESMC_TCR_TCR_0                       (0x1UL << ESMC_TCR_TCR_Pos)
#define ESMC_TCR_TCR_1                       (0x2UL << ESMC_TCR_TCR_Pos)
#define ESMC_TCR_TCR_2                       (0x4UL << ESMC_TCR_TCR_Pos)
#define ESMC_TCR_TCR_3                       (0x8UL << ESMC_TCR_TCR_Pos)
#define ESMC_TCR_TCR_4                       (0x10UL << ESMC_TCR_TCR_Pos)
#define ESMC_TCR_TCR_5                       (0x20UL << ESMC_TCR_TCR_Pos)
#define ESMC_TCR_TCR_6                       (0x40UL << ESMC_TCR_TCR_Pos)
#define ESMC_TCR_TCR_7                       (0x80UL << ESMC_TCR_TCR_Pos)

/*!< ESMC BAUD */
#define ESMC_BAUD_BAUD_Pos                     (0U)
#define ESMC_BAUD_BAUD_Msk                     (0xFFUL << ESMC_BAUD_BAUD_Pos)
#define ESMC_BAUD_BAUD                          ESMC_BAUD_BAUD_Msk
#define ESMC_BAUD_BAUD_0                       (0x1UL << ESMC_BAUD_BAUD_Pos)
#define ESMC_BAUD_BAUD_1                       (0x2UL << ESMC_BAUD_BAUD_Pos)
#define ESMC_BAUD_BAUD_2                       (0x4UL << ESMC_BAUD_BAUD_Pos)
#define ESMC_BAUD_BAUD_3                       (0x8UL << ESMC_BAUD_BAUD_Pos)
#define ESMC_BAUD_BAUD_4                       (0x10UL << ESMC_BAUD_BAUD_Pos)
#define ESMC_BAUD_BAUD_5                       (0x20UL << ESMC_BAUD_BAUD_Pos)
#define ESMC_BAUD_BAUD_6                       (0x40UL << ESMC_BAUD_BAUD_Pos)
#define ESMC_BAUD_BAUD_7                       (0x80UL << ESMC_BAUD_BAUD_Pos)

/*!< ESMC SOCR */
#define ESMC_SOCR_SPIMODE_Pos                     (0U)
#define ESMC_SOCR_SPIMODE_Msk                     (0x3UL << ESMC_SOCR_SPIMODE_Pos)
#define ESMC_SOCR_SPIMODE                         ESMC_SOCR_SPIMODE_Msk
#define ESMC_SOCR_SPIMODE_0                       (0x1UL << ESMC_SOCR_SPIMODE_Pos)
#define ESMC_SOCR_SPIMODE_1                       (0x2UL << ESMC_SOCR_SPIMODE_Pos)

#define ESMC_SOCR_SENM_Pos                        (2U)
#define ESMC_SOCR_SENM_Msk                        (0x1UL << ESMC_SOCR_SENM_Pos)
#define ESMC_SOCR_SENM                            ESMC_SOCR_SENM_Msk

#define ESMC_SOCR_MULTICMD_Pos                    (3U)
#define ESMC_SOCR_MULTICMD_Msk                    (0x1UL << ESMC_SOCR_MULTICMD_Pos)
#define ESMC_SOCR_MULTICMD                        ESMC_SOCR_MULTICMD_Msk

#define ESMC_SOCR_MULTIADDR_Pos                   (4U)
#define ESMC_SOCR_MULTIADDR_Msk                   (0x1UL << ESMC_SOCR_MULTIADDR_Pos)
#define ESMC_SOCR_MULTIADDR                       ESMC_SOCR_MULTIADDR_Msk

#define ESMC_SOCR_DDRCMD_Pos                      (5U)
#define ESMC_SOCR_DDRCMD_Msk                      (0x1UL << ESMC_SOCR_DDRCMD_Pos)
#define ESMC_SOCR_DDRCMD                          ESMC_SOCR_DDRCMD_Msk

#define ESMC_SOCR_DDRADDR_Pos                     (6U)
#define ESMC_SOCR_DDRADDR_Msk                     (0x1UL << ESMC_SOCR_DDRADDR_Pos)
#define ESMC_SOCR_DDRADDR                         ESMC_SOCR_DDRADDR_Msk

#define ESMC_SOCR_DDRDATA_Pos                     (7U)
#define ESMC_SOCR_DDRDATA_Msk                     (0x1UL << ESMC_SOCR_DDRDATA_Pos)
#define ESMC_SOCR_DDRDATA                         ESMC_SOCR_DDRDATA_Msk

/*!< ESMC DCR */
#define ESMC_DCR_DUMMY_Pos                     (0x0U)
#define ESMC_DCR_DUMMY_Msk                     (0x1FUL << ESMC_DCR_DUMMY_Pos)
#define ESMC_DCR_DUMMY                         ESMC_DCR_DUMMY_Msk
#define ESMC_DCR_DUMMY_0                       (0x1UL << ESMC_DCR_DUMMY_Pos)
#define ESMC_DCR_DUMMY_1                       (0x2UL << ESMC_DCR_DUMMY_Pos)
#define ESMC_DCR_DUMMY_2                       (0x4UL << ESMC_DCR_DUMMY_Pos)
#define ESMC_DCR_DUMMY_3                       (0x8UL << ESMC_DCR_DUMMY_Pos)
#define ESMC_DCR_DUMMY_4                       (0x10UL << ESMC_DCR_DUMMY_Pos)

#define ESMC_DCR_NOADDR_Pos                      (5U)
#define ESMC_DCR_NOADDR_Msk                      (0x1UL << ESMC_DCR_NOADDR_Pos)
#define ESMC_DCR_NOADDR                          ESMC_DCR_NOADDR_Msk

#define ESMC_DCR_NOCMD_Pos                     (6U)
#define ESMC_DCR_NOCMD_Msk                     (0x1UL << ESMC_DCR_NOCMD_Pos)
#define ESMC_DCR_NOCMD                        ESMC_DCR_NOCMD_Msk

#define ESMC_DCR_REC_Pos                     (7U)
#define ESMC_DCR_REC_Msk                     (0x1UL << ESMC_DCR_REC_Pos)
#define ESMC_DCR_REC                          ESMC_DCR_REC_Msk

/*!< ESMC CR3 */
#define ESMC_CR3_ADDR_Pos                     (0U)
#define ESMC_CR3_ADDR_Msk                     (0x7UL << ESMC_CR3_ADDR_Pos)
#define ESMC_CR3_ADDR                         ESMC_CR3_ADDR_Msk

#define ESMC_CR3_ADDR8BIT_Pos                     (0U)
#define ESMC_CR3_ADDR8BIT_Msk                     (0x1UL << ESMC_CR3_ADDR8BIT_Pos)
#define ESMC_CR3_ADDR8BIT                         ESMC_CR3_ADDR8BIT_Msk

#define ESMC_CR3_ADDR16BIT_Pos                     (1U)
#define ESMC_CR3_ADDR16BIT_Msk                     (0x1UL << ESMC_CR3_ADDR16BIT_Pos)
#define ESMC_CR3_ADDR16BIT                         ESMC_CR3_ADDR16BIT_Msk

#define ESMC_CR3_ADDR32BIT_Pos                     (2U)
#define ESMC_CR3_ADDR32BIT_Msk                     (0x1UL << ESMC_CR3_ADDR32BIT_Pos)
#define ESMC_CR3_ADDR32BIT                         ESMC_CR3_ADDR32BIT_Msk

#define ESMC_CR3_FIFOCLR_Pos                     (5U)
#define ESMC_CR3_FIFOCLR_Msk                     (0x1UL << ESMC_CR3_FIFOCLR_Pos)
#define ESMC_CR3_FIFOCLR                         ESMC_CR3_FIFOCLR_Msk

#define ESMC_CR3_SSCLRRQ_Pos                     (6U)
#define ESMC_CR3_SSCLRRQ_Msk                     (0x1UL << ESMC_CR3_SSCLRRQ_Pos)
#define ESMC_CR3_SSCLRRQ                         ESMC_CR3_SSCLRRQ_Msk

#define ESMC_CR3_SSSETRQ_Pos                     (7U)
#define ESMC_CR3_SSSETRQ_Msk                     (0x1UL << ESMC_CR3_SSSETRQ_Pos)
#define ESMC_CR3_SSSETRQ                         ESMC_CR3_SSSETRQ_Msk

/*!< ESMC SR */
#define ESMC_SR_SSACTIVE_Pos                     (0U)
#define ESMC_SR_SSACTIVE_Msk                     (0x1UL << ESMC_SR_SSACTIVE_Pos)
#define ESMC_SR_SSACTIVE                         ESMC_SR_SSACTIVE_Msk

#define ESMC_SR_RXBUSY_Pos                       (1U)
#define ESMC_SR_RXBUSY_Msk                       (0x1UL << ESMC_SR_RXBUSY_Pos)
#define ESMC_SR_RXBUSY                           ESMC_SR_RXBUSY_Msk

#define ESMC_SR_TXBUSY_Pos                       (2U)
#define ESMC_SR_TXBUSY_Msk                       (0x1UL << ESMC_SR_TXBUSY_Pos)
#define ESMC_SR_TXBUSY                            ESMC_SR_TXBUSY_Msk

#define ESMC_SR_IDLE_Pos                         (4U)
#define ESMC_SR_IDLE_Msk                         (0x1UL << ESMC_SR_IDLE_Pos)
#define ESMC_SR_IDLE                             ESMC_SR_IDLE_Msk

#define ESMC_SR_FIFIOOVERFOLOW_Pos               (6U)
#define ESMC_SR_FIFIOOVERFOLOW__Msk              (0x1UL << ESMC_SR_FIFIOOVERFOLOW__Pos)
#define ESMC_SR_FIFIOOVERFOLOW_                   ESMC_SR_FIFIOOVERFOLOW__Msk

#define ESMC_SR_SPIF_Pos                         (7U)
#define ESMC_SR_SPIF_Msk                         (0x1UL << ESMC_ESMC_SR_SPIFQ_Pos)
#define ESMC_SR_SPIF                             ESMC_SR_SPIF_Msk

/*!< ESMC IFR */
#define ESMC_IFR_CMDIF_Pos                        (0U)
#define ESMC_IFR_CMDIF_Msk                        (0x1UL << ESMC_IFR_CMDIF_Pos)
#define ESMC_IFR_CMDIF                            ESMC_IFR_CMDIF_Msk

#define ESMC_IFR_FIFOEIF_Pos                      (1U)
#define ESMC_IFR_FIFOEIF_Msk                      (0x1UL << ESMC_IFR_FIFOEIF_Pos)
#define ESMC_IFR_FIFOEIF                          ESMC_IFR_FIFOEIF_Msk

#define ESMC_IFR_FIFOHIF_Pos                      (2U)
#define ESMC_IFR_FIFOHIF_Msk                      (0x1UL << ESMC_IFR_FIFOHIF_Pos)
#define ESMC_IFR_FIFOHIF                          ESMC_IFR_FIFOHIF_Msk

#define ESMC_IFR_FIFOFIF_Pos                      (3U)
#define ESMC_IFR_FIFOFIF_Msk                      (0x1UL << ESMC_IFR_FIFOFIF_Pos)
#define ESMC_IFR_FIFOFIF                          ESMC_IFR_FIFOFIF_Msk

#define ESMC_IFR_IDLEIF_Pos                       (4U)
#define ESMC_IFR_IDLEIF_Msk                       (0x1UL << ESMC_IFR_IDLEIF_Pos)
#define ESMC_IFR_IDLEIF                            ESMC_IFR_IDLEIF_Msk

#define ESMC_IFR_DATAWAITIF_Pos                   (5U)
#define ESMC_IFR_DATAWAITIF_Msk                   (0x1UL << ESMC_IFR_DATAWAITIF_Pos)
#define ESMC_IFR_DATAWAITIF                       ESMC_IFR_DATAWAITIF_Msk

#define ESMC_IFR_FIFOOIF_Pos                      (6U)
#define ESMC_IFR_FIFOOIF_Msk                      (0x1UL << ESMC_IFR_FIFOOIF_Pos)
#define ESMC_IFR_FIFOOIF                          ESMC_IFR_FIFOOIF_Msk

#define ESMC_IFR_ADDRDONEIF_Pos                   (7U)
#define ESMC_IFR_ADDRDONEIF_Msk                   (0x1UL << ESMC_IFR_ADDRDONEIF_Pos)
#define ESMC_IFR_ADDRDONEIF                       ESMC_IFR_ADDRDONEIF_Msk

/*!< ESMC IER */
#define ESMC_IER_CMDIE_Pos                        (0U)
#define ESMC_IER_CMDIE_Msk                        (0x1UL << ESMC_IER_CMDIE_Pos)
#define ESMC_IER_CMDIE                            ESMC_IER_CMDIE_Msk

#define ESMC_IER_FIFOEIE_Pos                      (1U)
#define ESMC_IER_FIFOEIE_Msk                      (0x1UL << ESMC_IER_FIFOEIE_Pos)
#define ESMC_IER_FIFOEIE                          ESMC_IER_FIFOEIE_Msk

#define ESMC_IER_FIFOHIE_Pos                      (2U)
#define ESMC_IER_FIFOHIE_Msk                      (0x1UL << ESMC_IER_FIFOHIE_Pos)
#define ESMC_IER_FIFOHIE                          ESMC_IER_FIFOHIE_Msk

#define ESMC_IER_FIFOFIE_Pos                      (3U)
#define ESMC_IER_FIFOFIE_Msk                      (0x1UL << ESMC_IER_FIFOFIE_Pos)
#define ESMC_IER_FIFOFIE                          ESMC_IER_FIFOFIE_Msk

#define ESMC_IER_IDLEIE_Pos                       (4U)
#define ESMC_IER_IDLEIE_Msk                       (0x1UL << ESMC_IER_IDLEIE_Pos)
#define ESMC_IER_IDLEIE                           ESMC_IER_IDLEIE_Msk

#define ESMC_IER_DATAWAITIE_Pos                   (5U)
#define ESMC_IER_DATAWAITIE_Msk                   (0x1UL << ESMC_IER_DATAWAITIE_Pos)
#define ESMC_IER_DATAWAITIE                       ESMC_IER_DATAWAITIE_Msk

#define ESMC_IER_FIFOOIE_Pos                      (6U)
#define ESMC_IER_FIFOOIE_Msk                      (0x1UL << ESMC_IER_FIFOOIE_Pos)
#define ESMC_IER_FIFOOIE                          ESMC_IER_FIFOOIE_Msk

#define ESMC_IER_ADDRDONEIE_Pos                   (7U)
#define ESMC_IER_ADDRDONEIE_Msk                   (0x1UL << ESMC_IER_ADDRDONEIE_Pos)
#define ESMC_IER_ADDRDONEIE                       ESMC_IER_ADDRDONEIE_Msk

/*!< ESMC ADDR24 */
#define ESMC_ADDR24_INS_Pos                     (0U)
#define ESMC_ADDR24_INS_Msk                     (0xFFUL << ESMC_ADDR24_INS_Pos)
#define ESMC_ADDR24_INS                         ESMC_ADDR24_INS_Msk
#define ESMC_ADDR24_INS_0                       (0x1UL << ESMC_ADDR24_INS_Pos)
#define ESMC_ADDR24_INS_1                       (0x2UL << ESMC_ADDR24_INS_Pos)
#define ESMC_ADDR24_INS_2                       (0x4UL << ESMC_ADDR24_INS_Pos)
#define ESMC_ADDR24_INS_3                       (0x8UL << ESMC_ADDR24_INS_Pos)
#define ESMC_ADDR24_INS_4                       (0x10UL << ESMC_ADDR24_INS_Pos)
#define ESMC_ADDR24_INS_5                       (0x20UL << ESMC_ADDR24_INS_Pos)
#define ESMC_ADDR24_INS_6                       (0x40UL << ESMC_ADDR24_INS_Pos)
#define ESMC_ADDR24_INS_7                       (0x80UL << ESMC_ADDR24_INS_Pos)

#define ESMC_ADDR24_ADDR0_Pos                   (8U)
#define ESMC_ADDR24_ADDR0_Msk                   (0xFFUL << ESMC_ADDR24_ADDR0_Pos)
#define ESMC_ADDR24_ADDR0                       ESMC_ADDR24_ADDR0_Msk
#define ESMC_ADDR24_ADDR0_0                     (0x1UL << ESMC_ADDR24_ADDR0_Pos)
#define ESMC_ADDR24_ADDR0_1                     (0x2UL << ESMC_ADDR24_ADDR0_Pos)
#define ESMC_ADDR24_ADDR0_2                     (0x4UL << ESMC_ADDR24_ADDR0_Pos)
#define ESMC_ADDR24_ADDR0_3                     (0x8UL << ESMC_ADDR24_ADDR0_Pos)
#define ESMC_ADDR24_ADDR0_4                     (0x10UL << ESMC_ADDR24_ADDR0_Pos)
#define ESMC_ADDR24_ADDR0_5                     (0x20UL << ESMC_ADDR24_ADDR0_Pos)
#define ESMC_ADDR24_ADDR0_6                     (0x40UL << ESMC_ADDR24_ADDR0_Pos)
#define ESMC_ADDR24_ADDR0_7                     (0x80UL << ESMC_ADDR24_ADDR0_Pos)

#define ESMC_ADDR24_ADDR1_Pos                   (16U)
#define ESMC_ADDR24_ADDR1_Msk                   (0xFFUL << ESMC_ADDR24_ADDR1_Pos)
#define ESMC_ADDR24_ADDR1                       ESMC_ADDR24_ADDR1_Msk
#define ESMC_ADDR24_ADDR1_0                     (0x1UL << ESMC_ADDR24_ADDR1_Pos)
#define ESMC_ADDR24_ADDR1_1                     (0x2UL << ESMC_ADDR24_ADDR1_Pos)
#define ESMC_ADDR24_ADDR1_2                     (0x4UL << ESMC_ADDR24_ADDR1_Pos)
#define ESMC_ADDR24_ADDR1_3                     (0x8UL << ESMC_ADDR24_ADDR1_Pos)
#define ESMC_ADDR24_ADDR1_4                     (0x10UL << ESMC_ADDR24_ADDR1_Pos)
#define ESMC_ADDR24_ADDR1_5                     (0x20UL << ESMC_ADDR24_ADDR1_Pos)
#define ESMC_ADDR24_ADDR1_6                     (0x40UL << ESMC_ADDR24_ADDR1_Pos)
#define ESMC_ADDR24_ADDR1_7                     (0x80UL << ESMC_ADDR24_ADDR1_Pos)

#define ESMC_ADDR24_ADDR2_Pos                   (24U)
#define ESMC_ADDR24_ADDR2_Msk                   (0xFFUL << ESMC_ADDR24_ADDR2_Pos)
#define ESMC_ADDR24_ADDR2                       ESMC_ADDR24_ADDR2_Msk
#define ESMC_ADDR24_ADDR2_0                     (0x1UL << ESMC_ADDR24_ADDR2_Pos)
#define ESMC_ADDR24_ADDR2_1                     (0x2UL << ESMC_ADDR24_ADDR2_Pos)
#define ESMC_ADDR24_ADDR2_2                     (0x4UL << ESMC_ADDR24_ADDR2_Pos)
#define ESMC_ADDR24_ADDR2_3                     (0x8UL << ESMC_ADDR24_ADDR2_Pos)
#define ESMC_ADDR24_ADDR2_4                     (0x10UL << ESMC_ADDR24_ADDR2_Pos)
#define ESMC_ADDR24_ADDR2_5                     (0x20UL << ESMC_ADDR24_ADDR2_Pos)
#define ESMC_ADDR24_ADDR2_6                     (0x40UL << ESMC_ADDR24_ADDR2_Pos)
#define ESMC_ADDR24_ADDR2_7                     (0x80UL << ESMC_ADDR24_ADDR2_Pos)

/*!< ESMC ADDR32 */
#define ESMC_ADDR32_ADDR3_Pos                     (0U)
#define ESMC_ADDR32_ADDR3_Msk                     (0xFFUL << ESMC_ADDR32_ADDR3_Pos)
#define ESMC_ADDR32_ADDR3                         ESMC_ADDR32_ADDR3_Msk
#define ESMC_ADDR32_ADDR3_0                       (0x1UL << ESMC_ADDR32_ADDR3_Pos)
#define ESMC_ADDR32_ADDR3_1                       (0x2UL << ESMC_ADDR32_ADDR3_Pos)
#define ESMC_ADDR32_ADDR3_2                       (0x4UL << ESMC_ADDR32_ADDR3_Pos)
#define ESMC_ADDR32_ADDR3_3                       (0x8UL << ESMC_ADDR32_ADDR3_Pos)
#define ESMC_ADDR32_ADDR3_4                       (0x10UL << ESMC_ADDR32_ADDR3_Pos)
#define ESMC_ADDR32_ADDR3_5                       (0x20UL << ESMC_ADDR32_ADDR3_Pos)
#define ESMC_ADDR32_ADDR3_6                       (0x40UL << ESMC_ADDR32_ADDR3_Pos)
#define ESMC_ADDR32_ADDR3_7                       (0x80UL << ESMC_ADDR32_ADDR3_Pos)

#define ESMC_ADDR32_MREG_Pos                   (8U)
#define ESMC_ADDR32_MREG_Msk                   (0xFFUL << ESMC_ADDR32_MREG_Pos)
#define ESMC_ADDR32_MREG                       ESMC_ADDR32_MREG_Msk
#define ESMC_ADDR32_MREG_0                     (0x1UL << ESMC_ADDR32_MREG_Pos)
#define ESMC_ADDR32_MREG_1                     (0x2UL << ESMC_ADDR32_MREG_Pos)
#define ESMC_ADDR32_MREG_2                     (0x4UL << ESMC_ADDR32_MREG_Pos)
#define ESMC_ADDR32_MREG_3                     (0x8UL << ESMC_ADDR32_MREG_Pos)
#define ESMC_ADDR32_MREG_4                     (0x10UL << ESMC_ADDR32_MREG_Pos)
#define ESMC_ADDR32_MREG_5                     (0x20UL << ESMC_ADDR32_MREG_Pos)
#define ESMC_ADDR32_MREG_6                     (0x40UL << ESMC_ADDR32_MREG_Pos)
#define ESMC_ADDR32_MREG_7                     (0x80UL << ESMC_ADDR32_MREG_Pos)

#define ESMC_ADDR32_SS_Pos                   (16U)
#define ESMC_ADDR32_SS_Msk                   (0xFUL << ESMC_ADDR32_SS_Pos)
#define ESMC_ADDR32_SS                       ESMC_ADDR32_SS_Msk
#define ESMC_ADDR32_SS_0                     (0x1UL << ESMC_ADDR32_SS_Pos)
#define ESMC_ADDR32_SS_1                     (0x2UL << ESMC_ADDR32_SS_Pos)
#define ESMC_ADDR32_SS_2                     (0x4UL << ESMC_ADDR32_SS_Pos)
#define ESMC_ADDR32_SS_3                     (0x8UL << ESMC_ADDR32_SS_Pos)

#define ESMC_ADDR32_XSS_Pos                   (24U)
#define ESMC_ADDR32_XSS_Msk                   (0xFFUL << ESMC_ADDR32_XSS_Pos)
#define ESMC_ADDR32_XSS                        ESMC_ADDR32_XSS_Msk
#define ESMC_ADDR32_XSS_0                     (0x1UL << ESMC_ADDR32_XSS_Pos)
#define ESMC_ADDR32_XSS_1                     (0x1UL << ESMC_ADDR32_XSS_Pos)
#define ESMC_ADDR32_XSS_2                     (0x1UL << ESMC_ADDR32_XSS_Pos)
#define ESMC_ADDR32_XSS_3                     (0x1UL << ESMC_ADDR32_XSS_Pos)

/*!< ESMC DATA */
#define ESMC_DATA_DATA0_Pos                   (0U)
#define ESMC_DATA_DATA0_Msk                   (0xFFUL << ESMC_DATA_DATA0_Pos)
#define ESMC_DATA_DATA0                       ESMC_DATA_DATA0_Msk
#define ESMC_DATA_DATA0_0                     (0x1UL << ESMC_DATA_DATA0_Pos)
#define ESMC_DATA_DATA0_1                     (0x2UL << ESMC_DATA_DATA0_Pos)
#define ESMC_DATA_DATA0_2                     (0x4UL << ESMC_DATA_DATA0_Pos)
#define ESMC_DATA_DATA0_3                     (0x8UL << ESMC_DATA_DATA0_Pos)
#define ESMC_DATA_DATA0_4                     (0x10UL << ESMC_DATA_DATA0_Pos)
#define ESMC_DATA_DATA0_5                     (0x20UL << ESMC_DATA_DATA0_Pos)
#define ESMC_DATA_DATA0_6                     (0x40UL << ESMC_DATA_DATA0_Pos)
#define ESMC_DATA_DATA0_7                     (0x80UL << ESMC_DATA_DATA0_Pos)

#define ESMC_DATA_DATA1_Pos                   (8U)
#define ESMC_DATA_DATA1_Msk                   (0xFFUL << ESMC_DATA_DATA1_Pos)
#define ESMC_DATA_DATA1                       ESMC_DATA_DATA1_Msk
#define ESMC_DATA_DATA1_0                     (0x1UL << ESMC_DATA_DATA1_Pos)
#define ESMC_DATA_DATA1_1                     (0x2UL << ESMC_DATA_DATA1_Pos)
#define ESMC_DATA_DATA1_2                     (0x4UL << ESMC_DATA_DATA1_Pos)
#define ESMC_DATA_DATA1_3                     (0x8UL << ESMC_DATA_DATA1_Pos)
#define ESMC_DATA_DATA1_4                     (0x10UL << ESMC_DATA_DATA1_Pos)
#define ESMC_DATA_DATA1_5                     (0x20UL << ESMC_DATA_DATA1_Pos)
#define ESMC_DATA_DATA1_6                     (0x40UL << ESMC_DATA_DATA1_Pos)
#define ESMC_DATA_DATA1_7                     (0x80UL << ESMC_DATA_DATA1_Pos)

#define ESMC_DATA_DATA2_Pos                   (16U)
#define ESMC_DATA_DATA2_Msk                   (0xFFUL << ESMC_DATA_DATA2_Pos)
#define ESMC_DATA_DATA2                       ESMC_DATA_DATA2_Msk
#define ESMC_DATA_DATA2_0                     (0x1UL << ESMC_DATA_DATA2_Pos)
#define ESMC_DATA_DATA2_1                     (0x2UL << ESMC_DATA_DATA2_Pos)
#define ESMC_DATA_DATA2_2                     (0x4UL << ESMC_DATA_DATA2_Pos)
#define ESMC_DATA_DATA2_3                     (0x8UL << ESMC_DATA_DATA2_Pos)
#define ESMC_DATA_DATA2_4                     (0x10UL << ESMC_DATA_DATA2_Pos)
#define ESMC_DATA_DATA2_5                     (0x20UL << ESMC_DATA_DATA2_Pos)
#define ESMC_DATA_DATA2_6                     (0x40UL << ESMC_DATA_DATA2_Pos)
#define ESMC_DATA_DATA2_7                     (0x80UL << ESMC_DATA_DATA2_Pos)

#define ESMC_DATA_DATA3_Pos                   (24U)
#define ESMC_DATA_DATA3_Msk                   (0xFFUL << ESMC_DATA_DATA3_Pos)
#define ESMC_DATA_DATA3                       ESMC_DATA_DATA3_Msk
#define ESMC_DATA_DATA3_0                     (0x1UL << ESMC_DATA_DATA3_Pos)
#define ESMC_DATA_DATA3_1                     (0x2UL << ESMC_DATA_DATA3_Pos)
#define ESMC_DATA_DATA3_2                     (0x4UL << ESMC_DATA_DATA3_Pos)
#define ESMC_DATA_DATA3_3                     (0x8UL << ESMC_DATA_DATA3_Pos)
#define ESMC_DATA_DATA3_4                     (0x10UL << ESMC_DATA_DATA3_Pos)
#define ESMC_DATA_DATA3_5                     (0x20UL << ESMC_DATA_DATA3_Pos)
#define ESMC_DATA_DATA3_6                     (0x40UL << ESMC_DATA_DATA3_Pos)
#define ESMC_DATA_DATA3_7                     (0x80UL << ESMC_DATA_DATA3_Pos)

/*********************  Bits Define For Peripheral RCC  *********************/
/*!< RCC_CR */
#define RCC_CR_HSION_Pos                          (0U)
#define RCC_CR_HSION_Msk                          (0x1UL << RCC_CR_HSION_Pos)                       /*!< 0x00000001 */
#define RCC_CR_HSION                              RCC_CR_HSION_Msk                                  /*!< desc HSION */
#define RCC_CR_HSIRDY_Pos                         (1U)
#define RCC_CR_HSIRDY_Msk                         (0x1UL << RCC_CR_HSIRDY_Pos)                      /*!< 0x00000002 */
#define RCC_CR_HSIRDY                             RCC_CR_HSIRDY_Msk                                 /*!< desc HSIRDY */
#define RCC_CR_HSITRIM_Pos                        (3U)
#define RCC_CR_HSITRIM_Msk                        (0x1FUL << RCC_CR_HSITRIM_Pos)                    /*!< 0x000000F8 */
#define RCC_CR_HSITRIM                            RCC_CR_HSITRIM_Msk                                /*!< HSITRIM[7:3] bits (Internal high-speed clock trimming) */
#define RCC_CR_HSITRIM_0                          (0x1UL << RCC_CR_HSITRIM_Pos)                     /*!< 0x00000008 */
#define RCC_CR_HSITRIM_1                          (0x2UL << RCC_CR_HSITRIM_Pos)                     /*!< 0x00000010 */
#define RCC_CR_HSITRIM_2                          (0x4UL << RCC_CR_HSITRIM_Pos)                     /*!< 0x00000020 */
#define RCC_CR_HSITRIM_3                          (0x8UL << RCC_CR_HSITRIM_Pos)                     /*!< 0x00000040 */
#define RCC_CR_HSITRIM_4                          (0x10UL << RCC_CR_HSITRIM_Pos)                    /*!< 0x00000080 */

#define RCC_CR_HSICAL_Pos                         (8U)
#define RCC_CR_HSICAL_Msk                         (0xE0FFUL << RCC_CR_HSICAL_Pos)                   /*!< 0x00E0FF00 */
#define RCC_CR_HSICAL                             RCC_CR_HSICAL_Msk                                 /*!< HSICAL[15:8] bits (Internal high-speed clock calibration) */
#define RCC_CR_HSEON_Pos                          (16U)
#define RCC_CR_HSEON_Msk                          (0x1UL << RCC_CR_HSEON_Pos)                       /*!< 0x00010000 */
#define RCC_CR_HSEON                              RCC_CR_HSEON_Msk                                  /*!< desc HSEON */
#define RCC_CR_HSERDY_Pos                         (17U)
#define RCC_CR_HSERDY_Msk                         (0x1UL << RCC_CR_HSERDY_Pos)                      /*!< 0x00020000 */
#define RCC_CR_HSERDY                             RCC_CR_HSERDY_Msk                                 /*!< desc HSERDY */
#define RCC_CR_HSEBYP_Pos                         (18U)
#define RCC_CR_HSEBYP_Msk                         (0x1UL << RCC_CR_HSEBYP_Pos)                      /*!< 0x00040000 */
#define RCC_CR_HSEBYP                             RCC_CR_HSEBYP_Msk                                 /*!< desc HSEBYP */
#define RCC_CR_CSSON_Pos                          (19U)
#define RCC_CR_CSSON_Msk                          (0x1UL << RCC_CR_CSSON_Pos)                       /*!< 0x00080000 */
#define RCC_CR_CSSON                              RCC_CR_CSSON_Msk                                  /*!< desc CSSON */
#define RCC_CR_PLLON_Pos                          (24U)
#define RCC_CR_PLLON_Msk                          (0x1UL << RCC_CR_PLLON_Pos)                       /*!< 0x01000000 */
#define RCC_CR_PLLON                              RCC_CR_PLLON_Msk                                  /*!< PLL enable */
#define RCC_CR_PLLRDY_Pos                         (25U)
#define RCC_CR_PLLRDY_Msk                         (0x1UL << RCC_CR_PLLRDY_Pos)                      /*!< 0x02000000 */
#define RCC_CR_PLLRDY                             RCC_CR_PLLRDY_Msk                                 /*!< desc PLLRDY */

/*!< RCC_CFGR */
#define RCC_CFGR_SW_Pos                           (0U)
#define RCC_CFGR_SW_Msk                           (0x3UL << RCC_CFGR_SW_Pos)                        /*!< 0x00000003 */
#define RCC_CFGR_SW                               RCC_CFGR_SW_Msk                                   /*!< SW[1:0] bits (desc SW) */
#define RCC_CFGR_SW_0                             (0x1UL << RCC_CFGR_SW_Pos)                        /*!< 0x00000001 */
#define RCC_CFGR_SW_1                             (0x2UL << RCC_CFGR_SW_Pos)                        /*!< 0x00000002 */

#define RCC_CFGR_SW_HSI                           0x00000000U                                       /*!< HSI selected as system clock */
#define RCC_CFGR_SW_HSE                           0x00000001U                                       /*!< HSE selected as system clock */
#define RCC_CFGR_SW_PLL                           0x00000002U                                       /*!< PLL selected as system clock */
#define RCC_CFGR_SWS_Pos                          (2U)
#define RCC_CFGR_SWS_Msk                          (0x3UL << RCC_CFGR_SWS_Pos)                       /*!< 0x0000000C */
#define RCC_CFGR_SWS                              RCC_CFGR_SWS_Msk                                  /*!< SWS[3:2] bits (desc SWS) */
#define RCC_CFGR_SWS_0                            (0x1UL << RCC_CFGR_SWS_Pos)                       /*!< 0x00000004 */
#define RCC_CFGR_SWS_1                            (0x2UL << RCC_CFGR_SWS_Pos)                       /*!< 0x00000008 */

#define RCC_CFGR_SWS_HSI                          0x00000000U                                       /*!< HSI oscillator used as system clock */
#define RCC_CFGR_SWS_HSE                          0x00000004U                                       /*!< HSE oscillator used as system clock */
#define RCC_CFGR_SWS_PLL                          0x00000008U                                       /*!< PLL used as system clock */
#define RCC_CFGR_HPRE_Pos                         (4U)
#define RCC_CFGR_HPRE_Msk                         (0xFUL << RCC_CFGR_HPRE_Pos)                      /*!< 0x000000F0 */
#define RCC_CFGR_HPRE                             RCC_CFGR_HPRE_Msk                                 /*!< HPRE[7:4] bits (desc HPRE) */
#define RCC_CFGR_HPRE_0                           (0x1UL << RCC_CFGR_HPRE_Pos)                      /*!< 0x00000010 */
#define RCC_CFGR_HPRE_1                           (0x2UL << RCC_CFGR_HPRE_Pos)                      /*!< 0x00000020 */
#define RCC_CFGR_HPRE_2                           (0x4UL << RCC_CFGR_HPRE_Pos)                      /*!< 0x00000040 */
#define RCC_CFGR_HPRE_3                           (0x8UL << RCC_CFGR_HPRE_Pos)                      /*!< 0x00000080 */

#define RCC_CFGR_HPRE_DIV1                        0x00000000U                                       /*!< SYSCLK not divided */
#define RCC_CFGR_HPRE_DIV2                        0x00000080U                                       /*!< SYSCLK divided by 2 */
#define RCC_CFGR_HPRE_DIV4                        0x00000090U                                       /*!< SYSCLK divided by 4 */
#define RCC_CFGR_HPRE_DIV8                        0x000000A0U                                       /*!< SYSCLK divided by 8 */
#define RCC_CFGR_HPRE_DIV16                       0x000000B0U                                       /*!< SYSCLK divided by 16 */
#define RCC_CFGR_HPRE_DIV64                       0x000000C0U                                       /*!< SYSCLK divided by 64 */
#define RCC_CFGR_HPRE_DIV128                      0x000000D0U                                       /*!< SYSCLK divided by 128 */
#define RCC_CFGR_HPRE_DIV256                      0x000000E0U                                       /*!< SYSCLK divided by 256 */
#define RCC_CFGR_HPRE_DIV512                      0x000000F0U                                       /*!< SYSCLK divided by 512 */
#define RCC_CFGR_PPRE1_Pos                        (8U)
#define RCC_CFGR_PPRE1_Msk                        (0x7UL << RCC_CFGR_PPRE1_Pos)                     /*!< 0x00000700 */
#define RCC_CFGR_PPRE1                            RCC_CFGR_PPRE1_Msk                                /*!< PPRE1[10:8] bits (APB Low speed prescaler (APB1)) */
#define RCC_CFGR_PPRE1_0                          (0x1UL << RCC_CFGR_PPRE1_Pos)                     /*!< 0x00000100 */
#define RCC_CFGR_PPRE1_1                          (0x2UL << RCC_CFGR_PPRE1_Pos)                     /*!< 0x00000200 */
#define RCC_CFGR_PPRE1_2                          (0x4UL << RCC_CFGR_PPRE1_Pos)                     /*!< 0x00000400 */

#define RCC_CFGR_PPRE1_DIV1                       0x00000000U                                       /*!< HCLK not divided */
#define RCC_CFGR_PPRE1_DIV2                       0x00000400U                                       /*!< HCLK divided by 2 */
#define RCC_CFGR_PPRE1_DIV4                       0x00000500U                                       /*!< HCLK divided by 4 */
#define RCC_CFGR_PPRE1_DIV8                       0x00000600U                                       /*!< HCLK divided by 8 */
#define RCC_CFGR_PPRE1_DIV16                      0x00000700U                                       /*!< HCLK divided by 16 */
#define RCC_CFGR_PPRE2_Pos                        (11U)
#define RCC_CFGR_PPRE2_Msk                        (0x7UL << RCC_CFGR_PPRE2_Pos)                     /*!< 0x00003800 */
#define RCC_CFGR_PPRE2                            RCC_CFGR_PPRE2_Msk                                /*!< PPRE2[13:11] bits (APB high-speed prescaler(APB2)) */
#define RCC_CFGR_PPRE2_0                          (0x1UL << RCC_CFGR_PPRE2_Pos)                     /*!< 0x00000800 */
#define RCC_CFGR_PPRE2_1                          (0x2UL << RCC_CFGR_PPRE2_Pos)                     /*!< 0x00001000 */
#define RCC_CFGR_PPRE2_2                          (0x4UL << RCC_CFGR_PPRE2_Pos)                     /*!< 0x00002000 */

#define RCC_CFGR_ADCPRE_Pos                       (14U)
#define RCC_CFGR_ADCPRE_Msk                       (0x4003UL << RCC_CFGR_ADCPRE_Pos)                 /*!< 0x1000C000 */
#define RCC_CFGR_ADCPRE                           RCC_CFGR_ADCPRE_Msk                               /*!< ADCPRE[15:14] bits (desc ADCPRE) */
#define RCC_CFGR_ADCPRE_0                         (0x1UL << RCC_CFGR_ADCPRE_Pos)                    /*!< 0x00004000 */
#define RCC_CFGR_ADCPRE_1                         (0x2UL << RCC_CFGR_ADCPRE_Pos)                    /*!< 0x00008000 */
#define RCC_CFGR_ADCPRE_2                         (0x4000UL << RCC_CFGR_ADCPRE_Pos)                 /*!< 0x10000000 */

#define RCC_CFGR_ADCPRE_DIV2                     ((uint32_t)0x00000000)                             /*!< PCLK2 divided by 2 */
#define RCC_CFGR_ADCPRE_DIV4                     ((uint32_t)0x00004000)                             /*!< PCLK2 divided by 4 */
#define RCC_CFGR_ADCPRE_DIV6                     ((uint32_t)0x00008000)                             /*!< PCLK2 divided by 6 */
#define RCC_CFGR_ADCPRE_DIV8                     ((uint32_t)0x0000C000)                             /*!< PCLK2 divided by 8 */
#define RCC_CFGR_ADCPRE_DIV12                    ((uint32_t)0x10004000)                             /*!< PCLK2 divided by 12 */
#define RCC_CFGR_ADCPRE_DIV16                    ((uint32_t)0x1000C000)                             /*!< PCLK2 divided by 16 */

#define RCC_CFGR_PLLSRC_Pos                       (16U)
#define RCC_CFGR_PLLSRC_Msk                       (0x1UL << RCC_CFGR_PLLSRC_Pos)                    /*!< 0x00010000 */
#define RCC_CFGR_PLLSRC                           RCC_CFGR_PLLSRC_Msk                               /*!< desc PLLSRC */
#define RCC_CFGR_PLLXTPRE_Pos                     (17U)
#define RCC_CFGR_PLLXTPRE_Msk                     (0x1UL << RCC_CFGR_PLLXTPRE_Pos)                  /*!< 0x00020000 */
#define RCC_CFGR_PLLXTPRE                         RCC_CFGR_PLLXTPRE_Msk                             /*!< desc PLLXTPRE */
#define RCC_CFGR_PLLMULL_Pos                      (18U)
#define RCC_CFGR_PLLMULL_Msk                      (0x180FUL << RCC_CFGR_PLLMULL_Pos)                /*!< 0x603C0000 */
#define RCC_CFGR_PLLMULL                          RCC_CFGR_PLLMULL_Msk                              /*!< PLLMULL[21:18] bits (desc PLLMULL) */
#define RCC_CFGR_PLLMULL_0                        (0x1UL << RCC_CFGR_PLLMULL_Pos)                   /*!< 0x00040000 */
#define RCC_CFGR_PLLMULL_1                        (0x2UL << RCC_CFGR_PLLMULL_Pos)                   /*!< 0x00080000 */
#define RCC_CFGR_PLLMULL_2                        (0x4UL << RCC_CFGR_PLLMULL_Pos)                   /*!< 0x00100000 */
#define RCC_CFGR_PLLMULL_3                        (0x8UL << RCC_CFGR_PLLMULL_Pos)                   /*!< 0x00200000 */
#define RCC_CFGR_PLLMULL_4                        (0x800UL << RCC_CFGR_PLLMULL_Pos)                 /*!< 0x20000000 */
#define RCC_CFGR_PLLMULL_5                        (0x1000UL << RCC_CFGR_PLLMULL_Pos)                /*!< 0x40000000 */

#define RCC_CFGR_PLLMULL2                         0x00000000U                                       /*!< PLL input clock*2 */
#define RCC_CFGR_PLLMULL3                         0x00040000U                                       /*!< PLL input clock*3 */
#define RCC_CFGR_PLLMULL4                         0x00080000U                                       /*!< PLL input clock*4 */
#define RCC_CFGR_PLLMULL5                         0x000C0000U                                       /*!< PLL input clock*5 */
#define RCC_CFGR_PLLMULL6                         0x00100000U                                       /*!< PLL input clock*6 */
#define RCC_CFGR_PLLMULL7                         0x00140000U                                       /*!< PLL input clock*7 */
#define RCC_CFGR_PLLMULL8                         0x00180000U                                       /*!< PLL input clock*8 */
#define RCC_CFGR_PLLMULL9                         0x001C0000U                                       /*!< PLL input clock*9 */
#define RCC_CFGR_PLLMULL10                        0x00200000U                                       /*!< PLL input clock10 */
#define RCC_CFGR_PLLMULL11                        0x00240000U                                       /*!< PLL input clock*11 */
#define RCC_CFGR_PLLMULL12                        0x00280000U                                       /*!< PLL input clock*12 */
#define RCC_CFGR_PLLMULL13                        0x002C0000U                                       /*!< PLL input clock*13 */
#define RCC_CFGR_PLLMULL14                        0x00300000U                                       /*!< PLL input clock*14 */
#define RCC_CFGR_PLLMULL15                        0x00340000U                                       /*!< PLL input clock*15 */
#define RCC_CFGR_PLLMULL16                        0x00380000U                                       /*!< PLL input clock*16 */
#define RCC_CFGR_PLLMULL17                        0x003C0000U                                       /*!< PLL input clock*17 */
#define RCC_CFGR_PLLMULL18                        0x20000000U                                       /*!< PLL input clock*18 */
#define RCC_CFGR_PLLMULL19                        0x20040000U                                       /*!< PLL input clock*19 */
#define RCC_CFGR_PLLMULL20                        0x20080000U                                       /*!< PLL input clock*20 */
#define RCC_CFGR_PLLMULL21                        0x200C0000U                                       /*!< PLL input clock*21 */
#define RCC_CFGR_PLLMULL22                        0x20100000U                                       /*!< PLL input clock*22 */
#define RCC_CFGR_PLLMULL23                        0x20140000U                                       /*!< PLL input clock*23 */
#define RCC_CFGR_PLLMULL24                        0x20180000U                                       /*!< PLL input clock*24 */
#define RCC_CFGR_PLLMULL25                        0x201C0000U                                       /*!< PLL input clock*35 */
#define RCC_CFGR_PLLMULL26                        0x20200000U                                       /*!< PLL input clock*46 */
#define RCC_CFGR_PLLMULL27                        0x20240000U                                       /*!< PLL input clock*27 */
#define RCC_CFGR_PLLMULL28                        0x20280000U                                       /*!< PLL input clock*28 */
#define RCC_CFGR_PLLMULL29                        0x202C0000U                                       /*!< PLL input clock*29 */
#define RCC_CFGR_PLLMULL30                        0x20300000U                                       /*!< PLL input clock*30 */
#define RCC_CFGR_PLLMULL31                        0x20340000U                                       /*!< PLL input clock*31 */
#define RCC_CFGR_PLLMULL32                        0x20380000U                                       /*!< PLL input clock*32 */
#define RCC_CFGR_PLLMULL33                        0x203C0000U                                       /*!< PLL input clock*33 */
#define RCC_CFGR_PLLMULL34                        0x40000000U                                       /*!< PLL input clock*34 */
#define RCC_CFGR_PLLMULL35                        0x40040000U                                       /*!< PLL input clock*35 */
#define RCC_CFGR_PLLMULL36                        0x40080000U                                       /*!< PLL input clock*36 */
#define RCC_CFGR_PLLMULL37                        0x400C0000U                                       /*!< PLL input clock*37 */
#define RCC_CFGR_PLLMULL38                        0x40100000U                                       /*!< PLL input clock*38 */
#define RCC_CFGR_PLLMULL39                        0x40140000U                                       /*!< PLL input clock*39 */
#define RCC_CFGR_PLLMULL40                        0x40180000U                                       /*!< PLL input clock*40 */
#define RCC_CFGR_PLLMULL41                        0x401C0000U                                       /*!< PLL input clock*41 */
#define RCC_CFGR_PLLMULL42                        0x40200000U                                       /*!< PLL input clock*42 */
#define RCC_CFGR_PLLMULL43                        0x40240000U                                       /*!< PLL input clock*43 */
#define RCC_CFGR_PLLMULL44                        0x40280000U                                       /*!< PLL input clock*44 */
#define RCC_CFGR_PLLMULL45                        0x402C0000U                                       /*!< PLL input clock*45 */
#define RCC_CFGR_PLLMULL46                        0x40300000U                                       /*!< PLL input clock*46 */
#define RCC_CFGR_PLLMULL47                        0x40340000U                                       /*!< PLL input clock*47 */
#define RCC_CFGR_PLLMULL48                        0x40380000U                                       /*!< PLL input clock*48 */
#define RCC_CFGR_PLLMULL49                        0x403C0000U                                       /*!< PLL input clock*49 */
#define RCC_CFGR_PLLMULL50                        0x60000000U                                       /*!< PLL input clock*50 */
#define RCC_CFGR_PLLMULL51                        0x60040000U                                       /*!< PLL input clock*51 */
#define RCC_CFGR_PLLMULL52                        0x60080000U                                       /*!< PLL input clock*52 */
#define RCC_CFGR_PLLMULL53                        0x600C0000U                                       /*!< PLL input clock*53 */
#define RCC_CFGR_PLLMULL54                        0x60100000U                                       /*!< PLL input clock*54 */
#define RCC_CFGR_PLLMULL55                        0x60140000U                                       /*!< PLL input clock*55 */
#define RCC_CFGR_PLLMULL56                        0x60180000U                                       /*!< PLL input clock*56 */
#define RCC_CFGR_PLLMULL57                        0x601C0000U                                       /*!< PLL input clock*57 */
#define RCC_CFGR_PLLMULL58                        0x60200000U                                       /*!< PLL input clock*58 */
#define RCC_CFGR_PLLMULL59                        0x60240000U                                       /*!< PLL input clock*59 */
#define RCC_CFGR_PLLMULL60                        0x60280000U                                       /*!< PLL input clock*60 */
#define RCC_CFGR_PLLMULL61                        0x602C0000U                                       /*!< PLL input clock*61 */
#define RCC_CFGR_PLLMULL62                        0x60300000U                                       /*!< PLL input clock*62 */
#define RCC_CFGR_PLLMULL63                        0x60340000U                                       /*!< PLL input clock*63 */

#define RCC_CFGR_USBPRE_Pos                       (22U)
#define RCC_CFGR_USBPRE_Msk                       (0x203UL << RCC_CFGR_USBPRE_Pos)                  /*!< 0x80C00000 */
#define RCC_CFGR_USBPRE                           RCC_CFGR_USBPRE_Msk                               /*!< USBPRE[23:22] bits (desc USBPRE) */
#define RCC_CFGR_USBPRE_0                         (0x1UL << RCC_CFGR_USBPRE_Pos)                    /*!< 0x00400000 */
#define RCC_CFGR_USBPRE_1                         (0x2UL << RCC_CFGR_USBPRE_Pos)                    /*!< 0x00800000 */
#define RCC_CFGR_USBPRE_2                         (0x200UL << RCC_CFGR_USBPRE_Pos)                  /*!< 0x80000000 */

#define RCC_CFGR_MCO_Pos                          (24U)
#define RCC_CFGR_MCO_Msk                          (0x7UL << RCC_CFGR_MCO_Pos)                       /*!< 0x07000000 */
#define RCC_CFGR_MCO                              RCC_CFGR_MCO_Msk                                  /*!< MCO[27:24] bits (desc MCO) */
#define RCC_CFGR_MCO_0                            (0x1UL << RCC_CFGR_MCO_Pos)                       /*!< 0x01000000 */
#define RCC_CFGR_MCO_1                            (0x2UL << RCC_CFGR_MCO_Pos)                       /*!< 0x02000000 */
#define RCC_CFGR_MCO_2                            (0x4UL << RCC_CFGR_MCO_Pos)                       /*!< 0x04000000 */

#define RCC_CFGR_MCO_NOCLOCK                      0x00000000U                                       /*!< No clock */
#define RCC_CFGR_MCO_LSE                          0x01000000U                                       /*!< LSE selected as MCO source */
#define RCC_CFGR_MCO_LSI                          0x02000000U                                       /*!< LSI selected as MCO source */
#define RCC_CFGR_MCO_HSI48M                       0x03000000U                                       /*!< HSI48 selected as MCO source */
#define RCC_CFGR_MCO_SYSCLK                       0x04000000U                                       /*!< SYSTEM CLOCK selected as MCO source */
#define RCC_CFGR_MCO_HSI8M                        0x05000000U                                       /*!< HSI8 selected as MCO source*/
#define RCC_CFGR_MCO_HSE                          0x06000000U                                       /*!< HSE selected as MCO source*/
#define RCC_CFGR_MCO_PLL                          0x07000000U                                       /*!< PLL selected as MCO source */

/*!< RCC_CIR */
#define RCC_CIR_LSIRDYF_Pos                       (0U)
#define RCC_CIR_LSIRDYF_Msk                       (0x1UL << RCC_CIR_LSIRDYF_Pos)                    /*!< 0x00000001 */
#define RCC_CIR_LSIRDYF                           RCC_CIR_LSIRDYF_Msk                               /*!< desc LSIRDYF */
#define RCC_CIR_LSERDYF_Pos                       (1U)
#define RCC_CIR_LSERDYF_Msk                       (0x1UL << RCC_CIR_LSERDYF_Pos)                    /*!< 0x00000002 */
#define RCC_CIR_LSERDYF                           RCC_CIR_LSERDYF_Msk                               /*!< desc LSERDYF */
#define RCC_CIR_HSIRDYF_Pos                       (2U)
#define RCC_CIR_HSIRDYF_Msk                       (0x1UL << RCC_CIR_HSIRDYF_Pos)                    /*!< 0x00000004 */
#define RCC_CIR_HSIRDYF                           RCC_CIR_HSIRDYF_Msk                               /*!< desc HSIRDYF */
#define RCC_CIR_HSERDYF_Pos                       (3U)
#define RCC_CIR_HSERDYF_Msk                       (0x1UL << RCC_CIR_HSERDYF_Pos)                    /*!< 0x00000008 */
#define RCC_CIR_HSERDYF                           RCC_CIR_HSERDYF_Msk                               /*!< desc HSERDYF */
#define RCC_CIR_PLLRDYF_Pos                       (4U)
#define RCC_CIR_PLLRDYF_Msk                       (0x1UL << RCC_CIR_PLLRDYF_Pos)                    /*!< 0x00000010 */
#define RCC_CIR_PLLRDYF                           RCC_CIR_PLLRDYF_Msk                               /*!< desc PLLRDYF */
#define RCC_CIR_HSI48RDYF_Pos                     (5U)
#define RCC_CIR_HSI48RDYF_Msk                     (0x1UL << RCC_CIR_HSI48RDYF_Pos)                  /*!< 0x00000020 */
#define RCC_CIR_HSI48RDYF                         RCC_CIR_HSI48RDYF_Msk                             /*!< desc HSI48RDYF */
#define RCC_CIR_CSSF_Pos                          (7U)
#define RCC_CIR_CSSF_Msk                          (0x1UL << RCC_CIR_CSSF_Pos)                       /*!< 0x00000080 */
#define RCC_CIR_CSSF                              RCC_CIR_CSSF_Msk                                  /*!< desc CSSF */
#define RCC_CIR_LSIRDYIE_Pos                      (8U)
#define RCC_CIR_LSIRDYIE_Msk                      (0x1UL << RCC_CIR_LSIRDYIE_Pos)                   /*!< 0x00000100 */
#define RCC_CIR_LSIRDYIE                          RCC_CIR_LSIRDYIE_Msk                              /*!< desc LSIRDYIE */
#define RCC_CIR_LSERDYIE_Pos                      (9U)
#define RCC_CIR_LSERDYIE_Msk                      (0x1UL << RCC_CIR_LSERDYIE_Pos)                   /*!< 0x00000200 */
#define RCC_CIR_LSERDYIE                          RCC_CIR_LSERDYIE_Msk                              /*!< desc LSERDYIE */
#define RCC_CIR_HSIRDYIE_Pos                      (10U)
#define RCC_CIR_HSIRDYIE_Msk                      (0x1UL << RCC_CIR_HSIRDYIE_Pos)                   /*!< 0x00000400 */
#define RCC_CIR_HSIRDYIE                          RCC_CIR_HSIRDYIE_Msk                              /*!< desc HSIRDYIE */
#define RCC_CIR_HSERDYIE_Pos                      (11U)
#define RCC_CIR_HSERDYIE_Msk                      (0x1UL << RCC_CIR_HSERDYIE_Pos)                   /*!< 0x00000800 */
#define RCC_CIR_HSERDYIE                          RCC_CIR_HSERDYIE_Msk                              /*!< desc HSERDYIE */
#define RCC_CIR_PLLRDYIE_Pos                      (12U)
#define RCC_CIR_PLLRDYIE_Msk                      (0x1UL << RCC_CIR_PLLRDYIE_Pos)                   /*!< 0x00001000 */
#define RCC_CIR_PLLRDYIE                          RCC_CIR_PLLRDYIE_Msk                              /*!< desc PLLRDYIE */
#define RCC_CIR_HSI48RDYIE_Pos                    (13U)
#define RCC_CIR_HSI48RDYIE_Msk                    (0x1UL << RCC_CIR_HSI48RDYIE_Pos)                 /*!< 0x00002000 */
#define RCC_CIR_HSI48RDYIE                        RCC_CIR_HSI48RDYIE_Msk                            /*!< desc HSI48RDYIE */
#define RCC_CIR_LSIRDYC_Pos                       (16U)
#define RCC_CIR_LSIRDYC_Msk                       (0x1UL << RCC_CIR_LSIRDYC_Pos)                    /*!< 0x00010000 */
#define RCC_CIR_LSIRDYC                           RCC_CIR_LSIRDYC_Msk                               /*!< desc LSIRDYC */
#define RCC_CIR_LSERDYC_Pos                       (17U)
#define RCC_CIR_LSERDYC_Msk                       (0x1UL << RCC_CIR_LSERDYC_Pos)                    /*!< 0x00020000 */
#define RCC_CIR_LSERDYC                           RCC_CIR_LSERDYC_Msk                               /*!< desc LSERDYC */
#define RCC_CIR_HSIRDYC_Pos                       (18U)
#define RCC_CIR_HSIRDYC_Msk                       (0x1UL << RCC_CIR_HSIRDYC_Pos)                    /*!< 0x00040000 */
#define RCC_CIR_HSIRDYC                           RCC_CIR_HSIRDYC_Msk                               /*!< desc HSIRDYC */
#define RCC_CIR_HSERDYC_Pos                       (19U)
#define RCC_CIR_HSERDYC_Msk                       (0x1UL << RCC_CIR_HSERDYC_Pos)                    /*!< 0x00080000 */
#define RCC_CIR_HSERDYC                           RCC_CIR_HSERDYC_Msk                               /*!< desc HSERDYC */
#define RCC_CIR_PLLRDYC_Pos                       (20U)
#define RCC_CIR_PLLRDYC_Msk                       (0x1UL << RCC_CIR_PLLRDYC_Pos)                    /*!< 0x00100000 */
#define RCC_CIR_PLLRDYC                           RCC_CIR_PLLRDYC_Msk                               /*!< desc PLLRDYC */
#define RCC_CIR_HSI48RDYC_Pos                     (21U)
#define RCC_CIR_HSI48RDYC_Msk                     (0x1UL << RCC_CIR_HSI48RDYC_Pos)                  /*!< 0x00200000 */
#define RCC_CIR_HSI48RDYC                         RCC_CIR_HSI48RDYC_Msk                             /*!< desc HSI48RDYC */
#define RCC_CIR_CSSC_Pos                          (23U)
#define RCC_CIR_CSSC_Msk                          (0x1UL << RCC_CIR_CSSC_Pos)                       /*!< 0x00800000 */
#define RCC_CIR_CSSC                              RCC_CIR_CSSC_Msk                                  /*!< desc CSSC */

/*!< RCC_APB2RSTR */
#define RCC_APB2RSTR_SYSCFGRST_Pos                (0U)
#define RCC_APB2RSTR_SYSCFGRST_Msk                (0x1UL << RCC_APB2RSTR_SYSCFGRST_Pos)             /*!< 0x00000001 */
#define RCC_APB2RSTR_SYSCFGRST                    RCC_APB2RSTR_SYSCFGRST_Msk                        /*!< desc SYSCFGRST */
#define RCC_APB2RSTR_ADC1RST_Pos                  (9U)
#define RCC_APB2RSTR_ADC1RST_Msk                  (0x1UL << RCC_APB2RSTR_ADC1RST_Pos)               /*!< 0x00000200 */
#define RCC_APB2RSTR_ADC1RST                      RCC_APB2RSTR_ADC1RST_Msk                          /*!< desc ADC1RST */
#define RCC_APB2RSTR_ADC2RST_Pos                  (10U)
#define RCC_APB2RSTR_ADC2RST_Msk                  (0x1UL << RCC_APB2RSTR_ADC2RST_Pos)               /*!< 0x00000400 */
#define RCC_APB2RSTR_ADC2RST                      RCC_APB2RSTR_ADC2RST_Msk                          /*!< desc ADC2RST */
#define RCC_APB2RSTR_TIM1RST_Pos                  (11U)
#define RCC_APB2RSTR_TIM1RST_Msk                  (0x1UL << RCC_APB2RSTR_TIM1RST_Pos)               /*!< 0x00000800 */
#define RCC_APB2RSTR_TIM1RST                      RCC_APB2RSTR_TIM1RST_Msk                          /*!< desc TIM1RST */
#define RCC_APB2RSTR_SPI1RST_Pos                  (12U)
#define RCC_APB2RSTR_SPI1RST_Msk                  (0x1UL << RCC_APB2RSTR_SPI1RST_Pos)               /*!< 0x00001000 */
#define RCC_APB2RSTR_SPI1RST                      RCC_APB2RSTR_SPI1RST_Msk                          /*!< desc SPI1RST */
#define RCC_APB2RSTR_TIM8RST_Pos                  (13U)
#define RCC_APB2RSTR_TIM8RST_Msk                  (0x1UL << RCC_APB2RSTR_TIM8RST_Pos)               /*!< 0x00002000 */
#define RCC_APB2RSTR_TIM8RST                      RCC_APB2RSTR_TIM8RST_Msk                          /*!< desc TIM8RST */
#define RCC_APB2RSTR_USART1RST_Pos                (14U)
#define RCC_APB2RSTR_USART1RST_Msk                (0x1UL << RCC_APB2RSTR_USART1RST_Pos)             /*!< 0x00004000 */
#define RCC_APB2RSTR_USART1RST                    RCC_APB2RSTR_USART1RST_Msk                        /*!< desc USART1RST */
#define RCC_APB2RSTR_ADC3RST_Pos                  (15U)
#define RCC_APB2RSTR_ADC3RST_Msk                  (0x1UL << RCC_APB2RSTR_ADC3RST_Pos)               /*!< 0x00008000 */
#define RCC_APB2RSTR_ADC3RST                      RCC_APB2RSTR_ADC3RST_Msk                          /*!< desc ADC3RST */
#define RCC_APB2RSTR_TIM9RST_Pos                  (19U)
#define RCC_APB2RSTR_TIM9RST_Msk                  (0x1UL << RCC_APB2RSTR_TIM9RST_Pos)               /*!< 0x00080000 */
#define RCC_APB2RSTR_TIM9RST                      RCC_APB2RSTR_TIM9RST_Msk                          /*!< desc TIM9RST */
#define RCC_APB2RSTR_TIM10RST_Pos                 (20U)
#define RCC_APB2RSTR_TIM10RST_Msk                 (0x1UL << RCC_APB2RSTR_TIM10RST_Pos)              /*!< 0x00100000 */
#define RCC_APB2RSTR_TIM10RST                     RCC_APB2RSTR_TIM10RST_Msk                         /*!< desc TIM10RST */
#define RCC_APB2RSTR_TIM11RST_Pos                 (21U)
#define RCC_APB2RSTR_TIM11RST_Msk                 (0x1UL << RCC_APB2RSTR_TIM11RST_Pos)              /*!< 0x00200000 */
#define RCC_APB2RSTR_TIM11RST                     RCC_APB2RSTR_TIM11RST_Msk                         /*!< desc TIM11RST */

/*!< RCC_APB1RSTR */
#define RCC_APB1RSTR_TIM2RST_Pos                  (0U)
#define RCC_APB1RSTR_TIM2RST_Msk                  (0x1UL << RCC_APB1RSTR_TIM2RST_Pos)               /*!< 0x00000001 */
#define RCC_APB1RSTR_TIM2RST                      RCC_APB1RSTR_TIM2RST_Msk                          /*!< desc TIM2RST */
#define RCC_APB1RSTR_TIM3RST_Pos                  (1U)
#define RCC_APB1RSTR_TIM3RST_Msk                  (0x1UL << RCC_APB1RSTR_TIM3RST_Pos)               /*!< 0x00000002 */
#define RCC_APB1RSTR_TIM3RST                      RCC_APB1RSTR_TIM3RST_Msk                          /*!< desc TIM3RST */
#define RCC_APB1RSTR_TIM4RST_Pos                  (2U)
#define RCC_APB1RSTR_TIM4RST_Msk                  (0x1UL << RCC_APB1RSTR_TIM4RST_Pos)               /*!< 0x00000004 */
#define RCC_APB1RSTR_TIM4RST                      RCC_APB1RSTR_TIM4RST_Msk                          /*!< desc TIM4RST */
#define RCC_APB1RSTR_TIM5RST_Pos                  (3U)
#define RCC_APB1RSTR_TIM5RST_Msk                  (0x1UL << RCC_APB1RSTR_TIM5RST_Pos)               /*!< 0x00000008 */
#define RCC_APB1RSTR_TIM5RST                      RCC_APB1RSTR_TIM5RST_Msk                          /*!< desc TIM5RST */
#define RCC_APB1RSTR_TIM6RST_Pos                  (4U)
#define RCC_APB1RSTR_TIM6RST_Msk                  (0x1UL << RCC_APB1RSTR_TIM6RST_Pos)               /*!< 0x00000010 */
#define RCC_APB1RSTR_TIM6RST                      RCC_APB1RSTR_TIM6RST_Msk                          /*!< desc TIM6RST */
#define RCC_APB1RSTR_TIM7RST_Pos                  (5U)
#define RCC_APB1RSTR_TIM7RST_Msk                  (0x1UL << RCC_APB1RSTR_TIM7RST_Pos)               /*!< 0x00000020 */
#define RCC_APB1RSTR_TIM7RST                      RCC_APB1RSTR_TIM7RST_Msk                          /*!< desc TIM7RST */
#define RCC_APB1RSTR_TIM12RST_Pos                 (6U)
#define RCC_APB1RSTR_TIM12RST_Msk                 (0x1UL << RCC_APB1RSTR_TIM12RST_Pos)              /*!< 0x00000040 */
#define RCC_APB1RSTR_TIM12RST                     RCC_APB1RSTR_TIM12RST_Msk                         /*!< desc TIM12RST */
#define RCC_APB1RSTR_TIM13RST_Pos                 (7U)
#define RCC_APB1RSTR_TIM13RST_Msk                 (0x1UL << RCC_APB1RSTR_TIM13RST_Pos)              /*!< 0x00000080 */
#define RCC_APB1RSTR_TIM13RST                     RCC_APB1RSTR_TIM13RST_Msk                         /*!< desc TIM13RST */
#define RCC_APB1RSTR_TIM14RST_Pos                 (8U)
#define RCC_APB1RSTR_TIM14RST_Msk                 (0x1UL << RCC_APB1RSTR_TIM14RST_Pos)              /*!< 0x00000100 */
#define RCC_APB1RSTR_TIM14RST                     RCC_APB1RSTR_TIM14RST_Msk                         /*!< desc TIM14RST */
#define RCC_APB1RSTR_WWDGRST_Pos                  (11U)
#define RCC_APB1RSTR_WWDGRST_Msk                  (0x1UL << RCC_APB1RSTR_WWDGRST_Pos)               /*!< 0x00000800 */
#define RCC_APB1RSTR_WWDGRST                      RCC_APB1RSTR_WWDGRST_Msk                          /*!< desc WWDGRST */
#define RCC_APB1RSTR_SPI2RST_Pos                  (14U)
#define RCC_APB1RSTR_SPI2RST_Msk                  (0x1UL << RCC_APB1RSTR_SPI2RST_Pos)               /*!< 0x00004000 */
#define RCC_APB1RSTR_SPI2RST                      RCC_APB1RSTR_SPI2RST_Msk                          /*!< desc SPI2RST */
#define RCC_APB1RSTR_SPI3RST_Pos                  (15U)
#define RCC_APB1RSTR_SPI3RST_Msk                  (0x1UL << RCC_APB1RSTR_SPI3RST_Pos)               /*!< 0x00008000 */
#define RCC_APB1RSTR_SPI3RST                      RCC_APB1RSTR_SPI3RST_Msk                          /*!< desc SPI3RST */
#define RCC_APB1RSTR_USART2RST_Pos                (17U)
#define RCC_APB1RSTR_USART2RST_Msk                (0x1UL << RCC_APB1RSTR_USART2RST_Pos)             /*!< 0x00020000 */
#define RCC_APB1RSTR_USART2RST                    RCC_APB1RSTR_USART2RST_Msk                        /*!< desc USART2RST */
#define RCC_APB1RSTR_USART3RST_Pos                (18U)
#define RCC_APB1RSTR_USART3RST_Msk                (0x1UL << RCC_APB1RSTR_USART3RST_Pos)             /*!< 0x00040000 */
#define RCC_APB1RSTR_USART3RST                    RCC_APB1RSTR_USART3RST_Msk                        /*!< desc USART3RST */
#define RCC_APB1RSTR_USART4RST_Pos                (19U)
#define RCC_APB1RSTR_USART4RST_Msk                (0x1UL << RCC_APB1RSTR_USART4RST_Pos)             /*!< 0x00080000 */
#define RCC_APB1RSTR_USART4RST                    RCC_APB1RSTR_USART4RST_Msk                        /*!< desc USART4RST */
#define RCC_APB1RSTR_USART5RST_Pos                (20U)
#define RCC_APB1RSTR_USART5RST_Msk                (0x1UL << RCC_APB1RSTR_USART5RST_Pos)             /*!< 0x00100000 */
#define RCC_APB1RSTR_USART5RST                    RCC_APB1RSTR_USART5RST_Msk                        /*!< desc USART5RST */
#define RCC_APB1RSTR_I2C1RST_Pos                  (21U)
#define RCC_APB1RSTR_I2C1RST_Msk                  (0x1UL << RCC_APB1RSTR_I2C1RST_Pos)               /*!< 0x00200000 */
#define RCC_APB1RSTR_I2C1RST                      RCC_APB1RSTR_I2C1RST_Msk                          /*!< desc I2C1RST */
#define RCC_APB1RSTR_I2C2RST_Pos                  (22U)
#define RCC_APB1RSTR_I2C2RST_Msk                  (0x1UL << RCC_APB1RSTR_I2C2RST_Pos)               /*!< 0x00400000 */
#define RCC_APB1RSTR_I2C2RST                      RCC_APB1RSTR_I2C2RST_Msk                          /*!< desc I2C2RST */
#define RCC_APB1RSTR_USBRST_Pos                   (23U)
#define RCC_APB1RSTR_USBRST_Msk                   (0x1UL << RCC_APB1RSTR_USBRST_Pos)                /*!< 0x00800000 */
#define RCC_APB1RSTR_USBRST                       RCC_APB1RSTR_USBRST_Msk                           /*!< desc USBRST */
#define RCC_APB1RSTR_CANRST_Pos                   (25U)
#define RCC_APB1RSTR_CANRST_Msk                   (0x1UL << RCC_APB1RSTR_CANRST_Pos)                /*!< 0x02000000 */
#define RCC_APB1RSTR_CANRST                       RCC_APB1RSTR_CANRST_Msk                           /*!< desc CANRST */
#define RCC_APB1RSTR_PWRRST_Pos                   (28U)
#define RCC_APB1RSTR_PWRRST_Msk                   (0x1UL << RCC_APB1RSTR_PWRRST_Pos)                /*!< 0x10000000 */
#define RCC_APB1RSTR_PWRRST                       RCC_APB1RSTR_PWRRST_Msk                           /*!< desc PWRRST */
#define RCC_APB1RSTR_CTCRST_Pos                   (31U)
#define RCC_APB1RSTR_CTCRST_Msk                   (0x1UL << RCC_APB1RSTR_CTCRST_Pos)                /*!< 0x80000000 */
#define RCC_APB1RSTR_CTCRST                       RCC_APB1RSTR_CTCRST_Msk                           /*!< CTCRST bits (desc CTCRST) */

/*!< RCC_AHB1ENR */
#define RCC_AHB1ENR_DMA1EN_Pos                    (0U)
#define RCC_AHB1ENR_DMA1EN_Msk                    (0x1UL << RCC_AHB1ENR_DMA1EN_Pos)                 /*!< 0x00000001 */
#define RCC_AHB1ENR_DMA1EN                        RCC_AHB1ENR_DMA1EN_Msk                            /*!< desc DMA1EN */
#define RCC_AHB1ENR_DMA2EN_Pos                    (1U)
#define RCC_AHB1ENR_DMA2EN_Msk                    (0x1UL << RCC_AHB1ENR_DMA2EN_Pos)                 /*!< 0x00000002 */
#define RCC_AHB1ENR_DMA2EN                        RCC_AHB1ENR_DMA2EN_Msk                            /*!< desc DMA2EN */
#define RCC_AHB1ENR_SRAMEN_Pos                    (2U)
#define RCC_AHB1ENR_SRAMEN_Msk                    (0x1UL << RCC_AHB1ENR_SRAMEN_Pos)                 /*!< 0x00000004 */
#define RCC_AHB1ENR_SRAMEN                        RCC_AHB1ENR_SRAMEN_Msk                            /*!< desc SRAMEN */
#define RCC_AHB1ENR_FMCEN_Pos                     (4U)
#define RCC_AHB1ENR_FMCEN_Msk                     (0x1UL << RCC_AHB1ENR_FMCEN_Pos)                  /*!< 0x00000010 */
#define RCC_AHB1ENR_FMCEN                         RCC_AHB1ENR_FMCEN_Msk                             /*!< desc FMCEN */
#define RCC_AHB1ENR_CRCEN_Pos                     (6U)
#define RCC_AHB1ENR_CRCEN_Msk                     (0x1UL << RCC_AHB1ENR_CRCEN_Pos)                  /*!< 0x00000040 */
#define RCC_AHB1ENR_CRCEN                         RCC_AHB1ENR_CRCEN_Msk                             /*!< desc CRCEN */
#define RCC_AHB1ENR_SDIOEN_Pos                    (10U)
#define RCC_AHB1ENR_SDIOEN_Msk                    (0x1UL << RCC_AHB1ENR_SDIOEN_Pos)                 /*!< 0x00000400 */
#define RCC_AHB1ENR_SDIOEN                        RCC_AHB1ENR_SDIOEN_Msk                            /*!< desc SDIOEN */
#define RCC_AHB1ENR_ESMCEN_Pos                    (12U)
#define RCC_AHB1ENR_ESMCEN_Msk                    (0x1UL << RCC_AHB1ENR_ESMCEN_Pos)                 /*!< 0x00001000 */
#define RCC_AHB1ENR_ESMCEN                        RCC_AHB1ENR_ESMCEN_Msk                            /*!< desc ESMCEN */

/*!< RCC_APB2ENR */
#define RCC_APB2ENR_SYSCFGEN_Pos                  (0U)
#define RCC_APB2ENR_SYSCFGEN_Msk                  (0x1UL << RCC_APB2ENR_SYSCFGEN_Pos)               /*!< 0x00000001 */
#define RCC_APB2ENR_SYSCFGEN                      RCC_APB2ENR_SYSCFGEN_Msk                          /*!< desc SYSCFGEN */
#define RCC_APB2ENR_ADC1EN_Pos                    (9U)
#define RCC_APB2ENR_ADC1EN_Msk                    (0x1UL << RCC_APB2ENR_ADC1EN_Pos)                 /*!< 0x00000200 */
#define RCC_APB2ENR_ADC1EN                        RCC_APB2ENR_ADC1EN_Msk                            /*!< desc ADC1EN */
#define RCC_APB2ENR_ADC2EN_Pos                    (10U)
#define RCC_APB2ENR_ADC2EN_Msk                    (0x1UL << RCC_APB2ENR_ADC2EN_Pos)                 /*!< 0x00000400 */
#define RCC_APB2ENR_ADC2EN                        RCC_APB2ENR_ADC2EN_Msk                            /*!< desc ADC2EN */
#define RCC_APB2ENR_TIM1EN_Pos                    (11U)
#define RCC_APB2ENR_TIM1EN_Msk                    (0x1UL << RCC_APB2ENR_TIM1EN_Pos)                 /*!< 0x00000800 */
#define RCC_APB2ENR_TIM1EN                        RCC_APB2ENR_TIM1EN_Msk                            /*!< desc TIM1EN */
#define RCC_APB2ENR_SPI1EN_Pos                    (12U)
#define RCC_APB2ENR_SPI1EN_Msk                    (0x1UL << RCC_APB2ENR_SPI1EN_Pos)                 /*!< 0x00001000 */
#define RCC_APB2ENR_SPI1EN                        RCC_APB2ENR_SPI1EN_Msk                            /*!< desc SPI1EN */
#define RCC_APB2ENR_TIM8EN_Pos                    (13U)
#define RCC_APB2ENR_TIM8EN_Msk                    (0x1UL << RCC_APB2ENR_TIM8EN_Pos)                 /*!< 0x00002000 */
#define RCC_APB2ENR_TIM8EN                        RCC_APB2ENR_TIM8EN_Msk                            /*!< desc TIM8EN */
#define RCC_APB2ENR_USART1EN_Pos                  (14U)
#define RCC_APB2ENR_USART1EN_Msk                  (0x1UL << RCC_APB2ENR_USART1EN_Pos)               /*!< 0x00004000 */
#define RCC_APB2ENR_USART1EN                      RCC_APB2ENR_USART1EN_Msk                          /*!< desc USART1EN */
#define RCC_APB2ENR_ADC3EN_Pos                    (15U)
#define RCC_APB2ENR_ADC3EN_Msk                    (0x1UL << RCC_APB2ENR_ADC3EN_Pos)                 /*!< 0x00008000 */
#define RCC_APB2ENR_ADC3EN                        RCC_APB2ENR_ADC3EN_Msk                            /*!< desc ADC3EN */
#define RCC_APB2ENR_TIM9EN_Pos                    (19U)
#define RCC_APB2ENR_TIM9EN_Msk                    (0x1UL << RCC_APB2ENR_TIM9EN_Pos)                 /*!< 0x00080000 */
#define RCC_APB2ENR_TIM9EN                        RCC_APB2ENR_TIM9EN_Msk                            /*!< desc TIM9EN */
#define RCC_APB2ENR_TIM10EN_Pos                   (20U)
#define RCC_APB2ENR_TIM10EN_Msk                   (0x1UL << RCC_APB2ENR_TIM10EN_Pos)                /*!< 0x00100000 */
#define RCC_APB2ENR_TIM10EN                       RCC_APB2ENR_TIM10EN_Msk                           /*!< desc TIM10EN */
#define RCC_APB2ENR_TIM11EN_Pos                   (21U)
#define RCC_APB2ENR_TIM11EN_Msk                   (0x1UL << RCC_APB2ENR_TIM11EN_Pos)                /*!< 0x00200000 */
#define RCC_APB2ENR_TIM11EN                       RCC_APB2ENR_TIM11EN_Msk                           /*!< desc TIM11EN */

/*!< RCC_APB1ENR */
#define RCC_APB1ENR_TIM2EN_Pos                    (0U)
#define RCC_APB1ENR_TIM2EN_Msk                    (0x1UL << RCC_APB1ENR_TIM2EN_Pos)                 /*!< 0x00000001 */
#define RCC_APB1ENR_TIM2EN                        RCC_APB1ENR_TIM2EN_Msk                            /*!< desc TIM2EN */
#define RCC_APB1ENR_TIM3EN_Pos                    (1U)
#define RCC_APB1ENR_TIM3EN_Msk                    (0x1UL << RCC_APB1ENR_TIM3EN_Pos)                 /*!< 0x00000002 */
#define RCC_APB1ENR_TIM3EN                        RCC_APB1ENR_TIM3EN_Msk                            /*!< desc TIM3EN */
#define RCC_APB1ENR_TIM4EN_Pos                    (2U)
#define RCC_APB1ENR_TIM4EN_Msk                    (0x1UL << RCC_APB1ENR_TIM4EN_Pos)                 /*!< 0x00000004 */
#define RCC_APB1ENR_TIM4EN                        RCC_APB1ENR_TIM4EN_Msk                            /*!< desc TIM4EN */
#define RCC_APB1ENR_TIM5EN_Pos                    (3U)
#define RCC_APB1ENR_TIM5EN_Msk                    (0x1UL << RCC_APB1ENR_TIM5EN_Pos)                 /*!< 0x00000008 */
#define RCC_APB1ENR_TIM5EN                        RCC_APB1ENR_TIM5EN_Msk                            /*!< desc TIM5EN */
#define RCC_APB1ENR_TIM6EN_Pos                    (4U)
#define RCC_APB1ENR_TIM6EN_Msk                    (0x1UL << RCC_APB1ENR_TIM6EN_Pos)                 /*!< 0x00000010 */
#define RCC_APB1ENR_TIM6EN                        RCC_APB1ENR_TIM6EN_Msk                            /*!< desc TIM6EN */
#define RCC_APB1ENR_TIM7EN_Pos                    (5U)
#define RCC_APB1ENR_TIM7EN_Msk                    (0x1UL << RCC_APB1ENR_TIM7EN_Pos)                 /*!< 0x00000020 */
#define RCC_APB1ENR_TIM7EN                        RCC_APB1ENR_TIM7EN_Msk                            /*!< desc TIM7EN */
#define RCC_APB1ENR_TIM12EN_Pos                   (6U)
#define RCC_APB1ENR_TIM12EN_Msk                   (0x1UL << RCC_APB1ENR_TIM12EN_Pos)                /*!< 0x00000040 */
#define RCC_APB1ENR_TIM12EN                       RCC_APB1ENR_TIM12EN_Msk                           /*!< desc TIM12EN */
#define RCC_APB1ENR_TIM13EN_Pos                   (7U)
#define RCC_APB1ENR_TIM13EN_Msk                   (0x1UL << RCC_APB1ENR_TIM13EN_Pos)                /*!< 0x00000080 */
#define RCC_APB1ENR_TIM13EN                       RCC_APB1ENR_TIM13EN_Msk                           /*!< desc TIM13EN */
#define RCC_APB1ENR_TIM14EN_Pos                   (8U)
#define RCC_APB1ENR_TIM14EN_Msk                   (0x1UL << RCC_APB1ENR_TIM14EN_Pos)                /*!< 0x00000100 */
#define RCC_APB1ENR_TIM14EN                       RCC_APB1ENR_TIM14EN_Msk                           /*!< desc TIM14EN */
#define RCC_APB1ENR_WWDGEN_Pos                    (11U)
#define RCC_APB1ENR_WWDGEN_Msk                    (0x1UL << RCC_APB1ENR_WWDGEN_Pos)                 /*!< 0x00000800 */
#define RCC_APB1ENR_WWDGEN                        RCC_APB1ENR_WWDGEN_Msk                            /*!< desc WWDGEN */
#define RCC_APB1ENR_SPI2EN_Pos                    (14U)
#define RCC_APB1ENR_SPI2EN_Msk                    (0x1UL << RCC_APB1ENR_SPI2EN_Pos)                 /*!< 0x00004000 */
#define RCC_APB1ENR_SPI2EN                        RCC_APB1ENR_SPI2EN_Msk                            /*!< desc SPI2EN */
#define RCC_APB1ENR_SPI3EN_Pos                    (15U)
#define RCC_APB1ENR_SPI3EN_Msk                    (0x1UL << RCC_APB1ENR_SPI3EN_Pos)                 /*!< 0x00008000 */
#define RCC_APB1ENR_SPI3EN                        RCC_APB1ENR_SPI3EN_Msk                            /*!< desc SPI3EN */
#define RCC_APB1ENR_USART2EN_Pos                  (17U)
#define RCC_APB1ENR_USART2EN_Msk                  (0x1UL << RCC_APB1ENR_USART2EN_Pos)               /*!< 0x00020000 */
#define RCC_APB1ENR_USART2EN                      RCC_APB1ENR_USART2EN_Msk                          /*!< desc USART2EN */
#define RCC_APB1ENR_USART3EN_Pos                  (18U)
#define RCC_APB1ENR_USART3EN_Msk                  (0x1UL << RCC_APB1ENR_USART3EN_Pos)               /*!< 0x00040000 */
#define RCC_APB1ENR_USART3EN                      RCC_APB1ENR_USART3EN_Msk                          /*!< desc USART3EN */
#define RCC_APB1ENR_USART4EN_Pos                  (19U)
#define RCC_APB1ENR_USART4EN_Msk                  (0x1UL << RCC_APB1ENR_USART4EN_Pos)               /*!< 0x00080000 */
#define RCC_APB1ENR_USART4EN                      RCC_APB1ENR_USART4EN_Msk                          /*!< desc USART4EN */
#define RCC_APB1ENR_USART5EN_Pos                  (20U)
#define RCC_APB1ENR_USART5EN_Msk                  (0x1UL << RCC_APB1ENR_USART5EN_Pos)               /*!< 0x00100000 */
#define RCC_APB1ENR_USART5EN                      RCC_APB1ENR_USART5EN_Msk                          /*!< desc USART5EN */
#define RCC_APB1ENR_I2C1EN_Pos                    (21U)
#define RCC_APB1ENR_I2C1EN_Msk                    (0x1UL << RCC_APB1ENR_I2C1EN_Pos)                 /*!< 0x00200000 */
#define RCC_APB1ENR_I2C1EN                        RCC_APB1ENR_I2C1EN_Msk                            /*!< desc I2C1EN */
#define RCC_APB1ENR_I2C2EN_Pos                    (22U)
#define RCC_APB1ENR_I2C2EN_Msk                    (0x1UL << RCC_APB1ENR_I2C2EN_Pos)                 /*!< 0x00400000 */
#define RCC_APB1ENR_I2C2EN                        RCC_APB1ENR_I2C2EN_Msk                            /*!< desc I2C2EN */
#define RCC_APB1ENR_USBEN_Pos                     (23U)
#define RCC_APB1ENR_USBEN_Msk                     (0x1UL << RCC_APB1ENR_USBEN_Pos)                  /*!< 0x00800000 */
#define RCC_APB1ENR_USBEN                         RCC_APB1ENR_USBEN_Msk                             /*!< desc USBEN */
#define RCC_APB1ENR_CANEN_Pos                     (25U)
#define RCC_APB1ENR_CANEN_Msk                     (0x1UL << RCC_APB1ENR_CANEN_Pos)                  /*!< 0x02000000 */
#define RCC_APB1ENR_CANEN                         RCC_APB1ENR_CANEN_Msk                             /*!< desc CANEN */
#define RCC_APB1ENR_BKPEN_Pos                     (27U)
#define RCC_APB1ENR_BKPEN_Msk                     (0x1UL << RCC_APB1ENR_BKPEN_Pos)                  /*!< 0x08000000 */
#define RCC_APB1ENR_BKPEN                         RCC_APB1ENR_BKPEN_Msk                             /*!< desc BKPEN */
#define RCC_APB1ENR_PWREN_Pos                     (28U)
#define RCC_APB1ENR_PWREN_Msk                     (0x1UL << RCC_APB1ENR_PWREN_Pos)                  /*!< 0x10000000 */
#define RCC_APB1ENR_PWREN                         RCC_APB1ENR_PWREN_Msk                             /*!< desc PWREN */
#define RCC_APB1ENR_CTCEN_Pos                     (31U)
#define RCC_APB1ENR_CTCEN_Msk                     (0x1UL << RCC_APB1ENR_CTCEN_Pos)                  /*!< 0x80000000 */
#define RCC_APB1ENR_CTCEN                         RCC_APB1ENR_CTCEN_Msk                             /*!< desc CTCEN */

/*!< RCC_BDCR */
#define RCC_BDCR_LSEON_Pos                        (0U)
#define RCC_BDCR_LSEON_Msk                        (0x1UL << RCC_BDCR_LSEON_Pos)                     /*!< 0x00000001 */
#define RCC_BDCR_LSEON                            RCC_BDCR_LSEON_Msk                                /*!< desc LSEON */
#define RCC_BDCR_LSERDY_Pos                       (1U)
#define RCC_BDCR_LSERDY_Msk                       (0x1UL << RCC_BDCR_LSERDY_Pos)                    /*!< 0x00000002 */
#define RCC_BDCR_LSERDY                           RCC_BDCR_LSERDY_Msk                               /*!< desc LSERDY */
#define RCC_BDCR_LSEBYP_Pos                       (2U)
#define RCC_BDCR_LSEBYP_Msk                       (0x1UL << RCC_BDCR_LSEBYP_Pos)                    /*!< 0x00000004 */
#define RCC_BDCR_LSEBYP                           RCC_BDCR_LSEBYP_Msk                               /*!< desc LSEBYP */
#define RCC_BDCR_LSEDRV_Pos                       (3U)
#define RCC_BDCR_LSEDRV_Msk                       (0x3UL << RCC_BDCR_LSEDRV_Pos)                    /*!< 0x00000018 */
#define RCC_BDCR_LSEDRV                           RCC_BDCR_LSEDRV_Msk                               /*!< LSEDRV[4:3] bits (desc LSEDRV) */
#define RCC_BDCR_LSEDRV_0                         (0x1UL << RCC_BDCR_LSEDRV_Pos)                    /*!< 0x00000008 */
#define RCC_BDCR_LSEDRV_1                         (0x2UL << RCC_BDCR_LSEDRV_Pos)                    /*!< 0x00000010 */

#define RCC_BDCR_RTCSEL_Pos                       (8U)
#define RCC_BDCR_RTCSEL_Msk                       (0x3UL << RCC_BDCR_RTCSEL_Pos)                    /*!< 0x00000300 */
#define RCC_BDCR_RTCSEL                           RCC_BDCR_RTCSEL_Msk                               /*!< RTCSEL[9:8] bits (desc RTCSEL) */
#define RCC_BDCR_RTCSEL_0                         (0x1UL << RCC_BDCR_RTCSEL_Pos)                    /*!< 0x00000100 */
#define RCC_BDCR_RTCSEL_1                         (0x2UL << RCC_BDCR_RTCSEL_Pos)                    /*!< 0x00000200 */

#define RCC_BDCR_RSTOUT_DIS_Pos                   (10U)
#define RCC_BDCR_RSTOUT_DIS_Msk                   (0x1UL << RCC_BDCR_RSTOUT_DIS_Pos)                /*!< 0x00000400 */
#define RCC_BDCR_RSTOUT_DIS                       RCC_BDCR_RSTOUT_DIS_Msk                           /*!< desc RSTOUT_DIS */

#define RCC_BDCR_RTCSEL_NOCLOCK                   0x00000000U                                       /*!< No clock */
#define RCC_BDCR_RTCSEL_LSE                       0x00000100U                                       /*!< LSE oscillator clock used as RTC clock */
#define RCC_BDCR_RTCSEL_LSI                       0x00000200U                                       /*!< LSI oscillator clock used as RTC clock */
#define RCC_BDCR_RTCSEL_HSE                       0x00000300U                                       /*!< HSE oscillator clock divided by 128 used as RTC clock */
#define RCC_BDCR_RTCEN_Pos                        (15U)
#define RCC_BDCR_RTCEN_Msk                        (0x1UL << RCC_BDCR_RTCEN_Pos)                     /*!< 0x00008000 */
#define RCC_BDCR_RTCEN                            RCC_BDCR_RTCEN_Msk                                /*!< desc RTCEN */
#define RCC_BDCR_BDRST_Pos                        (16U)
#define RCC_BDCR_BDRST_Msk                        (0x1UL << RCC_BDCR_BDRST_Pos)                     /*!< 0x00010000 */
#define RCC_BDCR_BDRST                            RCC_BDCR_BDRST_Msk                                /*!< desc BDRST */

/*!< RCC_CSR */
#define RCC_CSR_LSION_Pos                         (0U)
#define RCC_CSR_LSION_Msk                         (0x1UL << RCC_CSR_LSION_Pos)                      /*!< 0x00000001 */
#define RCC_CSR_LSION                             RCC_CSR_LSION_Msk                                 /*!< desc LSION */
#define RCC_CSR_LSIRDY_Pos                        (1U)
#define RCC_CSR_LSIRDY_Msk                        (0x1UL << RCC_CSR_LSIRDY_Pos)                     /*!< 0x00000002 */
#define RCC_CSR_LSIRDY                            RCC_CSR_LSIRDY_Msk                                /*!< desc LSIRDY */
#define RCC_CSR_RMVF_Pos                          (24U)
#define RCC_CSR_RMVF_Msk                          (0x1UL << RCC_CSR_RMVF_Pos)                       /*!< 0x01000000 */
#define RCC_CSR_RMVF                              RCC_CSR_RMVF_Msk                                  /*!< desc RMVF */
#define RCC_CSR_OBLRSTF_Pos                       (25U)
#define RCC_CSR_OBLRSTF_Msk                       (0x1UL << RCC_CSR_OBLRSTF_Pos)                    /*!< 0x02000000 */
#define RCC_CSR_OBLRSTF                           RCC_CSR_OBLRSTF_Msk                               /*!< desc OBLRSTF */
#define RCC_CSR_PINRSTF_Pos                       (26U)
#define RCC_CSR_PINRSTF_Msk                       (0x1UL << RCC_CSR_PINRSTF_Pos)                    /*!< 0x04000000 */
#define RCC_CSR_PINRSTF                           RCC_CSR_PINRSTF_Msk                               /*!< desc PINRSTF */
#define RCC_CSR_PWRRSTF_Pos                       (27U)
#define RCC_CSR_PWRRSTF_Msk                       (0x1UL << RCC_CSR_PWRRSTF_Pos)                    /*!< 0x08000000 */
#define RCC_CSR_PWRRSTF                           RCC_CSR_PWRRSTF_Msk                               /*!< desc PWRRSTF */
#define RCC_CSR_SFTRSTF_Pos                       (28U)
#define RCC_CSR_SFTRSTF_Msk                       (0x1UL << RCC_CSR_SFTRSTF_Pos)                    /*!< 0x10000000 */
#define RCC_CSR_SFTRSTF                           RCC_CSR_SFTRSTF_Msk                               /*!< desc SFTRSTF */
#define RCC_CSR_IWDGRSTF_Pos                      (29U)
#define RCC_CSR_IWDGRSTF_Msk                      (0x1UL << RCC_CSR_IWDGRSTF_Pos)                   /*!< 0x20000000 */
#define RCC_CSR_IWDGRSTF                          RCC_CSR_IWDGRSTF_Msk                              /*!< desc IWDGRSTF */
#define RCC_CSR_WWDGRSTF_Pos                      (30U)
#define RCC_CSR_WWDGRSTF_Msk                      (0x1UL << RCC_CSR_WWDGRSTF_Pos)                   /*!< 0x40000000 */
#define RCC_CSR_WWDGRSTF                          RCC_CSR_WWDGRSTF_Msk                              /*!< desc WWDGRSTF */
#define RCC_CSR_LPWRRSTF_Pos                      (31U)
#define RCC_CSR_LPWRRSTF_Msk                      (0x1UL << RCC_CSR_LPWRRSTF_Pos)                   /*!< 0x80000000 */
#define RCC_CSR_LPWRRSTF                          RCC_CSR_LPWRRSTF_Msk                              /*!< desc LPWRRSTF */

/*!< RCC_CFGR1 */
#define RCC_CFGR1_MCOPRE_Pos                      (0U)
#define RCC_CFGR1_MCOPRE_Msk                      (0x7UL << RCC_CFGR1_MCOPRE_Pos)                   /*!< 0x00000007 */
#define RCC_CFGR1_MCOPRE                          RCC_CFGR1_MCOPRE_Msk                              /*!< MCOPRE[2:0] bits (desc MCOPRE) */
#define RCC_CFGR1_MCOPRE_0                        (0x1UL << RCC_CFGR1_MCOPRE_Pos)                   /*!< 0x00000001 */
#define RCC_CFGR1_MCOPRE_1                        (0x2UL << RCC_CFGR1_MCOPRE_Pos)                   /*!< 0x00000002 */
#define RCC_CFGR1_MCOPRE_2                        (0x4UL << RCC_CFGR1_MCOPRE_Pos)                   /*!< 0x00000004 */

#define RCC_CFGR1_HSI48ON_Pos                     (4U)
#define RCC_CFGR1_HSI48ON_Msk                     (0x1UL << RCC_CFGR1_HSI48ON_Pos)                  /*!< 0x00000010 */
#define RCC_CFGR1_HSI48ON                         RCC_CFGR1_HSI48ON_Msk                             /*!< desc HSI48ON */
#define RCC_CFGR1_HSI48RDY_Pos                    (5U)
#define RCC_CFGR1_HSI48RDY_Msk                    (0x1UL << RCC_CFGR1_HSI48RDY_Pos)                 /*!< 0x00000020 */
#define RCC_CFGR1_HSI48RDY                        RCC_CFGR1_HSI48RDY_Msk                            /*!< desc HSI48RDY */
#define RCC_CFGR1_HSI48TRIM_Pos                   (9U)
#define RCC_CFGR1_HSI48TRIM_Msk                   (0x7FUL << RCC_CFGR1_HSI48TRIM_Pos)               /*!< 0x0000FE00 */
#define RCC_CFGR1_HSI48TRIM                       RCC_CFGR1_HSI48TRIM_Msk                           /*!< HSI48TRIM[15:9] bits (desc HSI48TRIM) */
#define RCC_CFGR1_HSI48TRIM_0                     (0x1UL << RCC_CFGR1_HSI48TRIM_Pos)                /*!< 0x00000200 */
#define RCC_CFGR1_HSI48TRIM_1                     (0x2UL << RCC_CFGR1_HSI48TRIM_Pos)                /*!< 0x00000400 */
#define RCC_CFGR1_HSI48TRIM_2                     (0x4UL << RCC_CFGR1_HSI48TRIM_Pos)                /*!< 0x00000800 */
#define RCC_CFGR1_HSI48TRIM_3                     (0x8UL << RCC_CFGR1_HSI48TRIM_Pos)                /*!< 0x00001000 */
#define RCC_CFGR1_HSI48TRIM_4                     (0x10UL << RCC_CFGR1_HSI48TRIM_Pos)               /*!< 0x00002000 */
#define RCC_CFGR1_HSI48TRIM_5                     (0x20UL << RCC_CFGR1_HSI48TRIM_Pos)               /*!< 0x00004000 */
#define RCC_CFGR1_HSI48TRIM_6                     (0x40UL << RCC_CFGR1_HSI48TRIM_Pos)               /*!< 0x00008000 */

#define RCC_CFGR1_HSI48CAL_Pos                    (16U)
#define RCC_CFGR1_HSI48CAL_Msk                    (0x1FFFUL << RCC_CFGR1_HSI48CAL_Pos)              /*!< 0x1FFF0000 */
#define RCC_CFGR1_HSI48CAL                        RCC_CFGR1_HSI48CAL_Msk                            /*!< HSI48CAL[28:16] bits (desc HSI48CAL) */

#define RCC_CFGR1_USBSELHSI48_Pos                 (31U)
#define RCC_CFGR1_USBSELHSI48_Msk                 (0x1UL << RCC_CFGR1_USBSELHSI48_Pos)              /*!< 0x80000000 */
#define RCC_CFGR1_USBSELHSI48                     RCC_CFGR1_USBSELHSI48_Msk                         /*!< desc USBSELHSI48 */

/*!< RCC_AHB1RSTR */
#define RCC_AHB1RSTR_DMA1RST_Pos                  (0U)
#define RCC_AHB1RSTR_DMA1RST_Msk                  (0x1UL << RCC_AHB1RSTR_DMA1RST_Pos)               /*!< 0x00000001 */
#define RCC_AHB1RSTR_DMA1RST                      RCC_AHB1RSTR_DMA1RST_Msk                          /*!< desc DMA1RST */
#define RCC_AHB1RSTR_DMA2RST_Pos                  (1U)
#define RCC_AHB1RSTR_DMA2RST_Msk                  (0x1UL << RCC_AHB1RSTR_DMA2RST_Pos)               /*!< 0x00000002 */
#define RCC_AHB1RSTR_DMA2RST                      RCC_AHB1RSTR_DMA2RST_Msk                          /*!< desc DMA2RST */
#define RCC_AHB1RSTR_CRCRST_Pos                   (6U)
#define RCC_AHB1RSTR_CRCRST_Msk                   (0x1UL << RCC_AHB1RSTR_CRCRST_Pos)                /*!< 0x00000040 */
#define RCC_AHB1RSTR_CRCRST                       RCC_AHB1RSTR_CRCRST_Msk                           /*!< desc CRCRST */
#define RCC_AHB1RSTR_SDIORST_Pos                  (10U)
#define RCC_AHB1RSTR_SDIORST_Msk                  (0x1UL << RCC_AHB1RSTR_SDIORST_Pos)               /*!< 0x00000400 */
#define RCC_AHB1RSTR_SDIORST                      RCC_AHB1RSTR_SDIORST_Msk                          /*!< desc SDIORST */
#define RCC_AHB1RSTR_ESMCRST_Pos                  (12U)
#define RCC_AHB1RSTR_ESMCRST_Msk                  (0x1UL << RCC_AHB1RSTR_ESMCRST_Pos)               /*!< 0x00001000 */
#define RCC_AHB1RSTR_ESMCRST                      RCC_AHB1RSTR_ESMCRST_Msk                          /*!< desc ESMCRST */

/*!< RCC_AHB2RSTR */
#define RCC_AHB2RSTR_IOPARST_Pos                  (2U)
#define RCC_AHB2RSTR_IOPARST_Msk                  (0x1UL << RCC_AHB2RSTR_IOPARST_Pos)               /*!< 0x00000004 */
#define RCC_AHB2RSTR_IOPARST                      RCC_AHB2RSTR_IOPARST_Msk                          /*!< desc IOPARST */
#define RCC_AHB2RSTR_IOPBRST_Pos                  (3U)
#define RCC_AHB2RSTR_IOPBRST_Msk                  (0x1UL << RCC_AHB2RSTR_IOPBRST_Pos)               /*!< 0x00000008 */
#define RCC_AHB2RSTR_IOPBRST                      RCC_AHB2RSTR_IOPBRST_Msk                          /*!< desc IOPBRST */
#define RCC_AHB2RSTR_IOPCRST_Pos                  (4U)
#define RCC_AHB2RSTR_IOPCRST_Msk                  (0x1UL << RCC_AHB2RSTR_IOPCRST_Pos)               /*!< 0x00000010 */
#define RCC_AHB2RSTR_IOPCRST                      RCC_AHB2RSTR_IOPCRST_Msk                          /*!< desc IOPCRST */
#define RCC_AHB2RSTR_IOPDRST_Pos                  (5U)
#define RCC_AHB2RSTR_IOPDRST_Msk                  (0x1UL << RCC_AHB2RSTR_IOPDRST_Pos)               /*!< 0x00000020 */
#define RCC_AHB2RSTR_IOPDRST                      RCC_AHB2RSTR_IOPDRST_Msk                          /*!< desc IOPDRST */
#define RCC_AHB2RSTR_IOPERST_Pos                  (6U)
#define RCC_AHB2RSTR_IOPERST_Msk                  (0x1UL << RCC_AHB2RSTR_IOPERST_Pos)               /*!< 0x00000040 */
#define RCC_AHB2RSTR_IOPERST                      RCC_AHB2RSTR_IOPERST_Msk                          /*!< desc IOPERST */

/*!< RCC_AHB2ENR */
#define RCC_AHB2ENR_IOPAEN_Pos                    (2U)
#define RCC_AHB2ENR_IOPAEN_Msk                    (0x1UL << RCC_AHB2ENR_IOPAEN_Pos)                 /*!< 0x00000004 */
#define RCC_AHB2ENR_IOPAEN                        RCC_AHB2ENR_IOPAEN_Msk                            /*!< desc IOPAEN */
#define RCC_AHB2ENR_IOPBEN_Pos                    (3U)
#define RCC_AHB2ENR_IOPBEN_Msk                    (0x1UL << RCC_AHB2ENR_IOPBEN_Pos)                 /*!< 0x00000008 */
#define RCC_AHB2ENR_IOPBEN                        RCC_AHB2ENR_IOPBEN_Msk                            /*!< desc IOPBEN */
#define RCC_AHB2ENR_IOPCEN_Pos                    (4U)
#define RCC_AHB2ENR_IOPCEN_Msk                    (0x1UL << RCC_AHB2ENR_IOPCEN_Pos)                 /*!< 0x00000010 */
#define RCC_AHB2ENR_IOPCEN                        RCC_AHB2ENR_IOPCEN_Msk                            /*!< desc IOPCEN */
#define RCC_AHB2ENR_IOPDEN_Pos                    (5U)
#define RCC_AHB2ENR_IOPDEN_Msk                    (0x1UL << RCC_AHB2ENR_IOPDEN_Pos)                 /*!< 0x00000020 */
#define RCC_AHB2ENR_IOPDEN                        RCC_AHB2ENR_IOPDEN_Msk                            /*!< desc IOPDEN */
#define RCC_AHB2ENR_IOPEEN_Pos                    (6U)
#define RCC_AHB2ENR_IOPEEN_Msk                    (0x1UL << RCC_AHB2ENR_IOPEEN_Pos)                 /*!< 0x00000040 */
#define RCC_AHB2ENR_IOPEEN                        RCC_AHB2ENR_IOPEEN_Msk                            /*!< desc IOPEEN */

/*!< RCC_CFGR2 */
#define RCC_CFGR2_CANCKSEL_Pos                    (0U)
#define RCC_CFGR2_CANCKSEL_Msk                    (0xFUL << RCC_CFGR2_CANCKSEL_Pos)                 /*!< 0x0000000F */
#define RCC_CFGR2_CANCKSEL                        RCC_CFGR2_CANCKSEL_Msk                            /*!< CANCKSEL[3:0] bits (desc CANCKSEL) */
#define RCC_CFGR2_CANCKSEL_0                      (0x1UL << RCC_CFGR2_CANCKSEL_Pos)                 /*!< 0x00000001 */
#define RCC_CFGR2_CANCKSEL_1                      (0x2UL << RCC_CFGR2_CANCKSEL_Pos)                 /*!< 0x00000002 */
#define RCC_CFGR2_CANCKSEL_2                      (0x4UL << RCC_CFGR2_CANCKSEL_Pos)                 /*!< 0x00000004 */
#define RCC_CFGR2_CANCKSEL_3                      (0x8UL << RCC_CFGR2_CANCKSEL_Pos)                 /*!< 0x00000008 */

#define RCC_CFGR2_HSEDRV_Pos                      (8U)
#define RCC_CFGR2_HSEDRV_Msk                      (0x3UL << RCC_CFGR2_HSEDRV_Pos)                   /*!< 0x00000300 */
#define RCC_CFGR2_HSEDRV                          RCC_CFGR2_HSEDRV_Msk                              /*!< HSEDRV[9:8] bits (desc HSEDRV) */
#define RCC_CFGR2_HSEDRV_0                        (0x1UL << RCC_CFGR2_HSEDRV_Pos)                   /*!< 0x00000100 */
#define RCC_CFGR2_HSEDRV_1                        (0x2UL << RCC_CFGR2_HSEDRV_Pos)                   /*!< 0x00000200 */

/*********************  Bits Define For Peripheral RTC  *********************/
/*!< RTC_CRH */
#define RTC_CRH_SECIE_Pos                         (0U)
#define RTC_CRH_SECIE_Msk                         (0x1UL << RTC_CRH_SECIE_Pos)                      /*!< 0x00000001 */
#define RTC_CRH_SECIE                             RTC_CRH_SECIE_Msk                                 /*!< desc SECIE */
#define RTC_CRH_ALRIE_Pos                         (1U)
#define RTC_CRH_ALRIE_Msk                         (0x1UL << RTC_CRH_ALRIE_Pos)                      /*!< 0x00000002 */
#define RTC_CRH_ALRIE                             RTC_CRH_ALRIE_Msk                                 /*!< desc ALRIE */
#define RTC_CRH_OWIE_Pos                          (2U)
#define RTC_CRH_OWIE_Msk                          (0x1UL << RTC_CRH_OWIE_Pos)                       /*!< 0x00000004 */
#define RTC_CRH_OWIE                              RTC_CRH_OWIE_Msk                                  /*!< desc OWIE */

/*!< RTC_CRL */
#define RTC_CRL_SECF_Pos                          (0U)
#define RTC_CRL_SECF_Msk                          (0x1UL << RTC_CRL_SECF_Pos)                       /*!< 0x00000001 */
#define RTC_CRL_SECF                              RTC_CRL_SECF_Msk                                  /*!< desc SECF */
#define RTC_CRL_ALRF_Pos                          (1U)
#define RTC_CRL_ALRF_Msk                          (0x1UL << RTC_CRL_ALRF_Pos)                       /*!< 0x00000002 */
#define RTC_CRL_ALRF                              RTC_CRL_ALRF_Msk                                  /*!< desc ALRF */
#define RTC_CRL_OWF_Pos                           (2U)
#define RTC_CRL_OWF_Msk                           (0x1UL << RTC_CRL_OWF_Pos)                        /*!< 0x00000004 */
#define RTC_CRL_OWF                               RTC_CRL_OWF_Msk                                   /*!< desc OWF */
#define RTC_CRL_RSF_Pos                           (3U)
#define RTC_CRL_RSF_Msk                           (0x1UL << RTC_CRL_RSF_Pos)                        /*!< 0x00000008 */
#define RTC_CRL_RSF                               RTC_CRL_RSF_Msk                                   /*!< desc RSF */
#define RTC_CRL_CNF_Pos                           (4U)
#define RTC_CRL_CNF_Msk                           (0x1UL << RTC_CRL_CNF_Pos)                        /*!< 0x00000010 */
#define RTC_CRL_CNF                               RTC_CRL_CNF_Msk                                   /*!< desc CNF */
#define RTC_CRL_RTOFF_Pos                         (5U)
#define RTC_CRL_RTOFF_Msk                         (0x1UL << RTC_CRL_RTOFF_Pos)                      /*!< 0x00000020 */
#define RTC_CRL_RTOFF                             RTC_CRL_RTOFF_Msk                                 /*!< desc RTOFF */

/*!< RTC_PRLH */
#define RTC_PRLH_PRL_Pos                          (0U)
#define RTC_PRLH_PRL_Msk                          (0xFUL << RTC_PRLH_PRL_Pos)                       /*!< 0x0000000F */
#define RTC_PRLH_PRL                              RTC_PRLH_PRL_Msk                                  /*!< PRL[3:0] bits (desc PRL) */
#define RTC_PRLH_PRL_0                            (0x1UL << RTC_PRLH_PRL_Pos)                       /*!< 0x00000001 */
#define RTC_PRLH_PRL_1                            (0x2UL << RTC_PRLH_PRL_Pos)                       /*!< 0x00000002 */
#define RTC_PRLH_PRL_2                            (0x4UL << RTC_PRLH_PRL_Pos)                       /*!< 0x00000004 */
#define RTC_PRLH_PRL_3                            (0x8UL << RTC_PRLH_PRL_Pos)                       /*!< 0x00000008 */


/*!< RTC_PRLL */
#define RTC_PRLL_PRL_Pos                          (0U)
#define RTC_PRLL_PRL_Msk                          (0xFFFFUL << RTC_PRLL_PRL_Pos)                    /*!< 0x0000FFFF */
#define RTC_PRLL_PRL                              RTC_PRLL_PRL_Msk                                  /*!< PRL[15:0] bits (desc PRL) */

/*!< RTC_DIVH */
#define RTC_DIVH_DIV_Pos                          (0U)
#define RTC_DIVH_DIV_Msk                          (0xFUL << RTC_DIVH_DIV_Pos)                       /*!< 0x0000000F */
#define RTC_DIVH_DIV                              RTC_DIVH_DIV_Msk                                  /*!< RTC_DIV[19:16] bits (desc DIV) */

/*!< RTC_DIVL */
#define RTC_DIVL_DIV_Pos                          (0U)
#define RTC_DIVL_DIV_Msk                          (0xFFFFUL << RTC_DIVL_DIV_Pos)                       /*!< 0x0000FFFF */
#define RTC_DIVL_DIV                              RTC_DIVL_DIV_Msk                                  /*!< DIV[15:0] bits (desc DIV) */

/*!< RTC_CNTH */
#define RTC_CNTH_RTC_CNT_Pos                      (0U)
#define RTC_CNTH_RTC_CNT_Msk                      (0xFFFFUL << RTC_CNTH_RTC_CNT_Pos)                /*!< 0x0000FFFF */
#define RTC_CNTH_RTC_CNT                          RTC_CNTH_RTC_CNT_Msk                              /*!< RTC_CNT[15:0] bits (desc RTC_CNT) */

/*!< RTC_CNTL */
#define RTC_CNTL_RTC_CNT_Pos                      (0U)
#define RTC_CNTL_RTC_CNT_Msk                      (0xFFFFUL << RTC_CNTL_RTC_CNT_Pos)                /*!< 0x0000FFFF */
#define RTC_CNTL_RTC_CNT                          RTC_CNTL_RTC_CNT_Msk                              /*!< RTC_CNT[15:0] bits (desc RTC_CNT) */

/*!< RTC_ALRH */
#define RTC_ALRH_RTC_ALR_Pos                      (0U)
#define RTC_ALRH_RTC_ALR_Msk                      (0xFFFFUL << RTC_ALRH_RTC_ALR_Pos)                /*!< 0x0000FFFF */
#define RTC_ALRH_RTC_ALR                          RTC_ALRH_RTC_ALR_Msk                              /*!< RTC_ALR[15:0] bits (desc RTC_ALR) */

/*!< RTC_ALRL */
#define RTC_ALRL_RTC_ALR_Pos                      (0U)
#define RTC_ALRL_RTC_ALR_Msk                      (0xFFFFUL << RTC_ALRL_RTC_ALR_Pos)                /*!< 0x0000FFFF */
#define RTC_ALRL_RTC_ALR                          RTC_ALRL_RTC_ALR_Msk                              /*!< RTC_ALR[15:0] bits (desc RTC_ALR) */

/*********************  Bits Define For Peripheral SDIO  *********************/
/*!< SDIO_POWER */
#define SDIO_POWER_PWRCTRL_Pos                    (0U)
#define SDIO_POWER_PWRCTRL_Msk                    (0x1UL << SDIO_POWER_PWRCTRL_Pos)                 /*!< 0x00000001 */
#define SDIO_POWER_PWRCTRL                        SDIO_POWER_PWRCTRL_Msk                            /*!< desc PWRCTRL */

/*!< SDIO_CLKCR */
#define SDIO_CLKCR_CLKDIV_Pos                     (0U)
#define SDIO_CLKCR_CLKDIV_Msk                     (0xFFUL << SDIO_CLKCR_CLKDIV_Pos)                 /*!< 0x000000FF */
#define SDIO_CLKCR_CLKDIV                         SDIO_CLKCR_CLKDIV_Msk                             /*!< CLKDIV[7:0] bits (desc CLKDIV) */
#define SDIO_CLKCR_CLKEN_Pos                      (8U)
#define SDIO_CLKCR_CLKEN_Msk                      (0x1UL << SDIO_CLKCR_CLKEN_Pos)                   /*!< 0x00000100 */
#define SDIO_CLKCR_CLKEN                          SDIO_CLKCR_CLKEN_Msk                              /*!< desc CLKEN */
#define SDIO_CLKCR_PWRSAV_Pos                     (9U)
#define SDIO_CLKCR_PWRSAV_Msk                     (0x1UL << SDIO_CLKCR_PWRSAV_Pos)                  /*!< 0x00000200 */
#define SDIO_CLKCR_PWRSAV                         SDIO_CLKCR_PWRSAV_Msk                             /*!< desc PWRSAV */
#define SDIO_CLKCR_WIDBUS_Pos                     (10U)
#define SDIO_CLKCR_WIDBUS_Msk                     (0x3UL << SDIO_CLKCR_WIDBUS_Pos)                  /*!< 0x00000C00 */
#define SDIO_CLKCR_WIDBUS                         SDIO_CLKCR_WIDBUS_Msk                             /*!< WIDBUS[11:10] bits (desc WIDBUS) */
#define SDIO_CLKCR_WIDBUS_0                       (0x1UL << SDIO_CLKCR_WIDBUS_Pos)                  /*!< 0x00000400 */
#define SDIO_CLKCR_WIDBUS_1                       (0x2UL << SDIO_CLKCR_WIDBUS_Pos)                  /*!< 0x00000800 */

#define SDIO_CLKCR_SMPCLKSEL_Pos                  (12U)
#define SDIO_CLKCR_SMPCLKSEL_Msk                  (0x1UL << SDIO_CLKCR_SMPCLKSEL_Pos)               /*!< 0x00001000 */
#define SDIO_CLKCR_SMPCLKSEL                      SDIO_CLKCR_SMPCLKSEL_Msk                          /*!< desc SMPCLKSEL */
#define SDIO_CLKCR_SMPEN_Pos                      (13U)
#define SDIO_CLKCR_SMPEN_Msk                      (0x1UL << SDIO_CLKCR_SMPEN_Pos)                   /*!< 0x00002000 */
#define SDIO_CLKCR_SMPEN                          SDIO_CLKCR_SMPEN_Msk                              /*!< desc SMPEN */
#define SDIO_CLKCR_CKSEL_Pos                      (14U)
#define SDIO_CLKCR_CKSEL_Msk                      (0x1UL << SDIO_CLKCR_CKSEL_Pos)                   /*!< 0x00004000 */
#define SDIO_CLKCR_CKSEL                          SDIO_CLKCR_CKSEL_Msk                              /*!< desc CKSEL */

/*!< SDIO_ARG */
#define SDIO_ARG_CMDARG_Pos                       (0U)
#define SDIO_ARG_CMDARG_Msk                       (0xFFFFFFFFUL << SDIO_ARG_CMDARG_Pos)             /*!< 0xFFFFFFFF */
#define SDIO_ARG_CMDARG                           SDIO_ARG_CMDARG_Msk                               /*!< CMDARG[31:0] bits (desc CMDARG) */

/*!< SDIO_CMD */
#define SDIO_CMD_CMDINDEX_Pos                     (0U)
#define SDIO_CMD_CMDINDEX_Msk                     (0x3FUL << SDIO_CMD_CMDINDEX_Pos)                 /*!< 0x0000003F */
#define SDIO_CMD_CMDINDEX                         SDIO_CMD_CMDINDEX_Msk                             /*!< CMDINDEX[5:0] bits (desc CMDINDEX) */
#define SDIO_CMD_CMDINDEX_0                       (0x1UL << SDIO_CMD_CMDINDEX_Pos)                  /*!< 0x00000001 */
#define SDIO_CMD_CMDINDEX_1                       (0x2UL << SDIO_CMD_CMDINDEX_Pos)                  /*!< 0x00000002 */
#define SDIO_CMD_CMDINDEX_2                       (0x4UL << SDIO_CMD_CMDINDEX_Pos)                  /*!< 0x00000004 */
#define SDIO_CMD_CMDINDEX_3                       (0x8UL << SDIO_CMD_CMDINDEX_Pos)                  /*!< 0x00000008 */
#define SDIO_CMD_CMDINDEX_4                       (0x10UL << SDIO_CMD_CMDINDEX_Pos)                 /*!< 0x00000010 */
#define SDIO_CMD_CMDINDEX_5                       (0x20UL << SDIO_CMD_CMDINDEX_Pos)                 /*!< 0x00000020 */

#define SDIO_CMD_WAITRESP_Pos                     (6U)
#define SDIO_CMD_WAITRESP_Msk                     (0x1UL << SDIO_CMD_WAITRESP_Pos)                  /*!< 0x00000040 */
#define SDIO_CMD_WAITRESP                         SDIO_CMD_WAITRESP_Msk                             /*!< desc WAITRESP */
#define SDIO_CMD_RESPLEN_Pos                      (7U)
#define SDIO_CMD_RESPLEN_Msk                      (0x1UL << SDIO_CMD_RESPLEN_Pos)                   /*!< 0x00000080 */
#define SDIO_CMD_RESPLEN                          SDIO_CMD_RESPLEN_Msk                              /*!< desc RESPLEN */
#define SDIO_CMD_CHECKRESPCRC_Pos                 (8U)
#define SDIO_CMD_CHECKRESPCRC_Msk                 (0x1UL << SDIO_CMD_CHECKRESPCRC_Pos)              /*!< 0x00000100 */
#define SDIO_CMD_CHECKRESPCRC                     SDIO_CMD_CHECKRESPCRC_Msk                         /*!< desc CHECKRESPCRC */
#define SDIO_CMD_DEXPECT_Pos                      (9U)
#define SDIO_CMD_DEXPECT_Msk                      (0x1UL << SDIO_CMD_DEXPECT_Pos)                   /*!< 0x00000200 */
#define SDIO_CMD_DEXPECT                          SDIO_CMD_DEXPECT_Msk                              /*!< desc DEXPECT */
#define SDIO_CMD_DIR_Pos                          (10U)
#define SDIO_CMD_DIR_Msk                          (0x1UL << SDIO_CMD_DIR_Pos)                       /*!< 0x00000400 */
#define SDIO_CMD_DIR                              SDIO_CMD_DIR_Msk                                  /*!< desc DIR */
#define SDIO_CMD_DTMODE_Pos                       (11U)
#define SDIO_CMD_DTMODE_Msk                       (0x1UL << SDIO_CMD_DTMODE_Pos)                    /*!< 0x00000800 */
#define SDIO_CMD_DTMODE                           SDIO_CMD_DTMODE_Msk                               /*!< desc DTMODE */
#define SDIO_CMD_AUTOSTOP_Pos                     (12U)
#define SDIO_CMD_AUTOSTOP_Msk                     (0x1UL << SDIO_CMD_AUTOSTOP_Pos)                  /*!< 0x00001000 */
#define SDIO_CMD_AUTOSTOP                         SDIO_CMD_AUTOSTOP_Msk                             /*!< desc AUTOSTOP */
#define SDIO_CMD_WAITPEND_Pos                     (13U)
#define SDIO_CMD_WAITPEND_Msk                     (0x1UL << SDIO_CMD_WAITPEND_Pos)                  /*!< 0x00002000 */
#define SDIO_CMD_WAITPEND                         SDIO_CMD_WAITPEND_Msk                             /*!< desc WAITPEND */
#define SDIO_CMD_ABORTCMD_Pos                     (14U)
#define SDIO_CMD_ABORTCMD_Msk                     (0x1UL << SDIO_CMD_ABORTCMD_Pos)                  /*!< 0x00004000 */
#define SDIO_CMD_ABORTCMD                         SDIO_CMD_ABORTCMD_Msk                             /*!< desc ABORTCMD */
#define SDIO_CMD_AUTOINIT_Pos                     (15U)
#define SDIO_CMD_AUTOINIT_Msk                     (0x1UL << SDIO_CMD_AUTOINIT_Pos)                  /*!< 0x00008000 */
#define SDIO_CMD_AUTOINIT                         SDIO_CMD_AUTOINIT_Msk                             /*!< desc AUTOINIT */
#define SDIO_CMD_REGSYNC_Pos                      (21U)
#define SDIO_CMD_REGSYNC_Msk                      (0x1UL << SDIO_CMD_REGSYNC_Pos)                   /*!< 0x00200000 */
#define SDIO_CMD_REGSYNC                          SDIO_CMD_REGSYNC_Msk                              /*!< desc REGSYNC */
#define SDIO_CMD_ATACMD_Pos                       (22U)
#define SDIO_CMD_ATACMD_Msk                       (0x1UL << SDIO_CMD_ATACMD_Pos)                    /*!< 0x00400000 */
#define SDIO_CMD_ATACMD                           SDIO_CMD_ATACMD_Msk                               /*!< desc ATACMD */
#define SDIO_CMD_IEN_Pos                          (23U)
#define SDIO_CMD_IEN_Msk                          (0x1UL << SDIO_CMD_IEN_Pos)                       /*!< 0x00800000 */
#define SDIO_CMD_IEN                              SDIO_CMD_IEN_Msk                                  /*!< desc IEN */
#define SDIO_CMD_BOOTEN_Pos                       (24U)
#define SDIO_CMD_BOOTEN_Msk                       (0x1UL << SDIO_CMD_BOOTEN_Pos)                    /*!< 0x01000000 */
#define SDIO_CMD_BOOTEN                           SDIO_CMD_BOOTEN_Msk                               /*!< desc BOOTEN */
#define SDIO_CMD_BOOTACK_Pos                      (25U)
#define SDIO_CMD_BOOTACK_Msk                      (0x1UL << SDIO_CMD_BOOTACK_Pos)                   /*!< 0x02000000 */
#define SDIO_CMD_BOOTACK                          SDIO_CMD_BOOTACK_Msk                              /*!< desc BOOTACK */
#define SDIO_CMD_BOOTDIS_Pos                      (26U)
#define SDIO_CMD_BOOTDIS_Msk                      (0x1UL << SDIO_CMD_BOOTDIS_Pos)                   /*!< 0x04000000 */
#define SDIO_CMD_BOOTDIS                          SDIO_CMD_BOOTDIS_Msk                              /*!< desc BOOTDIS */
#define SDIO_CMD_BOOTMODE_Pos                     (27U)
#define SDIO_CMD_BOOTMODE_Msk                     (0x1UL << SDIO_CMD_BOOTMODE_Pos)                  /*!< 0x08000000 */
#define SDIO_CMD_BOOTMODE                         SDIO_CMD_BOOTMODE_Msk                             /*!< desc BOOTMODE */
#define SDIO_CMD_STARTCMD_Pos                     (31U)
#define SDIO_CMD_STARTCMD_Msk                     (0x1UL << SDIO_CMD_STARTCMD_Pos)                  /*!< 0x80000000 */
#define SDIO_CMD_STARTCMD                         SDIO_CMD_STARTCMD_Msk                             /*!< desc STARTCMD */

/*!< SDIO_RESPCMD */
#define SDIO_RESPCMD_RESPCMD_Pos                  (0U)
#define SDIO_RESPCMD_RESPCMD_Msk                  (0x3FUL << SDIO_RESPCMD_RESPCMD_Pos)              /*!< 0x0000003F */
#define SDIO_RESPCMD_RESPCMD                      SDIO_RESPCMD_RESPCMD_Msk                          /*!< RESPCMD[5:0] bits (desc RESPCMD) */
#define SDIO_RESPCMD_RESPCMD_0                    (0x1UL << SDIO_RESPCMD_RESPCMD_Pos)               /*!< 0x00000001 */
#define SDIO_RESPCMD_RESPCMD_1                    (0x2UL << SDIO_RESPCMD_RESPCMD_Pos)               /*!< 0x00000002 */
#define SDIO_RESPCMD_RESPCMD_2                    (0x4UL << SDIO_RESPCMD_RESPCMD_Pos)               /*!< 0x00000004 */
#define SDIO_RESPCMD_RESPCMD_3                    (0x8UL << SDIO_RESPCMD_RESPCMD_Pos)               /*!< 0x00000008 */
#define SDIO_RESPCMD_RESPCMD_4                    (0x10UL << SDIO_RESPCMD_RESPCMD_Pos)              /*!< 0x00000010 */
#define SDIO_RESPCMD_RESPCMD_5                    (0x20UL << SDIO_RESPCMD_RESPCMD_Pos)              /*!< 0x00000020 */


/*!< SDIO_RESP0 */
#define SDIO_RESP0_CARDSTATUS1_Pos                (0U)
#define SDIO_RESP0_CARDSTATUS1_Msk                (0xFFFFFFFFUL << SDIO_RESP0_CARDSTATUS1_Pos)      /*!< 0xFFFFFFFF */
#define SDIO_RESP0_CARDSTATUS1                    SDIO_RESP0_CARDSTATUS1_Msk                        /*!< CARDSTATUS1[31:0] bits (desc CARDSTATUS1) */

/*!< SDIO_RESP1 */
#define SDIO_RESP1_CARDSTATUS2_Pos                (0U)
#define SDIO_RESP1_CARDSTATUS2_Msk                (0xFFFFFFFFUL << SDIO_RESP1_CARDSTATUS2_Pos)      /*!< 0xFFFFFFFF */
#define SDIO_RESP1_CARDSTATUS2                    SDIO_RESP1_CARDSTATUS2_Msk                        /*!< CARDSTATUS2[31:0] bits (desc CARDSTATUS2) */

/*!< SDIO_RESP2 */
#define SDIO_RESP2_CARDSTATUS3_Pos                (0U)
#define SDIO_RESP2_CARDSTATUS3_Msk                (0xFFFFFFFFUL << SDIO_RESP2_CARDSTATUS3_Pos)      /*!< 0xFFFFFFFF */
#define SDIO_RESP2_CARDSTATUS3                    SDIO_RESP2_CARDSTATUS3_Msk                        /*!< CARDSTATUS3[31:0] bits (desc CARDSTATUS3) */

/*!< SDIO_RESP3 */
#define SDIO_RESP3_CARDSTATUS4_Pos                (0U)
#define SDIO_RESP3_CARDSTATUS4_Msk                (0xFFFFFFFFUL << SDIO_RESP3_CARDSTATUS4_Pos)      /*!< 0xFFFFFFFF */
#define SDIO_RESP3_CARDSTATUS4                    SDIO_RESP3_CARDSTATUS4_Msk                        /*!< CARDSTATUS4[31:0] bits (desc CARDSTATUS4) */

/*!< SDIO_TMOUT */
#define SDIO_TMOUT_RESPTIME_Pos                   (0U)
#define SDIO_TMOUT_RESPTIME_Msk                   (0xFFUL << SDIO_TMOUT_RESPTIME_Pos)               /*!< 0x000000FF */
#define SDIO_TMOUT_RESPTIME                       SDIO_TMOUT_RESPTIME_Msk                           /*!< RESPTIME[7:0] bits (desc RESPTIME) */
#define SDIO_TMOUT_DATATIME_Pos                   (8U)
#define SDIO_TMOUT_DATATIME_Msk                   (0xFFFFFFUL << SDIO_TMOUT_DATATIME_Pos)           /*!< 0xFFFFFF00 */
#define SDIO_TMOUT_DATATIME                       SDIO_TMOUT_DATATIME_Msk                           /*!< DATATIME[31:8] bits (desc DATATIME) */

/*!< SDIO_BLKSIZ */
#define SDIO_BLKSIZ_DBLOCKSIZE_Pos                (0U)
#define SDIO_BLKSIZ_DBLOCKSIZE_Msk                (0xFFFFUL << SDIO_BLKSIZ_DBLOCKSIZE_Pos)          /*!< 0x0000FFFF */
#define SDIO_BLKSIZ_DBLOCKSIZE                    SDIO_BLKSIZ_DBLOCKSIZE_Msk                        /*!< DBLOCKSIZE[15:0] bits (desc DBLOCKSIZE) */

/*!< SDIO_DLEN */
#define SDIO_DLEN_DATALENGTH_Pos                  (0U)
#define SDIO_DLEN_DATALENGTH_Msk                  (0xFFFFFFFFUL << SDIO_DLEN_DATALENGTH_Pos)        /*!< 0xFFFFFFFF */
#define SDIO_DLEN_DATALENGTH                      SDIO_DLEN_DATALENGTH_Msk                          /*!< DATALENGTH[31:0] bits (desc DATALENGTH) */

/*!< SDIO_CTRL */
#define SDIO_CTRL_SDIORST_Pos                     (0U)
#define SDIO_CTRL_SDIORST_Msk                     (0x1UL << SDIO_CTRL_SDIORST_Pos)                  /*!< 0x00000001 */
#define SDIO_CTRL_SDIORST                         SDIO_CTRL_SDIORST_Msk                             /*!< desc SDIORST */
#define SDIO_CTRL_FIFORST_Pos                     (1U)
#define SDIO_CTRL_FIFORST_Msk                     (0x1UL << SDIO_CTRL_FIFORST_Pos)                  /*!< 0x00000002 */
#define SDIO_CTRL_FIFORST                         SDIO_CTRL_FIFORST_Msk                             /*!< desc FIFORST */
#define SDIO_CTRL_INTEN_Pos                       (4U)
#define SDIO_CTRL_INTEN_Msk                       (0x1UL << SDIO_CTRL_INTEN_Pos)                    /*!< 0x00000010 */
#define SDIO_CTRL_INTEN                           SDIO_CTRL_INTEN_Msk                               /*!< desc INTEN */
#define SDIO_CTRL_DMAEN_Pos                       (5U)
#define SDIO_CTRL_DMAEN_Msk                       (0x1UL << SDIO_CTRL_DMAEN_Pos)                    /*!< 0x00000020 */
#define SDIO_CTRL_DMAEN                           SDIO_CTRL_DMAEN_Msk                               /*!< desc DMAEN */
#define SDIO_CTRL_READWAIT_Pos                    (6U)
#define SDIO_CTRL_READWAIT_Msk                    (0x1UL << SDIO_CTRL_READWAIT_Pos)                 /*!< 0x00000040 */
#define SDIO_CTRL_READWAIT                        SDIO_CTRL_READWAIT_Msk                            /*!< desc READWAIT */
#define SDIO_CTRL_AUTOIRQRESP_Pos                 (7U)
#define SDIO_CTRL_AUTOIRQRESP_Msk                 (0x1UL << SDIO_CTRL_AUTOIRQRESP_Pos)              /*!< 0x00000080 */
#define SDIO_CTRL_AUTOIRQRESP                     SDIO_CTRL_AUTOIRQRESP_Msk                         /*!< desc AUTOIRQRESP */
#define SDIO_CTRL_ABORTRD_Pos                     (8U)
#define SDIO_CTRL_ABORTRD_Msk                     (0x1UL << SDIO_CTRL_ABORTRD_Pos)                  /*!< 0x00000100 */
#define SDIO_CTRL_ABORTRD                         SDIO_CTRL_ABORTRD_Msk                             /*!< desc ABORTRD */
#define SDIO_CTRL_CCSDEN_Pos                      (9U)
#define SDIO_CTRL_CCSDEN_Msk                      (0x1UL << SDIO_CTRL_CCSDEN_Pos)                   /*!< 0x00000200 */
#define SDIO_CTRL_CCSDEN                          SDIO_CTRL_CCSDEN_Msk                              /*!< desc CCSDEN */
#define SDIO_CTRL_AUTOSTOPCCSD_Pos                (10U)
#define SDIO_CTRL_AUTOSTOPCCSD_Msk                (0x1UL << SDIO_CTRL_AUTOSTOPCCSD_Pos)             /*!< 0x00000400 */
#define SDIO_CTRL_AUTOSTOPCCSD                    SDIO_CTRL_AUTOSTOPCCSD_Msk                        /*!< desc AUTOSTOPCCSD */
#define SDIO_CTRL_CEATAINTEN_Pos                  (11U)
#define SDIO_CTRL_CEATAINTEN_Msk                  (0x1UL << SDIO_CTRL_CEATAINTEN_Pos)               /*!< 0x00000800 */
#define SDIO_CTRL_CEATAINTEN                      SDIO_CTRL_CEATAINTEN_Msk                          /*!< desc CEATAINTEN */
#define SDIO_CTRL_ODPUEN_Pos                      (24U)
#define SDIO_CTRL_ODPUEN_Msk                      (0x1UL << SDIO_CTRL_ODPUEN_Pos)                   /*!< 0x01000000 */
#define SDIO_CTRL_ODPUEN                          SDIO_CTRL_ODPUEN_Msk                              /*!< desc ODPUEN */

/*!< SDIO_STATUS */
#define SDIO_STATUS_RXWMARK_Pos                   (0U)
#define SDIO_STATUS_RXWMARK_Msk                   (0x1UL << SDIO_STATUS_RXWMARK_Pos)                /*!< 0x00000001 */
#define SDIO_STATUS_RXWMARK                       SDIO_STATUS_RXWMARK_Msk                           /*!< desc RXWMARK */
#define SDIO_STATUS_TXWMARK_Pos                   (1U)
#define SDIO_STATUS_TXWMARK_Msk                   (0x1UL << SDIO_STATUS_TXWMARK_Pos)                /*!< 0x00000002 */
#define SDIO_STATUS_TXWMARK                       SDIO_STATUS_TXWMARK_Msk                           /*!< desc TXWMARK */
#define SDIO_STATUS_FIFOE_Pos                     (2U)
#define SDIO_STATUS_FIFOE_Msk                     (0x1UL << SDIO_STATUS_FIFOE_Pos)                  /*!< 0x00000004 */
#define SDIO_STATUS_FIFOE                         SDIO_STATUS_FIFOE_Msk                             /*!< desc FIFOE */
#define SDIO_STATUS_FIFOF_Pos                     (3U)
#define SDIO_STATUS_FIFOF_Msk                     (0x1UL << SDIO_STATUS_FIFOF_Pos)                  /*!< 0x00000008 */
#define SDIO_STATUS_FIFOF                         SDIO_STATUS_FIFOF_Msk                             /*!< desc FIFOF */
#define SDIO_STATUS_CMDFSM_Pos                    (4U)
#define SDIO_STATUS_CMDFSM_Msk                    (0xFUL << SDIO_STATUS_CMDFSM_Pos)                 /*!< 0x000000F0 */
#define SDIO_STATUS_CMDFSM                        SDIO_STATUS_CMDFSM_Msk                            /*!< CMDFSM[7:4] bits (desc CMDFSM) */
#define SDIO_STATUS_CMDFSM_0                      (0x1UL << SDIO_STATUS_CMDFSM_Pos)                 /*!< 0x00000010 */
#define SDIO_STATUS_CMDFSM_1                      (0x2UL << SDIO_STATUS_CMDFSM_Pos)                 /*!< 0x00000020 */
#define SDIO_STATUS_CMDFSM_2                      (0x4UL << SDIO_STATUS_CMDFSM_Pos)                 /*!< 0x00000040 */
#define SDIO_STATUS_CMDFSM_3                      (0x8UL << SDIO_STATUS_CMDFSM_Pos)                 /*!< 0x00000080 */

#define SDIO_STATUS_CARDPRESENT_Pos               (8U)
#define SDIO_STATUS_CARDPRESENT_Msk               (0x1UL << SDIO_STATUS_CARDPRESENT_Pos)            /*!< 0x00000100 */
#define SDIO_STATUS_CARDPRESENT                   SDIO_STATUS_CARDPRESENT_Msk                       /*!< desc CARDPRESENT */
#define SDIO_STATUS_CARDBSY_Pos                   (9U)
#define SDIO_STATUS_CARDBSY_Msk                   (0x1UL << SDIO_STATUS_CARDBSY_Pos)                /*!< 0x00000200 */
#define SDIO_STATUS_CARDBSY                       SDIO_STATUS_CARDBSY_Msk                           /*!< desc CARDBSY */
#define SDIO_STATUS_FIFOCNT_Pos                   (17U)
#define SDIO_STATUS_FIFOCNT_Msk                   (0x1FFFUL << SDIO_STATUS_FIFOCNT_Pos)             /*!< 0x3FFE0000 */
#define SDIO_STATUS_FIFOCNT                       SDIO_STATUS_FIFOCNT_Msk                           /*!< FIFOCNT[29:17] bits (desc FIFOCNT) */

/*!< SDIO_INTSTS */
#define SDIO_INTSTS_CAD_Pos                       (0U)
#define SDIO_INTSTS_CAD_Msk                       (0x1UL << SDIO_INTSTS_CAD_Pos)                    /*!< 0x00000001 */
#define SDIO_INTSTS_CAD                           SDIO_INTSTS_CAD_Msk                               /*!< desc CAD */
#define SDIO_INTSTS_RE_Pos                        (1U)
#define SDIO_INTSTS_RE_Msk                        (0x1UL << SDIO_INTSTS_RE_Pos)                     /*!< 0x00000002 */
#define SDIO_INTSTS_RE                            SDIO_INTSTS_RE_Msk                                /*!< desc RE */
#define SDIO_INTSTS_CD_Pos                        (2U)
#define SDIO_INTSTS_CD_Msk                        (0x1UL << SDIO_INTSTS_CD_Pos)                     /*!< 0x00000004 */
#define SDIO_INTSTS_CD                            SDIO_INTSTS_CD_Msk                                /*!< desc CD */
#define SDIO_INTSTS_DTO_Pos                       (3U)
#define SDIO_INTSTS_DTO_Msk                       (0x1UL << SDIO_INTSTS_DTO_Pos)                    /*!< 0x00000008 */
#define SDIO_INTSTS_DTO                           SDIO_INTSTS_DTO_Msk                               /*!< desc DTO */
#define SDIO_INTSTS_TXDR_Pos                      (4U)
#define SDIO_INTSTS_TXDR_Msk                      (0x1UL << SDIO_INTSTS_TXDR_Pos)                   /*!< 0x00000010 */
#define SDIO_INTSTS_TXDR                          SDIO_INTSTS_TXDR_Msk                              /*!< desc TXDR */
#define SDIO_INTSTS_RXDR_Pos                      (5U)
#define SDIO_INTSTS_RXDR_Msk                      (0x1UL << SDIO_INTSTS_RXDR_Pos)                   /*!< 0x00000020 */
#define SDIO_INTSTS_RXDR                          SDIO_INTSTS_RXDR_Msk                              /*!< desc RXDR */
#define SDIO_INTSTS_RCRC_Pos                      (6U)
#define SDIO_INTSTS_RCRC_Msk                      (0x1UL << SDIO_INTSTS_RCRC_Pos)                   /*!< 0x00000040 */
#define SDIO_INTSTS_RCRC                          SDIO_INTSTS_RCRC_Msk                              /*!< desc RCRC */
#define SDIO_INTSTS_DCRC_Pos                      (7U)
#define SDIO_INTSTS_DCRC_Msk                      (0x1UL << SDIO_INTSTS_DCRC_Pos)                   /*!< 0x00000080 */
#define SDIO_INTSTS_DCRC                          SDIO_INTSTS_DCRC_Msk                              /*!< desc DCRC */
#define SDIO_INTSTS_RTO_BAR_Pos                   (8U)
#define SDIO_INTSTS_RTO_BAR_Msk                   (0x1UL << SDIO_INTSTS_RTO_BAR_Pos)                /*!< 0x00000100 */
#define SDIO_INTSTS_RTO_BAR                       SDIO_INTSTS_RTO_BAR_Msk                           /*!< desc RTO_BAR */
#define SDIO_INTSTS_DRTO_BDS_Pos                  (9U)
#define SDIO_INTSTS_DRTO_BDS_Msk                  (0x1UL << SDIO_INTSTS_DRTO_BDS_Pos)               /*!< 0x00000200 */
#define SDIO_INTSTS_DRTO_BDS                      SDIO_INTSTS_DRTO_BDS_Msk                          /*!< desc DRTO_BDS */
#define SDIO_INTSTS_HTO_Pos                       (10U)
#define SDIO_INTSTS_HTO_Msk                       (0x1UL << SDIO_INTSTS_HTO_Pos)                    /*!< 0x00000400 */
#define SDIO_INTSTS_HTO                           SDIO_INTSTS_HTO_Msk                               /*!< desc HTO */
#define SDIO_INTSTS_FRUN_Pos                      (11U)
#define SDIO_INTSTS_FRUN_Msk                      (0x1UL << SDIO_INTSTS_FRUN_Pos)                   /*!< 0x00000800 */
#define SDIO_INTSTS_FRUN                          SDIO_INTSTS_FRUN_Msk                              /*!< desc FRUN */
#define SDIO_INTSTS_HLE_Pos                       (12U)
#define SDIO_INTSTS_HLE_Msk                       (0x1UL << SDIO_INTSTS_HLE_Pos)                    /*!< 0x00001000 */
#define SDIO_INTSTS_HLE                           SDIO_INTSTS_HLE_Msk                               /*!< desc HLE */
#define SDIO_INTSTS_SBE_Pos                       (13U)
#define SDIO_INTSTS_SBE_Msk                       (0x1UL << SDIO_INTSTS_SBE_Pos)                    /*!< 0x00002000 */
#define SDIO_INTSTS_SBE                           SDIO_INTSTS_SBE_Msk                               /*!< desc SBE */
#define SDIO_INTSTS_ACD_Pos                       (14U)
#define SDIO_INTSTS_ACD_Msk                       (0x1UL << SDIO_INTSTS_ACD_Pos)                    /*!< 0x00004000 */
#define SDIO_INTSTS_ACD                           SDIO_INTSTS_ACD_Msk                               /*!< desc ACD */
#define SDIO_INTSTS_EBE_Pos                       (15U)
#define SDIO_INTSTS_EBE_Msk                       (0x1UL << SDIO_INTSTS_EBE_Pos)                    /*!< 0x00008000 */
#define SDIO_INTSTS_EBE                           SDIO_INTSTS_EBE_Msk                               /*!< desc EBE */
#define SDIO_INTSTS_SDIOINT_Pos                   (16U)
#define SDIO_INTSTS_SDIOINT_Msk                   (0x1UL << SDIO_INTSTS_SDIOINT_Pos)                /*!< 0x00010000 */
#define SDIO_INTSTS_SDIOINT                       SDIO_INTSTS_SDIOINT_Msk                           /*!< desc SDIOINT */

/*!< SDIO_INTMASK */
#define SDIO_INTMASK_CADIE_Pos                    (0U)
#define SDIO_INTMASK_CADIE_Msk                    (0x1UL << SDIO_INTMASK_CADIE_Pos)                 /*!< 0x00000001 */
#define SDIO_INTMASK_CADIE                        SDIO_INTMASK_CADIE_Msk                            /*!< desc CADIE */
#define SDIO_INTMASK_REIE_Pos                     (1U)
#define SDIO_INTMASK_REIE_Msk                     (0x1UL << SDIO_INTMASK_REIE_Pos)                  /*!< 0x00000002 */
#define SDIO_INTMASK_REIE                         SDIO_INTMASK_REIE_Msk                             /*!< desc REIE */
#define SDIO_INTMASK_CDIE_Pos                     (2U)
#define SDIO_INTMASK_CDIE_Msk                     (0x1UL << SDIO_INTMASK_CDIE_Pos)                  /*!< 0x00000004 */
#define SDIO_INTMASK_CDIE                         SDIO_INTMASK_CDIE_Msk                             /*!< desc CDIE */
#define SDIO_INTMASK_DTOIE_Pos                    (3U)
#define SDIO_INTMASK_DTOIE_Msk                    (0x1UL << SDIO_INTMASK_DTOIE_Pos)                 /*!< 0x00000008 */
#define SDIO_INTMASK_DTOIE                        SDIO_INTMASK_DTOIE_Msk                            /*!< desc DTOIE */
#define SDIO_INTMASK_TXDRIE_Pos                   (4U)
#define SDIO_INTMASK_TXDRIE_Msk                   (0x1UL << SDIO_INTMASK_TXDRIE_Pos)                /*!< 0x00000010 */
#define SDIO_INTMASK_TXDRIE                       SDIO_INTMASK_TXDRIE_Msk                           /*!< desc TXDRIE */
#define SDIO_INTMASK_RXDRIE_Pos                   (5U)
#define SDIO_INTMASK_RXDRIE_Msk                   (0x1UL << SDIO_INTMASK_RXDRIE_Pos)                /*!< 0x00000020 */
#define SDIO_INTMASK_RXDRIE                       SDIO_INTMASK_RXDRIE_Msk                           /*!< desc RXDRIE */
#define SDIO_INTMASK_RCRCIE_Pos                   (6U)
#define SDIO_INTMASK_RCRCIE_Msk                   (0x1UL << SDIO_INTMASK_RCRCIE_Pos)                /*!< 0x00000040 */
#define SDIO_INTMASK_RCRCIE                       SDIO_INTMASK_RCRCIE_Msk                           /*!< desc RCRCIE */
#define SDIO_INTMASK_DCRCIE_Pos                   (7U)
#define SDIO_INTMASK_DCRCIE_Msk                   (0x1UL << SDIO_INTMASK_DCRCIE_Pos)                /*!< 0x00000080 */
#define SDIO_INTMASK_DCRCIE                       SDIO_INTMASK_DCRCIE_Msk                           /*!< desc DCRCIE */
#define SDIO_INTMASK_RTOIE_Pos                    (8U)
#define SDIO_INTMASK_RTOIE_Msk                    (0x1UL << SDIO_INTMASK_RTOIE_Pos)                 /*!< 0x00000100 */
#define SDIO_INTMASK_RTOIE                        SDIO_INTMASK_RTOIE_Msk                            /*!< desc RTOIE */
#define SDIO_INTMASK_DRTOIE_Pos                   (9U)
#define SDIO_INTMASK_DRTOIE_Msk                   (0x1UL << SDIO_INTMASK_DRTOIE_Pos)                /*!< 0x00000200 */
#define SDIO_INTMASK_DRTOIE                       SDIO_INTMASK_DRTOIE_Msk                           /*!< desc DRTOIE */
#define SDIO_INTMASK_HTOIE_Pos                    (10U)
#define SDIO_INTMASK_HTOIE_Msk                    (0x1UL << SDIO_INTMASK_HTOIE_Pos)                 /*!< 0x00000400 */
#define SDIO_INTMASK_HTOIE                        SDIO_INTMASK_HTOIE_Msk                            /*!< desc HTOIE */
#define SDIO_INTMASK_FRUNIE_Pos                   (11U)
#define SDIO_INTMASK_FRUNIE_Msk                   (0x1UL << SDIO_INTMASK_FRUNIE_Pos)                /*!< 0x00000800 */
#define SDIO_INTMASK_FRUNIE                       SDIO_INTMASK_FRUNIE_Msk                           /*!< desc FRUNIE */
#define SDIO_INTMASK_HLEIE_Pos                    (12U)
#define SDIO_INTMASK_HLEIE_Msk                    (0x1UL << SDIO_INTMASK_HLEIE_Pos)                 /*!< 0x00001000 */
#define SDIO_INTMASK_HLEIE                        SDIO_INTMASK_HLEIE_Msk                            /*!< desc HLEIE */
#define SDIO_INTMASK_SBEIE_Pos                    (13U)
#define SDIO_INTMASK_SBEIE_Msk                    (0x1UL << SDIO_INTMASK_SBEIE_Pos)                 /*!< 0x00002000 */
#define SDIO_INTMASK_SBEIE                        SDIO_INTMASK_SBEIE_Msk                            /*!< desc SBEIE */
#define SDIO_INTMASK_ACDIE_Pos                    (14U)
#define SDIO_INTMASK_ACDIE_Msk                    (0x1UL << SDIO_INTMASK_ACDIE_Pos)                 /*!< 0x00004000 */
#define SDIO_INTMASK_ACDIE                        SDIO_INTMASK_ACDIE_Msk                            /*!< desc ACDIE */
#define SDIO_INTMASK_EBEIE_Pos                    (15U)
#define SDIO_INTMASK_EBEIE_Msk                    (0x1UL << SDIO_INTMASK_EBEIE_Pos)                 /*!< 0x00008000 */
#define SDIO_INTMASK_EBEIE                        SDIO_INTMASK_EBEIE_Msk                            /*!< desc EBEIE */
#define SDIO_INTMASK_SDIOINTIE_Pos                (16U)
#define SDIO_INTMASK_SDIOINTIE_Msk                (0x1UL << SDIO_INTMASK_SDIOINTIE_Pos)             /*!< 0x00010000 */
#define SDIO_INTMASK_SDIOINTIE                    SDIO_INTMASK_SDIOINTIE_Msk                        /*!< desc SDIOINTIE */

/*!< SDIO_FIFOTH */
#define SDIO_FIFOTH_TXWMARK_Pos                   (0U)
#define SDIO_FIFOTH_TXWMARK_Msk                   (0xFFFUL << SDIO_FIFOTH_TXWMARK_Pos)              /*!< 0x00000FFF */
#define SDIO_FIFOTH_TXWMARK                       SDIO_FIFOTH_TXWMARK_Msk                           /*!< TXWMARK[11:0] bits (desc TXWMARK) */
#define SDIO_FIFOTH_RXWMARK_Pos                   (16U)
#define SDIO_FIFOTH_RXWMARK_Msk                   (0xFFFUL << SDIO_FIFOTH_RXWMARK_Pos)              /*!< 0x0FFF0000 */
#define SDIO_FIFOTH_RXWMARK                       SDIO_FIFOTH_RXWMARK_Msk                           /*!< RXWMARK[27:16] bits (desc RXWMARK) */

/*!< SDIO_TCBCNT */
#define SDIO_TCBCNT_TCBCNT_Pos                    (0U)
#define SDIO_TCBCNT_TCBCNT_Msk                    (0xFFFFFFFFUL << SDIO_TCBCNT_TCBCNT_Pos)          /*!< 0xFFFFFFFF */
#define SDIO_TCBCNT_TCBCNT                        SDIO_TCBCNT_TCBCNT_Msk                            /*!< TCBCNT[31:0] bits (desc TCBCNT) */

/*!< SDIO_TBBCNT */
#define SDIO_TBBCNT_TBBCNT_Pos                    (0U)
#define SDIO_TBBCNT_TBBCNT_Msk                    (0xFFFFFFFFUL << SDIO_TBBCNT_TBBCNT_Pos)          /*!< 0xFFFFFFFF */
#define SDIO_TBBCNT_TBBCNT                        SDIO_TBBCNT_TBBCNT_Msk                            /*!< TBBCNT[31:0] bits (desc TBBCNT) */

/*!< SDIO_FIFODATA */
#define SDIO_FIFODATA_FIFODATA_Pos                (0U)
#define SDIO_FIFODATA_FIFODATA_Msk                (0xFFFFFFFFUL << SDIO_FIFODATA_FIFODATA_Pos)      /*!< 0xFFFFFFFF */
#define SDIO_FIFODATA_FIFODATA                    SDIO_FIFODATA_FIFODATA_Msk                        /*!< FIFODATA[31:0] bits (desc FIFODATA) */

/*********************  Bits Define For Peripheral SPI  *********************/
/*!< SPI_CR1 */
#define SPI_CR1_CPHA_Pos                          (0U)
#define SPI_CR1_CPHA_Msk                          (0x1UL << SPI_CR1_CPHA_Pos)                       /*!< 0x00000001 */
#define SPI_CR1_CPHA                              SPI_CR1_CPHA_Msk                                  /*!< desc CPHA */
#define SPI_CR1_CPOL_Pos                          (1U)
#define SPI_CR1_CPOL_Msk                          (0x1UL << SPI_CR1_CPOL_Pos)                       /*!< 0x00000002 */
#define SPI_CR1_CPOL                              SPI_CR1_CPOL_Msk                                  /*!< desc CPOL */
#define SPI_CR1_MSTR_Pos                          (2U)
#define SPI_CR1_MSTR_Msk                          (0x1UL << SPI_CR1_MSTR_Pos)                       /*!< 0x00000004 */
#define SPI_CR1_MSTR                              SPI_CR1_MSTR_Msk                                  /*!< desc MSTR */
#define SPI_CR1_BR_Pos                            (3U)
#define SPI_CR1_BR_Msk                            (0x7UL << SPI_CR1_BR_Pos)                         /*!< 0x00000038 */
#define SPI_CR1_BR                                SPI_CR1_BR_Msk                                    /*!< BR[5:3] bits (desc BR) */
#define SPI_CR1_BR_0                              (0x1UL << SPI_CR1_BR_Pos)                         /*!< 0x00000008 */
#define SPI_CR1_BR_1                              (0x2UL << SPI_CR1_BR_Pos)                         /*!< 0x00000010 */
#define SPI_CR1_BR_2                              (0x4UL << SPI_CR1_BR_Pos)                         /*!< 0x00000020 */

#define SPI_CR1_SPE_Pos                           (6U)
#define SPI_CR1_SPE_Msk                           (0x1UL << SPI_CR1_SPE_Pos)                        /*!< 0x00000040 */
#define SPI_CR1_SPE                               SPI_CR1_SPE_Msk                                   /*!< desc SPE */
#define SPI_CR1_LSBFIRST_Pos                      (7U)
#define SPI_CR1_LSBFIRST_Msk                      (0x1UL << SPI_CR1_LSBFIRST_Pos)                   /*!< 0x00000080 */
#define SPI_CR1_LSBFIRST                          SPI_CR1_LSBFIRST_Msk                              /*!< desc LSBFIRST */
#define SPI_CR1_SSI_Pos                           (8U)
#define SPI_CR1_SSI_Msk                           (0x1UL << SPI_CR1_SSI_Pos)                        /*!< 0x00000100 */
#define SPI_CR1_SSI                               SPI_CR1_SSI_Msk                                   /*!< desc SSI */
#define SPI_CR1_SSM_Pos                           (9U)
#define SPI_CR1_SSM_Msk                           (0x1UL << SPI_CR1_SSM_Pos)                        /*!< 0x00000200 */
#define SPI_CR1_SSM                               SPI_CR1_SSM_Msk                                   /*!< desc SSM */
#define SPI_CR1_RXONLY_Pos                        (10U)
#define SPI_CR1_RXONLY_Msk                        (0x1UL << SPI_CR1_RXONLY_Pos)                     /*!< 0x00000400 */
#define SPI_CR1_RXONLY                            SPI_CR1_RXONLY_Msk                                /*!< desc RXONLY */
#define SPI_CR1_DFF_Pos                           (11U)
#define SPI_CR1_DFF_Msk                           (0x1UL << SPI_CR1_DFF_Pos)                        /*!< 0x00000800 */
#define SPI_CR1_DFF                               SPI_CR1_DFF_Msk                                   /*!< desc DFF */
#define SPI_CR1_CRCNEXT_Pos                       (12U)
#define SPI_CR1_CRCNEXT_Msk                       (0x1UL << SPI_CR1_CRCNEXT_Pos)                    /*!< 0x00001000 */
#define SPI_CR1_CRCNEXT                           SPI_CR1_CRCNEXT_Msk                               /*!< desc CRCNEXT */
#define SPI_CR1_CRCEN_Pos                         (13U)
#define SPI_CR1_CRCEN_Msk                         (0x1UL << SPI_CR1_CRCEN_Pos)                      /*!< 0x00002000 */
#define SPI_CR1_CRCEN                             SPI_CR1_CRCEN_Msk                                 /*!< desc CRCEN */
#define SPI_CR1_BIDIOE_Pos                        (14U)
#define SPI_CR1_BIDIOE_Msk                        (0x1UL << SPI_CR1_BIDIOE_Pos)                     /*!< 0x00004000 */
#define SPI_CR1_BIDIOE                            SPI_CR1_BIDIOE_Msk                                /*!< desc BIDIOE */
#define SPI_CR1_BIDIMODE_Pos                      (15U)
#define SPI_CR1_BIDIMODE_Msk                      (0x1UL << SPI_CR1_BIDIMODE_Pos)                   /*!< 0x00008000 */
#define SPI_CR1_BIDIMODE                          SPI_CR1_BIDIMODE_Msk                              /*!< desc BIDIMODE */

/*!< SPI_CR2 */
#define SPI_CR2_RXDMAEN_Pos                       (0U)
#define SPI_CR2_RXDMAEN_Msk                       (0x1UL << SPI_CR2_RXDMAEN_Pos)                    /*!< 0x00000001 */
#define SPI_CR2_RXDMAEN                           SPI_CR2_RXDMAEN_Msk                               /*!< desc RXDMAEN */
#define SPI_CR2_TXDMAEN_Pos                       (1U)
#define SPI_CR2_TXDMAEN_Msk                       (0x1UL << SPI_CR2_TXDMAEN_Pos)                    /*!< 0x00000002 */
#define SPI_CR2_TXDMAEN                           SPI_CR2_TXDMAEN_Msk                               /*!< desc TXDMAEN */
#define SPI_CR2_SSOE_Pos                          (2U)
#define SPI_CR2_SSOE_Msk                          (0x1UL << SPI_CR2_SSOE_Pos)                       /*!< 0x00000004 */
#define SPI_CR2_SSOE                              SPI_CR2_SSOE_Msk                                  /*!< desc SSOE */
#define SPI_CR2_CLRTXFIFO_Pos                     (4U)
#define SPI_CR2_CLRTXFIFO_Msk                     (0x1UL << SPI_CR2_CLRTXFIFO_Pos)                  /*!< 0x00000010 */
#define SPI_CR2_CLRTXFIFO                         SPI_CR2_CLRTXFIFO_Msk                             /*!< desc CLRTXFIFO */
#define SPI_CR2_ERRIE_Pos                         (5U)
#define SPI_CR2_ERRIE_Msk                         (0x1UL << SPI_CR2_ERRIE_Pos)                      /*!< 0x00000020 */
#define SPI_CR2_ERRIE                             SPI_CR2_ERRIE_Msk                                 /*!< desc ERRIE */
#define SPI_CR2_RXNEIE_Pos                        (6U)
#define SPI_CR2_RXNEIE_Msk                        (0x1UL << SPI_CR2_RXNEIE_Pos)                     /*!< 0x00000040 */
#define SPI_CR2_RXNEIE                            SPI_CR2_RXNEIE_Msk                                /*!< desc RXNEIE */
#define SPI_CR2_TXEIE_Pos                         (7U)
#define SPI_CR2_TXEIE_Msk                         (0x1UL << SPI_CR2_TXEIE_Pos)                      /*!< 0x00000080 */
#define SPI_CR2_TXEIE                             SPI_CR2_TXEIE_Msk                                 /*!< desc TXEIE */
#define SPI_CR2_FRXTH_Pos                         (12U)
#define SPI_CR2_FRXTH_Msk                         (0x1UL << SPI_CR2_FRXTH_Pos)                      /*!< 0x00001000 */
#define SPI_CR2_FRXTH                             SPI_CR2_FRXTH_Msk                                 /*!< desc FRXTH */
#define SPI_CR2_LDMA_RX_Pos                       (13U)
#define SPI_CR2_LDMA_RX_Msk                       (0x1UL << SPI_CR2_LDMA_RX_Pos)                     /*!< 0x00002000 */
#define SPI_CR2_LDMA_RX                           SPI_CR2_LDMA_RX_Msk                                /*!< desc LDMARX */
#define SPI_CR2_LDMA_TX_Pos                       (14U)
#define SPI_CR2_LDMA_TX_Msk                       (0x1UL << SPI_CR2_LDMA_TX_Pos)                     /*!< 0x00004000 */
#define SPI_CR2_LDMA_TX                           SPI_CR2_LDMA_TX_Msk                                /*!< desc LDMATX */
#define SPI_CR2_SLVFM_Pos                         (15U)
#define SPI_CR2_SLVFM_Msk                         (0x1UL << SPI_CR2_SLVFM_Pos)                      /*!< 0x00008000 */
#define SPI_CR2_SLVFM                             SPI_CR2_SLVFM_Msk                                 /*!< desc SLVFM */

/*!< SPI_SR */
#define SPI_SR_RXNE_Pos                           (0U)
#define SPI_SR_RXNE_Msk                           (0x1UL << SPI_SR_RXNE_Pos)                        /*!< 0x00000001 */
#define SPI_SR_RXNE                               SPI_SR_RXNE_Msk                                   /*!< desc RXNE */
#define SPI_SR_TXE_Pos                            (1U)
#define SPI_SR_TXE_Msk                            (0x1UL << SPI_SR_TXE_Pos)                         /*!< 0x00000002 */
#define SPI_SR_TXE                                SPI_SR_TXE_Msk                                    /*!< desc TXE */
#define SPI_SR_CHSIDE_Pos                         (2U)
#define SPI_SR_CHSIDE_Msk                         (0x1UL << SPI_SR_CHSIDE_Pos)                      /*!< 0x00000004 */
#define SPI_SR_CHSIDE                             SPI_SR_CHSIDE_Msk                                 /*!< desc CHSIDE */
#define SPI_SR_UDR_Pos                            (3U)
#define SPI_SR_UDR_Msk                            (0x1UL << SPI_SR_UDR_Pos)                         /*!< 0x00000008 */
#define SPI_SR_UDR                                SPI_SR_UDR_Msk                                    /*!< desc UDR */
#define SPI_SR_CRCERR_Pos                         (4U)
#define SPI_SR_CRCERR_Msk                         (0x1UL << SPI_SR_CRCERR_Pos)                      /*!< 0x00000010 */
#define SPI_SR_CRCERR                             SPI_SR_CRCERR_Msk                                 /*!< desc CRCERR */
#define SPI_SR_MODF_Pos                           (5U)
#define SPI_SR_MODF_Msk                           (0x1UL << SPI_SR_MODF_Pos)                        /*!< 0x00000020 */
#define SPI_SR_MODF                               SPI_SR_MODF_Msk                                   /*!< desc MODF */
#define SPI_SR_OVR_Pos                            (6U)
#define SPI_SR_OVR_Msk                            (0x1UL << SPI_SR_OVR_Pos)                         /*!< 0x00000040 */
#define SPI_SR_OVR                                SPI_SR_OVR_Msk                                    /*!< desc OVR */
#define SPI_SR_BSY_Pos                            (7U)
#define SPI_SR_BSY_Msk                            (0x1UL << SPI_SR_BSY_Pos)                         /*!< 0x00000080 */
#define SPI_SR_BSY                                SPI_SR_BSY_Msk                                    /*!< desc BSY */
#define SPI_SR_FRLVL_Pos                          (9U)
#define SPI_SR_FRLVL_Msk                          (0x3UL << SPI_SR_FRLVL_Pos)                       /*!< 0x00000600 */
#define SPI_SR_FRLVL                              SPI_SR_FRLVL_Msk                                  /*!< FRLVL[10:9] bits (desc FRLVL) */
#define SPI_SR_FRLVL_0                            (0x1UL << SPI_SR_FRLVL_Pos)                       /*!< 0x00000200 */
#define SPI_SR_FRLVL_1                            (0x2UL << SPI_SR_FRLVL_Pos)                       /*!< 0x00000400 */

#define SPI_SR_FTLVL_Pos                          (11U)
#define SPI_SR_FTLVL_Msk                          (0x3UL << SPI_SR_FTLVL_Pos)                       /*!< 0x00001800 */
#define SPI_SR_FTLVL                              SPI_SR_FTLVL_Msk                                  /*!< FTLVL[12:11] bits (desc FTLVL) */
#define SPI_SR_FTLVL_0                            (0x1UL << SPI_SR_FTLVL_Pos)                       /*!< 0x00000800 */
#define SPI_SR_FTLVL_1                            (0x2UL << SPI_SR_FTLVL_Pos)                       /*!< 0x00001000 */


/*!< SPI_DR */
#define SPI_DR_DR_Pos                             (0U)
#define SPI_DR_DR_Msk                             (0xFFFFUL << SPI_DR_DR_Pos)                       /*!< 0x0000FFFF */
#define SPI_DR_DR                                 SPI_DR_DR_Msk                                     /*!< DR[15:0] bits (desc DR) */

/*!< SPI_CRCPR */
#define SPI_CRCPR_CRCPOLY_Pos                     (0U)
#define SPI_CRCPR_CRCPOLY_Msk                     (0xFFFFUL << SPI_CRCPR_CRCPOLY_Pos)               /*!< 0x0000FFFF */
#define SPI_CRCPR_CRCPOLY                         SPI_CRCPR_CRCPOLY_Msk                             /*!< CRCPOLY[15:0] bits (desc CRCPOLY) */

/*!< SPI_RXCRCR */
#define SPI_RXCRCR_RXCRC_Pos                      (0U)
#define SPI_RXCRCR_RXCRC_Msk                      (0xFFFFUL << SPI_RXCRCR_RXCRC_Pos)                /*!< 0x0000FFFF */
#define SPI_RXCRCR_RXCRC                          SPI_RXCRCR_RXCRC_Msk                              /*!< RXCRC[15:0] bits (desc RXCRC) */

/*!< SPI_TXCRCR */
#define SPI_TXCRCR_TXCRC_Pos                      (0U)
#define SPI_TXCRCR_TXCRC_Msk                      (0xFFFFUL << SPI_TXCRCR_TXCRC_Pos)                /*!< 0x0000FFFF */
#define SPI_TXCRCR_TXCRC                          SPI_TXCRCR_TXCRC_Msk                              /*!< TXCRC[15:0] bits (desc TXCRC) */

/*!< SPI_I2SCFGR */
#define SPI_I2SCFGR_CHLEN_Pos                     (0U)
#define SPI_I2SCFGR_CHLEN_Msk                     (0x1UL << SPI_I2SCFGR_CHLEN_Pos)                  /*!< 0x00000001 */
#define SPI_I2SCFGR_CHLEN                         SPI_I2SCFGR_CHLEN_Msk                             /*!< desc CHLEN */
#define SPI_I2SCFGR_DATLEN_Pos                    (1U)
#define SPI_I2SCFGR_DATLEN_Msk                    (0x3UL << SPI_I2SCFGR_DATLEN_Pos)                 /*!< 0x00000006 */
#define SPI_I2SCFGR_DATLEN                        SPI_I2SCFGR_DATLEN_Msk                            /*!< DATLEN[2:1] bits (desc DATLEN) */
#define SPI_I2SCFGR_DATLEN_0                      (0x1UL << SPI_I2SCFGR_DATLEN_Pos)                 /*!< 0x00000002 */
#define SPI_I2SCFGR_DATLEN_1                      (0x2UL << SPI_I2SCFGR_DATLEN_Pos)                 /*!< 0x00000004 */

#define SPI_I2SCFGR_CKPOL_Pos                     (3U)
#define SPI_I2SCFGR_CKPOL_Msk                     (0x1UL << SPI_I2SCFGR_CKPOL_Pos)                  /*!< 0x00000008 */
#define SPI_I2SCFGR_CKPOL                         SPI_I2SCFGR_CKPOL_Msk                             /*!< desc CKPOL */
#define SPI_I2SCFGR_I2SSTD_Pos                    (4U)
#define SPI_I2SCFGR_I2SSTD_Msk                    (0x3UL << SPI_I2SCFGR_I2SSTD_Pos)                 /*!< 0x00000030 */
#define SPI_I2SCFGR_I2SSTD                        SPI_I2SCFGR_I2SSTD_Msk                            /*!< I2SSTD[5:4] bits (desc I2SSTD) */
#define SPI_I2SCFGR_I2SSTD_0                      (0x1UL << SPI_I2SCFGR_I2SSTD_Pos)                 /*!< 0x00000010 */
#define SPI_I2SCFGR_I2SSTD_1                      (0x2UL << SPI_I2SCFGR_I2SSTD_Pos)                 /*!< 0x00000020 */

#define SPI_I2SCFGR_PCMSYNC_Pos                   (7U)
#define SPI_I2SCFGR_PCMSYNC_Msk                   (0x1UL << SPI_I2SCFGR_PCMSYNC_Pos)                /*!< 0x00000080 */
#define SPI_I2SCFGR_PCMSYNC                       SPI_I2SCFGR_PCMSYNC_Msk                           /*!< desc PCMSYNC */
#define SPI_I2SCFGR_I2SCFG_Pos                    (8U)
#define SPI_I2SCFGR_I2SCFG_Msk                    (0x3UL << SPI_I2SCFGR_I2SCFG_Pos)                 /*!< 0x00000300 */
#define SPI_I2SCFGR_I2SCFG                        SPI_I2SCFGR_I2SCFG_Msk                            /*!< I2SCFG[9:8] bits (desc I2SCFG) */
#define SPI_I2SCFGR_I2SCFG_0                      (0x1UL << SPI_I2SCFGR_I2SCFG_Pos)                 /*!< 0x00000100 */
#define SPI_I2SCFGR_I2SCFG_1                      (0x2UL << SPI_I2SCFGR_I2SCFG_Pos)                 /*!< 0x00000200 */

#define SPI_I2SCFGR_I2SE_Pos                      (10U)
#define SPI_I2SCFGR_I2SE_Msk                      (0x1UL << SPI_I2SCFGR_I2SE_Pos)                   /*!< 0x00000400 */
#define SPI_I2SCFGR_I2SE                          SPI_I2SCFGR_I2SE_Msk                              /*!< desc I2SE */
#define SPI_I2SCFGR_I2SMOD_Pos                    (11U)
#define SPI_I2SCFGR_I2SMOD_Msk                    (0x1UL << SPI_I2SCFGR_I2SMOD_Pos)                 /*!< 0x00000800 */
#define SPI_I2SCFGR_I2SMOD                        SPI_I2SCFGR_I2SMOD_Msk                            /*!< desc I2SMOD */

/*!< SPI_I2SPR */
#define SPI_I2SPR_I2SDIV_Pos                      (0U)
#define SPI_I2SPR_I2SDIV_Msk                      (0xFFUL << SPI_I2SPR_I2SDIV_Pos)                  /*!< 0x000000FF */
#define SPI_I2SPR_I2SDIV                          SPI_I2SPR_I2SDIV_Msk                              /*!< I2SDIV[7:0] bits (desc I2SDIV) */
#define SPI_I2SPR_ODD_Pos                         (8U)
#define SPI_I2SPR_ODD_Msk                         (0x1UL << SPI_I2SPR_ODD_Pos)                      /*!< 0x00000100 */
#define SPI_I2SPR_ODD                             SPI_I2SPR_ODD_Msk                                 /*!< desc ODD */
#define SPI_I2SPR_MCKOE_Pos                       (9U)
#define SPI_I2SPR_MCKOE_Msk                       (0x1UL << SPI_I2SPR_MCKOE_Pos)                    /*!< 0x00000200 */
#define SPI_I2SPR_MCKOE                           SPI_I2SPR_MCKOE_Msk                               /*!< desc MCKOE */

#define SPI_I2S_SUPPORT       /*!< I2S support */

/*********************  Bits Define For Peripheral SYSCFG  *********************/
/*!< SYSCFG_CFGR1 */
#define SYSCFG_CFGR1_MEM_MODE_Pos                 (0U)
#define SYSCFG_CFGR1_MEM_MODE_Msk                 (0x3UL << SYSCFG_CFGR1_MEM_MODE_Pos)              /*!< 0x00000003 */
#define SYSCFG_CFGR1_MEM_MODE                     SYSCFG_CFGR1_MEM_MODE_Msk                         /*!< MEM_MODE[1:0] bits (desc MEM_MODE) */
#define SYSCFG_CFGR1_MEM_MODE_0                   (0x1UL << SYSCFG_CFGR1_MEM_MODE_Pos)              /*!< 0x00000001 */
#define SYSCFG_CFGR1_MEM_MODE_1                   (0x2UL << SYSCFG_CFGR1_MEM_MODE_Pos)              /*!< 0x00000002 */

#define SYSCFG_CFGR1_I2C_PB5_Pos                  (16U)
#define SYSCFG_CFGR1_I2C_PB5_Msk                  (0x1UL << SYSCFG_CFGR1_I2C_PB5_Pos)               /*!< 0x00010000 */
#define SYSCFG_CFGR1_I2C_PB5                      SYSCFG_CFGR1_I2C_PB5_Msk                          /*!< desc I2C_PB5 */
#define SYSCFG_CFGR1_I2C_PB6_Pos                  (17U)
#define SYSCFG_CFGR1_I2C_PB6_Msk                  (0x1UL << SYSCFG_CFGR1_I2C_PB6_Pos)               /*!< 0x00020000 */
#define SYSCFG_CFGR1_I2C_PB6                      SYSCFG_CFGR1_I2C_PB6_Msk                          /*!< desc I2C_PB6 */
#define SYSCFG_CFGR1_I2C_PB7_Pos                  (18U)
#define SYSCFG_CFGR1_I2C_PB7_Msk                  (0x1UL << SYSCFG_CFGR1_I2C_PB7_Pos)               /*!< 0x00040000 */
#define SYSCFG_CFGR1_I2C_PB7                      SYSCFG_CFGR1_I2C_PB7_Msk                          /*!< desc I2C_PB7 */
#define SYSCFG_CFGR1_I2C_PB8_Pos                  (19U)
#define SYSCFG_CFGR1_I2C_PB8_Msk                  (0x1UL << SYSCFG_CFGR1_I2C_PB8_Pos)               /*!< 0x00080000 */
#define SYSCFG_CFGR1_I2C_PB8                      SYSCFG_CFGR1_I2C_PB8_Msk                          /*!< desc I2C_PB8 */
#define SYSCFG_CFGR1_I2C_PB9_Pos                  (20U)
#define SYSCFG_CFGR1_I2C_PB9_Msk                  (0x1UL << SYSCFG_CFGR1_I2C_PB9_Pos)               /*!< 0x00100000 */
#define SYSCFG_CFGR1_I2C_PB9                      SYSCFG_CFGR1_I2C_PB9_Msk                          /*!< desc I2C_PB9 */
#define SYSCFG_CFGR1_I2C_PB10_Pos                 (21U)
#define SYSCFG_CFGR1_I2C_PB10_Msk                 (0x1UL << SYSCFG_CFGR1_I2C_PB10_Pos)              /*!< 0x00200000 */
#define SYSCFG_CFGR1_I2C_PB10                     SYSCFG_CFGR1_I2C_PB10_Msk                         /*!< desc I2C_PB10 */
#define SYSCFG_CFGR1_I2C_PB11_Pos                 (22U)
#define SYSCFG_CFGR1_I2C_PB11_Msk                 (0x1UL << SYSCFG_CFGR1_I2C_PB11_Pos)              /*!< 0x00400000 */
#define SYSCFG_CFGR1_I2C_PB11                     SYSCFG_CFGR1_I2C_PB11_Msk                         /*!< desc I2C_PB11 */
#define SYSCFG_CFGR1_I2C_PB12_Pos                 (23U)
#define SYSCFG_CFGR1_I2C_PB12_Msk                 (0x1UL << SYSCFG_CFGR1_I2C_PB12_Pos)              /*!< 0x00800000 */
#define SYSCFG_CFGR1_I2C_PB12                     SYSCFG_CFGR1_I2C_PB12_Msk                         /*!< desc I2C_PB12 */

/*!< SYSCFG_CFGR2 */
#define SYSCFG_CFGR2_LOCKUP_LOCK_Pos              (0U)
#define SYSCFG_CFGR2_LOCKUP_LOCK_Msk              (0x1UL << SYSCFG_CFGR2_LOCKUP_LOCK_Pos)           /*!< 0x00000001 */
#define SYSCFG_CFGR2_LOCKUP_LOCK                  SYSCFG_CFGR2_LOCKUP_LOCK_Msk                      /*!< desc LOCKUP_LOCK */
#define SYSCFG_CFGR2_PVD_LOCK_Pos                 (2U)
#define SYSCFG_CFGR2_PVD_LOCK_Msk                 (0x1UL << SYSCFG_CFGR2_PVD_LOCK_Pos)              /*!< 0x00000004 */
#define SYSCFG_CFGR2_PVD_LOCK                     SYSCFG_CFGR2_PVD_LOCK_Msk                         /*!< desc PVD_LOCK */
#define SYSCFG_CFGR2_ADC1_ETRGINJ_REMAP_Pos       (8U)
#define SYSCFG_CFGR2_ADC1_ETRGINJ_REMAP_Msk       (0x1UL << SYSCFG_CFGR2_ADC1_ETRGINJ_REMAP_Pos)    /*!< 0x00000100 */
#define SYSCFG_CFGR2_ADC1_ETRGINJ_REMAP           SYSCFG_CFGR2_ADC1_ETRGINJ_REMAP_Msk               /*!< desc ADC1_ETRGINJ_REMAP */
#define SYSCFG_CFGR2_ADC1_ETRGREG_REMAP_Pos       (9U)
#define SYSCFG_CFGR2_ADC1_ETRGREG_REMAP_Msk       (0x1UL << SYSCFG_CFGR2_ADC1_ETRGREG_REMAP_Pos)    /*!< 0x00000200 */
#define SYSCFG_CFGR2_ADC1_ETRGREG_REMAP           SYSCFG_CFGR2_ADC1_ETRGREG_REMAP_Msk               /*!< desc ADC1_ETRGREG_REMAP */
#define SYSCFG_CFGR2_ADC2_ETRGINJ_REMAP_Pos       (10U)
#define SYSCFG_CFGR2_ADC2_ETRGINJ_REMAP_Msk       (0x1UL << SYSCFG_CFGR2_ADC2_ETRGINJ_REMAP_Pos)    /*!< 0x00000400 */
#define SYSCFG_CFGR2_ADC2_ETRGINJ_REMAP           SYSCFG_CFGR2_ADC2_ETRGINJ_REMAP_Msk               /*!< desc ADC2_ETRGINJ_REMAP */
#define SYSCFG_CFGR2_ADC2_ETRGREG_REMAP_Pos       (11U)
#define SYSCFG_CFGR2_ADC2_ETRGREG_REMAP_Msk       (0x1UL << SYSCFG_CFGR2_ADC2_ETRGREG_REMAP_Pos)    /*!< 0x00000800 */
#define SYSCFG_CFGR2_ADC2_ETRGREG_REMAP           SYSCFG_CFGR2_ADC2_ETRGREG_REMAP_Msk               /*!< desc ADC2_ETRGREG_REMAP */

/*!< SYSCFG_CFGR3 */
#define SYSCFG_CFGR3_DMA1_MAP_Pos                 (0U)
#define SYSCFG_CFGR3_DMA1_MAP_Msk                 (0x7FUL << SYSCFG_CFGR3_DMA1_MAP_Pos)             /*!< 0x0000007F */
#define SYSCFG_CFGR3_DMA1_MAP                     SYSCFG_CFGR3_DMA1_MAP_Msk                         /*!< DMA1_MAP[6:0] bits (desc DMA1_MAP) */
#define SYSCFG_CFGR3_DMA1_MAP_0                   (0x1UL << SYSCFG_CFGR3_DMA1_MAP_Pos)              /*!< 0x00000001 */
#define SYSCFG_CFGR3_DMA1_MAP_1                   (0x2UL << SYSCFG_CFGR3_DMA1_MAP_Pos)              /*!< 0x00000002 */
#define SYSCFG_CFGR3_DMA1_MAP_2                   (0x4UL << SYSCFG_CFGR3_DMA1_MAP_Pos)              /*!< 0x00000004 */
#define SYSCFG_CFGR3_DMA1_MAP_3                   (0x8UL << SYSCFG_CFGR3_DMA1_MAP_Pos)              /*!< 0x00000008 */
#define SYSCFG_CFGR3_DMA1_MAP_4                   (0x10UL << SYSCFG_CFGR3_DMA1_MAP_Pos)             /*!< 0x00000010 */
#define SYSCFG_CFGR3_DMA1_MAP_5                   (0x20UL << SYSCFG_CFGR3_DMA1_MAP_Pos)             /*!< 0x00000020 */
#define SYSCFG_CFGR3_DMA1_MAP_6                   (0x40UL << SYSCFG_CFGR3_DMA1_MAP_Pos)             /*!< 0x00000040 */

// #define SYSCFG_CFGR3_DMA1_ACKLVL_Pos              (7U)
// #define SYSCFG_CFGR3_DMA1_ACKLVL_Msk              (0x1UL << SYSCFG_CFGR3_DMA1_ACKLVL_Pos)           /*!< 0x00000080 */
// #define SYSCFG_CFGR3_DMA1_ACKLVL                  SYSCFG_CFGR3_DMA1_ACKLVL_Msk                      /*!< desc DMA1_ACKLVL */

#define SYSCFG_CFGR3_DMA2_MAP_Pos                 (8U)
#define SYSCFG_CFGR3_DMA2_MAP_Msk                 (0x7FUL << SYSCFG_CFGR3_DMA2_MAP_Pos)             /*!< 0x00007F00 */
#define SYSCFG_CFGR3_DMA2_MAP                     SYSCFG_CFGR3_DMA2_MAP_Msk                         /*!< DMA2_MAP[14:8] bits (desc DMA2_MAP) */
#define SYSCFG_CFGR3_DMA2_MAP_0                   (0x1UL << SYSCFG_CFGR3_DMA2_MAP_Pos)              /*!< 0x00000100 */
#define SYSCFG_CFGR3_DMA2_MAP_1                   (0x2UL << SYSCFG_CFGR3_DMA2_MAP_Pos)              /*!< 0x00000200 */
#define SYSCFG_CFGR3_DMA2_MAP_2                   (0x4UL << SYSCFG_CFGR3_DMA2_MAP_Pos)              /*!< 0x00000400 */
#define SYSCFG_CFGR3_DMA2_MAP_3                   (0x8UL << SYSCFG_CFGR3_DMA2_MAP_Pos)              /*!< 0x00000800 */
#define SYSCFG_CFGR3_DMA2_MAP_4                   (0x10UL << SYSCFG_CFGR3_DMA2_MAP_Pos)             /*!< 0x00001000 */
#define SYSCFG_CFGR3_DMA2_MAP_5                   (0x20UL << SYSCFG_CFGR3_DMA2_MAP_Pos)             /*!< 0x00002000 */
#define SYSCFG_CFGR3_DMA2_MAP_6                   (0x40UL << SYSCFG_CFGR3_DMA2_MAP_Pos)             /*!< 0x00004000 */

// #define SYSCFG_CFGR3_DMA2_ACKLVL_Pos              (15U)
// #define SYSCFG_CFGR3_DMA2_ACKLVL_Msk              (0x1UL << SYSCFG_CFGR3_DMA2_ACKLVL_Pos)           /*!< 0x00008000 */
// #define SYSCFG_CFGR3_DMA2_ACKLVL                  SYSCFG_CFGR3_DMA2_ACKLVL_Msk                      /*!< desc DMA2_ACKLVL */

#define SYSCFG_CFGR3_DMA3_MAP_Pos                 (16U)
#define SYSCFG_CFGR3_DMA3_MAP_Msk                 (0x7FUL << SYSCFG_CFGR3_DMA3_MAP_Pos)             /*!< 0x007F0000 */
#define SYSCFG_CFGR3_DMA3_MAP                     SYSCFG_CFGR3_DMA3_MAP_Msk                         /*!< DMA3_MAP[22:16] bits (desc DMA3_MAP) */
#define SYSCFG_CFGR3_DMA3_MAP_0                   (0x1UL << SYSCFG_CFGR3_DMA3_MAP_Pos)              /*!< 0x00010000 */
#define SYSCFG_CFGR3_DMA3_MAP_1                   (0x2UL << SYSCFG_CFGR3_DMA3_MAP_Pos)              /*!< 0x00020000 */
#define SYSCFG_CFGR3_DMA3_MAP_2                   (0x4UL << SYSCFG_CFGR3_DMA3_MAP_Pos)              /*!< 0x00040000 */
#define SYSCFG_CFGR3_DMA3_MAP_3                   (0x8UL << SYSCFG_CFGR3_DMA3_MAP_Pos)              /*!< 0x00080000 */
#define SYSCFG_CFGR3_DMA3_MAP_4                   (0x10UL << SYSCFG_CFGR3_DMA3_MAP_Pos)             /*!< 0x00100000 */
#define SYSCFG_CFGR3_DMA3_MAP_5                   (0x20UL << SYSCFG_CFGR3_DMA3_MAP_Pos)             /*!< 0x00200000 */
#define SYSCFG_CFGR3_DMA3_MAP_6                   (0x40UL << SYSCFG_CFGR3_DMA3_MAP_Pos)             /*!< 0x00400000 */

// #define SYSCFG_CFGR3_DMA3_ACKLVL_Pos              (23U)
// #define SYSCFG_CFGR3_DMA3_ACKLVL_Msk              (0x1UL << SYSCFG_CFGR3_DMA3_ACKLVL_Pos)           /*!< 0x00800000 */
// #define SYSCFG_CFGR3_DMA3_ACKLVL                  SYSCFG_CFGR3_DMA3_ACKLVL_Msk                      /*!< desc DMA3_ACKLVL */

#define SYSCFG_CFGR3_DMA4_MAP_Pos                 (24U)
#define SYSCFG_CFGR3_DMA4_MAP_Msk                 (0x7FUL << SYSCFG_CFGR3_DMA4_MAP_Pos)             /*!< 0x7F000000 */
#define SYSCFG_CFGR3_DMA4_MAP                     SYSCFG_CFGR3_DMA4_MAP_Msk                         /*!< DMA4_MAP[30:24] bits (desc DMA4_MAP) */
#define SYSCFG_CFGR3_DMA4_MAP_0                   (0x1UL << SYSCFG_CFGR3_DMA4_MAP_Pos)              /*!< 0x01000000 */
#define SYSCFG_CFGR3_DMA4_MAP_1                   (0x2UL << SYSCFG_CFGR3_DMA4_MAP_Pos)              /*!< 0x02000000 */
#define SYSCFG_CFGR3_DMA4_MAP_2                   (0x4UL << SYSCFG_CFGR3_DMA4_MAP_Pos)              /*!< 0x04000000 */
#define SYSCFG_CFGR3_DMA4_MAP_3                   (0x8UL << SYSCFG_CFGR3_DMA4_MAP_Pos)              /*!< 0x08000000 */
#define SYSCFG_CFGR3_DMA4_MAP_4                   (0x10UL << SYSCFG_CFGR3_DMA4_MAP_Pos)             /*!< 0x10000000 */
#define SYSCFG_CFGR3_DMA4_MAP_5                   (0x20UL << SYSCFG_CFGR3_DMA4_MAP_Pos)             /*!< 0x20000000 */
#define SYSCFG_CFGR3_DMA4_MAP_6                   (0x40UL << SYSCFG_CFGR3_DMA4_MAP_Pos)             /*!< 0x40000000 */

// #define SYSCFG_CFGR3_DMA4_ACKLVL_Pos              (31U)
// #define SYSCFG_CFGR3_DMA4_ACKLVL_Msk              (0x1UL << SYSCFG_CFGR3_DMA4_ACKLVL_Pos)           /*!< 0x80000000 */
// #define SYSCFG_CFGR3_DMA4_ACKLVL                  SYSCFG_CFGR3_DMA4_ACKLVL_Msk                      /*!< desc DMA4_ACKLVL */

/*!< SYSCFG_CFGR4 */
#define SYSCFG_CFGR4_DMA5_MAP_Pos                 (0U)
#define SYSCFG_CFGR4_DMA5_MAP_Msk                 (0x7FUL << SYSCFG_CFGR4_DMA5_MAP_Pos)             /*!< 0x0000007F */
#define SYSCFG_CFGR4_DMA5_MAP                     SYSCFG_CFGR4_DMA5_MAP_Msk                         /*!< DMA5_MAP[6:0] bits (desc DMA5_MAP) */
#define SYSCFG_CFGR4_DMA5_MAP_0                   (0x1UL << SYSCFG_CFGR4_DMA5_MAP_Pos)              /*!< 0x00000001 */
#define SYSCFG_CFGR4_DMA5_MAP_1                   (0x2UL << SYSCFG_CFGR4_DMA5_MAP_Pos)              /*!< 0x00000002 */
#define SYSCFG_CFGR4_DMA5_MAP_2                   (0x4UL << SYSCFG_CFGR4_DMA5_MAP_Pos)              /*!< 0x00000004 */
#define SYSCFG_CFGR4_DMA5_MAP_3                   (0x8UL << SYSCFG_CFGR4_DMA5_MAP_Pos)              /*!< 0x00000008 */
#define SYSCFG_CFGR4_DMA5_MAP_4                   (0x10UL << SYSCFG_CFGR4_DMA5_MAP_Pos)             /*!< 0x00000010 */
#define SYSCFG_CFGR4_DMA5_MAP_5                   (0x20UL << SYSCFG_CFGR4_DMA5_MAP_Pos)             /*!< 0x00000020 */
#define SYSCFG_CFGR4_DMA5_MAP_6                   (0x40UL << SYSCFG_CFGR4_DMA5_MAP_Pos)             /*!< 0x00000040 */

#define SYSCFG_CFGR4_DMA5_ACKLVL_Pos              (7U)
#define SYSCFG_CFGR4_DMA5_ACKLVL_Msk              (0x1UL << SYSCFG_CFGR4_DMA5_ACKLVL_Pos)           /*!< 0x00000080 */
#define SYSCFG_CFGR4_DMA5_ACKLVL                  SYSCFG_CFGR4_DMA5_ACKLVL_Msk                      /*!< desc DMA5_ACKLVL */
#define SYSCFG_CFGR4_DMA6_MAP_Pos                 (8U)
#define SYSCFG_CFGR4_DMA6_MAP_Msk                 (0x7FUL << SYSCFG_CFGR4_DMA6_MAP_Pos)             /*!< 0x00007F00 */
#define SYSCFG_CFGR4_DMA6_MAP                     SYSCFG_CFGR4_DMA6_MAP_Msk                         /*!< DMA6_MAP[14:8] bits (desc DMA6_MAP) */
#define SYSCFG_CFGR4_DMA6_MAP_0                   (0x1UL << SYSCFG_CFGR4_DMA6_MAP_Pos)              /*!< 0x00000100 */
#define SYSCFG_CFGR4_DMA6_MAP_1                   (0x2UL << SYSCFG_CFGR4_DMA6_MAP_Pos)              /*!< 0x00000200 */
#define SYSCFG_CFGR4_DMA6_MAP_2                   (0x4UL << SYSCFG_CFGR4_DMA6_MAP_Pos)              /*!< 0x00000400 */
#define SYSCFG_CFGR4_DMA6_MAP_3                   (0x8UL << SYSCFG_CFGR4_DMA6_MAP_Pos)              /*!< 0x00000800 */
#define SYSCFG_CFGR4_DMA6_MAP_4                   (0x10UL << SYSCFG_CFGR4_DMA6_MAP_Pos)             /*!< 0x00001000 */
#define SYSCFG_CFGR4_DMA6_MAP_5                   (0x20UL << SYSCFG_CFGR4_DMA6_MAP_Pos)             /*!< 0x00002000 */
#define SYSCFG_CFGR4_DMA6_MAP_6                   (0x40UL << SYSCFG_CFGR4_DMA6_MAP_Pos)             /*!< 0x00004000 */

#define SYSCFG_CFGR4_DMA6_ACKLVL_Pos              (15U)
#define SYSCFG_CFGR4_DMA6_ACKLVL_Msk              (0x1UL << SYSCFG_CFGR4_DMA6_ACKLVL_Pos)           /*!< 0x00008000 */
#define SYSCFG_CFGR4_DMA6_ACKLVL                  SYSCFG_CFGR4_DMA6_ACKLVL_Msk                      /*!< desc DMA6_ACKLVL */
#define SYSCFG_CFGR4_DMA7_MAP_Pos                 (16U)
#define SYSCFG_CFGR4_DMA7_MAP_Msk                 (0x7FUL << SYSCFG_CFGR4_DMA7_MAP_Pos)             /*!< 0x007F0000 */
#define SYSCFG_CFGR4_DMA7_MAP                     SYSCFG_CFGR4_DMA7_MAP_Msk                         /*!< DMA7_MAP[22:16] bits (desc DMA7_MAP) */
#define SYSCFG_CFGR4_DMA7_MAP_0                   (0x1UL << SYSCFG_CFGR4_DMA7_MAP_Pos)              /*!< 0x00010000 */
#define SYSCFG_CFGR4_DMA7_MAP_1                   (0x2UL << SYSCFG_CFGR4_DMA7_MAP_Pos)              /*!< 0x00020000 */
#define SYSCFG_CFGR4_DMA7_MAP_2                   (0x4UL << SYSCFG_CFGR4_DMA7_MAP_Pos)              /*!< 0x00040000 */
#define SYSCFG_CFGR4_DMA7_MAP_3                   (0x8UL << SYSCFG_CFGR4_DMA7_MAP_Pos)              /*!< 0x00080000 */
#define SYSCFG_CFGR4_DMA7_MAP_4                   (0x10UL << SYSCFG_CFGR4_DMA7_MAP_Pos)             /*!< 0x00100000 */
#define SYSCFG_CFGR4_DMA7_MAP_5                   (0x20UL << SYSCFG_CFGR4_DMA7_MAP_Pos)             /*!< 0x00200000 */
#define SYSCFG_CFGR4_DMA7_MAP_6                   (0x40UL << SYSCFG_CFGR4_DMA7_MAP_Pos)             /*!< 0x00400000 */

#define SYSCFG_CFGR4_DMA7_ACKLVL_Pos              (23U)
#define SYSCFG_CFGR4_DMA7_ACKLVL_Msk              (0x1UL << SYSCFG_CFGR4_DMA7_ACKLVL_Pos)           /*!< 0x00800000 */
#define SYSCFG_CFGR4_DMA7_ACKLVL                  SYSCFG_CFGR4_DMA7_ACKLVL_Msk                      /*!< desc DMA7_ACKLVL */
#define SYSCFG_CFGR4_DMA8_MAP_Pos                 (24U)
#define SYSCFG_CFGR4_DMA8_MAP_Msk                 (0x7FUL << SYSCFG_CFGR4_DMA8_MAP_Pos)             /*!< 0x7F000000 */
#define SYSCFG_CFGR4_DMA8_MAP                     SYSCFG_CFGR4_DMA8_MAP_Msk                         /*!< DMA8_MAP[30:24] bits (desc DMA8_MAP) */
#define SYSCFG_CFGR4_DMA8_MAP_0                   (0x1UL << SYSCFG_CFGR4_DMA8_MAP_Pos)              /*!< 0x01000000 */
#define SYSCFG_CFGR4_DMA8_MAP_1                   (0x2UL << SYSCFG_CFGR4_DMA8_MAP_Pos)              /*!< 0x02000000 */
#define SYSCFG_CFGR4_DMA8_MAP_2                   (0x4UL << SYSCFG_CFGR4_DMA8_MAP_Pos)              /*!< 0x04000000 */
#define SYSCFG_CFGR4_DMA8_MAP_3                   (0x8UL << SYSCFG_CFGR4_DMA8_MAP_Pos)              /*!< 0x08000000 */
#define SYSCFG_CFGR4_DMA8_MAP_4                   (0x10UL << SYSCFG_CFGR4_DMA8_MAP_Pos)             /*!< 0x10000000 */
#define SYSCFG_CFGR4_DMA8_MAP_5                   (0x20UL << SYSCFG_CFGR4_DMA8_MAP_Pos)             /*!< 0x20000000 */
#define SYSCFG_CFGR4_DMA8_MAP_6                   (0x40UL << SYSCFG_CFGR4_DMA8_MAP_Pos)             /*!< 0x40000000 */

#define SYSCFG_CFGR4_DMA8_ACKLVL_Pos              (31U)
#define SYSCFG_CFGR4_DMA8_ACKLVL_Msk              (0x1UL << SYSCFG_CFGR4_DMA8_ACKLVL_Pos)           /*!< 0x80000000 */
#define SYSCFG_CFGR4_DMA8_ACKLVL                  SYSCFG_CFGR4_DMA8_ACKLVL_Msk                      /*!< desc DMA8_ACKLVL */

/*!< SYSCFG_CFGR5 */
#define SYSCFG_CFGR5_DMA9_MAP_Pos                 (0U)
#define SYSCFG_CFGR5_DMA9_MAP_Msk                 (0x7FUL << SYSCFG_CFGR5_DMA9_MAP_Pos)             /*!< 0x0000007F */
#define SYSCFG_CFGR5_DMA9_MAP                     SYSCFG_CFGR5_DMA9_MAP_Msk                         /*!< DMA9_MAP[6:0] bits (desc DMA9_MAP) */
#define SYSCFG_CFGR5_DMA9_MAP_0                   (0x1UL << SYSCFG_CFGR5_DMA9_MAP_Pos)              /*!< 0x00000001 */
#define SYSCFG_CFGR5_DMA9_MAP_1                   (0x2UL << SYSCFG_CFGR5_DMA9_MAP_Pos)              /*!< 0x00000002 */
#define SYSCFG_CFGR5_DMA9_MAP_2                   (0x4UL << SYSCFG_CFGR5_DMA9_MAP_Pos)              /*!< 0x00000004 */
#define SYSCFG_CFGR5_DMA9_MAP_3                   (0x8UL << SYSCFG_CFGR5_DMA9_MAP_Pos)              /*!< 0x00000008 */
#define SYSCFG_CFGR5_DMA9_MAP_4                   (0x10UL << SYSCFG_CFGR5_DMA9_MAP_Pos)             /*!< 0x00000010 */
#define SYSCFG_CFGR5_DMA9_MAP_5                   (0x20UL << SYSCFG_CFGR5_DMA9_MAP_Pos)             /*!< 0x00000020 */
#define SYSCFG_CFGR5_DMA9_MAP_6                   (0x40UL << SYSCFG_CFGR5_DMA9_MAP_Pos)             /*!< 0x00000040 */

#define SYSCFG_CFGR5_DMA9_ACKLVL_Pos              (7U)
#define SYSCFG_CFGR5_DMA9_ACKLVL_Msk              (0x1UL << SYSCFG_CFGR5_DMA9_ACKLVL_Pos)           /*!< 0x00000080 */
#define SYSCFG_CFGR5_DMA9_ACKLVL                  SYSCFG_CFGR5_DMA9_ACKLVL_Msk                      /*!< desc DMA9_ACKLVL */
#define SYSCFG_CFGR5_DMA10_MAP_Pos                (8U)
#define SYSCFG_CFGR5_DMA10_MAP_Msk                (0x7FUL << SYSCFG_CFGR5_DMA10_MAP_Pos)            /*!< 0x00007F00 */
#define SYSCFG_CFGR5_DMA10_MAP                    SYSCFG_CFGR5_DMA10_MAP_Msk                        /*!< DMA10_MAP[14:8] bits (desc DMA10_MAP) */
#define SYSCFG_CFGR5_DMA10_MAP_0                  (0x1UL << SYSCFG_CFGR5_DMA10_MAP_Pos)             /*!< 0x00000100 */
#define SYSCFG_CFGR5_DMA10_MAP_1                  (0x2UL << SYSCFG_CFGR5_DMA10_MAP_Pos)             /*!< 0x00000200 */
#define SYSCFG_CFGR5_DMA10_MAP_2                  (0x4UL << SYSCFG_CFGR5_DMA10_MAP_Pos)             /*!< 0x00000400 */
#define SYSCFG_CFGR5_DMA10_MAP_3                  (0x8UL << SYSCFG_CFGR5_DMA10_MAP_Pos)             /*!< 0x00000800 */
#define SYSCFG_CFGR5_DMA10_MAP_4                  (0x10UL << SYSCFG_CFGR5_DMA10_MAP_Pos)            /*!< 0x00001000 */
#define SYSCFG_CFGR5_DMA10_MAP_5                  (0x20UL << SYSCFG_CFGR5_DMA10_MAP_Pos)            /*!< 0x00002000 */
#define SYSCFG_CFGR5_DMA10_MAP_6                  (0x40UL << SYSCFG_CFGR5_DMA10_MAP_Pos)            /*!< 0x00004000 */

#define SYSCFG_CFGR5_DMA10_ACKLVL_Pos             (15U)
#define SYSCFG_CFGR5_DMA10_ACKLVL_Msk             (0x1UL << SYSCFG_CFGR5_DMA10_ACKLVL_Pos)          /*!< 0x00008000 */
#define SYSCFG_CFGR5_DMA10_ACKLVL                 SYSCFG_CFGR5_DMA10_ACKLVL_Msk                     /*!< desc DMA10_ACKLVL */
#define SYSCFG_CFGR5_DMA11_MAP_Pos                (16U)
#define SYSCFG_CFGR5_DMA11_MAP_Msk                (0x7FUL << SYSCFG_CFGR5_DMA11_MAP_Pos)            /*!< 0x007F0000 */
#define SYSCFG_CFGR5_DMA11_MAP                    SYSCFG_CFGR5_DMA11_MAP_Msk                        /*!< DMA11_MAP[22:16] bits (desc DMA11_MAP) */
#define SYSCFG_CFGR5_DMA11_MAP_0                  (0x1UL << SYSCFG_CFGR5_DMA11_MAP_Pos)             /*!< 0x00010000 */
#define SYSCFG_CFGR5_DMA11_MAP_1                  (0x2UL << SYSCFG_CFGR5_DMA11_MAP_Pos)             /*!< 0x00020000 */
#define SYSCFG_CFGR5_DMA11_MAP_2                  (0x4UL << SYSCFG_CFGR5_DMA11_MAP_Pos)             /*!< 0x00040000 */
#define SYSCFG_CFGR5_DMA11_MAP_3                  (0x8UL << SYSCFG_CFGR5_DMA11_MAP_Pos)             /*!< 0x00080000 */
#define SYSCFG_CFGR5_DMA11_MAP_4                  (0x10UL << SYSCFG_CFGR5_DMA11_MAP_Pos)            /*!< 0x00100000 */
#define SYSCFG_CFGR5_DMA11_MAP_5                  (0x20UL << SYSCFG_CFGR5_DMA11_MAP_Pos)            /*!< 0x00200000 */
#define SYSCFG_CFGR5_DMA11_MAP_6                  (0x40UL << SYSCFG_CFGR5_DMA11_MAP_Pos)            /*!< 0x00400000 */

#define SYSCFG_CFGR5_DMA11_ACKLVL_Pos             (23U)
#define SYSCFG_CFGR5_DMA11_ACKLVL_Msk             (0x1UL << SYSCFG_CFGR5_DMA11_ACKLVL_Pos)          /*!< 0x00800000 */
#define SYSCFG_CFGR5_DMA11_ACKLVL                 SYSCFG_CFGR5_DMA11_ACKLVL_Msk                     /*!< desc DMA11_ACKLVL */
#define SYSCFG_CFGR5_DMA12_MAP_Pos                (24U)
#define SYSCFG_CFGR5_DMA12_MAP_Msk                (0x7FUL << SYSCFG_CFGR5_DMA12_MAP_Pos)            /*!< 0x7F000000 */
#define SYSCFG_CFGR5_DMA12_MAP                    SYSCFG_CFGR5_DMA12_MAP_Msk                        /*!< DMA12_MAP[30:24] bits (desc DMA12_MAP) */
#define SYSCFG_CFGR5_DMA12_MAP_0                  (0x1UL << SYSCFG_CFGR5_DMA12_MAP_Pos)             /*!< 0x01000000 */
#define SYSCFG_CFGR5_DMA12_MAP_1                  (0x2UL << SYSCFG_CFGR5_DMA12_MAP_Pos)             /*!< 0x02000000 */
#define SYSCFG_CFGR5_DMA12_MAP_2                  (0x4UL << SYSCFG_CFGR5_DMA12_MAP_Pos)             /*!< 0x04000000 */
#define SYSCFG_CFGR5_DMA12_MAP_3                  (0x8UL << SYSCFG_CFGR5_DMA12_MAP_Pos)             /*!< 0x08000000 */
#define SYSCFG_CFGR5_DMA12_MAP_4                  (0x10UL << SYSCFG_CFGR5_DMA12_MAP_Pos)            /*!< 0x10000000 */
#define SYSCFG_CFGR5_DMA12_MAP_5                  (0x20UL << SYSCFG_CFGR5_DMA12_MAP_Pos)            /*!< 0x20000000 */
#define SYSCFG_CFGR5_DMA12_MAP_6                  (0x40UL << SYSCFG_CFGR5_DMA12_MAP_Pos)            /*!< 0x40000000 */

#define SYSCFG_CFGR5_DMA12_ACKLVL_Pos             (31U)
#define SYSCFG_CFGR5_DMA12_ACKLVL_Msk             (0x1UL << SYSCFG_CFGR5_DMA12_ACKLVL_Pos)          /*!< 0x80000000 */
#define SYSCFG_CFGR5_DMA12_ACKLVL                 SYSCFG_CFGR5_DMA12_ACKLVL_Msk                     /*!< desc DMA12_ACKLVL */

/*!< SYSCFG_EXTICR1 */
#define SYSCFG_EXTICR1_EXTI0_Pos                  (0U)
#define SYSCFG_EXTICR1_EXTI0_Msk                  (0xFUL << SYSCFG_EXTICR1_EXTI0_Pos)               /*!< 0x0000000F */
#define SYSCFG_EXTICR1_EXTI0                      SYSCFG_EXTICR1_EXTI0_Msk                          /*!< EXTI0[3:0] bits (desc EXTI0) */
#define SYSCFG_EXTICR1_EXTI0_0                    (0x1UL << SYSCFG_EXTICR1_EXTI0_Pos)               /*!< 0x00000001 */
#define SYSCFG_EXTICR1_EXTI0_1                    (0x2UL << SYSCFG_EXTICR1_EXTI0_Pos)               /*!< 0x00000002 */
#define SYSCFG_EXTICR1_EXTI0_2                    (0x4UL << SYSCFG_EXTICR1_EXTI0_Pos)               /*!< 0x00000004 */
#define SYSCFG_EXTICR1_EXTI0_3                    (0x8UL << SYSCFG_EXTICR1_EXTI0_Pos)               /*!< 0x00000008 */

#define SYSCFG_EXTICR1_EXTI1_Pos                  (4U)
#define SYSCFG_EXTICR1_EXTI1_Msk                  (0xFUL << SYSCFG_EXTICR1_EXTI1_Pos)               /*!< 0x000000F0 */
#define SYSCFG_EXTICR1_EXTI1                      SYSCFG_EXTICR1_EXTI1_Msk                          /*!< EXTI1[7:4] bits (desc EXTI1) */
#define SYSCFG_EXTICR1_EXTI1_0                    (0x1UL << SYSCFG_EXTICR1_EXTI1_Pos)               /*!< 0x00000010 */
#define SYSCFG_EXTICR1_EXTI1_1                    (0x2UL << SYSCFG_EXTICR1_EXTI1_Pos)               /*!< 0x00000020 */
#define SYSCFG_EXTICR1_EXTI1_2                    (0x4UL << SYSCFG_EXTICR1_EXTI1_Pos)               /*!< 0x00000040 */
#define SYSCFG_EXTICR1_EXTI1_3                    (0x8UL << SYSCFG_EXTICR1_EXTI1_Pos)               /*!< 0x00000080 */

#define SYSCFG_EXTICR1_EXTI2_Pos                  (8U)
#define SYSCFG_EXTICR1_EXTI2_Msk                  (0xFUL << SYSCFG_EXTICR1_EXTI2_Pos)               /*!< 0x00000F00 */
#define SYSCFG_EXTICR1_EXTI2                      SYSCFG_EXTICR1_EXTI2_Msk                          /*!< EXTI2[11:8] bits (desc EXTI2) */
#define SYSCFG_EXTICR1_EXTI2_0                    (0x1UL << SYSCFG_EXTICR1_EXTI2_Pos)               /*!< 0x00000100 */
#define SYSCFG_EXTICR1_EXTI2_1                    (0x2UL << SYSCFG_EXTICR1_EXTI2_Pos)               /*!< 0x00000200 */
#define SYSCFG_EXTICR1_EXTI2_2                    (0x4UL << SYSCFG_EXTICR1_EXTI2_Pos)               /*!< 0x00000400 */
#define SYSCFG_EXTICR1_EXTI2_3                    (0x8UL << SYSCFG_EXTICR1_EXTI2_Pos)               /*!< 0x00000800 */

#define SYSCFG_EXTICR1_EXTI3_Pos                  (12U)
#define SYSCFG_EXTICR1_EXTI3_Msk                  (0xFUL << SYSCFG_EXTICR1_EXTI3_Pos)               /*!< 0x0000F000 */
#define SYSCFG_EXTICR1_EXTI3                      SYSCFG_EXTICR1_EXTI3_Msk                          /*!< EXTI3[15:12] bits (desc EXTI3) */
#define SYSCFG_EXTICR1_EXTI3_0                    (0x1UL << SYSCFG_EXTICR1_EXTI3_Pos)               /*!< 0x00001000 */
#define SYSCFG_EXTICR1_EXTI3_1                    (0x2UL << SYSCFG_EXTICR1_EXTI3_Pos)               /*!< 0x00002000 */
#define SYSCFG_EXTICR1_EXTI3_2                    (0x4UL << SYSCFG_EXTICR1_EXTI3_Pos)               /*!< 0x00004000 */
#define SYSCFG_EXTICR1_EXTI3_3                    (0x8UL << SYSCFG_EXTICR1_EXTI3_Pos)               /*!< 0x00008000 */


/*!< SYSCFG_EXTICR2 */
#define SYSCFG_EXTICR2_EXTI4_Pos                  (0U)
#define SYSCFG_EXTICR2_EXTI4_Msk                  (0xFUL << SYSCFG_EXTICR2_EXTI4_Pos)               /*!< 0x0000000F */
#define SYSCFG_EXTICR2_EXTI4                      SYSCFG_EXTICR2_EXTI4_Msk                          /*!< EXTI4[3:0] bits (desc EXTI4) */
#define SYSCFG_EXTICR2_EXTI4_0                    (0x1UL << SYSCFG_EXTICR2_EXTI4_Pos)               /*!< 0x00000001 */
#define SYSCFG_EXTICR2_EXTI4_1                    (0x2UL << SYSCFG_EXTICR2_EXTI4_Pos)               /*!< 0x00000002 */
#define SYSCFG_EXTICR2_EXTI4_2                    (0x4UL << SYSCFG_EXTICR2_EXTI4_Pos)               /*!< 0x00000004 */
#define SYSCFG_EXTICR2_EXTI4_3                    (0x8UL << SYSCFG_EXTICR2_EXTI4_Pos)               /*!< 0x00000008 */

#define SYSCFG_EXTICR2_EXTI5_Pos                  (4U)
#define SYSCFG_EXTICR2_EXTI5_Msk                  (0xFUL << SYSCFG_EXTICR2_EXTI5_Pos)               /*!< 0x000000F0 */
#define SYSCFG_EXTICR2_EXTI5                      SYSCFG_EXTICR2_EXTI5_Msk                          /*!< EXTI5[7:4] bits (desc EXTI5) */
#define SYSCFG_EXTICR2_EXTI5_0                    (0x1UL << SYSCFG_EXTICR2_EXTI5_Pos)               /*!< 0x00000010 */
#define SYSCFG_EXTICR2_EXTI5_1                    (0x2UL << SYSCFG_EXTICR2_EXTI5_Pos)               /*!< 0x00000020 */
#define SYSCFG_EXTICR2_EXTI5_2                    (0x4UL << SYSCFG_EXTICR2_EXTI5_Pos)               /*!< 0x00000040 */
#define SYSCFG_EXTICR2_EXTI5_3                    (0x8UL << SYSCFG_EXTICR2_EXTI5_Pos)               /*!< 0x00000080 */

#define SYSCFG_EXTICR2_EXTI6_Pos                  (8U)
#define SYSCFG_EXTICR2_EXTI6_Msk                  (0xFUL << SYSCFG_EXTICR2_EXTI6_Pos)               /*!< 0x00000F00 */
#define SYSCFG_EXTICR2_EXTI6                      SYSCFG_EXTICR2_EXTI6_Msk                          /*!< EXTI6[11:8] bits (desc EXTI6) */
#define SYSCFG_EXTICR2_EXTI6_0                    (0x1UL << SYSCFG_EXTICR2_EXTI6_Pos)               /*!< 0x00000100 */
#define SYSCFG_EXTICR2_EXTI6_1                    (0x2UL << SYSCFG_EXTICR2_EXTI6_Pos)               /*!< 0x00000200 */
#define SYSCFG_EXTICR2_EXTI6_2                    (0x4UL << SYSCFG_EXTICR2_EXTI6_Pos)               /*!< 0x00000400 */
#define SYSCFG_EXTICR2_EXTI6_3                    (0x8UL << SYSCFG_EXTICR2_EXTI6_Pos)               /*!< 0x00000800 */

#define SYSCFG_EXTICR2_EXTI7_Pos                  (12U)
#define SYSCFG_EXTICR2_EXTI7_Msk                  (0xFUL << SYSCFG_EXTICR2_EXTI7_Pos)               /*!< 0x0000F000 */
#define SYSCFG_EXTICR2_EXTI7                      SYSCFG_EXTICR2_EXTI7_Msk                          /*!< EXTI7[15:12] bits (desc EXTI7) */
#define SYSCFG_EXTICR2_EXTI7_0                    (0x1UL << SYSCFG_EXTICR2_EXTI7_Pos)               /*!< 0x00001000 */
#define SYSCFG_EXTICR2_EXTI7_1                    (0x2UL << SYSCFG_EXTICR2_EXTI7_Pos)               /*!< 0x00002000 */
#define SYSCFG_EXTICR2_EXTI7_2                    (0x4UL << SYSCFG_EXTICR2_EXTI7_Pos)               /*!< 0x00004000 */
#define SYSCFG_EXTICR2_EXTI7_3                    (0x8UL << SYSCFG_EXTICR2_EXTI7_Pos)               /*!< 0x00008000 */


/*!< SYSCFG_EXTICR3 */
#define SYSCFG_EXTICR3_EXTI8_Pos                  (0U)
#define SYSCFG_EXTICR3_EXTI8_Msk                  (0xFUL << SYSCFG_EXTICR3_EXTI8_Pos)               /*!< 0x0000000F */
#define SYSCFG_EXTICR3_EXTI8                      SYSCFG_EXTICR3_EXTI8_Msk                          /*!< EXTI8[3:0] bits (desc EXTI8) */
#define SYSCFG_EXTICR3_EXTI8_0                    (0x1UL << SYSCFG_EXTICR3_EXTI8_Pos)               /*!< 0x00000001 */
#define SYSCFG_EXTICR3_EXTI8_1                    (0x2UL << SYSCFG_EXTICR3_EXTI8_Pos)               /*!< 0x00000002 */
#define SYSCFG_EXTICR3_EXTI8_2                    (0x4UL << SYSCFG_EXTICR3_EXTI8_Pos)               /*!< 0x00000004 */
#define SYSCFG_EXTICR3_EXTI8_3                    (0x8UL << SYSCFG_EXTICR3_EXTI8_Pos)               /*!< 0x00000008 */

#define SYSCFG_EXTICR3_EXTI9_Pos                  (4U)
#define SYSCFG_EXTICR3_EXTI9_Msk                  (0xFUL << SYSCFG_EXTICR3_EXTI9_Pos)               /*!< 0x000000F0 */
#define SYSCFG_EXTICR3_EXTI9                      SYSCFG_EXTICR3_EXTI9_Msk                          /*!< EXTI9[7:4] bits (desc EXTI9) */
#define SYSCFG_EXTICR3_EXTI9_0                    (0x1UL << SYSCFG_EXTICR3_EXTI9_Pos)               /*!< 0x00000010 */
#define SYSCFG_EXTICR3_EXTI9_1                    (0x2UL << SYSCFG_EXTICR3_EXTI9_Pos)               /*!< 0x00000020 */
#define SYSCFG_EXTICR3_EXTI9_2                    (0x4UL << SYSCFG_EXTICR3_EXTI9_Pos)               /*!< 0x00000040 */
#define SYSCFG_EXTICR3_EXTI9_3                    (0x8UL << SYSCFG_EXTICR3_EXTI9_Pos)               /*!< 0x00000080 */

#define SYSCFG_EXTICR3_EXTI10_Pos                 (8U)
#define SYSCFG_EXTICR3_EXTI10_Msk                 (0xFUL << SYSCFG_EXTICR3_EXTI10_Pos)              /*!< 0x00000F00 */
#define SYSCFG_EXTICR3_EXTI10                     SYSCFG_EXTICR3_EXTI10_Msk                         /*!< EXTI10[11:8] bits (desc EXTI10) */
#define SYSCFG_EXTICR3_EXTI10_0                   (0x1UL << SYSCFG_EXTICR3_EXTI10_Pos)              /*!< 0x00000100 */
#define SYSCFG_EXTICR3_EXTI10_1                   (0x2UL << SYSCFG_EXTICR3_EXTI10_Pos)              /*!< 0x00000200 */
#define SYSCFG_EXTICR3_EXTI10_2                   (0x4UL << SYSCFG_EXTICR3_EXTI10_Pos)              /*!< 0x00000400 */
#define SYSCFG_EXTICR3_EXTI10_3                   (0x8UL << SYSCFG_EXTICR3_EXTI10_Pos)              /*!< 0x00000800 */

#define SYSCFG_EXTICR3_EXTI11_Pos                 (12U)
#define SYSCFG_EXTICR3_EXTI11_Msk                 (0xFUL << SYSCFG_EXTICR3_EXTI11_Pos)              /*!< 0x0000F000 */
#define SYSCFG_EXTICR3_EXTI11                     SYSCFG_EXTICR3_EXTI11_Msk                         /*!< EXTI11[15:12] bits (desc EXTI11) */
#define SYSCFG_EXTICR3_EXTI11_0                   (0x1UL << SYSCFG_EXTICR3_EXTI11_Pos)              /*!< 0x00001000 */
#define SYSCFG_EXTICR3_EXTI11_1                   (0x2UL << SYSCFG_EXTICR3_EXTI11_Pos)              /*!< 0x00002000 */
#define SYSCFG_EXTICR3_EXTI11_2                   (0x4UL << SYSCFG_EXTICR3_EXTI11_Pos)              /*!< 0x00004000 */
#define SYSCFG_EXTICR3_EXTI11_3                   (0x8UL << SYSCFG_EXTICR3_EXTI11_Pos)              /*!< 0x00008000 */


/*!< SYSCFG_EXTICR4 */
#define SYSCFG_EXTICR4_EXTI12_Pos                  (0U)
#define SYSCFG_EXTICR4_EXTI12_Msk                  (0xFUL << SYSCFG_EXTICR4_EXTI12_Pos)             /*!< 0x0000000F */
#define SYSCFG_EXTICR4_EXTI12                      SYSCFG_EXTICR4_EXTI12_Msk                        /*!< EXTI12[3:0] bits (desc EXTI12) */
#define SYSCFG_EXTICR4_EXTI12_0                    (0x1UL << SYSCFG_EXTICR4_EXTI12_Pos)             /*!< 0x00000001 */
#define SYSCFG_EXTICR4_EXTI12_1                    (0x2UL << SYSCFG_EXTICR4_EXTI12_Pos)             /*!< 0x00000002 */
#define SYSCFG_EXTICR4_EXTI12_2                    (0x4UL << SYSCFG_EXTICR4_EXTI12_Pos)             /*!< 0x00000004 */
#define SYSCFG_EXTICR4_EXTI12_3                    (0x8UL << SYSCFG_EXTICR4_EXTI12_Pos)             /*!< 0x00000008 */

#define SYSCFG_EXTICR4_EXTI13_Pos                  (4U)
#define SYSCFG_EXTICR4_EXTI13_Msk                  (0xFUL << SYSCFG_EXTICR4_EXTI13_Pos)             /*!< 0x000000F0 */
#define SYSCFG_EXTICR4_EXTI13                      SYSCFG_EXTICR4_EXTI13_Msk                        /*!< EXTI13[7:4] bits (desc EXTI13) */
#define SYSCFG_EXTICR4_EXTI13_0                    (0x1UL << SYSCFG_EXTICR4_EXTI13_Pos)             /*!< 0x00000010 */
#define SYSCFG_EXTICR4_EXTI13_1                    (0x2UL << SYSCFG_EXTICR4_EXTI13_Pos)             /*!< 0x00000020 */
#define SYSCFG_EXTICR4_EXTI13_2                    (0x4UL << SYSCFG_EXTICR4_EXTI13_Pos)             /*!< 0x00000040 */
#define SYSCFG_EXTICR4_EXTI13_3                    (0x8UL << SYSCFG_EXTICR4_EXTI13_Pos)             /*!< 0x00000080 */

#define SYSCFG_EXTICR4_EXTI14_Pos                  (8U)
#define SYSCFG_EXTICR4_EXTI14_Msk                  (0xFUL << SYSCFG_EXTICR4_EXTI14_Pos)             /*!< 0x00000F00 */
#define SYSCFG_EXTICR4_EXTI14                      SYSCFG_EXTICR4_EXTI14_Msk                        /*!< EXTI14[11:8] bits (desc EXTI14) */
#define SYSCFG_EXTICR4_EXTI14_0                    (0x1UL << SYSCFG_EXTICR4_EXTI14_Pos)             /*!< 0x00000100 */
#define SYSCFG_EXTICR4_EXTI14_1                    (0x2UL << SYSCFG_EXTICR4_EXTI14_Pos)             /*!< 0x00000200 */
#define SYSCFG_EXTICR4_EXTI14_2                    (0x4UL << SYSCFG_EXTICR4_EXTI14_Pos)             /*!< 0x00000400 */
#define SYSCFG_EXTICR4_EXTI14_3                    (0x8UL << SYSCFG_EXTICR4_EXTI14_Pos)             /*!< 0x00000800 */

#define SYSCFG_EXTICR4_EXTI15_Pos                  (12U)
#define SYSCFG_EXTICR4_EXTI15_Msk                  (0xFUL << SYSCFG_EXTICR4_EXTI15_Pos)             /*!< 0x0000F000 */
#define SYSCFG_EXTICR4_EXTI15                      SYSCFG_EXTICR4_EXTI15_Msk                        /*!< EXTI15[15:12] bits (desc EXTI15) */
#define SYSCFG_EXTICR4_EXTI15_0                    (0x1UL << SYSCFG_EXTICR4_EXTI15_Pos)             /*!< 0x00001000 */
#define SYSCFG_EXTICR4_EXTI15_1                    (0x2UL << SYSCFG_EXTICR4_EXTI15_Pos)             /*!< 0x00002000 */
#define SYSCFG_EXTICR4_EXTI15_2                    (0x4UL << SYSCFG_EXTICR4_EXTI15_Pos)             /*!< 0x00004000 */
#define SYSCFG_EXTICR4_EXTI15_3                    (0x8UL << SYSCFG_EXTICR4_EXTI15_Pos)             /*!< 0x00008000 */


/*!< SYSCFG_PAENS */
#define SYSCFG_PAENS_PA_ENS_Pos                   (0U)
#define SYSCFG_PAENS_PA_ENS_Msk                   (0xFFFFUL << SYSCFG_PAENS_PA_ENS_Pos)             /*!< 0x0000FFFF */
#define SYSCFG_PAENS_PA_ENS                       SYSCFG_PAENS_PA_ENS_Msk                           /*!< PA_ENS[15:0] bits (desc PA_ENS) */

/*!< SYSCFG_PBENS */
#define SYSCFG_PBENS_PB_ENS_Pos                   (0U)
#define SYSCFG_PBENS_PB_ENS_Msk                   (0xFFFFUL << SYSCFG_PBENS_PB_ENS_Pos)             /*!< 0x0000FFFF */
#define SYSCFG_PBENS_PB_ENS                       SYSCFG_PBENS_PB_ENS_Msk                           /*!< PB_ENS[15:0] bits (desc PB_ENS) */

/*!< SYSCFG_PCENS */
#define SYSCFG_PCENS_PC_ENS_Pos                   (0U)
#define SYSCFG_PCENS_PC_ENS_Msk                   (0xFFFFUL << SYSCFG_PCENS_PC_ENS_Pos)             /*!< 0x0000FFFF */
#define SYSCFG_PCENS_PC_ENS                       SYSCFG_PCENS_PC_ENS_Msk                           /*!< PC_ENS[15:0] bits (desc PC_ENS) */

/*!< SYSCFG_PDENS */
#define SYSCFG_PDENS_PD_ENS_Pos                   (0U)
#define SYSCFG_PDENS_PD_ENS_Msk                   (0xFFFFUL << SYSCFG_PDENS_PD_ENS_Pos)             /*!< 0x0000FFFF */
#define SYSCFG_PDENS_PD_ENS                       SYSCFG_PDENS_PD_ENS_Msk                           /*!< PD_ENS[15:0] bits (desc PD_ENS) */

/*!< SYSCFG_PEENS */
#define SYSCFG_PEENS_PE_ENS_Pos                   (0U)
#define SYSCFG_PEENS_PE_ENS_Msk                   (0xFFFFUL << SYSCFG_PEENS_PE_ENS_Pos)             /*!< 0x0000FFFF */
#define SYSCFG_PEENS_PE_ENS                       SYSCFG_PEENS_PE_ENS_Msk                           /*!< PE_ENS[15:0] bits (desc PE_ENS) */

/*!< SYSCFG_GPIOENA */
#define SYSCFG_GPIOENA_PA_ENA_Pos                 (0U)
#define SYSCFG_GPIOENA_PA_ENA_Msk                 (0xFFUL << SYSCFG_GPIOENA_PA_ENA_Pos)             /*!< 0x000000FF */
#define SYSCFG_GPIOENA_PA_ENA                     SYSCFG_GPIOENA_PA_ENA_Msk                         /*!< PA_ENA[7:0] bits (desc PA_ENA) */
#define SYSCFG_GPIOENA_PA_ENA_0                   (0x1UL << SYSCFG_GPIOENA_PA_ENA_Pos)              /*!< 0x00000001 */
#define SYSCFG_GPIOENA_PA_ENA_1                   (0x2UL << SYSCFG_GPIOENA_PA_ENA_Pos)              /*!< 0x00000002 */
#define SYSCFG_GPIOENA_PA_ENA_2                   (0x4UL << SYSCFG_GPIOENA_PA_ENA_Pos)              /*!< 0x00000004 */
#define SYSCFG_GPIOENA_PA_ENA_3                   (0x8UL << SYSCFG_GPIOENA_PA_ENA_Pos)              /*!< 0x00000008 */
#define SYSCFG_GPIOENA_PA_ENA_4                   (0x10UL << SYSCFG_GPIOENA_PA_ENA_Pos)             /*!< 0x00000010 */
#define SYSCFG_GPIOENA_PA_ENA_5                   (0x20UL << SYSCFG_GPIOENA_PA_ENA_Pos)             /*!< 0x00000020 */
#define SYSCFG_GPIOENA_PA_ENA_6                   (0x40UL << SYSCFG_GPIOENA_PA_ENA_Pos)             /*!< 0x00000040 */
#define SYSCFG_GPIOENA_PA_ENA_7                   (0x80UL << SYSCFG_GPIOENA_PA_ENA_Pos)             /*!< 0x00000080 */

#define SYSCFG_GPIOENA_PB_ENA_Pos                 (8U)
#define SYSCFG_GPIOENA_PB_ENA_Msk                 (0x3UL << SYSCFG_GPIOENA_PB_ENA_Pos)              /*!< 0x00000300 */
#define SYSCFG_GPIOENA_PB_ENA                     SYSCFG_GPIOENA_PB_ENA_Msk                         /*!< PB_ENA[9:8] bits (desc PB_ENA) */
#define SYSCFG_GPIOENA_PB_ENA_0                   (0x1UL << SYSCFG_GPIOENA_PB_ENA_Pos)              /*!< 0x00000100 */
#define SYSCFG_GPIOENA_PB_ENA_1                   (0x2UL << SYSCFG_GPIOENA_PB_ENA_Pos)              /*!< 0x00000200 */

#define SYSCFG_GPIOENA_PC_ENA_Pos                 (16U)
#define SYSCFG_GPIOENA_PC_ENA_Msk                 (0x3FUL << SYSCFG_GPIOENA_PC_ENA_Pos)             /*!< 0x003F0000 */
#define SYSCFG_GPIOENA_PC_ENA                     SYSCFG_GPIOENA_PC_ENA_Msk                         /*!< PC_ENA[21:16] bits (desc PC_ENA) */
#define SYSCFG_GPIOENA_PC_ENA_0                   (0x1UL << SYSCFG_GPIOENA_PC_ENA_Pos)              /*!< 0x00010000 */
#define SYSCFG_GPIOENA_PC_ENA_1                   (0x2UL << SYSCFG_GPIOENA_PC_ENA_Pos)              /*!< 0x00020000 */
#define SYSCFG_GPIOENA_PC_ENA_2                   (0x4UL << SYSCFG_GPIOENA_PC_ENA_Pos)              /*!< 0x00040000 */
#define SYSCFG_GPIOENA_PC_ENA_3                   (0x8UL << SYSCFG_GPIOENA_PC_ENA_Pos)              /*!< 0x00080000 */
#define SYSCFG_GPIOENA_PC_ENA_4                   (0x10UL << SYSCFG_GPIOENA_PC_ENA_Pos)             /*!< 0x00100000 */
#define SYSCFG_GPIOENA_PC_ENA_5                   (0x20UL << SYSCFG_GPIOENA_PC_ENA_Pos)             /*!< 0x00200000 */

#define SYSCFG_TIM_PCLK1_SEL_Pos                 (6U)
#define SYSCFG_TIM_PCLK1_SEL_Msk                 (0x1UL << SYSCFG_TIM_PCLK1_SEL_Pos)
#define SYSCFG_TIM_PCLK1_SEL                     SYSCFG_TIM_PCLK1_SEL_Msk
#define SYSCFG_TIM_PCLK2_SEL_Pos                 (7U)
#define SYSCFG_TIM_PCLK2_SEL_Msk                 (0x1UL << SYSCFG_TIM_PCLK2_SEL_Pos)
#define SYSCFG_TIM_PCLK2_SEL                     SYSCFG_TIM_PCLK2_SEL_Msk

/*********************  Bits Define For Peripheral TIM  *********************/
/*!< TIM_CR1 */
#define TIM_CR1_CEN_Pos                           (0U)
#define TIM_CR1_CEN_Msk                           (0x1UL << TIM_CR1_CEN_Pos)                        /*!< 0x00000001 */
#define TIM_CR1_CEN                               TIM_CR1_CEN_Msk                                   /*!< desc CEN */
#define TIM_CR1_UDIS_Pos                          (1U)
#define TIM_CR1_UDIS_Msk                          (0x1UL << TIM_CR1_UDIS_Pos)                       /*!< 0x00000002 */
#define TIM_CR1_UDIS                              TIM_CR1_UDIS_Msk                                  /*!< desc UDIS */
#define TIM_CR1_URS_Pos                           (2U)
#define TIM_CR1_URS_Msk                           (0x1UL << TIM_CR1_URS_Pos)                        /*!< 0x00000004 */
#define TIM_CR1_URS                               TIM_CR1_URS_Msk                                   /*!< desc URS */
#define TIM_CR1_OPM_Pos                           (3U)
#define TIM_CR1_OPM_Msk                           (0x1UL << TIM_CR1_OPM_Pos)                        /*!< 0x00000008 */
#define TIM_CR1_OPM                               TIM_CR1_OPM_Msk                                   /*!< desc OPM */
#define TIM_CR1_DIR_Pos                           (4U)
#define TIM_CR1_DIR_Msk                           (0x1UL << TIM_CR1_DIR_Pos)                        /*!< 0x00000010 */
#define TIM_CR1_DIR                               TIM_CR1_DIR_Msk                                   /*!< desc DIR */
#define TIM_CR1_CMS_Pos                           (5U)
#define TIM_CR1_CMS_Msk                           (0x3UL << TIM_CR1_CMS_Pos)                        /*!< 0x00000060 */
#define TIM_CR1_CMS                               TIM_CR1_CMS_Msk                                   /*!< CMS[6:5] bits (desc CMS) */
#define TIM_CR1_CMS_0                             (0x1UL << TIM_CR1_CMS_Pos)                        /*!< 0x00000020 */
#define TIM_CR1_CMS_1                             (0x2UL << TIM_CR1_CMS_Pos)                        /*!< 0x00000040 */

#define TIM_CR1_ARPE_Pos                          (7U)
#define TIM_CR1_ARPE_Msk                          (0x1UL << TIM_CR1_ARPE_Pos)                       /*!< 0x00000080 */
#define TIM_CR1_ARPE                              TIM_CR1_ARPE_Msk                                  /*!< desc ARPE */
#define TIM_CR1_CKD_Pos                           (8U)
#define TIM_CR1_CKD_Msk                           (0x3UL << TIM_CR1_CKD_Pos)                        /*!< 0x00000300 */
#define TIM_CR1_CKD                               TIM_CR1_CKD_Msk                                   /*!< CKD[9:8] bits (desc CKD) */
#define TIM_CR1_CKD_0                             (0x1UL << TIM_CR1_CKD_Pos)                        /*!< 0x00000100 */
#define TIM_CR1_CKD_1                             (0x2UL << TIM_CR1_CKD_Pos)                        /*!< 0x00000200 */


/*!< TIM_CR2 */
#define TIM_CR2_CCPC_Pos                          (0U)
#define TIM_CR2_CCPC_Msk                          (0x1UL << TIM_CR2_CCPC_Pos)                       /*!< 0x00000001 */
#define TIM_CR2_CCPC                              TIM_CR2_CCPC_Msk                                  /*!< desc CCPC */
#define TIM_CR2_CCUS_Pos                          (2U)
#define TIM_CR2_CCUS_Msk                          (0x1UL << TIM_CR2_CCUS_Pos)                       /*!< 0x00000004 */
#define TIM_CR2_CCUS                              TIM_CR2_CCUS_Msk                                  /*!< desc CCUS */
#define TIM_CR2_CCDS_Pos                          (3U)
#define TIM_CR2_CCDS_Msk                          (0x1UL << TIM_CR2_CCDS_Pos)                       /*!< 0x00000008 */
#define TIM_CR2_CCDS                              TIM_CR2_CCDS_Msk                                  /*!< desc CCDS */
#define TIM_CR2_MMS_Pos                           (4U)
#define TIM_CR2_MMS_Msk                           (0x7UL << TIM_CR2_MMS_Pos)                        /*!< 0x00000070 */
#define TIM_CR2_MMS                               TIM_CR2_MMS_Msk                                   /*!< MMS[6:4] bits (desc MMS) */
#define TIM_CR2_MMS_0                             (0x1UL << TIM_CR2_MMS_Pos)                        /*!< 0x00000010 */
#define TIM_CR2_MMS_1                             (0x2UL << TIM_CR2_MMS_Pos)                        /*!< 0x00000020 */
#define TIM_CR2_MMS_2                             (0x4UL << TIM_CR2_MMS_Pos)                        /*!< 0x00000040 */

#define TIM_CR2_TI1S_Pos                          (7U)
#define TIM_CR2_TI1S_Msk                          (0x1UL << TIM_CR2_TI1S_Pos)                       /*!< 0x00000080 */
#define TIM_CR2_TI1S                              TIM_CR2_TI1S_Msk                                  /*!< desc TI1S */
#define TIM_CR2_OIS1_Pos                          (8U)
#define TIM_CR2_OIS1_Msk                          (0x1UL << TIM_CR2_OIS1_Pos)                       /*!< 0x00000100 */
#define TIM_CR2_OIS1                              TIM_CR2_OIS1_Msk                                  /*!< desc OIS1 */
#define TIM_CR2_OIS1N_Pos                         (9U)
#define TIM_CR2_OIS1N_Msk                         (0x1UL << TIM_CR2_OIS1N_Pos)                      /*!< 0x00000200 */
#define TIM_CR2_OIS1N                             TIM_CR2_OIS1N_Msk                                 /*!< desc OIS1N */
#define TIM_CR2_OIS2_Pos                          (10U)
#define TIM_CR2_OIS2_Msk                          (0x1UL << TIM_CR2_OIS2_Pos)                       /*!< 0x00000400 */
#define TIM_CR2_OIS2                              TIM_CR2_OIS2_Msk                                  /*!< desc OIS2 */
#define TIM_CR2_OIS2N_Pos                         (11U)
#define TIM_CR2_OIS2N_Msk                         (0x1UL << TIM_CR2_OIS2N_Pos)                      /*!< 0x00000800 */
#define TIM_CR2_OIS2N                             TIM_CR2_OIS2N_Msk                                 /*!< desc OIS2N */
#define TIM_CR2_OIS3_Pos                          (12U)
#define TIM_CR2_OIS3_Msk                          (0x1UL << TIM_CR2_OIS3_Pos)                       /*!< 0x00001000 */
#define TIM_CR2_OIS3                              TIM_CR2_OIS3_Msk                                  /*!< desc OIS3 */
#define TIM_CR2_OIS3N_Pos                         (13U)
#define TIM_CR2_OIS3N_Msk                         (0x1UL << TIM_CR2_OIS3N_Pos)                      /*!< 0x00002000 */
#define TIM_CR2_OIS3N                             TIM_CR2_OIS3N_Msk                                 /*!< desc OIS3N */
#define TIM_CR2_OIS4_Pos                          (14U)
#define TIM_CR2_OIS4_Msk                          (0x1UL << TIM_CR2_OIS4_Pos)                       /*!< 0x00004000 */
#define TIM_CR2_OIS4                              TIM_CR2_OIS4_Msk                                  /*!< desc OIS4 */

/*!< TIM_SMCR */
#define TIM_SMCR_SMS_Pos                          (0U)
#define TIM_SMCR_SMS_Msk                          (0x7UL << TIM_SMCR_SMS_Pos)                       /*!< 0x00000007 */
#define TIM_SMCR_SMS                              TIM_SMCR_SMS_Msk                                  /*!< SMS[2:0] bits (desc SMS) */
#define TIM_SMCR_SMS_0                            (0x1UL << TIM_SMCR_SMS_Pos)                       /*!< 0x00000001 */
#define TIM_SMCR_SMS_1                            (0x2UL << TIM_SMCR_SMS_Pos)                       /*!< 0x00000002 */
#define TIM_SMCR_SMS_2                            (0x4UL << TIM_SMCR_SMS_Pos)                       /*!< 0x00000004 */

#define TIM_SMCR_TS_Pos                           (4U)
#define TIM_SMCR_TS_Msk                           (0x7UL << TIM_SMCR_TS_Pos)                        /*!< 0x00000070 */
#define TIM_SMCR_TS                               TIM_SMCR_TS_Msk                                   /*!< TS[6:4] bits (desc TS) */
#define TIM_SMCR_TS_0                             (0x1UL << TIM_SMCR_TS_Pos)                        /*!< 0x00000010 */
#define TIM_SMCR_TS_1                             (0x2UL << TIM_SMCR_TS_Pos)                        /*!< 0x00000020 */
#define TIM_SMCR_TS_2                             (0x4UL << TIM_SMCR_TS_Pos)                        /*!< 0x00000040 */

#define TIM_SMCR_MSM_Pos                          (7U)
#define TIM_SMCR_MSM_Msk                          (0x1UL << TIM_SMCR_MSM_Pos)                       /*!< 0x00000080 */
#define TIM_SMCR_MSM                              TIM_SMCR_MSM_Msk                                  /*!< desc MSM */
#define TIM_SMCR_ETF_Pos                          (8U)
#define TIM_SMCR_ETF_Msk                          (0xFUL << TIM_SMCR_ETF_Pos)                       /*!< 0x00000F00 */
#define TIM_SMCR_ETF                              TIM_SMCR_ETF_Msk                                  /*!< ETF[11:8] bits (desc ETF) */
#define TIM_SMCR_ETF_0                            (0x1UL << TIM_SMCR_ETF_Pos)                       /*!< 0x00000100 */
#define TIM_SMCR_ETF_1                            (0x2UL << TIM_SMCR_ETF_Pos)                       /*!< 0x00000200 */
#define TIM_SMCR_ETF_2                            (0x4UL << TIM_SMCR_ETF_Pos)                       /*!< 0x00000400 */
#define TIM_SMCR_ETF_3                            (0x8UL << TIM_SMCR_ETF_Pos)                       /*!< 0x00000800 */

#define TIM_SMCR_ETPS_Pos                         (12U)
#define TIM_SMCR_ETPS_Msk                         (0x3UL << TIM_SMCR_ETPS_Pos)                      /*!< 0x00003000 */
#define TIM_SMCR_ETPS                             TIM_SMCR_ETPS_Msk                                 /*!< ETPS[13:12] bits (desc ETPS) */
#define TIM_SMCR_ETPS_0                           (0x1UL << TIM_SMCR_ETPS_Pos)                      /*!< 0x00001000 */
#define TIM_SMCR_ETPS_1                           (0x2UL << TIM_SMCR_ETPS_Pos)                      /*!< 0x00002000 */

#define TIM_SMCR_ECE_Pos                          (14U)
#define TIM_SMCR_ECE_Msk                          (0x1UL << TIM_SMCR_ECE_Pos)                       /*!< 0x00004000 */
#define TIM_SMCR_ECE                              TIM_SMCR_ECE_Msk                                  /*!< desc ECE */
#define TIM_SMCR_ETP_Pos                          (15U)
#define TIM_SMCR_ETP_Msk                          (0x1UL << TIM_SMCR_ETP_Pos)                       /*!< 0x00008000 */
#define TIM_SMCR_ETP                              TIM_SMCR_ETP_Msk                                  /*!< desc ETP */

/*!< TIM_DIER */
#define TIM_DIER_UIE_Pos                          (0U)
#define TIM_DIER_UIE_Msk                          (0x1UL << TIM_DIER_UIE_Pos)                       /*!< 0x00000001 */
#define TIM_DIER_UIE                              TIM_DIER_UIE_Msk                                  /*!< desc UIE */
#define TIM_DIER_CC1IE_Pos                        (1U)
#define TIM_DIER_CC1IE_Msk                        (0x1UL << TIM_DIER_CC1IE_Pos)                     /*!< 0x00000002 */
#define TIM_DIER_CC1IE                            TIM_DIER_CC1IE_Msk                                /*!< desc CC1IE */
#define TIM_DIER_CC2IE_Pos                        (2U)
#define TIM_DIER_CC2IE_Msk                        (0x1UL << TIM_DIER_CC2IE_Pos)                     /*!< 0x00000004 */
#define TIM_DIER_CC2IE                            TIM_DIER_CC2IE_Msk                                /*!< desc CC2IE */
#define TIM_DIER_CC3IE_Pos                        (3U)
#define TIM_DIER_CC3IE_Msk                        (0x1UL << TIM_DIER_CC3IE_Pos)                     /*!< 0x00000008 */
#define TIM_DIER_CC3IE                            TIM_DIER_CC3IE_Msk                                /*!< desc CC3IE */
#define TIM_DIER_CC4IE_Pos                        (4U)
#define TIM_DIER_CC4IE_Msk                        (0x1UL << TIM_DIER_CC4IE_Pos)                     /*!< 0x00000010 */
#define TIM_DIER_CC4IE                            TIM_DIER_CC4IE_Msk                                /*!< desc CC4IE */
#define TIM_DIER_COMIE_Pos                        (5U)
#define TIM_DIER_COMIE_Msk                        (0x1UL << TIM_DIER_COMIE_Pos)                     /*!< 0x00000020 */
#define TIM_DIER_COMIE                            TIM_DIER_COMIE_Msk                                /*!< desc COMIE */
#define TIM_DIER_TIE_Pos                          (6U)
#define TIM_DIER_TIE_Msk                          (0x1UL << TIM_DIER_TIE_Pos)                       /*!< 0x00000040 */
#define TIM_DIER_TIE                              TIM_DIER_TIE_Msk                                  /*!< desc TIE */
#define TIM_DIER_BIE_Pos                          (7U)
#define TIM_DIER_BIE_Msk                          (0x1UL << TIM_DIER_BIE_Pos)                       /*!< 0x00000080 */
#define TIM_DIER_BIE                              TIM_DIER_BIE_Msk                                  /*!< desc BIE */
#define TIM_DIER_UDE_Pos                          (8U)
#define TIM_DIER_UDE_Msk                          (0x1UL << TIM_DIER_UDE_Pos)                       /*!< 0x00000100 */
#define TIM_DIER_UDE                              TIM_DIER_UDE_Msk                                  /*!< desc UDE */
#define TIM_DIER_CC1DE_Pos                        (9U)
#define TIM_DIER_CC1DE_Msk                        (0x1UL << TIM_DIER_CC1DE_Pos)                     /*!< 0x00000200 */
#define TIM_DIER_CC1DE                            TIM_DIER_CC1DE_Msk                                /*!< desc CC1DE */
#define TIM_DIER_CC2DE_Pos                        (10U)
#define TIM_DIER_CC2DE_Msk                        (0x1UL << TIM_DIER_CC2DE_Pos)                     /*!< 0x00000400 */
#define TIM_DIER_CC2DE                            TIM_DIER_CC2DE_Msk                                /*!< desc CC2DE */
#define TIM_DIER_CC3DE_Pos                        (11U)
#define TIM_DIER_CC3DE_Msk                        (0x1UL << TIM_DIER_CC3DE_Pos)                     /*!< 0x00000800 */
#define TIM_DIER_CC3DE                            TIM_DIER_CC3DE_Msk                                /*!< desc CC3DE */
#define TIM_DIER_CC4DE_Pos                        (12U)
#define TIM_DIER_CC4DE_Msk                        (0x1UL << TIM_DIER_CC4DE_Pos)                     /*!< 0x00001000 */
#define TIM_DIER_CC4DE                            TIM_DIER_CC4DE_Msk                                /*!< desc CC4DE */
#define TIM_DIER_COMDE_Pos                        (13U)
#define TIM_DIER_COMDE_Msk                        (0x1UL << TIM_DIER_COMDE_Pos)                     /*!< 0x00002000 */
#define TIM_DIER_COMDE                            TIM_DIER_COMDE_Msk                                /*!< desc COMDE */
#define TIM_DIER_TDE_Pos                          (14U)
#define TIM_DIER_TDE_Msk                          (0x1UL << TIM_DIER_TDE_Pos)                       /*!< 0x00004000 */
#define TIM_DIER_TDE                              TIM_DIER_TDE_Msk                                  /*!< desc TDE */

/*!< TIM_SR */
#define TIM_SR_UIF_Pos                            (0U)
#define TIM_SR_UIF_Msk                            (0x1UL << TIM_SR_UIF_Pos)                         /*!< 0x00000001 */
#define TIM_SR_UIF                                TIM_SR_UIF_Msk                                    /*!< desc UIF */
#define TIM_SR_CC1IF_Pos                          (1U)
#define TIM_SR_CC1IF_Msk                          (0x1UL << TIM_SR_CC1IF_Pos)                       /*!< 0x00000002 */
#define TIM_SR_CC1IF                              TIM_SR_CC1IF_Msk                                  /*!< desc CC1IF */
#define TIM_SR_CC2IF_Pos                          (2U)
#define TIM_SR_CC2IF_Msk                          (0x1UL << TIM_SR_CC2IF_Pos)                       /*!< 0x00000004 */
#define TIM_SR_CC2IF                              TIM_SR_CC2IF_Msk                                  /*!< desc CC2IF */
#define TIM_SR_CC3IF_Pos                          (3U)
#define TIM_SR_CC3IF_Msk                          (0x1UL << TIM_SR_CC3IF_Pos)                       /*!< 0x00000008 */
#define TIM_SR_CC3IF                              TIM_SR_CC3IF_Msk                                  /*!< desc CC3IF */
#define TIM_SR_CC4IF_Pos                          (4U)
#define TIM_SR_CC4IF_Msk                          (0x1UL << TIM_SR_CC4IF_Pos)                       /*!< 0x00000010 */
#define TIM_SR_CC4IF                              TIM_SR_CC4IF_Msk                                  /*!< desc CC4IF */
#define TIM_SR_COMIF_Pos                          (5U)
#define TIM_SR_COMIF_Msk                          (0x1UL << TIM_SR_COMIF_Pos)                       /*!< 0x00000020 */
#define TIM_SR_COMIF                              TIM_SR_COMIF_Msk                                  /*!< desc COMIF */
#define TIM_SR_TIF_Pos                            (6U)
#define TIM_SR_TIF_Msk                            (0x1UL << TIM_SR_TIF_Pos)                         /*!< 0x00000040 */
#define TIM_SR_TIF                                TIM_SR_TIF_Msk                                    /*!< desc TIF */
#define TIM_SR_BIF_Pos                            (7U)
#define TIM_SR_BIF_Msk                            (0x1UL << TIM_SR_BIF_Pos)                         /*!< 0x00000080 */
#define TIM_SR_BIF                                TIM_SR_BIF_Msk                                    /*!< desc BIF */
#define TIM_SR_CC1OF_Pos                          (9U)
#define TIM_SR_CC1OF_Msk                          (0x1UL << TIM_SR_CC1OF_Pos)                       /*!< 0x00000200 */
#define TIM_SR_CC1OF                              TIM_SR_CC1OF_Msk                                  /*!< desc CC1OF */
#define TIM_SR_CC2OF_Pos                          (10U)
#define TIM_SR_CC2OF_Msk                          (0x1UL << TIM_SR_CC2OF_Pos)                       /*!< 0x00000400 */
#define TIM_SR_CC2OF                              TIM_SR_CC2OF_Msk                                  /*!< desc CC2OF */
#define TIM_SR_CC3OF_Pos                          (11U)
#define TIM_SR_CC3OF_Msk                          (0x1UL << TIM_SR_CC3OF_Pos)                       /*!< 0x00000800 */
#define TIM_SR_CC3OF                              TIM_SR_CC3OF_Msk                                  /*!< desc CC3OF */
#define TIM_SR_CC4OF_Pos                          (12U)
#define TIM_SR_CC4OF_Msk                          (0x1UL << TIM_SR_CC4OF_Pos)                       /*!< 0x00001000 */
#define TIM_SR_CC4OF                              TIM_SR_CC4OF_Msk                                  /*!< desc CC4OF */

/*!< TIM_EGR */
#define TIM_EGR_UG_Pos                            (0U)
#define TIM_EGR_UG_Msk                            (0x1UL << TIM_EGR_UG_Pos)                         /*!< 0x00000001 */
#define TIM_EGR_UG                                TIM_EGR_UG_Msk                                    /*!< desc UG */
#define TIM_EGR_CC1G_Pos                          (1U)
#define TIM_EGR_CC1G_Msk                          (0x1UL << TIM_EGR_CC1G_Pos)                       /*!< 0x00000002 */
#define TIM_EGR_CC1G                              TIM_EGR_CC1G_Msk                                  /*!< desc CC1G */
#define TIM_EGR_CC2G_Pos                          (2U)
#define TIM_EGR_CC2G_Msk                          (0x1UL << TIM_EGR_CC2G_Pos)                       /*!< 0x00000004 */
#define TIM_EGR_CC2G                              TIM_EGR_CC2G_Msk                                  /*!< desc CC2G */
#define TIM_EGR_CC3G_Pos                          (3U)
#define TIM_EGR_CC3G_Msk                          (0x1UL << TIM_EGR_CC3G_Pos)                       /*!< 0x00000008 */
#define TIM_EGR_CC3G                              TIM_EGR_CC3G_Msk                                  /*!< desc CC3G */
#define TIM_EGR_CC4G_Pos                          (4U)
#define TIM_EGR_CC4G_Msk                          (0x1UL << TIM_EGR_CC4G_Pos)                       /*!< 0x00000010 */
#define TIM_EGR_CC4G                              TIM_EGR_CC4G_Msk                                  /*!< desc CC4G */
#define TIM_EGR_COMG_Pos                          (5U)
#define TIM_EGR_COMG_Msk                          (0x1UL << TIM_EGR_COMG_Pos)                       /*!< 0x00000020 */
#define TIM_EGR_COMG                              TIM_EGR_COMG_Msk                                  /*!< desc COMG */
#define TIM_EGR_TG_Pos                            (6U)
#define TIM_EGR_TG_Msk                            (0x1UL << TIM_EGR_TG_Pos)                         /*!< 0x00000040 */
#define TIM_EGR_TG                                TIM_EGR_TG_Msk                                    /*!< desc TG */
#define TIM_EGR_BG_Pos                            (7U)
#define TIM_EGR_BG_Msk                            (0x1UL << TIM_EGR_BG_Pos)                         /*!< 0x00000080 */
#define TIM_EGR_BG                                TIM_EGR_BG_Msk                                    /*!< desc BG */

/*!< TIM_CCMR1_OUTPUT */
#define TIM_CCMR1_CC1S_Pos                        (0U)
#define TIM_CCMR1_CC1S_Msk                        (0x3UL << TIM_CCMR1_CC1S_Pos)                     /*!< 0x00000003 */
#define TIM_CCMR1_CC1S                            TIM_CCMR1_CC1S_Msk                                /*!< CC1S[1:0] bits (desc CC1S) */
#define TIM_CCMR1_CC1S_0                          (0x1UL << TIM_CCMR1_CC1S_Pos)                     /*!< 0x00000001 */
#define TIM_CCMR1_CC1S_1                          (0x2UL << TIM_CCMR1_CC1S_Pos)                     /*!< 0x00000002 */

#define TIM_CCMR1_OC1FE_Pos                       (2U)
#define TIM_CCMR1_OC1FE_Msk                       (0x1UL << TIM_CCMR1_OC1FE_Pos)                    /*!< 0x00000004 */
#define TIM_CCMR1_OC1FE                           TIM_CCMR1_OC1FE_Msk                               /*!< desc OC1FE */
#define TIM_CCMR1_OC1PE_Pos                       (3U)
#define TIM_CCMR1_OC1PE_Msk                       (0x1UL << TIM_CCMR1_OC1PE_Pos)                    /*!< 0x00000008 */
#define TIM_CCMR1_OC1PE                           TIM_CCMR1_OC1PE_Msk                               /*!< desc OC1PE */
#define TIM_CCMR1_OC1M_Pos                        (4U)
#define TIM_CCMR1_OC1M_Msk                        (0x7UL << TIM_CCMR1_OC1M_Pos)                     /*!< 0x00000070 */
#define TIM_CCMR1_OC1M                            TIM_CCMR1_OC1M_Msk                                /*!< OC1M[6:4] bits (desc OC1M) */
#define TIM_CCMR1_OC1M_0                          (0x1UL << TIM_CCMR1_OC1M_Pos)                     /*!< 0x00000010 */
#define TIM_CCMR1_OC1M_1                          (0x2UL << TIM_CCMR1_OC1M_Pos)                     /*!< 0x00000020 */
#define TIM_CCMR1_OC1M_2                          (0x4UL << TIM_CCMR1_OC1M_Pos)                     /*!< 0x00000040 */

#define TIM_CCMR1_OC1CE_Pos                       (7U)
#define TIM_CCMR1_OC1CE_Msk                       (0x1UL << TIM_CCMR1_OC1CE_Pos)                    /*!< 0x00000080 */
#define TIM_CCMR1_OC1CE                           TIM_CCMR1_OC1CE_Msk                               /*!< desc OC1CE */
#define TIM_CCMR1_CC2S_Pos                        (8U)
#define TIM_CCMR1_CC2S_Msk                        (0x3UL << TIM_CCMR1_CC2S_Pos)                     /*!< 0x00000300 */
#define TIM_CCMR1_CC2S                            TIM_CCMR1_CC2S_Msk                                /*!< CC2S[9:8] bits (desc CC2S) */
#define TIM_CCMR1_CC2S_0                          (0x1UL << TIM_CCMR1_CC2S_Pos)                     /*!< 0x00000100 */
#define TIM_CCMR1_CC2S_1                          (0x2UL << TIM_CCMR1_CC2S_Pos)                     /*!< 0x00000200 */

#define TIM_CCMR1_OC2FE_Pos                       (10U)
#define TIM_CCMR1_OC2FE_Msk                       (0x1UL << TIM_CCMR1_OC2FE_Pos)                    /*!< 0x00000400 */
#define TIM_CCMR1_OC2FE                           TIM_CCMR1_OC2FE_Msk                               /*!< desc OC2FE */
#define TIM_CCMR1_OC2PE_Pos                       (11U)
#define TIM_CCMR1_OC2PE_Msk                       (0x1UL << TIM_CCMR1_OC2PE_Pos)                    /*!< 0x00000800 */
#define TIM_CCMR1_OC2PE                           TIM_CCMR1_OC2PE_Msk                               /*!< desc OC2PE */
#define TIM_CCMR1_OC2M_Pos                        (12U)
#define TIM_CCMR1_OC2M_Msk                        (0x7UL << TIM_CCMR1_OC2M_Pos)                     /*!< 0x00007000 */
#define TIM_CCMR1_OC2M                            TIM_CCMR1_OC2M_Msk                                /*!< OC2M[14:12] bits (desc OC2M) */
#define TIM_CCMR1_OC2M_0                          (0x1UL << TIM_CCMR1_OC2M_Pos)                     /*!< 0x00001000 */
#define TIM_CCMR1_OC2M_1                          (0x2UL << TIM_CCMR1_OC2M_Pos)                     /*!< 0x00002000 */
#define TIM_CCMR1_OC2M_2                          (0x4UL << TIM_CCMR1_OC2M_Pos)                     /*!< 0x00004000 */

#define TIM_CCMR1_OC2CE_Pos                       (15U)
#define TIM_CCMR1_OC2CE_Msk                       (0x1UL << TIM_CCMR1_OC2CE_Pos)                    /*!< 0x00008000 */
#define TIM_CCMR1_OC2CE                           TIM_CCMR1_OC2CE_Msk                               /*!< desc OC2CE */

/*!< TIM_CCMR1_INPUT */
#define TIM_CCMR1_IC1PSC_Pos                      (2U)
#define TIM_CCMR1_IC1PSC_Msk                      (0x3UL << TIM_CCMR1_IC1PSC_Pos)                   /*!< 0x0000000C */
#define TIM_CCMR1_IC1PSC                          TIM_CCMR1_IC1PSC_Msk                              /*!< IC1PSC[3:2] bits (desc IC1PSC) */
#define TIM_CCMR1_IC1PSC_0                        (0x1UL << TIM_CCMR1_IC1PSC_Pos)                   /*!< 0x00000004 */
#define TIM_CCMR1_IC1PSC_1                        (0x2UL << TIM_CCMR1_IC1PSC_Pos)                   /*!< 0x00000008 */

#define TIM_CCMR1_IC1F_Pos                        (4U)
#define TIM_CCMR1_IC1F_Msk                        (0xFUL << TIM_CCMR1_IC1F_Pos)                     /*!< 0x000000F0 */
#define TIM_CCMR1_IC1F                            TIM_CCMR1_IC1F_Msk                                /*!< IC1F[7:4] bits (desc IC1F) */
#define TIM_CCMR1_IC1F_0                          (0x1UL << TIM_CCMR1_IC1F_Pos)                     /*!< 0x00000010 */
#define TIM_CCMR1_IC1F_1                          (0x2UL << TIM_CCMR1_IC1F_Pos)                     /*!< 0x00000020 */
#define TIM_CCMR1_IC1F_2                          (0x4UL << TIM_CCMR1_IC1F_Pos)                     /*!< 0x00000040 */
#define TIM_CCMR1_IC1F_3                          (0x8UL << TIM_CCMR1_IC1F_Pos)                     /*!< 0x00000080 */

#define TIM_CCMR1_IC2PSC_Pos                      (10U)
#define TIM_CCMR1_IC2PSC_Msk                      (0x3UL << TIM_CCMR1_IC2PSC_Pos)                   /*!< 0x00000C00 */
#define TIM_CCMR1_IC2PSC                          TIM_CCMR1_IC2PSC_Msk                              /*!< IC2PSC[11:10] bits (desc IC2PSC) */
#define TIM_CCMR1_IC2PSC_0                        (0x1UL << TIM_CCMR1_IC2PSC_Pos)                   /*!< 0x00000400 */
#define TIM_CCMR1_IC2PSC_1                        (0x2UL << TIM_CCMR1_IC2PSC_Pos)                   /*!< 0x00000800 */

#define TIM_CCMR1_IC2F_Pos                        (12U)
#define TIM_CCMR1_IC2F_Msk                        (0xFUL << TIM_CCMR1_IC2F_Pos)                     /*!< 0x0000F000 */
#define TIM_CCMR1_IC2F                            TIM_CCMR1_IC2F_Msk                                /*!< IC2F[15:12] bits (desc IC2F) */
#define TIM_CCMR1_IC2F_0                          (0x1UL << TIM_CCMR1_IC2F_Pos)                     /*!< 0x00001000 */
#define TIM_CCMR1_IC2F_1                          (0x2UL << TIM_CCMR1_IC2F_Pos)                     /*!< 0x00002000 */
#define TIM_CCMR1_IC2F_2                          (0x4UL << TIM_CCMR1_IC2F_Pos)                     /*!< 0x00004000 */
#define TIM_CCMR1_IC2F_3                          (0x8UL << TIM_CCMR1_IC2F_Pos)                     /*!< 0x00008000 */


/*!< TIM_CCMR2_OUTPUT */
#define TIM_CCMR2_CC3S_Pos                        (0U)
#define TIM_CCMR2_CC3S_Msk                        (0x3UL << TIM_CCMR2_CC3S_Pos)                     /*!< 0x00000003 */
#define TIM_CCMR2_CC3S                            TIM_CCMR2_CC3S_Msk                                /*!< CC3S[1:0] bits (desc CC3S) */
#define TIM_CCMR2_CC3S_0                          (0x1UL << TIM_CCMR2_CC3S_Pos)                     /*!< 0x00000001 */
#define TIM_CCMR2_CC3S_1                          (0x2UL << TIM_CCMR2_CC3S_Pos)                     /*!< 0x00000002 */

#define TIM_CCMR2_OC3FE_Pos                       (2U)
#define TIM_CCMR2_OC3FE_Msk                       (0x1UL << TIM_CCMR2_OC3FE_Pos)                    /*!< 0x00000004 */
#define TIM_CCMR2_OC3FE                           TIM_CCMR2_OC3FE_Msk                               /*!< desc OC3FE */
#define TIM_CCMR2_OC3PE_Pos                       (3U)
#define TIM_CCMR2_OC3PE_Msk                       (0x1UL << TIM_CCMR2_OC3PE_Pos)                    /*!< 0x00000008 */
#define TIM_CCMR2_OC3PE                           TIM_CCMR2_OC3PE_Msk                               /*!< desc OC3PE */
#define TIM_CCMR2_OC3M_Pos                        (4U)
#define TIM_CCMR2_OC3M_Msk                        (0x7UL << TIM_CCMR2_OC3M_Pos)                     /*!< 0x00000070 */
#define TIM_CCMR2_OC3M                            TIM_CCMR2_OC3M_Msk                                /*!< OC3M[6:4] bits (desc OC3M) */
#define TIM_CCMR2_OC3M_0                          (0x1UL << TIM_CCMR2_OC3M_Pos)                     /*!< 0x00000010 */
#define TIM_CCMR2_OC3M_1                          (0x2UL << TIM_CCMR2_OC3M_Pos)                     /*!< 0x00000020 */
#define TIM_CCMR2_OC3M_2                          (0x4UL << TIM_CCMR2_OC3M_Pos)                     /*!< 0x00000040 */

#define TIM_CCMR2_OC3CE_Pos                       (7U)
#define TIM_CCMR2_OC3CE_Msk                       (0x1UL << TIM_CCMR2_OC3CE_Pos)                    /*!< 0x00000080 */
#define TIM_CCMR2_OC3CE                           TIM_CCMR2_OC3CE_Msk                               /*!< desc OC3CE */
#define TIM_CCMR2_CC4S_Pos                        (8U)
#define TIM_CCMR2_CC4S_Msk                        (0x3UL << TIM_CCMR2_CC4S_Pos)                     /*!< 0x00000300 */
#define TIM_CCMR2_CC4S                            TIM_CCMR2_CC4S_Msk                                /*!< CC4S[9:8] bits (desc CC4S) */
#define TIM_CCMR2_CC4S_0                          (0x1UL << TIM_CCMR2_CC4S_Pos)                     /*!< 0x00000100 */
#define TIM_CCMR2_CC4S_1                          (0x2UL << TIM_CCMR2_CC4S_Pos)                     /*!< 0x00000200 */

#define TIM_CCMR2_OC4FE_Pos                       (10U)
#define TIM_CCMR2_OC4FE_Msk                       (0x1UL << TIM_CCMR2_OC4FE_Pos)                    /*!< 0x00000400 */
#define TIM_CCMR2_OC4FE                           TIM_CCMR2_OC4FE_Msk                               /*!< desc OC4FE */
#define TIM_CCMR2_OC4PE_Pos                       (11U)
#define TIM_CCMR2_OC4PE_Msk                       (0x1UL << TIM_CCMR2_OC4PE_Pos)                    /*!< 0x00000800 */
#define TIM_CCMR2_OC4PE                           TIM_CCMR2_OC4PE_Msk                               /*!< desc OC4PE */
#define TIM_CCMR2_OC4M_Pos                        (12U)
#define TIM_CCMR2_OC4M_Msk                        (0x7UL << TIM_CCMR2_OC4M_Pos)                     /*!< 0x00007000 */
#define TIM_CCMR2_OC4M                            TIM_CCMR2_OC4M_Msk                                /*!< OC4M[14:12] bits (desc OC4M) */
#define TIM_CCMR2_OC4M_0                          (0x1UL << TIM_CCMR2_OC4M_Pos)                     /*!< 0x00001000 */
#define TIM_CCMR2_OC4M_1                          (0x2UL << TIM_CCMR2_OC4M_Pos)                     /*!< 0x00002000 */
#define TIM_CCMR2_OC4M_2                          (0x4UL << TIM_CCMR2_OC4M_Pos)                     /*!< 0x00004000 */

#define TIM_CCMR2_OC4CE_Pos                       (15U)
#define TIM_CCMR2_OC4CE_Msk                       (0x1UL << TIM_CCMR2_OC4CE_Pos)                    /*!< 0x00008000 */
#define TIM_CCMR2_OC4CE                           TIM_CCMR2_OC4CE_Msk                               /*!< desc OC4CE */

/*!< TIM_CCMR2_INPUT */
#define TIM_CCMR2_IC3PSC_Pos                      (2U)
#define TIM_CCMR2_IC3PSC_Msk                      (0x3UL << TIM_CCMR2_IC3PSC_Pos)                   /*!< 0x0000000C */
#define TIM_CCMR2_IC3PSC                          TIM_CCMR2_IC3PSC_Msk                              /*!< IC3PSC[3:2] bits (desc IC3PSC) */
#define TIM_CCMR2_IC3PSC_0                        (0x1UL << TIM_CCMR2_IC3PSC_Pos)                   /*!< 0x00000004 */
#define TIM_CCMR2_IC3PSC_1                        (0x2UL << TIM_CCMR2_IC3PSC_Pos)                   /*!< 0x00000008 */

#define TIM_CCMR2_IC3F_Pos                        (4U)
#define TIM_CCMR2_IC3F_Msk                        (0xFUL << TIM_CCMR2_IC3F_Pos)                     /*!< 0x000000F0 */
#define TIM_CCMR2_IC3F                            TIM_CCMR2_IC3F_Msk                                /*!< IC3F[7:4] bits (desc IC3F) */
#define TIM_CCMR2_IC3F_0                          (0x1UL << TIM_CCMR2_IC3F_Pos)                     /*!< 0x00000010 */
#define TIM_CCMR2_IC3F_1                          (0x2UL << TIM_CCMR2_IC3F_Pos)                     /*!< 0x00000020 */
#define TIM_CCMR2_IC3F_2                          (0x4UL << TIM_CCMR2_IC3F_Pos)                     /*!< 0x00000040 */
#define TIM_CCMR2_IC3F_3                          (0x8UL << TIM_CCMR2_IC3F_Pos)                     /*!< 0x00000080 */

#define TIM_CCMR2_IC4PSC_Pos                      (10U)
#define TIM_CCMR2_IC4PSC_Msk                      (0x3UL << TIM_CCMR2_IC4PSC_Pos)                   /*!< 0x00000C00 */
#define TIM_CCMR2_IC4PSC                          TIM_CCMR2_IC4PSC_Msk                              /*!< IC4PSC[11:10] bits (desc IC4PSC) */
#define TIM_CCMR2_IC4PSC_0                        (0x1UL << TIM_CCMR2_IC4PSC_Pos)                   /*!< 0x00000400 */
#define TIM_CCMR2_IC4PSC_1                        (0x2UL << TIM_CCMR2_IC4PSC_Pos)                   /*!< 0x00000800 */

#define TIM_CCMR2_IC4F_Pos                        (12U)
#define TIM_CCMR2_IC4F_Msk                        (0xFUL << TIM_CCMR2_IC4F_Pos)                     /*!< 0x0000F000 */
#define TIM_CCMR2_IC4F                            TIM_CCMR2_IC4F_Msk                                /*!< IC4F[15:12] bits (desc IC4F) */
#define TIM_CCMR2_IC4F_0                          (0x1UL << TIM_CCMR2_IC4F_Pos)                     /*!< 0x00001000 */
#define TIM_CCMR2_IC4F_1                          (0x2UL << TIM_CCMR2_IC4F_Pos)                     /*!< 0x00002000 */
#define TIM_CCMR2_IC4F_2                          (0x4UL << TIM_CCMR2_IC4F_Pos)                     /*!< 0x00004000 */
#define TIM_CCMR2_IC4F_3                          (0x8UL << TIM_CCMR2_IC4F_Pos)                     /*!< 0x00008000 */


/*!< TIM_CCER */
#define TIM_CCER_CC1E_Pos                         (0U)
#define TIM_CCER_CC1E_Msk                         (0x1UL << TIM_CCER_CC1E_Pos)                      /*!< 0x00000001 */
#define TIM_CCER_CC1E                             TIM_CCER_CC1E_Msk                                 /*!< desc CC1E */
#define TIM_CCER_CC1P_Pos                         (1U)
#define TIM_CCER_CC1P_Msk                         (0x1UL << TIM_CCER_CC1P_Pos)                      /*!< 0x00000002 */
#define TIM_CCER_CC1P                             TIM_CCER_CC1P_Msk                                 /*!< desc CC1P */
#define TIM_CCER_CC1NE_Pos                        (2U)
#define TIM_CCER_CC1NE_Msk                        (0x1UL << TIM_CCER_CC1NE_Pos)                     /*!< 0x00000004 */
#define TIM_CCER_CC1NE                            TIM_CCER_CC1NE_Msk                                /*!< desc CC1NE */
#define TIM_CCER_CC1NP_Pos                        (3U)
#define TIM_CCER_CC1NP_Msk                        (0x1UL << TIM_CCER_CC1NP_Pos)                     /*!< 0x00000008 */
#define TIM_CCER_CC1NP                            TIM_CCER_CC1NP_Msk                                /*!< desc CC1NP */
#define TIM_CCER_CC2E_Pos                         (4U)
#define TIM_CCER_CC2E_Msk                         (0x1UL << TIM_CCER_CC2E_Pos)                      /*!< 0x00000010 */
#define TIM_CCER_CC2E                             TIM_CCER_CC2E_Msk                                 /*!< desc CC2E */
#define TIM_CCER_CC2P_Pos                         (5U)
#define TIM_CCER_CC2P_Msk                         (0x1UL << TIM_CCER_CC2P_Pos)                      /*!< 0x00000020 */
#define TIM_CCER_CC2P                             TIM_CCER_CC2P_Msk                                 /*!< desc CC2P */
#define TIM_CCER_CC2NE_Pos                        (6U)
#define TIM_CCER_CC2NE_Msk                        (0x1UL << TIM_CCER_CC2NE_Pos)                     /*!< 0x00000040 */
#define TIM_CCER_CC2NE                            TIM_CCER_CC2NE_Msk                                /*!< desc CC2NE */
#define TIM_CCER_CC2NP_Pos                        (7U)
#define TIM_CCER_CC2NP_Msk                        (0x1UL << TIM_CCER_CC2NP_Pos)                     /*!< 0x00000080 */
#define TIM_CCER_CC2NP                            TIM_CCER_CC2NP_Msk                                /*!< desc CC2NP */
#define TIM_CCER_CC3E_Pos                         (8U)
#define TIM_CCER_CC3E_Msk                         (0x1UL << TIM_CCER_CC3E_Pos)                      /*!< 0x00000100 */
#define TIM_CCER_CC3E                             TIM_CCER_CC3E_Msk                                 /*!< desc CC3E */
#define TIM_CCER_CC3P_Pos                         (9U)
#define TIM_CCER_CC3P_Msk                         (0x1UL << TIM_CCER_CC3P_Pos)                      /*!< 0x00000200 */
#define TIM_CCER_CC3P                             TIM_CCER_CC3P_Msk                                 /*!< desc CC3P */
#define TIM_CCER_CC3NE_Pos                        (10U)
#define TIM_CCER_CC3NE_Msk                        (0x1UL << TIM_CCER_CC3NE_Pos)                     /*!< 0x00000400 */
#define TIM_CCER_CC3NE                            TIM_CCER_CC3NE_Msk                                /*!< desc CC3NE */
#define TIM_CCER_CC3NP_Pos                        (11U)
#define TIM_CCER_CC3NP_Msk                        (0x1UL << TIM_CCER_CC3NP_Pos)                     /*!< 0x00000800 */
#define TIM_CCER_CC3NP                            TIM_CCER_CC3NP_Msk                                /*!< desc CC3NP */
#define TIM_CCER_CC4E_Pos                         (12U)
#define TIM_CCER_CC4E_Msk                         (0x1UL << TIM_CCER_CC4E_Pos)                      /*!< 0x00001000 */
#define TIM_CCER_CC4E                             TIM_CCER_CC4E_Msk                                 /*!< desc CC4E */
#define TIM_CCER_CC4P_Pos                         (13U)
#define TIM_CCER_CC4P_Msk                         (0x1UL << TIM_CCER_CC4P_Pos)                      /*!< 0x00002000 */
#define TIM_CCER_CC4P                             TIM_CCER_CC4P_Msk                                 /*!< desc CC4P */

/*!< TIM_CNT */
#define TIM_CNT_CNT_Pos                           (0U)
#define TIM_CNT_CNT_Msk                           (0xFFFFUL << TIM_CNT_CNT_Pos)                     /*!< 0x0000FFFF */
#define TIM_CNT_CNT                               TIM_CNT_CNT_Msk                                   /*!< CNT[15:0] bits (desc CNT) */

/*!< TIM_PSC */
#define TIM_PSC_PSC_Pos                           (0U)
#define TIM_PSC_PSC_Msk                           (0xFFFFUL << TIM_PSC_PSC_Pos)                     /*!< 0x0000FFFF */
#define TIM_PSC_PSC                               TIM_PSC_PSC_Msk                                   /*!< PSC[15:0] bits (desc PSC) */

/*!< TIM_ARR */
#define TIM_ARR_ARR_Pos                           (0U)
#define TIM_ARR_ARR_Msk                           (0xFFFFUL << TIM_ARR_ARR_Pos)                     /*!< 0x0000FFFF */
#define TIM_ARR_ARR                               TIM_ARR_ARR_Msk                                   /*!< ARR[15:0] bits (desc ARR) */

/*!< TIM_RCR */
#define TIM_RCR_REP_Pos                           (0U)
#define TIM_RCR_REP_Msk                           (0xFFUL << TIM_RCR_REP_Pos)                       /*!< 0x000000FF */
#define TIM_RCR_REP                               TIM_RCR_REP_Msk                                   /*!< REP[7:0] bits (desc REP) */

/*!< TIM_CCR1 */
#define TIM_CCR1_CCR1_Pos                         (0U)
#define TIM_CCR1_CCR1_Msk                         (0xFFFFUL << TIM_CCR1_CCR1_Pos)                   /*!< 0x0000FFFF */
#define TIM_CCR1_CCR1                             TIM_CCR1_CCR1_Msk                                 /*!< CCR1[15:0] bits (desc CCR1) */

/*!< TIM_CCR2 */
#define TIM_CCR2_CCR2_Pos                         (0U)
#define TIM_CCR2_CCR2_Msk                         (0xFFFFUL << TIM_CCR2_CCR2_Pos)                   /*!< 0x0000FFFF */
#define TIM_CCR2_CCR2                             TIM_CCR2_CCR2_Msk                                 /*!< CCR2[15:0] bits (desc CCR2) */

/*!< TIM_CCR3 */
#define TIM_CCR3_CCR3_Pos                         (0U)
#define TIM_CCR3_CCR3_Msk                         (0xFFFFUL << TIM_CCR3_CCR3_Pos)                   /*!< 0x0000FFFF */
#define TIM_CCR3_CCR3                             TIM_CCR3_CCR3_Msk                                 /*!< CCR3[15:0] bits (desc CCR3) */

/*!< TIM_CCR4 */
#define TIM_CCR4_CCR4_Pos                         (0U)
#define TIM_CCR4_CCR4_Msk                         (0xFFFFUL << TIM_CCR4_CCR4_Pos)                   /*!< 0x0000FFFF */
#define TIM_CCR4_CCR4                             TIM_CCR4_CCR4_Msk                                 /*!< CCR4[15:0] bits (desc CCR4) */

/*!< TIM_BDTR */
#define TIM_BDTR_DTG_Pos                          (0U)
#define TIM_BDTR_DTG_Msk                          (0xFFUL << TIM_BDTR_DTG_Pos)                      /*!< 0x000000FF */
#define TIM_BDTR_DTG                              TIM_BDTR_DTG_Msk                                  /*!< DTG[7:0] bits (desc DTG) */
#define TIM_BDTR_DTG_0                            (0x1UL << TIM_BDTR_DTG_Pos)                       /*!< 0x00000001 */
#define TIM_BDTR_DTG_1                            (0x2UL << TIM_BDTR_DTG_Pos)                       /*!< 0x00000002 */
#define TIM_BDTR_DTG_2                            (0x4UL << TIM_BDTR_DTG_Pos)                       /*!< 0x00000004 */
#define TIM_BDTR_DTG_3                            (0x8UL << TIM_BDTR_DTG_Pos)                       /*!< 0x00000008 */
#define TIM_BDTR_DTG_4                            (0x10UL << TIM_BDTR_DTG_Pos)                      /*!< 0x00000010 */
#define TIM_BDTR_DTG_5                            (0x20UL << TIM_BDTR_DTG_Pos)                      /*!< 0x00000020 */
#define TIM_BDTR_DTG_6                            (0x40UL << TIM_BDTR_DTG_Pos)                      /*!< 0x00000040 */
#define TIM_BDTR_DTG_7                            (0x80UL << TIM_BDTR_DTG_Pos)                      /*!< 0x00000080 */

#define TIM_BDTR_LOCK_Pos                         (8U)
#define TIM_BDTR_LOCK_Msk                         (0x3UL << TIM_BDTR_LOCK_Pos)                      /*!< 0x00000300 */
#define TIM_BDTR_LOCK                             TIM_BDTR_LOCK_Msk                                 /*!< LOCK[9:8] bits (desc LOCK) */
#define TIM_BDTR_LOCK_0                           (0x1UL << TIM_BDTR_LOCK_Pos)                      /*!< 0x00000100 */
#define TIM_BDTR_LOCK_1                           (0x2UL << TIM_BDTR_LOCK_Pos)                      /*!< 0x00000200 */

#define TIM_BDTR_OSSI_Pos                         (10U)
#define TIM_BDTR_OSSI_Msk                         (0x1UL << TIM_BDTR_OSSI_Pos)                      /*!< 0x00000400 */
#define TIM_BDTR_OSSI                             TIM_BDTR_OSSI_Msk                                 /*!< desc OSSI */
#define TIM_BDTR_OSSR_Pos                         (11U)
#define TIM_BDTR_OSSR_Msk                         (0x1UL << TIM_BDTR_OSSR_Pos)                      /*!< 0x00000800 */
#define TIM_BDTR_OSSR                             TIM_BDTR_OSSR_Msk                                 /*!< desc OSSR */
#define TIM_BDTR_BKE_Pos                          (12U)
#define TIM_BDTR_BKE_Msk                          (0x1UL << TIM_BDTR_BKE_Pos)                       /*!< 0x00001000 */
#define TIM_BDTR_BKE                              TIM_BDTR_BKE_Msk                                  /*!< desc BKE */
#define TIM_BDTR_BKP_Pos                          (13U)
#define TIM_BDTR_BKP_Msk                          (0x1UL << TIM_BDTR_BKP_Pos)                       /*!< 0x00002000 */
#define TIM_BDTR_BKP                              TIM_BDTR_BKP_Msk                                  /*!< desc BKP */
#define TIM_BDTR_AOE_Pos                          (14U)
#define TIM_BDTR_AOE_Msk                          (0x1UL << TIM_BDTR_AOE_Pos)                       /*!< 0x00004000 */
#define TIM_BDTR_AOE                              TIM_BDTR_AOE_Msk                                  /*!< desc AOE */
#define TIM_BDTR_MOE_Pos                          (15U)
#define TIM_BDTR_MOE_Msk                          (0x1UL << TIM_BDTR_MOE_Pos)                       /*!< 0x00008000 */
#define TIM_BDTR_MOE                              TIM_BDTR_MOE_Msk                                  /*!< desc MOE */

/*!< TIM_DCR */
#define TIM_DCR_DBA_Pos                           (0U)
#define TIM_DCR_DBA_Msk                           (0x1FUL << TIM_DCR_DBA_Pos)                       /*!< 0x0000001F */
#define TIM_DCR_DBA                               TIM_DCR_DBA_Msk                                   /*!< DBA[4:0] bits (desc DBA) */
#define TIM_DCR_DBA_0                             (0x1UL << TIM_DCR_DBA_Pos)                        /*!< 0x00000001 */
#define TIM_DCR_DBA_1                             (0x2UL << TIM_DCR_DBA_Pos)                        /*!< 0x00000002 */
#define TIM_DCR_DBA_2                             (0x4UL << TIM_DCR_DBA_Pos)                        /*!< 0x00000004 */
#define TIM_DCR_DBA_3                             (0x8UL << TIM_DCR_DBA_Pos)                        /*!< 0x00000008 */
#define TIM_DCR_DBA_4                             (0x10UL << TIM_DCR_DBA_Pos)                       /*!< 0x00000010 */

#define TIM_DCR_DBL_Pos                           (8U)
#define TIM_DCR_DBL_Msk                           (0x1FUL << TIM_DCR_DBL_Pos)                       /*!< 0x00001F00 */
#define TIM_DCR_DBL                               TIM_DCR_DBL_Msk                                   /*!< DBL[12:8] bits (desc DBL) */
#define TIM_DCR_DBL_0                             (0x1UL << TIM_DCR_DBL_Pos)                        /*!< 0x00000100 */
#define TIM_DCR_DBL_1                             (0x2UL << TIM_DCR_DBL_Pos)                        /*!< 0x00000200 */
#define TIM_DCR_DBL_2                             (0x4UL << TIM_DCR_DBL_Pos)                        /*!< 0x00000400 */
#define TIM_DCR_DBL_3                             (0x8UL << TIM_DCR_DBL_Pos)                        /*!< 0x00000800 */
#define TIM_DCR_DBL_4                             (0x10UL << TIM_DCR_DBL_Pos)                       /*!< 0x00001000 */

/*!< TIM_DMAR */
#define TIM_DMAR_DMAB_Pos                         (0U)
#define TIM_DMAR_DMAB_Msk                         (0xFFFFUL << TIM_DMAR_DMAB_Pos)                   /*!< 0x0000FFFF */
#define TIM_DMAR_DMAB                             TIM_DMAR_DMAB_Msk                                 /*!< DMAB[15:0] bits (desc DMAB) */

/*!< TIM_OR */
#define TIM_OR_TI1_RMP_Pos                        (0U)
#define TIM_OR_TI1_RMP_Msk                        (0x3UL << TIM_OR_TI1_RMP_Pos)                     /*!< 0x00000003 */
#define TIM_OR_TI1_RMP                            TIM_OR_TI1_RMP_Msk                                /*!< OR[1:0] bits (desc OR) */
#define TIM_OR_TI1_RMP_0                          (0x1UL << TIM_OR_TI1_RMP_Pos)                     /*!< 0x00000001 */
#define TIM_OR_TI1_RMP_1                          (0x2UL << TIM_OR_TI1_RMP_Pos)                     /*!< 0x00000002 */


/*********************  Bits Define For Peripheral USART  *********************/
/*!< USART_SR */
#define USART_SR_PE_Pos                           (0U)
#define USART_SR_PE_Msk                           (0x1UL << USART_SR_PE_Pos)                        /*!< 0x00000001 */
#define USART_SR_PE                               USART_SR_PE_Msk                                   /*!< desc PE */
#define USART_SR_FE_Pos                           (1U)
#define USART_SR_FE_Msk                           (0x1UL << USART_SR_FE_Pos)                        /*!< 0x00000002 */
#define USART_SR_FE                               USART_SR_FE_Msk                                   /*!< desc FE */
#define USART_SR_NE_Pos                           (2U)
#define USART_SR_NE_Msk                           (0x1UL << USART_SR_NE_Pos)                        /*!< 0x00000004 */
#define USART_SR_NE                               USART_SR_NE_Msk                                   /*!< desc NE */
#define USART_SR_ORE_Pos                          (3U)
#define USART_SR_ORE_Msk                          (0x1UL << USART_SR_ORE_Pos)                       /*!< 0x00000008 */
#define USART_SR_ORE                              USART_SR_ORE_Msk                                  /*!< desc ORE */
#define USART_SR_IDLE_Pos                         (4U)
#define USART_SR_IDLE_Msk                         (0x1UL << USART_SR_IDLE_Pos)                      /*!< 0x00000010 */
#define USART_SR_IDLE                             USART_SR_IDLE_Msk                                 /*!< desc IDLE */
#define USART_SR_RXNE_Pos                         (5U)
#define USART_SR_RXNE_Msk                         (0x1UL << USART_SR_RXNE_Pos)                      /*!< 0x00000020 */
#define USART_SR_RXNE                             USART_SR_RXNE_Msk                                 /*!< desc RXNE */
#define USART_SR_TC_Pos                           (6U)
#define USART_SR_TC_Msk                           (0x1UL << USART_SR_TC_Pos)                        /*!< 0x00000040 */
#define USART_SR_TC                               USART_SR_TC_Msk                                   /*!< desc TC */
#define USART_SR_TXE_Pos                          (7U)
#define USART_SR_TXE_Msk                          (0x1UL << USART_SR_TXE_Pos)                       /*!< 0x00000080 */
#define USART_SR_TXE                              USART_SR_TXE_Msk                                  /*!< desc TXE */
#define USART_SR_LBD_Pos                          (8U)
#define USART_SR_LBD_Msk                          (0x1UL << USART_SR_LBD_Pos)                       /*!< 0x00000100 */
#define USART_SR_LBD                              USART_SR_LBD_Msk                                  /*!< desc LBD */
#define USART_SR_CTS_Pos                          (9U)
#define USART_SR_CTS_Msk                          (0x1UL << USART_SR_CTS_Pos)                       /*!< 0x00000200 */
#define USART_SR_CTS                              USART_SR_CTS_Msk                                  /*!< desc CTS */
#define USART_SR_ABRF_Pos                         (10U)
#define USART_SR_ABRF_Msk                         (0x1UL << USART_SR_ABRF_Pos)                      /*!< 0x00000400 */
#define USART_SR_ABRF                             USART_SR_ABRF_Msk                                 /*!< desc ABRF */
#define USART_SR_ABRE_Pos                         (11U)
#define USART_SR_ABRE_Msk                         (0x1UL << USART_SR_ABRE_Pos)                      /*!< 0x00000800 */
#define USART_SR_ABRE                             USART_SR_ABRE_Msk                                 /*!< desc ABRE */
#define USART_SR_ABRRQ_Pos                        (12U)
#define USART_SR_ABRRQ_Msk                        (0x1UL << USART_SR_ABRRQ_Pos)                     /*!< 0x00001000 */
#define USART_SR_ABRRQ                            USART_SR_ABRRQ_Msk                                /*!< desc ABRRQ */

/*!< USART_DR */
#define USART_DR_DR_Pos                           (0U)
#define USART_DR_DR_Msk                           (0x1FFUL << USART_DR_DR_Pos)                      /*!< 0x000001FF */
#define USART_DR_DR                               USART_DR_DR_Msk                                   /*!< DR[8:0] bits (desc DR) */

/*!< USART_BRR */
#define USART_BRR_DIV_FRACTION_Pos                (0U)
#define USART_BRR_DIV_FRACTION_Msk                (0xFUL << USART_BRR_DIV_FRACTION_Pos)             /*!< 0x0000000F */
#define USART_BRR_DIV_FRACTION                    USART_BRR_DIV_FRACTION_Msk                        /*!< DIV_Fraction[3:0] bits (desc DIV_Fraction) */
#define USART_BRR_DIV_FRACTION_0                  (0x1UL << USART_BRR_DIV_FRACTION_Pos)             /*!< 0x00000001 */
#define USART_BRR_DIV_FRACTION_1                  (0x2UL << USART_BRR_DIV_FRACTION_Pos)             /*!< 0x00000002 */
#define USART_BRR_DIV_FRACTION_2                  (0x4UL << USART_BRR_DIV_FRACTION_Pos)             /*!< 0x00000004 */
#define USART_BRR_DIV_FRACTION_3                  (0x8UL << USART_BRR_DIV_FRACTION_Pos)             /*!< 0x00000008 */

#define USART_BRR_DIV_MANTISSA_Pos                (4U)
#define USART_BRR_DIV_MANTISSA_Msk                (0xFFFUL << USART_BRR_DIV_MANTISSA_Pos)           /*!< 0x0000FFF0 */
#define USART_BRR_DIV_MANTISSA                    USART_BRR_DIV_MANTISSA_Msk                        /*!< DIV_Mantissa[15:4] bits (desc DIV_Mantissa) */

/*!< USART_CR1 */
#define USART_CR1_SBK_Pos                         (0U)
#define USART_CR1_SBK_Msk                         (0x1UL << USART_CR1_SBK_Pos)                      /*!< 0x00000001 */
#define USART_CR1_SBK                             USART_CR1_SBK_Msk                                 /*!< desc SBK */
#define USART_CR1_RWU_Pos                         (1U)
#define USART_CR1_RWU_Msk                         (0x1UL << USART_CR1_RWU_Pos)                      /*!< 0x00000002 */
#define USART_CR1_RWU                             USART_CR1_RWU_Msk                                 /*!< desc RWU */
#define USART_CR1_RE_Pos                          (2U)
#define USART_CR1_RE_Msk                          (0x1UL << USART_CR1_RE_Pos)                       /*!< 0x00000004 */
#define USART_CR1_RE                              USART_CR1_RE_Msk                                  /*!< desc RE */
#define USART_CR1_TE_Pos                          (3U)
#define USART_CR1_TE_Msk                          (0x1UL << USART_CR1_TE_Pos)                       /*!< 0x00000008 */
#define USART_CR1_TE                              USART_CR1_TE_Msk                                  /*!< desc TE */
#define USART_CR1_IDLEIE_Pos                      (4U)
#define USART_CR1_IDLEIE_Msk                      (0x1UL << USART_CR1_IDLEIE_Pos)                   /*!< 0x00000010 */
#define USART_CR1_IDLEIE                          USART_CR1_IDLEIE_Msk                              /*!< desc IDLEIE */
#define USART_CR1_RXNEIE_Pos                      (5U)
#define USART_CR1_RXNEIE_Msk                      (0x1UL << USART_CR1_RXNEIE_Pos)                   /*!< 0x00000020 */
#define USART_CR1_RXNEIE                          USART_CR1_RXNEIE_Msk                              /*!< desc RXNEIE */
#define USART_CR1_TCIE_Pos                        (6U)
#define USART_CR1_TCIE_Msk                        (0x1UL << USART_CR1_TCIE_Pos)                     /*!< 0x00000040 */
#define USART_CR1_TCIE                            USART_CR1_TCIE_Msk                                /*!< desc TCIE */
#define USART_CR1_TXEIE_Pos                       (7U)
#define USART_CR1_TXEIE_Msk                       (0x1UL << USART_CR1_TXEIE_Pos)                    /*!< 0x00000080 */
#define USART_CR1_TXEIE                           USART_CR1_TXEIE_Msk                               /*!< desc TXEIE */
#define USART_CR1_PEIE_Pos                        (8U)
#define USART_CR1_PEIE_Msk                        (0x1UL << USART_CR1_PEIE_Pos)                     /*!< 0x00000100 */
#define USART_CR1_PEIE                            USART_CR1_PEIE_Msk                                /*!< desc PEIE */
#define USART_CR1_PS_Pos                          (9U)
#define USART_CR1_PS_Msk                          (0x1UL << USART_CR1_PS_Pos)                       /*!< 0x00000200 */
#define USART_CR1_PS                              USART_CR1_PS_Msk                                  /*!< desc PS */
#define USART_CR1_PCE_Pos                         (10U)
#define USART_CR1_PCE_Msk                         (0x1UL << USART_CR1_PCE_Pos)                      /*!< 0x00000400 */
#define USART_CR1_PCE                             USART_CR1_PCE_Msk                                 /*!< desc PCE */
#define USART_CR1_WAKE_Pos                        (11U)
#define USART_CR1_WAKE_Msk                        (0x1UL << USART_CR1_WAKE_Pos)                     /*!< 0x00000800 */
#define USART_CR1_WAKE                            USART_CR1_WAKE_Msk                                /*!< desc WAKE */
#define USART_CR1_M_Pos                           (12U)
#define USART_CR1_M_Msk                           (0x1UL << USART_CR1_M_Pos)                        /*!< 0x00001000 */
#define USART_CR1_M                               USART_CR1_M_Msk                                   /*!< desc M */
#define USART_CR1_UE_Pos                          (13U)
#define USART_CR1_UE_Msk                          (0x1UL << USART_CR1_UE_Pos)                       /*!< 0x00002000 */
#define USART_CR1_UE                              USART_CR1_UE_Msk                                  /*!< desc UE */

/*!< USART_CR2 */
#define USART_CR2_ADD_Pos                         (0U)
#define USART_CR2_ADD_Msk                         (0xFUL << USART_CR2_ADD_Pos)                      /*!< 0x0000000F */
#define USART_CR2_ADD                             USART_CR2_ADD_Msk                                 /*!< ADD[3:0] bits (desc ADD) */
#define USART_CR2_ADD_0                           (0x1UL << USART_CR2_ADD_Pos)                      /*!< 0x00000001 */
#define USART_CR2_ADD_1                           (0x2UL << USART_CR2_ADD_Pos)                      /*!< 0x00000002 */
#define USART_CR2_ADD_2                           (0x4UL << USART_CR2_ADD_Pos)                      /*!< 0x00000004 */
#define USART_CR2_ADD_3                           (0x8UL << USART_CR2_ADD_Pos)                      /*!< 0x00000008 */

#define USART_CR2_LBDL_Pos                        (5U)
#define USART_CR2_LBDL_Msk                        (0x1UL << USART_CR2_LBDL_Pos)                     /*!< 0x00000020 */
#define USART_CR2_LBDL                            USART_CR2_LBDL_Msk                                /*!< desc LBDL */
#define USART_CR2_LBDIE_Pos                       (6U)
#define USART_CR2_LBDIE_Msk                       (0x1UL << USART_CR2_LBDIE_Pos)                    /*!< 0x00000040 */
#define USART_CR2_LBDIE                           USART_CR2_LBDIE_Msk                               /*!< desc LBDIE */
#define USART_CR2_LBCL_Pos                        (8U)
#define USART_CR2_LBCL_Msk                        (0x1UL << USART_CR2_LBCL_Pos)                     /*!< 0x00000100 */
#define USART_CR2_LBCL                            USART_CR2_LBCL_Msk                                /*!< desc LBCL */
#define USART_CR2_CPHA_Pos                        (9U)
#define USART_CR2_CPHA_Msk                        (0x1UL << USART_CR2_CPHA_Pos)                     /*!< 0x00000200 */
#define USART_CR2_CPHA                            USART_CR2_CPHA_Msk                                /*!< desc CPHA */
#define USART_CR2_CPOL_Pos                        (10U)
#define USART_CR2_CPOL_Msk                        (0x1UL << USART_CR2_CPOL_Pos)                     /*!< 0x00000400 */
#define USART_CR2_CPOL                            USART_CR2_CPOL_Msk                                /*!< desc CPOL */
#define USART_CR2_CLKEN_Pos                       (11U)
#define USART_CR2_CLKEN_Msk                       (0x1UL << USART_CR2_CLKEN_Pos)                    /*!< 0x00000800 */
#define USART_CR2_CLKEN                           USART_CR2_CLKEN_Msk                               /*!< desc CLKEN */
#define USART_CR2_STOP_Pos                        (12U)
#define USART_CR2_STOP_Msk                        (0x3UL << USART_CR2_STOP_Pos)                     /*!< 0x00003000 */
#define USART_CR2_STOP                            USART_CR2_STOP_Msk                                /*!< STOP[13:12] bits (desc STOP) */
#define USART_CR2_STOP_0                          (0x1UL << USART_CR2_STOP_Pos)                     /*!< 0x00001000 */
#define USART_CR2_STOP_1                          (0x2UL << USART_CR2_STOP_Pos)                     /*!< 0x00002000 */

#define USART_CR2_LINEN_Pos                       (14U)
#define USART_CR2_LINEN_Msk                       (0x1UL << USART_CR2_LINEN_Pos)                    /*!< 0x00004000 */
#define USART_CR2_LINEN                           USART_CR2_LINEN_Msk                               /*!< desc LINEN */

/*!< USART_CR3 */
#define USART_CR3_EIE_Pos                         (0U)
#define USART_CR3_EIE_Msk                         (0x1UL << USART_CR3_EIE_Pos)                      /*!< 0x00000001 */
#define USART_CR3_EIE                             USART_CR3_EIE_Msk                                 /*!< desc EIE */
#define USART_CR3_IREN_Pos                        (1U)
#define USART_CR3_IREN_Msk                        (0x1UL << USART_CR3_IREN_Pos)                     /*!< 0x00000002 */
#define USART_CR3_IREN                            USART_CR3_IREN_Msk                                /*!< desc IREN */
#define USART_CR3_IRLP_Pos                        (2U)
#define USART_CR3_IRLP_Msk                        (0x1UL << USART_CR3_IRLP_Pos)                     /*!< 0x00000004 */
#define USART_CR3_IRLP                            USART_CR3_IRLP_Msk                                /*!< desc IRLP */
#define USART_CR3_HDSEL_Pos                       (3U)
#define USART_CR3_HDSEL_Msk                       (0x1UL << USART_CR3_HDSEL_Pos)                    /*!< 0x00000008 */
#define USART_CR3_HDSEL                           USART_CR3_HDSEL_Msk                               /*!< desc HDSEL */
#define USART_CR3_NACK_Pos                        (4U)
#define USART_CR3_NACK_Msk                        (0x1UL << USART_CR3_NACK_Pos)                     /*!< 0x00000010 */
#define USART_CR3_NACK                            USART_CR3_NACK_Msk                                /*!< desc NACK */
#define USART_CR3_SCEN_Pos                        (5U)
#define USART_CR3_SCEN_Msk                        (0x1UL << USART_CR3_SCEN_Pos)                     /*!< 0x00000020 */
#define USART_CR3_SCEN                            USART_CR3_SCEN_Msk                                /*!< desc SCEN */
#define USART_CR3_DMAR_Pos                        (6U)
#define USART_CR3_DMAR_Msk                        (0x1UL << USART_CR3_DMAR_Pos)                     /*!< 0x00000040 */
#define USART_CR3_DMAR                            USART_CR3_DMAR_Msk                                /*!< desc DMAR */
#define USART_CR3_DMAT_Pos                        (7U)
#define USART_CR3_DMAT_Msk                        (0x1UL << USART_CR3_DMAT_Pos)                     /*!< 0x00000080 */
#define USART_CR3_DMAT                            USART_CR3_DMAT_Msk                                /*!< desc DMAT */
#define USART_CR3_RTSE_Pos                        (8U)
#define USART_CR3_RTSE_Msk                        (0x1UL << USART_CR3_RTSE_Pos)                     /*!< 0x00000100 */
#define USART_CR3_RTSE                            USART_CR3_RTSE_Msk                                /*!< desc RTSE */
#define USART_CR3_CTSE_Pos                        (9U)
#define USART_CR3_CTSE_Msk                        (0x1UL << USART_CR3_CTSE_Pos)                     /*!< 0x00000200 */
#define USART_CR3_CTSE                            USART_CR3_CTSE_Msk                                /*!< desc CTSE */
#define USART_CR3_CTSIE_Pos                       (10U)
#define USART_CR3_CTSIE_Msk                       (0x1UL << USART_CR3_CTSIE_Pos)                    /*!< 0x00000400 */
#define USART_CR3_CTSIE                           USART_CR3_CTSIE_Msk                               /*!< desc CTSIE */
#define USART_CR3_OVER8_Pos                       (11U)
#define USART_CR3_OVER8_Msk                       (0x1UL << USART_CR3_OVER8_Pos)                    /*!< 0x00000800 */
#define USART_CR3_OVER8                           USART_CR3_OVER8_Msk                               /*!< desc OVER8 */
#define USART_CR3_ABREN_Pos                       (12U)
#define USART_CR3_ABREN_Msk                       (0x1UL << USART_CR3_ABREN_Pos)                    /*!< 0x00001000 */
#define USART_CR3_ABREN                           USART_CR3_ABREN_Msk                               /*!< desc ABREN */
#define USART_CR3_ABRMOD_Pos                      (13U)
#define USART_CR3_ABRMOD_Msk                      (0x3UL << USART_CR3_ABRMOD_Pos)                   /*!< 0x00006000 */
#define USART_CR3_ABRMOD                          USART_CR3_ABRMOD_Msk                              /*!< ABRMOD[14:13] bits (desc ABRMOD) */
#define USART_CR3_ABRMOD_0                        (0x1UL << USART_CR3_ABRMOD_Pos)                   /*!< 0x00002000 */
#define USART_CR3_ABRMOD_1                        (0x2UL << USART_CR3_ABRMOD_Pos)                   /*!< 0x00004000 */


/*!< USART_GTPR */
#define USART_GTPR_PSC_Pos                        (0U)
#define USART_GTPR_PSC_Msk                        (0xFFUL << USART_GTPR_PSC_Pos)                    /*!< 0x000000FF */
#define USART_GTPR_PSC                            USART_GTPR_PSC_Msk                                /*!< PSC[7:0] bits (desc PSC) */
#define USART_GTPR_GT_Pos                         (8U)
#define USART_GTPR_GT_Msk                         (0xFFUL << USART_GTPR_GT_Pos)                     /*!< 0x0000FF00 */
#define USART_GTPR_GT                             USART_GTPR_GT_Msk                                 /*!< GT[15:8] bits (desc GT) */

/*********************  Bits Define For Peripheral WWDG  *********************/
/*!< WWDG_CR */
#define WWDG_CR_T_Pos                             (0U)
#define WWDG_CR_T_Msk                             (0x7FUL << WWDG_CR_T_Pos)                         /*!< 0x0000007F */
#define WWDG_CR_T                                 WWDG_CR_T_Msk                                     /*!< T[6:0] bits (desc T) */
#define WWDG_CR_T_0                               (0x1UL << WWDG_CR_T_Pos)                          /*!< 0x00000001 */
#define WWDG_CR_T_1                               (0x2UL << WWDG_CR_T_Pos)                          /*!< 0x00000002 */
#define WWDG_CR_T_2                               (0x4UL << WWDG_CR_T_Pos)                          /*!< 0x00000004 */
#define WWDG_CR_T_3                               (0x8UL << WWDG_CR_T_Pos)                          /*!< 0x00000008 */
#define WWDG_CR_T_4                               (0x10UL << WWDG_CR_T_Pos)                         /*!< 0x00000010 */
#define WWDG_CR_T_5                               (0x20UL << WWDG_CR_T_Pos)                         /*!< 0x00000020 */
#define WWDG_CR_T_6                               (0x40UL << WWDG_CR_T_Pos)                         /*!< 0x00000040 */

#define WWDG_CR_WDGA_Pos                          (7U)
#define WWDG_CR_WDGA_Msk                          (0x1UL << WWDG_CR_WDGA_Pos)                       /*!< 0x00000080 */
#define WWDG_CR_WDGA                              WWDG_CR_WDGA_Msk                                  /*!< desc WDGA */

/*!< WWDG_CFR */
#define WWDG_CFR_W_Pos                            (0U)
#define WWDG_CFR_W_Msk                            (0x7FUL << WWDG_CFR_W_Pos)                        /*!< 0x0000007F */
#define WWDG_CFR_W                                WWDG_CFR_W_Msk                                    /*!< W[6:0] bits (desc W) */
#define WWDG_CFR_W_0                              (0x1UL << WWDG_CFR_W_Pos)                         /*!< 0x00000001 */
#define WWDG_CFR_W_1                              (0x2UL << WWDG_CFR_W_Pos)                         /*!< 0x00000002 */
#define WWDG_CFR_W_2                              (0x4UL << WWDG_CFR_W_Pos)                         /*!< 0x00000004 */
#define WWDG_CFR_W_3                              (0x8UL << WWDG_CFR_W_Pos)                         /*!< 0x00000008 */
#define WWDG_CFR_W_4                              (0x10UL << WWDG_CFR_W_Pos)                        /*!< 0x00000010 */
#define WWDG_CFR_W_5                              (0x20UL << WWDG_CFR_W_Pos)                        /*!< 0x00000020 */
#define WWDG_CFR_W_6                              (0x40UL << WWDG_CFR_W_Pos)                        /*!< 0x00000040 */

#define WWDG_CFR_WDGTB_Pos                        (7U)
#define WWDG_CFR_WDGTB_Msk                        (0x3UL << WWDG_CFR_WDGTB_Pos)                     /*!< 0x00000180 */
#define WWDG_CFR_WDGTB                            WWDG_CFR_WDGTB_Msk                                /*!< WDGTB[8:7] bits (desc WDGTB) */
#define WWDG_CFR_WDGTB_0                          (0x1UL << WWDG_CFR_WDGTB_Pos)                     /*!< 0x00000080 */
#define WWDG_CFR_WDGTB_1                          (0x2UL << WWDG_CFR_WDGTB_Pos)                     /*!< 0x00000100 */

#define WWDG_CFR_EWI_Pos                          (9U)
#define WWDG_CFR_EWI_Msk                          (0x1UL << WWDG_CFR_EWI_Pos)                       /*!< 0x00000200 */
#define WWDG_CFR_EWI                              WWDG_CFR_EWI_Msk                                  /*!< desc EWI */

/*!< WWDG_SR */
#define WWDG_SR_EWIF_Pos                          (0U)
#define WWDG_SR_EWIF_Msk                          (0x1UL << WWDG_SR_EWIF_Pos)                       /*!< 0x00000001 */
#define WWDG_SR_EWIF                              WWDG_SR_EWIF_Msk                                  /*!< desc EWIF */


/****************************** ADC Instances *********************************/
#define IS_ADC_ALL_INSTANCE(INSTANCE) (((INSTANCE) == ADC1) || \
                                       ((INSTANCE) == ADC2) || \
                                       ((INSTANCE) == ADC3))

#define IS_ADC_COMMON_INSTANCE(INSTANCE) ((INSTANCE) == ADC12_COMMON)

#define IS_ADC_MULTIMODE_MASTER_INSTANCE(INSTANCE) ((INSTANCE) == ADC1)

#define IS_ADC_DMA_CAPABILITY_INSTANCE(INSTANCE) (((INSTANCE) == ADC1) || \
                                                  ((INSTANCE) == ADC3))

/****************************** CANFD Instances *******************************/
#define IS_CANFD_ALL_INSTANCE(INSTANCE) ((INSTANCE) == CANFD)

/****************************** CRC Instances *********************************/
#define IS_CRC_ALL_INSTANCE(INSTANCE) ((INSTANCE) == CRC)

/****************************** CRC Instances *********************************/
#define IS_CTC_ALL_INSTANCE(INSTANCE) ((INSTANCE) == CTC)

/****************************** DMA Instances *********************************/
#define IS_DMA_ALL_INSTANCE(INSTANCE) (((INSTANCE) == DMA1_Channel1) || \
                                       ((INSTANCE) == DMA1_Channel2) || \
                                       ((INSTANCE) == DMA1_Channel3) || \
                                       ((INSTANCE) == DMA1_Channel4) || \
                                       ((INSTANCE) == DMA1_Channel5) || \
                                       ((INSTANCE) == DMA1_Channel6) || \
                                       ((INSTANCE) == DMA1_Channel7) || \
                                       ((INSTANCE) == DMA2_Channel1) || \
                                       ((INSTANCE) == DMA2_Channel2) || \
                                       ((INSTANCE) == DMA2_Channel3) || \
                                       ((INSTANCE) == DMA2_Channel4) || \
                                       ((INSTANCE) == DMA2_Channel5))

/****************************** ESMC Instances ********************************/
#define IS_ESMC_ALL_INSTANCE(INSTANCE) ((INSTANCE) == ESMC)

/******************************* GPIO Instances *******************************/
#define IS_GPIO_ALL_INSTANCE(INSTANCE) (((INSTANCE) == GPIOA) || \
                                        ((INSTANCE) == GPIOB) || \
                                        ((INSTANCE) == GPIOC) || \
                                        ((INSTANCE) == GPIOD) || \
                                        ((INSTANCE) == GPIOE))

/**************************** GPIO Alternate Function Instances ***************/
#define IS_GPIO_AF_INSTANCE(INSTANCE) IS_GPIO_ALL_INSTANCE(INSTANCE)

/**************************** GPIO Lock Instances *****************************/
#define IS_GPIO_LOCK_INSTANCE(INSTANCE) IS_GPIO_ALL_INSTANCE(INSTANCE)

/******************************** I2C Instances *******************************/
#define IS_I2C_ALL_INSTANCE(INSTANCE) (((INSTANCE) == I2C1) || \
                                       ((INSTANCE) == I2C2))

/******************************* SMBUS Instances ******************************/
#define IS_SMBUS_ALL_INSTANCE         IS_I2C_ALL_INSTANCE

/******************************** I2S Instances *******************************/
#define IS_I2S_ALL_INSTANCE(INSTANCE) (((INSTANCE) == SPI1) || \
                                       ((INSTANCE) == SPI2) || \
                                       ((INSTANCE) == SPI3))

/****************************** IWDG Instances ********************************/
#define IS_IWDG_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == IWDG)

/******************************** SPI Instances *******************************/
#define IS_SPI_ALL_INSTANCE(INSTANCE) (((INSTANCE) == SPI1) || \
                                       ((INSTANCE) == SPI2) || \
                                       ((INSTANCE) == SPI3))

/****************************** START TIM Instances ***************************/
/****************************** TIM Instances *********************************/
#define IS_TIM_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM6)    || \
   ((INSTANCE) == TIM7)    || \
   ((INSTANCE) == TIM8)    || \
   ((INSTANCE) == TIM9)    || \
   ((INSTANCE) == TIM10)   || \
   ((INSTANCE) == TIM11)   || \
   ((INSTANCE) == TIM12)   || \
   ((INSTANCE) == TIM13)   || \
   ((INSTANCE) == TIM14))

#define IS_TIM_ADVANCED_INSTANCE(INSTANCE)  (((INSTANCE) == TIM1) || ((INSTANCE) == TIM8))

#define IS_TIM_CC1_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM6)    || \
   ((INSTANCE) == TIM7)    || \
   ((INSTANCE) == TIM8)    || \
   ((INSTANCE) == TIM9)    || \
   ((INSTANCE) == TIM10)   || \
   ((INSTANCE) == TIM11)   || \
   ((INSTANCE) == TIM12)   || \
   ((INSTANCE) == TIM13)   || \
   ((INSTANCE) == TIM14))

#define IS_TIM_CC2_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM9)    || \
   ((INSTANCE) == TIM8)    || \
   ((INSTANCE) == TIM12))

#define IS_TIM_CC3_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8))

#define IS_TIM_CC4_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8))

#define IS_TIM_CLOCKSOURCE_ETRMODE1_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8)    || \
   ((INSTANCE) == TIM9)    || \
   ((INSTANCE) == TIM12))

#define IS_TIM_CLOCKSOURCE_ETRMODE2_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8))

#define IS_TIM_CLOCKSOURCE_TIX_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8))

#define IS_TIM_CLOCKSOURCE_ITRX_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8)    || \
   ((INSTANCE) == TIM9)    || \
   ((INSTANCE) == TIM12))

#define IS_TIM_OCXREF_CLEAR_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8))

#define IS_TIM_ENCODER_INTERFACE_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8))

#define IS_TIM_XOR_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8))

#define IS_TIM_MASTER_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM6)    || \
   ((INSTANCE) == TIM7)    || \
   ((INSTANCE) == TIM8))

#define IS_TIM_SLAVE_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8)    || \
   ((INSTANCE) == TIM9)    || \
   ((INSTANCE) == TIM12))

#define IS_TIM_SYNCHRO_INSTANCE(INSTANCE)  IS_TIM_MASTER_INSTANCE(INSTANCE)

#define IS_TIM_DMABURST_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8))

#define IS_TIM_BREAK_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1) || ((INSTANCE) == TIM8))

#define IS_TIM_CCX_INSTANCE(INSTANCE, CHANNEL) \
   ((((INSTANCE) == TIM1) &&                  \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4)))           \
    ||                                         \
    (((INSTANCE) == TIM2) &&                   \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4)))           \
    ||                                         \
    (((INSTANCE) == TIM3) &&                   \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4)))           \
    ||                                         \
    (((INSTANCE) == TIM4) &&                   \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4)))           \
    ||                                         \
    (((INSTANCE) == TIM5) &&                   \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4)))           \
    ||                                         \
    (((INSTANCE) == TIM8) &&                   \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4)))           \
    ||                                         \
    (((INSTANCE) == TIM9) &&                   \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2)))           \
    ||                                         \
    (((INSTANCE) == TIM10) &&                  \
      ((CHANNEL) == TIM_CHANNEL_1))            \
    ||                                         \
    (((INSTANCE) == TIM11) &&                  \
      ((CHANNEL) == TIM_CHANNEL_1))            \
    ||                                         \
    (((INSTANCE) == TIM12) &&                  \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2)))           \
    ||                                         \
    (((INSTANCE) == TIM13) &&                  \
      ((CHANNEL) == TIM_CHANNEL_1))            \
    ||                                         \
    (((INSTANCE) == TIM14) &&                  \
      ((CHANNEL) == TIM_CHANNEL_1)))

#define IS_TIM_CCXN_INSTANCE(INSTANCE, CHANNEL) \
    ((((INSTANCE) == TIM1) &&                    \
     (((CHANNEL) == TIM_CHANNEL_1) ||           \
      ((CHANNEL) == TIM_CHANNEL_2) ||           \
      ((CHANNEL) == TIM_CHANNEL_3)))           \
    ||                                         \
    (((INSTANCE) == TIM8) &&                   \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3))))

#define IS_TIM_COUNTER_MODE_SELECT_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8))

#define IS_TIM_REPETITION_COUNTER_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1) || ((INSTANCE) == TIM8))

#define IS_TIM_CLOCK_DIVISION_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8)    || \
   ((INSTANCE) == TIM9)    || \
   ((INSTANCE) == TIM10)   || \
   ((INSTANCE) == TIM11)   || \
   ((INSTANCE) == TIM12)   || \
   ((INSTANCE) == TIM13)   || \
   ((INSTANCE) == TIM14))

#define IS_TIM_DMA_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM6)    || \
   ((INSTANCE) == TIM7)    || \
   ((INSTANCE) == TIM8))

#define IS_TIM_DMA_CC_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM8))

#define IS_TIM_COMMUTATION_EVENT_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1) || ((INSTANCE) == TIM8))

#define IS_TIM_ETR_INSTANCE(INSTANCE)  (((INSTANCE) == TIM1)    || \
                                        ((INSTANCE) == TIM2)    || \
                                        ((INSTANCE) == TIM3)    || \
                                        ((INSTANCE) == TIM4)    || \
                                        ((INSTANCE) == TIM5)    || \
                                        ((INSTANCE) == TIM8))

#define IS_TIM_HALL_SENSOR_INTERFACE_INSTANCE(INSTANCE) (((INSTANCE) == TIM1)    || \
                                                         ((INSTANCE) == TIM2)    || \
                                                         ((INSTANCE) == TIM3)    || \
                                                         ((INSTANCE) == TIM4)    || \
                                                         ((INSTANCE) == TIM5)    || \
                                                         ((INSTANCE) == TIM8))

#define IS_TIM_32B_COUNTER_INSTANCE(INSTANCE)           0U

/****************************** END TIM Instances *****************************/


/******************** USART Instances : Synchronous mode **********************/
#define IS_USART_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                     ((INSTANCE) == USART2) || \
                                     ((INSTANCE) == USART3) || \
                                     ((INSTANCE) == USART4) || \
                                     ((INSTANCE) == USART5))

/******************** UART Instances : Asynchronous mode **********************/
#define IS_UART_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                    ((INSTANCE) == USART2) || \
                                    ((INSTANCE) == USART3) || \
                                    ((INSTANCE) == USART4) || \
                                    ((INSTANCE) == USART5))

/******************** UART Instances : Half-Duplex mode **********************/
#define IS_UART_HALFDUPLEX_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                               ((INSTANCE) == USART2) || \
                                               ((INSTANCE) == USART3) || \
                                               ((INSTANCE) == USART4) || \
                                               ((INSTANCE) == USART5))

/******************** UART Instances : LIN mode **********************/
#define IS_UART_LIN_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                        ((INSTANCE) == USART2) || \
                                        ((INSTANCE) == USART3) || \
                                        ((INSTANCE) == USART4) || \
                                        ((INSTANCE) == USART5))

/****************** UART Instances : Hardware Flow control ********************/
#define IS_UART_HWFLOW_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                           ((INSTANCE) == USART2) || \
                                           ((INSTANCE) == USART3) || \
                                           ((INSTANCE) == USART4) || \
                                           ((INSTANCE) == USART5))

/********************* UART Instances : Smard card mode ***********************/
#define IS_SMARTCARD_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                         ((INSTANCE) == USART2) || \
                                         ((INSTANCE) == USART3) || \
                                         ((INSTANCE) == USART4) || \
                                         ((INSTANCE) == USART5))

/****************** UART Instances : Auto Baud Rate detection *****************/
#define IS_USART_AUTOBAUDRATE_DETECTION_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                                            ((INSTANCE) == USART2) || \
                                                            ((INSTANCE) == USART3) || \
                                                            ((INSTANCE) == USART4) || \
                                                            ((INSTANCE) == USART5))

/*********************** UART Instances : IRDA mode ***************************/
#define IS_IRDA_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                    ((INSTANCE) == USART2) || \
                                    ((INSTANCE) == USART3) || \
                                    ((INSTANCE) == USART4) || \
                                    ((INSTANCE) == USART5))

/***************** UART Instances : Multi-Processor mode **********************/
#define IS_UART_MULTIPROCESSOR_INSTANCE(INSTANCE)  (((INSTANCE) == USART1) || \
                                                    ((INSTANCE) == USART2) || \
                                                    ((INSTANCE) == USART3) || \
                                                    ((INSTANCE) == USART4) || \
                                                    ((INSTANCE) == USART5))

/***************** UART Instances : DMA mode available **********************/
#define IS_UART_DMA_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                        ((INSTANCE) == USART2) || \
                                        ((INSTANCE) == USART3) || \
                                        ((INSTANCE) == USART4) || \
                                        ((INSTANCE) == USART5))

/****************************** RTC Instances *********************************/
#define IS_RTC_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == RTC)

/**************************** WWDG Instances *****************************/
#define IS_WWDG_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == WWDG)


/*********************** PCD Instances ****************************************/
#define IS_PCD_ALL_INSTANCE(INSTANCE) ((INSTANCE) == USB_OTG_FS)

/*********************** HCD Instances ****************************************/
#define IS_HCD_ALL_INSTANCE(INSTANCE) ((INSTANCE) == USB_OTG_FS)

/****************************** ETH Instances ********************************/
#define IS_ETH_ALL_INSTANCE(INSTANCE) ((INSTANCE) == ETH)

#define RCC_HSE_MIN         3000000U
#define RCC_HSE_MAX        25000000U

#define RCC_MAX_FREQUENCY  72000000U

/**
  * @}
  */
/******************************************************************************/
/*  For a painless codes migration between the STM32F1xx device product       */
/*  lines, the aliases defined below are put in place to overcome the         */
/*  differences in the interrupt handlers and IRQn definitions.               */
/*  No need to update developed interrupt code when moving across             */
/*  product lines within the same STM32F1 Family                              */
/******************************************************************************/

/* Aliases for __IRQn */
#define ADC1_IRQn               ADC1_2_IRQn
#define DMA2_Channel4_IRQn      DMA2_Channel4_5_IRQn
#define TIM9_IRQn               TIM1_BRK_TIM9_IRQn
#define TIM1_BRK_TIM15_IRQn     TIM1_BRK_TIM9_IRQn
#define TIM1_BRK_IRQn           TIM1_BRK_TIM9_IRQn
#define TIM1_TRG_COM_IRQn       TIM1_TRG_COM_TIM11_IRQn
#define TIM11_IRQn              TIM1_TRG_COM_TIM11_IRQn
#define TIM1_TRG_COM_TIM17_IRQn TIM1_TRG_COM_TIM11_IRQn
#define TIM10_IRQn              TIM1_UP_TIM10_IRQn
#define TIM1_UP_IRQn            TIM1_UP_TIM10_IRQn
#define TIM1_UP_TIM16_IRQn      TIM1_UP_TIM10_IRQn
#define TIM12_IRQn              TIM8_BRK_TIM12_IRQn
#define TIM8_BRK_IRQn           TIM8_BRK_TIM12_IRQn
#define TIM14_IRQn              TIM8_TRG_COM_TIM14_IRQn
#define TIM8_TRG_COM_IRQn       TIM8_TRG_COM_TIM14_IRQn
#define TIM8_UP_IRQn            TIM8_UP_TIM13_IRQn
#define TIM13_IRQn              TIM8_UP_TIM13_IRQn
#define CEC_IRQn                USBWakeUp_IRQn
#define OTG_FS_WKUP_IRQn        USBWakeUp_IRQn
#define CAN1_TX_IRQn            USB_HP_CAN1_TX_IRQn
#define USB_HP_IRQn             USB_HP_CAN1_TX_IRQn
#define USB_LP_IRQn             USB_LP_CAN1_RX0_IRQn
#define CAN1_RX0_IRQn           USB_LP_CAN1_RX0_IRQn


/* Aliases for __IRQHandler */
#define ADC1_IRQHandler               ADC1_2_IRQHandler
#define DMA2_Channel4_IRQHandler      DMA2_Channel4_5_IRQHandler
#define DMA2_Channel5_IRQHandler      DMA2_Channel4_5_IRQHandler
#define TIM9_IRQHandler               TIM1_BRK_TIM9_IRQHandler
#define TIM1_BRK_TIM15_IRQHandler     TIM1_BRK_TIM9_IRQHandler
#define TIM1_BRK_IRQHandler           TIM1_BRK_TIM9_IRQHandler
#define TIM1_TRG_COM_IRQHandler       TIM1_TRG_COM_TIM11_IRQHandler
#define TIM11_IRQHandler              TIM1_TRG_COM_TIM11_IRQHandler
#define TIM1_TRG_COM_TIM17_IRQHandler TIM1_TRG_COM_TIM11_IRQHandler
#define TIM10_IRQHandler              TIM1_UP_TIM10_IRQHandler
#define TIM1_UP_IRQHandler            TIM1_UP_TIM10_IRQHandler
#define TIM1_UP_TIM16_IRQHandler      TIM1_UP_TIM10_IRQHandler
#define TIM12_IRQHandler              TIM8_BRK_TIM12_IRQHandler
#define TIM8_BRK_IRQHandler           TIM8_BRK_TIM12_IRQHandler
#define TIM14_IRQHandler              TIM8_TRG_COM_TIM14_IRQHandler
#define TIM8_TRG_COM_IRQHandler       TIM8_TRG_COM_TIM14_IRQHandler
#define TIM8_UP_IRQHandler            TIM8_UP_TIM13_IRQHandler
#define TIM13_IRQHandler              TIM8_UP_TIM13_IRQHandler


#ifdef __cplusplus
}
#endif

#endif /* __PY32F403xC_H__ */

