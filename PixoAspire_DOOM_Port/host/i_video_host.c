// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Host display: the I_* video interface and the LCD pump of lcd.h on an
// emulated 320x240 landscape screen, as the meter shows it. The panel keeps
// the palette index of every pixel and the RGB565 the pump LUT gave it.
// Submitted strips are converted as late as the device may convert them:
// when pump_sync or pump_end waits for them, or when a fifth is submitted,
// so a strip buffer written again while still pending shows up as wrong
// pixels. A submit without a window, a window opened over an open one and
// a window that ends before all its columns came are errors.
//
// Options (renderer and display, taken by R_HostOption):
//   --frame-out DIR         the whole screen as indexed PNG, DIR/fNNNNNN.png
//   --frame-stride N        frame every N tics (default 1)
//   --view-hash FILE        per frame: gametic and a hash of the 3D view and
//                           of the status band (palette indices)
//   --view-cols FILE        per level frame: gametic, the hash of the 3D view in the
//                           firmware's order (columns from the left, rows 0..199 of
//                           each, palette indices) and the hash of its RGB565 pixels in
//                           the same order (low byte first)
//   --strip-w N             strip width 1..160 (default: the map's plan)
//   --solid-floors 0|1      plan D floors (default: the map's plan)
//   --flat-slots K          flat cache slots 0..4 (default: the map's plan)
//   --no-lod                walls at their stored base level
//   --strip-clear           strips start cleared (pixels nothing draws)
//   --rstats-out FILE       per-frame renderer counters (port/render/r_strip.h)
//   --frame-report          frame region per plan and strip width, then exit
//   --lcd-check             after every frame, every pixel of the screen must
//                           show the current palette (a palette change sends
//                           the whole status band again, the picture frozen
//                           under the menu outside its panel keeps the palette
//                           it was sent with) and the HUD message line must be
//                           the message widget as the vanilla code draws it
//                           into a screen, exit 3 otherwise
//   --screen-hash FILE      per frame outside a level: gametic and a hash of
//                           the screen rows 0..199 and of the band rows
//   --screen-full           after every frame of a front-end screen or the
//                           menu, the same screen or the menu's panel composed
//                           whole into a second panel must show what the
//                           changed groups left on the panel, exit 3 otherwise
//   --panel-out FILE        per frame that shows the menu: gametic,
//                           menuactive, messageToPrint, the page, its item,
//                           the message, the panel x0 x1 y0 y1 and gamestate

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "f_finale.h"
#include "hu_stuff.h"
#include "i_video.h"
#include "m_menu.h"
#include "r_local.h"
#include "v_video.h"
#include "wi_stuff.h"
#include "z_zone.h"

#include "fe.h"
#include "fetch.h"
#include "lcd.h"
#include "nor_file.h"
#include "png.h"
#include "r_strip.h"
#include "stats.h"
#include "trig.h"

extern boolean tempscreen;

// Temporary screen for the automap and the pause picture over the view
// (D_Display). The status band and the front-end screens never use it.
byte *I_VideoBuffer;

uint16_t pump_lut[256];
volatile struct pump_stats pump_stats;

static uint8_t lcd_idx[LCD_H][LCD_W], full_idx[LCD_H][LCD_W];
static uint16_t lcd_rgb[LCD_H][LCD_W], full_rgb[LCD_H][LCD_W];
// the panel the pump writes: lcd_*, or full_* for --screen-full
static uint8_t (*pidx)[LCD_W] = lcd_idx;
static uint16_t (*prgb)[LCD_W] = lcd_rgb;
static uint8_t palette[768];
static int wx0, wx1, wy0, wh, cx, reps, wopen, wsent;

// strips submitted and not converted yet
#define PUMP_QUEUE 4
static struct {
    const uint8_t *cols;
    int n;
} pq[PUMP_QUEUE];
static int pqn;

static const char *frame_dir;
static int frame_stride = 1;
static FILE *view_hash, *view_cols, *rstats_out, *screen_hash, *panel_out;
static int lcd_check, palettes, hud_frames, hud_text, screen_full, full_frames;

#ifdef DMC_RSTATS
static void stats_line(void);
#endif

// ---------------------------------------------------------------- pump

static void pump_fail(const char *what)
{
    fprintf(stderr, "pump: %s (tic %d)\n", what, gametic);
    exit(1);
}

