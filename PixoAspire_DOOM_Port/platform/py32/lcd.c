// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
// Copyright (C) 2026 VapeDoom port
//
// ST7789-style panel of the Aspire PIXO on SPI1, shared with the NOR flash.
//
// The panel is 173 x 323 in its own portrait orientation with a 34 pixel
// column offset. Held sideways it is 323 x 173, and that is how the game is
// played. The engine draws on a 320 x 240 screen, so:
//
//   screen x 0..319  ->  panel row 0..319, one row per screen column
//   screen y 0..239  ->  panel column ymap[y], 240 squeezed into 173
//
// MADCTL 0x40 was picked by measurement, not from the datasheet: with it the
// panel's row index runs the same way as the screen's x axis, so a column of
// pixels is one contiguous run in panel memory and the whole frame streams
// without a single extra window command. That is also the order the strip
// renderer produces columns in, which is why the pump can hand them straight
// to SPI.
//
// The vertical squeeze lives here and nowhere else. lcd_push() takes screen
// pixels; how many panel pixels each one becomes (0, 1 or 2) comes out of
// ymap. Callers - the pump, the menus, the fault screen - never see it.

#include "lcd.h"
#include "platform.h"
#include "py32_hw.h"

#ifndef LCD_SPI_BR
#define LCD_SPI_BR 0u  // PCLK2/2 = 36 MHz, the part's maximum
#endif

static uint8_t madctl = 0x40u;
static uint8_t profile;
static uint8_t flipped;
static uint8_t lcd_ready;

// screen y -> first panel column of that row, plus a terminator
static uint8_t ymap[LCD_H + 1];

// Open window in screen coordinates and the cursor inside it
static int16_t wx0, wy0, wx1, wy1, cx, cy;
static uint8_t win_open;
static uint8_t win_suspend;  // stream paused because the NOR had the bus

// ---- SPI ----
// Mode 3 and software chip select. The NOR runs on the same wires in the same
// mode, so a switch between the two is a chip select and nothing else.
void py32_spi_init(void)
{
    SPI_CR1 = 0;
    SPI_CR2 = SPI_CR2_FRXTH;  // RXNE at 8 bits, not 16: the FIFO is 32 bit wide
    SPI_CR1 = ((uint32_t)LCD_SPI_BR << SPI_CR1_BR_SHIFT) | SPI_CR1_MSTR | SPI_CR1_SSM |
              SPI_CR1_SSI | SPI_CR1_CPOL | SPI_CR1_CPHA;
    SPI_CR1 |= SPI_CR1_SPE;
}

void py32_spi_idle(void)
{
    uint32_t guard = 200000u;
    while ((SPI_SR & (SPI_SR_FTLVL | SPI_SR_BSY)) && --guard) {
    }
}

void py32_spi_drain(void)
{
    uint32_t guard = 64u;
    py32_spi_idle();
    while ((SPI_SR & SPI_SR_FRLVL) && --guard)
        (void)SPI_DR8;
    (void)SPI_SR;
}

static void cmd(uint8_t c, const uint8_t *d, unsigned n)
{
    NOR_CS_HI();
    py32_spi_idle();
    LCD_DC_CMD();
    LCD_CS_LO();
    py32_spi_tx(c);
    py32_spi_idle();
    if (n) {
        LCD_DC_DAT();
        while (n--)
            py32_spi_tx(*d++);
        py32_spi_idle();
    }
    LCD_CS_HI();
    py32_spi_drain();
}

// ---- Window ----
// x0..x1, y0..y1 are screen coordinates. The panel window is the rows they
// cover and the panel columns ymap[y0] .. ymap[y1+1]-1.
void lcd_window(int x0, int y0, int x1, int y1)
{
    uint8_t b[4];
    unsigned nx0 = ymap[y0], nx1 = (unsigned)ymap[y1 + 1] - 1u;
    unsigned ny0 = (unsigned)x0, ny1 = (unsigned)x1;

    nx0 += PANEL_XOFF;
    nx1 += PANEL_XOFF;
    b[0] = (uint8_t)(nx0 >> 8);
    b[1] = (uint8_t)nx0;
    b[2] = (uint8_t)(nx1 >> 8);
    b[3] = (uint8_t)nx1;
    cmd(0x2A, b, 4);

    b[0] = (uint8_t)(ny0 >> 8);
    b[1] = (uint8_t)ny0;
    b[2] = (uint8_t)(ny1 >> 8);
    b[3] = (uint8_t)ny1;
    cmd(0x2B, b, 4);

    cmd(0x2C, 0, 0);
}

// Open a window and start the pixel stream. lcd_begin() closes an open pump
// run first, lcd_begin_raw() does not - the pump calls it itself and must
// not trip over its own feet.
static void begin_window(int x0, int y0, int x1, int y1)
{
    if (x0 < 0)
        x0 = 0;
    if (y0 < 0)
        y0 = 0;
    if (x1 >= LCD_W)
        x1 = LCD_W - 1;
    if (y1 >= LCD_H)
        y1 = LCD_H - 1;
    if (x1 < x0 || y1 < y0 || ymap[y1 + 1] == ymap[y0]) {
        win_open = 0;  // nothing of this rectangle survives the squeeze
        return;
    }
    wx0 = (int16_t)x0;
    wy0 = (int16_t)y0;
    wx1 = (int16_t)x1;
    wy1 = (int16_t)y1;
    cx = (int16_t)x0;
    cy = (int16_t)y0;
    win_suspend = 0;
    lcd_window(x0, y0, x1, y1);
    NOR_CS_HI();
    py32_spi_idle();
    LCD_DC_DAT();
    LCD_CS_LO();
    win_open = 1;
}

