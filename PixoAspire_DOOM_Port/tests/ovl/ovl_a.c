// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Test overlay A: argument passing, return values, rodata, a jump table, a resident
// callback, and state kept in a resident struct.

#include "ovl_demo.h"

static int32_t a_sum5(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e)
{
    return a + 2 * b + 3 * c + 4 * d + 5 * e;
}

static uint64_t a_mul64(uint32_t a, uint32_t b) { return (uint64_t)a * b; }

static struct demo_trio a_trio(int32_t x) { return (struct demo_trio){x, x * x, -x}; }

static const char *a_name(void) { return "overlay A"; }

static int a_classify(int k)
{
    switch (k) {
    case 0:
        return 17;
    case 1:
        return -3;
    case 2:
        return 250;
    case 3:
        return 9;
    case 4:
        return -77;
    case 5:
        return 1000;
    case 6:
        return 4;
    default:
        return 0;
    }
}

static int a_apply(int (*fn)(int), int x) { return fn(x) + 1000; }

static uint32_t a_state(void) { return ++demo_state.a_calls; }

OVL_TABLE(TEST_A,
          OVL_ENTRY(A_SUM5, a_sum5_fn, a_sum5),
          OVL_ENTRY(A_MUL64, a_mul64_fn, a_mul64),
          OVL_ENTRY(A_TRIO, a_trio_fn, a_trio),
          OVL_ENTRY(A_NAME, a_name_fn, a_name),
          OVL_ENTRY(A_CLASSIFY, a_classify_fn, a_classify),
          OVL_ENTRY(A_APPLY, a_apply_fn, a_apply),
          OVL_ENTRY(A_STATE, a_state_fn, a_state));
