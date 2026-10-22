// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
// Copyright (C) 2026 VapeDoom port
//
// 1 kHz SysTick: millisecond time, both watchdogs and the input sampler.
//
// Feeding the watchdogs from the timer interrupt rather than the main loop is
// the difference between a port that survives a slow frame and one that
// reboots in the middle of it. A full-screen blit takes 25 ms on this board
// and a NOR read can stall on a busy chip; neither may look like a hang to
// the external watchdog chip on PC6.

#include "platform.h"
#include "py32_hw.h"

static volatile uint32_t ms_now;

uint32_t plat_ms(void) { return ms_now; }

void SysTick_Handler(void)
{
    ms_now++;
    wdt_kick();
    buttons_sample();
    snd_tick();
    dbg_tick(ms_now, plat_buttons());
    irq_end();
}

void systick_init(void)
{
    SYST_CSR = 0;
    SYST_RVR = PLAT_CPU_HZ / 1000u - 1u;
    SYST_CVR = 0;
    SCB_SHPR3 = (SCB_SHPR3 & 0x00FFFFFFu) | ((uint32_t)PLAT_IRQ_PRIO << 24);
    SYST_CSR = 7u;  // processor clock, interrupt, enable
}
