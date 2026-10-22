// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Packer test fixture: an overlay that calls demo_drop_frame, an alias of demo_take_frame,
// which calls ovl_invalidate and writes the frame region the overlay runs from. It links,
// pack_overlays.py must refuse it.

#include "ovl_demo.h"

static void drop(void) { demo_drop_frame(); }

typedef void drop_fn(void);
OVL_TABLE(WI, OVL_ENTRY(0, drop_fn, drop));
