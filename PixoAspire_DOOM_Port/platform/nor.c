// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// W25Q32JV on SPI1 (mode 3, 36 MHz, 72 MHz with NOR_SPI_SLOW=0), CS on PA4. See nor.h for
// the API and the queue rules.
//
// DMA read transaction: CS low, the FAST READ header written polled without waiting for
// each received byte, wait until idle, read DT and STS to clear the overrun, disable SPI
// while arming (AT32 erratum: a receive DMA request raised during the header is only
// cleared by disabling SPI), arm ch2 (RX) and ch3 (TX of a constant 0xFF), enable SPI, RX
// DMA, then TX DMA. Completion by polling (sync) or the ch2 transfer-complete interrupt.

#include <stddef.h>

#include "at32.h"
#include "nor.h"
#include "platform.h"

#ifndef NOR_POLLED
#define NOR_POLLED 0
#endif
#ifndef NOR_PROFILE
#define NOR_PROFILE 0  // 1: nor_stats.prof[] times the phases of sync DMA reads
#endif

#define SPI1_CFG (SPI_SWCSEN | SPI_SWCSIL | SPI_MSTEN | SPI_CPOL | SPI_CPHA | (NOR_SPI_SLOW ? SPI_MDIV4 : 0u))
#define CHUNK 256u
#define QLEN 4
#define WAIT_N 100000u
#ifndef NOR_WAIT_MS
#define NOR_WAIT_MS 100  // no async chunk started for this long while waiting: the DMA is stuck
#endif

volatile struct nor_stats nor_stats;
static uint32_t jedec;
static int next_ticket = 1;

#ifdef NOR_MODEL
// Host model test: chip select and data register go to the emulated chip
void cs_lo(void);
void cs_hi(void);
void spi_tx(uint8_t b);
uint8_t spi_rx(void);
#else
static inline void cs_lo(void) { GPIO_CLR(GPIOA) = 1u << PIN_NOR_CS; }

// Raises CS and keeps it high for at least 50 ns (the W25Q32 minimum before a write
// command), so every command after this one sees a clean deselect.
static inline void cs_hi(void)
{
    GPIO_SCR(GPIOA) = 1u << PIN_NOR_CS;
    __asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
}

static inline void spi_tx(uint8_t b) { SPI_DT(SPI1) = b; }
static inline uint8_t spi_rx(void) { return (uint8_t)SPI_DT(SPI1); }
#endif

static int wait_sts(uint32_t mask, uint32_t want)
{
    for (uint32_t n = WAIT_N; n; n--) {
        if ((SPI_STS(SPI1) & mask) == want)
            return 0;
    }
    nor_stats.errors++;
    return -1;
}

static uint8_t xfer(uint8_t b)
{
    if (wait_sts(SPI_TDBE, SPI_TDBE))
        return 0xFF;
    spi_tx(b);
    if (wait_sts(SPI_RDBF, SPI_RDBF))
        return 0xFF;
    return spi_rx();
}

static int new_ticket(void)
{
    int t = next_ticket;
    next_ticket = t >= 0x3FFFFFFF ? 1 : t + 1;
    return t;
}

uint32_t nor_jedec(void) { return jedec; }

#if NOR_POLLED

static void claim(void) {}
static void release(void) {}

// The transfer of nor_read and nor_read_async, which count their calls apart
static void read_polled(uint32_t a, uint8_t *d, uint32_t n)
{
    cs_lo();
    xfer(0x0B);
    xfer((uint8_t)(a >> 16));
    xfer((uint8_t)(a >> 8));
    xfer((uint8_t)a);
    xfer(0xFF);
    while (n--)
        *d++ = xfer(0xFF);
    cs_hi();
}

void nor_read(uint32_t a, void *dst, uint32_t n)
{
    nor_stats.sync++;
    if (!NOR_PROFILE)
        nor_stats.bytes += n;
    read_polled(a, dst, n);
}

