// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Corpus bot of the oracle runner (--bot-out PREFIX, tests/lockstep corpus
// workload), meant to run with the play aids of tests/lockstep/play.c
// (refill or god mode, no exit). It plays in cycles of 1,600 tics:
//   rally (600 tics)   a tour stop every 100 tics, firing straight ahead
//                      all the time: the noise wakes the monsters of the
//                      map, which come for the player
//   fight (1,000 tics) it turns to the nearest monster in sight and fires,
//                      with a weapon by distance and phase (chainsaw to
//                      plasma), wanders and presses use when none is in
//                      sight and moves on to the next stop when it has
//                      seen none for a while
// Tour stops are item spots with a living monster near, places 32 units
// in front of a walk-over trigger line (doors, lifts, floors that let
// monsters out), which the bot then walks across, and places beside a
// living monster, in turn.
//
// Each ticcmd it builds is written to PREFIX.cmds (the --cmds format of
// hash.h, record n for maketic n) and each tour stop to PREFIX.tour
// (play.h), so the engine and a second oracle run replay the same game.
// It reads the game state only and draws no game random numbers. Its own
// random numbers come from --bot-seed.

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomstat.h"
#include "d_event.h"
#include "d_player.h"
#include "d_ticcmd.h"
#include "info.h"
#include "p_local.h"
#include "r_main.h"
#include "r_state.h"

#include "bot.h"
#include "play.h"

#define MAXSPOTS 1024

static FILE *fcmds, *ftour;
static uint32_t rng = 0x2545f491;
static int level = -1;
static int idle, dwell, stuck, segleft;
static int segfwd, segside, segturn, seguse;
static fixed_t lastx, lasty;
static int nspots, cursor;
static fixed_t spotx[MAXSPOTS], spoty[MAXSPOTS];
static unsigned char used[MAXSPOTS];
static int ntrig, nexttrig, nstops, cross;
static fixed_t trigx[MAXSPOTS], trigy[MAXSPOTS];
static angle_t trigang[MAXSPOTS];

// Walk-over specials (W1 and WR) without the teleports and exits.
static const unsigned char walkover[] = {
    2, 3, 4, 5, 6, 8, 10, 12, 13, 16, 17, 19, 22, 25, 30, 35, 36, 37, 38, 40, 44, 53, 54, 56,
    57, 58, 59, 72, 73, 74, 75, 76, 77, 79, 80, 81, 82, 83, 84, 86, 87, 88, 89, 90, 91, 92, 93,
    94, 95, 96, 98, 100, 104, 105, 106, 107, 108, 109, 110, 119, 120, 121, 128, 129, 130, 141
};

static int Walkover(int special)
{
    for (unsigned k = 0; k < sizeof(walkover); k++)
        if (walkover[k] == special)
            return 1;
    return 0;
}

static int Room(sector_t *sec)
{
    return sec->ceilingheight - sec->floorheight >= 56 * FRACUNIT && sec->special != 11;
}

// The places in front of the walk-over trigger lines.
static void Triggers(void)
{
    ntrig = nexttrig = 0;
    for (int i = 0; i < numlines && ntrig < MAXSPOTS; i++) {
        line_t *ld = &lines[i];
        if (!Walkover(ld->special) || !ld->backsector)
            continue;
        fixed_t mx = ld->v1->x / 2 + ld->v2->x / 2, my = ld->v1->y / 2 + ld->v2->y / 2;
        fixed_t len = P_AproxDistance(ld->dx, ld->dy);
        if (len < FRACUNIT)
            continue;
        // the front side is on the right of v1 to v2
        fixed_t x = mx + 32 * FixedDiv(ld->dy, len), y = my - 32 * FixedDiv(ld->dx, len);
        sector_t *sec = R_PointInSubsector(x, y)->sector;
        if (sec != ld->frontsector || !Room(sec))
            continue;
        trigx[ntrig] = x;
        trigy[ntrig] = y;
        trigang[ntrig++] = R_PointToAngle2(x, y, mx, my);
    }
}

static uint32_t Rnd(void)
{
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return rng;
}

int BOT_Open(const char *prefix, unsigned seed)
{
    char path[1024];
    snprintf(path, sizeof(path), "%s.cmds", prefix);
    fcmds = fopen(path, "wb");
    snprintf(path, sizeof(path), "%s.tour", prefix);
    ftour = fopen(path, "w");
    if (seed)
        rng = seed;
    return fcmds && ftour ? 0 : -1;
}

int BOT_Active(void)
{
    return fcmds != NULL;
}

void BOT_Close(void)
{
    if (fcmds)
        fclose(fcmds);
    if (ftour)
        fclose(ftour);
    fcmds = ftour = NULL;
}

static int IsMobj(thinker_t *th)
{
    return th->function.acp1 == (actionf_p1)P_MobjThinker;
}

static int LiveMonster(mobj_t *m)
{
    return m->health > 0 && (m->flags & MF_SHOOTABLE)
        && ((m->flags & MF_COUNTKILL) || m->type == MT_SKULL);
}

