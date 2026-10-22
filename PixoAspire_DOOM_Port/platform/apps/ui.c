// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek

#include "ui.h"

#include "lcd.h"
#include "platform.h"

void ui_clear(void) { lcd_fill(0, 0, LCD_W - 1, LCD_H - 1, C_BLACK); }

void ui_text_row(int y, uint16_t fg, uint16_t bg, const char *s)
{
    lcd_fill(0, y, LCD_W - 1, y, bg);
    int x = lcd_text(0, y + 1, s, fg, bg, 1);
    lcd_fill(x, y + 1, LCD_W - 1, y + 8, bg);
    lcd_fill(0, y + 9, LCD_W - 1, y + 9, bg);
}

void ui_header(const char *title, int page, int npages)
{
    char s[44];
    plat_fmt(s, sizeof(s), "%d/%d %s", page + 1, npages, title);
    ui_text_row(0, C_BLACK, C_YELLOW, s);
}

void ui_footer(const char *s) { ui_text_row(LCD_H - 10, C_BLACK, C_GREY, s); }

void ui_row(int r, uint16_t fg, const char *f, ...)
{
    char s[44];
    va_list ap;
    va_start(ap, f);
    plat_vfmt(s, sizeof(s), f, ap);
    va_end(ap);
    if (r >= 0 && r < UI_ROWS)
        ui_text_row(UI_ROW_Y(r), fg, C_BLACK, s);
}

int ui_power_held(uint32_t ms)
{
    static uint32_t since;
    static int armed, released;
    uint32_t b = plat_buttons();
    // POWER may still be held from switching the meter on: count only after a release
    if (!(b & BTN_BIT(BTN_POWER)))
        released = 1;
    if (b != BTN_BIT(BTN_POWER) || !released) {
        armed = 0;
        return 0;
    }
    if (!armed) {
        armed = 1;
        since = plat_ms();
    }
    return plat_ms() - since >= ms;
}

void ui_fix2(char *out, int cap, uint32_t v100)
{
    plat_fmt(out, cap, "%u.%02u", v100 / 100u, v100 % 100u);
}

uint32_t ui_cyc_to_us100(uint32_t cycles)
{
    return cycles / 144u * 100u + (cycles % 144u) * 100u / 144u;
}
