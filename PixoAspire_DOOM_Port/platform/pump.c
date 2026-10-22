// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// LCD pump: indexed columns to RGB565 through pump_lut, sent by DMA1 ch5 to SPI2 in 16-bit
// frames from two half-column buffers. Buffer 0 always holds top halves and buffer 1 bottom
// halves. Each transfer-complete interrupt arms the other buffer, then refills the one just
// sent with the next half (in low detail only after its second send). When the next buffer
// is empty the interrupt stops and pump_submit() restarts the DMA. The file is built with
// -O2 for the interrupt and fill(), everything else is marked minsize.
//
// The LCD_POLLED=1 build converts and sends in pump_submit() with the CPU.

#include "at32.h"
#include "lcd.h"
#include "platform.h"

#ifndef LCD_POLLED
#define LCD_POLLED 0
#endif

#define QLEN 4
#ifndef PUMP_WAIT_MS
#define PUMP_WAIT_MS 100  // no pump interrupt for this long while waiting: the DMA is stuck
#endif

uint16_t pump_lut[256];
volatile struct pump_stats pump_stats;

#define COLD __attribute__((minsize))

static uint8_t is_open, reps;
static uint16_t half;

COLD static void check_window(int x0, int x1, int y0, int y1)
{
    int h = y1 - y0 + 1;
    if (h <= 0 || h > 200 || h % 4 || x0 < 0 || x1 < x0 || x1 >= LCD_W || y0 < 0 || y1 >= LCD_H)
        plat_panic("pump: bad window");
}

#if LCD_POLLED

COLD void pump_begin(int x0, int x1, int y0, int y1, int flags)
{
    check_window(x0, x1, y0, y1);
    pump_close();
    half = (uint16_t)((y1 - y0 + 1) / 2);
    reps = (flags & PUMP_DOUBLE) ? 2 : 1;
    lcd_window(x0, y0, x1, y1);
    is_open = 1;
}

COLD void pump_submit(const uint8_t *cols, int ncols)
{
    uint32_t h = 2u * half;
    for (; ncols > 0; ncols--, cols += h) {
        for (unsigned r = 0; r < reps; r++) {
            for (uint32_t i = 0; i < h; i++)
                lcd_push(pump_lut[cols[i]]);
        }
    }
}

COLD void pump_sync(int max) { (void)max; }
COLD int pump_pending(void) { return 0; }

COLD void pump_end(void)
{
    if (is_open)
        lcd_data_end();
    is_open = 0;
}

COLD void pump_reset(void) { is_open = 0; }

void DMA1_Channel5_IRQHandler(void) { DMA_CLR = DMA_FLAGS(5); }

#else

#define CH5_CFG (DMA_DTD | DMA_MINCM | DMA_P16 | DMA_M16 | DMA_CHPL(1) | DMA_FDTIEN)

static uint32_t hbuf[2][50];  // two RGB565 pixels per word
static volatile uint8_t ready[2];  // sends left for each half buffer
static volatile uint8_t cur, next, running, primed;
static struct {
    const uint8_t *p;
    uint32_t n;
} q[QLEN];
static volatile uint8_t qhead, qcount;
static const uint8_t *src;
static uint32_t src_left;

// Converts the next half column into buffer k. Interrupts masked or in the handler.
// Returns 1 when it converted one.
__attribute__((noinline)) static int fill(unsigned k)
{
    if (!qcount)
        return 0;
    const uint8_t *s = src;
    const uint16_t *lut = pump_lut;
    uint32_t *d = hbuf[k];
#pragma clang loop unroll_count(2)
    for (unsigned i = half / 2u; i; i--, s += 2)
        *d++ = lut[s[0]] | ((uint32_t)lut[s[1]] << 16);
    src = s;
    ready[k] = reps;
    src_left -= half;
    if (!src_left) {
        unsigned h = (qhead + 1u) % QLEN;
        qhead = (uint8_t)h;
        if (--qcount) {
            src = q[h].p;
            src_left = q[h].n;
        }
    }
    return 1;
}

