// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
// Copyright (C) 2026 VapeDoom port
//
// Fault screen and panic.
//
// Two things happen before anything else: the load path goes low and the
// watchdogs keep being fed. A vape that faults with its coil driven is a
// different kind of problem from a vape that faults with a message on the
// screen, and a fault handler that stops feeding the external watchdog chip
// turns every bug into a reset loop with nothing to read.
//
// Everything is also written to the debug ring, so a fault that happens
// before the panel works is still visible over SWD.

#include "lcd.h"
#include "nor.h"
#include "platform.h"
#include "py32_hw.h"

static struct plat_fault cur;
static void (*hook)(void);
static int line_y;

void plat_fault_get(struct plat_fault *f) { *f = cur; }
void plat_fault_hook(void (*fn)(void)) { hook = fn; }

static void show(const char *label, uint32_t v, int hex)
{
    char b[48];
    if (hex)
        plat_fmt(b, sizeof b, "%-10s %08x", label, (unsigned)v);
    else
        plat_fmt(b, sizeof b, "%-10s %u", label, (unsigned)v);
    if (line_y < LCD_H - 8)
        lcd_text(2, line_y, b, 0xFFFFu, 0, 1);
    line_y += 9;
    dbg_str(b);
    dbg_nl();
}

void plat_fault_line(const char *label, uint32_t value) { show(label, value, 0); }

__attribute__((noreturn)) static void fault_screen(void)
{
    pin_low(GPIOB, PIN_COIL);
    nor_recover();
    lcd_recover();
    line_y = 2;
    lcd_fill(0, 0, LCD_W - 1, LCD_H - 1, 0);
    lcd_backlight(1);

    dbg_nl();
    dbg_str("!! FAULT exc=");
    dbg_dec(cur.exc);
    dbg_nl();

    if (cur.msg && cur.msg[0]) {
        lcd_text(2, line_y, cur.msg, 0xF800u, 0, 1);
        line_y += 10;
        dbg_str(cur.msg);
        dbg_nl();
    } else {
        lcd_text(2, line_y, "CPU FAULT", 0xF800u, 0, 1);
        line_y += 10;
    }
    show("exc", cur.exc, 0);
    show("pc", cur.pc, 1);
    show("lr", cur.lr, 1);
    show("cfsr", cur.cfsr, 1);
    show("hfsr", cur.hfsr, 1);
    show("bfar", cur.bfar, 1);
    show("stack", plat_stack_used(), 0);
    if (hook)
        hook();

    // Stay alive and readable. The watchdogs are fed here by hand because
    // the SysTick interrupt may be exactly what died.
    for (;;) {
        wdt_kick();
        spin_cycles(144000u);
    }
}

__attribute__((noreturn)) void plat_panic(const char *msg)
{
    __asm volatile("cpsid i");
    pin_low(GPIOB, PIN_COIL);
    cur.exc = 0;
    cur.pc = 0;
    cur.lr = 0;
    cur.cfsr = SCB_CFSR;
    cur.hfsr = SCB_HFSR;
    cur.bfar = SCB_BFAR;
    cur.mmfar = SCB_MMFAR;
    cur.msg = msg;
    fault_screen();
}

__attribute__((used, noreturn)) void fault_c(const uint32_t *sp, uint32_t exc);

__attribute__((used, noreturn)) void fault_c(const uint32_t *sp, uint32_t exc)
{
    pin_low(GPIOB, PIN_COIL);
    cur.exc = exc & 0x1FFu;
    // The stacked frame is only meaningful when it is in SRAM.
    if ((uintptr_t)sp >= 0x20000000u && (uintptr_t)sp < 0x20010000u) {
        cur.pc = sp[6];
        cur.lr = sp[5];
    } else {
        cur.pc = 0xFFFFFFFFu;
        cur.lr = 0xFFFFFFFFu;
    }
    cur.cfsr = SCB_CFSR;
    cur.hfsr = SCB_HFSR;
    cur.bfar = SCB_BFAR;
    cur.mmfar = SCB_MMFAR;
    cur.msg = "";
    fault_screen();
}

__attribute__((naked)) void Fault_Handler(void)
{
    __asm volatile(
        "cpsid i\n"
        // Load path off immediately, before any C runs.
        "ldr r2, =0x48000428\n"  // GPIOB_BRR
        "mov r3, #0x2000\n"
        "str r3, [r2]\n"
        "tst lr, #4\n"
        "ite eq\n"
        "mrseq r0, msp\n"
        "mrsne r0, psp\n"
        "mrs r1, ipsr\n"
        "b fault_c\n");
}
