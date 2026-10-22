// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Data layer test on host NOR images built by tests/host/mknor.py.
//
// usage: test_data PACK_DIR NOR_IMAGE [EXPECT [CODE_FILE]]
//   EXPECT is the img_init result the image must give (default 0). With 0 the
//   test also checks img_verify, logical reads against the pack files, the
//   record cache, every map directory, texture, patch and flat reads (with
//   async reads completing at once and as late as the device may), the
//   ordering of async reads, flat prefetches, the NOR regions of both FAT12
//   volumes, and that damaged headers and sections are detected. With
//   CODE_FILE, DOOMCODE.DMC must be found and hold exactly that file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache.h"
#include "crc32.h"
#include "extents.h"
#include "fat12.h"
#include "fetch.h"
#include "img.h"
#include "nor.h"
#include "nor_file.h"
#include "rec.h"

static int fails;

#define CHECK(c, ...)                                   \
    do {                                                \
        if (!(c)) {                                     \
            fails++;                                    \
            if (fails < 20) {                           \
                printf("  FAIL %s:%d: ", __FILE__, __LINE__); \
                printf(__VA_ARGS__);                    \
                printf("\n");                           \
            }                                           \
        }                                               \
    } while (0)

void dmc_fatal(const char *msg)
{
    printf("dmc_fatal: %s\n", msg);
    exit(1);
}

static uint8_t *logical;
static uint32_t logical_size;

static uint8_t *slurp(const char *dir, const char *name, uint32_t *size)
{
    char path[1024];
    snprintf(path, sizeof(path), "%s%s%s", dir, name ? "/" : "", name ? name : "");
    FILE *f = fopen(path, "rb");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *p = malloc((size_t)n + 1);
    if (fread(p, 1, (size_t)n, f) != (size_t)n)
        n = 0;
    fclose(f);
    *size = (uint32_t)n;
    return p;
}

static uint32_t rnd_state = 12345;
static uint32_t rnd(void)
{
    rnd_state ^= rnd_state << 13;
    rnd_state ^= rnd_state >> 17;
    rnd_state ^= rnd_state << 5;
    return rnd_state;
}

static void test_reads(void)
{
    uint8_t buf[4096];
    for (int i = 0; i < 20000; i++) {
        uint32_t n = 1 + rnd() % (i & 1 ? 16 : 4096);
        uint32_t o = rnd() % (logical_size - n);
        // reads across the DOOM.DAT/DOOMX.DAT split
        if (i % 7 == 0 && img.split < img.size)
            o = img.split - n / 2;
        img_read(o, buf, n);
        CHECK(!memcmp(buf, logical + o, n), "img_read %u+%u", o, n);
    }
    uint64_t tx0 = nor_count.tx;
    rc_reset();
    rc_hits = rc_misses = 0;
    for (int i = 0; i < 50000; i++) {
        uint32_t n = 1 + rnd() % 16;
        // hot spot of 1 KB plus random far reads
        uint32_t o = i % 3 ? 100000 + rnd() % 1024 : rnd() % (logical_size - n);
        if (o + n > logical_size)
            o = logical_size - n;
        rec_get(o, buf, n);
        CHECK(!memcmp(buf, logical + o, n), "rec_get %u+%u", o, n);
    }
    printf("  rec_get: %u hits, %u misses, %llu NOR transactions\n", rc_hits, rc_misses,
           (unsigned long long)(nor_count.tx - tx0));
    CHECK(rc_hits > rc_misses, "cache does not hit");
    // LRU order inside one set: 4 lines of set 0 stay, a fifth evicts the oldest
    rc_reset();
    rc_hits = rc_misses = 0;
    for (int k = 0; k < 4; k++)
        rec_get(16u * 32u * k, buf, 1);
    rec_get(0, buf, 1);              // line 0 becomes most recent
    rec_get(16u * 32u * 4, buf, 1);  // evicts line 1
    rec_get(0, buf, 1);
    rec_get(16u * 32u * 2, buf, 1);
    rec_get(16u * 32u * 1, buf, 1);
    CHECK(rc_hits == 3 && rc_misses == 6, "LRU: %u hits %u misses", rc_hits, rc_misses);
}

