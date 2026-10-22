// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Implementation of the lockstep hash in hash.h. It is compiled against the
// headers of whichever tree it is linked into (oracle or engine). The engine
// keeps its objects in the store of port/obj: map things in tiers, full
// objects in pages, references as handles. Its list is built in vanilla
// thinker order from the store (O_ForEach), with views of the static and
// sleeping things. Its types and states have the dense ids of gen/info.h,
// mapped back to the vanilla numbers here.
//
// LS_SMALL (engine only) computes the same hash without the list, for the
// firmware: a scratch of 2 B per handle slot, three passes over the store
// (candidates and pins, ordinals in thinker order, the hash) and no static
// data, so the firmware can run it from an overlay. LS_NOFILE leaves out
// the hash stream, the dump and the scripted input (no stdio).

#ifndef LS_NOFILE
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomstat.h"
#include "d_event.h"
#include "d_loop.h"
#include "i_system.h"
#include "m_random.h"
#include "p_local.h"
#include "r_state.h"

#include "hash.h"

#if __has_include("things.h")
#define LS_STORE 1
#include "things.h"
#define LS_TYPE(t) dmc_mobj_vanilla[t]
#define LS_STATE(s) dmc_state_vanilla[s]
#define LS_PSPSTATE(st) dmc_state_vanilla[(st) - dmc_states]
#else
#define LS_PSPSTATE(st) ((st) - states)
#endif

extern int prndindex;

#ifndef LS_NOFILE
int LS_NoGametic;
#define LS_GAMETIC (LS_NoGametic ? 0 : gametic)
#else
#define LS_GAMETIC gametic
#endif

typedef struct {
    uint64_t h;
#ifndef LS_NOFILE
    FILE *dump;
#endif
#ifdef LS_SMALL
    struct small *sm;
#endif
} sink_t;

static void put(sink_t *s, const char *name, int idx, int32_t v)
{
    s->h = LS_Feed(s->h, (uint32_t)v);
#ifndef LS_NOFILE
    if (s->dump) {
        if (idx >= 0)
            fprintf(s->dump, "%s[%d] %d\n", name, idx, v);
        else
            fprintf(s->dump, "%s %d\n", name, v);
    }
#else
    (void)name;
    (void)idx;
#endif
}

// One hashed object with the fields of the hash, its key (pointer or
// handle) and the key of its target.
typedef struct {
    int32_t type, x, y, z, momx, momy, momz, angle, flags, health, state, tics;
    int32_t movedir, movecount, reactiontime, threshold, lastlook;
    uintptr_t key, target, tracer;
    int candidate;
} hobj_t;

// A target that no longer exists: never in the table.
#define KEY_GONE ((uintptr_t)-1)

#ifndef LS_SMALL
// Hashed list and a key to ordinal table, rebuilt every call.
static hobj_t *list;
static int nlist, caplist;
static uintptr_t *tab_key;
static int *tab_val;
static unsigned tab_mask;

static unsigned key_slot(uintptr_t x)
{
    x ^= x >> 17;
    x *= 0x9e3779b1u;
    return (unsigned)(x ^ (x >> 15)) & tab_mask;
}

static void tab_build(void)
{
    unsigned n = 64;
    while (n < 2u * (unsigned)nlist)
        n <<= 1;
    if (n - 1 != tab_mask || !tab_key) {
        free(tab_key);
        free(tab_val);
        tab_key = malloc(n * sizeof(*tab_key));
        tab_val = malloc(n * sizeof(*tab_val));
        tab_mask = n - 1;
    }
    memset(tab_key, 0, n * sizeof(*tab_key));
    for (int i = 0; i < nlist; i++) {
        unsigned k = key_slot(list[i].key);
        while (tab_key[k])
            k = (k + 1) & tab_mask;
        tab_key[k] = list[i].key;
        tab_val[k] = i + 1;
    }
}

static int32_t ref(const sink_t *s, uintptr_t key)
{
    (void)s;
    if (!key)
        return 0;
    for (unsigned k = key_slot(key); tab_key[k]; k = (k + 1) & tab_mask)
        if (tab_key[k] == key)
            return tab_val[k];
    return -1;
}