void pump_begin(int x0, int x1, int y0, int y1, int flags)
{
    int h = y1 - y0 + 1;
    if (h % 4 || h > 200 || x0 < 0 || x1 >= LCD_W || x1 < x0 || y0 < 0 || y1 >= LCD_H) {
        fprintf(stderr, "pump_begin: bad window %d..%d x %d..%d\n", x0, x1, y0, y1);
        exit(1);
    }
    if (wopen)
        pump_fail("pump_begin with a window open");
    wopen = 1;
    wsent = 0;
    wx0 = x0;
    wx1 = x1;
    wy0 = y0;
    wh = h;
    cx = x0;
    reps = (flags & PUMP_DOUBLE) ? 2 : 1;
}

// The oldest pending strip goes to the panel.
static void pump_convert(void)
{
    const uint8_t *cols = pq[0].cols;
    for (int n = pq[0].n; n > 0; n--, cols += wh)
        for (int r = 0; r < reps; r++) {
            for (int i = 0; i < wh; i++) {
                pidx[wy0 + i][cx] = cols[i];
                prgb[wy0 + i][cx] = pump_lut[cols[i]];
            }
            pump_stats.fills += 2;
            wsent++;
            if (++cx > wx1)
                cx = wx0;  // the panel wraps to the window start
        }
    memmove(&pq[0], &pq[1], sizeof(pq[0]) * (size_t)--pqn);
}

void pump_submit(const uint8_t *cols, int ncols)
{
    if (!wopen)
        pump_fail("pump_submit without a window");
    if (pqn == PUMP_QUEUE)
        pump_convert();
    pq[pqn].cols = cols;
    pq[pqn].n = ncols;
    pqn++;
}

void pump_sync(int max)
{
    while (pqn > (max < 0 ? 0 : max))
        pump_convert();
}

int pump_pending(void)
{
    return pqn;
}

void pump_end(void)
{
    if (!wopen)
        pump_fail("pump_end without a window");
    pump_sync(0);
    if (wsent != wx1 - wx0 + 1) {
        fprintf(stderr, "pump: window %d..%d x %d..%d ends after %d of its %d columns (tic %d)\n",
                wx0, wx1, wy0, wy0 + wh - 1, wsent, wx1 - wx0 + 1, gametic);
        exit(1);
    }
    wopen = 0;
}

// ---------------------------------------------------------------- video

void I_InitGraphics(void)
{
    I_VideoBuffer = Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
    memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT);
}

void I_StartFrame(void)
{
}

void I_StartTic(void)
{
}

void I_UpdateNoBlit(void)
{
}

// Palettes are RGB565 in the image: the pump LUT takes them as they are,
// the PNG frames get them widened. The status band is sent again.
void I_SetPalette(int pal)
{
    R_PaletteChanged();
    pal_read(pal, pump_lut);
    for (int i = 0; i < 256; i++) {
        uint16_t c = pump_lut[i];
        palette[3 * i] = (uint8_t)((c >> 11) * 255 / 31);
        palette[3 * i + 1] = (uint8_t)(((c >> 5) & 63) * 255 / 63);
        palette[3 * i + 2] = (uint8_t)((c & 31) * 255 / 31);
    }
    palettes++;
}

// The temporary screen goes to the 3D window in strips of 8 columns.
static void push_screen(void)
{
    static uint8_t cols[8 * SCREENHEIGHT];
    R_PumpWindow(0, SCREENWIDTH - 1, 0, SCREENHEIGHT - 1, 0);
    for (int x = 0; x < SCREENWIDTH; x += 8) {
        pump_sync(0);  // the one buffer is free again
        for (int i = 0; i < 8; i++)
            for (int y = 0; y < SCREENHEIGHT; y++)
                cols[i * SCREENHEIGHT + y] = I_VideoBuffer[y * SCREENWIDTH + x + i];
        pump_submit(cols, 8);
    }
}

static uint64_t fnv(uint64_t h, const uint8_t *p, size_t n)
{
    while (n--)
        h = (h ^ *p++) * 0x100000001b3ull;
    return h;
}

