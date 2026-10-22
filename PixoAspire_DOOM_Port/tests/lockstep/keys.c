// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Key event file of keys.h, compiled against the doomkeys.h of whichever
// tree it is linked into (both have the same values).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "keys.h"

static const struct {
    const char *name;
    int key;
} names[] = {
    { "KEY_ESCAPE", KEY_ESCAPE },         { "KEY_F10", KEY_F10 },
    { "KEY_UPARROW", KEY_UPARROW },       { "KEY_DOWNARROW", KEY_DOWNARROW },
    { "KEY_LEFTARROW", KEY_LEFTARROW },   { "KEY_RIGHTARROW", KEY_RIGHTARROW },
    { "KEY_ENTER", KEY_ENTER },           { "KEY_BACKSPACE", KEY_BACKSPACE },
    { "y", 'y' },                         { "n", 'n' },
    { "KEY_STRAFE_L", KEY_STRAFE_L },     { "KEY_STRAFE_R", KEY_STRAFE_R },
    { "KEY_FIRE", KEY_FIRE },             { "KEY_USE", KEY_USE },
    { "KEY_RSHIFT", KEY_RSHIFT },         { "KEY_RALT", KEY_RALT },
};

typedef struct {
    int tic, down, key;
} kev_t;

static kev_t *ev;
static int nev, cap, next;

static int fail(const char *path, int line, const char *why)
{
    fprintf(stderr, "%s:%d: %s\n", path, line, why);
    return -1;
}

int LS_LoadKeys(const char *path)
{
    FILE *f = fopen(path, "r");
    char buf[256], name[64], dir[8];
    int line = 0, r = 0;
    if (!f) {
        fprintf(stderr, "cannot read %s\n", path);
        return -1;
    }
    while (fgets(buf, sizeof(buf), f)) {
        int tic, n, end = 0, k = -1;
        char *c = strchr(buf, '#');
        line++;
        if (c)
            *c = 0;
        if (strspn(buf, " \t\r\n") == strlen(buf))
            continue;
        n = sscanf(buf, "%d %63s %7s %n", &tic, name, dir, &end);
        if (n != 3 || buf[end] || tic < 0 || (strcmp(dir, "down") && strcmp(dir, "up"))) {
            r = fail(path, line, "expected: tic key down|up");
            break;
        }
        for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]); i++)
            if (!strcmp(name, names[i].name))
                k = names[i].key;
        if (k < 0) {
            r = fail(path, line, "not a key of the meter");
            break;
        }
        if (nev && tic < ev[nev - 1].tic) {
            r = fail(path, line, "tics out of order");
            break;
        }
        if (nev && tic == ev[nev - 1].tic && !ev[nev - 1].down) {
            r = fail(path, line,
                     "an event after the key up of its tic (the oracle stops reading at a key up)");
            break;
        }
        if (nev == cap) {
            cap = cap ? 2 * cap : 64;
            ev = realloc(ev, cap * sizeof(*ev));
            if (!ev) {
                fprintf(stderr, "LS_LoadKeys: out of memory\n");
                exit(2);
            }
        }
        ev[nev++] = (kev_t){ tic, !strcmp(dir, "down"), k };
    }
    fclose(f);
    return r;
}

int LS_NextKey(int tic, int *down, int *key)
{
    while (next < nev && ev[next].tic < tic)
        next++;
    if (next == nev || ev[next].tic != tic)
        return 0;
    *down = ev[next].down;
    *key = ev[next].key;
    next++;
    return 1;
}
