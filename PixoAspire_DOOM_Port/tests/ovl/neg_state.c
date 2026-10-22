// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Packer test fixture: an overlay with its own .data (neg_calls) and .bss (neg_acc, 64 B
// aligned, so .ovl_bss starts after a gap). pack_overlays.py must refuse it, and the bss word
// of its blob header must cover the gap.

#include "ovl.h"

static int neg_calls = 1;
static uint64_t neg_acc[2] __attribute__((aligned(64)));

static uint32_t step(uint32_t x)
{
    neg_acc[x & 1] += x;
    return (uint32_t)neg_acc[0] + (uint32_t)neg_calls++;
}

typedef uint32_t step_fn(uint32_t x);
OVL_TABLE(FINALE, OVL_ENTRY(0, step_fn, step));
