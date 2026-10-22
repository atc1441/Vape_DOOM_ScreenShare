// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Fault screen for CPU faults and plat_panic (I_Error). Both reset MSP to the top of the
// stack first, so an overflowed or corrupted stack still shows the screen. Everything here is
// polled: the screen runs with interrupts masked (or in the HardFault handler).

#include "at32.h"
#include "lcd.h"
#include "platform.h"

extern uint32_t __stack_top[];

// What the screen shows, at FRAME_BASE: the frame region (overlays, a test app's buffers) is
// not used again once a fault or plat_panic starts, so this costs the fixed core nothing.
// plat_panic copies its message there first: it must not come from those 80 bytes.
struct fault_info {
    uint32_t exc, pc, lr, excret;
    char msg[64];
};
#define info (*(struct fault_info *)(void *)__frame_start)
static void (*hook)(void);
static int line_y, in_fault;

#define BG 0x9000u  // rgb565(150, 0, 0)

void plat_fault_hook(void (*fn)(void)) { hook = fn; }

void plat_fault_get(struct plat_fault *f)
{
    f->exc = info.exc;
    f->pc = info.exc ? info.pc : 0;
    f->lr = info.lr;
    f->cfsr = SCB_CFSR;
    f->hfsr = SCB_HFSR;
    f->bfar = SCB_BFAR;
    f->mmfar = SCB_MMFAR;
    f->msg = info.exc ? "" : info.msg;
}

static int power_raw(void) { return !(GPIO_IDT(GPIOF) & (1u << 7)); }

static void hex8(char *o, uint32_t v)
{
    for (int i = 7; i >= 0; i--, v >>= 4)
        o[i] = "0123456789ABCDEF"[v & 15u];
    o[8] = 0;
}

static void dec(char *o, uint32_t v)
{
    char r[10];
    int n = 0;
    do {
        r[n++] = (char)('0' + v % 10u);
        v /= 10u;
    } while (v);
    while (n)
        *o++ = r[--n];
    *o = 0;
}

void plat_fault_line(const char *label, uint32_t value)
{
    char s[12];
    if (line_y > LCD_H - 24)
        return;
    dec(s, value);
    lcd_text(8, line_y, label, 0xFFFF, BG, 1);
    lcd_text(8 + 8 * 16, line_y, s, 0xFFFF, BG, 1);
    line_y += 10;
}

static void kv(int y, const char *k, uint32_t v, int scale)
{
    char h[9];
    hex8(h, v);
    int x = lcd_text(8, y, k, 0xFFFF, BG, scale);
    lcd_text(x + 8 * scale, y, h, 0xFFFF, BG, scale);
}

__attribute__((used, noreturn)) void fault_screen(void)
{
    if (in_fault++) {  // fault while drawing: wait for POWER only
        while (!power_raw()) {
        }
        power_cut();
    }
    if (lcd_ready) {
        lcd_recover();
    } else {  // before plat_init: clock GPIOA/B/F and SPI2, then a full panel init
        CRM_APB2EN |= (1u << 2) | (1u << 3) | (1u << 7);
        CRM_APB1EN |= 1u << 14;
        lcd_init(LCD_INIT_DEFAULT);
    }
    lcd_backlight(1);
    snd_tone(2000);
    lcd_fill(0, 0, LCD_W - 1, LCD_H - 1, BG);
    snd_tone(0);
    lcd_text(8, 8, info.exc ? "CPU FAULT" : "ERROR", 0xFFFF, BG, 2);
    if (info.exc) {
        char s[12];
        dec(s, info.exc);
        lcd_text(lcd_text(168, 12, "exception ", 0xFFFF, BG, 1), 12, s, 0xFFFF, BG, 1);
        kv(36, "PC", info.pc, 2);
        kv(58, "LR", info.lr, 2);
    } else {
        char l[40];
        unsigned n = 0;
        for (; n < 39u && info.msg[n]; n++)
            l[n] = info.msg[n];
        l[n] = 0;
        lcd_text(8, 36, l, 0xFFFF, BG, 1);
        lcd_text(8, 46, info.msg + n, 0xFFFF, BG, 1);
        kv(58, "LR", info.lr, 2);
    }
    kv(84, "CFSR ", SCB_CFSR, 1);
    kv(94, "HFSR ", SCB_HFSR, 1);
    kv(104, "BFAR ", SCB_BFAR, 1);
    kv(114, "MMFAR", SCB_MMFAR, 1);
    line_y = 128;
    plat_fault_line("stack used", plat_stack_used());
    plat_fault_line("stack size", (uint32_t)((uintptr_t)__stack_top - 0x20000000u));
    if (hook)
        hook();
    lcd_text(8, LCD_H - 12, "POWER: off", 0xFFE0, BG, 1);
    while (power_raw()) {
    }
    for (uint32_t held = 0; held < 40u;) {
        spin_cycles(144000u);
        held = power_raw() ? held + 1u : 0u;
    }
    plat_power_off();
}

// Called from Fault_Handler on the reset stack. pc and lr are 0xFFFFFFFF when the exception
// frame was outside SRAM.
__attribute__((used, noreturn)) void fault_c(uint32_t exc, uint32_t pc, uint32_t lr, uint32_t excret)
{
    info.exc = exc;
    info.pc = pc;
    info.lr = lr;
    info.excret = excret;
    fault_screen();
}

__attribute__((naked, noreturn)) void Fault_Handler(void)
{
    __asm volatile("tst lr, #4\n"
                   "ite eq\n"
                   "mrseq r0, msp\n"
                   "mrsne r0, psp\n"
                   "mov r3, lr\n"
                   "movs r1, #0\n"
                   "subs r1, #1\n"
                   "mov r2, r1\n"
                   "ldr r12, =0x20000000\n"
                   "subs r12, r0, r12\n"
                   "cmp r12, #0x7F00\n"
                   "bhs 1f\n"
                   "ldr r1, [r0, #24]\n"
                   "ldr r2, [r0, #20]\n"
                   "1: ldr r0, =__stack_top\n"
                   "mov sp, r0\n"
                   "mrs r0, ipsr\n"
                   "b fault_c\n");
}

__attribute__((naked, noreturn)) static void panic_reset(void)
{
    __asm volatile("ldr r0, =__stack_top\n"
                   "mov sp, r0\n"
                   "b fault_screen\n");
}

void plat_panic(const char *msg)
{
    __asm volatile("cpsid i");
    info.exc = 0;
    info.lr = (uint32_t)(uintptr_t)__builtin_return_address(0);
    unsigned i = 0;
    for (; msg && msg[i] && i < sizeof(info.msg) - 1u; i++)
        info.msg[i] = msg[i];
    info.msg[i] = 0;
    panic_reset();
}