// The HUD message line of the LCD against the message widget drawn by the
// vanilla V_DrawPatch path into a cleared screen.
static void hud_check(void)
{
    static uint8_t scr[SCREENWIDTH * SCREENHEIGHT];
    int h = LCD_BAR_Y - LCD_HUD_Y;
    memset(scr, 0, sizeof(scr));
    V_UseBuffer(scr);
    HU_DrawBand();
    V_RestoreBuffer();
    for (int y = 0; y < h; y++)
        for (int x = 0; x < LCD_W; x++)
            if (lcd_idx[LCD_HUD_Y + y][x] != scr[(HU_MSGY + y) * SCREENWIDTH + x]) {
                fprintf(stderr, "lcd-check: tic %d HUD line pixel %d,%d is %d, the widget draws %d\n",
                        gametic, x, LCD_HUD_Y + y, lcd_idx[LCD_HUD_Y + y][x],
                        scr[(HU_MSGY + y) * SCREENWIDTH + x]);
                exit(3);
            }
    hud_frames++;
    for (int i = 0; i < h * SCREENWIDTH; i++)
        if (scr[HU_MSGY * SCREENWIDTH + i]) {
            hud_text++;
            break;
        }
}

// The menu's panel: rows 0..199 of the LCD that the menu covers, empty without the menu
static int panel(int *x0, int *x1, int *y0, int *y1)
{
    if (!menuactive && !messageToPrint)
        return 0;
    *x0 = 2 * fe.px0;
    *x1 = 2 * fe.px1 + 1;
    *y0 = fe.py0;
    *y1 = fe.py1;
    return *x0 <= *x1 && *y0 <= *y1;
}

// --screen-full: the front-end screen of this frame composed again with every group dirty,
// into the second panel, against the panel, or the menu's panel alone when the menu is up.
// The drawers take no random numbers and change no game state, the hashes of the screen sent
// last are kept.
static void full_check(void)
{
    int menu = menuactive || messageToPrint;
    void (*draw)(void) = menu ? M_Drawer
                       : gamestate == GS_INTERMISSION ? WI_Drawer
                       : gamestate == GS_FINALE ? F_Drawer
                       : gamestate == GS_DEMOSCREEN ? D_PageDrawer : NULL;
    int x0 = 0, x1 = LCD_W - 1, y0 = 0, y1 = SCREENHEIGHT - 1;
    if (!draw || (fe.flags & FE_VIEW) || (menu && !panel(&x0, &x1, &y0, &y1)))
        return;
    uint32_t hash[arrlen(fe_scr.hash)];
    uint8_t flags = fe.flags;
    memcpy(hash, fe_scr.hash, sizeof(hash));
    fe.flags &= ~FE_HASHED;
    pidx = full_idx;
    prgb = full_rgb;
    draw();
    R_PumpIdle();
    pidx = lcd_idx;
    prgb = lcd_rgb;
    memcpy(fe_scr.hash, hash, sizeof(hash));
    fe.flags = flags;
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++)
            if (full_idx[y][x] != lcd_idx[y][x]) {
                fprintf(stderr, "screen-full: tic %d pixel %d,%d is %d, the whole screen has %d\n",
                        gametic, x, y, lcd_idx[y][x], full_idx[y][x]);
                exit(3);
            }
    full_frames++;
}

