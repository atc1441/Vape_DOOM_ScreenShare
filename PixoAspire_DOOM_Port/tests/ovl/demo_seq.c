// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Resident half of the overlay demo, shared by the Renode firmware and the host tests. The
// checks are the same on both, overlays keep their state in demo_state.

#include <stddef.h>

#include "ovl_demo.h"
#include "platform.h"

void *memset(void *d, int c, size_t n);

struct demo_state demo_state;
const uint8_t demo_tab[16] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3};

#ifdef PLAT_HOST
#define FRAME ovl_host_frame
#else
#define FRAME __frame_start
#endif

int demo_twice(int x) { return 2 * x + demo_tab[x & 15]; }

void demo_take_frame(void)
{
    ovl_invalidate();
    memset(FRAME, 0xA5, OVL_WIN_FRAME);
}
void demo_drop_frame(void) __attribute__((alias("demo_take_frame")));

static int demo_cb(int x)
{
    demo_state.cb_calls++;
    return 3 * x + 1;
}

#define CHECK(bit, cond)          \
    do {                          \
        if (!(cond))              \
            fail |= 1u << (bit);  \
    } while (0)

uint32_t demo_calls(void)
{
    uint32_t fail = 0;
    ovl_invalidate();
    unsigned l0 = ovl_loads;
    CHECK(CHK_LOAD_A, ovl_load(OVL_TEST_A) == OVL_OK && ovl_resident() == OVL_TEST_A &&
                          ovl_loads == l0 + 1);

    CHECK(CHK_ARGS, OVL_CALL(OVL_TEST_A, A_SUM5, a_sum5_fn, 1, -2, 3, -4, 100000) == 499990);
    CHECK(CHK_RET64, OVL_CALL(OVL_TEST_A, A_MUL64, a_mul64_fn, 0xDEADBEEFu, 0x12345u) ==
                         0xDEADBEEFull * 0x12345u);
    struct demo_trio t = OVL_CALL(OVL_TEST_A, A_TRIO, a_trio_fn, 7);
    CHECK(CHK_SRET, t.a == 7 && t.b == 49 && t.c == -7);

    const char *s = OVL_FN(OVL_TEST_A, A_NAME, a_name_fn)();
    static const char want_name[] = "overlay A";
    int same = 1;
    for (unsigned i = 0; i < sizeof(want_name); i++)
        same &= s[i] == want_name[i];
    CHECK(CHK_RODATA, same);

    static const int16_t want_cls[10] = {0, 17, -3, 250, 9, -77, 1000, 4, 0, 0};
    same = 1;
    for (int k = -1; k <= 8; k++)
        same &= OVL_CALL(OVL_TEST_A, A_CLASSIFY, a_classify_fn, k) == want_cls[k + 1];
    CHECK(CHK_SWITCH, same);

    uint32_t cb0 = demo_state.cb_calls;
    CHECK(CHK_CALLBACK, OVL_CALL(OVL_TEST_A, A_APPLY, a_apply_fn, demo_cb, 14) == 1043 &&
                            demo_state.cb_calls == cb0 + 1);

    uint32_t ac0 = demo_state.a_calls;
    uint32_t s1 = OVL_FN(OVL_TEST_A, A_STATE, a_state_fn)();
    uint32_t s2 = OVL_FN(OVL_TEST_A, A_STATE, a_state_fn)();
    CHECK(CHK_STATE, s1 == ac0 + 1 && s2 == ac0 + 2 && demo_state.a_calls == ac0 + 2);
    CHECK(CHK_NO_RELOAD, ovl_loads == l0 + 1 && ovl_resident() == OVL_TEST_A);

    uint32_t sb0 = demo_state.b_seen;
    uint32_t m = OVL_CALL(OVL_TEST_B, B_MIX, b_mix_fn, 5);
    CHECK(CHK_LOAD_B, ovl_resident() == OVL_TEST_B && ovl_loads == l0 + 2);
    CHECK(CHK_RESIDENT, m == (2u * 5u + 9u) * 1000u + 8u * 0x11u + 80u &&
                            demo_state.b_seen == sb0 + 5);
    CHECK(CHK_PLATFORM, OVL_CALL(OVL_TEST_B, B_DRAW, b_draw_fn, 220) == 8 + 8 * 18);

    uint32_t s3 = OVL_FN(OVL_TEST_A, A_STATE, a_state_fn)();
    CHECK(CHK_RELOAD, s3 == s2 + 1 && ovl_resident() == OVL_TEST_A && ovl_loads == l0 + 3);

    demo_take_frame();
    CHECK(CHK_INVALIDATE, OVL_CALL(OVL_TEST_A, A_SUM5, a_sum5_fn, 5, 4, 3, 2, 1) == 35 &&
                              ovl_resident() == OVL_TEST_A && ovl_loads == l0 + 4);
    return fail;
}

