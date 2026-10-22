// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Host tool linked against the unmodified upstream sources. Prints the
// upstream tables (info, items, sounds, trig, view and light tables) as JSON
// so the converter never parses C.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomstat.h"
#include "d_items.h"
#include "info.h"
#include "i_sound.h"
#include "m_fixed.h"
#include "r_local.h"
#include "p_spec.h"
#include "sounds.h"
#include "tables.h"

struct dmc_named_ptr { const char *name; const void *ptr; };
extern const struct dmc_named_ptr dmc_action_names[];
extern const struct dmc_named_ptr dmc_string_values[];
extern char *doom1_endmsg[];
extern int pars[4][10];
extern switchlist_t alphSwitchList[];
extern void dmc_dump_animdefs(FILE *f);
extern void R_ExecuteSetViewSize(void);
extern void R_InitLightTables(void);

// Enum objects keep the enumerators in the debug info read by the converter.
statenum_t dmc_enum_state;
mobjtype_t dmc_enum_mobj;
spritenum_t dmc_enum_sprite;
sfxenum_t dmc_enum_sfx;
weapontype_t dmc_enum_weapon;
ammotype_t dmc_enum_ammo;

extern int setblocks, setdetail;
extern lighttable_t *colormaps;
extern lighttable_t *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *zlight[LIGHTLEVELS][MAXLIGHTZ];
extern int viewangletox[FINEANGLES / 2];
extern angle_t xtoviewangle[SCREENWIDTH + 1];
extern fixed_t yslope[SCREENHEIGHT];
extern fixed_t distscale[SCREENWIDTH];
extern int viewwidth, viewheight, detailshift;

static lighttable_t cmap_dummy[34 * 256];

void DG_Init(void) {}
void DG_DrawFrame(void) {}
void DG_SleepMs(uint32_t ms) { (void)ms; }
uint32_t DG_GetTicksMs(void) { return 0; }
int DG_GetKey(int *pressed, unsigned char *key) { (void)pressed; (void)key; return 0; }
void DG_SetWindowTitle(const char *title) { (void)title; }

static void jstr(FILE *f, const char *s)
{
    fputc('"', f);
    for (; s && *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\')
            fprintf(f, "\\%c", c);
        else if (c < 32 || c > 126)
            fprintf(f, "\\u%04x", c);
        else
            fputc(c, f);
    }
    fputc('"', f);
}

static const char *action_name(const void *p)
{
    if (!p)
        return "";
    for (const struct dmc_named_ptr *a = dmc_action_names; a->name; a++)
        if (a->ptr == p)
            return a->name;
    fprintf(stderr, "unknown action pointer\n");
    exit(1);
}

static void int_array(FILE *f, const char *name, const int *v, int n, int last)
{
    fprintf(f, "\"%s\":[", name);
    for (int i = 0; i < n; i++)
        fprintf(f, "%s%d", i ? "," : "", v[i]);
    fprintf(f, "]%s\n", last ? "" : ",");
}

static void view_tables(FILE *f, int detail)
{
    static int tmp[SCREENWIDTH * 2 + 4096];
    setblocks = 11;
    setdetail = detail;
    colormaps = cmap_dummy;
    R_ExecuteSetViewSize();
    fprintf(f, "{\"detail\":%d,\"viewwidth\":%d,\"viewheight\":%d,\n", detail, viewwidth, viewheight);
    int_array(f, "viewangletox", viewangletox, FINEANGLES / 2, 0);
    for (int i = 0; i <= viewwidth; i++)
        tmp[i] = (int)xtoviewangle[i];
    int_array(f, "xtoviewangle", tmp, viewwidth + 1, 0);
    int_array(f, "yslope", yslope, viewheight, 0);
    int_array(f, "distscale", distscale, viewwidth, 0);
    for (int i = 0; i < LIGHTLEVELS; i++)
        for (int j = 0; j < MAXLIGHTSCALE; j++)
            tmp[i * MAXLIGHTSCALE + j] = (int)((scalelight[i][j] - colormaps) / 256);
    int_array(f, "scalelight", tmp, LIGHTLEVELS * MAXLIGHTSCALE, 1);
    fprintf(f, "}");
}

