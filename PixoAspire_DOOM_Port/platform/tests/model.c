// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Host model test of the DMA paths of pump.c and nor.c, which Renode cannot run (its SPI
// model drops receive DMA inside transmit DMA and mangles 16-bit frames).
//
// The sources are built unchanged except for -include model.h (registers go to an emulated
// register file) and NOR_MODEL (SPI1 data and chip select go to a W25Q32 model). A thread
// plays DMA1 channels 2, 3 and 5: it runs a transfer as soon as one is armed and calls the
// interrupt handler with the interrupt lock held, which is what irq_save() takes. Random
// delays on both sides drive the pump into starvation restarts and mix NOR sync reads,
// queued async chunks and erase/program.
//
// Buffers that DMA touches must be static: the 32-bit address registers need them below
// 4 GB, so the test is linked without PIE.
//
// plat_panic() returns to the test through longjmp when a test expects it, so the stuck-DMA
// timeouts and the window check can be checked too.

#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "at32.h"
#include "lcd.h"
#include "nor.h"
#include "platform.h"

void DMA1_Channel2_IRQHandler(void);
void DMA1_Channel5_IRQHandler(void);

static int failures;
#define CHECK(c, ...)                         \
    do {                                      \
        printf((c) ? "  ok   " : "  FAIL "); \
        printf(__VA_ARGS__);                  \
        printf("\n");                         \
        failures += !(c);                     \
    } while (0)

// ---- Register file ----

static struct {
    uint32_t addr;
    volatile uint32_t val;
} slot[256];
static volatile int nslot;
static pthread_mutex_t slot_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t irq_lock;

static volatile uint32_t *find(uint32_t a)
{
    for (int i = 0; i < nslot; i++)
        if (slot[i].addr == a)
            return &slot[i].val;
    pthread_mutex_lock(&slot_lock);
    int i = 0;
    for (; i < nslot && slot[i].addr != a; i++) {
    }
    if (i == nslot) {
        if (nslot == 256) {
            fprintf(stderr, "register file full\n");
            exit(2);
        }
        slot[i].addr = a;
        slot[i].val = a == 0x40013008u || a == 0x40003808u ? SPI_TDBE : 0u;
        __sync_synchronize();
        nslot = i + 1;
    }
    pthread_mutex_unlock(&slot_lock);
    return &slot[i].val;
}

#define R(a) (*find(a))

static int chip_rx_pending;
static volatile int dma_hold;  // 1: the DMA channels stop answering
static void step_nor(void);

static void apply_clr(void)
{
    uint32_t c = R(DMA1 + 0x04u);
    if (c) {
        R(DMA1 + 0x00u) &= ~c;
        R(DMA1 + 0x04u) = 0;
    }
}

volatile uint32_t *model_reg(uint32_t a)
{
    if (a == 0xE0001004u)  // DWT_CYCCNT
        R(a) += 7u;
    if (a == SPI1 + 0x08u)
        R(a) = SPI_TDBE | (chip_rx_pending ? SPI_RDBF : 0u);
    if (a == DMA1 + 0x00u) {
        // A polled (sync) NOR transfer completes while the CPU polls, like on the chip
        pthread_mutex_lock(&irq_lock);
        apply_clr();
        if (!(R(DMA1 + 0x08u + 20u * 1u) & DMA_FDTIEN) && !dma_hold)
            step_nor();
        pthread_mutex_unlock(&irq_lock);
    }
    return find(a);
}

uint32_t irq_save(void)
{
    pthread_mutex_lock(&irq_lock);
    return 0;
}

void irq_restore(uint32_t m)
{
    (void)m;
    pthread_mutex_unlock(&irq_lock);
}

void spin_cycles(uint32_t n) { (void)n; }

static jmp_buf panic_jmp;
static int panic_armed;
static char panic_msg[64];

void plat_panic(const char *msg)
{
    snprintf(panic_msg, sizeof(panic_msg), "%s", msg);
    if (panic_armed) {
        panic_armed = 0;
        longjmp(panic_jmp, 1);
    }
    fprintf(stderr, "panic: %s\n", msg);
    exit(2);
}

uint32_t plat_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// ---- W25Q32 model behind the SPI1 hooks ----