static uint32_t le32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static void test_maps(void)
{
    for (int k = 0; k < img.nmaps; k++) {
        dmc_mapent_t e;
        img_mapent(k, &e);
        CHECK(map_find(e.episode, e.map) == k, "map_find E%dM%d", e.episode, e.map);
        map_load(k, 2);
        const dmc_maphdr_t *h = (const dmc_maphdr_t *)(logical + e.off);
        CHECK(M.nlines == h->nlines && M.nsectors == h->nsectors && M.nnodes == h->nnodes
                  && M.nsegs == h->nsegs && M.root == h->root && M.bmap_cols == h->bmap_cols,
              "%s counts", e.name);
        for (unsigned i = 0; i < M.nlines; i += 7) {
            dmc_line_t l;
            map_line((lineidx_t)i, &l);
            CHECK(!memcmp(&l, logical + e.off + h->stream_off[DMC_ST_LINES] + 16 * i, 16), "line");
        }
        for (unsigned s = 0; s < M.nssectors; s += 5) {
            dmc_seg_t run[64];
            segidx_t first;
            secidx_t sec;
            int n = map_seg_run((ssecidx_t)s, run, 64, &first, &sec);
            CHECK(!memcmp(run, logical + e.off + h->stream_off[DMC_ST_SEGS] + 16 * first, 16u * n),
                  "seg run");
        }
        // limits the engine relies on: seg runs fit its buffer, and no line
        // uses specials 30 and 96 (raiseToTexture needs the height of vanilla
        // texture 0 for a missing lower texture, the image does not have it)
        for (unsigned s = 0; s < M.nssectors; s++) {
            dmc_ssector_t ss;
            map_ssector((ssecidx_t)s, &ss);
            CHECK(ss.numsegs <= MAP_SEGRUN_MAX, "%s subsector %u: %u segs", e.name, s, ss.numsegs);
        }
        for (unsigned i = 0; i < M.nlines; i++) {
            dmc_line_t l;
            map_line((lineidx_t)i, &l);
            CHECK(l.special != 30 && l.special != 96, "%s line %u: special %u", e.name, i, l.special);
        }
        // tag values: index 0 is 0, the rest ascending
        unsigned prev = 0;
        for (unsigned t = 1; t <= M.ntags; t++) {
            unsigned v = map_tagval(t);
            CHECK(v > prev && map_tagindex((int)v) == t, "tag %u", t);
            prev = v;
        }
        CHECK(memcmp(e.name, "E", 1) == 0, "map name");
    }
}

