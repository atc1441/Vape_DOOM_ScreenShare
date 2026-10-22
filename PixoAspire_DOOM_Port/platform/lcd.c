// SPDX-License-Identifier: GPL-2.0-or-later AND MIT
// Copyright (C) 2026 Artur Bieniek
// Copyright (c) 2021 Pimoroni Ltd (the panel values of init_pimoroni, MIT License)
//
// ST7789V on SPI2 (transmit-only, mode 3, 72 MHz, 36 MHz with LCD_SPI_SLOW=1). Commands use
// 8-bit frames, pixel data 16-bit frames. The frame size (FBN) is changed only while SPI2 is
// disabled. The LCD_POLLED=1 build (Renode) sends pixels as two 8-bit frames.
//
// Pins: SCK PB13, MOSI PB15, D/C PB12, CS PB14, reset PA8, backlight PA3.

#include "at32.h"
#include "lcd.h"
#include "platform.h"

#ifndef LCD_POLLED
#define LCD_POLLED 0
#endif

#define SPI2_CFG (SPI_SLBEN | SPI_SLBTD | SPI_SWCSEN | SPI_SWCSIL | SPI_MSTEN | SPI_CPOL | SPI_CPHA | \
                  (LCD_SPI_SLOW ? SPI_MDIV4 : 0u))

uint8_t lcd_ready;
static uint8_t frame16, profile, flipped, madctl = 0x80;

// Busy wait, so lcd_init also works with interrupts masked (fault screen before plat_init)
static void wait_ms(uint32_t ms) { spin_cycles(ms * 144000u); }

static inline void cs_lo(void) { GPIO_CLR(GPIOB) = 1u << PIN_LCD_CS; }

static inline void cs_hi(void)
{
    GPIO_SCR(GPIOB) = 1u << PIN_LCD_CS;
    spin_cycles(8);
}

static void wait_idle(void)
{
    for (uint32_t n = 100000u; n && !(SPI_STS(SPI2) & SPI_TDBE); n--) {
    }
    for (uint32_t n = 100000u; n && (SPI_STS(SPI2) & SPI_BF); n--) {
    }
}

static inline void put(uint32_t v)
{
    for (uint32_t n = 100000u; n && !(SPI_STS(SPI2) & SPI_TDBE); n--) {
    }
    SPI_DT(SPI2) = v;
}

static void set_frame16(uint8_t on)
{
    if (frame16 == on)
        return;
    wait_idle();
    uint32_t c = SPI2_CFG | (on ? SPI_FBN16 : 0u);
    SPI_CTRL1(SPI2) = c;
    SPI_CTRL1(SPI2) = c | SPI_EN;
    frame16 = on;
}

static void cmd(uint8_t c, const uint8_t *d, unsigned n)
{
    set_frame16(0);
    wait_idle();
    pin_low(GPIOB, PIN_LCD_DC);
    cs_lo();
    put(c);
    wait_idle();
    pin_high(GPIOB, PIN_LCD_DC);
    while (n--)
        put(*d++);
    wait_idle();
    cs_hi();
}

void lcd_window(int x0, int y0, int x1, int y1)
{
    // Screen x is the panel row (RASET), screen y the panel column (CASET)
    uint8_t ca[4] = {(uint8_t)(y0 >> 8), (uint8_t)y0, (uint8_t)(y1 >> 8), (uint8_t)y1};
    uint8_t ra[4] = {(uint8_t)(x0 >> 8), (uint8_t)x0, (uint8_t)(x1 >> 8), (uint8_t)x1};
    cmd(0x2A, ca, 4);
    cmd(0x2B, ra, 4);
    cmd(0x2C, 0, 0);
    set_frame16(!LCD_POLLED);
    pin_high(GPIOB, PIN_LCD_DC);
    cs_lo();
}

void lcd_data_end(void)
{
    wait_idle();
    cs_hi();
}

void lcd_begin(int x0, int y0, int x1, int y1)
{
    pump_close();
    lcd_window(x0, y0, x1, y1);
}

void lcd_push(uint16_t c)
{
#if LCD_POLLED
    put(c >> 8);
    put(c & 0xFFu);
#else
    put(c);
#endif
}

void lcd_push_n(const uint16_t *px, uint32_t n)
{
    while (n--)
        lcd_push(*px++);
}

void lcd_end(void) { lcd_data_end(); }

void lcd_fill(int x0, int y0, int x1, int y1, uint16_t c)
{
    if (x0 < 0)
        x0 = 0;
    if (y0 < 0)
        y0 = 0;
    if (x1 >= LCD_W)
        x1 = LCD_W - 1;
    if (y1 >= LCD_H)
        y1 = LCD_H - 1;
    if (x1 < x0 || y1 < y0)
        return;
    lcd_begin(x0, y0, x1, y1);
    for (uint32_t n = (uint32_t)(x1 - x0 + 1) * (uint32_t)(y1 - y0 + 1); n; n--)
        lcd_push(c);
    lcd_end();
}

