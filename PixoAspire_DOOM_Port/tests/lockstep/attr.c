// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Pixel attribution of the 3D view for the frame test (tests/host/frametest.py).
// Linked into the oracle with --wrap on the column and span drawers,
// R_ClearPlanes (start of a view) and R_DrawMasked (end of a view), the
// engine writes the same files from its strips (host/rattr_host.c). For every
// view it writes the finished view pixels and, per pixel, the class of the
// drawer that wrote it last:
//   0 nothing, 1 column (walls, sky, sprites, masked middles, weapon),
//   2 span (floors and ceilings), 3 fuzz, 4 wall or sky pixel read beyond
//   the texture height, 5 sprite or masked pixel read beyond the rebuilt
//   post column (4 and 5 only with dc_texheight and dc_ispost, which the
//   oracle does not have).
// Environment: DMC_ATTR_DIR output directory (unset: off), DMC_ATTR_STRIDE
// every N-th tic (default 1), DMC_ATTR_VIEW "screenblocks,detail" for the
// oracle's view size before it starts (doomgeneric reads no config file).
// File DIR/vTTTTTT.bin: u16 width, u16 height, pixels[width * height],
// classes[width * height].

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AT_W 320
#define AT_H 200

enum { AT_NONE, AT_COLUMN, AT_SPAN, AT_FUZZ, AT_BEYOND, AT_POSTREAD };

extern int dc_x, dc_yl, dc_yh;
extern int ds_y, ds_x1, ds_x2;
extern int viewwindowx, viewwindowy, viewheight, scaledviewwidth;
extern int detailshift;
extern int gametic;
extern unsigned char *I_VideoBuffer;

static uint8_t at_class[AT_W * AT_H];

extern int screenblocks, detailLevel;

__attribute__((constructor)) static void at_view(void)
{
    const char *v = getenv("DMC_ATTR_VIEW");
    if (v)
        sscanf(v, "%d,%d", &screenblocks, &detailLevel);
}
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

extern int centery;
extern int dc_texturemid, dc_iscale;
extern unsigned char *dc_source;
extern int dc_texheight __attribute__((weak));
extern int dc_ispost __attribute__((weak));

// DMC_ATTR_PIXEL=x,y,tic prints what the column drawer reads for that pixel.
static void at_debug(int x0, int w, void *caller)
{
    static int px = -1, py, pt;
    if (px == -1) {
        const char *e = getenv("DMC_ATTR_PIXEL");
        px = -2;
        if (e)
            sscanf(e, "%d,%d,%d", &px, &py, &pt);
    }
    if (px < 0 || gametic != pt || py < dc_yl || py > dc_yh || px < x0 || px >= x0 + w)
        return;
    int frac = dc_texturemid + (py - centery) * dc_iscale;
    int row = (frac >> 16) & 127;
    fprintf(stderr, "pixel %d,%d tic %d: caller %p yl %d yh %d texturemid %d iscale %d row %d "
            "src[row-2..row+2] %d %d [%d] %d %d\n", px, py, pt, caller, dc_yl, dc_yh,
            dc_texturemid, dc_iscale, row, row >= 2 ? dc_source[row - 2] : -1,
            row >= 1 ? dc_source[row - 1] : -1, dc_source[row], dc_source[row + 1],
            dc_source[row + 2]);
    fprintf(stderr, "  bytes from source-3:");
    for (int k = -3; k < 24; k++)
        fprintf(stderr, " %d", dc_source[k]);
    fprintf(stderr, "\n");
}

static void at_col(int cls, int x0, int w)
{
    if (dc_yh < dc_yl)
        return;
    int limit = &dc_texheight ? dc_texheight : 1 << 30;
    int beyond = &dc_ispost && dc_ispost ? AT_POSTREAD : AT_BEYOND;
    for (int y = dc_yl; y <= dc_yh; y++) {
        // the row the drawer reads for this pixel
        int row = ((dc_texturemid + (y - centery) * dc_iscale) >> 16) & 127;
        int c = cls == AT_COLUMN && row >= limit ? beyond : cls;
        for (int k = 0; k < w; k++) {
            int x = viewwindowx + x0 + k, yy = viewwindowy + y;
            if (x >= 0 && x < AT_W && yy >= 0 && yy < AT_H)
                at_class[yy * AT_W + x] = (uint8_t)c;
        }
    }
}

static void at_span(int w)
{
    for (int x = ds_x1; x <= ds_x2; x++)
        for (int k = 0; k < w; k++) {
            int xx = viewwindowx + x * w + k, yy = viewwindowy + ds_y;
            if (xx >= 0 && xx < AT_W && yy >= 0 && yy < AT_H)
                at_class[yy * AT_W + xx] = AT_SPAN;
        }
}

#define AT_WRAP_COL(name, cls, w)              \
    void __real_##name(void);                  \
    void __wrap_##name(void)                   \
    {                                          \
        at_debug(dc_x * (w), (w), __builtin_return_address(0)); \
        if (at_on())                           \
            at_col(cls, dc_x * (w), (w));      \
        __real_##name();                       \
    }

AT_WRAP_COL(R_DrawColumn, AT_COLUMN, 1)
AT_WRAP_COL(R_DrawColumnLow, AT_COLUMN, 2)
AT_WRAP_COL(R_DrawTranslatedColumn, AT_COLUMN, 1)
AT_WRAP_COL(R_DrawTranslatedColumnLow, AT_COLUMN, 2)
AT_WRAP_COL(R_DrawFuzzColumn, AT_FUZZ, 1)
AT_WRAP_COL(R_DrawFuzzColumnLow, AT_FUZZ, 2)

void __real_R_DrawSpan(void);
void __wrap_R_DrawSpan(void)
{
    if (at_on())
        at_span(1);
    __real_R_DrawSpan();
}

void __real_R_DrawSpanLow(void);
void __wrap_R_DrawSpanLow(void)
{
    if (at_on())
        at_span(2);
    __real_R_DrawSpanLow();
}

void __real_R_ClearPlanes(void);
void __wrap_R_ClearPlanes(void)
{
    if (at_on())
        memset(at_class, 0, sizeof(at_class));
    __real_R_ClearPlanes();
}

void __real_R_DrawMasked(void);
void __wrap_R_DrawMasked(void)
{
    __real_R_DrawMasked();
    if (!at_on())
        return;
    char path[1024];
    snprintf(path, sizeof(path), "%s/v%06d.bin", at_dir, gametic);
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "cannot write %s\n", path);
        exit(2);
    }
    uint16_t hdr[2] = { AT_W, AT_H };
    fwrite(hdr, sizeof(hdr), 1, f);
    fwrite(I_VideoBuffer, 1, AT_W * AT_H, f);
    fwrite(at_class, 1, AT_W * AT_H, f);
    fclose(f);
}