// Item spots of the level (the tour goes from one to the next).
static void LevelInit(void)
{
    nspots = cursor = idle = dwell = stuck = segleft = nstops = cross = 0;
    Triggers();
    for (thinker_t *th = thinkercap.next; th != &thinkercap; th = th->next) {
        mobj_t *m = (mobj_t *)th;
        if (!IsMobj(th) || !(m->flags & MF_SPECIAL) || nspots == MAXSPOTS)
            continue;
        // never into the damaging floor that ends E1M8
        if (m->subsector->sector->special == 11)
            continue;
        spotx[nspots] = m->x;
        spoty[nspots] = m->y;
        used[nspots++] = 0;
    }
}

// The living monster nearest to x, y within range (lost souls only with
// skull), in sight of eye when given.
static mobj_t *NearestOf(fixed_t x, fixed_t y, fixed_t range, mobj_t *eye, fixed_t *dist,
                         int skull)
{
    mobj_t *best = NULL;
    fixed_t bd = range;
    for (thinker_t *th = thinkercap.next; th != &thinkercap; th = th->next) {
        mobj_t *m = (mobj_t *)th;
        if (!IsMobj(th) || !LiveMonster(m) || (skull && m->type != MT_SKULL))
            continue;
        fixed_t d = P_AproxDistance(m->x - x, m->y - y);
        if (d >= bd || (eye && !P_CheckSight(eye, m)))
            continue;
        bd = d;
        best = m;
    }
    if (dist)
        *dist = bd;
    return best;
}

static mobj_t *Nearest(fixed_t x, fixed_t y, fixed_t range, mobj_t *eye, fixed_t *dist)
{
    return NearestOf(x, y, range, eye, dist, 0);
}

static void AddStop(int tic, fixed_t x, fixed_t y, mobj_t *m)
{
    unsigned a = R_PointToAngle2(x, y, m->x, m->y);
    LS_AddTour(tic, x >> FRACBITS, y >> FRACBITS, a);
    fprintf(ftour, "%d %d %d %u\n", tic, x >> FRACBITS, y >> FRACBITS, a);
}

// No shootable thing where the player would stand at x, y (the
// teleport would stomp it).
static int Clear(fixed_t x, fixed_t y)
{
    for (thinker_t *th = thinkercap.next; th != &thinkercap; th = th->next) {
        mobj_t *m = (mobj_t *)th;
        if (IsMobj(th) && (m->flags & MF_SHOOTABLE) && !m->player
            && abs(m->x - x) < m->radius + 17 * FRACUNIT && abs(m->y - y) < m->radius + 17 * FRACUNIT)
            return 0;
    }
    return 1;
}

// A place 48 to 96 units from monster m in its own sector, with room for
// the player.
static int BesideMonster(int tic, mobj_t *m)
{
    static const signed char dir[8][2] = {
        { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 }
    };
    for (int r = 48; r <= 96; r += 16)
        for (int k = 0; k < 8; k++) {
            fixed_t x = m->x + dir[k][0] * r * FRACUNIT, y = m->y + dir[k][1] * r * FRACUNIT;
            sector_t *sec = R_PointInSubsector(x, y)->sector;
            if (sec != m->subsector->sector || !Room(sec) || !Clear(x, y))
                continue;
            AddStop(tic, x, y, m);
            return 1;
        }
    return 0;
}

// Next tour stop, in turn: the next unused item spot with a living lost
// soul near (the fights the lost souls of E1M6 and E1M7 bring) or else
// with a living monster near, a trigger line while any is left, a place
// beside the living lost soul or monster nearest to the player.
static int NextStop(int tic)
{
    mobj_t *m;
    int kind = nstops++ % 3;

    if (kind == 2) {
        m = NearestOf(players[0].mo->x, players[0].mo->y, INT_MAX, NULL, NULL, 1);
        if (!m)
            m = Nearest(players[0].mo->x, players[0].mo->y, INT_MAX, NULL, NULL);
        if (m && BesideMonster(tic, m))
            return 1;
    }
    if (kind == 1 && nexttrig < ntrig) {
        int k = nexttrig++;
        LS_AddTour(tic, trigx[k] >> FRACBITS, trigy[k] >> FRACBITS, trigang[k]);
        fprintf(ftour, "%d %d %d %u\n", tic, trigx[k] >> FRACBITS, trigy[k] >> FRACBITS, trigang[k]);
        cross = 24;
        return 1;
    }
    for (int lap = 0; lap < 3; lap++) {
        for (int k = 0; k < nspots; k++) {
            int s = (cursor + k) % nspots;
            if (used[s])
                continue;
            m = NearestOf(spotx[s], spoty[s], 768 * FRACUNIT, NULL, NULL, lap == 0);
            if (!m)
                continue;
            used[s] = 1;
            cursor = s + 1;
            AddStop(tic, spotx[s], spoty[s], m);
            return 1;
        }
        if (lap)
            memset(used, 0, sizeof(used));
    }
    m = NearestOf(players[0].mo->x, players[0].mo->y, INT_MAX, NULL, NULL, 1);
    if (!m)
        m = Nearest(players[0].mo->x, players[0].mo->y, INT_MAX, NULL, NULL);
    return m && BesideMonster(tic, m);
}

