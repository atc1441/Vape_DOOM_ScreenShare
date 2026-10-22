/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright (C) 2026 Artur Bieniek */
/* Copyright (C) 2026 VapeDoom port */

/* Memory map of the Aspire PIXO (PY32F403xC).

   The firmware owns the whole chip: the flash loader erases it, so there is
   no bootloader to leave room for and the image starts at 0x08000000.

   Against the DMC-100 this board has twice the RAM and two and a half times
   the flash. The frame region keeps the sizes the converted packs were
   planned for, because the plan of every map is baked into the data; the
   extra RAM goes to the level arena, which the firmware sizes at run time
   from __ram_end. More arena is never wrong - it is more object pages than
   the plan asked for. */

MEMORY
{
    FLASH    (rx)  : ORIGIN = 0x08000000, LENGTH = 0x3F800
    SETTINGS (r)   : ORIGIN = 0x0803F800, LENGTH = 0x400
    RAM      (rwx) : ORIGIN = 0x20000000, LENGTH = 0x10000
}

/* The DMC-100 squeezes its stack to 1,536 B. There is no reason to here, and
   the renderer's BSP walk is the deepest thing in the engine. */
STACK_SIZE = DEFINED(STACK_SIZE_OVERRIDE) ? STACK_SIZE_OVERRIDE : 0x1000;
CORE_SIZE  = DEFINED(CORE_SIZE_OVERRIDE) ? CORE_SIZE_OVERRIDE : 0x2800;

FRAME_BASE = ORIGIN(RAM) + STACK_SIZE + CORE_SIZE;
RAM_END    = ORIGIN(RAM) + LENGTH(RAM);

/* Unchanged: the converter planned the packs for these. */
FRAME_MIN = 8368;
FRAME_MAX = 15208;
/* The record cache is eight times the size it has on the DMC-100
   (RC_SETS=256 instead of 32, mk/py32.mk). There every byte of cache costs a
   byte of arena out of 32 KB of RAM; here there are 64 KB, and every miss
   avoided saves a serial NOR access on a bus that also has to serve the
   panel. */
CACHE_MIN = 18688;
CACHE_MAX = 22784;
ARENA_MIN = 3848;

SETTINGS_PAGE = ORIGIN(SETTINGS);
