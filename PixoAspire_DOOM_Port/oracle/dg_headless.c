// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Headless doomgeneric backend for the lockstep oracle. No window, no sound,
// input only from a key file. The clock only advances when the game sleeps
// and singletics is forced, so every doomgeneric_Tick runs exactly one tic.
// The per-tic hooks live in hash_hook.c. Configuration and saves go to the
// current directory (.savegame/), so a run that may save is started in an
// empty directory of its own.
//
// Options (removed before the rest reaches doomgeneric):
//   --hash-out FILE         per-tic hash stream (tests/lockstep/hash.h)
//   --hash-dump TIC FILE    text dump of the hashed values of one tic
//   --frame-out DIR         write frames as indexed PNG, DIR/fNNNNNN.png
//   --frame-stride N        frame every N tics (default 1)
//   --screen-hash FILE      per frame outside a level: gametic and a hash of
//                           the screen (a wipe writes several lines for one
//                           tic, the last one is its frame)
//   --cmds FILE             scripted ticcmd stream instead of the keyboard
//   --tics N                stop after N tics
//   --exit-at TIC           force a level exit (G_ExitLevel) before tic TIC
//   --exit-list L1,L2,L3s   forced level exits by level time (play.h)
//   --keys FILE             key events by tic (tests/lockstep/keys.h)
//   --menu-out DIR          the menu alone of every frame that shows one,
//                           DIR/mNNNNNN.png (hash_hook.h)
//   --state-out FILE        gametic, gamestate, gamemap, leveltime and rndindex
//                           after every tic
//   --god                   god mode with every weapon, full ammo and the six
//                           keycards (tests/lockstep/play.h)
//   --refill                health and armor refilled, every weapon, full ammo
//                           and the six keycards
//   --no-exit               level exits do nothing
//   --tour FILE             teleport the player at the stops of FILE (play.h)
//   --route FILE            per-map ticcmds and stops by level time (play.h),
//                           repeatable
//   --bot-out PREFIX        the corpus bot plays (bot.c) and writes PREFIX.cmds
//                           and PREFIX.tour
//   --bot-seed N            random numbers of the bot

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomgeneric.h"
#include "doomtype.h"
#include "doomstat.h"
#include "d_loop.h"

#include "bot.h"
#include "hash.h"
#include "keys.h"
#include "play.h"
#include "png.h"

#include "hash_hook.h"

static uint32_t clock_ms = 1000;
static const char *frame_dir;
static int frame_stride = 1;
static FILE *screen_hash;

void DG_Init(void)
{
    singletics = true;
}

void DG_DrawFrame(void)
{
    if (screen_hash && gamestate != GS_LEVEL) {
        uint64_t h = LS_FNV_OFFSET;
        for (int i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY; i++)
            h = (h ^ (uint8_t)DG_ScreenBuffer[i]) * LS_FNV_PRIME;
        fprintf(screen_hash, "%d %016llx\n", gametic, (unsigned long long)h);
    }
    if (!frame_dir || gametic % frame_stride)
        return;
    // A wipe draws several frames per tic, the last one wins.
    char path[1024];
    snprintf(path, sizeof(path), "%s/f%06d.png", frame_dir, gametic);
    if (png_write_indexed(path, DOOMGENERIC_RESX, DOOMGENERIC_RESY, DG_ScreenBuffer, hook_palette)) {
        fprintf(stderr, "cannot write %s\n", path);
        exit(2);
    }
}

void DG_SleepMs(uint32_t ms)
{
    clock_ms += ms ? ms : 1;
}

uint32_t DG_GetTicksMs(void)
{
    return clock_ms;
}

// I_GetEvent calls this until it returns 0 or gives a key up, once per tic
// (I_StartTic in BuildNewTic).
int DG_GetKey(int *pressed, unsigned char *key)
{
    int down, k;
    if (!LS_NextKey(gametic, &down, &k))
        return 0;
    *pressed = down;
    *key = (unsigned char)k;
    return 1;
}

void DG_SetWindowTitle(const char *title)
{
    (void)title;
}

