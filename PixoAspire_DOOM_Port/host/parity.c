// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Accessor-parity mode of the host engine (build/host/dmcdoom-parity).
// The map lumps of the WAD given with -iwad are read with the engine's WAD
// code and turned into reference structures here. This is a separate
// implementation of what doomgeneric's P_Load* and P_GroupLines compute
// (vertices, line sectors, sector line lists, block boxes, sound origins,
// spawned things), written from the lump formats, because the engine's
// p_setup.c no longer loads lumps. Every record the engine reads from the
// image is compared with it: the raw line records, the decoded views, and
// the line specials against a shadow of the vanilla line->special values
// that follows P_SetLineSpecial. Textures, flats and patches are matched by
// name through the pack's names.json (-parnames FILE). The first difference
// ends the run with I_Error.

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_bbox.h"
#include "p_local.h"
#include "r_data.h"
#include "sounds.h"
#include "w_wad.h"

#include "fetch.h"
#include "img.h"

typedef struct {
    int x, y;
} pv_vertex_t;

typedef struct {
    int v1, v2, flags, special, tag, sidenum[2];
    int front, back;  // -1 none
} pv_line_t;

typedef struct {
    int textureoffset, rowoffset, sector;
    char tex[3][9];  // top, bottom, mid
} pv_side_t;

typedef struct {
    int floor, ceiling, light, special, tag;
    char floorpic[9], ceilingpic[9];
    int first, count;  // in pv_seclines
    int blockbox[4];
    fixed_t soundorg[2];
} pv_sector_t;

typedef struct {
    int x, y, angle, type, options;
} pv_thing_t;

typedef struct {
    int v1, v2, angle, linedef, side, offset;
} pv_seg_t;

typedef struct {
    int x, y, dx, dy, bbox[2][4], child[2];
} pv_node_t;

static pv_vertex_t *pv_vert;
static pv_line_t *pv_line;
static pv_side_t *pv_side;
static pv_sector_t *pv_sec;
static pv_seg_t *pv_seg;
static int (*pv_ss)[2];  // numsegs, firstseg
static pv_node_t *pv_node;
static int *pv_seclines;
static short *pv_bmap;
static unsigned char *pv_reject;
static int pv_nlines, pv_nsides, pv_nsecs, pv_nsegs, pv_nss, pv_nnodes, pv_nbmap;
static int *pv_scroller;  // per WAD side: front side of a special 48 line
static int *pv_side_of;   // first WAD side of each unique side record, -1 unknown
static int *pv_special;   // vanilla line->special, cleared as the game clears it
static pv_thing_t *pv_thing;  // things vanilla P_LoadThings hands to P_SpawnMapThing
static int pv_nthings;

static char (*tex_names)[9];
static int ntex_names;
static char (*flat_names)[9];
static int nflat_names;
static char (*patch_names)[9];
static int npatch_names;

static unsigned long long checks;
static int failed;

// checks per kind for the summary at exit
#define PAR_KINDS 64
static const char *kind_name[PAR_KINDS];
static unsigned long long kind_count[PAR_KINDS];

static void par_count(const char *what)
{
    int k;
    checks++;
    for (k = 0; k < PAR_KINDS && kind_name[k] && kind_name[k] != what; k++)
        ;
    if (k < PAR_KINDS) {
        kind_name[k] = what;
        kind_count[k]++;
    }
}

static void par_summary(void)
{
    printf("parity: %llu checks, %s\n", checks, failed ? "stopped at a difference" : "no difference");
    for (int k = 0; k < PAR_KINDS && kind_name[k]; k++)
        printf("parity:   %-32s %llu\n", kind_name[k], kind_count[k]);
}

static void par_fail(const char *what, int a, int b, int c)
{
    failed = 1;
    I_Error("parity: %s (%d %d %d) after %llu checks", what, a, b, c, checks);
}

static void par_check_fail(const char *what, int a, int b, int c)
{
    failed = 1;
    I_Error("parity: check failed: %s (%d %d %d) after %llu checks", what, a, b, c, checks);
}

#define PCHECK(cond, what, a, b, c) \
    do { par_count(what); if (!(cond)) par_check_fail(what, a, b, c); } while (0)

// ---------------------------------------------------------------- names.json