uint32_t demo_errors(int32_t codes[])
{
    static const struct {
        uint32_t base;
        uint8_t id;
        int8_t want;
    } cases[ERR_COUNT] = {
        [ERR_CRC] = {DEMO_NOR_CRC, OVL_TEST_A, OVL_ECRC},
        [ERR_HDR] = {DEMO_NOR_HDR, OVL_TEST_A, OVL_EHEADER},
        [ERR_BUILD] = {DEMO_NOR_BUILD, OVL_TEST_A, OVL_EBUILD},
        [ERR_NOENT] = {DEMO_NOR_NOENT, OVL_TEST_A, OVL_ENOENT},
        [ERR_BLOB] = {DEMO_NOR_BLOB, OVL_TEST_A, OVL_EBLOB},
        [ERR_EMPTY] = {DEMO_NOR_EMPTY, OVL_TEST_A, OVL_EHEADER},
        [ERR_UNKNOWN] = {DEMO_NOR_GOOD, OVL_IDS + 1, OVL_ENOENT},
        [ERR_BSS] = {DEMO_NOR_BSS, OVL_TEST_A, OVL_OK},
        [ERR_OTHER] = {DEMO_NOR_NOENT, OVL_TEST_B, OVL_OK},
        [ERR_RECOVER] = {DEMO_NOR_GOOD, OVL_TEST_A, OVL_OK},
    };
    uint32_t fail = 0;
    for (int k = 0; k < ERR_COUNT; k++) {
        ovl_invalidate();
        if (k == ERR_CRC) {  // B resident first: the failed copy must leave nothing resident
            ovl_fixed_base = DEMO_NOR_GOOD;
            if (ovl_load(OVL_TEST_B) != OVL_OK)
                fail |= 1u << k;
        }
        if (k == ERR_BSS)
            demo_take_frame();
        ovl_fixed_base = cases[k].base;
        codes[k] = ovl_load(cases[k].id);
        int want_res = cases[k].want ? 0 : cases[k].id;
        if (codes[k] != cases[k].want || ovl_resident() != want_res)
            fail |= 1u << k;
        if (k == ERR_BSS) {  // copy size and bss from the blob header, then 0xA5 again
            uint32_t copy = FRAME[8] | FRAME[9] << 8 | (uint32_t)FRAME[10] << 16;
            int ok = (FRAME[12] | FRAME[13] << 8) == DEMO_BSS_SIZE &&
                     copy + DEMO_BSS_SIZE < OVL_WIN_FRAME;
            for (uint32_t i = 0; ok && i <= DEMO_BSS_SIZE; i++)
                ok = FRAME[copy + i] == (i < DEMO_BSS_SIZE ? 0 : 0xA5);
            if (!ok)
                fail |= 1u << k;
        }
    }
    if (OVL_CALL(OVL_TEST_A, A_SUM5, a_sum5_fn, 1, 1, 1, 1, 1) != 15)
        fail |= 1u << ERR_RECOVER;
    return fail;
}
