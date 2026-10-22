// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "png.h"

static uint32_t crc_tab[256];

static void crc_init(void)
{
    if (crc_tab[1])
        return;
    for (uint32_t n = 0; n < 256; n++) {
        uint32_t c = n;
        for (int k = 0; k < 8; k++)
            c = (c & 1) ? 0xedb88320u ^ (c >> 1) : c >> 1;
        crc_tab[n] = c;
    }
}

static uint32_t crc_upd(uint32_t c, const uint8_t *p, size_t n)
{
    while (n--)
        c = crc_tab[(c ^ *p++) & 0xff] ^ (c >> 8);
    return c;
}

static void be32(uint8_t *p, uint32_t v)
{
    p[0] = v >> 24;
    p[1] = v >> 16;
    p[2] = v >> 8;
    p[3] = v;
}

static int chunk(FILE *f, const char *type, const uint8_t *data, uint32_t len)
{
    uint8_t hdr[8];
    be32(hdr, len);
    memcpy(hdr + 4, type, 4);
    uint32_t c = crc_upd(0xffffffffu, hdr + 4, 4);
    c = crc_upd(c, data, len) ^ 0xffffffffu;
    uint8_t tail[4];
    be32(tail, c);
    return fwrite(hdr, 1, 8, f) != 8 || (len && fwrite(data, 1, len, f) != len)
        || fwrite(tail, 1, 4, f) != 4;
}

int png_write_indexed(const char *path, int w, int h, const uint8_t *pix, const uint8_t *pal)
{
    static const uint8_t sig[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n' };
    size_t raw_len = (size_t)(w + 1) * h;
    size_t nblk = (raw_len + 65534) / 65535;
    size_t z_len = 2 + raw_len + 5 * nblk + 4;
    uint8_t *z = malloc(z_len);
    if (!z)
        return -1;
    crc_init();

    // zlib stream of stored blocks over the filtered rows (filter type 0)
    uint8_t *q = z;
    *q++ = 0x78;
    *q++ = 0x01;
    uint32_t a = 1, b = 0;
    size_t left = raw_len, pos = 0;
    while (left) {
        uint32_t n = left > 65535 ? 65535 : (uint32_t)left;
        left -= n;
        *q++ = left ? 0 : 1;
        *q++ = n & 0xff;
        *q++ = n >> 8;
        *q++ = ~n & 0xff;
        *q++ = (~n >> 8) & 0xff;
        for (uint32_t i = 0; i < n; i++, pos++) {
            size_t row = pos / (w + 1), col = pos % (w + 1);
            uint8_t v = col ? pix[row * w + col - 1] : 0;
            *q++ = v;
            a = (a + v) % 65521;
            b = (b + a) % 65521;
        }
    }
    be32(q, (b << 16) | a);

    uint8_t ihdr[13];
    be32(ihdr, w);
    be32(ihdr + 4, h);
    ihdr[8] = 8;
    ihdr[9] = 3;
    ihdr[10] = ihdr[11] = ihdr[12] = 0;

    FILE *f = fopen(path, "wb");
    int err = !f;
    if (f) {
        err |= fwrite(sig, 1, 8, f) != 8;
        err |= chunk(f, "IHDR", ihdr, 13);
        err |= chunk(f, "PLTE", pal, 768);
        err |= chunk(f, "IDAT", z, (uint32_t)z_len);
        err |= chunk(f, "IEND", NULL, 0);
        err |= fclose(f) != 0;
    }
    free(z);
    return err ? -1 : 0;
}