// Reads the string array that follows "key": in the file.
static int read_names(const char *json, const char *key, char (**out)[9])
{
    char pat[32];
    snprintf(pat, sizeof(pat), "\"%s\"", key);
    const char *p = strstr(json, pat);
    int n = 0, cap = 64;
    *out = malloc(cap * 9);
    if (!p || !(p = strchr(p + strlen(pat), '[')))
        return 0;
    p++;
    while (*p && *p != ']') {
        if (*p == '"') {
            const char *e = strchr(p + 1, '"');
            if (n == cap)
                *out = realloc(*out, (cap *= 2) * 9);
            int len = (int)(e - p - 1);
            memset((*out)[n], 0, 9);
            memcpy((*out)[n], p + 1, len > 8 ? 8 : len);
            n++;
            p = e + 1;
        } else {
            p++;
        }
    }
    return n;
}

static void load_names(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        I_Error("parity: cannot read %s", path);
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(n + 1);
    if (fread(buf, 1, n, f) != (size_t)n)
        I_Error("parity: cannot read %s", path);
    buf[n] = 0;
    fclose(f);
    ntex_names = read_names(buf, "textures", &tex_names);
    nflat_names = read_names(buf, "flats", &flat_names);
    npatch_names = read_names(buf, "patches", &patch_names);
    free(buf);
}

static int name_eq(const char *a, const char *b)
{
    return !strncasecmp(a, b, 8);
}

// Name of an engine texture number (global id + 1), "-" for none.
static const char *tex_name(int tex)
{
    if (!tex)
        return "-";
    if (tex - 1 >= ntex_names)
        par_fail("texture number beyond names.json", tex, ntex_names, 0);
    return tex_names[tex - 1];
}

// ---------------------------------------------------------------- loading

static void *lump(int base, int k, int *count, int recsize)
{
    int l = base + k;
    int len = W_LumpLength(l);
    void *p = malloc(len ? len : 1);
    W_ReadLump(l, p);
    *count = len / recsize;
    return p;
}

static short rd16(const unsigned char *p)
{
    return (short)(p[0] | p[1] << 8);
}

static void copyname(char *d, const unsigned char *s)
{
    memset(d, 0, 9);
    memcpy(d, s, 8);
}

void PAR_Init(void)
{
    int p = M_CheckParmWithArgs("-iwad", 1);
    if (!p || !W_AddFile(myargv[p + 1]))
        I_Error("parity: give the WAD with -iwad");
    W_GenerateHashTable();
    p = M_CheckParmWithArgs("-parnames", 1);
    if (!p)
        I_Error("parity: give the pack's names.json with -parnames");
    load_names(myargv[p + 1]);
    atexit(par_summary);
    printf("parity: %d textures, %d flats, %d patches by name\n", ntex_names, nflat_names,
           npatch_names);
}

static void free_level(void)
{
    free(pv_vert);
    free(pv_line);
    free(pv_side);
    free(pv_sec);
    free(pv_seg);
    free(pv_ss);
    free(pv_node);
    free(pv_seclines);
    free(pv_bmap);
    free(pv_reject);
    free(pv_scroller);
    free(pv_side_of);
    free(pv_special);
    free(pv_thing);
}

