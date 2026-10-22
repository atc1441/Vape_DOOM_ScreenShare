// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Platform stand-ins of the host overlay tests: the NOR is an image file in memory,
// plat_panic returns to a setjmp point when one is armed, plat_fmt is vsnprintf and lcd_text
// only measures.

#pragma once

#include <setjmp.h>
#include <stdint.h>

extern uint8_t *nor_mem;
extern unsigned nor_reads, failures;
extern jmp_buf panic_jmp;
extern int panic_armed;
extern char panic_msg[64];

// Reads --nor FILE, --build-id HEX40 and --vma HEX into the NOR image, dmc_build_id and
// ovl_host_vma. Exits on errors.
void stub_setup(int argc, char **argv);
void check(int ok, const char *what);
uint32_t rd32(const uint8_t *p);