static uint8_t chip[NOR_SIZE];
static int cs_low, cmd, nb, wel, chip_err, plen;
static uint32_t caddr;
static uint8_t rx, pbuf[256];
static uint32_t erases, blocks, programs;

void cs_lo(void)
{
    chip_err += cs_low;
    cs_low = 1;
    cmd = -1;
    nb = 0;
    caddr = 0;
    plen = 0;
}

void cs_hi(void)
{
    if (!cs_low)
        return;
    cs_low = 0;
    if (cmd == 0x06) {
        wel = 1;
    } else if ((cmd == 0x20 || cmd == 0xD8) && nb == 4) {
        // sector or block erase, the address bits inside it are ignored
        uint32_t n = cmd == 0x20 ? 4096u : 0x10000u;
        chip_err += !wel;
        if (wel)
            memset(chip + (caddr & (NOR_SIZE - n)), 0xFF, n);
        if (cmd == 0x20)
            erases++;
        else
            blocks++;
        wel = 0;
    } else if (cmd == 0x02 && nb > 4) {
        chip_err += !wel;
        for (int i = 0; wel && i < plen; i++)
            chip[((caddr & ~255u) | ((caddr + (uint32_t)i) & 255u)) & (NOR_SIZE - 1u)] &= pbuf[i];
        programs++;
        wel = 0;
    }
}

void spi_tx(uint8_t b)
{
    uint8_t out = 0xFF;
    chip_err += !cs_low;
    if (nb == 0)
        cmd = b;
    else if ((cmd == 0x0B || cmd == 0x02 || cmd == 0x20 || cmd == 0xD8) && nb <= 3)
        caddr = (caddr << 8) | b;
    else if (cmd == 0x0B && nb >= 5)
        out = chip[caddr++ & (NOR_SIZE - 1u)];
    else if (cmd == 0x02 && plen < 256)
        pbuf[plen++] = b;
    else if (cmd == 0x9F && nb <= 3)
        out = (uint8_t[]){0, 0xEF, 0x40, 0x16}[nb];
    else if (cmd == 0x05)
        out = (uint8_t)(wel << 1);
    nb++;
    rx = out;
    chip_rx_pending = 1;
}

uint8_t spi_rx(void)
{
    chip_rx_pending = 0;
    return rx;
}

// ---- LCD side of the pump ----

static uint16_t stream[320 * 240 * 2];
static uint32_t nstream, ends;
static int win[4];

void lcd_window(int x0, int y0, int x1, int y1)
{
    win[0] = x0, win[1] = y0, win[2] = x1, win[3] = y1;
    nstream = 0;
}

void lcd_data_end(void) { ends++; }

// ---- DMA thread ----

static volatile int stop, jitter;
static uint32_t dma_err;

// Async NOR transfers in the order the DMA ran them (destination of each chunk)
static uint8_t *xfer_log[4096];
static volatile uint32_t nxfer;

static void step_pump(void)
{
    uint32_t cc = R(DMA1 + 0x08u + 20u * 4u);
    if (!(cc & DMA_CHEN) || !(R(SPI2 + 0x04u) & SPI_DMATEN))
        return;
    apply_clr();  // flag clears written before the channel was armed
    uint32_t n = R(DMA1 + 0x0Cu + 20u * 4u);
    const uint16_t *src = (const uint16_t *)(uintptr_t)R(DMA1 + 0x14u + 20u * 4u);
    if (cc != (DMA_DTD | DMA_MINCM | DMA_P16 | DMA_M16 | DMA_CHPL(1) | DMA_FDTIEN | DMA_CHEN) ||
        R(DMA1 + 0x10u + 20u * 4u) != SPI_DT_ADDR(SPI2) || nstream + n > sizeof(stream) / 2)
        dma_err++;
    else
        for (uint32_t i = 0; i < n; i++)
            stream[nstream++] = src[i];
    R(DMA1 + 0x08u + 20u * 4u) = cc & ~DMA_CHEN;
    R(DMA1 + 0x00u) |= DMA_FDTF(5);
    if (cc & DMA_FDTIEN)
        DMA1_Channel5_IRQHandler();
}