void PAR_SetupLevel(int episode, int map)
{
    char name[9];
    int n, i, k;
    unsigned char *raw;

    free_level();
    snprintf(name, sizeof(name), "E%dM%d", episode, map);
    int base = W_GetNumForName(name);

    raw = lump(base, ML_VERTEXES, &n, 4);
    pv_vert = malloc(n * sizeof(*pv_vert));
    for (i = 0; i < n; i++) {
        pv_vert[i].x = rd16(raw + 4 * i);
        pv_vert[i].y = rd16(raw + 4 * i + 2);
    }
    free(raw);

    raw = lump(base, ML_SECTORS, &pv_nsecs, 26);
    pv_sec = calloc(pv_nsecs, sizeof(*pv_sec));
    for (i = 0; i < pv_nsecs; i++) {
        const unsigned char *r = raw + 26 * i;
        pv_sec[i].floor = rd16(r);
        pv_sec[i].ceiling = rd16(r + 2);
        copyname(pv_sec[i].floorpic, r + 4);
        copyname(pv_sec[i].ceilingpic, r + 12);
        pv_sec[i].light = rd16(r + 20);
        pv_sec[i].special = rd16(r + 22);
        pv_sec[i].tag = rd16(r + 24);
    }
    free(raw);

    raw = lump(base, ML_SIDEDEFS, &pv_nsides, 30);
    pv_side = malloc(pv_nsides * sizeof(*pv_side));
    for (i = 0; i < pv_nsides; i++) {
        const unsigned char *r = raw + 30 * i;
        pv_side[i].textureoffset = rd16(r);
        pv_side[i].rowoffset = rd16(r + 2);
        copyname(pv_side[i].tex[0], r + 4);
        copyname(pv_side[i].tex[1], r + 12);
        copyname(pv_side[i].tex[2], r + 20);
        pv_side[i].sector = rd16(r + 28);
    }
    free(raw);
    pv_scroller = calloc(pv_nsides, sizeof(int));

    raw = lump(base, ML_LINEDEFS, &pv_nlines, 14);
    pv_line = malloc(pv_nlines * sizeof(*pv_line));
    pv_special = malloc(pv_nlines * sizeof(int));
    for (i = 0; i < pv_nlines; i++) {
        const unsigned char *r = raw + 14 * i;
        pv_line_t *l = &pv_line[i];
        l->v1 = (unsigned short)rd16(r);
        l->v2 = (unsigned short)rd16(r + 2);
        l->flags = rd16(r + 4);
        l->special = rd16(r + 6);
        l->tag = rd16(r + 8);
        l->sidenum[0] = rd16(r + 10);
        l->sidenum[1] = rd16(r + 12);
        l->front = l->sidenum[0] != -1 ? pv_side[l->sidenum[0]].sector : -1;
        l->back = l->sidenum[1] != -1 ? pv_side[l->sidenum[1]].sector : -1;
        if (l->special == 48)
            pv_scroller[l->sidenum[0]] = 1;
        pv_special[i] = l->special;
    }
    free(raw);

    // vanilla P_LoadThings stops at the first Doom 2 monster, P_SpawnMapThing
    // spawns nothing for deathmatch starts, player starts 2-4 and things
    // flagged not single player
    raw = lump(base, ML_THINGS, &n, 10);
    pv_thing = malloc((n ? n : 1) * sizeof(*pv_thing));
    pv_nthings = 0;
    for (i = 0; i < n; i++) {
        const unsigned char *r = raw + 10 * i;
        int type = rd16(r + 6), opt = rd16(r + 8);
        static const short doom2[] = { 68, 64, 88, 89, 69, 67, 71, 65, 66, 84 };
        for (k = 0; k < 10 && doom2[k] != type; k++)
            ;
        if (k < 10)
            break;
        if (type == 11 || type <= 0 || (type >= 2 && type <= 4) || (type != 1 && (opt & 16)))
            continue;
        pv_thing[pv_nthings++] = (pv_thing_t){ rd16(r), rd16(r + 2), rd16(r + 4), type, opt };
    }
    free(raw);

    raw = lump(base, ML_SSECTORS, &pv_nss, 4);
    pv_ss = malloc(pv_nss * sizeof(*pv_ss));
    for (i = 0; i < pv_nss; i++) {
        pv_ss[i][0] = rd16(raw + 4 * i);
        pv_ss[i][1] = rd16(raw + 4 * i + 2);
    }
    free(raw);

    raw = lump(base, ML_SEGS, &pv_nsegs, 12);
    pv_seg = malloc(pv_nsegs * sizeof(*pv_seg));
    for (i = 0; i < pv_nsegs; i++) {
        const unsigned char *r = raw + 12 * i;
        pv_seg[i].v1 = (unsigned short)rd16(r);
        pv_seg[i].v2 = (unsigned short)rd16(r + 2);
        pv_seg[i].angle = rd16(r + 4);
        pv_seg[i].linedef = rd16(r + 6);
        pv_seg[i].side = rd16(r + 8);
        pv_seg[i].offset = rd16(r + 10);
    }
    free(raw);

    raw = lump(base, ML_NODES, &pv_nnodes, 28);
    pv_node = malloc(pv_nnodes * sizeof(*pv_node));
    for (i = 0; i < pv_nnodes; i++) {
        const unsigned char *r = raw + 28 * i;
        pv_node[i].x = rd16(r);
        pv_node[i].y = rd16(r + 2);
        pv_node[i].dx = rd16(r + 4);
        pv_node[i].dy = rd16(r + 6);
        for (k = 0; k < 8; k++)
            pv_node[i].bbox[k / 4][k % 4] = rd16(r + 8 + 2 * k);
        pv_node[i].child[0] = (unsigned short)rd16(r + 24);
        pv_node[i].child[1] = (unsigned short)rd16(r + 26);
    }
    free(raw);

    pv_bmap = lump(base, ML_BLOCKMAP, &pv_nbmap, 2);
    pv_reject = lump(base, ML_REJECT, &n, 1);

    // P_GroupLines: line lists, bounding boxes, block boxes
    int total = 0;
    for (i = 0; i < pv_nlines; i++) {
        pv_sec[pv_line[i].front].count++;
        total++;
        if (pv_line[i].back != -1 && pv_line[i].back != pv_line[i].front) {
            pv_sec[pv_line[i].back].count++;
            total++;
        }
    }
    pv_seclines = malloc(total * sizeof(int));
    for (i = 0, k = 0; i < pv_nsecs; i++) {
        pv_sec[i].first = k;
        k += pv_sec[i].count;
        pv_sec[i].count = 0;
    }
    for (i = 0; i < pv_nlines; i++) {
        pv_sector_t *s = &pv_sec[pv_line[i].front];
        pv_seclines[s->first + s->count++] = i;
        if (pv_line[i].back != -1 && pv_line[i].back != pv_line[i].front) {
            s = &pv_sec[pv_line[i].back];
            pv_seclines[s->first + s->count++] = i;
        }
    }
    int ox = pv_bmap[0] << FRACBITS, oy = pv_bmap[1] << FRACBITS, bw = pv_bmap[2], bh = pv_bmap[3];
    for (i = 0; i < pv_nsecs; i++) {
        fixed_t bbox[4];
        int block;
        M_ClearBox(bbox);
        for (k = 0; k < pv_sec[i].count; k++) {
            pv_line_t *l = &pv_line[pv_seclines[pv_sec[i].first + k]];
            M_AddToBox(bbox, pv_vert[l->v1].x << FRACBITS, pv_vert[l->v1].y << FRACBITS);
            M_AddToBox(bbox, pv_vert[l->v2].x << FRACBITS, pv_vert[l->v2].y << FRACBITS);
        }
        block = (bbox[BOXTOP] - oy + MAXRADIUS) >> MAPBLOCKSHIFT;
        pv_sec[i].blockbox[BOXTOP] = block >= bh ? bh - 1 : block;
        block = (bbox[BOXBOTTOM] - oy - MAXRADIUS) >> MAPBLOCKSHIFT;
        pv_sec[i].blockbox[BOXBOTTOM] = block < 0 ? 0 : block;
        block = (bbox[BOXRIGHT] - ox + MAXRADIUS) >> MAPBLOCKSHIFT;
        pv_sec[i].blockbox[BOXRIGHT] = block >= bw ? bw - 1 : block;
        block = (bbox[BOXLEFT] - ox - MAXRADIUS) >> MAPBLOCKSHIFT;
        pv_sec[i].blockbox[BOXLEFT] = block < 0 ? 0 : block;
        pv_sec[i].soundorg[0] = (bbox[BOXRIGHT] + bbox[BOXLEFT]) / 2;
        pv_sec[i].soundorg[1] = (bbox[BOXTOP] + bbox[BOXBOTTOM]) / 2;
    }

    pv_side_of = malloc(M.nsides * sizeof(int));
    for (i = 0; i < M.nsides; i++)
        pv_side_of[i] = -1;
    printf("parity: %s %d lines %d sides (%d unique) %d sectors %d segs %d nodes\n", name,
           pv_nlines, pv_nsides, M.nsides, pv_nsecs, pv_nsegs, pv_nnodes);
}

