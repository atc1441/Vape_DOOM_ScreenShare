// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Test overlay B: calls resident functions (demo_twice, memset, memcpy, plat_fmt, lcd_text)
// and reads and writes resident data.

#include <stddef.h>

#include "lcd.h"
#include "ovl_demo.h"
#include "platform.h"

void *memcpy(void *restrict d, const void *restrict s, size_t n);
void *memset(void *d, int c, size_t n);

static uint32_t b_mix(uint32_t x)
{
    uint8_t buf[24];
    uint32_t sum = 0;
    memset(buf, 0x11, sizeof(buf));
    memcpy(buf + 4, demo_tab, sizeof(demo_tab));
    for (unsigned i = 0; i < sizeof(buf); i++)
        sum += buf[i];
    demo_state.b_seen += x;
    return (uint32_t)demo_twice((int)x) * 1000u + sum;
}

static int b_draw(int y)
{
    char s[32];
    plat_fmt(s, sizeof(s), "overlay B at y %d", y);
    return lcd_text(8, y, s, 0xFFFF, 0x0000, 1);
}

OVL_TABLE(TEST_B, OVL_ENTRY(B_MIX, b_mix_fn, b_mix), OVL_ENTRY(B_DRAW, b_draw_fn, b_draw));
