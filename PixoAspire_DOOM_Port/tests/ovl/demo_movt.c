// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Resident demo code built with -O2, where clang loads addresses with MOVW/MOVT instead of
// literal words. demo_defer only stores the address of a function that loads an overlay, the
// packer must still follow it (tests/ovl/neg_movt.c).

#include "ovl_demo.h"

void (*volatile demo_later)(void);

static void demo_later_load(void) { ovl_load(OVL_TEST_B); }

__attribute__((used, retain)) void demo_defer(void) { demo_later = demo_later_load; }