static hobj_t *list_new(void)
{
    if (nlist == caplist) {
        caplist = caplist ? 2 * caplist : 1024;
        list = realloc(list, caplist * sizeof(*list));
    }
    return &list[nlist++];
}
#endif

#ifdef LS_STORE
// Engine: handles are the keys, player pointers are mapped to handles.
static uintptr_t mobj_key(mobj_t *mo)
{
    if (!mo)
        return 0;
    if (mo == O_DETACHED)
        return KEY_GONE;
    return O_Hnd(mo);
}

static void fill_obj(hobj_t *o, mobj_t *mo, unsigned h)
{
    o->type = LS_TYPE(mo->type);
    o->x = mo->x;
    o->y = mo->y;
    o->z = mo->z;
    o->momx = mo->momx;
    o->momy = mo->momy;
    o->momz = mo->momz;
    o->angle = (int32_t)mo->angle;
    o->flags = mo->flags;
    o->health = mo->health;
    o->state = LS_STATE(mo->statenum);
    o->tics = mo->tics;
    o->movedir = mo->movedir;
    o->movecount = mo->movecount;
    o->reactiontime = mo->reactiontime;
    o->threshold = mo->threshold;
    o->lastlook = mo->lastlook;
    o->key = h;
    o->target = mo->target == H_DANGLE ? KEY_GONE : mo->target;
    o->tracer = 0;
    o->candidate = O_RestingCorpse(mo);
}

#ifndef LS_SMALL
static void add_obj(mobj_t *mo, unsigned h, void *ctx)
{
    (void)ctx;
    fill_obj(list_new(), mo, h);
}

static void collect(void)
{
    O_ForEach(add_obj, NULL);
}
#endif

static int32_t sec_floor(int i) { return P_FloorHeight(i); }
static int32_t sec_ceil(int i) { return P_CeilingHeight(i); }
static int32_t sec_light(int i) { return P_LightLevel(i); }
static int32_t sec_special(int i) { return P_SecSpecial(i); }
static int32_t sec_mover(int i) { return P_SecMover(i) != NOMOVER; }
#else
// Oracle: pointers are the keys.
static uintptr_t mobj_key(mobj_t *mo)
{
    return (uintptr_t)mo;
}

static void collect(void)
{
    thinker_t *th;
    // No thinker list before the first P_SetupLevel.
    if (!thinkercap.next)
        return;
    for (th = thinkercap.next; th != &thinkercap; th = th->next) {
        if (th->function.acp1 != (actionf_p1)P_MobjThinker)
            continue;
        mobj_t *mo = (mobj_t *)th;
        hobj_t *o = list_new();
        o->type = mo->type;
        o->x = mo->x;
        o->y = mo->y;
        o->z = mo->z;
        o->momx = mo->momx;
        o->momy = mo->momy;
        o->momz = mo->momz;
        o->angle = (int32_t)mo->angle;
        o->flags = mo->flags;
        o->health = mo->health;
        o->state = (int32_t)(mo->state - states);
        o->tics = mo->tics;
        o->movedir = mo->movedir;
        o->movecount = mo->movecount;
        o->reactiontime = mo->reactiontime;
        o->threshold = mo->threshold;
        o->lastlook = mo->lastlook;
        o->key = (uintptr_t)mo;
        o->target = (uintptr_t)mo->target;
        o->tracer = (uintptr_t)mo->tracer;
        o->candidate = (mo->flags & MF_CORPSE) && !mo->player && mo->tics == -1 && !mo->momx
            && !mo->momy && !mo->momz && mo->z == mo->floorz;
    }
}

static int32_t sec_floor(int i) { return sectors[i].floorheight; }
static int32_t sec_ceil(int i) { return sectors[i].ceilingheight; }
static int32_t sec_light(int i) { return sectors[i].lightlevel; }
static int32_t sec_special(int i) { return sectors[i].special; }
static int32_t sec_mover(int i) { return sectors[i].specialdata != NULL; }
#endif

