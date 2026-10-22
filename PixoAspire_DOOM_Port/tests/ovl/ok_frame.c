// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Packer test fixture: an automap overlay that forms its compose strip address from the
// linker symbol __frame_start (FRAME_BASE + OVL_WIN_AUTOMAP). pack_overlays.py must accept it.

#include "ovl.h"
#include "platform.h"

static uint8_t *strip(void) { return __frame_start + OVL_WIN_AUTOMAP; }

typedef uint8_t *strip_fn(void);
OVL_TABLE(AUTOMAP, OVL_ENTRY(0, strip_fn, strip));
