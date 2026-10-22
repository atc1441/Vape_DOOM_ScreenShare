/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright (C) 2026 Artur Bieniek */

/* Memory map of the DMC-100 app. Included by app.ld and by the overlay links. */

MEMORY
{
    /* App slot code area. The settings page follows it. */
    FLASH    (rx)  : ORIGIN = 0x08007000, LENGTH = 0x18C00
    SETTINGS (r)   : ORIGIN = 0x0801FC00, LENGTH = 0x400
    RAM      (rwx) : ORIGIN = 0x20000000, LENGTH = 0x8000
}

/* Stack at the bottom of SRAM, the bootloader's 16 byte pop lands just above it. The DOOM
   firmware links with -Wl,--defsym=STACK_SIZE_OVERRIDE=0x600, which gives 512 B of its stack
   to its fixed core (mk/fw.mk). */
STACK_SIZE = DEFINED(STACK_SIZE_OVERRIDE) ? STACK_SIZE_OVERRIDE : 0x800;

/* Fixed core (.data + .bss) budget: what the stack leaves of the 7,264 B that both take. A
   small app can raise it with -Wl,--defsym=CORE_SIZE_OVERRIDE=<bytes>, which moves FRAME_BASE
   up. */
CORE_SIZE = DEFINED(CORE_SIZE_OVERRIDE) ? CORE_SIZE_OVERRIDE : 7264 - STACK_SIZE;

FRAME_BASE = ORIGIN(RAM) + STACK_SIZE + CORE_SIZE;
RAM_END    = ORIGIN(RAM) + LENGTH(RAM);

/* Per-plan sizes. Overlays link at FRAME_BASE within FRAME_MIN. */
FRAME_MIN = 8368;    /* plan D */
FRAME_MAX = 15208;   /* plan A */
CACHE_MIN = 2336;    /* record cache only */
CACHE_MAX = 6448;    /* record cache and four 32x32 flat slots */
ARENA_MIN = 3848;    /* smallest level arena (E1M8, plan A) */

SETTINGS_PAGE = ORIGIN(SETTINGS);