#ifndef LS_SMALL
static void build_list(void)
{
    nlist = 0;
    collect();
    tab_build();
#if LS_SKIP_RESTING_CORPSES
    // Pinned candidates stay: the target or tracer of an object that is
    // not a candidate itself, or the mo or attacker of a player.
    char *pin = calloc(nlist + 1, 1);
    for (int i = 0; i < nlist; i++) {
        if (list[i].candidate)
            continue;
        int r = ref(NULL, list[i].target);
        if (r > 0)
            pin[r - 1] = 1;
        r = ref(NULL, list[i].tracer);
        if (r > 0)
            pin[r - 1] = 1;
    }
    for (int p = 0; p < MAXPLAYERS; p++) {
        if (!playeringame[p])
            continue;
        int r = ref(NULL, mobj_key(players[p].mo));
        if (r > 0)
            pin[r - 1] = 1;
        r = ref(NULL, mobj_key(players[p].attacker));
        if (r > 0)
            pin[r - 1] = 1;
    }
    int n = 0;
    for (int i = 0; i < nlist; i++)
        if (pin[i] || !list[i].candidate)
            list[n++] = list[i];
    nlist = n;
    free(pin);
    tab_build();
#endif
}

#endif

static int32_t ref(const sink_t *s, uintptr_t key);

// Globals, players and sectors (parts 1 to 3).
static void walk_head(sink_t *s)
{
    put(s, "gametic", -1, LS_GAMETIC);
    put(s, "leveltime", -1, leveltime);
    put(s, "gamestate", -1, gamestate);
    put(s, "gameskill", -1, gameskill);
    put(s, "gameepisode", -1, gameepisode);
    put(s, "gamemap", -1, gamemap);
    put(s, "prndindex", -1, prndindex);
    put(s, "rndindex", -1, rndindex);

    for (int i = 0; i < MAXPLAYERS; i++) {
        player_t *p = &players[i];
        if (!playeringame[i])
            continue;
        put(s, "player", -1, i);
        put(s, "pl.mo", i, ref(s, mobj_key(p->mo)));
        put(s, "pl.playerstate", i, p->playerstate);
        put(s, "pl.cmd.forwardmove", i, p->cmd.forwardmove);
        put(s, "pl.cmd.sidemove", i, p->cmd.sidemove);
        put(s, "pl.cmd.angleturn", i, p->cmd.angleturn);
        put(s, "pl.cmd.buttons", i, p->cmd.buttons);
        put(s, "pl.viewz", i, p->viewz);
        put(s, "pl.viewheight", i, p->viewheight);
        put(s, "pl.deltaviewheight", i, p->deltaviewheight);
        put(s, "pl.bob", i, p->bob);
        put(s, "pl.health", i, p->health);
        put(s, "pl.armorpoints", i, p->armorpoints);
        put(s, "pl.armortype", i, p->armortype);
        for (int k = 0; k < 6; k++)
            put(s, "pl.powers", k, p->powers[k]);
        for (int k = 0; k < 6; k++)
            put(s, "pl.cards", k, p->cards[k] != 0);
        put(s, "pl.backpack", i, p->backpack != 0);
        put(s, "pl.readyweapon", i, p->readyweapon);
        put(s, "pl.pendingweapon", i, p->pendingweapon);
        for (int k = 0; k < 9; k++)
            put(s, "pl.weaponowned", k, k < NUMWEAPONS ? p->weaponowned[k] != 0 : 0);
        for (int k = 0; k < 4; k++)
            put(s, "pl.ammo", k, p->ammo[k]);
        for (int k = 0; k < 4; k++)
            put(s, "pl.maxammo", k, p->maxammo[k]);
        put(s, "pl.attackdown", i, p->attackdown);
        put(s, "pl.usedown", i, p->usedown);
        put(s, "pl.cheats", i, p->cheats);
        put(s, "pl.refire", i, p->refire);
        put(s, "pl.killcount", i, p->killcount);
        put(s, "pl.itemcount", i, p->itemcount);
        put(s, "pl.secretcount", i, p->secretcount);
        put(s, "pl.damagecount", i, p->damagecount);
        put(s, "pl.bonuscount", i, p->bonuscount);
        put(s, "pl.attacker", i, ref(s, mobj_key(p->attacker)));
        put(s, "pl.extralight", i, p->extralight);
        put(s, "pl.fixedcolormap", i, p->fixedcolormap);
        for (int k = 0; k < 2; k++) {
            pspdef_t *ps = &p->psprites[k];
            put(s, "pl.psp.state", k, ps->state ? (int32_t)LS_PSPSTATE(ps->state) : -1);
            put(s, "pl.psp.tics", k, ps->tics);
            put(s, "pl.psp.sx", k, ps->sx);
            put(s, "pl.psp.sy", k, ps->sy);
        }
        put(s, "pl.didsecret", i, p->didsecret != 0);
    }

    put(s, "numsectors", -1, numsectors);
    for (int i = 0; i < numsectors; i++) {
        put(s, "sec.floorheight", i, sec_floor(i));
        put(s, "sec.ceilingheight", i, sec_ceil(i));
        put(s, "sec.lightlevel", i, sec_light(i));
        put(s, "sec.special", i, sec_special(i));
        put(s, "sec.mover", i, sec_mover(i));
    }
}

