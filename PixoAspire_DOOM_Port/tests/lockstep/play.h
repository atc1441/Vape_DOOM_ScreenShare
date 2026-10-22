// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Play aids of the corpus runs, shared by both runners (tests/lockstep/
// play.c). The oracle and the engine make the same changes to the game,
// so lockstep still compares them tic by tic.
//
//   god mode      the player takes no damage below 1000 (CF_GODMODE)
//   refill mode   health 100 and 200 blue armor before every
//                 P_PlayerThink: the player is hurt (attacker, pain) and
//                 lives on
//   both modes    the player holds every weapon of E1 in single player
//                 (all but the BFG and the super shotgun) with full ammo
//                 and the six keycards, set before every P_PlayerThink
//   no exit       G_ExitLevel and G_SecretExitLevel do nothing, a run
//                 stays on its map
//   tour          the player is moved (P_TeleportMove) at given tics, before
//                 P_PlayerThink of that tic. Text file, one stop per line:
//                 gametic x y angle (map units, angle as BAM)
//   route         per map and level time, so a run follows it however long
//                 its intermissions last. Text file, # starts a comment:
//                   cmd MAP LEVELTIME COUNT FORWARD SIDE TURN BUTTONS
//                   stop MAP LEVELTIME X Y ANGLE
//                 In a level the player's ticcmd is replaced before
//                 P_PlayerThink by the cmd line that covers the level time
//                 (all zero when none does) and a stop moves the player as
//                 a tour stop does. Outside a level the runner builds a
//                 ticcmd that fires on every 8th tic (LS_RouteIdle), which
//                 moves the intermission on. Several files may be given.
//   exit list     forced level exits by level time (LS_ExitList): before
//                 G_Ticker, when a level runs and its leveltime is the k-th
//                 value, G_ExitLevel (G_SecretExitLevel for a value with a
//                 trailing s), then the next value waits for a later level

#ifndef LS_PLAY_H
#define LS_PLAY_H

extern int LS_God;
extern int LS_Refill;
extern int LS_NoExit;

int LS_LoadTour(const char *path);
void LS_AddTour(int tic, int x, int y, unsigned angle);

int LS_LoadRoute(const char *path);
int LS_HaveRoute(void);
void LS_RouteIdle(void *ticcmd, int maketic);

// LIST: L1,L2,L3s,... Returns 0, or -1 for a malformed list.
int LS_ExitList(const char *list);
// Called by the runners' G_Ticker wrap before the real G_Ticker.
void LS_ExitTic(void);

#endif
