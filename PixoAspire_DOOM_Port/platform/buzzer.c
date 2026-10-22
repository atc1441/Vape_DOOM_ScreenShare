// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright(C) 2005-2014 Simon Howard
// Copyright (C) 2026 Artur Bieniek
//
// Passive buzzer on PB8 (TMR4 CH3, PWM). Plays DMX PC-speaker tone bytes: the SysTick
// interrupt adds 140 to an accumulator every millisecond and takes the next tone byte each
// time it passes 1000, which gives exactly 140 steps per second.

#include "at32.h"
#include "platform.h"

#define TMR_HZ 9000000u     // 144 MHz / 16
#define PIT_HZ 1193181ull   // PC timer input clock

// PIT divisor per tone byte, as in vanilla DOOM's PC speaker driver (table from Chocolate
// Doom src/i_pcsound.c, Copyright (C) 2005-2014 Simon Howard, GPL-2.0-or-later), turned
// into TMR4 periods at compile time.
#define P(d) (uint16_t)(((d) * (unsigned long long)TMR_HZ + PIT_HZ / 2) / PIT_HZ)
#define P8(a, b, c, d, e, f, g, h) P(a), P(b), P(c), P(d), P(e), P(f), P(g), P(h)
static const uint16_t period[128] = {
    0,
    P(6818), P(6628), P(6449), P(6279), P(6087), P(5906), P(5736),
    P8(5575, 5423, 5279, 5120, 4971, 4830, 4697, 4554),
    P8(4435, 4307, 4186, 4058, 3950, 3836, 3728, 3615),
    P8(3519, 3418, 3323, 3224, 3131, 3043, 2960, 2875),
    P8(2794, 2711, 2633, 2560, 2485, 2415, 2348, 2281),
    P8(2213, 2153, 2089, 2032, 1975, 1918, 1864, 1810),
    P8(1757, 1709, 1659, 1612, 1565, 1521, 1478, 1435),
    P8(1395, 1355, 1316, 1280, 1242, 1207, 1173, 1140),
    P8(1107, 1075, 1045, 1015, 986, 959, 931, 905),
    P8(879, 854, 829, 806, 783, 760, 739, 718),
    P8(697, 677, 658, 640, 621, 604, 586, 570),
    P8(553, 538, 522, 507, 493, 479, 465, 452),
    P8(439, 427, 415, 403, 391, 380, 369, 359),
    P8(348, 339, 329, 319, 310, 302, 293, 285),
    P8(276, 269, 261, 253, 246, 239, 232, 226),
    P8(219, 213, 207, 201, 195, 190, 184, 179),
};

static uint8_t buf[SND_MAX];
static volatile uint8_t pos, len;
static uint16_t acc;
static uint8_t vol = SND_HIGH;
static volatile uint32_t nsteps;

static void out(uint32_t per, uint32_t duty)
{
    if (per) {
        TMR_PR(TMR4) = per - 1u;
        TMR_C3DT(TMR4) = duty;
        TMR_SWEVT(TMR4) = 1u;  // load the new period now
    } else {
        TMR_C3DT(TMR4) = 0;
    }
}

void snd_init(void)
{
    pin_cfg(GPIOB, PIN_BUZZER, PIN_AF_PP);
    TMR_CTRL1(TMR4) = 0;
    TMR_DIV(TMR4) = PLAT_CPU_HZ / TMR_HZ - 1u;
    TMR_PR(TMR4) = 0xFFFFu;
    TMR_C3DT(TMR4) = 0;
    TMR_CM2(TMR4) = (6u << 4) | (1u << 3);  // CH3 PWM mode A, preload
    TMR_CCTRL(TMR4) = 1u << 8;              // CH3 output on
    TMR_SWEVT(TMR4) = 1u;
    TMR_CTRL1(TMR4) = (1u << 7) | 1u;       // period preload, run
}

// SysTick context
void snd_tick(void)
{
    acc += 140u;
    if (acc < 1000u)
        return;
    acc -= 1000u;
    nsteps++;
    uint8_t p = pos, n = len;
    if (!n)
        return;
    if (p >= n) {
        len = 0;
        out(0, 0);
        return;
    }
    uint8_t t = buf[p];
    pos = (uint8_t)(p + 1u);
    uint32_t per = t < 128u ? period[t] : 0u;
    if (!per || vol == SND_OFF)
        out(0, 0);
    else
        out(per, vol == SND_HIGH ? per / 2u : per / 8u);
}

void snd_play(const uint8_t *tones, uint32_t n)
{
    if (n > SND_MAX)
        n = SND_MAX;
    uint32_t m = irq_save();
    len = 0;
    for (uint32_t i = 0; i < n; i++)
        buf[i] = tones[i];
    pos = 0;
    len = (uint8_t)n;
    irq_restore(m);
}

void snd_stop(void)
{
    uint32_t m = irq_save();
    len = 0;
    out(0, 0);
    irq_restore(m);
}

bool snd_busy(void) { return len != 0; }

void snd_volume(int v)
{
    vol = (uint8_t)v;
    if (v == SND_OFF)
        snd_stop();
}

void snd_tone(uint32_t hz)
{
    snd_stop();
    if (hz >= 140u && hz <= 20000u) {
        uint32_t per = TMR_HZ / hz;
        out(per, per / 2u);
    }
}

uint32_t snd_steps(void) { return nsteps; }