// Part 4: object i (1-based) of the hashed list.
static void put_obj(sink_t *s, int i, const hobj_t *mo)
{
    put(s, "mo.type", i, mo->type);
    put(s, "mo.x", i, mo->x);
    put(s, "mo.y", i, mo->y);
    put(s, "mo.z", i, mo->z);
    put(s, "mo.momx", i, mo->momx);
    put(s, "mo.momy", i, mo->momy);
    put(s, "mo.momz", i, mo->momz);
    put(s, "mo.angle", i, mo->angle);
    put(s, "mo.flags", i, mo->flags);
    put(s, "mo.health", i, mo->health);
    put(s, "mo.state", i, mo->state);
    put(s, "mo.tics", i, mo->tics);
    put(s, "mo.target", i, ref(s, mo->target));
    put(s, "mo.movedir", i, mo->movedir);
    put(s, "mo.movecount", i, mo->movecount);
    put(s, "mo.reactiontime", i, mo->reactiontime);
    put(s, "mo.threshold", i, mo->threshold);
    put(s, "mo.lastlook", i, mo->lastlook);
}

#ifndef LS_SMALL
static void walk(sink_t *s)
{
    build_list();
    walk_head(s);
    for (int i = 0; i < nlist; i++)
        put_obj(s, i + 1, &list[i]);
    put(s, "nummobjs", -1, nlist);
}
#else
// Handle slots: map thing i is slot i, page p slot nthings + p, drop d slot
// nthings + npages + d. Per slot a u16 of the scratch: the 1-based ordinal
// in the hashed list (0 when not in it) and two flags.
#define F_CAND 0x8000u
#define F_PIN 0x4000u
#define F_ORD 0x3FFFu

struct small {
    uint16_t *slot;
    int nslots, n;
};

static int slot(const struct small *sm, uintptr_t key)
{
    unsigned h = (unsigned)key;
    if (key == 0 || key == KEY_GONE)
        return -1;
    int k = H_IDX(h);
    switch (H_KIND(h)) {
    case HK_MAP:
        k -= 1;
        break;
    case HK_OBJ:
        k += M.nthings;
        break;
    case HK_DROP:
        k += M.nthings + o_pool.npages;
        break;
    default:
        return -1;
    }
    return k >= 0 && k < sm->nslots ? k : -1;
}

static int32_t ref(const sink_t *s, uintptr_t key)
{
    if (!key)
        return 0;
    int k = slot(s->sm, key);
    return k >= 0 && (s->sm->slot[k] & F_ORD) ? s->sm->slot[k] & F_ORD : -1;
}

static void pin(struct small *sm, uintptr_t key)
{
    int k = slot(sm, key);
    if (k >= 0)
        sm->slot[k] |= F_PIN;
}

// First pass: candidates, and the pins of the objects that are no
// candidates.
static void mark_obj(mobj_t *mo, unsigned h, void *ctx)
{
    struct small *sm = ctx;
    hobj_t o;
    fill_obj(&o, mo, h);
    int k = slot(sm, h);
    if (k < 0)
        I_Error("LS_Hash: handle %x out of range", h);
    if (o.candidate)
        sm->slot[k] |= F_CAND;
    else
        pin(sm, o.target);
}

// Second pass: the ordinals, in thinker order.
static void order_obj(mobj_t *mo, unsigned h, void *ctx)
{
    struct small *sm = ctx;
    uint16_t *v = &sm->slot[slot(sm, h)];
    (void)mo;
    if (!LS_SKIP_RESTING_CORPSES || !(*v & F_CAND) || (*v & F_PIN))
        *v |= (uint16_t)++sm->n;
}