static void step_nor(void)
{
    uint32_t c2 = R(DMA1 + 0x08u + 20u * 1u), c3 = R(DMA1 + 0x08u + 20u * 2u);
    if (!(c2 & DMA_CHEN) || !(c3 & DMA_CHEN) || R(SPI1 + 0x04u) != (SPI_DMAREN | SPI_DMATEN) ||
        !(R(SPI1 + 0x00u) & SPI_EN))
        return;
    apply_clr();
    uint32_t n = R(DMA1 + 0x0Cu + 20u * 1u);
    uint8_t *dst = (uint8_t *)(uintptr_t)R(DMA1 + 0x14u + 20u * 1u);
    const uint8_t *ffp = (const uint8_t *)(uintptr_t)R(DMA1 + 0x14u + 20u * 2u);
    if ((c2 & ~(DMA_CHEN | DMA_FDTIEN)) != (DMA_MINCM | DMA_CHPL(3)) ||
        (c3 & ~DMA_CHEN) != (DMA_DTD | DMA_CHPL(2)) || R(DMA1 + 0x0Cu + 20u * 2u) != n || *ffp != 0xFF ||
        !cs_low || cmd != 0x0B || nb < 5 || !n) {
        dma_err++;
    } else {
        for (uint32_t i = 0; i < n; i++)
            dst[i] = chip[(caddr + i) & (NOR_SIZE - 1u)];
        caddr += n;
        nb += (int)n;
        if ((c2 & DMA_FDTIEN) && nxfer < 4096)
            xfer_log[nxfer++] = dst;
    }
    R(DMA1 + 0x08u + 20u * 1u) = c2 & ~DMA_CHEN;
    R(DMA1 + 0x08u + 20u * 2u) = c3 & ~DMA_CHEN;
    R(DMA1 + 0x00u) |= DMA_FDTF(2) | DMA_FDTF(3);
    if (c2 & DMA_FDTIEN)
        DMA1_Channel2_IRQHandler();
}

static void *dma_thread(void *arg)
{
    unsigned seed = 12345;
    (void)arg;
    while (!stop) {
        pthread_mutex_lock(&irq_lock);
        apply_clr();
        if (!dma_hold) {
            step_pump();
            step_nor();
        }
        pthread_mutex_unlock(&irq_lock);
        int r = rand_r(&seed) % 64;
        if (jitter && r < 3)
            usleep((useconds_t)(r * 30));
        else
            sched_yield();
    }
    return NULL;
}

// ---- Tests ----

static unsigned seed = 1;
static uint32_t rnd(void) { return (uint32_t)rand_r(&seed); }

static void main_jitter(void)
{
    if (jitter && rnd() % 4 == 0)
        usleep(rnd() % 200);
}

static uint8_t pat(int x, int y, int salt) { return (uint8_t)(x * 7 + y * 13 + salt * 31 + (x >> 3)); }

static uint8_t strips[2][16 * 200];

static int pump_case(int y0, int y1, int dbl, int w, int salt)
{
    int h = y1 - y0 + 1, cols = dbl ? 160 : 320;
    uint32_t irq0 = pump_stats.irqs, ends0 = ends;
    pump_begin(0, LCD_W - 1, y0, y1, dbl ? PUMP_DOUBLE : 0);
    for (int s = 0; s < cols / w; s++) {
        uint8_t *b = strips[s & 1];
        pump_sync(1);
        // The pump must no longer read b: any stale read shows up in the stream
        for (int c = 0; c < w; c++)
            for (int y = 0; y < h; y++)
                b[c * h + y] = pat(s * w + c, y, salt);
        main_jitter();
        pump_submit(b, w);
    }
    pump_end();
    int bad = nstream != (uint32_t)(320 * h);
    for (uint32_t i = 0; !bad && i < nstream; i++) {
        int x = (int)i / h, y = (int)i % h;
        bad = stream[i] != pump_lut[pat(dbl ? x / 2 : x, y, salt)];
    }
    bad |= win[0] != 0 || win[2] != LCD_W - 1 || win[1] != y0 || win[3] != y1;
    bad |= pump_stats.irqs - irq0 != 640u || ends != ends0 + 1 || (R(SPI2 + 0x04u) & SPI_DMATEN);
    return !bad;
}

