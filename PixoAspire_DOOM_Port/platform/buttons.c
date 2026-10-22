// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Six buttons, active low with pull-ups. Sampled by the SysTick interrupt, a state change
// needs three equal samples in a row, edges go into an 8-entry ring.

#include "at32.h"
#include "platform.h"

static const struct {
    uint32_t port;
    uint8_t pin;
} pins[BTN_COUNT] = {
    [BTN_HOLD] = {GPIOB, 4}, [BTN_SAVE] = {GPIOB, 5},   [BTN_MODE] = {GPIOB, 6},
    [BTN_VA] = {GPIOB, 7},   [BTN_LIGHT] = {GPIOA, 15}, [BTN_POWER] = {GPIOF, 7},
};

static uint8_t s1, s2;
static volatile uint8_t stable;
static uint8_t ring[8];
static volatile uint8_t head, tail;

void buttons_init(void)
{
    for (unsigned i = 0; i < BTN_COUNT; i++) {
        pin_cfg(pins[i].port, pins[i].pin, PIN_IN_PU);
        pin_high(pins[i].port, pins[i].pin);  // pull-up
    }
}

static uint32_t raw(void)
{
    uint32_t b = 0;
    for (unsigned i = 0; i < BTN_COUNT; i++) {
        if (!(GPIO_IDT(pins[i].port) & (1u << pins[i].pin)))
            b |= 1u << i;
    }
    return b;
}

// SysTick context
void buttons_sample(void)
{
    uint8_t s0 = (uint8_t)raw();
    uint8_t on = s0 & s1 & s2, off = (uint8_t)~(s0 | s1 | s2);
    uint8_t next = (uint8_t)((stable | on) & ~off);
    uint8_t diff = next ^ stable;
    s2 = s1;
    s1 = s0;
    stable = next;
    for (unsigned i = 0; diff; i++, diff >>= 1) {
        if (!(diff & 1u))
            continue;
        uint8_t h = head;
        if ((uint8_t)(h - tail) >= sizeof(ring))
            continue;
        ring[h & 7u] = (uint8_t)(i | ((next >> i) & 1u ? EV_PRESS : 0));
        __asm volatile("" ::: "memory");
        head = (uint8_t)(h + 1u);
    }
}

uint32_t plat_buttons(void) { return stable; }

int plat_event(void)
{
    uint8_t t = tail;
    if (t == head)
        return -1;
    __asm volatile("" ::: "memory");
    int ev = ring[t & 7u];
    tail = (uint8_t)(t + 1u);
    return ev;
}