// ---------------------------------------------------------------- record checks

static int flat_index_name_eq(int mapflat, const char *name)
{
    if (!mapflat)
        return name_eq(name, "F_SKY1");
    int g = P_MapFlat(mapflat);
    return g < nflat_names && name_eq(flat_names[g], name);
}

// The raw record: every field against the WAD line.
void PAR_LineRec(lineidx_t i, const dmc_line_t *r)
{
    PCHECK(i < pv_nlines && M.nlines == pv_nlines, "line count", i, M.nlines, pv_nlines);
    const pv_line_t *v = &pv_line[i];
    int x1 = pv_vert[v->v1].x, y1 = pv_vert[v->v1].y;
    PCHECK(r->x1 == x1 && r->y1 == y1 && r->dx == pv_vert[v->v2].x - x1
           && r->dy == pv_vert[v->v2].y - y1, "line record vertices", i, 0, 0);
    PCHECK(r->flags == v->flags, "line record flags", i, r->flags, v->flags);
    PCHECK(r->special == v->special, "line record special", i, r->special, v->special);
    PCHECK((r->slot == DMC_NONE8) == !v->special, "line record slot", i, r->slot, v->special);
    PCHECK(r->tag ? map_tagval(r->tag) == v->tag : !v->special || !v->tag, "line record tag", i,
           r->tag, v->tag);
    PCHECK(r->front == v->front && r->back == (v->back < 0 ? IDX_NONE : v->back),
           "line record sectors", i, r->front, r->back);
}

