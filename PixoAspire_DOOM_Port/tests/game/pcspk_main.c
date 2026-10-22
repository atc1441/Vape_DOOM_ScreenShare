// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Host harness for port/game/s_pcspk.c. The engine's s_sound.c sits on top, platform/buzzer.c
// with a register model below, and the SysTick hook (snd_tick) is driven by hand. Commands
// come on stdin, one per line:
//   names                  sfx table: "sfx N NAME PRIORITY LINKED"
//   vol V                  snd_volume(V)
//   start O SFX            S_StartSound from origin O (0 = none, 1-15 objects next to the player)
//   stop O                 S_StopSound(O)
//   frame                  S_UpdateSounds, as once per frame in the main loop
//   istart SFX CH | istop H | iplaying H     the I_* interface directly
//   init OFF SIZE          S_PcsInit with this SOUNDS section, reads outside it fail
//   raw N                  snd_play of N tone bytes 60, as another module would
//   busy                   prints "busy snd_busy()"
//   ms N                   N SysTick periods
//   run                    SysTick until the stepper is idle, then 3 more steps
//   mark TEXT              echo
// Every 140 Hz step prints "step PERIOD DUTY" from the TMR4 registers (PERIOD 0 = silent).
// At start it prints "channels N" and "sounds OFF SIZE" of the image's SOUNDS section.
// Usage: pcspk_test DOOM.DAT

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dmcdat.h"
#include "platform.h"
#include "s_pcspk.h"

#include "doomstat.h"
#include "i_sound.h"
#include "i_system.h"
#include "m_fixed.h"
#include "s_sound.h"
#include "sounds.h"
#include "tables.h"
#include "z_zone.h"

void snd_init(void);
void snd_tick(void);
extern int snd_channels;
angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);

// ---- Register model for buzzer.c ----

#define TMR4_PR   0x4000082Cu
#define TMR4_C3DT 0x4000083Cu

static struct {
    uint32_t addr, val;
} regs[64];
static int nregs;

volatile uint32_t *reg_model(uint32_t addr)
{
    for (int i = 0; i < nregs; i++)
        if (regs[i].addr == addr)
            return &regs[i].val;
    if (nregs == 64) {
        fprintf(stderr, "register file full\n");
        exit(2);
    }
    regs[nregs].addr = addr;
    return &regs[nregs++].val;
}

uint32_t irq_save(void) { return 0; }
void irq_restore(uint32_t m) { (void)m; }

// ---- What s_sound.c needs from the engine ----

player_t players[MAXPLAYERS];
int consoleplayer;
int gamemap = 1;
int dmc_finesine(unsigned i)
{
    (void)i;
    return 0;
}
static mobj_t objs[16];

fixed_t FixedMul(fixed_t a, fixed_t b) { return (fixed_t)(((int64_t)a * b) >> FRACBITS); }

angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{
    (void)x1;
    (void)y1;
    (void)x2;
    (void)y2;
    return 0;
}