int nor_read_async(uint32_t a, void *dst, uint32_t n, int prio)
{
    (void)prio;
    if (!NOR_PROFILE) {
        nor_stats.async++;
        nor_stats.bytes += n;
    }
    read_polled(a, dst, n);
    return new_ticket();
}

int nor_col_start(uint32_t a, void *dst, uint32_t n)
{
    if (n - 1u >= CHUNK)
        return -1;
    if (!NOR_PROFILE) {
        nor_stats.async++;
        nor_stats.bytes += n;
    }
    read_polled(a, dst, n);
    return 0;
}

void nor_col_wait(void) {}

bool nor_done(int ticket)
{
    (void)ticket;
    return true;
}

void nor_wait_all(void) {}

int nor_read16(uint32_t a, void *dst, uint32_t n)
{
    (void)a;
    (void)dst;
    (void)n;
    return -1;
}

static void dma_setup(void) {}

void DMA1_Channel2_IRQHandler(void) { DMA_CLR = DMA_FLAGS(2); }

void nor_recover(void) { cs_hi(); }

#else

static uint8_t ff = 0xFF;
static uint16_t ff16 = 0xFFFF;

static struct req {
    uint32_t addr, left, seq;
    uint8_t *dst;
    int ticket;  // 0 = free slot
    int prio;
} q[QLEN];
// 1 + index of the request on the bus, BUSY_COL for a column read (nor_col_start), 0 when
// idle. The queue is empty while a column read is on the bus: every call that queues or
// takes the bus ends it first.
static volatile int busy;
#define BUSY_COL (QLEN + 1)
static volatile uint8_t sync_wanted;
static uint32_t cur_len, seq_ctr;  // bytes of the chunk or column read on the bus

// One tight loop: on the meter, a wait_sts() call per byte made the header cost 340 cycles
// instead of about 150. The bus is idle on entry.
static void header(uint32_t a)
{
    const uint8_t h[4] = {(uint8_t)(a >> 16), (uint8_t)(a >> 8), (uint8_t)a, 0xFF};
    uint32_t g = WAIT_N;
    cs_lo();
    spi_tx(0x0B);
    for (unsigned i = 0; i < 4; i++) {
        while (g && !(SPI_STS(SPI1) & SPI_TDBE))
            g--;
        spi_tx(h[i]);
    }
    while (g && (SPI_STS(SPI1) & (SPI_TDBE | SPI_BF)) != SPI_TDBE)
        g--;
    if (!g)
        nor_stats.errors++;
    (void)spi_rx();
    (void)SPI_STS(SPI1);
}

// Both channels are disabled and their flags clear here (finish() or dma_setup() ran last)
static void arm(void *dst, uint32_t n, uint32_t irq)
{
    SPI_CTRL1(SPI1) = SPI1_CFG;
    DMA_CMADDR(2) = (uint32_t)(uintptr_t)dst;
    DMA_CDTCNT(2) = n;
    DMA_CCTRL(2) = DMA_MINCM | DMA_CHPL(3) | irq | DMA_CHEN;
    DMA_CDTCNT(3) = n;
    DMA_CCTRL(3) = DMA_DTD | DMA_CHPL(2) | DMA_CHEN;
    SPI_CTRL1(SPI1) = SPI1_CFG | SPI_EN;
    SPI_CTRL2(SPI1) = SPI_DMAREN | SPI_DMATEN;
}

static void finish(void)
{
    SPI_CTRL2(SPI1) = 0;
    DMA_CCTRL(2) = 0;
    DMA_CCTRL(3) = 0;
    DMA_CLR = DMA_FLAGS(2) | DMA_FLAGS(3);
    cs_hi();
}

// Ends the column read on the bus, if there is one: polls its completion with the bound of
// nor_read and counts an error when it runs out or the DMA reports one.
static void col_end(void)
{
    if (busy != BUSY_COL)
        return;
    uint32_t st = 0;
    for (uint32_t w = cur_len * 64u + WAIT_N; w; w--) {
        st = DMA_STS & (DMA_FDTF(2) | DMA_ERRF(2));
        if (st)
            break;
    }
    if (st != DMA_FDTF(2))
        nor_stats.errors++;
    finish();
    busy = 0;
}