int main(void)
{
    FILE *f = stdout;
    fprintf(f, "{\n\"numstates\":%d,\"nummobjtypes\":%d,\"numsprites\":%d,\"numsfx\":%d,\"numweapons\":%d,\n",
            NUMSTATES, NUMMOBJTYPES, NUMSPRITES, NUMSFX, NUMWEAPONS);

    fprintf(f, "\"states\":[\n");
    for (int i = 0; i < NUMSTATES; i++) {
        const state_t *s = &states[i];
        fprintf(f, "%s[%d,%d,%d,", i ? ",\n" : "", (int)s->sprite, s->frame, s->tics);
        jstr(f, action_name((const void *)s->action.acv));
        fprintf(f, ",%d,%d,%d]", (int)s->nextstate, s->misc1, s->misc2);
    }
    fprintf(f, "],\n\"sprnames\":[");
    for (int i = 0; i < NUMSPRITES; i++) {
        fprintf(f, "%s", i ? "," : "");
        jstr(f, sprnames[i]);
    }

    fprintf(f, "],\n\"mobjinfo\":[\n");
    for (int i = 0; i < NUMMOBJTYPES; i++) {
        const mobjinfo_t *m = &mobjinfo[i];
        fprintf(f, "%s{\"doomednum\":%d,\"spawnstate\":%d,\"spawnhealth\":%d,\"seestate\":%d,"
                "\"seesound\":%d,\"reactiontime\":%d,\"attacksound\":%d,\"painstate\":%d,"
                "\"painchance\":%d,\"painsound\":%d,\"meleestate\":%d,\"missilestate\":%d,"
                "\"deathstate\":%d,\"xdeathstate\":%d,\"deathsound\":%d,\"speed\":%d,\"radius\":%d,"
                "\"height\":%d,\"mass\":%d,\"damage\":%d,\"activesound\":%d,\"flags\":%d,"
                "\"raisestate\":%d}",
                i ? ",\n" : "", m->doomednum, m->spawnstate, m->spawnhealth, m->seestate, m->seesound,
                m->reactiontime, m->attacksound, m->painstate, m->painchance, m->painsound,
                m->meleestate, m->missilestate, m->deathstate, m->xdeathstate, m->deathsound,
                m->speed, m->radius, m->height, m->mass, m->damage, m->activesound, m->flags,
                m->raisestate);
    }

    fprintf(f, "],\n\"weaponinfo\":[\n");
    for (int i = 0; i < NUMWEAPONS; i++) {
        const weaponinfo_t *w = &weaponinfo[i];
        fprintf(f, "%s[%d,%d,%d,%d,%d,%d]", i ? ",\n" : "", (int)w->ammo, w->upstate, w->downstate,
                w->readystate, w->atkstate, w->flashstate);
    }

    fprintf(f, "],\n\"sfx\":[\n");
    for (int i = 0; i < NUMSFX; i++) {
        const sfxinfo_t *s = &S_sfx[i];
        fprintf(f, "%s[", i ? ",\n" : "");
        jstr(f, s->name);
        fprintf(f, ",%d,%d,%d,%d]", s->priority, s->link ? (int)(s->link - S_sfx) : -1, s->pitch,
                s->volume);
    }

    fprintf(f, "],\n\"switches\":[");
    for (int i = 0; alphSwitchList[i].episode; i++) {
        fprintf(f, "%s[", i ? "," : "");
        jstr(f, alphSwitchList[i].name1);
        fputc(',', f);
        jstr(f, alphSwitchList[i].name2);
        fprintf(f, ",%d]", alphSwitchList[i].episode);
    }
    fprintf(f, "],\n\"animdefs\":");
    dmc_dump_animdefs(f);

    fprintf(f, ",\n\"pars\":[");
    for (int e = 0; e < 4; e++)
        for (int m = 0; m < 10; m++)
            fprintf(f, "%s%d", e + m ? "," : "", pars[e][m]);
    fprintf(f, "],\n\"strings\":{");
    for (const struct dmc_named_ptr *s = dmc_string_values; s->name; s++) {
        fprintf(f, "%s", s == dmc_string_values ? "" : ",\n");
        jstr(f, s->name);
        fputc(':', f);
        jstr(f, (const char *)s->ptr);
    }
    fprintf(f, "},\n\"endmsg\":[");
    for (int i = 0; i < 8; i++) {
        fprintf(f, "%s", i ? "," : "");
        jstr(f, doom1_endmsg[i]);
    }
    fprintf(f, "],\n");

    int_array(f, "finesine", finesine, 5 * FINEANGLES / 4, 0);
    int_array(f, "finetangent", finetangent, FINEANGLES / 2, 0);
    {
        static int tta[SLOPERANGE + 1];
        for (int i = 0; i <= SLOPERANGE; i++)
            tta[i] = (int)tantoangle[i];
        int_array(f, "tantoangle", tta, SLOPERANGE + 1, 0);
    }
    {
        static int zl[LIGHTLEVELS * MAXLIGHTZ];
        colormaps = cmap_dummy;
        R_InitLightTables();
        for (int i = 0; i < LIGHTLEVELS; i++)
            for (int j = 0; j < MAXLIGHTZ; j++)
                zl[i * MAXLIGHTZ + j] = (int)((zlight[i][j] - colormaps) / 256);
        int_array(f, "zlight", zl, LIGHTLEVELS * MAXLIGHTZ, 0);
    }
    fprintf(f, "\"view\":[");
    view_tables(f, 0);
    fprintf(f, ",\n");
    view_tables(f, 1);
    fprintf(f, "]}\n");
    return 0;
}
