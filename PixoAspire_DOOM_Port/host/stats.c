// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek

#include <stdio.h>
#include <string.h>

#include "doomstat.h"
#include "p_local.h"

#include "cache.h"
#include "img.h"
#include "nor_file.h"
#include "rec.h"
#include "stats.h"

// Kinds: map streams 0-23, sections 24 + id, 37 header and map header.
#define HS_KINDS (DMC_NSTREAMS + DMC_SECT_MAX + 1)
#define HS_OTHER (HS_KINDS - 1)

static const char *const stream_names[DMC_NSTREAMS] = {
    "NODES", "NODEBB", "SSECTORS", "SEGS", "LINES", "LINESIDES", "SIDES", "SECTORS",
    "SECEXTRA", "SECLINES", "TAGS", "BLOCKMAP", "GRID", "REJECT", "SNDADJ", "THINGS",
    "CELLTHINGS", "SECTHINGS", "ZCLIP", "TELEDEST", "SPECIALS", "MONSTERS", "TEXXLAT",
    "FLATXLAT",
};
static const char *const sect_names[DMC_SECT_MAX] = {
    "-", "PALETTE", "TEXDIR", "TEXDATA", "FLATS", "PATCHDIR", "PATCHDATA", "SPRDEF",
    "SOUNDS", "STRINGS", "UIDIR", "ANIMS", "DEMOS",
};

static FILE *out;
static int ctx = -1;
static nor_count_t start[HS_NCTX], acc[HS_NCTX];
static unsigned long long hits[HS_NCTX][HS_KINDS], misses[HS_NCTX][HS_KINDS];
static unsigned long long rtx[HS_NCTX][HS_KINDS], rbytes[HS_NCTX][HS_KINDS];

// map block bounds of the loaded map, refreshed when the map changes
static uint32_t map_st0, map_end;

// Events of the current tic: LINES entries by thing (player, monster,
// projectile), the entries the tests of other things let through, line
// openings, sight checks and lines the second level gave.
static unsigned ev_entry[3], ev_kept[3], ev_open, ev_sight, ev_l2hit;
static unsigned long long l2_hits[HS_NCTX], l2_fills[HS_NCTX];
static unsigned long long l2_kind[HS_NCTX][HS_KINDS];
static int rc2_mode;
static uint8_t rc2_buf[RC2_BYTES] __attribute__((aligned(4)));

static int classify(uint32_t loff)
{
    if (M.st[0] != map_st0) {
        // the map table reads are not traffic of the period
        nor_count_t keep = nor_count;
        int c = ctx;
        ctx = -1;
        map_st0 = M.st[0];
        map_end = 0;
        for (int k = 0; k < img.nmaps; k++) {
            dmc_mapent_t e;
            img_mapent(k, &e);
            if (e.off <= M.st[0] && M.st[0] < e.off + e.size)
                map_end = e.off + e.size;
        }
        nor_count = keep;
        ctx = c;
    }
    if (loff >= M.st[0] && loff < map_end) {
        int best = 0;
        for (int s = 0; s < DMC_NSTREAMS; s++)
            if (M.st[s] <= loff && M.st[s] >= M.st[best])
                best = s;
        return best;
    }
    for (int id = 1; id < DMC_SECT_MAX; id++)
        if (img.sect[id].size && loff >= img.sect[id].off
            && loff < img.sect[id].off + img.sect[id].size)
            return DMC_NSTREAMS + id;
    return HS_OTHER;
}

void rc_stat_hook(uint32_t loff, int hit)
{
    if (!out || ctx < 0)
        return;
    int k = classify(loff);
    if (hit)
        hits[ctx][k]++;
    else
        misses[ctx][k]++;
}

void img_stat_hook(uint32_t loff, uint32_t n, uint32_t tx)
{
    if (!out || ctx < 0)
        return;
    int k = classify(loff);
    rtx[ctx][k] += tx;
    rbytes[ctx][k] += n;
}

void rc2_stat_hook(uint32_t loff, int hit)
{
    if (!out || ctx < 0)
        return;
    if (hit) {
        l2_hits[ctx]++;
        l2_kind[ctx][classify(loff)]++;
        ev_l2hit++;
    } else {
        l2_fills[ctx]++;
    }
}

