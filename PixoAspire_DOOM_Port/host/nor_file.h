// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Host NOR: platform/nor.h on a 4 MB image file held in memory, with
// transaction and byte counters. A sync read is one transaction, an async
// read is one transaction per 256 B chunk as on the device, a direct column
// read one. Async and direct reads complete at once, or in deferred mode as
// late as the device may.

#ifndef DMC_NOR_FILE_H
#define DMC_NOR_FILE_H

#include <stdint.h>

typedef struct {
    uint64_t tx, bytes;            // all reads
    uint64_t sync_tx, sync_bytes;  // nor_read
    uint64_t erase4k, erase64k;    // nor_erase of a sector and of a block
} nor_count_t;

extern nor_count_t nor_count;

// Loads the image. Returns 0, or -1 when the file cannot be read.
int nor_file_open(const char *path);

// Writes the image as it is now, erases and programs included. Returns 0,
// or -1 when the file cannot be written.
int nor_file_save(const char *path);

// 1: async reads complete only when waited for (or when the 4-entry queue
// is full), in device order, and their buffers read 0xA5 until then. A
// direct column read (nor_col_start) completes only at the calls that end
// it explicitly (nor_col_wait, the next nor_col_start, nor_wait_all, erase
// and program), and it is refused while queued requests are pending.
// 0 (default): async and direct reads complete at once.
void nor_file_defer(int on);

// k > 0: every k-th nor_col_start is refused, so the caller's queued
// fallback runs (--nor-col-reject).
void nor_file_col_reject(int k);

// The n bytes at phys, without counting a transaction (self checks).
void nor_file_peek(uint32_t phys, void *dst, uint32_t n);

#endif
