// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
// Copyright (C) 2026 VapeDoom port
//
// LCD pump: indexed columns to RGB565 through pump_lut, written to SPI1 by
// the CPU.
//
// The DMC-100 hands this job to a DMA channel and refills two half-column
// buffers from its interrupt, because there the NOR sits on a second SPI and
// both can run at once. On this board the panel and the flash share one bus
// and one set of wires, so there is nothing to overlap with: while a frame
// goes out, no map data can be fetched anyway. A polled pump is then not a
// compromise, it is the honest shape of the hardware, and it removes an
// interrupt that would otherwise have to arbitrate for the bus.
//
// Every submitted batch opens its own panel window, covering exactly the
// columns it carries.
//
// The renderer opens one window per frame and streams all 320 columns into
// it, fetching textures from the NOR between strips. Each of those fetches
// has to lift the panel's chip select, and picking the half-written window
// up again then rests on the controller keeping its address counter across
// that. It did not, reliably: the picture arrived in blocks, each shifted
// against the next. Addressing every batch absolutely settles the question
// instead of trusting it. The price is three short commands per strip,
// about twenty-five per frame, against the 46,000 pixels they carry.
//
// The inner loop is what the frame rate rides on, so it does the palette
// lookup and the vertical squeeze without a call per pixel: lcd.c says how
// many panel pixels each screen row becomes, that is collected once per
// window into rowrep[], and rows that fall out of the squeeze cost one
// compare.

#include "lcd.h"
#include "platform.h"
#include "py32_hw.h"

uint16_t pump_lut[256];
volatile struct pump_stats pump_stats;

static uint8_t is_open;
static uint8_t reps_dbl;
static uint8_t first_frame;
static int16_t win_x1, win_y0, win_y1;
static int16_t cur_x;  // first panel column row the next batch writes
static int16_t win_h;
static uint8_t rowrep[LCD_H];

__attribute__((minsize)) static void check_window(int x0, int x1, int y0, int y1)
{
    int h = y1 - y0 + 1;
    if (h <= 0 || h > LCD_H || x0 < 0 || x1 < x0 || x1 >= LCD_W || y0 < 0 || y1 >= LCD_H)
        plat_panic("pump: bad window");
}

__attribute__((minsize)) void pump_begin(int x0, int x1, int y0, int y1, int flags)
{
    check_window(x0, x1, y0, y1);
    pump_close();
    win_x1 = (int16_t)x1;
    win_y0 = (int16_t)y0;
    win_y1 = (int16_t)y1;
    cur_x = (int16_t)x0;
    win_h = (int16_t)(y1 - y0 + 1);
    reps_dbl = (flags & PUMP_DOUBLE) ? 2u : 1u;
    for (int i = 0; i < win_h; i++)
        rowrep[i] = (uint8_t)lcd_row_reps(y0 + i);
    is_open = 1;
}

void pump_submit(const uint8_t *cols, int ncols)
{
    uint32_t t0 = DWT_CYCCNT;
    uint32_t px = 0;
    const uint16_t *lut = pump_lut;
    const uint8_t *rep = rowrep;
    unsigned h = (unsigned)win_h;
    int span;

    if (!is_open || ncols <= 0)
        return;
    if (!first_frame) {
        first_frame = 1;
        dbg_str("[pump] first columns on their way to the panel");
        dbg_nl();
        dbg_mark(6);
    }

    // In low detail every column goes out twice, so a batch of ncols
    // columns covers twice as many panel column rows.
    span = ncols * (int)reps_dbl;
    if (cur_x + span - 1 > win_x1)
        span = win_x1 - cur_x + 1;
    if (span <= 0)
        return;
    lcd_begin_raw(cur_x, win_y0, cur_x + span - 1, win_y1);

    for (; ncols > 0; ncols--, cols += h) {
        for (unsigned r = 0; r < reps_dbl; r++) {
            for (unsigned i = 0; i < h; i++) {
                unsigned n = rep[i];
                if (!n)
                    continue;
                uint16_t c = lut[cols[i]];
                px += n;
                do {
                    py32_spi_tx((uint8_t)(c >> 8));
                    py32_spi_tx((uint8_t)c);
                } while (--n);
            }
        }
    }
    lcd_end();
    cur_x = (int16_t)(cur_x + span);
    dbg_pump(DWT_CYCCNT - t0, px);
}

__attribute__((minsize)) void pump_sync(int max) { (void)max; }
__attribute__((minsize)) int pump_pending(void) { return 0; }

__attribute__((minsize)) void pump_end(void)
{
    if (!is_open)
        return;
    is_open = 0;
    if (win_h >= LCD_VIEW_H)
        dbg_frame();  // a window as tall as the 3D view is one frame
    lcd_end();
}


__attribute__((minsize)) void pump_close(void) { pump_end(); }

__attribute__((minsize)) void pump_reset(void) { is_open = 0; }