void PAR_LineSpecial(lineidx_t i, int special)
{
    PCHECK(special == pv_special[i], "line special state", i, special, pv_special[i]);
}

void PAR_SetLineSpecial(lineidx_t i, int special)
{
    if (i < pv_nlines)
        pv_special[i] = special;
}

void PAR_Line(lineidx_t i, const line_t *l)
{
    PCHECK(i < pv_nlines && M.nlines == pv_nlines, "line count", i, M.nlines, pv_nlines);
    const pv_line_t *v = &pv_line[i];
    fixed_t x1 = pv_vert[v->v1].x << FRACBITS, y1 = pv_vert[v->v1].y << FRACBITS;
    fixed_t x2 = pv_vert[v->v2].x << FRACBITS, y2 = pv_vert[v->v2].y << FRACBITS;
    PCHECK(l->v1x == x1 && l->v1y == y1 && l->v2x == x2 && l->v2y == y2, "line vertices", i, 0, 0);
    PCHECK(l->dx == x2 - x1 && l->dy == y2 - y1, "line delta", i, 0, 0);
    PCHECK((l->flags & ~ML_MAPPED) == (v->flags & ~ML_MAPPED), "line flags", i, l->flags, v->flags);
    PCHECK(l->special == pv_special[i], "line special", i, l->special, pv_special[i]);
    PCHECK(l->tag ? map_tagval(l->tag) == v->tag : !v->special || !v->tag, "line tag", i, l->tag,
           v->tag);
    PCHECK(l->frontsector == v->front, "line front", i, l->frontsector, v->front);
    PCHECK(l->backsector == (v->back < 0 ? IDX_NONE : v->back), "line back", i, l->backsector,
           v->back);
    fixed_t bb[4];
    bb[BOXLEFT] = x1 < x2 ? x1 : x2;
    bb[BOXRIGHT] = x1 < x2 ? x2 : x1;
    bb[BOXBOTTOM] = y1 < y2 ? y1 : y2;
    bb[BOXTOP] = y1 < y2 ? y2 : y1;
    PCHECK(!memcmp(bb, l->bbox, sizeof(bb)), "line bbox", i, 0, 0);
    slopetype_t st = !l->dx ? ST_VERTICAL : !l->dy ? ST_HORIZONTAL
             : FixedDiv(l->dy, l->dx) > 0 ? ST_POSITIVE : ST_NEGATIVE;
    PCHECK(l->slopetype == st, "line slopetype", i, l->slopetype, st);
}

// A unique side record against the WAD sidedef it stands for.
static void side_record(sideidx_t sd, int wadside)
{
    dmc_side_t r;
    const pv_side_t *w = &pv_side[wadside];
    map_side(sd, &r);
    PCHECK(r.textureoffset == w->textureoffset && r.rowoffset == w->rowoffset, "side offsets", sd,
           wadside, 0);
    PCHECK(name_eq(tex_name(P_MapTexture(r.top)), w->tex[0]), "side top texture", sd, wadside, 0);
    PCHECK(name_eq(tex_name(P_MapTexture(r.bottom)), w->tex[1]), "side bottom texture", sd,
           wadside, 0);
    PCHECK(name_eq(tex_name(P_MapTexture(r.mid)), w->tex[2]), "side mid texture", sd, wadside, 0);
}

void PAR_LineSide(lineidx_t i, int side, sideidx_t sd)
{
    int ws = pv_line[i].sidenum[side];
    PCHECK((ws == -1) == (sd == IDX_NONE), "line side presence", i, side, ws);
    if (ws == -1)
        return;
    if (pv_side_of[sd] < 0)
        pv_side_of[sd] = ws;
    side_record(sd, ws);
}

