/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright (C) 2026 Artur Bieniek */

/* Link script of the DOOM firmware.

   RAM, 32 KB:
     0x20000000  stack, STACK_SIZE bytes (painted at reset): 1,536 B (mk/fw.mk), initial SP
                 0x20000600
     next        fixed core: .data and .bss, at most CORE_SIZE bytes, 7,264 B with the stack
     FRAME_BASE  frame region of the map's plan (strip buffers, pass-1 lists, pools, strip
                 arrays), FRAME_MIN (plan D) to FRAME_MAX (plan A) bytes. The tic scratch
                 dmc_tic is fixed at FRAME_BASE + 6,912 (1,456 B), above the strip buffers of
                 every plan, so the pump may still read them while tics run. Overlays run
                 from FRAME_BASE.
     run time    the caches (record cache, then the flat slots) where R_FrameSetup() says the
                 plan's frame region ends (port/game/i_dmc.c), then the level arena of the
                 plan, which ends at __ram_end (port/obj/level.c).
     USB mode    the USB overlay at FRAME_BASE, behind its window (FRAME_MIN) its block buffer
                 and state (usb_ram, port/usb/usb.h). The state of the controller driver
                 (usbd_ram, platform/usbd.h) is in the fixed core, the game pad uses it in
                 play mode. It ends in a reset.
   Flash: the app slot 0x08007000-0x0801FBFF, the settings page at 0x0801FC00. */

INCLUDE regions.ld

ENTRY(Reset_Handler)

/* Sizes this firmware is linked for */
TIC_OFFSET = 6912;      /* dmc_tic from FRAME_BASE */
TIC_SIZE   = 1456;      /* sizeof(ticscratch_t) at most (engine/p_local.h) */
STRIP_MAX  = 6400;      /* strip buffers of plan A, 2 x 16 x 200 B */
RC_SIZE    = 2336;      /* record cache, RC_BYTES of port/data/cache.h */
FLAT_SLOT  = 1024;      /* one 32x32 flat */
USB_RAM    = 4608;      /* USB_RAM_MAX of port/usb/usb.h */

SECTIONS
{
    .stack (NOLOAD) :
    {
        __stack_bottom = .;
        . += STACK_SIZE;
        __stack_top = .;
    } > RAM

    .text :
    {
        KEEP(*(.vectors))
        *(.text .text.*)
        *(.rodata .rodata.*)
        . = ALIGN(4);
    } > FLASH

    /* the fixed core sorted by alignment, so it loses no bytes to padding */
    .data :
    {
        __data_start = .;
        *(SORT_BY_ALIGNMENT(.data .data.*))
        . = ALIGN(4);
        __data_end = .;
    } > RAM AT > FLASH
    __data_load = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        __bss_start = .;
        *(SORT_BY_ALIGNMENT(.bss .bss.*))
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > RAM
    __core_end = .;

    /* Nothing of the firmware is placed in the frame region at link time */
    .dynram FRAME_BASE (NOLOAD) :
    {
        __frame_start = .;
        __dyn_start = .;
        *(.dynram .dynram.*)
        . = ALIGN(4);
        __dyn_free = .;
    } > RAM
    __ram_end = RAM_END;

    __image_end = LOADADDR(.data) + SIZEOF(.data);
    __settings_page = SETTINGS_PAGE;
    __frame_min = FRAME_MIN;
    __frame_max = FRAME_MAX;
    __core_size = __core_end - __data_start;
    dmc_tic = FRAME_BASE + TIC_OFFSET;
    usb_ram = FRAME_BASE + FRAME_MIN;

    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) *(.ARM.attributes) }
}

ASSERT(__stack_bottom == ORIGIN(RAM), "the stack must start at 0x20000000")
ASSERT(__stack_top == ORIGIN(RAM) + STACK_SIZE && STACK_SIZE >= 0x600,
       "the stack must keep at least 1,536 B")
ASSERT(FRAME_BASE == ORIGIN(RAM) + 7264, "stack and fixed core must take 7,264 B")
ASSERT(__data_start == __stack_top, "the fixed core must follow the stack")
ASSERT(__core_end <= FRAME_BASE, "fixed core (.data + .bss) over CORE_SIZE, see make fw-size")
ASSERT(FRAME_BASE % 16 == 0, "FRAME_BASE must be 16 B aligned")
ASSERT(__dyn_free == FRAME_BASE, "no static data may sit in the frame region")
ASSERT(TIC_OFFSET >= STRIP_MAX, "the tic scratch must not overlap the strip buffers")
ASSERT(TIC_OFFSET + TIC_SIZE <= FRAME_MIN, "the tic scratch must end in the smallest frame region")
ASSERT(dmc_tic % 4 == 0, "the tic scratch must be word aligned")
ASSERT(CACHE_MIN == RC_SIZE && CACHE_MAX >= RC_SIZE + 4 * FLAT_SLOT,
       "the cache budgets of regions.ld do not match the record cache and four flat slots")
ASSERT(FRAME_BASE + FRAME_MAX + CACHE_MAX + ARENA_MIN <= RAM_END,
       "the largest plan (frame A, all caches, smallest arena) does not fit")
ASSERT(FRAME_BASE + FRAME_MIN + CACHE_MIN + ARENA_MIN <= RAM_END, "the smallest plan does not fit")
ASSERT(RAM_END == ORIGIN(RAM) + 0x8000, "the level arena must end at the end of SRAM")
ASSERT(usb_ram + USB_RAM <= RAM_END, "the RAM of the USB overlay does not fit")
ASSERT(__image_end <= ORIGIN(SETTINGS), "the image runs into the settings page")
ASSERT(SETTINGS_PAGE == 0x0801FC00, "the settings page must be at 0x0801FC00")
