/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright (C) 2026 Artur Bieniek */

/* Stand-in for the meter's bootloader in Renode, placed at 0x08000000. It starts an app the
   way the real one does: MSP from the app's vector table, a pop of 16 bytes from that new
   stack, then a jump to the app's reset vector. */

    .syntax unified
    .cpu cortex-m4
    .thumb

    .section .text
    .global _start
_start:
    .word 0x20007F00
    .word stub_reset  /* .thumb_func sets the Thumb bit */

    .thumb_func
stub_reset:
    ldr r0, =0x08007000
    ldr r1, [r0]
    ldr r2, [r0, #4]
    msr msp, r1
    pop {r4, r5, r6, lr}
    bx r2
    .ltorg
