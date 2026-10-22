// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 VapeDoom port
//
// Debug log in RAM, read over SWD while the CPU runs.
//
// The DMC-100 can print on its own screen and talk over USB; this board has
// neither during bring-up. A Cortex-M4 lets the debugger read memory without
// halting, so a plain ring buffer with a magic and a monotonic write counter
// is enough: the host polls it, the target never blocks, and the watchdogs
// keep being fed. The same block carries a stage marker so a hang can be
// located even when nothing was logged.

#include <stdint.h>

#include "py32_hw.h"

#define DBG_MAGIC 0x564C4F47u  // "VLOG"
#define DBG_SIZE 2048u

struct dbg_block {
    volatile uint32_t magic;
    volatile uint32_t size;
    volatile uint32_t wr;     // monotonic, never wraps back
    volatile uint32_t stage;  // last dbg_mark, for hangs with no output
    volatile uint32_t ms;      // copy of plat_ms, so the host sees it run
    volatile uint32_t frames;  // full views pushed to the panel
    volatile uint32_t buttons; // last debounced button state
    volatile uint32_t fps100;  // frames per second x100, over the last second
    // Where a frame goes. Cycles at 144 MHz, summed; the host divides by
    // the frame count to get per-frame costs.
    volatile uint32_t nor_cyc, nor_calls, nor_bytes;
    volatile uint32_t pump_cyc, pump_px;
    volatile uint32_t tc_ok;     // geglueckte I2C-Lesevorgaenge am Touch
    volatile uint32_t tc_touch;  // davon mit Finger
    volatile uint32_t touch_sx, touch_sy;  // screen coordinates of the touch
    volatile uint32_t touch_raw;          // rohe Controllerwerte, x<<16|y
    volatile uint8_t buf[DBG_SIZE];
};

__attribute__((used)) struct dbg_block dbg;

void dbg_init(void)
{
    dbg.size = DBG_SIZE;
    dbg.wr = 0;
    dbg.stage = 0;
    dbg.magic = DBG_MAGIC;
}

void dbg_putc(char c)
{
    uint32_t w = dbg.wr;
    dbg.buf[w % DBG_SIZE] = (uint8_t)c;
    dbg.wr = w + 1u;
}

void dbg_str(const char *s)
{
    while (*s)
        dbg_putc(*s++);
}

void dbg_hex(uint32_t v, unsigned digits)
{
    static const char hx[] = "0123456789ABCDEF";
    for (unsigned i = digits; i; i--)
        dbg_putc(hx[(v >> ((i - 1u) * 4u)) & 0xFu]);
}

void dbg_dec(uint32_t v)
{
    char b[11];
    int n = 0;
    if (!v) {
        dbg_putc('0');
        return;
    }
    while (v && n < 10) {
        b[n++] = (char)('0' + v % 10u);
        v /= 10u;
    }
    while (n--)
        dbg_putc(b[n]);
}

void dbg_nl(void)
{
    dbg_putc('\r');
    dbg_putc('\n');
}

void dbg_mark(uint32_t stage) { dbg.stage = stage; }

// Called from the SysTick: the running millisecond count is the cheapest
// liveness signal the host can read, and the frame rate falls out of it.
void dbg_tick(uint32_t ms, uint32_t buttons)
{
    static uint32_t last_ms, last_frames;
    dbg.ms = ms;
    dbg.buttons = buttons;
    if (ms - last_ms >= 1000u) {
        dbg.fps100 = (dbg.frames - last_frames) * 100000u / (ms - last_ms);
        last_ms = ms;
        last_frames = dbg.frames;
    }
}

void dbg_frame(void) { dbg.frames++; }

void dbg_touch(int ok, int down)
{
    if (ok)
        dbg.tc_ok++;
    if (down)
        dbg.tc_touch++;
}

void dbg_touch_pos(uint32_t rx, uint32_t ry, uint32_t sx, uint32_t sy)
{
    dbg.touch_raw = (rx << 16) | (ry & 0xFFFFu);
    dbg.touch_sx = sx;
    dbg.touch_sy = sy;
}

void dbg_nor(uint32_t cyc, uint32_t bytes)
{
    dbg.nor_cyc += cyc;
    dbg.nor_calls++;
    dbg.nor_bytes += bytes;
}

void dbg_pump(uint32_t cyc, uint32_t px)
{
    dbg.pump_cyc += cyc;
    dbg.pump_px += px;
}
