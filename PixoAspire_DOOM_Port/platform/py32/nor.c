// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
// Copyright (C) 2026 VapeDoom port
//
// 16 MB Puya SPI NOR (JEDEC 0x852018) on SPI1, sharing the bus and the data
// line with the panel.
//
// Two things about this board decide the shape of the driver.
//
// First, MOSI and MISO are the same net. That was not in any document; it was
// measured by driving one pin and reading the other against both pull
// directions, in both directions, after the chip had stayed silent and
// answered every read with the command byte it had just been sent. The bus is
// three-wire: the master has to let go of the data line before the chip
// answers. Instead of switching the peripheral to half duplex - where the
// clock free-runs and stopping it cleanly is fiddly - PB5 is simply taken off
// the pad for the data phase. SPI1 keeps generating the clock, the answer
// comes back on PB4 from the same net, and the byte count stays exact.
//
// Second, the panel is on the same chip select domain. Every access here
// lifts the panel's CS and puts the pixel stream back afterwards, so map data
// can be fetched in the middle of a frame.
//
// There is no DMA and no queue: with one bus there is nothing to overlap, so
// the asynchronous API completes synchronously, exactly as the upstream
// NOR_POLLED build does.

#include "lcd.h"
#include "nor.h"
#include "platform.h"
#include "py32_hw.h"

volatile struct nor_stats nor_stats;

static uint32_t jedec_id;

// PB5 is MOSI. Taking it off the pad for the data phase is what makes a
// three-wire read work; the pull-up on the net holds the line meanwhile.
#define MOSI_MODER_SHIFT (5u * 2u)  /* PB5 */

static inline void mosi_drive(void)
{
    GPIO_MODER(GPIOB) = (GPIO_MODER(GPIOB) & ~(3u << MOSI_MODER_SHIFT)) |
                        (2u << MOSI_MODER_SHIFT);
}

static inline void mosi_release(void)
{
    GPIO_MODER(GPIOB) &= ~(3u << MOSI_MODER_SHIFT);
}

static int bus_taken;

static void nor_begin(void)
{
    bus_taken = lcd_bus_release();
    mosi_drive();
    // The panel is written without ever reading, so its bursts leave the
    // receive FIFO full. Draining costs a poll loop, so only do it when
    // there is something to drain.
    if (SPI_SR & (SPI_SR_FRLVL | SPI_SR_FTLVL | SPI_SR_BSY))
        py32_spi_drain();
    NOR_CS_LO();
}

static void nor_end(void)
{
    py32_spi_idle();
    NOR_CS_HI();
    mosi_drive();
    bus_taken = 0;
}

// Command and address go straight into the transmit FIFO. Waiting for each
// byte to come back first, as a general purpose transfer must, more than
// doubled the cost of a short read.
static inline void nor_cmd(uint8_t op, uint32_t addr)
{
    py32_spi_tx(op);
    py32_spi_tx((uint8_t)(addr >> 16));
    py32_spi_tx((uint8_t)(addr >> 8));
    py32_spi_tx((uint8_t)addr);
}

// Command and address are out, the answer is next: empty the receive FIFO of
// the bytes the transmit phase shifted in, then let the line go.
static inline void nor_turnaround(void)
{
    py32_spi_idle();
    py32_spi_drain();
    mosi_release();
}

static uint8_t xfer(uint8_t b)
{
    uint32_t guard = 200000u;
    while (!(SPI_SR & SPI_SR_TXE) && --guard) {
    }
    SPI_DR8 = b;
    guard = 200000u;
    while (!(SPI_SR & SPI_SR_RXNE) && --guard) {
    }
    return SPI_DR8;
}

// Keep several bytes in flight instead of waiting after each one. A strict
// send-wait-read leaves the FIFO idle and costs about two thirds of the
// achievable rate (measured: 1,624 kB/s against 3,091 kB/s).
#define INFLIGHT 4u

static void pump_in(uint8_t *buf, uint32_t len)
{
    uint32_t sent = 0, got = 0, guard = 0;
    while (got < len) {
        while (sent < len && sent - got < INFLIGHT && (SPI_SR & SPI_SR_TXE)) {
            SPI_DR8 = 0xFFu;
            sent++;
        }
        if (SPI_SR & SPI_SR_RXNE) {
            buf[got++] = SPI_DR8;
            guard = 0;
        } else if (++guard > 2000000u) {
            break;
        }
    }
}

