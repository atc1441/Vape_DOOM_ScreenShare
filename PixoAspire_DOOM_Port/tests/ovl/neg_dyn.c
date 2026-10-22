// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Packer test fixture, linked first into an alternative demo firmware: a resident function
// that moves every other resident address, and a buffer at FRAME_BASE (PLAT_DYN), where
// overlays would overwrite it. pack_overlays.py must refuse that firmware and overlays linked
// against it.

#include "platform.h"

uint8_t neg_dyn_buf[64] PLAT_DYN;

int neg_dyn_fn(int i);
int neg_dyn_fn(int i) { return neg_dyn_buf[i & 63] + i * 3; }
