// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Force-included (-include) when platform/buzzer.c is built for the sound test: register
// accesses go to a small register file in pcspk_main.c.

#pragma once

#include <stdint.h>

volatile uint32_t *reg_model(uint32_t addr);

#define REG32(a) (*reg_model(a))
#define REG16(a) (*(volatile uint16_t *)(void *)reg_model(a))
#define REG8(a)  (*(volatile uint8_t *)(void *)reg_model(a))
