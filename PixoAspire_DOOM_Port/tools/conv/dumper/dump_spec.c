// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Compiles the unmodified upstream p_spec.c in this unit, so the animation
// definitions (whose type is private to that file) can be printed. The
// dumper links this object instead of p_spec.o.

#include "p_spec.c"

void dmc_dump_animdefs(FILE *f)
{
    fprintf(f, "[");
    for (int i = 0; animdefs[i].istexture != -1; i++)
        fprintf(f, "%s[%d,\"%s\",\"%s\",%d]", i ? "," : "", animdefs[i].istexture,
                animdefs[i].startname, animdefs[i].endname, animdefs[i].speed);
    fprintf(f, "]");
}
