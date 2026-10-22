// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Packer test fixture: an overlay that calls a resident function (demo_defer, built with -O2)
// which takes the address of a loader caller with MOVW/MOVT. It links, pack_overlays.py must
// refuse it.

#include "ovl_demo.h"

static void defer(void) { demo_defer(); }

typedef void defer_fn(void);
OVL_TABLE(MENU, OVL_ENTRY(0, defer_fn, defer));
