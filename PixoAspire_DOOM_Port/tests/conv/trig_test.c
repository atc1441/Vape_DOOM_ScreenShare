// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Host test: every decoded table entry of port/math equals the upstream value,
// and FixedMulI (fixed.h) equals the FixedMul of the upstream m_fixed.c. The
// reference arrays come from build/conv/test/trig_ref.c.

#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include "fixed.h"
#include "trig.h"
#include "trig_tables.h"

extern const int ref_finesine[10240];
extern const int ref_finetangent[4096];
extern const uint32_t ref_tantoangle[2049];
extern const int ref_vatx[2][4096];
extern const uint32_t ref_xtva[2][321];
extern const int ref_viewwidth[2];
extern const int ref_yslope[200];
extern const int ref_scalelight[2][16 * 48];
extern const int ref_zlight[16 * 128];
extern const uint8_t ref_colormap[33 * 256];
int ref_FixedMul(int a, int b);

static long errors;

static void expect(int ok, const char *what, long i, long long got, long long want)
{
    if (!ok && errors++ < 20)
        printf("MISMATCH %s[%ld]: got %lld want %lld\n", what, i, got, want);
}

static uint64_t rng_state = 0x9E3779B97F4A7C15ull;

static uint64_t rnd(void)
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 7;
    rng_state ^= rng_state << 17;
    return rng_state;
}

static int32_t rnd_int(void)
{
    uint32_t v = (uint32_t)rnd();
    int bits = (int)(rnd() % 33);
    v = bits == 32 ? v : (v & ((1u << bits) - 1u));
    return (rnd() & 1) ? -(int32_t)v : (int32_t)v;
}

int main(void)
{
    long n = 0;
    for (unsigned i = 0; i < 10240; i++, n++)
        expect(dmc_finesine(i) == ref_finesine[i], "finesine", i, dmc_finesine(i), ref_finesine[i]);
    for (unsigned i = 0; i < 8192; i++, n++)
        expect(dmc_finecosine(i) == ref_finesine[i + 2048], "finecosine", i, dmc_finecosine(i),
               ref_finesine[i + 2048]);
    for (unsigned i = 0; i < 4096; i++, n++)
        expect(dmc_finetangent(i) == ref_finetangent[i], "finetangent", i, dmc_finetangent(i),
               ref_finetangent[i]);
    for (unsigned i = 0; i <= 2048; i++, n++)
        expect(dmc_tantoangle(i) == ref_tantoangle[i], "tantoangle", i, dmc_tantoangle(i), ref_tantoangle[i]);
    for (int d = 0; d < 2; d++) {
        for (unsigned i = 0; i < 4096; i++, n++)
            expect(dmc_viewangletox(d, i) == ref_vatx[d][i], "viewangletox", d * 4096 + i,
                   dmc_viewangletox(d, i), ref_vatx[d][i]);
        for (int x = 0; x <= ref_viewwidth[d]; x++, n++)
            expect(dmc_xtoviewangle(d, (unsigned)x) == ref_xtva[d][x], "xtoviewangle", d * 1000 + x,
                   dmc_xtoviewangle(d, (unsigned)x), ref_xtva[d][x]);
        for (unsigned i = 0; i < 16; i++)
            for (unsigned j = 0; j < 48; j++, n++)
                expect(dmc_scalelight(i, j) == ref_scalelight[d][i * 48 + j], "scalelight", (long)(i * 48 + j),
                       dmc_scalelight(i, j), ref_scalelight[d][i * 48 + j]);
    }
    for (unsigned y = 0; y < 200; y++, n++)
        expect(dmc_yslope(y) == ref_yslope[y], "yslope", y, dmc_yslope(y), ref_yslope[y]);
    for (unsigned i = 0; i < 16; i++)
        for (unsigned j = 0; j < 128; j++, n++)
            expect(dmc_zlight(i, j) == ref_zlight[i * 128 + j], "zlight", (long)(i * 128 + j), dmc_zlight(i, j),
                   ref_zlight[i * 128 + j]);
    for (unsigned r = 0; r < 33; r++)
        for (unsigned c = 0; c < 256; c++, n++)
            expect(dmc_colormap_row(r)[c] == ref_colormap[r * 256 + c], "colormap", (long)(r * 256 + c),
                   dmc_colormap_row(r)[c], ref_colormap[r * 256 + c]);
    printf("tables: %ld entries checked\n", n);

    static const int32_t edge[] = {0, 1, -1, 2, -2, 3, 16383, 16384, -16384, 32767, 32768, -32768, 65535,
                                   65536, -65536, 65537, 0x3FFFFFFF, 0x40000000, -0x40000000, INT_MAX,
                                   INT_MIN, INT_MIN + 1, 0x12345678, -0x12345678, 0x7FFF0000, 1 << 24};
    long nd = 0;
    int ne = (int)(sizeof(edge) / sizeof(edge[0]));
    for (int i = 0; i < ne; i++)
        for (int j = 0; j < ne; j++, nd++) {
            int a = edge[i], b = edge[j];
            expect(FixedMulI(a, b) == ref_FixedMul(a, b), "FixedMulI edge", i * 100 + j, FixedMulI(a, b),
                   ref_FixedMul(a, b));
        }
    for (long k = 0; k < 20000000; k++, nd++) {
        int a = rnd_int(), b = rnd_int();
        if (FixedMulI(a, b) != ref_FixedMul(a, b))
            expect(0, "FixedMulI", k, FixedMulI(a, b), ref_FixedMul(a, b));
    }
    printf("FixedMulI: %ld pairs checked against upstream m_fixed.c\n", nd);
    printf("%s: %ld errors\n", errors ? "FAIL" : "PASS", errors);
    return errors ? 1 : 0;
}
