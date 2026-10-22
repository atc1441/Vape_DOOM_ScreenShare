// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Scripted key events shared by both runners (--keys FILE). Text file, one
// event per line, # starts a comment:
//   TIC KEY down|up
// KEY is a doomkeys.h name (KEY_ESCAPE) or the character itself (y). TIC is
// the gametic at I_StartTic, the tic whose ticcmd is built next: both
// runners call I_StartTic once per tic (singletics) and hand the events of
// that tic to D_ProcessEvents in file order.
//
// Only keys the meter sends (port/game/i_input.c) and the front end handles
// are accepted: Escape, F10, the arrows, Enter, Backspace, y and n for the
// menu, the movement, fire, use, run and strafe keys for the game (no
// automap). Lines must be in tic order, and a key up must be the last event
// of its tic, because the oracle stops reading events after a key up
// (doomgeneric i_input.c, I_GetEvent). The meter sends y and n only as the
// answers of a prompt. Elsewhere in the menu the oracle takes them as item
// shortcuts (it posts the typed character in data2, the engine posts 0), so
// a key file uses them only while a prompt is up.

#ifndef LS_KEYS_H
#define LS_KEYS_H

// Reads the file. 0, or -1 with the reason on stderr.
int LS_LoadKeys(const char *path);

// The next event of tic: 1 with *down and *key set, 0 when every event of
// tic has been taken.
int LS_NextKey(int tic, int *down, int *key);

#endif