void lcd_backlight(int on)
{
    if (on)
        pin_high(GPIOA, PIN_LCD_BL);
    else
        pin_low(GPIOA, PIN_LCD_BL);
}

void lcd_flip(int flip)
{
    pump_close();
    flipped = flip ? 1 : 0;
    madctl = flipped ? 0x40 : 0x80;
    cmd(0x36, &madctl, 1);
}

int lcd_flipped(void) { return flipped; }
int lcd_profile(void) { return profile; }

// Power, VCOM and gamma values of the 320x240 panel profile in the pimoroni-pico ST7789
// driver (drivers/st7789/st7789.cpp), MIT License, Copyright (c) 2021 Pimoroni Ltd, see
// LICENSES/MIT.txt. LCMCTRL (C0) is never written: its reset value 0x2C is what MADCTL 0x80
// and the colour order assume. Entries: command, count, parameters.
static const uint8_t init_pimoroni[] = {
    0x3A, 1, 0x05,                          // COLMOD 16 bpp
    0xB2, 5, 0x0C, 0x0C, 0x00, 0x33, 0x33,  // PORCTRL
    0xB7, 1, 0x35,                          // GCTRL
    0xBB, 1, 0x1F,                          // VCOMS
    0xC2, 1, 0x01,                          // VDVVRHEN
    0xC3, 1, 0x12,                          // VRHS
    0xC4, 1, 0x20,                          // VDVS
    0xC6, 1, 0x0F,                          // FRCTRL2 60 Hz
    0xD0, 2, 0xA4, 0xA1,                    // PWCTRL1
    0xE0, 14, 0xD0, 0x08, 0x11, 0x08, 0x0C, 0x15, 0x39, 0x33, 0x50, 0x36, 0x13, 0x14, 0x29, 0x2D,
    0xE1, 14, 0xD0, 0x08, 0x10, 0x08, 0x06, 0x06, 0x39, 0x44, 0x51, 0x0B, 0x16, 0x14, 0x2F, 0x31,
    0x21, 0,                                // INVON, the IPS panel needs it
    0,
};

// Chip reset values for everything else
static const uint8_t init_minimal[] = {
    0x3A, 1, 0x05,
    0x21, 0,
    0,
};

void lcd_init(int prof)
{
    pump_close();
    pin_cfg(GPIOA, PIN_LCD_BL, PIN_OUT_PP);
    pin_cfg(GPIOA, PIN_LCD_RST, PIN_OUT_PP);
    pin_cfg(GPIOB, PIN_LCD_DC, PIN_OUT_PP);
    pin_cfg(GPIOB, PIN_LCD_CS, PIN_OUT_PP);
    pin_high(GPIOB, PIN_LCD_DC);
    pin_high(GPIOB, PIN_LCD_CS);
    pin_cfg(GPIOB, 13, PIN_AF_PP);
    pin_cfg(GPIOB, 15, PIN_AF_PP);
    SPI_CTRL1(SPI2) = 0;
    SPI_CTRL2(SPI2) = 0;
    SPI_CTRL1(SPI2) = SPI2_CFG;
    SPI_CTRL1(SPI2) = SPI2_CFG | SPI_EN;
    frame16 = 0;

    pin_low(GPIOA, PIN_LCD_RST);
    wait_ms(10);
    pin_high(GPIOA, PIN_LCD_RST);
    wait_ms(120);
    cmd(0x11, 0, 0);  // SLPOUT
    wait_ms(120);
    cmd(0x36, &madctl, 1);
    profile = prof == LCD_INIT_MINIMAL ? LCD_INIT_MINIMAL : LCD_INIT_PIMORONI;
    for (const uint8_t *p = profile ? init_pimoroni : init_minimal; *p; p += 2 + p[1])
        cmd(p[0], p + 2, p[1]);
    lcd_fill(0, 0, LCD_W - 1, LCD_H - 1, 0);
    cmd(0x29, 0, 0);  // DISPON
    lcd_backlight(1);
    lcd_ready = 1;
}

void lcd_recover(void)
{
    if (!lcd_ready)
        return;
    DMA_CCTRL(5) = 0;
    DMA_CLR = DMA_FLAGS(5);
    SPI_CTRL2(SPI2) = 0;
    wait_idle();
    cs_hi();
    SPI_CTRL1(SPI2) = SPI2_CFG;
    SPI_CTRL1(SPI2) = SPI2_CFG | SPI_EN;
    frame16 = 0;
    pump_reset();
}