static void usage(void)
{
    fprintf(stderr, "usage: dg_oracle [--hash-out FILE] [--hash-dump TIC FILE] [--frame-out DIR]\n"
                    "  [--frame-stride N] [--screen-hash FILE] [--cmds FILE] [--tics N] [--exit-at TIC]\n"
                    "  [--exit-list LIST] [--keys FILE] [--menu-out DIR] [--state-out FILE] [--god]\n"
                    "  [--refill] [--no-exit] [--tour FILE] [--route FILE] [--bot-out PREFIX]\n"
                    "  [--bot-seed N] doom-args...\n");
    exit(2);
}

int main(int argc, char **argv)
{
    static char *extra[] = { "-nogui", "-config", "/dev/null", "-extraconfig", "/dev/null" };
    int n = (int)(sizeof(extra) / sizeof(extra[0]));
    char **av = calloc(argc + n + 1, sizeof(char *));
    int ac = 0;
    const char *bot = NULL;
    unsigned bot_seed = 0;
    av[ac++] = argv[0];
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        int more = argc - i - 1;
        if (!strcmp(a, "--hash-out") && more >= 1) {
            if (LS_Open(argv[++i])) {
                fprintf(stderr, "cannot open %s\n", argv[i]);
                return 2;
            }
        } else if (!strcmp(a, "--hash-dump") && more >= 2) {
            LS_DumpAt(atoi(argv[i + 1]), argv[i + 2]);
            i += 2;
        } else if (!strcmp(a, "--frame-out") && more >= 1) {
            frame_dir = argv[++i];
        } else if (!strcmp(a, "--screen-hash") && more >= 1) {
            screen_hash = fopen(argv[++i], "w");
            if (!screen_hash) {
                fprintf(stderr, "cannot open %s\n", argv[i]);
                return 2;
            }
        } else if (!strcmp(a, "--frame-stride") && more >= 1) {
            frame_stride = atoi(argv[++i]);
            if (frame_stride < 1)
                usage();
        } else if (!strcmp(a, "--cmds") && more >= 1) {
            if (LS_LoadCmds(argv[++i])) {
                fprintf(stderr, "cannot read %s\n", argv[i]);
                return 2;
            }
        } else if (!strcmp(a, "--tics") && more >= 1) {
            hook_max_tics = atoi(argv[++i]);
        } else if (!strcmp(a, "--exit-at") && more >= 1) {
            hook_exit_at = atoi(argv[++i]);
        } else if (!strcmp(a, "--exit-list") && more >= 1) {
            if (LS_ExitList(argv[++i]))
                usage();
        } else if (!strcmp(a, "--keys") && more >= 1) {
            if (LS_LoadKeys(argv[++i]))
                return 2;
        } else if (!strcmp(a, "--menu-out") && more >= 1) {
            hook_menu_dir = argv[++i];
        } else if (!strcmp(a, "--state-out") && more >= 1) {
            hook_state = fopen(argv[++i], "w");
            if (!hook_state) {
                fprintf(stderr, "cannot open %s\n", argv[i]);
                return 2;
            }
        } else if (!strcmp(a, "--god")) {
            LS_God = 1;
        } else if (!strcmp(a, "--refill")) {
            LS_Refill = 1;
        } else if (!strcmp(a, "--no-exit")) {
            LS_NoExit = 1;
        } else if (!strcmp(a, "--tour") && more >= 1) {
            if (LS_LoadTour(argv[++i])) {
                fprintf(stderr, "cannot read %s\n", argv[i]);
                return 2;
            }
        } else if (!strcmp(a, "--route") && more >= 1) {
            if (LS_LoadRoute(argv[++i])) {
                fprintf(stderr, "cannot read %s\n", argv[i]);
                return 2;
            }
        } else if (!strcmp(a, "--bot-out") && more >= 1) {
            bot = argv[++i];
        } else if (!strcmp(a, "--bot-seed") && more >= 1) {
            bot_seed = (unsigned)strtoul(argv[++i], NULL, 0);
        } else if (!strncmp(a, "--", 2)) {
            usage();
        } else {
            av[ac++] = argv[i];
        }
    }
    if (bot && BOT_Open(bot, bot_seed)) {
        fprintf(stderr, "cannot write %s.cmds or %s.tour\n", bot, bot);
        return 2;
    }
    for (int i = 0; i < n; i++)
        av[ac++] = extra[i];
    doomgeneric_Create(ac, av);
    for (;;)
        doomgeneric_Tick();
}