static void test_pump(void)
{
    for (int i = 0; i < 256; i++)
        pump_lut[i] = (uint16_t)(i * 0x0101u ^ 0x5A3Cu);
    for (int j = 0; j < 2; j++) {
        jitter = j;
        uint32_t r0 = pump_stats.restarts;
        int ok = 1;
        for (int k = 0; k < 4; k++) {
            ok &= pump_case(0, 199, 1, 8, k);
            ok &= pump_case(0, 199, 1, 16, k + 10);
            ok &= pump_case(0, 199, 0, 8, k + 20);
            ok &= pump_case(200, 207, 0, 160, k + 30);
            ok &= pump_case(208, 239, 0, 40, k + 40);
        }
        CHECK(ok, "pump %s: 20 frames (low W8, low W16, high W8, HUD, status bar) exact, %u restarts",
              j ? "with jitter" : "no jitter", pump_stats.restarts - r0);
    }
    CHECK(dma_err == 0, "pump DMA programming as expected (%u errors)", dma_err);

    // A window whose column height is not a multiple of 4 is refused
    panic_msg[0] = 0;
    panic_armed = 1;
    if (!setjmp(panic_jmp))
        pump_begin(0, LCD_W - 1, 0, 5, 0);
    panic_armed = 0;
    CHECK(!strcmp(panic_msg, "pump: bad window"), "pump_begin with h = 6 panics (%s)", panic_msg);

    // DMA stops answering: pump_end gives up, the frame completes once the DMA runs again
    jitter = 0;
    panic_msg[0] = 0;
    dma_hold = 1;
    pump_begin(0, LCD_W - 1, 0, 199, 0);
    for (int c = 0; c < 8 * 200; c++)
        strips[0][c] = (uint8_t)c;
    pump_submit(strips[0], 8);
    panic_armed = 1;
    if (!setjmp(panic_jmp))
        pump_end();
    panic_armed = 0;
    dma_hold = 0;
    pump_end();
    int ok = nstream == 8u * 200u;
    for (uint32_t i = 0; ok && i < nstream; i++)
        ok = stream[i] == pump_lut[(uint8_t)i];
    CHECK(!strcmp(panic_msg, "pump: DMA stuck") && ok,
          "stuck pump DMA: pump_end panics (%s), then the data still arrives", panic_msg);
}

static uint8_t nbuf[4][8192], ref[8192];

static int same(const uint8_t *a, uint32_t addr, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++)
        if (a[i] != chip[(addr + i) & (NOR_SIZE - 1u)])
            return 0;
    return 1;
}