// A queued chunk (with the DMA interrupt) or a column read (BUSY_COL, polled) goes on the bus
static void start(uint32_t a, void *dst, uint32_t n, int b)
{
    cur_len = n;
    busy = b;
    header(a);
    arm(dst, n, b == BUSY_COL ? 0u : DMA_FDTIEN);
    nor_stats.async_chunks++;
}

// Interrupts masked or in the interrupt handler
static void start_next(void)
{
    struct req *best = NULL;
    for (unsigned i = 0; i < QLEN; i++) {
        struct req *r = &q[i];
        if (r->ticket && (!best || r->prio < best->prio ||
                          (r->prio == best->prio && (int32_t)(r->seq - best->seq) < 0)))
            best = r;
    }
    if (!best) {
        busy = 0;
        return;
    }
    uint32_t n = best->left > CHUNK ? CHUNK : best->left;
    start(best->addr, best->dst, n, 1 + (int)(best - q));
}

// Bounded waits: panics when no async chunk started for NOR_WAIT_MS
struct watch {
    uint32_t seen, t0;
};

static void watch_init(struct watch *w)
{
    w->seen = nor_stats.async_chunks;
    w->t0 = plat_ms();
}

static void watch(struct watch *w)
{
    uint32_t now = plat_ms(), n = nor_stats.async_chunks;
    if (n != w->seen) {
        w->seen = n;
        w->t0 = now;
    } else if (now - w->t0 > NOR_WAIT_MS) {
        plat_panic("nor: DMA stuck");
    }
}

// nor_stats.wait: DWT cycles from entry to exit of nor_read, nor_wait, nor_wait_all and
// nor_col_wait, which never run inside one another. NOR_PROFILE builds keep prof[] there
// instead.
static inline uint32_t wait_begin(void)
{
    return NOR_PROFILE ? 0u : DWT_CYCCNT;
}

static inline void wait_end(uint32_t c0)
{
    if (!NOR_PROFILE)
        nor_stats.wait += DWT_CYCCNT - c0;
}

void DMA1_Channel2_IRQHandler(void)
{
    uint32_t t0 = DWT_CYCCNT;
    uint32_t st = DMA_STS;
    finish();
    if (st & DMA_ERRF(2))
        nor_stats.errors++;
    // a stray interrupt during a column read must not touch q[]
    int i = busy - 1;
    if ((unsigned)i < QLEN) {
        struct req *r = &q[i];
        r->addr += cur_len;
        r->dst += cur_len;
        r->left -= cur_len;
        if (!r->left)
            r->ticket = 0;
    }
    // busy goes straight to the next chunk, never through 0, so claim() cannot slip in
    if (sync_wanted)
        busy = 0;
    else
        start_next();
    nor_stats.isr_cycles += DWT_CYCCNT - t0;
    irq_end();
}

// The column read ends before irq_save: it may take thousands of cycles, which would hold
// off the pump interrupt
static void kick(void)
{
    col_end();
    uint32_t m = irq_save();
    if (!busy && !sync_wanted)
        start_next();
    irq_restore(m);
}

// Takes the bus from the queue: the chunk in flight finishes, no new one starts
static void claim(void)
{
    struct watch w;
    col_end();
    sync_wanted = 1;
    watch_init(&w);
    while (busy)
        watch(&w);
}

static void release(void)
{
    sync_wanted = 0;
    kick();
}