void lcd_begin(int x0, int y0, int x1, int y1)
{
    pump_close();
    begin_window(x0, y0, x1, y1);
}

void lcd_begin_raw(int x0, int y0, int x1, int y1)
{
    begin_window(x0, y0, x1, y1);
}

void lcd_push(uint16_t c)
{
    unsigned rep;
    if (!win_open)
        return;
    if (win_suspend)
        lcd_stream_arm();
    rep = (unsigned)ymap[cy + 1] - (unsigned)ymap[cy];
    while (rep--) {
        py32_spi_tx((uint8_t)(c >> 8));
        py32_spi_tx((uint8_t)c);
    }
    if (++cy > wy1) {
        cy = wy0;
        cx++;
    }
}

void lcd_push_n(const uint16_t *px, uint32_t n)
{
    while (n--)
        lcd_push(*px++);
}

void lcd_end(void)
{
    if (!win_open)
        return;
    if (!win_suspend) {
        py32_spi_idle();
        LCD_CS_HI();
        py32_spi_drain();
    }
    win_open = 0;
    win_suspend = 0;
}

// Hand the bus to the NOR.
//
// Resuming the pixel stream costs a chip select pair and a 0x3C command, and
// the renderer reads the NOR thousands of times per frame - at first every
// one of those paid for a resume. So the stream is only paused here and
// armed again lazily, right before the next pixel is pushed. A run of NOR
// reads between two strips now costs one resume instead of hundreds.
int lcd_bus_release(void)
{
    if (!win_open || win_suspend)
        return 0;
    py32_spi_idle();
    LCD_CS_HI();
    py32_spi_drain();
    win_suspend = 1;
    return 1;
}

void lcd_bus_resume(int was_open) { (void)was_open; }

// Put the panel back on the bus if a NOR access interrupted the stream.
void lcd_stream_arm(void)
{
    if (!win_open || !win_suspend)
        return;
    NOR_CS_HI();
    py32_spi_idle();
    LCD_DC_CMD();
    LCD_CS_LO();
    py32_spi_tx(0x3Cu);  // write memory continue, not 0x2C: keep the position
    py32_spi_idle();
    LCD_DC_DAT();
    win_suspend = 0;
}

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
        pin_high(GPIOE, PIN_BL);
    else
        pin_low(GPIOE, PIN_BL);
}

void lcd_flip(int flip)
{
    pump_close();
    flipped = flip ? 1u : 0u;
    madctl = flipped ? 0x80u : 0x40u;
    cmd(0x36, &madctl, 1);
}

int lcd_flipped(void) { return flipped; }
int lcd_profile(void) { return profile; }

// Power, VCOM and gamma of this panel, taken from the stock firmware's own
// init table (Vape_DOOM_ScreenShare/PixoAspire_Vape_FW/Src/main.c) because
// that is what the glass in this device was tuned with. MADCTL and the column
// address set are this port's.
static const uint8_t init_stock[] = {
    0x3A, 1, 0x05,                          // COLMOD 16 bpp
    0xB2, 5, 0x0C, 0x0C, 0x00, 0x33, 0x33,  // PORCTRL
    0xB7, 1, 0x35,                          // GCTRL
    0xBB, 1, 0x35,                          // VCOMS
    0xC0, 1, 0x2C,                          // LCMCTRL
    0xC2, 1, 0x01,                          // VDVVRHEN
    0xC3, 1, 0x13,                          // VRHS
    0xC4, 1, 0x20,                          // VDVS
    0xC6, 1, 0x0F,                          // FRCTRL2
    0xD0, 2, 0xA4, 0xA1,                    // PWCTRL1
    0xD6, 2, 0xA4, 0xA1,
    0xE0, 14, 0xF0, 0x00, 0x04, 0x04, 0x04, 0x05, 0x29, 0x33, 0x3E, 0x38, 0x12, 0x12, 0x28,
    0x30,
    0xE1, 14, 0xF0, 0x07, 0x0A, 0x0D, 0x0B, 0x07, 0x28, 0x33, 0x3E, 0x36, 0x14, 0x14, 0x29,
    0x32,
    0,
};

unsigned lcd_row_reps(int y)
{
    return (unsigned)ymap[y + 1] - (unsigned)ymap[y];
}

static void build_ymap(void)
{
    for (unsigned y = 0; y <= LCD_H; y++)
        ymap[y] = (uint8_t)((y * PANEL_W) / LCD_H);
}

void lcd_init(int prof)
{
    profile = (uint8_t)prof;
    build_ymap();
    py32_spi_init();

    NOR_CS_HI();
    LCD_CS_HI();
    pin_high(GPIOB, PIN_LCD_RST);
    plat_delay_ms(5);
    pin_low(GPIOB, PIN_LCD_RST);
    plat_delay_ms(15);
    pin_high(GPIOB, PIN_LCD_RST);
    plat_delay_ms(120);

    cmd(0x11, 0, 0);  // SLPOUT
    plat_delay_ms(120);
    cmd(0x36, &madctl, 1);
    for (const uint8_t *p = init_stock; *p; p += 2 + p[1])
        cmd(p[0], p + 2, p[1]);
    cmd(0x21, 0, 0);  // INVON, this IPS glass needs it
    lcd_ready = 1;
    lcd_fill(0, 0, LCD_W - 1, LCD_H - 1, 0);
    cmd(0x29, 0, 0);  // DISPON
    lcd_backlight(1);
}

void lcd_recover(void)
{
    if (!lcd_ready)
        return;
    win_open = 0;
    win_suspend = 0;
    LCD_CS_HI();
    NOR_CS_HI();
    py32_spi_init();
    py32_spi_drain();
    pump_reset();
}