static void test_nor(void)
{
    for (uint32_t i = 0; i < NOR_SIZE; i++)
        chip[i] = (uint8_t)(i * 2654435761u >> 13);
    CHECK(nor_init() == 0 && nor_jedec() == NOR_JEDEC_W25Q32, "JEDEC %06X", nor_jedec());
    CHECK(nor_status(0x05) == 0, "status register read");

    int ok = 1;
    uint32_t sync0 = nor_stats.sync, async0 = nor_stats.async, bytes0 = nor_stats.bytes;
    uint32_t wait0 = nor_stats.wait, nbytes = 0;
    for (int i = 0; i < 300; i++) {
        uint32_t n = 1 + rnd() % 8192, a = rnd() % NOR_SIZE;
        nor_read(a, nbuf[0], n);
        ok &= same(nbuf[0], a, n);
        nbytes += n;
    }
    CHECK(ok, "300 sync reads of 1..8192 B (wrapping at the chip end) match");
    CHECK(nor_stats.sync - sync0 == 300 && nor_stats.async == async0 &&
              nor_stats.bytes - bytes0 == nbytes && nor_stats.wait != wait0,
          "the driver counts 300 sync reads, their %u B and its cycles in them", nbytes);
    uint32_t nsync = 0, nasync = 0;
    sync0 = nor_stats.sync;
    bytes0 = nor_stats.bytes;
    nbytes = 0;

    int wait_ok = 1;
    for (int j = 0; j < 2; j++) {
        jitter = j;
        ok = 1;
        int order_ok = 1;
        for (int it = 0; it < 200; it++) {
            int t[4];
            uint32_t a[4], n[4];
            for (int k = 0; k < 4; k++) {
                a[k] = rnd() % NOR_SIZE;
                n[k] = 1 + rnd() % 8192;
                memset(nbuf[k], 0xA5, n[k]);
            }
            // A big flat read first, then a column read that must overtake it at a chunk
            // boundary, then two more at mixed priorities, with sync reads in between
            n[0] |= 4096u;
            n[1] = n[1] % 256u + 1u;
            uint32_t m = irq_save();
            nxfer = 0;
            t[0] = nor_read_async(a[0], nbuf[0], n[0], NOR_PRIO_FLAT);
            t[1] = nor_read_async(a[1], nbuf[1], n[1], NOR_PRIO_COL);
            irq_restore(m);
            uint32_t sa = rnd() % NOR_SIZE, sn = 1 + rnd() % 300;
            main_jitter();
            nor_read(sa, ref, sn);
            ok &= same(ref, sa, sn);
            uint32_t w = nor_stats.wait;
            t[2] = nor_read_async(a[2], nbuf[2], n[2], NOR_PRIO_REC);
            t[3] = nor_read_async(a[3], nbuf[3], n[3], (int)(rnd() % 3));
            wait_ok &= nor_stats.wait == w;
            for (int k = 0; k < 4; k++) {
                w = nor_stats.wait;
                nor_wait(t[k]);
                wait_ok &= nor_stats.wait != w;
                ok &= same(nbuf[k], a[k], n[k]);
                nbytes += n[k];
            }
            nsync++;
            nasync += 4;
            nbytes += sn;
            // The flat read's first chunk was already armed, the column read must be next
            order_ok &= nxfer >= 2 && xfer_log[0] == nbuf[0] && xfer_log[1] == nbuf[1];
        }
        uint32_t w = nor_stats.wait;
        nor_wait_all();
        wait_ok &= nor_stats.wait != w;
        CHECK(ok, "async %s: 200 rounds of 4 queued reads with sync reads between, data exact",
              j ? "with jitter" : "no jitter");
        CHECK(order_ok, "a column read queued behind a big flat read runs right after its first chunk");
    }
    CHECK(nor_stats.sync - sync0 == nsync && nor_stats.async - async0 == nasync &&
              nor_stats.bytes - bytes0 == nbytes,
          "the driver counts %u sync and %u queued reads of %u B", nsync, nasync, nbytes);
    CHECK(wait_ok, "nor_wait and nor_wait_all count their cycles in wait, nor_read_async does not");

    uint32_t base = 0x3FF000u;
    static uint8_t page[600];
    for (int i = 0; i < 600; i++)
        page[i] = (uint8_t)(i * 37 + 5);
    int t = nor_read_async(0x100000u, nbuf[0], 8000, NOR_PRIO_FLAT);
    int e = nor_erase4k(base);
    int p = nor_program(base + 200u, page, 600);
    nor_wait(t);
    ok = e == 0 && p == 0 && same(nbuf[0], 0x100000u, 8000);
    nor_read(base, nbuf[1], 4096);
    for (int i = 0; i < 4096; i++)
        ok &= nbuf[1][i] == (i >= 200 && i < 800 ? page[i - 200] : 0xFF);
    CHECK(ok, "erase 4 KB and a 600 B program across 3 pages while an async read is queued");
    CHECK(erases == 1 && programs == 4, "one erase, 4 page programs (%u, %u)", erases, programs);

    uint32_t blk = 0x2F0000u;
    uint8_t edge[2] = {chip[blk - 1u], chip[blk + 0x10000u]};
    t = nor_read_async(0x100000u, nbuf[0], 8000, NOR_PRIO_FLAT);
    e = nor_erase(blk, NOR_ERASE_64K);
    nor_wait(t);
    ok = e == 0 && same(nbuf[0], 0x100000u, 8000) && chip[blk - 1u] == edge[0] &&
         chip[blk + 0x10000u] == edge[1];
    for (uint32_t i = 0; i < 0x10000u; i++)
        ok &= chip[blk + i] == 0xFF;
    CHECK(ok, "erase of a 64 KB block while an async read is queued: the block reads 0xFF, the bytes "
              "around it are kept");
    CHECK(blocks == 1 && erases == 1, "one block erase command (%u, %u sector erases)", blocks, erases);
    CHECK(chip_err == 0 && dma_err == 0, "chip select, write enable and DMA use as expected (%d, %u)",
          chip_err, dma_err);
    CHECK(nor_stats.errors == 0, "driver error counter %u", nor_stats.errors);

    // DMA stops answering: nor_wait gives up, the read completes once the DMA runs again
    jitter = 0;
    panic_msg[0] = 0;
    dma_hold = 1;
    memset(nbuf[0], 0, 4096);
    t = nor_read_async(0x200000u, nbuf[0], 4096, NOR_PRIO_COL);
    panic_armed = 1;
    if (!setjmp(panic_jmp))
        nor_wait(t);
    panic_armed = 0;
    dma_hold = 0;
    nor_wait_all();
    CHECK(!strcmp(panic_msg, "nor: DMA stuck") && same(nbuf[0], 0x200000u, 4096),
          "stuck NOR DMA: nor_wait panics (%s), then the read completes", panic_msg);
}