void PAR_Side(sideidx_t i, const side_t *s, int switched)
{
    int ws = pv_side_of[i];
    if (ws < 0)
        return;  // reached without its line (not in vanilla code paths)
    const pv_side_t *w = &pv_side[ws];
    fixed_t off = (w->textureoffset << FRACBITS) + (pv_scroller[ws] ? leveltime << FRACBITS : 0);
    PCHECK(s->textureoffset == off && s->rowoffset == w->rowoffset << FRACBITS, "side view offsets",
           i, ws, 0);
    if (!switched) {
        PCHECK(name_eq(tex_name(s->toptexture), w->tex[0]) && name_eq(tex_name(s->bottomtexture),
               w->tex[1]) && name_eq(tex_name(s->midtexture), w->tex[2]), "side view textures", i,
               ws, 0);
    }
}

static void seg_check(int i, const seg_t *s)
{
    PCHECK(i < pv_nsegs, "seg index", i, pv_nsegs, 0);
    const pv_seg_t *v = &pv_seg[i];
    const pv_line_t *l = &pv_line[v->linedef];
    PCHECK(s->v1x == pv_vert[v->v1].x << FRACBITS && s->v1y == pv_vert[v->v1].y << FRACBITS
           && s->v2x == pv_vert[v->v2].x << FRACBITS && s->v2y == pv_vert[v->v2].y << FRACBITS,
           "seg vertices", i, 0, 0);
    PCHECK(s->angle == (angle_t)v->angle << 16, "seg angle", i, s->angle >> 16, v->angle);
    PCHECK(s->offset == v->offset << FRACBITS, "seg offset", i, 0, 0);
    PCHECK(s->linedef == v->linedef && s->side == v->side, "seg line", i, s->linedef, v->linedef);
    int back = (l->flags & ML_TWOSIDED) ? pv_side[l->sidenum[v->side ^ 1]].sector : -1;
    PCHECK(s->backsector == (back < 0 ? IDX_NONE : back), "seg back", i, s->backsector, back);
    PCHECK(s->hasmid == (pv_side[l->sidenum[v->side]].tex[2][0] != '-'), "seg mid flag", i, 0, 0);
}

void PAR_Seg(segidx_t i, const seg_t *s)
{
    seg_check(i, s);
}

void PAR_SegRun(ssecidx_t s, const dmc_seg_t *r, int n, int first)
{
    PCHECK(s < pv_nss && pv_ss[s][0] == n && pv_ss[s][1] == first, "seg run", s, n, first);
    for (int k = 0; k < n; k++) {
        seg_t v;
        P_DecodeSeg(&r[k], &v);
        seg_check(first + k, &v);
    }
}

void PAR_Subsec(ssecidx_t s, secidx_t sec)
{
    PCHECK(s < pv_nss, "subsector index", s, pv_nss, 0);
    const pv_seg_t *g = &pv_seg[pv_ss[s][1]];
    int want = pv_side[pv_line[g->linedef].sidenum[g->side]].sector;
    PCHECK(sec == want, "subsector sector", s, sec, want);
}

void PAR_SubsecSegs(ssecidx_t s, int first, int count)
{
    PCHECK(s < pv_nss && pv_ss[s][0] == count && pv_ss[s][1] == first, "subsector segs", s, count,
           first);
}

void PAR_Node(nodeidx_t i, const node_t *n)
{
    PCHECK(i < pv_nnodes, "node index", i, pv_nnodes, 0);
    const pv_node_t *v = &pv_node[i];
    PCHECK(n->x == v->x << FRACBITS && n->y == v->y << FRACBITS && n->dx == v->dx << FRACBITS
           && n->dy == v->dy << FRACBITS, "node line", i, 0, 0);
    PCHECK(n->children[0] == v->child[0] && n->children[1] == v->child[1], "node children", i, 0,
           0);
}

void PAR_NodeBox(nodeidx_t i, int child, const fixed_t *box)
{
    for (int k = 0; k < 4; k++)
        PCHECK(box[k] == pv_node[i].bbox[child][k] << FRACBITS, "node bbox", i, child, k);
}

void PAR_SecTag(secidx_t s, int tag)
{
    PCHECK(map_tagval(tag) == pv_sec[s].tag, "sector tag", s, tag, pv_sec[s].tag);
}