static void arm(unsigned k)
{
    DMA_CCTRL(5) = CH5_CFG;
    DMA_CMADDR(5) = (uint32_t)(uintptr_t)hbuf[k];
    DMA_CDTCNT(5) = half;
    DMA_CCTRL(5) = CH5_CFG | DMA_CHEN;
    cur = (uint8_t)k;
}

void DMA1_Channel5_IRQHandler(void)
{
    uint32_t t0 = DWT_CYCCNT;
    DMA_CLR = DMA_FLAGS(5);
    unsigned k = cur, n = k ^ 1u, f = 0;
    uint8_t left = (uint8_t)(ready[k] - 1u);
    ready[k] = left;
    if (!ready[n])
        f = fill(n);  // a strip may have arrived after n's refill found nothing
    if (ready[n]) {
        arm(n);
        if (!left)
            f += fill(k);
    } else {
        DMA_CCTRL(5) = CH5_CFG;
        running = 0;
        next = (uint8_t)n;
    }
    uint32_t dt = DWT_CYCCNT - t0;
    pump_stats.irqs++;
    pump_stats.fills += f;
    pump_stats.cycles += dt;
    if (dt > pump_stats.max)
        pump_stats.max = dt;
    irq_end();
}

// Interrupts masked
COLD static void restart(void)
{
    unsigned k = next;
    if (!ready[k])
        fill(k);
    if (!ready[k])
        return;
    if (!ready[k ^ 1u])
        fill(k ^ 1u);
    if (primed)
        pump_stats.restarts++;
    primed = 1;
    running = 1;
    arm(k);
}

// Waits until at most max strips are pending, with max < 0 until the DMA has stopped too.
// Panics when no pump interrupt came for PUMP_WAIT_MS.
COLD static void wait_pending(int max)
{
    uint32_t seen = pump_stats.irqs, t0 = plat_ms();
    while (qcount > (max < 0 ? 0 : max) || (max < 0 && running)) {
        uint32_t now = plat_ms(), n = pump_stats.irqs;
        if (n != seen) {
            seen = n;
            t0 = now;
        } else if (now - t0 > PUMP_WAIT_MS) {
            plat_panic("pump: DMA stuck");
        }
    }
}

COLD void pump_begin(int x0, int x1, int y0, int y1, int flags)
{
    check_window(x0, x1, y0, y1);
    pump_close();
    half = (uint16_t)((y1 - y0 + 1) / 2);
    reps = (flags & PUMP_DOUBLE) ? 2 : 1;
    lcd_window(x0, y0, x1, y1);
    DMA_CCTRL(5) = 0;
    DMA_CLR = DMA_FLAGS(5);
    DMA_CPADDR(5) = SPI_DT_ADDR(SPI2);
    NVIC_IPR(IRQ_DMA1_CH5) = IRQ_PRIO;
    NVIC_ISER0 = 1u << IRQ_DMA1_CH5;
    ready[0] = ready[1] = 0;
    next = 0;
    running = 0;
    primed = 0;
    qhead = 0;
    qcount = 0;
    SPI_CTRL2(SPI2) = SPI_DMATEN;
    is_open = 1;
}

COLD void pump_submit(const uint8_t *cols, int ncols)
{
    if (!is_open || ncols <= 0)
        return;
    wait_pending(QLEN - 1);
    uint32_t m = irq_save();
    unsigned i = (qhead + qcount) % QLEN;
    q[i].p = cols;
    q[i].n = (uint32_t)ncols * 2u * half;
    if (!qcount) {
        src = cols;
        src_left = q[i].n;
    }
    qcount++;
    if (!running)
        restart();
    irq_restore(m);
}

COLD void pump_sync(int max) { wait_pending(max < 0 ? 0 : max); }

COLD int pump_pending(void) { return qcount; }

COLD void pump_end(void)
{
    if (!is_open)
        return;
    wait_pending(-1);
    lcd_data_end();
    SPI_CTRL2(SPI2) = 0;
    DMA_CCTRL(5) = 0;
    is_open = 0;
}

COLD void pump_reset(void)
{
    is_open = 0;
    running = 0;
    qcount = 0;
}

#endif

COLD void pump_close(void) { pump_end(); }
