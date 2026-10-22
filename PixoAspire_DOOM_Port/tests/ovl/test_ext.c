// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Host test of the loader through the data layer (port/ovl/ovl_file.c on port/data/extents.c
// and fat12.c). The good pack lies in the NOR image as a file of 128 B clusters in reverse
// order with gaps between them (norimg.py), so header, directory and blob reads cross extent
// boundaries, and as DOOMCODE.DMC on a FAT12 volume that fat_mount and fat_open locate, as
// img_init does on the device. Also checks a missing and a truncated file.
//
// Usage: test_ext --nor FILE --build-id HEX40 --vma HEX

#include <stdio.h>
#include <stdlib.h>

#include "extents.h"
#include "fat12.h"
#include "host_stubs.h"
#include "ovl_demo.h"

uint32_t ovl_fixed_base;  // moved by demo_errors(), which this test does not run

static void report(const char *pass, uint32_t fail)
{
    static const char *const name[] = {
#define X(id, d) d,
        DEMO_CHECKS(X)
#undef X
    };
    char s[128];
    for (int k = 0; k < CHK_COUNT; k++) {
        snprintf(s, sizeof(s), "%s: %s", pass, name[k]);
        check(!(fail & 1u << k), s);
    }
}

int main(int argc, char **argv)
{
    char s[128];
    stub_setup(argc, argv);
    ovl_host_copy = 1;
    printf("loader on the data layer's extents\n");

    ext_reset();
    dmc_file_t *f = &dmc_files[DMC_F_CODE];
    f->data = DEMO_NOR_FRAG;
    f->cshift = DEMO_FRAG_SHIFT;
    check(ovl_load(OVL_TEST_A) == OVL_ENOFILE, "no DOOMCODE.DMC gives OVL_ENOFILE");
    f->size = 40;
    check(ext_add(f, 0, DEMO_FRAG_PCL0) == 0 && ovl_load(OVL_TEST_A) == OVL_EHEADER,
          "a file shorter than the header gives OVL_EHEADER");

    ext_reset();
    f->data = DEMO_NOR_FRAG;
    f->cshift = DEMO_FRAG_SHIFT;
    f->size = rd32(nor_mem + DEMO_NOR_GOOD + 32);
    unsigned n = (f->size + (1u << DEMO_FRAG_SHIFT) - 1) >> DEMO_FRAG_SHIFT, ok = 1;
    for (unsigned k = 0; k < n; k++)
        ok &= ext_add(f, (uint16_t)k, (uint16_t)(DEMO_FRAG_PCL0 - DEMO_FRAG_STEP * k)) == 0;
    snprintf(s, sizeof(s), "DOOMCODE.DMC (%u B) in %u extents", f->size, f->next);
    check(ok && f->next == n && n >= 3, s);

    report("fragmented", demo_calls());
    // The test is only worth something when the blobs straddle clusters
    const uint8_t *h = nor_mem + DEMO_NOR_GOOD;
    unsigned cross = 0, count = h[10] | h[11] << 8;
    for (unsigned i = 0; i < count; i++) {
        uint32_t off = rd32(h + 64 + 16 * i + 4), size = rd32(h + 64 + 16 * i + 8);
        cross += off >> DEMO_FRAG_SHIFT != (off + size - 1) >> DEMO_FRAG_SHIFT;
    }
    snprintf(s, sizeof(s), "%u of %u blobs cross an extent boundary", cross, count);
    check(count && cross == count, s);

    printf("DOOMCODE.DMC on a FAT12 volume (fat_mount, fat_open)\n");
    ext_reset();
    fat_vol_t v;
    int em = fat_mount(&v, DEMO_NOR_FAT);
    int eo = em ? em : fat_open(&v, "DOOMCODEDMC", f);
    snprintf(s, sizeof(s), "fat_open finds DOOMCODE.DMC: %u B in %u extents of %u B", f->size,
             f->next, 1u << f->cshift);
    check(!em && !eo && f->size == rd32(nor_mem + DEMO_NOR_GOOD + 32) &&
              f->next == (f->size + (1u << f->cshift) - 1) >> f->cshift, s);
    report("FAT12", demo_calls());

    free(nor_mem);
    printf(failures ? "%u FAILED\n" : "ALL PASSED\n", failures);
    return failures ? 1 : 0;
}