void PAR_SecLines(secidx_t s, int first, int count)
{
    PCHECK(pv_sec[s].first == first && pv_sec[s].count == count, "sector line list", s, first,
           count);
}

void PAR_SecLine(int k, lineidx_t l)
{
    PCHECK(pv_seclines[k] == l, "sector line", k, l, pv_seclines[k]);
}

void PAR_BlockBox(secidx_t s, const int *box)
{
    PCHECK(!memcmp(box, pv_sec[s].blockbox, sizeof(pv_sec[s].blockbox)), "sector block box", s,
           0, 0);
}

// Length of a tag list: the WAD sectors with that tag value. Index 0
// (no tag) is the empty list.
void PAR_TagSectors(int t, int count)
{
    int tag = map_tagval(t), n = 0;
    for (int i = 0; i < pv_nsecs; i++)
        n += pv_sec[i].tag == tag;
    PCHECK(t ? count == n : count == 0, "tag list length", t, count, n);
}

void PAR_TagSector(int t, int k, secidx_t s)
{
    int tag = map_tagval(t), n = 0;
    for (int i = 0; i < pv_nsecs; i++)
        if (pv_sec[i].tag == tag && n++ == k) {
            PCHECK(s == i, "tag list", t, k, s);
            return;
        }
    par_fail("tag list too long", t, k, s);
}

void PAR_Reject(int s1, int s2, int bit)
{
    int pnum = s1 * pv_nsecs + s2;
    PCHECK(((pv_reject[pnum >> 3] >> (pnum & 7)) & 1) == bit, "reject", s1, s2, bit);
}

void PAR_BlockWord(int k, int w)
{
    PCHECK(k + 4 < pv_nbmap && (unsigned short)pv_bmap[k + 4] == w, "blockmap word", k, w, 0);
}

void PAR_PointInSubsector(fixed_t x, fixed_t y, int ss)
{
    int n = pv_nnodes - 1;
    while (!(n & NF_SUBSECTOR)) {
        node_t v;
        v.x = pv_node[n].x << FRACBITS;
        v.y = pv_node[n].y << FRACBITS;
        v.dx = pv_node[n].dx << FRACBITS;
        v.dy = pv_node[n].dy << FRACBITS;
        n = pv_node[n].child[R_PointOnSide(x, y, &v)];
    }
    PCHECK((n & ~NF_SUBSECTOR) == ss, "GRID start node", x >> FRACBITS, y >> FRACBITS, ss);
}

void PAR_Sector(secidx_t s, const dmc_sector_t *r, const dmc_secextra_t *x)
{
    const pv_sector_t *v = &pv_sec[s];
    PCHECK(r->floor == v->floor && r->ceiling == v->ceiling, "sector heights", s, 0, 0);
    PCHECK(r->light == v->light && r->special == v->special, "sector light, special", s,
           r->special, v->special);
    PCHECK(flat_index_name_eq(r->floorpic, v->floorpic)
           && flat_index_name_eq(r->ceilingpic, v->ceilingpic), "sector flats", s, 0, 0);
    // stored in map units, the half unit of an odd extent is dropped
    PCHECK(x->soundorg_x == v->soundorg[0] >> FRACBITS && x->soundorg_y == v->soundorg[1] >> FRACBITS,
           "sector sound origin", s, x->soundorg_x, v->soundorg[0] >> FRACBITS);
}

void PAR_Thing(int i, const mapthing_t *t)
{
    if (i >= pv_nthings)
        par_fail("more things than the WAD spawns", i, pv_nthings, 0);
    const pv_thing_t *w = &pv_thing[i];
    PCHECK(t->x == w->x && t->y == w->y && t->type == w->type
           && (t->options & 31) == (w->options & 31), "thing", i, t->type, w->type);
    PCHECK(ANG45 * (unsigned)(t->angle / 45) == ANG45 * (unsigned)(w->angle / 45), "thing angle", i,
           t->angle, w->angle);
}

void PAR_ThingsEnd(int count)
{
    PCHECK(count == pv_nthings, "thing count", count, pv_nthings, 0);
}

// Directory entries are checked the first time they are read.
static unsigned char seen_tex[2048], seen_patch[8192];