// With the queue idle (busy == 0) nothing can start a transfer behind this thread's back
// (only the DMA interrupt of a running chunk and thread code start chunks), so the claim
// and its bookkeeping are skipped.
void nor_read(uint32_t a, void *dst, uint32_t n)
{
    uint32_t c0 = wait_begin();
    uint8_t *d = dst;
    int shared = busy != 0;
    nor_stats.sync++;
    if (!NOR_PROFILE)
        nor_stats.bytes += n;
    if (shared)
        claim();
    while (n) {
        uint32_t k = n > 0x8000u ? 0x8000u : n, t0 = DWT_CYCCNT, t1, t2, t3, st = 0;
        // A transfer that did not end (a byte lost on the bus) is read again: the record
        // cache would otherwise keep the short buffer as data.
        for (int tries = 3;; tries--) {
            header(a);
            t1 = DWT_CYCCNT;
            arm(d, k, 0);
            t2 = DWT_CYCCNT;
            for (uint32_t w = k * 64u + WAIT_N; w; w--) {
                st = DMA_STS & (DMA_FDTF(2) | DMA_ERRF(2));
                if (st)
                    break;
            }
            t3 = DWT_CYCCNT;
            finish();
            if (st == DMA_FDTF(2))
                break;
            nor_stats.errors++;
            if (!tries)
                plat_panic("nor: read failed");
        }
        if (NOR_PROFILE) {
            nor_stats.prof[0] += t1 - t0;
            nor_stats.prof[1] += t2 - t1;
            nor_stats.prof[2] += t3 - t2;
            nor_stats.prof[3] += DWT_CYCCNT - t3;
            nor_stats.prof_n++;
        }
        a += k;
        d += k;
        n -= k;
    }
    if (shared)
        release();
    wait_end(c0);
}

int nor_read_async(uint32_t a, void *dst, uint32_t n, int prio)
{
    if (!NOR_PROFILE) {
        nor_stats.async++;
        nor_stats.bytes += n;
    }
    if (!n)
        return new_ticket();
    col_end();
    struct watch w;
    watch_init(&w);
    for (;; watch(&w)) {
        uint32_t m = irq_save();
        for (unsigned i = 0; i < QLEN; i++) {
            struct req *r = &q[i];
            if (r->ticket)
                continue;
            r->addr = a;
            r->dst = dst;
            r->left = n;
            r->prio = prio;
            r->seq = seq_ctr++;
            r->ticket = new_ticket();
            int t = r->ticket;
            if (!busy && !sync_wanted)
                start_next();
            irq_restore(m);
            return t;
        }
        irq_restore(m);
        kick();
    }
}

bool nor_done(int ticket)
{
    __asm volatile("" ::: "memory");  // q[] is updated by the interrupt
    for (unsigned i = 0; i < QLEN; i++) {
        if (q[i].ticket == ticket)
            return false;
    }
    return true;
}

void nor_wait_all(void)
{
    uint32_t c0 = wait_begin();
    struct watch w;
    col_end();
    watch_init(&w);
    for (;; watch(&w)) {
        int any = 0;
        for (unsigned i = 0; i < QLEN; i++)
            any |= q[i].ticket;
        if (!any)
            break;
        kick();
    }
    wait_end(c0);
}

// The header goes out polled as for a sync read, the DMA runs without its interrupt and
// col_end polls the end. The counters see it as a queued request of one chunk. Outside the
// driver's calls busy is 0 only with the queue empty and sync_wanted clear, so busy alone
// tells whether queued requests own the bus.
int nor_col_start(uint32_t a, void *dst, uint32_t n)
{
    // the column read on the bus ends in nor_col_wait, which times it, a free bus skips that
    if (busy == BUSY_COL)
        nor_col_wait();
    if (busy || n - 1u >= CHUNK)
        return -1;
    if (!NOR_PROFILE) {
        nor_stats.async++;
        nor_stats.bytes += n;
    }
    start(a, dst, n, BUSY_COL);
    return 0;
}

void nor_col_wait(void)
{
    uint32_t c0 = wait_begin();
    col_end();
    wait_end(c0);
}

