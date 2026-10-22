// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek

#ifndef DMC_HASH_HOOK_H
#define DMC_HASH_HOOK_H

#include <stdint.h>
#include <stdio.h>

// Stop after this many tics when non-zero.
extern int hook_max_tics;

// Call G_ExitLevel before this tic (-1: never).
extern int hook_exit_at;

// Last palette passed to I_SetPalette, raw PLAYPAL RGB.
extern uint8_t hook_palette[768];

// When open, one line after every tic: gametic gamestate gamemap leveltime
// rndindex.
extern FILE *hook_state;

// When set, every frame with the menu or a message up also writes the menu
// alone, drawn into a zeroed 320x200 screen, as DIR/mNNNNNN.png.
extern const char *hook_menu_dir;

#endif
