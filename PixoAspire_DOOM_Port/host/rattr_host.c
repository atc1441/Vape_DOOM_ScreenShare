// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Pixel attribution of the strip renderer for the frame test
// (tests/host/frametest.py), linked into builds with DMC_ATTR. The drawers
// write a class per pixel next to the strip (port/render/r_strip.h), this
// file gathers the strips of a view into the vanilla 320x200 layout and
// writes the same files as tests/lockstep/attr.c does for the oracle:
// DIR/vTTTTTT.bin = u16 width, u16 height, pixels, classes.
// Environment: DMC_ATTR_DIR output directory (unset: off), DMC_ATTR_STRIDE
// every N-th tic (default 1).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomstat.h"
#include "r_local.h"

#include "r_strip.h"

#define AT_W 320
#define AT_H 200

static uint8_t at_pix[AT_W * AT_H], at_class[AT_W * AT_H];
static const char *at_dir;
static int at_stride = 1, at_init;

static int at_on(void)
{
    if (!at_init) {
        at_init = 1;
        at_dir = getenv("DMC_ATTR_DIR");
        if (getenv("DMC_ATTR_STRIDE"))
            at_stride = atoi(getenv("DMC_ATTR_STRIDE"));
        if (at_stride < 1)
            at_stride = 1;
    }
    return at_dir && gametic % at_stride == 0;
}

void R_AttrStrip(int x0, int n, const uint8_t *pix, const uint8_t *cls)
{
    if (!at_on())
        return;
    int reps = 1 << detailshift;
    for (int i = 0; i < n; i++)
        for (int r = 0; r < reps; r++) {
            int x = ((x0 + i) << detailshift) + r;
            for (int y = 0; y < AT_H; y++) {
                at_pix[y * AT_W + x] = pix[i * R_VIEWH + y];
                at_class[y * AT_W + x] = cls[i * R_VIEWH + y];
            }
        }
}

void R_AttrViewEnd(void)
{
    if (!at_on())
        return;
    char path[1024];
    snprintf(path, sizeof(path), "%s/v%06d.bin", at_dir, gametic);
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "cannot write %s\n", path);
        exit(2);
    }
    uint16_t hdr[2] = {AT_W, AT_H};
    fwrite(hdr, sizeof(hdr), 1, f);
    fwrite(at_pix, 1, sizeof(at_pix), f);
    fwrite(at_class, 1, sizeof(at_class), f);
    fclose(f);
}