// Alternative sync read, not used by the firmware: the whole transaction in 16-bit frames.
// The header names a - 1, so the dummy byte and the byte before a fill the third frame and
// data frames start at a.
int nor_read16(uint32_t a, void *dst, uint32_t n)
{
    if (((a | n | (uint32_t)(uintptr_t)dst) & 1u) || !a || !n)
        return -1;
    int err = 0;
    uint8_t *d = dst;
    nor_stats.sync++;
    claim();
    DMA_CMADDR(3) = (uint32_t)(uintptr_t)&ff16;
    SPI_CTRL1(SPI1) = SPI1_CFG;  // the frame size changes only while SPI1 is disabled
    SPI_CTRL1(SPI1) = SPI1_CFG | SPI_FBN16;
    while (n && !err) {
        uint32_t k = n > 0x8000u ? 0x8000u : n, h = a - 1u;
        SPI_CTRL1(SPI1) = SPI1_CFG | SPI_FBN16 | SPI_EN;
        cs_lo();
        const uint16_t f[3] = {(uint16_t)(0x0B00u | ((h >> 16) & 0xFFu)), (uint16_t)h, 0xFFFFu};
        for (unsigned i = 0; i < 3; i++) {
            wait_sts(SPI_TDBE, SPI_TDBE);
            SPI_DT(SPI1) = f[i];
        }
        wait_sts(SPI_TDBE | SPI_BF, SPI_TDBE);
        (void)SPI_DT(SPI1);
        (void)SPI_STS(SPI1);
        SPI_CTRL1(SPI1) = SPI1_CFG | SPI_FBN16;
        DMA_CCTRL(2) = 0;
        DMA_CCTRL(3) = 0;
        DMA_CLR = DMA_FLAGS(2) | DMA_FLAGS(3);
        DMA_CMADDR(2) = (uint32_t)(uintptr_t)d;
        DMA_CDTCNT(2) = k / 2u;
        DMA_CCTRL(2) = DMA_P16 | DMA_M16 | DMA_MINCM | DMA_CHPL(3) | DMA_CHEN;
        DMA_CDTCNT(3) = k / 2u;
        DMA_CCTRL(3) = DMA_P16 | DMA_M16 | DMA_DTD | DMA_CHPL(2) | DMA_CHEN;
        SPI_CTRL1(SPI1) = SPI1_CFG | SPI_FBN16 | SPI_EN;
        SPI_CTRL2(SPI1) = SPI_DMAREN;
        SPI_CTRL2(SPI1) = SPI_DMAREN | SPI_DMATEN;
        uint32_t st = 0;
        for (uint32_t w = k * 64u + WAIT_N; w; w--) {
            st = DMA_STS & (DMA_FDTF(2) | DMA_ERRF(2));
            if (st)
                break;
        }
        err = st != DMA_FDTF(2);
        finish();
        SPI_CTRL1(SPI1) = SPI1_CFG | SPI_FBN16;
        a += k;
        d += k;
        n -= k;
    }
    SPI_CTRL1(SPI1) = SPI1_CFG;
    SPI_CTRL1(SPI1) = SPI1_CFG | SPI_EN;
    DMA_CMADDR(3) = (uint32_t)(uintptr_t)&ff;
    nor_stats.errors += (uint32_t)err;
    release();
    return err ? -1 : 0;
}

void nor_recover(void)
{
    SPI_CTRL2(SPI1) = 0;
    DMA_CCTRL(2) = 0;
    DMA_CCTRL(3) = 0;
    DMA_CLR = DMA_FLAGS(2) | DMA_FLAGS(3);
    cs_hi();
    SPI_CTRL1(SPI1) = SPI1_CFG;
    SPI_CTRL1(SPI1) = SPI1_CFG | SPI_EN;
    (void)spi_rx();
    (void)SPI_STS(SPI1);
    for (unsigned i = 0; i < QLEN; i++)
        q[i].ticket = 0;
    busy = 0;
    sync_wanted = 0;
}

