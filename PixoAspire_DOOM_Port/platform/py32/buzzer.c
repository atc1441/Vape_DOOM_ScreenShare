// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 VapeDoom port
//
// Sound: the vape has no speaker. It has a vibration motor, but that hangs
// off the same kind of driver stage as the coil and is not worth the risk of
// pulsing it from the PC-speaker stepper, so the whole channel is silent.
// The engine keeps calling into it, so the state it asks about stays sane.

#include "platform.h"
#include "py32_hw.h"

static uint32_t steps;
static int volume = SND_HIGH;

void snd_init(void) { steps = 0; }
void snd_tick(void) { steps++; }
void snd_volume(int vol) { volume = vol; }
void snd_play(const uint8_t *tones, uint32_t n)
{
    (void)tones;
    (void)n;
    (void)volume;
}
void snd_stop(void) {}
bool snd_busy(void) { return false; }
void snd_tone(uint32_t hz) { (void)hz; }
uint32_t snd_steps(void) { return steps; }
