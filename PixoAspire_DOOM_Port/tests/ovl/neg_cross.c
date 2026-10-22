// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Packer test fixture: an overlay that uses a symbol of overlay A. Overlays link against
// main.elf only, so this link must fail with an undefined symbol.

#include "ovl.h"

extern const ovl_fn ovl_tab_TEST_A[];

static ovl_fn other(int i) { return ovl_tab_TEST_A[i]; }

typedef ovl_fn other_fn(int i);
OVL_TABLE(WI, OVL_ENTRY(0, other_fn, other));
