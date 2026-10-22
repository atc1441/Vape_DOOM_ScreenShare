// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek

#include "host_stubs.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lcd.h"
#include "nor.h"
#include "ovl.h"
#include "platform.h"

uint8_t *nor_mem;
unsigned nor_reads, failures;
jmp_buf panic_jmp;
int panic_armed;
char panic_msg[64];

void nor_read(uint32_t phys, void *dst, uint32_t n)
{
    nor_reads++;
    if (phys > NOR_SIZE || n > NOR_SIZE - phys) {
        fprintf(stderr, "nor_read out of range %#x + %u\n", phys, n);
        exit(1);
    }
    memcpy(dst, nor_mem + phys, n);
}

// The root directory rename of fat12.c, which these tests never call
int nor_erase(uint32_t phys, unsigned cmd)
{
    if (cmd != NOR_ERASE_4K || phys & 0xFFFu || phys >= NOR_SIZE)
        return -1;
    memset(nor_mem + phys, 0xFF, 4096);
    return 0;
}

int nor_program(uint32_t phys, const void *src, uint32_t n)
{
    const uint8_t *s = src;
    if (phys > NOR_SIZE || n > NOR_SIZE - phys)
        return -1;
    for (uint32_t i = 0; i < n; i++)
        nor_mem[phys + i] &= s[i];
    return 0;
}

void nor_wait_all(void) {}
void pump_sync(int max) { (void)max; }

int plat_vfmt(char *out, int cap, const char *f, va_list ap) { return vsnprintf(out, (size_t)cap, f, ap); }

int plat_fmt(char *out, int cap, const char *f, ...)
{
    va_list ap;
    va_start(ap, f);
    int n = vsnprintf(out, (size_t)cap, f, ap);
    va_end(ap);
    return n;
}

int lcd_text(int x, int y, const char *s, uint16_t fg, uint16_t bg, int scale)
{
    (void)y;
    (void)fg;
    (void)bg;
    return x + 8 * scale * (int)strlen(s);
}

void plat_panic(const char *msg)
{
    snprintf(panic_msg, sizeof(panic_msg), "%s", msg);
    if (panic_armed) {
        panic_armed = 0;
        longjmp(panic_jmp, 1);
    }
    fprintf(stderr, "unexpected panic: %s\n", msg);
    exit(2);
}

void check(int ok, const char *what)
{
    printf("  %s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok)
        failures++;
}

uint32_t rd32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

static int hexval(int c) { return c <= '9' ? c - '0' : (c | 0x20) - 'a' + 10; }

void stub_setup(int argc, char **argv)
{
    const char *nor_path = NULL, *id_hex = NULL;
    for (int i = 1; i + 1 < argc; i += 2) {
        if (!strcmp(argv[i], "--nor"))
            nor_path = argv[i + 1];
        else if (!strcmp(argv[i], "--build-id"))
            id_hex = argv[i + 1];
        else if (!strcmp(argv[i], "--vma"))
            ovl_host_vma = (uint32_t)strtoul(argv[i + 1], NULL, 16);
    }
    if (!nor_path || !id_hex || strlen(id_hex) != 40 || !ovl_host_vma) {
        fprintf(stderr, "usage: %s --nor FILE --build-id HEX40 --vma HEX\n", argv[0]);
        exit(1);
    }
    for (int i = 0; i < 20; i++)
        dmc_build_id[i] = (uint8_t)(hexval(id_hex[2 * i]) << 4 | hexval(id_hex[2 * i + 1]));
    nor_mem = malloc(NOR_SIZE);
    FILE *f = fopen(nor_path, "rb");
    if (!nor_mem || !f || fread(nor_mem, 1, NOR_SIZE, f) != NOR_SIZE) {
        fprintf(stderr, "cannot read %s\n", nor_path);
        exit(1);
    }
    fclose(f);
}
