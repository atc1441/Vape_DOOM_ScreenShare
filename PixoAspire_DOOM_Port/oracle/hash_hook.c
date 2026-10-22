// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Per-tic hooks of the oracle, attached with ld --wrap so the upstream
// sources stay untouched:
//   G_Ticker           hash and game state after every tic, forced level
//                      exits (hook_exit_at, the exit list of play.h)
//   G_BuildTiccmd      scripted input (a route outside a level, the ticcmd
//                      stream), or the corpus bot (bot.c)
//   M_Drawer           the menu alone into a zeroed screen (hook_menu_dir).
//                      The Nightmare and quit prompts show the strings of the
//                      IWAD's DEHACKED lump, as the pack has them.
//   F_StartFinale      the E1 text and background flat (BGFLATE1) of the
//                      IWAD's DEHACKED lump. doomgeneric is built without
//                      DEHACKED support, Chocolate Doom loads Freedoom's
//                      text and flat from that lump and the pack carries
//                      the same (tools/conv). The length of the text sets
//                      the tic at which the finale switches to the art stage.
//   I_Quit, I_Error    flush the hash stream and exit
//   I_SetPalette       keeps the raw palette for the frame writer
//   G_DeferedPlayDemo, G_TimeDemo
//                      get a static copy of the demo name. Upstream passes
//                      a buffer on the stack of D_DoomMain, which has
//                      returned by the time the demo ends.

#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "doomtype.h"
#include "doomstat.h"
#include "d_event.h"
#include "d_loop.h"
#include "d_ticcmd.h"
#include "dstrings.h"
#include "g_game.h"
#include "i_video.h"
#include "m_random.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "bot.h"
#include "hash.h"
#include "hash_hook.h"
#include "play.h"
#include "png.h"

// m_menu.c, dstrings.c and f_finale.c, not declared in a header
extern int messageToPrint;
extern char *messageString;
extern void (*messageRoutine)(int response);
void M_VerifyNightmare(int key);
void M_QuitResponse(int key);
extern char *doom1_endmsg[];
extern char *finaletext;
extern char *finaleflat;

void __real_G_Ticker(void);
void __real_G_BuildTiccmd(ticcmd_t *cmd, int maketic);
void __real_I_Quit(void);
void __real_I_Error(char *error, ...);
void __real_I_SetPalette(byte *palette);
void __real_M_Drawer(void);
void __real_F_StartFinale(void);
void __real_G_DeferedPlayDemo(char *name);
void __real_G_TimeDemo(char *name);

int hook_max_tics;
int hook_exit_at = -1;
uint8_t hook_palette[768];
FILE *hook_state;
const char *hook_menu_dir;

static void finish(void)
{
    LS_Close();
    BOT_Close();
    if (hook_state)
        fclose(hook_state);
    hook_state = NULL;
    fflush(stdout);
}

void __wrap_G_Ticker(void)
{
    if (gametic == hook_exit_at)
        G_ExitLevel();
    LS_ExitTic();
    __real_G_Ticker();
    LS_Tic();
    if (hook_state)
        fprintf(hook_state, "%d %d %d %d %d\n", gametic, gamestate, gamemap, leveltime, rndindex);
    if (hook_max_tics && gametic + 1 >= hook_max_tics) {
        finish();
        exit(0);
    }
}

void __wrap_G_BuildTiccmd(ticcmd_t *cmd, int maketic)
{
    if (LS_HaveRoute() && gamestate != GS_LEVEL)
        LS_RouteIdle(cmd, maketic);
    else if (LS_HaveCmds()) {
        __real_G_BuildTiccmd(cmd, maketic);
        LS_FillCmd(cmd, maketic);
    } else if (BOT_Active())
        BOT_Cmd(cmd, maketic);
    else
        __real_G_BuildTiccmd(cmd, maketic);
}

static char *deh_string(const char *key);

// The text of the prompt up with the strings of the DEHACKED lump: the
// Nightmare prompt, and the quit prompt, the quit message of the vanilla
// one ("%s\n\n" DOSY, m_menu.c M_QuitDOOM) and DOSY. NULL for the others
// or when the lump does not have the strings.
static char *deh_prompt(void)
{
    static char *nightmare, *dosy, *quit[NUM_QUITMESSAGES];
    if (!messageToPrint)
        return NULL;
    if (messageRoutine == M_VerifyNightmare) {
        if (!nightmare)
            nightmare = deh_string("NIGHTMARE");
        return nightmare;
    }
    if (messageRoutine != M_QuitResponse)
        return NULL;
    for (int i = 0; i < NUM_QUITMESSAGES; i++) {
        size_t n = strlen(doom1_endmsg[i]);
        if (strncmp(messageString, doom1_endmsg[i], n) || strncmp(messageString + n, "\n\n", 2))
            continue;
        if (!quit[i]) {
            char key[16], *msg;
            snprintf(key, sizeof(key), i ? "QUITMSG%d" : "QUITMSG", i);
            msg = deh_string(key);
            if (!dosy)
                dosy = deh_string("DOSY");
            if (msg && dosy && (quit[i] = malloc(strlen(msg) + strlen(dosy) + 3)))
                sprintf(quit[i], "%s\n\n%s", msg, dosy);
            free(msg);
        }
        return quit[i];
    }
    return NULL;
}

