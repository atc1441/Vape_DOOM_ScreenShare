// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
// Copyright (C) 2026 VapeDoom port
//
// Board bring-up for the Aspire PIXO: PY32F403xC at 144 MHz, the two
// watchdogs, and the order of the subsystem inits. The file keeps the name of
// the DMC-100 original so the shared build rules find it.
//
// The clock and pin values below are not derived from the reference manual.
// They were read back over SWD from a firmware that demonstrably drives this
// board (tools/dumpregs.py), and the sequence around them is the ordinary
// "latency first, PLL off while configuring, switch last" one. That keeps the
// risk where it belongs: in a sequence that is easy to check, not in a dozen
// field encodings guessed from a datasheet.

#include "lcd.h"
#include "nor.h"
#include "platform.h"
#include "py32_hw.h"

static uint8_t dwt_ok;
static uint8_t pc6_level;

uint32_t plat_cycles(void) { return dwt_ok ? DWT_CYCCNT : 0; }

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

// Both watchdogs. The internal one only needs a key; the external chip on PC6
// wants an edge, so the level is tracked and toggled. Called from the 1 kHz
// SysTick, which is the whole point: no render loop, no NOR transfer and no
// stalled bus can starve them.
void wdt_kick(void)
{
    IWDG_KR = 0x0000AAAAu;
    if (pc6_level) {
        pin_low(GPIOC, PIN_EXTWDT);
        pc6_level = 0;
    } else {
        pin_high(GPIOC, PIN_EXTWDT);
        pc6_level = 1;
    }
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

// ---- Clock tree ----
// HSI 8 MHz, PLL x18 = 144 MHz. AHB /1, APB1 /1, APB2 /2 (72 MHz, so SPI1 at
// PCLK2/2 = 36 MHz, the part's maximum). ADC /8. Flash needs six wait states
// at this speed.
#define PY_FLASH_ACR_VAL 0x00000706u
#define PY_RCC_CFGR_VAL  0x2000E00Au  // includes SW = PLL
#define PY_RCC_CFGR1_VAL 0x0C9B8030u
#define PY_RCC_CR_VAL    0x03602C83u  // HSI on, PLL on, HSE off
#define PY_RCC_AHB1EN    0x00000014u
#define PY_RCC_APB2EN    0x00001200u
#define PY_RCC_APB1EN    0x08000000u
#define PY_RCC_AHB2EN    0x0000007Cu

void py32_clock_init(void)
{
    // Wait states before the frequency goes up.
    FLASH_ACR = PY_FLASH_ACR_VAL;

    RCC_CR |= 1u;  // HSION
    while (!(RCC_CR & (1u << 1))) {
    }
    RCC_CFGR &= ~3u;  // run from HSI while the PLL is reconfigured
    while (RCC_CFGR & (3u << 2)) {
    }
    RCC_CR &= ~(1u << 24);  // PLLOFF
    while (RCC_CR & (1u << 25)) {
    }

    RCC_CFGR = PY_RCC_CFGR_VAL & ~3u;  // prescalers and PLL x18, still on HSI
    RCC_CFGR1 = PY_RCC_CFGR1_VAL;

    RCC_CR = PY_RCC_CR_VAL;  // PLLON
    while (!(RCC_CR & (1u << 25))) {
    }

    RCC_CFGR = PY_RCC_CFGR_VAL;  // SW = PLL
    while ((RCC_CFGR & (3u << 2)) != (2u << 2)) {
    }

    RCC_AHB1EN = PY_RCC_AHB1EN;
    RCC_APB2EN = PY_RCC_APB2EN;
    RCC_APB1EN = PY_RCC_APB1EN;
    RCC_AHB2EN = PY_RCC_AHB2EN;

    RCC_CSR |= 1u;  // LSI for the independent watchdog
    while (!(RCC_CSR & 2u)) {
    }
}

// ---- Pins ----
// The whole port configuration, exactly as the stock-derived firmware leaves
// it. ODR is written before MODER so a pin that becomes an output starts at
// the intended level, and PB13 is clear in that word.
struct port_cfg {
    uint32_t base, moder, otyper, ospeedr, pupdr, odr, afrl, afrh;
};

static const struct port_cfg ports[] = {
    {GPIOA, 0x6BEBD7FFu, 0x00000000u, 0xFC3C3C00u, 0x18140000u, 0x00008000u, 0x00000000u,
     0x00000220u},
    {GPIOB, 0x15F55ABFu, 0x00002300u, 0x3F0FFFC0u, 0x00050540u, 0x000003C0u, 0x00333000u,
     0x00000000u},
    {GPIOC, 0xF3FFD377u, 0x00000008u, 0x000030CCu, 0x00001400u, 0x00000008u, 0x00000000u,
     0x00000000u},
    {GPIOD, 0xFFFFD7FFu, 0x00000000u, 0x00003C00u, 0x00000000u, 0x00000060u, 0x00000000u,
     0x00000000u},
    {GPIOE, 0x155FCFFFu, 0x00000000u, 0x3FF00000u, 0x50002000u, 0x00006000u, 0x00000000u,
     0x00000000u},
};

void py32_pins_init(void)
{
    for (unsigned i = 0; i < sizeof(ports) / sizeof(ports[0]); i++) {
        const struct port_cfg *p = &ports[i];
        GPIO_ODR(p->base) = p->odr;
        GPIO_OTYPER(p->base) = p->otyper;
        GPIO_OSPEEDR(p->base) = p->ospeedr;
        GPIO_PUPDR(p->base) = p->pupdr;
        GPIO_AFRL(p->base) = p->afrl;
        GPIO_AFRH(p->base) = p->afrh;
        GPIO_MODER(p->base) = p->moder;
    }
    pin_low(GPIOB, PIN_COIL);  // load path, again

    // The backlight is an ordinary output here, not the timer channel the
    // stock firmware uses. It starts off so a half-drawn screen is not shown.
    pin_mode(GPIOE, PIN_BL, 1);
    pin_low(GPIOE, PIN_BL);
}

static void iwdg_init(void)
{
    // ~1.6 s at LSI/32. The SysTick feeds it every millisecond.
    IWDG_KR = 0x5555u;
    IWDG_PR = 3u;  // /32
    IWDG_RLR = 3000u;
    IWDG_KR = 0xAAAAu;
    IWDG_KR = 0xCCCCu;
}

// The vape has no power latch: it runs off its battery and the only way out is
// a reset. plat_power_off is what the engine calls on Quit, so it blanks the
// screen, parks the load path and resets.
void plat_power_off(void)
{
    __asm volatile("cpsid i");
    lcd_backlight(0);
    lcd_recover();
    pin_low(GPIOB, PIN_COIL);
    for (;;) {
        IWDG_KR = 0x0000AAAAu;  // stay alive, show nothing, wait for the user
        __asm volatile("wfi");
    }
}

void plat_init(void)
{
    py32_clock_init();
    py32_pins_init();
    dbg_init();
    dbg_nl();
    dbg_str("=== DOOM on the Aspire PIXO (PY32F403) ===");
    dbg_nl();
    dbg_str("[port] by @atc1441 - github.com/atc1441/Vape_DOOM_ScreenShare");
    dbg_nl();
    dbg_str("[engine] DMC100-Doom by Artur Bieniek"
            " - github.com/ArturBieniek4/DMC100-Doom");
    dbg_nl();
    dbg_mark(1);

    dwt_init();
    iwdg_init();
    systick_init();
    dbg_str("[sys] 144 MHz, SysTick 1 kHz, both watchdogs fed from it");
    dbg_nl();

    buttons_init();
    snd_init();

    if (nor_init())
        dbg_str("[nor] no answer");
    else {
        dbg_str("[nor] jedec 0x");
        dbg_hex(nor_jedec(), 6);
    }
    dbg_nl();
    dbg_mark(2);

    lcd_init(LCD_INIT_DEFAULT);
    dbg_str("[lcd] 320x240 screen on a 173x323 panel");
    dbg_nl();
    dbg_mark(3);

    input_legend();
}
