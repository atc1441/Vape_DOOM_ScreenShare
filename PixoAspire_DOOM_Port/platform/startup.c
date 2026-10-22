// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Vector table and reset for the DMC-100 app slot (0x08007000).
//
// The bootloader loads MSP from the vector table, pops 16 bytes from that stack and jumps to
// Reset_Handler. The initial SP is the top of the stack at the bottom of SRAM (0x20000800,
// 0x20000600 in the DOOM firmware), so the popped bytes are ordinary RAM above the stack. Reset_Handler sets SP again and paints
// the whole stack for the high-water mark.

#include <stdint.h>

#include "at32.h"
#include "platform.h"

#define STACK_PAINT 0x5AC3A55Cu  // also written literally in Reset_Handler

extern uint32_t __stack_bottom[], __stack_top[];
extern uint32_t __data_start[], __data_end[], __data_load[], __bss_start[], __bss_end[];
extern uint32_t __dyn_start[];

void Reset_Handler(void);
void Fault_Handler(void);
void SysTick_Handler(void);
void DMA1_Channel2_IRQHandler(void);
void DMA1_Channel5_IRQHandler(void);
void OTGFS1_IRQHandler(void);

__attribute__((naked, noreturn)) void Reset_Handler(void)
{
    __asm volatile("cpsid i\n"
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
    for (unsigned i = 0; i < 3; i++) {
        REG32(0xE000E180u + 4u * i) = 0xFFFFFFFFu;  // NVIC ICER
        REG32(0xE000E280u + 4u * i) = 0xFFFFFFFFu;  // NVIC ICPR
    }
    SCB_VTOR = 0x08007000u;
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

// The game pad of the DOOM firmware (controller/rc-n1/dmc_dji.c) has the OTGFS interrupt, the
// other apps never enable it
__attribute__((weak, naked)) void OTGFS1_IRQHandler(void)
{
    __asm volatile("b Fault_Handler");
}

uint32_t plat_stack_used(void)
{
    const uint32_t *p = __stack_bottom;
    while (p < __stack_top && *p == STACK_PAINT)
        p++;
    return (uint32_t)((uintptr_t)__stack_top - (uintptr_t)p);
}

#define VEC_COUNT 92  // 16 core vectors and 76 interrupts

__attribute__((section(".vectors"), used)) const uintptr_t vectors[VEC_COUNT] = {
    [0] = (uintptr_t)__stack_top,
    [1] = (uintptr_t)Reset_Handler,
    [2 ... 14] = (uintptr_t)Fault_Handler,
    [15] = (uintptr_t)SysTick_Handler,
    [16 ... 16 + IRQ_DMA1_CH2 - 1] = (uintptr_t)Fault_Handler,
    [16 + IRQ_DMA1_CH2] = (uintptr_t)DMA1_Channel2_IRQHandler,
    [16 + IRQ_DMA1_CH2 + 1 ... 16 + IRQ_DMA1_CH5 - 1] = (uintptr_t)Fault_Handler,
    [16 + IRQ_DMA1_CH5] = (uintptr_t)DMA1_Channel5_IRQHandler,
    [16 + IRQ_DMA1_CH5 + 1 ... 16 + IRQ_OTGFS - 1] = (uintptr_t)Fault_Handler,
    [16 + IRQ_OTGFS] = (uintptr_t)OTGFS1_IRQHandler,
    [16 + IRQ_OTGFS + 1 ... VEC_COUNT - 1] = (uintptr_t)Fault_Handler,
};
