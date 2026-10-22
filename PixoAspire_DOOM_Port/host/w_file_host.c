// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// WAD shim for the host build: WAD files read with stdio.

#include <stdio.h>
#include <stdlib.h>

#include "w_file.h"
#include "z_zone.h"

typedef struct {
    wad_file_t wad;
    FILE *fp;
} host_wad_file_t;

extern wad_file_class_t stdc_wad_file;

static wad_file_t *host_open(char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp)
        return NULL;
    host_wad_file_t *f = Z_Malloc(sizeof(*f), PU_STATIC, NULL);
    f->wad.file_class = &stdc_wad_file;
    f->wad.mapped = NULL;
    fseek(fp, 0, SEEK_END);
    f->wad.length = (unsigned int)ftell(fp);
    f->fp = fp;
    return &f->wad;
}

static void host_close(wad_file_t *wad)
{
    host_wad_file_t *f = (host_wad_file_t *)wad;
    fclose(f->fp);
    Z_Free(f);
}

static size_t host_read(wad_file_t *wad, unsigned int offset, void *buffer, size_t len)
{
    host_wad_file_t *f = (host_wad_file_t *)wad;
    if (fseek(f->fp, offset, SEEK_SET))
        return 0;
    return fread(buffer, 1, len, f->fp);
}

wad_file_class_t stdc_wad_file = { host_open, host_close, host_read };