// ---- Direct column reads ----

// The DMA thread stops (or resumes) stepping once it is outside a step, which it makes with
// the interrupt lock held
static void hold(int on)
{
    dma_hold = on;
    irq_restore(irq_save());
}

// The renderer's rule (engine/r_data.c, R_ColRead and R_DrawQueued) on buffers nbuf[0] and
// nbuf[1]: a column read first ends the direct read of the column queued before it, whose
// buffer then holds no -1 ticket, and goes to the bus directly when the driver takes it. A
// draw ends the direct read on the bus when its buffer holds -1 and waits for a queued
// ticket otherwise.
static int colt[2];

static void col_read(int k, uint32_t a, uint32_t n)
{
    nor_col_wait();
    if (colt[!k] < 0)
        colt[!k] = 0;
    memset(nbuf[k], 0xA5, n);
    colt[k] = nor_col_start(a, nbuf[k], n) ? nor_read_async(a, nbuf[k], n, NOR_PRIO_COL) : -1;
}

static void col_draw(int k)
{
    if (colt[k] < 0)
        nor_col_wait();
    else if (colt[k])
        nor_wait(colt[k]);
    colt[k] = 0;
}

static uint32_t col_addr(void) { return rnd() % (NOR_SIZE - 256u); }

// A renderer-like sequence of steps on both buffers, in about the proportions of a DEMO1
// frame (506 column requests of which 66 hit a buffer, 1,379 sync record reads, 80 strip
// halves), plus queued flat reads and nor_wait_all. Every column is checked when it is drawn.
static int col_random(int steps)
{
    int ok = 1, qb = -1, ft = 0;
    uint32_t ka[2] = {0, 0}, kn[2] = {0, 0}, fa = 0, fn = 0;
    for (int s = 0; s < steps; s++) {
        uint32_t r = rnd() % 1000;
        if (r < 270) {
            int k;
            if (qb >= 0 && r < 35 && kn[r & 1]) {
                k = (int)(r & 1);  // the data a buffer holds is used again
            } else {
                k = qb < 0 ? 0 : !qb;
                ka[k] = col_addr();
                kn[k] = 1 + rnd() % 256;
                col_read(k, ka[k], kn[k]);
            }
            main_jitter();
            if (qb >= 0) {
                col_draw(qb);
                ok &= same(nbuf[qb], ka[qb], kn[qb]);
            }
            qb = k;
        } else if (r < 930) {
            uint32_t sa = rnd() % NOR_SIZE, sn = 1 + rnd() % 64;
            nor_read(sa, ref, sn);
            ok &= same(ref, sa, sn);
        } else if (r < 960) {
            if (ft) {
                nor_wait(ft);
                ok &= same(nbuf[2], fa, fn);
                ft = 0;
            } else {
                fa = rnd() % NOR_SIZE;
                fn = 1 + rnd() % 1024;
                memset(nbuf[2], 0xA5, fn);
                ft = nor_read_async(fa, nbuf[2], fn, NOR_PRIO_FLAT);
            }
        } else if (r < 965) {
            nor_wait_all();
            if (ft)
                ok &= same(nbuf[2], fa, fn);
            ft = 0;
        } else if (qb >= 0) {
            // end of a strip half: the queued column is drawn, the buffers are forgotten
            col_draw(qb);
            ok &= same(nbuf[qb], ka[qb], kn[qb]);
            nor_col_wait();
            for (int k = 0; k < 2; k++) {
                if (colt[k] > 0)
                    nor_wait(colt[k]);
                colt[k] = 0;
                kn[k] = 0;
            }
            qb = -1;
        }
    }
    if (qb >= 0) {
        col_draw(qb);
        ok &= same(nbuf[qb], ka[qb], kn[qb]);
    }
    nor_wait_all();
    if (ft)
        ok &= same(nbuf[2], fa, fn);
    return ok;
}

