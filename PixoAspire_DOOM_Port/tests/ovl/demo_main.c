// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Overlay demo firmware for Renode (polled NOR and LCD). Runs the call checks on both test
// packs and the error checks, then makes an OVL_CALL against a DOOMCODE.DMC of another build:
// the fail hook draws the mismatch screen and the app stops there. test_renode.py reads
// demo_res[] from SRAM and the screen.

#include "lcd.h"
#include "ovl_demo.h"
#include "platform.h"

volatile uint32_t demo_res[R_COUNT];

static void mismatch_screen(int id, int err)
{
    demo_res[R_HOOK_ID] = (uint32_t)id;
    demo_res[R_HOOK_ERR] = (uint32_t)err;
    lcd_fill(0, 0, LCD_W - 1, LCD_H - 1, rgb565(0, 0, 128));
    lcd_text(16, 40, "WRONG DOOMCODE.DMC", 0xFFFF, rgb565(0, 0, 128), 2);
    lcd_text(16, 80, "Copy DOOMCODE.DMC from the same", 0xFFE0, rgb565(0, 0, 128), 1);
    lcd_text(16, 92, "release as the firmware.", 0xFFE0, rgb565(0, 0, 128), 1);
    demo_res[R_STAGE] = 4;
    for (;;)
        plat_idle();
}

void app_main(void)
{
    int32_t codes[ERR_COUNT];
    demo_res[R_MAGIC] = DEMO_MAGIC;
    demo_res[R_STAGE] = 1;
    plat_init();
    ovl_fixed_base = DEMO_NOR_GOOD;
    demo_res[R_FAIL0] = demo_calls();
    ovl_fixed_base = DEMO_NOR_GOOD_LC;
    demo_res[R_FAIL1] = demo_calls();
    demo_res[R_FAILE] = demo_errors(codes);
    for (int k = 0; k < ERR_COUNT; k++)
        demo_res[R_CODES + k] = (uint32_t)codes[k];
    demo_res[R_LOADS] = ovl_loads;
    demo_res[R_STAGE] = 2;

    ovl_fixed_base = DEMO_NOR_BUILD;
    ovl_invalidate();
    ovl_set_fail_hook(mismatch_screen);
    demo_res[R_STAGE] = 3;
    OVL_CALL(OVL_TEST_A, A_SUM5, a_sum5_fn, 1, 2, 3, 4, 5);
    demo_res[R_STAGE] = 0xBAD;
}
