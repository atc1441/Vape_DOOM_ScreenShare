// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Corpus bot of the oracle runner (bot.c).

#ifndef BOT_H
#define BOT_H

#include "d_ticcmd.h"

// Opens PREFIX.cmds and PREFIX.tour.
int BOT_Open(const char *prefix, unsigned seed);
int BOT_Active(void);
// The ticcmd of maketic, also written to PREFIX.cmds.
void BOT_Cmd(ticcmd_t *cmd, int maketic);
void BOT_Close(void);

#endif