void I_FinishUpdate(void)
{
    if (tempscreen && I_VideoBuffer)
        push_screen();
    R_PumpIdle();
#ifdef DMC_RSTATS
    stats_line();
#endif
    HS_End(HS_UI, gametic, gamestate == GS_LEVEL ? 0 : 2);
    // the picture frozen under the menu (the view, or the screen of any other gamestate) keeps
    // the palette it was sent with outside the panel
    int px0, px1, py0, py1;
    int frozen = menuactive || messageToPrint;
    if (!frozen || !panel(&px0, &px1, &py0, &py1))
        px0 = py0 = 0, px1 = py1 = -1;
    if (panel_out && frozen)
        fprintf(panel_out, "%d %d %d %d %d %d %d %d %d %d %d\n", gametic, menuactive, messageToPrint,
                fe.menu, fe.item, fe.msg, 2 * fe.px0, 2 * fe.px1 + 1, fe.py0, fe.py1, gamestate);
    if (lcd_check)
        for (int y = 0; y < LCD_H; y++)
            for (int x = 0; x < LCD_W; x++)
                if (lcd_rgb[y][x] != pump_lut[lcd_idx[y][x]]
                    && !(frozen && y < SCREENHEIGHT && !(x >= px0 && x <= px1 && y >= py0 && y <= py1))) {
                    fprintf(stderr, "lcd-check: tic %d pixel %d,%d shows an old palette"
                            " (%d palette changes)\n", gametic, x, y, palettes);
                    exit(3);
                }
    if (lcd_check && gamestate == GS_LEVEL && gametic)
        hud_check();
    if (screen_full)
        full_check();
    if (view_hash) {
        uint64_t hv = fnv(0xcbf29ce484222325ull, &lcd_idx[0][0], LCD_VIEW_H * LCD_W);
        uint64_t hb = fnv(0xcbf29ce484222325ull, &lcd_idx[LCD_VIEW_H][0], (LCD_H - LCD_VIEW_H) * LCD_W);
        fprintf(view_hash, "%d %016llx %016llx %d\n", gametic, (unsigned long long)hv,
                (unsigned long long)hb, gamestate == GS_LEVEL && !tempscreen);
    }
    if (view_cols && gamestate == GS_LEVEL && !tempscreen) {
        uint64_t hi = 0xcbf29ce484222325ull, hr = hi;
        for (int x = 0; x < LCD_W; x++)
            for (int y = 0; y < LCD_VIEW_H; y++) {
                hi = (hi ^ lcd_idx[y][x]) * 0x100000001b3ull;
                hr = (hr ^ (lcd_rgb[y][x] & 0xFF)) * 0x100000001b3ull;
                hr = (hr ^ (lcd_rgb[y][x] >> 8)) * 0x100000001b3ull;
            }
        fprintf(view_cols, "%d %016llx %016llx\n", gametic, (unsigned long long)hi,
                (unsigned long long)hr);
    }
    if (screen_hash && gamestate != GS_LEVEL) {
        uint64_t hs = fnv(0xcbf29ce484222325ull, &lcd_idx[0][0], SCREENHEIGHT * LCD_W);
        uint64_t hb = fnv(0xcbf29ce484222325ull, &lcd_idx[SCREENHEIGHT][0], (LCD_H - SCREENHEIGHT) * LCD_W);
        fprintf(screen_hash, "%d %016llx %016llx\n", gametic, (unsigned long long)hs,
                (unsigned long long)hb);
    }
    if (!frame_dir || gametic % frame_stride)
        return;
    char path[1024];
    snprintf(path, sizeof(path), "%s/f%06d.png", frame_dir, gametic);
    if (png_write_indexed(path, LCD_W, LCD_H, &lcd_idx[0][0], palette)) {
        fprintf(stderr, "cannot write %s\n", path);
        exit(2);
    }
}

// ---------------------------------------------------------------- statistics

#ifdef DMC_RSTATS
static nor_count_t stat_nor;

// per-strip pool use: count of strips per value
#define HIST_MAX 4800
static uint32_t hist[4][HIST_MAX + 1];

void R_StatsStrip(int drawsegs, int visplanes, int vissprites, int openings)
{
    int v[4] = {drawsegs, visplanes, vissprites, openings};
    for (int i = 0; i < 4; i++)
        hist[i][v[i] > HIST_MAX ? HIST_MAX : v[i]]++;
}

// Queued (async) reads only: a record miss inside a column request is a
// sync read and counts as blocking.
void R_StatsNor(uint32_t *tx, uint32_t *bytes)
{
    *tx = (uint32_t)(nor_count.tx - nor_count.sync_tx);
    *bytes = (uint32_t)(nor_count.bytes - nor_count.sync_bytes);
}

// decoder calls at the end of the last view and at the start of this one
static uint32_t trig_end[DMC_TRIG_KINDS], trig_begin[DMC_TRIG_KINDS];

void R_StatsBegin(void)
{
    stat_nor = nor_count;
    memcpy(trig_begin, dmc_trig_calls, sizeof(trig_begin));
    r_stats.logic_sin = trig_begin[DMC_TRIG_SINE] - trig_end[DMC_TRIG_SINE];
    r_stats.logic_atan = trig_begin[DMC_TRIG_ATAN] - trig_end[DMC_TRIG_ATAN];
}

