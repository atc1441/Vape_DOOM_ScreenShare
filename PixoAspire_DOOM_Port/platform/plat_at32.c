// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Board bring-up for the FNIRSI DMC-100: AT32F415 at 144 MHz (PLL, AHB and both APBs as the
// bootloader leaves them), power latch, DWT and the order of the subsystem inits.

#include "at32.h"
#include "lcd.h"
#include "nor.h"
#include "platform.h"

static uint8_t dwt_ok;

uint32_t plat_cycles(void) { return dwt_ok ? DWT_CYCCNT : 0; }

// Busy wait in CPU cycles, usable with interrupts masked. Uses DWT when it counts, otherwise
// a loop of about 4 cycles per turn.
void spin_cycles(uint32_t n)
{
    if (dwt_ok) {
        uint32_t t0 = DWT_CYCCNT;
        while (DWT_CYCCNT - t0 < n) {
        }
        return;
    }
    for (volatile uint32_t i = n / 4u; i; i--) {
    }
}

void plat_idle(void) { __asm volatile("wfi"); }

void plat_delay_ms(uint32_t ms)
{
    uint32_t t0 = plat_ms();
    while (plat_ms() - t0 < ms)
        plat_idle();
}

static void dwt_init(void)
{
    SCB_DEMCR |= 1u << 24;  // TRCENA
    DWT_CYCCNT = 0;
    DWT_CTRL |= 1u;
    uint32_t a = DWT_CYCCNT;
    for (volatile int i = 0; i < 64; i++) {
    }
    dwt_ok = DWT_CYCCNT != a;
}

// The latch drops while POWER may still be held, and a held POWER key keeps the board powered.
// Resetting then would boot the app again, so wait for 50 ms of released POWER first. Without
// USB power the board is off before that.
void power_cut(void)
{
    __asm volatile("cpsid i");
    pin_low(GPIOF, PIN_PWR_HOLD);
    for (uint32_t up = 0; up < 50u;) {
        spin_cycles(144000u);
        up = (GPIO_IDT(GPIOF) & (1u << 7)) ? up + 1u : 0u;  // PF7 high = POWER released
    }
    spin_cycles(250u * 144000u);
    __asm volatile("dsb");
    SCB_AIRCR = 0x05FA0004u;  // still powered (USB): reset
    __asm volatile("dsb");
    for (;;) {
    }
}

void plat_power_off(void)
{
    __asm volatile("cpsid i");
    lcd_recover();
    TMR_C3DT(TMR4) = 0;
    lcd_backlight(0);
    power_cut();
}

void plat_init(void)
{
    CRM_APB2EN |= (1u << 0) | (1u << 2) | (1u << 3) | (1u << 7) | (1u << 12);  // IOMUX, GPIOA/B/F, SPI1
    CRM_APB1EN |= (1u << 2) | (1u << 14);                                     // TMR4, SPI2
    CRM_AHBEN |= 1u << 0;                                                     // DMA1
    if (DMA_SRC_SEL1 & (1u << 24))
        DMA_SRC_SEL1 = 0;  // fixed DMA request map
    // SW-DP only: PA15 (LIGHT) and PB4 (HOLD) become plain GPIO whatever the bootloader set
    IOMUX_REMAP = (IOMUX_REMAP & ~(7u << 24)) | (2u << 24);
    IOMUX_REMAP7 = (IOMUX_REMAP7 & ~(7u << 16)) | (2u << 16);

    pin_cfg(GPIOF, PIN_PWR_HOLD, PIN_OUT_PP);
    pin_high(GPIOF, PIN_PWR_HOLD);

    dwt_init();
    systick_init();
    buttons_init();
    snd_init();
    nor_init();
    lcd_init(LCD_INIT_DEFAULT);
}
