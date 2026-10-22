// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Force-included (-include) when pump.c and nor.c are built for the host model test: every
// register access goes through model_reg(), which maps addresses to an emulated register
// file. See model.c.

#pragma once

#include <stdint.h>

volatile uint32_t *model_reg(uint32_t addr);

#define REG32(a) (*(volatile uint32_t *)model_reg(a))
#define REG16(a) (*(volatile uint16_t *)(void *)model_reg(a))
#define REG8(a)  (*(volatile uint8_t *)(void *)model_reg(a))
