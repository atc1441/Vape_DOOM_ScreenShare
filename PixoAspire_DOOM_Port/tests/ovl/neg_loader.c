// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Packer test fixture: an overlay that calls the loader (it would load another overlay over
// itself). It links, pack_overlays.py must refuse it.

#include "ovl.h"

static int load_other(int id) { return ovl_load(id); }

typedef int load_other_fn(int id);
OVL_TABLE(FINALE, OVL_ENTRY(0, load_other_fn, load_other));