// Weapon for a target at dist: chainsaw (as fist) close, then shotgun,
// chaingun, rockets far, plasma in some phases.
static int Weapon(int tic, fixed_t dist)
{
    int phase = (tic / 500) % 8;
    if (dist < 96 * FRACUNIT)
        return wp_fist;
    if (phase == 5 || phase == 7)
        return wp_plasma;
    if (dist < 256 * FRACUNIT)
        return wp_shotgun;
    if (dist > 768 * FRACUNIT || phase == 3)
        return wp_missile;
    return wp_chaingun;
}

static void Think(ticcmd_t *cmd, int tic)
{
    player_t *p = &players[0];
    mobj_t *mo = p->mo;
    fixed_t dist;
    int key = gameepisode * 16 + gamemap;
    int rally = leveltime % 1600 < 600;

    if (key != level || leveltime < 2) {
        level = key;
        LevelInit();
    }
    // lost souls in sight first
    mobj_t *t = NearestOf(mo->x, mo->y, 2048 * FRACUNIT, mo, &dist, 1);
    if (!t)
        t = Nearest(mo->x, mo->y, 2048 * FRACUNIT, mo, &dist);
    dwell++;
    idle = t ? 0 : idle + 1;
    if (rally ? dwell >= 100
        : (idle >= 70 && dwell >= 140) || dwell >= 700
          || mo->subsector->sector->special == 11) {
        if (NextStop(tic))
            dwell = idle = 0;
    }

    if (cross) {
        // over the trigger line ahead
        cross--;
        cmd->forwardmove = 50;
        return;
    }

    if (rally) {
        // noise: the chaingun fires on, the view turns slowly
        cmd->buttons = BT_ATTACK;
        if (p->readyweapon != wp_chaingun && p->pendingweapon == wp_nochange)
            cmd->buttons |= BT_CHANGE | wp_chaingun << BT_WEAPONSHIFT;
        cmd->angleturn = 512;
        return;
    }

    if (t) {
        // face it this tic, the shot follows the turn in P_PlayerThink
        int delta = (int)(R_PointToAngle2(mo->x, mo->y, t->x, t->y) - mo->angle);
        cmd->angleturn = (short)(delta >> 16);
        cmd->buttons = BT_ATTACK;
        int w = Weapon(tic, dist);
        if (w != (int)p->readyweapon && p->pendingweapon == wp_nochange)
            cmd->buttons |= BT_CHANGE | w << BT_WEAPONSHIFT;
        cmd->forwardmove = dist > 640 * FRACUNIT ? 40 : dist < 160 * FRACUNIT ? -30 : 0;
        cmd->sidemove = (tic / 35) & 1 ? 24 : -24;
        segleft = 0;
        return;
    }

    // wander: segments of one move, use pressed now and then, a sharp
    // turn when stuck
    if (abs(mo->x - lastx) + abs(mo->y - lasty) < 2 * FRACUNIT)
        stuck++;
    else
        stuck = 0;
    lastx = mo->x;
    lasty = mo->y;
    if (stuck > 12) {
        segleft = 10;
        segfwd = 0;
        segside = 0;
        segturn = Rnd() & 1 ? 2048 : -2048;
        seguse = 3;
        stuck = 0;
    } else if (!segleft) {
        segleft = 8 + Rnd() % 33;
        segfwd = (int[]){ 25, 50, 50, 0 }[Rnd() % 4];
        segside = (int[]){ 0, 0, 24, -24 }[Rnd() % 4];
        segturn = (int[]){ 0, 0, 320, -320, 640, -640, 1280, -1280 }[Rnd() % 8];
        seguse = (int[]){ 0, 7, 7 }[Rnd() % 3];
    }
    segleft--;
    cmd->forwardmove = segfwd;
    cmd->sidemove = segside;
    cmd->angleturn = segturn;
    if (seguse && segleft % seguse == 0)
        cmd->buttons = BT_USE;
}

void BOT_Cmd(ticcmd_t *cmd, int maketic)
{
    uint8_t r[8] = { 0 };
    memset(cmd, 0, sizeof(*cmd));
    if (gamestate == GS_LEVEL && players[0].mo) {
        if (players[0].playerstate == PST_LIVE)
            Think(cmd, maketic);
        else
            cmd->buttons = BT_USE;
    }
    r[0] = (uint8_t)cmd->forwardmove;
    r[1] = (uint8_t)cmd->sidemove;
    r[2] = (uint8_t)cmd->angleturn;
    r[3] = (uint8_t)((uint16_t)cmd->angleturn >> 8);
    r[4] = cmd->buttons;
    fseek(fcmds, (long)maketic * 8, SEEK_SET);
    fwrite(r, 1, sizeof(r), fcmds);
}