static void test_fetch(void)
{
    uint8_t buf[FETCH_COLBUF], ref[FETCH_COLBUF];
    uint32_t texdata = img.sect[DMC_SECT_TEXDATA].off;
    int ntex = tex_count();
    int checked = 0, reused = 0;
    fetch_reset();
    for (int i = 0; i < 4000; i++) {
        int t = (int)(rnd() % (uint32_t)ntex);
        dmc_texent_t e;
        tex_entry(t, &e);
        if (!e.nlevels)
            continue;
        int L = tex_base(&e);
        int m = L + (int)(rnd() % e.nlevels);
        unsigned n = 1u << (e.log2_sh - m);
        int col = (int)(rnd() % 512);
        int y0 = (int)(rnd() % n), y1 = (int)(rnd() % n);
        if (i % 3 == 0)
            y0 = 0, y1 = (int)n - 1;
        memset(buf, 0xAA, sizeof(buf));
        int tk = tex_col_request(t, col, m, y0, y1, buf);
        if (!tk) {
            reused++;
            continue;
        }
        fetch_wait(tk);
        // reference column from the logical image
        uint32_t off = texdata + e.data;
        for (int k = L; k < m; k++)
            off += 1u << (e.log2_w - k + e.log2_sh - k);
        unsigned c = (unsigned)col & ((1u << (e.log2_w - m)) - 1);
        off += c * n;
        for (unsigned y = 0; y < n; y++) {
            int want = y1 >= y0 ? (int)y >= y0 && (int)y <= y1 : (int)y >= y0 || (int)y <= y1;
            if (want)
                CHECK(buf[y] == logical[off + y], "tex %d col %d row %u", t, col, y);
        }
        checked++;
        // the same request again is served from the buffer
        CHECK(tex_col_request(t, col, m, y0, y1, buf) == 0, "no reuse");
    }
    printf("  texture columns: %d checked, %d reused\n", checked, reused);

    // masked columns and patches against the post lists in the image
    int masked = 0;
    for (int t = 0; t < ntex; t++) {
        dmc_texent_t e;
        tex_entry(t, &e);
        if (!(e.flags & DMC_TF_MASKED))
            continue;
        for (int col = 0; col < (1 << e.log2_w); col += 3) {
            int len;
            fetch_wait(tex_masked_request(t, col, buf, &len));
            uint32_t ci = le32(logical + texdata + e.colinfo + 4 * ((col >> tex_base(&e))));
            CHECK(len == (int)DMC_COLINFO_LEN(ci), "masked len");
            CHECK(!memcmp(buf, logical + texdata + e.data + DMC_COLINFO_OFF(ci), (size_t)len), "masked");
            CHECK(buf[len - 1] == 0xFF, "masked terminator");
            masked++;
        }
    }
    uint32_t pdir = img.sect[DMC_SECT_PATCHDIR].off, pdata = img.sect[DMC_SECT_PATCHDATA].off;
    unsigned npatch = le32(logical + pdir);
    int pcols = 0;
    for (unsigned p = 0; p < npatch; p += 3) {
        dmc_patch_t h;
        patch_header((int)p, &h);
        uint32_t po = pdata + le32(logical + pdir + 4 + 4 * p);
        CHECK(!memcmp(&h, logical + po, 16), "patch header %u", p);
        for (int col = 0; col < h.width; col += 5) {
            int len;
            fetch_wait(spr_col_request((int)p, col, buf, &len));
            uint32_t ci = le32(logical + po + h.colinfo + 4 * (col >> h.xshift));
            memcpy(ref, logical + po + DMC_COLINFO_OFF(ci), DMC_COLINFO_LEN(ci));
            CHECK(len == (int)DMC_COLINFO_LEN(ci) && !memcmp(buf, ref, (size_t)len), "patch %u col %d", p, col);
            pcols++;
        }
    }
    printf("  masked texture columns %d, patch columns %d\n", masked, pcols);

    uint32_t fl = img.sect[DMC_SECT_FLATS].off;
    flat_set_slots(3);
    int nflat = flat_count();
    uint64_t tx0 = nor_count.tx;
    for (int i = 0; i < 300; i++) {
        int f = (int)(rnd() % (uint32_t)(nflat < 5 ? nflat : 5));
        const uint8_t *d = flat_get(f);
        CHECK(!memcmp(d, logical + fl + 1024u * f, 1024), "flat %d", f);
        CHECK(flat_avg(f) == logical[fl + 1024u * nflat + f], "flat avg %d", f);
    }
    printf("  flats: %llu NOR transactions for 300 gets over 5 flats in 3 slots\n",
           (unsigned long long)(nor_count.tx - tx0));
    char s[256];
    uint32_t st = img.sect[DMC_SECT_STRINGS].off;
    for (int i = 0; i < DMC_NSTRINGS; i++) {
        str_get(i, s, sizeof(s));
        const char *want = (const char *)logical + st + le32(logical + st + 4 + 4 * i);
        CHECK(!strncmp(s, want, sizeof(s) - 1), "string %d", i);
    }
    printf("  level name: %s\n", str_get(DMC_STR_HUSTR_E1M1, s, sizeof(s)));
}

// A texture with a full column at its base level, -1 when there is none.
static int full_texture(dmc_texent_t *e)
{
    for (int t = 0; t < tex_count(); t++) {
        tex_entry(t, e);
        if (e->nlevels && !(e->flags & DMC_TF_MASKED))
            return t;
    }
    return -1;
}

static int flat_is(const uint8_t *d, int f)
{
    return !memcmp(d, logical + img.sect[DMC_SECT_FLATS].off + DMC_FLAT_SIZE * (uint32_t)f,
                   DMC_FLAT_SIZE);
}