uint32_t nor_jedec(void) { return jedec_id; }

uint8_t nor_status(uint8_t cmd)
{
    uint8_t s;
    nor_begin();
    (void)xfer(cmd);
    nor_turnaround();
    s = xfer(0xFFu);
    nor_end();
    return s;
}

static int wait_wip(uint32_t ms)
{
    uint32_t t0 = plat_ms();
    while (nor_status(0x05u) & 1u) {
        if (plat_ms() - t0 > ms)
            return -1;
    }
    return 0;
}

void nor_read(uint32_t phys, void *dst, uint32_t n)
{
    uint32_t t0 = DWT_CYCCNT;
    if (!n)
        return;
    nor_stats.sync++;
    nor_stats.bytes += n;
    nor_begin();
    nor_cmd(0x03u, phys);
    nor_turnaround();
    pump_in(dst, n);
    nor_end();
    dbg_nor(DWT_CYCCNT - t0, n);
}

// One bus, so nothing can run in the background: the asynchronous calls do
// the work now and hand back a ticket that is already done.
int nor_read_async(uint32_t phys, void *dst, uint32_t n, int prio)
{
    (void)prio;
    nor_stats.async++;
    nor_read(phys, dst, n);
    return 1;
}

bool nor_done(int ticket)
{
    (void)ticket;
    return true;
}

void nor_wait(int ticket) { (void)ticket; }
void nor_wait_all(void) {}

int nor_col_start(uint32_t phys, void *dst, uint32_t n)
{
    if (!n || n > 256u)
        return -1;
    nor_read(phys, dst, n);
    return 0;
}

void nor_col_wait(void) {}

static int write_enable(void)
{
    nor_begin();
    (void)xfer(0x06u);
    nor_end();
    return (nor_status(0x05u) & 2u) ? 0 : -1;
}

int nor_erase(uint32_t phys, unsigned cmd)
{
    if (write_enable())
        return -1;
    nor_begin();
    (void)xfer((uint8_t)cmd);
    (void)xfer((uint8_t)(phys >> 16));
    (void)xfer((uint8_t)(phys >> 8));
    (void)xfer((uint8_t)phys);
    nor_end();
    return wait_wip(3000u);
}

int nor_program(uint32_t phys, const void *src, uint32_t n)
{
    const uint8_t *p = src;
    while (n) {
        uint32_t room = 256u - (phys & 0xFFu);
        uint32_t k = n < room ? n : room;
        if (write_enable())
            return -1;
        nor_begin();
        (void)xfer(0x02u);
        (void)xfer((uint8_t)(phys >> 16));
        (void)xfer((uint8_t)(phys >> 8));
        (void)xfer((uint8_t)phys);
        for (uint32_t i = 0; i < k; i++)
            (void)xfer(p[i]);
        nor_end();
        if (wait_wip(500u))
            return -1;
        phys += k;
        p += k;
        n -= k;
    }
    return 0;
}

void nor_recover(void)
{
    NOR_CS_HI();
    mosi_drive();
    py32_spi_init();
    py32_spi_drain();
    bus_taken = 0;
}

int nor_read16(uint32_t phys, void *dst, uint32_t n)
{
    (void)phys;
    (void)dst;
    (void)n;
    return -1;  // 16-bit frame reads are not used and not implemented here
}

int nor_init(void)
{
    uint8_t a, b, c;
    py32_spi_init();
    NOR_CS_HI();
    LCD_CS_HI();
    mosi_drive();

    // Release from deep power down, in case the stock firmware left it there.
    nor_begin();
    (void)xfer(0xABu);
    nor_end();
    plat_delay_ms(1);

    nor_begin();
    (void)xfer(0x9Fu);
    nor_turnaround();
    a = xfer(0xFFu);
    b = xfer(0xFFu);
    c = xfer(0xFFu);
    nor_end();
    jedec_id = ((uint32_t)a << 16) | ((uint32_t)b << 8) | c;
    if (jedec_id == 0 || jedec_id == 0xFFFFFFu)
        return -1;
    return 0;
}
