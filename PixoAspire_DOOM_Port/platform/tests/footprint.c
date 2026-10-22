// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Size gate (make platform-size): an app that calls the engine-facing platform API and the
// runtime's memcpy and memset, so the link keeps what the firmware links. plat_fmt is left
// out, tools/gates/sizegate.py counts it in its runtime line. Never run.

#include <stddef.h>

#include "lcd.h"
#include "nor.h"
#include "platform.h"

void *memcpy(void *restrict d, const void *restrict s, size_t n);
void *memset(void *d, int c, size_t n);

static uint8_t buf[256] PLAT_DYN;
volatile uint32_t footprint_sink;

static void hook(void) { plat_fault_line("pool", plat_stack_used()); }

void app_main(void)
{
    plat_init();
    plat_fault_hook(hook);
    memcpy(buf + 128, memset(buf, 1, 64), 64);
    snd_volume(SND_HIGH);
    snd_play(buf, 16);
    snd_tone(0);
    footprint_sink = snd_busy() + snd_steps() + plat_buttons() + (uint32_t)plat_event();
    snd_stop();
    footprint_sink = (uint32_t)settings_load(buf, 16, 1) + (uint32_t)settings_save(buf, 16, 1);
    lcd_flip(!lcd_flipped());
    lcd_fill(0, 0, LCD_W - 1, LCD_H - 1, 0);
    lcd_begin(0, 0, 7, 7);
    lcd_push_n(pump_lut, 64);
    lcd_end();
    lcd_text(0, 0, "x", 0xFFFF, 0, 1);
    pump_begin(0, LCD_W - 1, 0, LCD_VIEW_H - 1, PUMP_DOUBLE);
    pump_submit(buf, 1);
    pump_sync(1);
    footprint_sink = (uint32_t)pump_pending();
    pump_end();
    nor_read(0, buf, 16);
    nor_wait(nor_read_async(0, buf, 16, NOR_PRIO_COL));
    footprint_sink = nor_done(1) + nor_jedec() + nor_status(0x05);
    nor_wait_all();
    footprint_sink = (uint32_t)nor_col_start(0, buf, 16);
    nor_col_wait();
    footprint_sink = (uint32_t)nor_erase4k(0) + (uint32_t)nor_program(0, buf, 16);
    plat_delay_ms(plat_ms() & 1u);
    footprint_sink = plat_cycles();
    plat_idle();
    if (footprint_sink)
        plat_panic("x");
    plat_power_off();
}
