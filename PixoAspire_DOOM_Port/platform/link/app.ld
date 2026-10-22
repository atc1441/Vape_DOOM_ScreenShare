/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright (C) 2026 Artur Bieniek */

/* Link script of the DMC-100 app: engine and test apps.
   RAM: stack | fixed core | FRAME_BASE: frame region, caches, level arena (split at run
   time by the map's plan). Statically placed .dynram input (test apps) starts at
   FRAME_BASE and ends at __dyn_free. */

INCLUDE regions.ld

ENTRY(Reset_Handler)

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

    .data :
    {
        __data_start = .;
        *(.data .data.*)
        . = ALIGN(4);
        __data_end = .;
    } > RAM AT > FLASH
    __data_load = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > RAM
    __core_end = .;

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

    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) *(.ARM.attributes) }
}

ASSERT(__stack_bottom == ORIGIN(RAM), "stack must start at 0x20000000")
ASSERT(__stack_top == ORIGIN(RAM) + 0x800, "initial SP must be 0x20000800")
ASSERT(__core_end <= FRAME_BASE, "fixed core (.data + .bss) is over its budget")
ASSERT(FRAME_BASE % 16 == 0, "FRAME_BASE must be 16 B aligned")
ASSERT(__dyn_free <= RAM_END, "static .dynram data does not fit in SRAM")
ASSERT(DEFINED(CORE_SIZE_OVERRIDE) || FRAME_BASE + FRAME_MAX + CACHE_MAX + ARENA_MIN <= RAM_END,
       "largest plan (frame A, all caches, smallest arena) does not fit")
ASSERT(DEFINED(CORE_SIZE_OVERRIDE) || FRAME_BASE + FRAME_MIN + CACHE_MIN + ARENA_MIN <= RAM_END,
       "smallest plan does not fit")
ASSERT(__image_end <= ORIGIN(SETTINGS), "image runs into the settings page")
