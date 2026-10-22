// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nor.h"
#include "nor_file.h"

#define NOR_CHUNK 256u

volatile struct nor_stats nor_stats;
nor_count_t nor_count;

static uint8_t *nor_mem;
static int nor_ticket;

// Deferred completion: the queue of the device (4 requests, lowest prio
// value first, then the oldest). A request lands in its buffer only when it
// is waited for, or when the queue is full, and the buffer reads 0xA5 until
// then, so a buffer read before its ticket is done shows up.
#define NOR_QUEUE 4

typedef struct {
    int ticket, prio;
    uint32_t phys, n, seq;
    uint8_t *dst;
} nor_req_t;

static int nor_defer;
static nor_req_t nor_q[NOR_QUEUE];
static int nor_qn;
static uint32_t nor_seq;

// The direct column read of nor_col_start in deferred mode, n 0 when none:
// it lands only at nor_col_wait, nor_col_start, nor_wait_all, nor_erase
// and nor_program (the device also ends it at sync reads and queued
// requests).
static nor_req_t nor_col;
static unsigned nor_col_calls, nor_col_reject;

void nor_file_defer(int on)
{
    nor_wait_all();
    nor_defer = on;
}

void nor_file_col_reject(int k)
{
    nor_col_reject = k > 0 ? (unsigned)k : 0;
}

static void nor_complete_one(void)
{
    int b = 0;
    for (int i = 1; i < nor_qn; i++)
        if (nor_q[i].prio < nor_q[b].prio || (nor_q[i].prio == nor_q[b].prio && nor_q[i].seq < nor_q[b].seq))
            b = i;
    memcpy(nor_q[b].dst, nor_mem + nor_q[b].phys, nor_q[b].n);
    nor_q[b] = nor_q[--nor_qn];
}

static int nor_pending(int ticket)
{
    for (int i = 0; i < nor_qn; i++)
        if (nor_q[i].ticket == ticket)
            return 1;
    return 0;
}

int nor_file_open(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return -1;
    if (!nor_mem)
        nor_mem = malloc(NOR_SIZE);
    memset(nor_mem, 0xFF, NOR_SIZE);
    size_t n = fread(nor_mem, 1, NOR_SIZE, f);
    fclose(f);
    return n ? 0 : -1;
}

int nor_file_save(const char *path)
{
    FILE *f;
    int r;
    if (!nor_mem)
        return -1;
    nor_wait_all();
    f = fopen(path, "wb");
    if (!f)
        return -1;
    r = fwrite(nor_mem, 1, NOR_SIZE, f) == NOR_SIZE ? 0 : -1;
    if (fclose(f))
        r = -1;
    return r;
}

static void nor_check(uint32_t phys, uint32_t n)
{
    if (!nor_mem || phys > NOR_SIZE || n > NOR_SIZE - phys) {
        fprintf(stderr, "nor: access 0x%x+%u outside the image\n", phys, n);
        abort();
    }
}

int nor_init(void)
{
    return nor_mem ? 0 : -1;
}

uint32_t nor_jedec(void)
{
    return NOR_JEDEC_W25Q32;
}

uint8_t nor_status(uint8_t cmd)
{
    return cmd == 0x35 ? 0x02 : 0x00;
}

void nor_read(uint32_t phys, void *dst, uint32_t n)
{
    nor_check(phys, n);
    memcpy(dst, nor_mem + phys, n);
    nor_stats.sync++;
    nor_count.tx++;
    nor_count.bytes += n;
    nor_count.sync_tx++;
    nor_count.sync_bytes += n;
}

void nor_file_peek(uint32_t phys, void *dst, uint32_t n)
{
    nor_check(phys, n);
    memcpy(dst, nor_mem + phys, n);
}

int nor_read_async(uint32_t phys, void *dst, uint32_t n, int prio)
{
    nor_check(phys, n);
    uint32_t chunks = (n + NOR_CHUNK - 1) / NOR_CHUNK;
    nor_stats.async_chunks += chunks;
    nor_count.tx += chunks;
    nor_count.bytes += n;
    if (++nor_ticket <= 0)
        nor_ticket = 1;
    if (!nor_defer) {
        memcpy(dst, nor_mem + phys, n);
        return nor_ticket;
    }
    if (nor_qn == NOR_QUEUE)
        nor_complete_one();
    memset(dst, 0xA5, n);
    nor_q[nor_qn++] = (nor_req_t){ nor_ticket, prio, phys, n, nor_seq++, dst };
    return nor_ticket;
}

// A negative ticket is a direct read (the renderer's -1): the device returns
// at once for it, which would draw before the data arrived.
static void nor_ticket_check(int ticket)
{
    if (ticket < 0) {
        fprintf(stderr, "nor: wait for ticket %d\n", ticket);
        abort();
    }
}

// Polling does not advance the deferred queue, only waiting does.
bool nor_done(int ticket)
{
    nor_ticket_check(ticket);
    return !nor_pending(ticket);
}

void nor_wait(int ticket)
{
    nor_ticket_check(ticket);
    while (nor_pending(ticket))
        nor_complete_one();
}

void nor_col_wait(void)
{
    if (nor_col.n)
        memcpy(nor_col.dst, nor_mem + nor_col.phys, nor_col.n);
    nor_col.n = 0;
}

// Refused while deferred queued requests are pending (the device's queue
// owns the bus), for n 0 or over 256 and at every K-th call with
// --nor-col-reject K. Counted like a queued read of one chunk.
int nor_col_start(uint32_t phys, void *dst, uint32_t n)
{
    nor_col_wait();
    nor_col_calls++;
    if (nor_qn || n - 1 >= NOR_CHUNK || (nor_col_reject && nor_col_calls % nor_col_reject == 0))
        return -1;
    nor_check(phys, n);
    nor_stats.async_chunks++;
    nor_count.tx++;
    nor_count.bytes += n;
    if (!nor_defer) {
        memcpy(dst, nor_mem + phys, n);
        return 0;
    }
    memset(dst, 0xA5, n);
    nor_col = (nor_req_t){ 0, NOR_PRIO_COL, phys, n, 0, dst };
    return 0;
}

void nor_wait_all(void)
{
    nor_col_wait();
    while (nor_qn)
        nor_complete_one();
}

int nor_erase(uint32_t phys, unsigned cmd)
{
    uint32_t n = cmd == NOR_ERASE_64K ? 0x10000u : 0x1000u;
    if (cmd != NOR_ERASE_4K && cmd != NOR_ERASE_64K) {
        fprintf(stderr, "nor: erase command 0x%x\n", cmd);
        abort();
    }
    nor_col_wait();
    phys &= ~(n - 1);
    nor_check(phys, n);
    memset(nor_mem + phys, 0xFF, n);
    if (n == 0x1000u)
        nor_count.erase4k++;
    else
        nor_count.erase64k++;
    return 0;
}

int nor_program(uint32_t phys, const void *src, uint32_t n)
{
    const uint8_t *s = src;
    nor_col_wait();
    nor_check(phys, n);
    for (uint32_t i = 0; i < n; i++)
        nor_mem[phys + i] &= s[i];
    return 0;
}

int nor_read16(uint32_t phys, void *dst, uint32_t n)
{
    uint8_t *d = dst;
    if ((phys | n | (uintptr_t)dst) & 1 || !phys)
        return -1;
    nor_check(phys, n);
    for (uint32_t i = 0; i < n; i += 2) {
        d[i] = nor_mem[phys + i + 1];
        d[i + 1] = nor_mem[phys + i];
    }
    nor_count.tx++;
    nor_count.bytes += n;
    return 0;
}
