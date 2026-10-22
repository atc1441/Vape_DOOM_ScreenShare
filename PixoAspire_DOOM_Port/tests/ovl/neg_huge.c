// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Packer test fixture: an overlay larger than FRAME_MIN. Its link must fail on the ASSERT in
// port/ovl/ovl.ld.

#include "ovl.h"

static const uint8_t huge[9000] = {1, 2, 3};

static int huge_get(int i) { return huge[i]; }

typedef int huge_get_fn(int i);
OVL_TABLE(LEVEL, OVL_ENTRY(0, huge_get_fn, huge_get));