#define RS_FIELDS(X)                                                                         \
    X(nodes) X(subsectors) X(segs) X(frags) X(frags_dropped) X(sprites) X(sprites_dropped) \
    X(strips) X(splits) X(vanilla_drops) X(replays) X(wallcols) X(wallpx) X(skycols)        \
    X(skypx) X(planes) X(spans) X(spanpx) X(solidcols) X(solidpx) X(vissprites) X(sprcols) \
    X(maskedcols) X(pspcols) X(colpx) X(fuzzpx) X(nor_tx) X(nor_bytes) X(colreq) X(colhit)    \
    X(coltx) X(colbytes) X(colwait) X(hw_drawsegs)                                         \
    X(hw_visplanes) X(hw_vissprites) X(hw_openings) X(band_cols) X(band_pieces)             \
    X(band_nor_tx) X(band_nor_bytes) X(trig_sin) X(trig_tan) X(trig_atan) X(logic_sin)    \
    X(logic_atan) X(ctx_fills) X(post_fills) X(col_direct) X(col_wrap) X(col_lv2)          \
    X(fragc_hits) X(solid_runs) X(solid_fillpx)

static int stats_pending;

void R_StatsEnd(void)
{
    r_stats.nor_tx = (uint32_t)(nor_count.tx - stat_nor.tx);
    r_stats.nor_bytes = (uint32_t)(nor_count.bytes - stat_nor.bytes);
    stat_nor = nor_count;
    r_stats.trig_sin = dmc_trig_calls[DMC_TRIG_SINE] - trig_begin[DMC_TRIG_SINE];
    r_stats.trig_tan = dmc_trig_calls[DMC_TRIG_TANGENT] - trig_begin[DMC_TRIG_TANGENT];
    r_stats.trig_atan = dmc_trig_calls[DMC_TRIG_ATAN] - trig_begin[DMC_TRIG_ATAN];
    memcpy(trig_end, dmc_trig_calls, sizeof(trig_end));
    stats_pending = 1;
}

// The line of a frame, written when the frame (and its status band) ends.
static void stats_line(void)
{
    if (!stats_pending)
        return;
    stats_pending = 0;
    r_stats.band_nor_tx = (uint32_t)(nor_count.tx - stat_nor.tx);
    r_stats.band_nor_bytes = (uint32_t)(nor_count.bytes - stat_nor.bytes);
    if (!rstats_out)
        return;
    static int header;
    if (!header) {
        header = 1;
        fprintf(rstats_out, "# tic w detail textured");
#define RS_NAME(f) fprintf(rstats_out, " %s", #f);
        RS_FIELDS(RS_NAME)
        fprintf(rstats_out, "\n");
    }
    fprintf(rstats_out, "%d %d %d %d", gametic, R_StripWidth(), detailshift, !r_solid);
#define RS_VAL(f) fprintf(rstats_out, " %u", (unsigned)r_stats.f);
    RS_FIELDS(RS_VAL)
    fprintf(rstats_out, "\n");
}
#endif

// ---------------------------------------------------------------- options

static void frame_report(void)
{
    static const struct {
        const char *name;
        int w, textured, budget;
    } plans[] = {{"A", 16, 1, 15208}, {"B", 8, 1, 9648}, {"C", 8, 1, 9648}, {"D", 8, 0, 8368}};
    int bad = 0;
    printf("frame region (bytes) per plan: strips + pass 1 + pools + strip arrays = total"
           " (budget)\n");
    for (unsigned i = 0; i < sizeof(plans) / sizeof(plans[0]); i++) {
        r_layout_t l;
        R_Layout(&l, plans[i].w, plans[i].textured);
        unsigned region = (l.total + 15u) & ~15u;  // the caches start 16 B aligned
        int over = (int)region > plans[i].budget;
        bad |= over;
        printf("plan %s W%-2d %s: %u + %u + %u + %u = %u, region %u (budget %d) %s\n",
               plans[i].name, plans[i].w, plans[i].textured ? "textured" : "solid   ",
               (unsigned)l.strip_bytes, (unsigned)l.pass1_bytes, (unsigned)l.pool_bytes,
               (unsigned)l.array_bytes, (unsigned)l.total, region, plans[i].budget,
               over ? "OVER" : "ok");
        printf("    pools: drawsegs %u x %u, visplanes %u x %u, vissprites %u x %u, openings %u x 2\n",
               l.pools.drawsegs, (unsigned)sizeof(drawseg_t), l.pools.visplanes,
               (unsigned)l.vp_size, l.pools.vissprites, (unsigned)sizeof(vissprite_t),
               l.pools.openings);
    }
    int wmax = 0;
    for (int w = 1; w <= R_MAXW; w++) {
        r_layout_t l;
        if (!R_Layout(&l, w, 1))
            wmax = w;
    }
    printf("the host frame region has %u B, enough for strips of up to %d columns\n",
           (unsigned)R_FRAME_HOST, wmax);
    printf("%s\n", bad ? "FAIL: a plan is over its budget" : "PASS");
    exit(bad);
}

