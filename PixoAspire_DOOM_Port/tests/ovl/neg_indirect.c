// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Packer test fixture: an overlay that calls a resident function (demo_calls) which calls the
// loader, so it would load another overlay over itself. It links, pack_overlays.py must
// refuse it.

#include "ovl_demo.h"

static uint32_t run_demo(void) { return demo_calls(); }

typedef uint32_t run_demo_fn(void);
OVL_TABLE(MENU, OVL_ENTRY(0, run_demo_fn, run_demo));