void I_Error(char *error, ...)
{
    va_list ap;
    va_start(ap, error);
    vfprintf(stderr, error, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(1);
}

void *Z_Malloc(int size, int tag, void *ptr)
{
    (void)tag;
    (void)ptr;
    void *p = calloc(1, (size_t)size);
    if (!p)
        I_Error("Z_Malloc %d", size);
    return p;
}

// ---- Image ----

static FILE *image;
static long image_size;
static long win_lo, win_hi;  // reads allowed here, the SOUNDS section given to S_PcsInit

static void image_read(uint32_t off, void *dst, uint32_t n)
{
    if ((long)off < win_lo || (long)off + (long)n > win_hi ||
        fseek(image, (long)off, SEEK_SET) || fread(dst, 1, n, image) != n)
        I_Error("image read %u+%u outside %ld..%ld", off, n, win_lo, win_hi);
}

static void init(uint32_t off, uint32_t size)
{
    win_lo = off;
    win_hi = (long)off + (long)size;
    S_PcsInit(image_read, off, size);
}

static uint32_t sounds_section(uint32_t *size)
{
    uint8_t h[DMC_HEADER_SIZE];
    image_read(0, h, sizeof(h));
    if (memcmp(h, DMC_MAGIC, 8))
        I_Error("not a DMCDOOM image");
    unsigned nsect = h[92] | h[93] << 8;
    for (unsigned i = 0; i < nsect; i++) {
        const uint8_t *e = h + DMC_SECT_TABLE_OFF + 16 * i;
        if ((e[0] | e[1] << 8) == DMC_SECT_SOUNDS) {
            *size = e[8] | e[9] << 8 | e[10] << 16 | (uint32_t)e[11] << 24;
            return e[4] | e[5] << 8 | e[6] << 16 | (uint32_t)e[7] << 24;
        }
    }
    I_Error("no SOUNDS section");
    return 0;
}

// ---- SysTick ----

static void tick(void)
{
    uint32_t s = snd_steps();
    snd_tick();
    if (snd_steps() == s)
        return;
    uint32_t duty = *reg_model(TMR4_C3DT);
    printf("step %u %u\n", duty ? *reg_model(TMR4_PR) + 1u : 0u, duty);
}

static mobj_t *origin(int o)
{
    if (o < 0 || o > 15)
        I_Error("bad origin %d", o);
    return o ? &objs[o] : NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: pcspk_test DOOM.DAT\n");
        return 2;
    }
    image = fopen(argv[1], "rb");
    if (!image || fseek(image, 0, SEEK_END) || (image_size = ftell(image)) < DMC_HEADER_SIZE) {
        fprintf(stderr, "cannot read %s\n", argv[1]);
        return 2;
    }
    players[0].mo = &objs[0];
    snd_init();
    win_hi = image_size;
    uint32_t size, off = sounds_section(&size);
    init(off, size);
    S_Init(120);
    printf("channels %d\n", snd_channels);
    printf("sounds %u %u\n", off, size);

    char line[256];
    while (fgets(line, sizeof(line), stdin)) {
        char cmd[16];
        int a = 0, b = 0;
        int n = sscanf(line, "%15s %d %d", cmd, &a, &b);
        if (n < 1)
            continue;
        if (!strcmp(cmd, "names")) {
            for (int i = 1; i < NUMSFX; i++)
                printf("sfx %d %s %d %d\n", i, dmc_sfxnames[i], S_sfx[i].priority, S_sfx[i].link != 0);
        } else if (!strcmp(cmd, "vol") && n == 2) {
            snd_volume(a);
        } else if (!strcmp(cmd, "start") && n == 3) {
            S_StartSound(origin(a), b);
        } else if (!strcmp(cmd, "stop") && n == 2) {
            S_StopSound(origin(a));
        } else if (!strcmp(cmd, "frame")) {
            S_UpdateSounds(players[0].mo);
        } else if (!strcmp(cmd, "istart") && n == 3) {
            printf("handle %d\n", I_StartSound(&S_sfx[a], b, 127, 128));
        } else if (!strcmp(cmd, "istop") && n == 2) {
            I_StopSound(a);
        } else if (!strcmp(cmd, "iplaying") && n == 2) {
            printf("playing %d %d\n", a, (int)I_SoundIsPlaying(a));
        } else if (!strcmp(cmd, "init") && n == 3) {
            init((uint32_t)a, (uint32_t)b);
        } else if (!strcmp(cmd, "raw") && n == 2) {
            uint8_t t[SND_MAX];
            memset(t, 60, sizeof(t));
            snd_play(t, (uint32_t)a);
        } else if (!strcmp(cmd, "busy")) {
            printf("busy %d\n", (int)snd_busy());
        } else if (!strcmp(cmd, "ms") && n == 2) {
            while (a-- > 0)
                tick();
        } else if (!strcmp(cmd, "run")) {
            int limit = 10000;
            while (snd_busy() && limit--)
                tick();
            for (uint32_t s = snd_steps(); snd_steps() - s < 3;)
                tick();
        } else if (!strcmp(cmd, "mark")) {
            fputs(line, stdout);
        } else {
            I_Error("bad command: %s", line);
        }
    }
    return 0;
}
