// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// 1 kHz SysTick interrupt: millisecond time, button sampling and the PC-speaker stepper.

#include "at32.h"
#include "platform.h"

static volatile uint32_t ms_now;

uint32_t plat_ms(void) { return ms_now; }

void SysTick_Handler(void)
{
    ms_now++;
    buttons_sample();
    snd_tick();
    irq_end();
}

void systick_init(void)
{
    SYST_CSR = 0;
    SYST_RVR = PLAT_CPU_HZ / 1000u - 1u;
    SYST_CVR = 0;
    SCB_SHPR3 = (SCB_SHPR3 & 0x00FFFFFFu) | (IRQ_PRIO << 24);
    SYST_CSR = 7u;  // processor clock, interrupt, enable
}