// Async reads that complete only when waited for: a buffer is never handed
// out as ready while its read is pending, prefetched flats survive until
// they are used, and a prefetch never takes the flat in use.
static void test_async(void)
{
    uint8_t buf[FETCH_COLBUF];
    dmc_texent_t e;
    nor_file_defer(1);
    fetch_reset();

    int t = full_texture(&e);
    CHECK(t >= 0, "no opaque texture");
    if (t >= 0) {
        int L = tex_base(&e);
        unsigned n = 1u << (e.log2_sh - L);
        uint32_t off = img.sect[DMC_SECT_TEXDATA].off + e.data;
        int tk = tex_col_request(t, 0, L, 0, (int)n - 1, buf);
        CHECK(tk && !nor_done(tk), "column read done before it was waited for");
        int tk2 = tex_col_request(t, 0, L, 0, (int)n - 1, buf);
        CHECK(tk2 == tk, "same column while pending: ticket %d, pending %d", tk2, tk);
        fetch_wait(tk2);
        CHECK(!memcmp(buf, logical + off, n), "column after the wait");
        CHECK(tex_col_request(t, 0, L, 0, (int)n - 1, buf) == 0, "column read done, ticket not 0");
        // rows inside the pending range of another column
        unsigned c1 = 1u & ((1u << (e.log2_w - L)) - 1u);
        fetch_reset();
        tk = tex_col_request(t, 1, L, 0, (int)n - 1, buf);
        tk2 = tex_col_request(t, 1, L, 1, 2, buf);
        CHECK(tk2 == tk, "rows of a pending column: ticket %d, pending %d", tk2, tk);
        fetch_wait(tk2);
        CHECK(!memcmp(buf, logical + off + n * c1, n), "second column after the wait");
    }
    int len, len2;
    int tk = spr_col_request(0, 0, buf, &len);
    int tk2 = spr_col_request(0, 0, buf, &len2);
    CHECK(tk && tk2 == tk && len == len2, "patch column while pending: %d %d", tk, tk2);
    fetch_wait(tk2);
    CHECK(buf[len - 1] == 0xFF, "patch column after the wait");

    // three prefetches in three slots, then each get is a hit
    uint32_t chunks = DMC_FLAT_SIZE / 256;
    flat_set_slots(3);
    uint64_t tx0 = nor_count.tx;
    flat_prefetch(0);
    flat_prefetch(1);
    flat_prefetch(2);
    uint64_t tx1 = nor_count.tx;
    CHECK(tx1 - tx0 == 3 * chunks, "3 prefetches: %llu flat loads", (unsigned long long)((tx1 - tx0) / chunks));
    for (int f = 0; f < 3; f++)
        CHECK(flat_is(flat_get(f), f), "prefetched flat %d", f);
    CHECK(nor_count.tx == tx1, "prefetched flats loaded again: %llu loads",
          (unsigned long long)((nor_count.tx - tx1) / chunks));

    // two slots: one prefetch fits beside the flat in use, a second does not
    flat_set_slots(2);
    const uint8_t *d = flat_get(0);
    tx0 = nor_count.tx;
    flat_prefetch(1);
    flat_prefetch(2);
    nor_wait_all();
    CHECK(nor_count.tx - tx0 == chunks, "2 slots: %llu prefetch loads", (unsigned long long)((nor_count.tx - tx0) / chunks));
    CHECK(flat_is(d, 0), "prefetch overwrote the flat in use");
    tx0 = nor_count.tx;
    CHECK(flat_is(flat_get(1), 1) && nor_count.tx == tx0, "prefetched flat 1 with 2 slots");

    // one slot: prefetch does nothing
    flat_set_slots(1);
    d = flat_get(0);
    tx0 = nor_count.tx;
    flat_prefetch(3);
    nor_wait_all();
    CHECK(nor_count.tx == tx0 && flat_is(d, 0), "prefetch with 1 slot");
    flat_set_slots(0);
    CHECK(flat_get(0) == NULL, "flat_get without slots");
    flat_set_slots(FLAT_SLOTS_MAX);
    nor_file_defer(0);
    printf("  async: deferred reads, reuse while pending and flat prefetch checked\n");
}

// A volume whose BPB claims one sector more than its NOR region holds is
// rejected: the firmware drive ends where the data drive starts, that one at 4 MB.
static void test_fat_bounds(void)
{
    static const uint32_t base[2] = { DMC_VOL_UPDATE, DMC_VOL_MAIN };
    static const uint32_t limit[2] = { DMC_VOL_MAIN, NOR_SIZE };
    uint8_t boot[4096], b[4096];
    for (int k = 0; k < 2; k++) {
        fat_vol_t v;
        nor_read(base[k], boot, sizeof(boot));
        unsigned bytes = boot[11] | boot[12] << 8, total = boot[19] | boot[20] << 8;
        CHECK(fat_mount(&v, base[k]) == FAT_OK, "volume at 0x%x", base[k]);
        memcpy(b, boot, sizeof(b));
        total = (limit[k] - base[k]) / bytes + 1;
        b[19] = (uint8_t)total;
        b[20] = (uint8_t)(total >> 8);
        nor_erase4k(base[k]);
        nor_program(base[k], b, sizeof(b));
        CHECK(fat_mount(&v, base[k]) == FAT_E_BPB, "volume at 0x%x past its region", base[k]);
        nor_erase4k(base[k]);
        nor_program(base[k], boot, sizeof(boot));
        CHECK(fat_mount(&v, base[k]) == FAT_OK, "volume at 0x%x restored", base[k]);
    }
}

