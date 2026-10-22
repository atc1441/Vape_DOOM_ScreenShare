// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Play aids of the corpus runs (play.h), compiled against the headers of
// whichever tree it is linked into, like hash.c, and attached with ld
// --wrap so the game sources stay untouched:
//   P_PlayerThink                    god or refill mode, the tour stops,
//                                    the route
//   G_ExitLevel, G_SecretExitLevel   ignored with LS_NoExit

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomstat.h"
#include "d_loop.h"
#include "d_player.h"
#include "g_game.h"
#include "p_local.h"

#include "play.h"

#if __has_include("things.h")
#define LS_STORE 1
#endif

int LS_God;
int LS_Refill;
int LS_NoExit;

// map 0: tour stop at gametic tic, else route stop at leveltime tic of map
typedef struct {
    int map, tic;
    int x, y;
    unsigned angle;
} stop_t;

typedef struct {
    int map, tic, count;
    ticcmd_t cmd;
} rcmd_t;

static stop_t *stops;
static int nstops, capstops;
static rcmd_t *rcmds;
static int nrcmds, caprcmds, route;

static void *grow(void *p, int *cap, size_t size)
{
    *cap = *cap ? 2 * *cap : 64;
    p = realloc(p, *cap * size);
    if (!p) {
        fprintf(stderr, "play: out of memory\n");
        exit(2);
    }
    return p;
}

static void AddStop(int map, int tic, int x, int y, unsigned angle)
{
    if (nstops == capstops)
        stops = grow(stops, &capstops, sizeof(*stops));
    stops[nstops++] = (stop_t){ map, tic, x, y, angle };
}

void LS_AddTour(int tic, int x, int y, unsigned angle)
{
    AddStop(0, tic, x, y, angle);
}

int LS_LoadTour(const char *path)
{
    FILE *f = fopen(path, "r");
    int tic, x, y;
    unsigned angle;
    if (!f)
        return -1;
    while (fscanf(f, "%d %d %d %u", &tic, &x, &y, &angle) == 4)
        LS_AddTour(tic, x, y, angle);
    fclose(f);
    return 0;
}

int LS_LoadRoute(const char *path)
{
    FILE *f = fopen(path, "r");
    char buf[256];
    int line = 0;
    if (!f)
        return -1;
    route = 1;
    while (fgets(buf, sizeof(buf), f)) {
        int map, tic, count, fwd, side, turn, buttons, x, y;
        unsigned angle;
        char *c = strchr(buf, '#');
        line++;
        if (c)
            *c = 0;
        if (sscanf(buf, " cmd %d %d %d %d %d %d %d", &map, &tic, &count, &fwd, &side, &turn, &buttons)
            == 7) {
            if (nrcmds == caprcmds)
                rcmds = grow(rcmds, &caprcmds, sizeof(*rcmds));
            rcmd_t *r = &rcmds[nrcmds++];
            memset(r, 0, sizeof(*r));
            r->map = map;
            r->tic = tic;
            r->count = count;
            r->cmd.forwardmove = (signed char)fwd;
            r->cmd.sidemove = (signed char)side;
            r->cmd.angleturn = (short)turn;
            r->cmd.buttons = (byte)buttons;
        } else if (sscanf(buf, " stop %d %d %d %d %u", &map, &tic, &x, &y, &angle) == 5 && map > 0) {
            AddStop(map, tic, x, y, angle);
        } else if (strspn(buf, " \t\r\n") != strlen(buf)) {
            fprintf(stderr, "%s:%d: expected a cmd or a stop line\n", path, line);
            fclose(f);
            return -1;
        }
    }
    fclose(f);
    return 0;
}

int LS_HaveRoute(void)
{
    return route;
}

void LS_RouteIdle(void *ticcmd, int maketic)
{
    ticcmd_t *cmd = ticcmd;
    memset(cmd, 0, sizeof(*cmd));
    if (!(maketic & 7))
        cmd->buttons = BT_ATTACK;
}

// The player stands at the stop, on the floor, at rest.
static void Teleport(player_t *p, const stop_t *s)
{
    mobj_t *mo = p->mo;
    if (!P_TeleportMove(mo, s->x << FRACBITS, s->y << FRACBITS))
        return;
#ifdef LS_STORE
    mo->z = MO_FLOORZ(mo);
#else
    mo->z = mo->floorz;
#endif
    mo->momx = mo->momy = mo->momz = 0;
    mo->angle = s->angle;
    p->viewz = mo->z + p->viewheight;
}

void __real_P_PlayerThink(player_t *player);

void __wrap_P_PlayerThink(player_t *player)
{
    if ((LS_God || LS_Refill) && player->playerstate == PST_LIVE) {
        if (LS_God)
            player->cheats |= CF_GODMODE;
        else {
            player->health = player->mo->health = 100;
            player->armorpoints = 200;
            player->armortype = 2;
        }
        // the weapons of E1 in single player, no BFG
        for (int i = 0; i < wp_supershotgun; i++)
            player->weaponowned[i] = i != wp_bfg;
        for (int i = 0; i < NUMAMMO; i++)
            player->ammo[i] = player->maxammo[i];
        for (int i = 0; i < NUMCARDS; i++)
            player->cards[i] = true;
    }
    if (route) {
        memset(&player->cmd, 0, sizeof(player->cmd));
        for (int k = 0; k < nrcmds; k++)
            if (rcmds[k].map == gamemap && leveltime >= rcmds[k].tic
                && leveltime < rcmds[k].tic + rcmds[k].count) {
                player->cmd = rcmds[k].cmd;
                break;
            }
    }
    for (int k = 0; k < nstops; k++) {
        const stop_t *s = &stops[k];
        if (s->map ? s->map == gamemap && s->tic == leveltime : s->tic == gametic)
            if (player->mo && player->playerstate == PST_LIVE)
                Teleport(player, s);
    }
    __real_P_PlayerThink(player);
}

#define MAX_EXITS 32

static int exits[MAX_EXITS], nexits, exit_k;

int LS_ExitList(const char *list)
{
    const char *p = list;
    nexits = exit_k = 0;
    while (*p) {
        char *end;
        long v = strtol(p, &end, 10);
        if (end == p || v < 0 || v > 0xFFFFFF || nexits == MAX_EXITS)
            return -1;
        exits[nexits] = (int)v;
        if (*end == 's') {
            exits[nexits] |= 1 << 24;
            end++;
        }
        nexits++;
        if (*end == ',' && end[1])
            end++;
        else if (*end)
            return -1;
        p = end;
    }
    return nexits ? 0 : -1;
}

void LS_ExitTic(void)
{
    if (exit_k == nexits || gamestate != GS_LEVEL || leveltime != (exits[exit_k] & 0xFFFFFF))
        return;
    if (exits[exit_k++] >> 24)
        G_SecretExitLevel();
    else
        G_ExitLevel();
}

void __real_G_ExitLevel(void);
void __real_G_SecretExitLevel(void);

void __wrap_G_ExitLevel(void)
{
    if (!LS_NoExit)
        __real_G_ExitLevel();
}

void __wrap_G_SecretExitLevel(void)
{
    if (!LS_NoExit)
        __real_G_SecretExitLevel();
}