static void test_col(void)
{
    uint32_t e0 = nor_stats.errors;
    for (int j = 0; j < 2; j++) {
        const char *jt = j ? "with jitter" : "no jitter";
        jitter = j;
        int ok = 1, onbus = 1;
        uint32_t async0 = nor_stats.async, bytes0 = nor_stats.bytes, nbytes = 0;
        for (int it = 0; it < 200; it++) {
            uint32_t a0 = col_addr(), a1 = col_addr(), n0 = 1 + rnd() % 256, n1 = 1 + rnd() % 256;
            col_read(0, a0, n0);
            main_jitter();
            // the read of B ends A's (the rule), then B stays on the bus during A's draw
            nor_col_wait();
            hold(1);
            col_read(1, a1, n1);
            onbus &= colt[0] == 0 && colt[1] == -1 && (R(DMA1 + 0x08u + 20u * 1u) & DMA_CHEN);
            ok &= same(nbuf[0], a0, n0);
            hold(0);
            col_draw(1);
            ok &= same(nbuf[1], a1, n1);
            nbytes += n0 + n1;
        }
        CHECK(ok && onbus, "direct %s: 200 pairs of column reads, data exact, the first drawn "
              "while the second is on the bus", jt);
        CHECK(nor_stats.async - async0 == 400 && nor_stats.bytes - bytes0 == nbytes,
              "the driver counts the 400 direct reads and their %u B as queued requests", nbytes);

        // what ends a direct read: sync reads, a queued flat read, erase and program,
        // nor_wait_all. Its data is complete right after each.
        ok = 1;
        uint32_t base = 0x3FE000u;
        static uint8_t page[300];
        for (int i = 0; i < 300; i++)
            page[i] = (uint8_t)(i * 11 + j);
        for (int it = 0; it < 50; it++) {
            uint32_t a = col_addr(), n = 1 + rnd() % 256;
            uint32_t sa = rnd() % NOR_SIZE, sn = 1 + rnd() % 300;
            col_read(0, a, n);
            nor_read(sa, ref, sn);
            ok &= same(nbuf[0], a, n) && same(ref, sa, sn);
            col_draw(0);

            col_read(0, a, n);
            memset(nbuf[2], 0xA5, 1024);
            int t = nor_read_async(sa, nbuf[2], 1024, NOR_PRIO_FLAT);
            ok &= same(nbuf[0], a, n);
            col_draw(0);
            nor_wait(t);
            ok &= same(nbuf[2], sa, 1024);

            col_read(1, a, n);
            nor_wait_all();
            ok &= same(nbuf[1], a, n);
            col_draw(1);
        }
        col_read(0, 0x123456u, 200);
        int e = nor_erase4k(base);
        ok &= e == 0 && same(nbuf[0], 0x123456u, 200);
        col_draw(0);
        col_read(1, 0x234567u, 256);
        int p = nor_program(base + 100u, page, 300);
        ok &= p == 0 && same(nbuf[1], 0x234567u, 256);
        col_draw(1);
        nor_read(base, nbuf[2], 4096);
        for (int i = 0; i < 4096; i++)
            ok &= nbuf[2][i] == (i >= 100 && i < 400 ? page[i - 100] : 0xFF);
        CHECK(ok, "direct %s: sync reads, queued reads, nor_wait_all, erase and program end the "
              "direct read first, data exact", jt);

        // queued requests own the bus: the direct read is refused, the caller queues it
        ok = 1;
        int refused = 1;
        for (int it = 0; it < 50; it++) {
            uint32_t fa = rnd() % NOR_SIZE, a = col_addr(), n = 1 + rnd() % 256;
            memset(nbuf[2], 0xA5, 4096);
            hold(1);
            int t = nor_read_async(fa, nbuf[2], 4096, NOR_PRIO_FLAT);
            col_read(0, a, n);
            refused &= colt[0] > 0;
            hold(0);
            col_draw(0);
            ok &= same(nbuf[0], a, n);
            nor_wait(t);
            ok &= same(nbuf[2], fa, 4096);
        }
        refused &= nor_col_start(0x1000u, nbuf[1], 0) == -1;
        refused &= nor_col_start(0x1000u, nbuf[1], 257) == -1;
        CHECK(ok && refused, "direct %s: refused while a queued read owns the bus (and for 0 and "
              "257 B), the queued fallback reads exact data", jt);

        // nor_recover with a direct read on the bus, then reads work
        hold(1);
        col_read(0, 0x345678u, 128);
        nor_recover();
        hold(0);
        colt[0] = 0;
        ok = 1;
        for (int it = 0; it < 20; it++) {
            uint32_t a = col_addr(), n = 1 + rnd() % 256, sa = rnd() % NOR_SIZE;
            col_read(1, a, n);
            nor_read(sa, ref, 100);
            ok &= same(ref, sa, 100);
            col_draw(1);
            ok &= same(nbuf[1], a, n);
            int t = nor_read_async(sa, nbuf[2], 600, NOR_PRIO_COL);
            nor_wait(t);
            ok &= same(nbuf[2], sa, 600);
        }
        CHECK(ok, "direct %s: nor_recover with a direct read on the bus, then every read works",
              jt);

        // a stray DMA ch2 interrupt during a direct read must not index q[] (ASan sees q[4]),
        // the queue keeps working
        hold(1);
        col_read(0, 0x456789u, 64);
        irq_save();
        DMA1_Channel2_IRQHandler();
        irq_restore(0);
        hold(0);
        colt[0] = 0;
        ok = 1;
        for (int it = 0; it < 20; it++) {
            int t[4];
            uint32_t a[4];
            for (int k = 0; k < 4; k++) {
                a[k] = rnd() % NOR_SIZE;
                memset(nbuf[k], 0xA5, 700);
                t[k] = nor_read_async(a[k], nbuf[k], 700, (int)(rnd() % 3));
            }
            for (int k = 0; k < 4; k++) {
                nor_wait(t[k]);
                ok &= same(nbuf[k], a[k], 700);
            }
            uint32_t ca = col_addr();
            col_read(1, ca, 256);
            col_draw(1);
            ok &= same(nbuf[1], ca, 256);
        }
        CHECK(ok, "direct %s: a stray DMA interrupt during a direct read, then queued and direct "
              "reads work", jt);

        CHECK(col_random(20000), "direct %s: 20,000 random renderer steps (column hits and misses, "
              "sync, flat and queued-fallback reads, waits), every column exact when drawn", jt);
    }
    CHECK(nor_stats.errors == e0 && chip_err == 0 && dma_err == 0,
          "no driver, chip select or DMA errors (%u, %d, %u)", nor_stats.errors - e0, chip_err,
          dma_err);

    // the DMA stops answering during a direct read: its end gives up after the bound with
    // one error, the next transactions read exact data
    jitter = 0;
    hold(1);
    col_read(0, 0x567890u, 256);
    uint32_t w0 = nor_stats.wait;
    col_draw(0);
    hold(0);
    uint32_t e1 = nor_stats.errors;
    int ok = 1;
    for (int it = 0; it < 20; it++) {
        uint32_t a = col_addr(), sa = rnd() % NOR_SIZE;
        col_read(1, a, 256);
        col_draw(1);
        ok &= same(nbuf[1], a, 256);
        nor_read(sa, ref, 200);
        ok &= same(ref, sa, 200);
        int t = nor_read_async(sa, nbuf[2], 900, NOR_PRIO_FLAT);
        nor_wait(t);
        ok &= same(nbuf[2], sa, 900);
    }
    CHECK(e1 == e0 + 1 && nor_stats.wait != w0 && ok && nor_stats.errors == e1,
          "stuck DMA during a direct read: nor_col_wait gives up with one error (%u), then reads "
          "are exact", e1 - e0);
}

int main(void)
{
    pthread_mutexattr_t at;
    pthread_mutexattr_init(&at);
    pthread_mutexattr_settype(&at, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&irq_lock, &at);
    if ((uintptr_t)(uint32_t)(uintptr_t)strips != (uintptr_t)strips) {
        fprintf(stderr, "static data above 4 GB, link without PIE\n");
        return 2;
    }
    pthread_t th;
    pthread_create(&th, NULL, dma_thread, NULL);
    printf("pump model\n");
    test_pump();
    printf("NOR model\n");
    test_nor();
    printf("NOR direct column reads\n");
    test_col();
    stop = 1;
    pthread_join(th, NULL);
    printf(failures ? "%d FAILED\n" : "ALL PASSED\n", failures);
    return failures != 0;
}