void HS_Rc2(int mode)
{
    rc2_mode = mode;
}

static int thing_kind(const mobj_t *thing)
{
    return MO_PLAYER(thing) ? 0 : (thing->flags & MF_MISSILE) ? 2 : 1;
}

// The meter enters the LINES overlay at every cross and shoot call and at the
// use calls past the tests of the resident wrapper (port/game/i_dmc.c), and
// any overlay entry turns the second level off for the rest of the tic.
void HS_Event(int ev, const mobj_t *thing)
{
    switch (ev) {
    case HS_E_THINK:
        if (rc2_mode)
            rc2_begin(rc2_buf);
        break;
    case HS_E_THINK_END:
        rc2_end();
        break;
    case HS_E_CROSS:
    case HS_E_SHOOT:
        ev_entry[thing_kind(thing)]++;
        if (rc2_mode == 2)
            rc2_drop();
        break;
    case HS_E_USE:
        ev_entry[thing_kind(thing)]++;
        ev_kept[thing_kind(thing)]++;
        rc2_drop();
        break;
    case HS_E_HANDLED:
        ev_kept[thing_kind(thing)]++;
        if (rc2_mode == 1)
            rc2_drop();
        break;
    case HS_E_OPENING:
        ev_open++;
        break;
    case HS_E_SIGHT:
        ev_sight++;
        break;
    }
}

int HS_Open(const char *path)
{
    out = fopen(path, "w");
    return out ? 0 : -1;
}

void HS_Begin(int c)
{
    memset(&acc[c], 0, sizeof(acc[c]));
    if (c == HS_TIC) {
        memset(ev_entry, 0, sizeof(ev_entry));
        memset(ev_kept, 0, sizeof(ev_kept));
        ev_open = ev_sight = ev_l2hit = 0;
    }
    HS_Resume(c);
}

void HS_Resume(int c)
{
    ctx = c;
    start[c] = nor_count;
}

void HS_Pause(int c)
{
    acc[c].tx += nor_count.tx - start[c].tx;
    acc[c].bytes += nor_count.bytes - start[c].bytes;
    acc[c].sync_tx += nor_count.sync_tx - start[c].sync_tx;
    start[c] = nor_count;
    ctx = -1;
}

void HS_End(int c, int tic, int flags)
{
    static const char tag[HS_NCTX] = { 'T', 'F', 'U' };
    HS_Pause(c);
    if (out)
        fprintf(out, "%c %d %llu %llu %llu %d\n", tag[c], tic, (unsigned long long)acc[c].tx,
                (unsigned long long)acc[c].bytes, (unsigned long long)acc[c].sync_tx, flags);
    // L tic flags, LINES entries and those let through by player, monster and
    // projectile, line openings, sight checks, lines from the second level
    if (out && c == HS_TIC)
        fprintf(out, "L %d %d %u %u %u %u %u %u %u %u %u\n", tic, flags, ev_entry[0], ev_entry[1],
                ev_entry[2], ev_kept[0], ev_kept[1], ev_kept[2], ev_open, ev_sight, ev_l2hit);
    memset(&acc[c], 0, sizeof(acc[c]));
}

void HS_Close(void)
{
    static const char *const ctxname[HS_NCTX] = { "tic", "frame", "ui" };
    if (!out)
        return;
    // K context kind, line hits, line misses, NOR transactions and bytes,
    // misses the second level gave
    for (int c = 0; c < HS_NCTX; c++)
        for (int k = 0; k < HS_KINDS; k++) {
            const char *name = k < DMC_NSTREAMS ? stream_names[k]
                               : k < HS_OTHER ? sect_names[k - DMC_NSTREAMS] : "OTHER";
            if (hits[c][k] || misses[c][k] || rtx[c][k])
                fprintf(out, "K %s %s %llu %llu %llu %llu %llu\n", ctxname[c], name, hits[c][k],
                        misses[c][k], rtx[c][k], rbytes[c][k], l2_kind[c][k]);
        }
    // second level: lines it gave to misses and lines it took
    for (int c = 0; c < HS_NCTX; c++)
        if (l2_hits[c] || l2_fills[c])
            fprintf(out, "C %s %llu %llu\n", ctxname[c], l2_hits[c], l2_fills[c]);
    fclose(out);
    out = NULL;
}