void PAR_TexEntry(int tex, const dmc_texent_t *e)
{
    if (tex < 0 || tex >= (int)sizeof(seen_tex) || seen_tex[tex])
        return;
    seen_tex[tex] = 1;
    const char *name = tex_name(tex + 1);
    int found = 0;
    // first definition of the name in TEXTURE1, then TEXTURE2 (R_TextureNumForName)
    for (int lt = 0; lt < 2 && !found; lt++) {
        int l = W_CheckNumForName(lt ? "TEXTURE2" : "TEXTURE1");
        if (l < 0)
            continue;
        int len = W_LumpLength(l);
        unsigned char *t1 = malloc(len);
        W_ReadLump(l, t1);
        int n = t1[0] | t1[1] << 8;
        for (int k = 0; k < n && !found; k++) {
            int o = t1[4 + 4 * k] | t1[5 + 4 * k] << 8 | t1[6 + 4 * k] << 16;
            if (name_eq((const char *)t1 + o, name)) {
                PCHECK(e->width == rd16(t1 + o + 12) && e->height == rd16(t1 + o + 14),
                       "texture size", tex, e->width, e->height);
                found = 1;
            }
        }
        free(t1);
    }
    PCHECK(found, "texture in TEXTURE1 or TEXTURE2", tex, 0, 0);
}

void PAR_Patch(int patch, const dmc_patch_t *h)
{
    if (patch < 0 || patch >= (int)sizeof(seen_patch) || seen_patch[patch])
        return;
    seen_patch[patch] = 1;
    PCHECK(patch < npatch_names, "patch in names.json", patch, npatch_names, 0);
    int l = W_CheckNumForName(patch_names[patch]);
    PCHECK(l >= 0, "patch in the WAD", patch, 0, 0);
    unsigned char hdr[8];
    if (W_LumpLength(l) < 8)
        par_fail("short patch lump", patch, 0, 0);
    unsigned char *p = malloc(W_LumpLength(l));
    W_ReadLump(l, p);
    memcpy(hdr, p, 8);
    free(p);
    PCHECK(h->width == rd16(hdr) && h->height == rd16(hdr + 2) && h->leftoffset == rd16(hdr + 4)
           && h->topoffset == rd16(hdr + 6), "patch header", patch, 0, 0);
}

void PAR_SpriteFrame(int sprite, int frame, int rot, int patch, int flip)
{
    static unsigned char seen[NUMSPRITES][32][9];
    int fr = frame & FF_FRAMEMASK;
    if (fr >= 32 || seen[sprite][fr][rot + 1])
        return;
    seen[sprite][fr][rot + 1] = 1;
    PCHECK(patch < npatch_names, "sprite patch in names.json", patch, 0, 0);
    const char *n = patch_names[patch];
    const char *s = dmc_sprnames[sprite];
    PCHECK(!strncasecmp(n, s, 4), "sprite patch name", sprite, frame, patch);
    int f = (frame & FF_FRAMEMASK) + 'A';
    // NNNNFr or NNNNFrFr: frame letter and rotation digit, mirrored in the second pair
    int ok = (toupper(n[4]) == f && (n[5] == '0' || n[5] == '1' + rot) && !flip)
             || (toupper(n[6]) == f && (n[7] == '0' || n[7] == '1' + rot) && flip);
    PCHECK(ok, "sprite frame patch", sprite, frame, rot);
}

// Started sounds: the PC speaker samples of the image against the DP lumps.
void PAR_Sound(int sfx, int r, uint32_t loff, uint16_t len)
{
    static unsigned char seen[NUMSFX];
    if (sfx < 0 || sfx >= NUMSFX || seen[sfx])
        return;
    seen[sfx] = 1;
    char name[16];
    snprintf(name, sizeof(name), "DP%s", dmc_sfxnames[sfx]);
    int l = W_CheckNumForName(name);
    PCHECK((l >= 0) == (r == 0), "sound presence", sfx, l, r);
    if (l < 0)
        return;
    int n = W_LumpLength(l);
    unsigned char *w = malloc(n), *m = malloc(len ? len : 1);
    W_ReadLump(l, w);
    int wl = n >= 4 ? (w[2] | w[3] << 8) : -1;
    PCHECK(wl == len && 4 + len <= n, "sound length", sfx, wl, len);
    img_read(loff, m, len);
    PCHECK(!memcmp(w + 4, m, len), "sound samples", sfx, 0, 0);
    free(w);
    free(m);
}