static void test_damage(void)
{
    // flip bits in the header and in a section, through the NOR image
    uint32_t run;
    uint32_t phys = ext_phys(&dmc_files[DMC_F_DOOM], offsetof(dmc_header_t, conv_version), &run);
    nor_program(phys, "\x00", 1);
    CHECK(img_init() == IMG_E_CRC, "damaged header not detected");
}

static void test_damage_section(void)
{
    uint32_t run;
    uint32_t o = img.sect[DMC_SECT_PALETTE].off + 100;
    uint8_t b = 0;
    uint32_t phys = ext_phys(&dmc_files[DMC_F_DOOM], o, &run);
    nor_program(phys, &b, 1);
    CHECK(img_init() == IMG_OK, "section damage must not stop img_init");
    CHECK(img_verify() == IMG_E_CRC, "damaged section not detected");
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: test_data PACK_DIR NOR_IMAGE [EXPECT [CODE_FILE]]\n");
        return 2;
    }
    int expect = argc > 3 ? atoi(argv[3]) : IMG_OK;
    if (nor_file_open(argv[2])) {
        fprintf(stderr, "cannot read %s\n", argv[2]);
        return 2;
    }
    int r = img_init();
    printf("%s: img_init %d (expected %d)\n", argv[2], r, expect);
    if (r != expect)
        return 1;
    if (r != IMG_OK)
        return 0;
    printf("  extents: DOOM.DAT %d, DOOMX.DAT %d, SAVE.DAT %d, DOOMCODE.DMC %d\n",
           dmc_files[DMC_F_DOOM].next, dmc_files[DMC_F_DOOMX].next, dmc_files[DMC_F_SAVE].next,
           dmc_files[DMC_F_CODE].next);

    uint32_t nd, nx = 0;
    uint8_t *doom = slurp(argv[1], "DOOM.DAT", &nd);
    uint8_t *doomx = slurp(argv[1], "DOOMX.DAT", &nx);
    logical_size = img.size;
    logical = malloc(logical_size);
    memcpy(logical, doom, img.split);
    if (img.split < img.size)
        memcpy(logical + img.split, doomx + DMC_DOOMX_HDR, img.size - img.split);
    CHECK(crc32_buf("123456789", 9) == 0xCBF43926u, "crc32 check value");

    uint64_t tx0 = nor_count.tx;
    CHECK(img_verify() == IMG_OK, "img_verify");
    printf("  img_verify: %llu transactions, %llu bytes\n", (unsigned long long)(nor_count.tx - tx0),
           (unsigned long long)nor_count.bytes);
    if (argc > 4) {
        uint32_t n;
        uint8_t *want = slurp(argv[4], NULL, &n);
        const dmc_file_t *f = &dmc_files[DMC_F_CODE];
        uint8_t *got = malloc(n ? n : 1);
        CHECK(want && f->size == n && !ext_read(DMC_F_CODE, 0, got, n) && !memcmp(got, want, n),
              "DOOMCODE.DMC is not %s (%u B found)", argv[4], f->size);
        printf("  DOOMCODE.DMC: %u B in %d extents at NOR 0x%x, as expected\n", f->size, f->next,
               f->data);
        free(want);
        free(got);
    }
    test_reads();
    test_maps();
    test_fetch();
    // the fetchers again, with async reads that land only when waited for
    nor_file_defer(1);
    test_fetch();
    nor_file_defer(0);
    test_async();
    test_fat_bounds();
    test_damage_section();
    test_damage();
    free(doom);
    free(doomx);
    free(logical);
    printf("%s\n", fails ? "FAIL" : "PASS");
    return fails ? 1 : 0;
}
