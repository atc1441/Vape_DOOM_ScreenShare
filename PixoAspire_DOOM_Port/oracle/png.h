// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Minimal indexed PNG writer (stored deflate blocks, no compression library).

#ifndef DMC_PNG_H
#define DMC_PNG_H

#include <stdint.h>

// Writes a w x h image of 8-bit palette indices with a 256-entry RGB
// palette (768 bytes). Returns 0 on success.
int png_write_indexed(const char *path, int w, int h, const uint8_t *pix, const uint8_t *pal);

#endif
