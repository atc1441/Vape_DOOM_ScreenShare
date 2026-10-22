// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Packer test fixture: an automap overlay of about 6 KB. It links (under FRAME_MIN) but is
// over the 5,312 B automap window, so pack_overlays.py must refuse it.

#include "ovl.h"

static const uint8_t big[6000] = {1, 2, 3};

static int big_get(int i) { return big[i]; }

typedef int big_get_fn(int i);
OVL_TABLE(AUTOMAP, OVL_ENTRY(0, big_get_fn, big_get));
