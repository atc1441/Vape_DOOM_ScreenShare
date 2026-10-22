// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Host NOR statistics: transactions and bytes per tic and per rendered
// frame, record cache hits and misses and NOR traffic per record kind
// (map stream or image section) and context, the engine events of each tic
// (LINES entries by thing, line openings, sight checks) and the second
// level of the record cache when it is on. Written as text lines by
// HS_Close.

#ifndef DMC_HOST_STATS_H
#define DMC_HOST_STATS_H

enum { HS_TIC, HS_FRAME, HS_UI, HS_NCTX };

int HS_Open(const char *path);
// Starts a period of the context. Traffic is counted for one context at a
// time, HS_Pause and HS_Resume leave a period open across another one.
void HS_Begin(int ctx);
void HS_Pause(int ctx);
void HS_Resume(int ctx);
// Ends the period of the context: one line with its traffic. tic is
// gametic, flags 1 marks a level load tic, 2 a period outside a level.
void HS_End(int ctx, int tic, int flags);
void HS_Close(void);
// The second level of the record cache during the thinkers of each tic: 0 off,
// 1 dropped where the LINES overlay is entered past the tests of the other
// things (P_CrossSpecialLine, P_ShootSpecialLine, P_UseSpecialLine), 2
// dropped at every LINES entry.
void HS_Rc2(int mode);

#endif