// The menu of this frame drawn again into a zeroed screen. Drawing it
// changes no game state.
void __wrap_M_Drawer(void)
{
    static byte screen[SCREENWIDTH * SCREENHEIGHT];
    char path[1024];
    char *text = deh_prompt(), *vanilla = messageString;
    if (text)
        messageString = text;
    __real_M_Drawer();
    if (hook_menu_dir && (menuactive || messageToPrint)) {
        memset(screen, 0, sizeof(screen));
        V_UseBuffer(screen);
        __real_M_Drawer();
        V_RestoreBuffer();
        snprintf(path, sizeof(path), "%s/m%06d.png", hook_menu_dir, gametic);
        if (png_write_indexed(path, SCREENWIDTH, SCREENHEIGHT, screen, hook_palette)) {
            fprintf(stderr, "cannot write %s\n", path);
            exit(2);
        }
    }
    messageString = vanilla;
}

void __wrap_I_Quit(void)
{
    finish();
    __real_I_Quit();
    exit(0);
}

void __wrap_I_Error(char *error, ...)
{
    static int nested;
    char msg[512];
    va_list ap;
    va_start(ap, error);
    vsnprintf(msg, sizeof(msg), error, ap);
    va_end(ap);
    if (nested++) {
        fprintf(stderr, "%s\n", msg);
        exit(1);
    }
    finish();
    if (!strncmp(msg, "timed ", 6)) {
        printf("%s\n", msg);
        exit(0);
    }
    __real_I_Error("%s", msg);
    exit(1);
}

static char demo_name[64];

void __wrap_G_DeferedPlayDemo(char *name)
{
    snprintf(demo_name, sizeof(demo_name), "%s", name);
    __real_G_DeferedPlayDemo(demo_name);
}

void __wrap_G_TimeDemo(char *name)
{
    snprintf(demo_name, sizeof(demo_name), "%s", name);
    __real_G_TimeDemo(demo_name);
}

void __wrap_I_SetPalette(byte *palette)
{
    memcpy(hook_palette, palette, sizeof(hook_palette));
    __real_I_SetPalette(palette);
}

// Line [*a, *b) of the text at *p, trimmed, and *p moved past it.
static int deh_line(const char **p, const char *end, const char **a, const char **b)
{
    const char *eol;
    if (*p >= end)
        return 0;
    eol = memchr(*p, '\n', end - *p);
    if (!eol)
        eol = end;
    *a = *p;
    *b = eol;
    *p = eol + (eol < end);
    while (*a < *b && isspace((unsigned char)**a))
        (*a)++;
    while (*b > *a && isspace((unsigned char)(*b)[-1]))
        (*b)--;
    return 1;
}

// String key of the [STRINGS] section of the DEHACKED lump, read as
// tools/conv/wadread.py does: a line ending in a backslash goes on with the
// next one, then the BEX escapes. NULL when there is none.
static char *deh_string(const char *key)
{
    size_t klen = strlen(key);
    int lump = W_CheckNumForName("DEHACKED");
    const char *p, *end, *a, *b;
    int strings = 0;
    if (lump < 0)
        return NULL;
    p = W_CacheLumpNum(lump, PU_STATIC);
    end = p + W_LumpLength(lump);
    while (deh_line(&p, end, &a, &b)) {
        if (a < b && *a == '[') {
            strings = b - a == 9 && !strncasecmp(a, "[STRINGS]", 9);
            continue;
        }
        if (!strings || b - a < (ptrdiff_t)klen || strncasecmp(a, key, klen))
            continue;
        for (a += klen; a < b && isspace((unsigned char)*a); a++)
            ;
        if (a == b || *a != '=')
            continue;
        for (a++; a < b && isspace((unsigned char)*a); a++)
            ;
        char *out = malloc(end - a + 1), *o = out;
        if (!out)
            return NULL;
        for (;;) {
            int more = b > a && b[-1] == '\\';
            for (const char *c = a; c < b - more; c++) {
                if (*c == '\\' && c + 1 < b - more) {
                    c++;
                    *o++ = *c == 'n' ? '\n' : *c == 't' ? '\t' : *c;
                } else
                    *o++ = *c;
            }
            if (!more || !deh_line(&p, end, &a, &b))
                break;
        }
        *o = 0;
        return out;
    }
    return NULL;
}

void __wrap_F_StartFinale(void)
{
    static char *text, *flat;
    __real_F_StartFinale();
    if (gameepisode == 1 && gamemap == 8) {
        if (!text)
            text = deh_string("E1TEXT");
        if (!flat)
            flat = deh_string("BGFLATE1");
        if (text)
            finaletext = text;
        if (flat)
            finaleflat = flat;
    }
}
