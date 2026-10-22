// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Settings page in internal flash (1 KB sector at 0x0801FC00):
// u32 magic, u16 version, u16 size, payload padded to 2 bytes, u32 CRC-32 of all before it.

#include <stddef.h>

#include "at32.h"
#include "platform.h"

#define PAGE    0x0801FC00u
#define MAGIC   0x53434D44u  // "DMCS"
#define HDR     8u

static uint32_t crc32(uint32_t crc, const uint8_t *p, uint32_t n)
{
    crc = ~crc;
    while (n--) {
        crc ^= *p++;
        for (int k = 0; k < 8; k++)
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

static uint32_t rd32(uint32_t a) { return REG32(a); }

int settings_load(void *dst, uint32_t size, uint16_t version)
{
    const uint8_t *page = (const uint8_t *)(uintptr_t)PAGE;
    uint32_t padded = (size + 1u) & ~1u;
    if (size > SETTINGS_MAX || rd32(PAGE) != MAGIC || REG16(PAGE + 4u) != version ||
        REG16(PAGE + 6u) != size)
        return -1;
    uint32_t crc_at = PAGE + HDR + padded;
    uint32_t stored = REG16(crc_at) | ((uint32_t)REG16(crc_at + 2u) << 16);
    if (crc32(0, page, HDR + padded) != stored)
        return -1;
    uint8_t *d = dst;
    for (uint32_t i = 0; i < size; i++)
        d[i] = page[HDR + i];
    return 0;
}

static int flash_wait(void)
{
    for (uint32_t n = 20000000u; n; n--) {
        if (!(FLASH_STS & 1u))
            return (FLASH_STS & 0x14u) ? -1 : 0;  // program or protection error
    }
    return -1;
}

static int prog16(uint32_t a, uint16_t v)
{
    FLASH_CTRL |= 1u;  // FPRGM
    REG16(a) = v;
    int err = flash_wait();
    FLASH_CTRL &= ~1u;
    return err || REG16(a) != v;
}

int settings_save(const void *src, uint32_t size, uint16_t version)
{
    if (size > SETTINGS_MAX)
        return -1;
    const uint8_t *s = src;
    uint32_t padded = (size + 1u) & ~1u;
    uint8_t hdr[HDR] = {(uint8_t)MAGIC, (uint8_t)(MAGIC >> 8), (uint8_t)(MAGIC >> 16),
                        (uint8_t)(MAGIC >> 24), (uint8_t)version, (uint8_t)(version >> 8),
                        (uint8_t)size, (uint8_t)(size >> 8)};
    uint32_t crc = crc32(0, hdr, HDR);
    crc = crc32(crc, s, size);
    if (padded != size)
        crc = crc32(crc, (const uint8_t *)"\xFF", 1);

    uint32_t m = irq_save();
    FLASH_UNLOCK = 0x45670123u;
    FLASH_UNLOCK = 0xCDEF89ABu;
    FLASH_STS = 0x34u;
    FLASH_CTRL |= 1u << 1;  // SECERS
    FLASH_ADDR = PAGE;
    FLASH_CTRL |= 1u << 6;  // ERSTR
    int err = flash_wait();
    FLASH_CTRL &= ~(1u << 1);
    for (uint32_t i = 0; !err && i < HDR; i += 2)
        err = prog16(PAGE + i, (uint16_t)(hdr[i] | (hdr[i + 1] << 8)));
    for (uint32_t i = 0; !err && i < padded; i += 2) {
        uint16_t lo = s[i], hi = i + 1u < size ? s[i + 1] : 0xFFu;
        err = prog16(PAGE + HDR + i, (uint16_t)(lo | (hi << 8)));
    }
    if (!err)
        err = prog16(PAGE + HDR + padded, (uint16_t)crc);
    if (!err)
        err = prog16(PAGE + HDR + padded + 2u, (uint16_t)(crc >> 16));
    FLASH_CTRL |= 1u << 7;  // OPLK
    irq_restore(m);
    return err ? -1 : 0;
}