static void hash_obj(mobj_t *mo, unsigned h, void *ctx)
{
    sink_t *s = ctx;
    int ord = s->sm->slot[slot(s->sm, h)] & F_ORD;
    hobj_t o;
    if (!ord)
        return;
    fill_obj(&o, mo, h);
    put_obj(s, ord, &o);
}

static void walk_small(sink_t *s, void *mem, uint32_t bytes)
{
    struct small sm;
    sm.nslots = M.nthings + o_pool.npages + o_ndrops;
    if ((uint32_t)sm.nslots * 2u > bytes || sm.nslots > (int)F_ORD)
        I_Error("LS_Hash: %d handle slots need %d B of scratch", sm.nslots, sm.nslots * 2);
    sm.slot = mem;
    sm.n = 0;
    memset(mem, 0, (uint32_t)sm.nslots * 2u);
    s->sm = &sm;
    O_ForEach(mark_obj, &sm);
    for (int p = 0; p < MAXPLAYERS; p++) {
        if (playeringame[p]) {
            pin(&sm, mobj_key(players[p].mo));
            pin(&sm, mobj_key(players[p].attacker));
        }
    }
    O_ForEach(order_obj, &sm);
    walk_head(s);
    O_ForEach(hash_obj, s);
    put(s, "nummobjs", -1, sm.n);
}

uint64_t LS_HashScratch(void *mem, uint32_t bytes)
{
    sink_t s;
    memset(&s, 0, sizeof(s));
    s.h = LS_FNV_OFFSET;
    walk_small(&s, mem, bytes);
    return s.h;
}

#ifndef LS_NOFILE
// Host builds: a static scratch
static void walk(sink_t *s)
{
    static uint16_t mem[1 << 15];
    walk_small(s, mem, sizeof(mem));
}
#endif
#endif

#if !defined(LS_SMALL) || !defined(LS_NOFILE)
uint64_t LS_Hash(void)
{
    sink_t s;
    memset(&s, 0, sizeof(s));
    s.h = LS_FNV_OFFSET;
    walk(&s);
    return s.h;
}
#endif

#ifndef LS_NOFILE

static FILE *out;
static int dump_tic = -1;
static const char *dump_path;

int LS_Open(const char *path)
{
    out = fopen(path, "w");
    if (!out)
        return -1;
    // Line buffered, so an abort loses no completed tic.
    setvbuf(out, NULL, _IOLBF, 0);
    return 0;
}

void LS_DumpAt(int tic, const char *path)
{
    dump_tic = tic;
    dump_path = path;
}

void LS_Tic(void)
{
    if (dump_tic == gametic && dump_path) {
        sink_t s = { LS_FNV_OFFSET, fopen(dump_path, "w") };
        if (s.dump) {
            walk(&s);
            fprintf(s.dump, "hash %016llx\n", (unsigned long long)s.h);
            fclose(s.dump);
        }
    }
    if (out)
        fprintf(out, "%d %016llx\n", gametic, (unsigned long long)LS_Hash());
}

void LS_Close(void)
{
    if (out)
        fclose(out);
    out = NULL;
}

static uint8_t *cmds;
static long ncmds;

int LS_LoadCmds(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return -1;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    free(cmds);
    cmds = malloc(n > 0 ? n : 1);
    ncmds = cmds && n > 0 && fread(cmds, 1, n, f) == (size_t)n ? n / LS_CMD_SIZE : 0;
    fclose(f);
    return ncmds ? 0 : -1;
}

int LS_HaveCmds(void)
{
    return ncmds > 0;
}

void LS_FillCmd(void *ticcmd, int maketic)
{
    ticcmd_t *cmd = ticcmd;
    byte special = cmd->buttons & BT_SPECIAL ? cmd->buttons : 0;
    memset(cmd, 0, sizeof(*cmd));
    if (maketic >= 0 && maketic < ncmds) {
        const uint8_t *r = cmds + (long)maketic * LS_CMD_SIZE;
        cmd->forwardmove = (signed char)r[0];
        cmd->sidemove = (signed char)r[1];
        cmd->angleturn = (short)(r[2] | r[3] << 8);
        cmd->buttons = r[4];
    }
    if (special)
        cmd->buttons = special;
}
#endif