static void dma_setup(void)
{
    DMA_CCTRL(2) = 0;
    DMA_CCTRL(3) = 0;
    DMA_CPADDR(2) = SPI_DT_ADDR(SPI1);
    DMA_CPADDR(3) = SPI_DT_ADDR(SPI1);
    DMA_CMADDR(3) = (uint32_t)(uintptr_t)&ff;
    NVIC_IPR(IRQ_DMA1_CH2) = IRQ_PRIO;
    NVIC_ISER0 = 1u << IRQ_DMA1_CH2;
}

#endif

void nor_wait(int ticket)
{
#if !NOR_POLLED
    uint32_t c0 = wait_begin();
    struct watch w;
    watch_init(&w);
    for (; !nor_done(ticket); watch(&w))
        kick();
    wait_end(c0);
#else
    (void)ticket;
#endif
}

static uint8_t status_raw(uint8_t cmd)
{
    cs_lo();
    xfer(cmd);
    uint8_t s = xfer(0xFF);
    cs_hi();
    return s;
}

uint8_t nor_status(uint8_t cmd)
{
    claim();
    uint8_t s = status_raw(cmd);
    release();
    return s;
}

static int wait_ready(uint32_t ms)
{
    uint32_t t0 = plat_ms();
    for (uint32_t n = 0;; n++) {
        if (!(status_raw(0x05) & 1u))
            return 0;
        if (plat_ms() - t0 > ms || n > ms * 20000u)
            return -1;
    }
}

static int write_enable(void)
{
    cs_lo();
    xfer(0x06);
    cs_hi();
    return (status_raw(0x05) & 2u) ? 0 : -1;  // WEL
}

static void cmd_addr(uint8_t cmd, uint32_t a)
{
    xfer(cmd);
    xfer((uint8_t)(a >> 16));
    xfer((uint8_t)(a >> 8));
    xfer((uint8_t)a);
}

// One timeout for both commands, above the worst case of the W25Q32JV: 400 ms for a sector,
// 2 s for a block
int nor_erase(uint32_t a, unsigned cmd)
{
    claim();
    int err = write_enable();
    if (!err) {
        cs_lo();
        cmd_addr((uint8_t)cmd, a);
        cs_hi();
        err = wait_ready(3000);
    }
    release();
    return err;
}

int nor_program(uint32_t a, const void *src, uint32_t n)
{
    const uint8_t *s = src;
    int err = 0;
    claim();
    while (n && !err) {
        uint32_t k = 256u - (a & 255u);
        if (k > n)
            k = n;
        err = write_enable();
        if (err)
            break;
        cs_lo();
        cmd_addr(0x02, a);
        for (uint32_t i = 0; i < k; i++)
            xfer(s[i]);
        cs_hi();
        err = wait_ready(10);
        a += k;
        s += k;
        n -= k;
    }
    release();
    return err;
}

int nor_init(void)
{
    pin_cfg(GPIOA, PIN_NOR_CS, PIN_OUT_PP);
    cs_hi();
    pin_cfg(GPIOA, 5, PIN_AF_PP);
    pin_cfg(GPIOA, 6, PIN_IN);
    pin_cfg(GPIOA, 7, PIN_AF_PP);
    SPI_CTRL1(SPI1) = 0;
    SPI_CTRL2(SPI1) = 0;
    SPI_CTRL1(SPI1) = SPI1_CFG;
    SPI_CTRL1(SPI1) = SPI1_CFG | SPI_EN;
    (void)spi_rx();
    (void)SPI_STS(SPI1);
    dma_setup();

    cs_lo();
    xfer(0xAB);  // release from power-down
    cs_hi();
    spin_cycles(144u * 50u);
    cs_lo();
    xfer(0x9F);
    uint32_t id = (uint32_t)xfer(0xFF) << 16;
    id |= (uint32_t)xfer(0xFF) << 8;
    id |= xfer(0xFF);
    cs_hi();
    jedec = id;
    return (id == 0 || id == 0xFFFFFFu) ? -1 : 0;
}
