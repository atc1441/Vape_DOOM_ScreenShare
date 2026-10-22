# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Reads a pack back into its logical image, with the format.py layouts."""

import os
import struct

from . import format as F
from .util import ConvError, crc32


class Image:
    def __init__(self, pack_dir):
        self.dir = pack_dir
        with open(os.path.join(pack_dir, "DOOM.DAT"), "rb") as f:
            self.doom = f.read()
        self.hdr = F.HEADER.unpack(self.doom, 0)
        if self.hdr["magic"] != F.MAGIC:
            raise ConvError("DOOM.DAT: bad magic")
        split = self.hdr["split"]
        self.doomx = None
        xp = os.path.join(pack_dir, "DOOMX.DAT")
        if split < self.hdr["image_size"]:
            with open(xp, "rb") as f:
                self.doomx = f.read()
            self.xhdr = F.DOOMX_HEADER.unpack(self.doomx, 0)
            self.img = self.doom[:split] + self.doomx[F.DOOMX_HDR:]
        else:
            self.img = self.doom
        self.sects = {}
        for k in range(self.hdr["nsect"]):
            e = F.SECTENT.unpack(self.img, F.SECT_TABLE_OFF + 16 * k)
            self.sects[e["id"]] = e
        self.maps = []
        for k in range(self.hdr["nmaps"]):
            e = F.MAPENT.unpack(self.img, F.MAP_TABLE_OFF + 32 * k)
            e["name"] = e["name"].rstrip(b"\0").decode()
            self.maps.append(e)

    def section(self, name):
        e = self.sects.get(F.SECT[name])
        if e is None:
            raise ConvError("section %s missing" % name)
        return self.img[e["off"]:e["off"] + e["size"]], e

    def header_crc_ok(self):
        h = bytearray(self.img[:F.HEADER_SIZE])
        h[F.HEADER.offsets["header_crc"]:F.HEADER.offsets["header_crc"] + 4] = b"\0" * 4
        return crc32(bytes(h)) == self.hdr["header_crc"]

    def map(self, name):
        for e in self.maps:
            if e["name"] == name:
                return MapBlock(self.img[e["off"]:e["off"] + e["size"]], e)
        raise ConvError("map %s missing" % name)


class MapBlock:
    def __init__(self, block, ent):
        self.block = block
        self.ent = ent
        self.h = F.MAPHDR.unpack(block, 0)
        self.plans = self.h["plans"]

    def stream(self, name):
        k = F.STREAM[name]
        o, n = self.h["stream_off"][k], self.h["stream_size"][k]
        return self.block[o:o + n]

    def records(self, name, st):
        data = self.stream(name)
        return [st.unpack(data, i) for i in range(0, len(data) - st.size + 1, st.size)]

    def u16s(self, name, start=0, count=None):
        data = self.stream(name)
        if count is None:
            count = (len(data) - start) // 2
        return list(struct.unpack_from("<%dH" % count, data, start))

    def ranges_and_list(self, name, n):
        """dmc_range_t[n] followed by u16 entries."""
        data = self.stream(name)
        rng = [F.RANGE.unpack(data, 4 * i) for i in range(n)]
        total = (len(data) - 4 * n) // 2
        ents = list(struct.unpack_from("<%dH" % total, data, 4 * n))
        return rng, ents

    def first_lists(self, name, n):
        """u16 first[n + 1] followed by the entries: returns a list of n lists."""
        data = self.stream(name)
        first = struct.unpack_from("<%dH" % (n + 1), data, 0)
        ents = struct.unpack_from("<%dH" % first[-1], data, 2 * (n + 1))
        return [list(ents[first[i]:first[i + 1]]) for i in range(n)]

    def tags(self):
        n = self.h["ntags"] + 1
        data = self.stream("TAGS")
        rng = [F.RANGE.unpack(data, 4 * i) for i in range(n)]
        total = sum(r["count"] for r in rng)
        secs = list(struct.unpack_from("<%dH" % total, data, 4 * n))
        vals = list(struct.unpack_from("<%dH" % n, data, 4 * n + 2 * total))
        return rng, secs, vals


def patch_columns(blob):
    """Decode a patch blob: (header dict, [[(topdelta, bytes)] per stored column])."""
    h = F.PATCH_HDR.unpack(blob, 0)
    cols = []
    for k in range(h["ncols"]):
        ci = struct.unpack_from("<I", blob, h["colinfo"] + 4 * k)[0]
        cols.append(decode_posts(blob, ci & F.COLINFO_OFF_MASK, ci >> F.COLINFO_LEN_SHIFT))
    return h, cols


def decode_posts(buf, off, length):
    posts = []
    p = off
    end = off + length
    while True:
        if p >= end:
            raise ConvError("post list runs past its length")
        td = buf[p]
        if td == 0xFF:
            if p + 1 != end:
                raise ConvError("post list shorter than its length")
            return posts
        n = buf[p + 1]
        posts.append((td, bytes(buf[p + 2:p + 2 + n])))
        p += 2 + n