int R_HostOption(int argc, char **argv, int *i)
{
    const char *a = argv[*i];
    int more = argc - *i - 1;
    if (!strcmp(a, "--frame-out") && more >= 1) {
        frame_dir = argv[++*i];
    } else if (!strcmp(a, "--frame-stride") && more >= 1) {
        frame_stride = atoi(argv[++*i]);
        if (frame_stride < 1)
            frame_stride = 1;
    } else if (!strcmp(a, "--view-hash") && more >= 1) {
        view_hash = fopen(argv[++*i], "w");
        if (!view_hash) {
            fprintf(stderr, "cannot open %s\n", argv[*i]);
            exit(2);
        }
        setvbuf(view_hash, NULL, _IOFBF, 1 << 16);
    } else if (!strcmp(a, "--view-cols") && more >= 1) {
        view_cols = fopen(argv[++*i], "w");
        if (!view_cols) {
            fprintf(stderr, "cannot open %s\n", argv[*i]);
            exit(2);
        }
    } else if (!strcmp(a, "--strip-w") && more >= 1) {
        int w = atoi(argv[++*i]);
        if (w < 1 || w > R_MAXW) {
            fprintf(stderr, "--strip-w: 1..%d\n", R_MAXW);
            exit(2);
        }
        R_SetStripWidth(w);
    } else if (!strcmp(a, "--solid-floors") && more >= 1) {
        R_SetSolidFloors(atoi(argv[++*i]) != 0);
    } else if (!strcmp(a, "--flat-slots") && more >= 1) {
        R_SetFlatSlots(atoi(argv[++*i]));
    } else if (!strcmp(a, "--no-lod")) {
        r_walllod = 0;
    } else if (!strcmp(a, "--strip-clear")) {
        r_stripclear = 1;
    } else if (!strcmp(a, "--rstats-out") && more >= 1) {
#ifdef DMC_RSTATS
        rstats_out = fopen(argv[++*i], "w");
        if (!rstats_out) {
            fprintf(stderr, "cannot open %s\n", argv[*i]);
            exit(2);
        }
#else
        fprintf(stderr, "--rstats-out: built without DMC_RSTATS\n");
        exit(2);
#endif
    } else if (!strcmp(a, "--lcd-check")) {
        lcd_check = 1;
    } else if (!strcmp(a, "--screen-hash") && more >= 1) {
        screen_hash = fopen(argv[++*i], "w");
        if (!screen_hash) {
            fprintf(stderr, "cannot open %s\n", argv[*i]);
            exit(2);
        }
        setvbuf(screen_hash, NULL, _IOFBF, 1 << 16);
    } else if (!strcmp(a, "--screen-full")) {
        screen_full = 1;
    } else if (!strcmp(a, "--panel-out") && more >= 1) {
        panel_out = fopen(argv[++*i], "w");
        if (!panel_out) {
            fprintf(stderr, "cannot open %s\n", argv[*i]);
            exit(2);
        }
    } else if (!strcmp(a, "--frame-report")) {
        frame_report();
    } else {
        return 0;
    }
    return 1;
}

void R_HostClose(void)
{
    if (lcd_check)
        printf("lcd-check: every frame shows its palette, %d palette changes, HUD line as"
               " drawn by the widget in %d frames (%d with a message)\n", palettes, hud_frames,
               hud_text);
    if (screen_full)
        printf("screen-full: %d frames of front-end screens as composed whole\n", full_frames);
    if (view_hash)
        fclose(view_hash);
    if (view_cols)
        fclose(view_cols);
    if (screen_hash)
        fclose(screen_hash);
    if (panel_out)
        fclose(panel_out);
    panel_out = NULL;
#ifdef DMC_RSTATS
    // per-strip histograms: H name value count ...
    static const char *const names[4] = {"drawsegs", "visplanes", "vissprites", "openings"};
    for (int i = 0; rstats_out && i < 4; i++) {
        fprintf(rstats_out, "H %s", names[i]);
        for (int k = 0; k <= HIST_MAX; k++)
            if (hist[i][k])
                fprintf(rstats_out, " %d %u", k, (unsigned)hist[i][k]);
        fprintf(rstats_out, "\n");
    }
#endif
    if (rstats_out)
        fclose(rstats_out);
    view_hash = view_cols = rstats_out = screen_hash = NULL;
}
