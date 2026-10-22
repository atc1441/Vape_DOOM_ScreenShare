// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
// Copyright (C) 2026 VapeDoom port
//
// Vector table and reset for the Aspire PIXO (PY32F403xC). The image starts at
// 0x08000000 and owns the whole chip: the flash loader erases it, so no
// bootloader is left to jump through.
//
// The first thing the reset path does is pull PB13 low. That pin is the load
// path of the vape and low is the state the stock firmware holds. It happens
// before the clock tree is touched and before .data is copied, so no fault on
// the way can leave the coil driven, and the fault handler repeats it.

#include <stdint.h>

#include "platform.h"
#include "py32_hw.h"

#define STACK_PAINT 0x5AC3A55Cu  // also written literally in Reset_Handler

extern uint32_t __stack_bottom[], __stack_top[];
extern uint32_t __data_start[], __data_end[], __data_load[], __bss_start[], __bss_end[];
extern uint32_t __dyn_start[];

void Reset_Handler(void);
void Fault_Handler(void);
void SysTick_Handler(void);

__attribute__((naked, noreturn)) void Reset_Handler(void)
{
    __asm volatile(
        "cpsid i\n"
        "ldr r0, =0x40021034\n"  // RCC_AHB2ENR: clocks for GPIOA..GPIOE
        "ldr r1, [r0]\n"
        "orr r1, r1, #0x7C\n"
        "str r1, [r0]\n"
        "ldr r0, =0x48000428\n"  // GPIOB_BRR
        "mov r1, #0x2000\n"      // PB13 low: load path off
        "str r1, [r0]\n"
        "ldr r0, =__stack_top\n"
        "mov sp, r0\n"
        "ldr r1, =__stack_bottom\n"
        "ldr r2, =0x5AC3A55C\n"
        "1: str r2, [r1], #4\n"
        "cmp r1, r0\n"
        "blo 1b\n"
        "b reset_c\n");
}

__attribute__((used, noreturn)) void reset_c(void)
{
    SYST_CSR = 0;
    for (unsigned i = 0; i < 4; i++) {
        REG32(0xE000E180u + 4u * i) = 0xFFFFFFFFu;  // NVIC ICER
        REG32(0xE000E280u + 4u * i) = 0xFFFFFFFFu;  // NVIC ICPR
    }
    SCB_VTOR = 0x08000000u;
    __asm volatile("dsb\nisb");
    for (uint32_t *s = __data_load, *d = __data_start; d < __data_end;)
        *d++ = *s++;
    for (uint32_t *p = __bss_start; p < __bss_end; p++)
        *p = 0;
    for (uint32_t *p = __dyn_start; p < (uint32_t *)(void *)__dyn_free; p++)
        *p = 0;
    __asm volatile("cpsie i");
    app_main();
    plat_power_off();
}

uint32_t plat_stack_used(void)
{
    const uint32_t *p = __stack_bottom;
    while (p < __stack_top && *p == STACK_PAINT)
        p++;
    return (uint32_t)((uintptr_t)__stack_top - (uintptr_t)p);
}

// 16 core vectors and 100 interrupts. Only SysTick is used; every other entry
// goes to the fault screen instead of spinning silently.
#define VEC_COUNT 116

__attribute__((section(".vectors"), used)) const uintptr_t vectors[VEC_COUNT] = {
    [0] = (uintptr_t)__stack_top,
    [1] = (uintptr_t)Reset_Handler,
    [2 ... 14] = (uintptr_t)Fault_Handler,
    [15] = (uintptr_t)SysTick_Handler,
    [16 ... VEC_COUNT - 1] = (uintptr_t)Fault_Handler,
};
