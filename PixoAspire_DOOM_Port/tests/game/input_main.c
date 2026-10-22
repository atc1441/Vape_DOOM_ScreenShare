// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Host harness for port/game/i_input.c with a scripted button ring. Commands on stdin, one
// per line:
//   cfg PRESET FLIP RUN          II_Configure with a preset, prints "speed KEY_SPEED"
//   map FLIP RUN A0 ... A9       II_Configure with a custom map
//   weap READY PENDING OWNED STRENGTH   player weapon state (PENDING -1 = no change, OWNED is
//                                a bit mask)
//   speed                        prints "speed KEY_SPEED"
//   pad BUTTONS                  the game pad's PAD_* bits (port/game/pad.h), 0 without a pad
//   state GAMESTATE DEMO         gamestate and demoplayback (both 0 at the start: a level)
//   t CTX MASK [drop | E...]     one tic: CTX g (game), m (menu), p (y/n prompt), MASK the
//                                buttons held at the end of the tic. Without more arguments
//                                the edges between the masks go into the ring, "drop" puts
//                                none there (full ring), E is an explicit edge, +B or -B.
//   u CTX MASK [drop | E...]     the same for an I_StartTic call that builds no ticcmd
//                                (D_WillBuildTic false)
// Each event prints "TIC +KEY" or "TIC -KEY", TIC counts t and u lines.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "i_input.h"
#include "pad.h"
#include "platform.h"

#include "d_event.h"
#include "doomkeys.h"
#include "doomstat.h"
#include "m_controls.h"

player_t players[MAXPLAYERS];
int consoleplayer;
boolean menuactive;
boolean demoplayback;
gamestate_t gamestate;
int messageToPrint;
boolean messageNeedsInput;

static uint32_t mask;
static int ring[64], rhead, rtail;
static int tic;
static boolean build;
static uint8_t custom[II_MAPLEN];

uint32_t plat_buttons(void) { return mask; }

boolean D_WillBuildTic(void) { return build; }

int plat_event(void) { return rtail == rhead ? -1 : ring[rtail++ & 63]; }

static void push(int ev)
{
    if (rhead - rtail >= 8)  // the platform ring holds 8 edges
        return;
    ring[rhead++ & 63] = ev;
}

static const char *key_name(int k)
{
    static char buf[8];
    switch (k) {
    case KEY_UPARROW: return "UP";
    case KEY_DOWNARROW: return "DOWN";
    case KEY_LEFTARROW: return "LEFT";
    case KEY_RIGHTARROW: return "RIGHT";
    case KEY_STRAFE_L: return "STRAFE_L";
    case KEY_STRAFE_R: return "STRAFE_R";
    case KEY_FIRE: return "FIRE";
    case KEY_USE: return "USE";
    case KEY_RALT: return "RALT";
    case KEY_RSHIFT: return "RSHIFT";
    case KEY_TAB: return "TAB";
    case KEY_ESCAPE: return "ESC";
    case KEY_ENTER: return "ENTER";
    case KEY_BACKSPACE: return "BACKSPACE";
    case KEY_F10: return "F10";
    }
    if ((k >= '1' && k <= '8') || k == 'y' || k == 'n') {
        buf[0] = (char)k;
        buf[1] = 0;
        return buf;
    }
    snprintf(buf, sizeof(buf), "0x%x", k);
    return buf;
}

void D_PostEvent(event_t *ev)
{
    if (ev->type != ev_keydown && ev->type != ev_keyup) {
        printf("%d bad event type %d\n", tic, ev->type);
        return;
    }
    printf("%d %c%s\n", tic, ev->type == ev_keydown ? '+' : '-', key_name(ev->data1));
}

static void bad(const char *line)
{
    fprintf(stderr, "bad command: %s\n", line);
    exit(2);
}

int main(void)
{
    char line[256];
    while (fgets(line, sizeof(line), stdin)) {
        char *tok[16];
        int n = 0;
        for (char *s = strtok(line, " \t\n"); s && n < 16; s = strtok(NULL, " \t\n"))
            tok[n++] = s;
        if (!n)
            continue;
        if (!strcmp(tok[0], "cfg") && n == 4) {
            II_Configure(ii_presets[atoi(tok[1])], atoi(tok[2]), atoi(tok[3]));
            printf("speed %d\n", key_speed);
        } else if (!strcmp(tok[0], "pad") && n == 2) {
            dmc_pad.buttons = (uint16_t)strtoul(tok[1], NULL, 0);
        } else if (!strcmp(tok[0], "state") && n == 3) {
            gamestate = (gamestate_t)atoi(tok[1]);
            demoplayback = atoi(tok[2]);
        } else if (!strcmp(tok[0], "speed") && n == 1) {
            printf("speed %d\n", key_speed);
        } else if (!strcmp(tok[0], "map") && n == 3 + II_MAPLEN) {
            for (int i = 0; i < II_MAPLEN; i++)
                custom[i] = (uint8_t)atoi(tok[3 + i]);
            II_Configure(custom, atoi(tok[1]), atoi(tok[2]));
        } else if (!strcmp(tok[0], "weap") && n == 5) {
            player_t *p = &players[consoleplayer];
            p->readyweapon = atoi(tok[1]);
            p->pendingweapon = atoi(tok[2]) < 0 ? wp_nochange : atoi(tok[2]);
            int owned = atoi(tok[3]);
            for (int w = 0; w < NUMWEAPONS; w++)
                p->weaponowned[w] = owned >> w & 1;
            p->powers[pw_strength] = atoi(tok[4]);
        } else if ((!strcmp(tok[0], "t") || !strcmp(tok[0], "u")) && n >= 3) {
            build = tok[0][0] == 't';
            menuactive = tok[1][0] != 'g';
            messageToPrint = messageNeedsInput = tok[1][0] == 'p';
            uint32_t next = (uint32_t)strtoul(tok[2], NULL, 0);
            if (n == 3) {
                for (int b = 0; b < BTN_COUNT; b++)
                    if ((mask ^ next) >> b & 1)
                        push(b | (next >> b & 1 ? EV_PRESS : 0));
            } else if (strcmp(tok[3], "drop")) {
                for (int i = 3; i < n; i++) {
                    if ((tok[i][0] != '+' && tok[i][0] != '-') || tok[i][1] < '0' || tok[i][1] > '5')
                        bad(tok[i]);
                    push((tok[i][1] - '0') | (tok[i][0] == '+' ? EV_PRESS : 0));
                }
            }
            mask = next;
            I_InputTic();
            tic++;
        } else {
            bad(tok[0]);
        }
    }
    return 0;
}
