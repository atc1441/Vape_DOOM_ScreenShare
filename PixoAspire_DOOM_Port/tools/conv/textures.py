# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Wall textures: vanilla compositing, power-of-two row tiling, decimated levels
and masked column posts (the TEXDIR and TEXDATA sections)."""

import struct

from . import format as F
from . import wadread
from .util import align, check


def log2_floor(n):
    k = 0
    while (2 << k) <= n:
        k += 1
    return k


def log2_ceil(n):
    k = 0
    while (1 << k) < n:
        k += 1
    return k


class Texture:
    def __init__(self, num, name, width, height, patches):
        self.num, self.name, self.width, self.height, self.patches = num, name, width, height, patches
        self.log2_w = log2_floor(width)            # vanilla texturewidthmask + 1 = 1 << log2_w
        self.w2 = 1 << self.log2_w
        # The vanilla wall drawer reads rows with & 127, so taller textures keep rows 0-127.
        self.log2_sh = min(log2_ceil(height), 7)
        self.sh = 1 << self.log2_sh


class TextureSet:
    """All vanilla textures (TEXTURE1 then TEXTURE2) with the R_GetColumn emulation."""

    def __init__(self, wad):
        self.wad = wad
        self.list = [Texture(i, n, w, h, p) for i, (n, w, h, p) in enumerate(wadread.textures(wad))]
        self.first = {}
        for t in self.list:
            self.first.setdefault(t.name, t.num)
        self._pics = {}
        self._cols = {}

    def pic(self, lump):
        p = self._pics.get(lump)
        if p is None:
            p = self._pics[lump] = wadread.Picture(self.wad.read(lump))
        return p

    def _lookup(self, t):
        """R_GenerateLookup: per column the covering patch count and the last covering patch."""
        count = [0] * t.width
        last = [None] * t.width
        for ox, oy, lump in t.patches:
            p = self.pic(lump)
            x1 = ox
            x2 = min(x1 + p.width, t.width)
            for x in range(max(x1, 0), x2):
                count[x] += 1
                last[x] = (lump, x - x1)
        return count, last

    def columns(self, t):
        """Per stored column (0 .. w2-1): (opaque bytes[height], masked posts)."""
        if t.num in self._cols:
            return self._cols[t.num]
        count, last = self._lookup(t)
        for x in range(t.w2):
            check(count[x] > 0, "texture %s: column %d without a patch", t.name, x)
        comp = None
        cover = None
        if any(c > 1 for c in count[:t.w2]):
            comp = [bytearray(t.height) for _ in range(t.width)]
            cover = [bytearray(t.height) for _ in range(t.width)]
            for ox, oy, lump in t.patches:
                p = self.pic(lump)
                for x in range(max(ox, 0), min(ox + p.width, t.width)):
                    if count[x] < 2:
                        continue
                    # R_DrawColumnInCache: a post clipped at the top still copies from its start
                    for td, data in p.columns[x - ox]:
                        pos = oy + td
                        n = len(data)
                        if pos < 0:
                            n += pos
                            pos = 0
                        if pos + n > t.height:
                            n = t.height - pos
                        if n > 0:
                            comp[x][pos:pos + n] = data[:n]
                            cover[x][pos:pos + n] = b"\1" * n
        out = []
        for x in range(t.w2):
            if count[x] == 1:
                lump, px = last[x]
                p = self.pic(lump)
                raw = p.raw
                start = p.colofs[px] + 3
                col = raw[start:start + t.height]
                col = bytes(col) + b"\0" * (t.height - len(col))
                posts = [(td, bytes(d)) for td, d in p.columns[px]]
            else:
                col = bytes(comp[x])
                posts = runs_to_posts(cover[x], comp[x])
            out.append((col, posts))
        self._cols[t.num] = out
        return out

    def multi_patch_masked_columns(self, t):
        count, _ = self._lookup(t)
        return sum(1 for x in range(t.w2) if count[x] > 1)


def runs_to_posts(cover, pixels):
    posts = []
    y = 0
    n = len(cover)
    while y < n:
        if not cover[y]:
            y += 1
            continue
        s = y
        while y < n and cover[y] and y - s < 255:
            y += 1
        posts.append((s, bytes(pixels[s:y])))
    return posts


def encode_posts(posts):
    out = bytearray()
    for td, data in posts:
        check(td < 255 and 0 < len(data) < 256, "post out of range")
        out += bytes((td, len(data))) + data
    out.append(0xFF)
    return bytes(out)


def level0(t, cols):
    """Opaque level 0: w2 columns of sh rows, rows tiled with the texture height."""
    k = t.sh // t.height + 1
    return [(col * k)[:t.sh] for col, _ in cols]


def level(t, l0, m):
    return [l0[j << m][::1 << m] for j in range(t.w2 >> m)]


def max_levels(t, base):
    n = 0
    while base + n <= 3 and (t.w2 >> (base + n)) >= 1 and (t.sh >> (base + n)) >= 1:
        n += 1
    return n


def build(ts, plan):
    """plan: list of (texture, base level, nlevels, masked) in global id order.

    Returns (TEXDIR bytes, TEXDATA bytes, per-texture info list).
    """
    data = bytearray()
    ents = []
    meta = []
    for t, base, nlev, masked in plan:
        cols = ts.columns(t)
        start = align(len(data))
        data += b"\0" * (start - len(data))
        flags = (base << F.TF_BASE_SHIFT) & F.TF_BASE_MASK
        if nlev > 1:
            flags |= F.TF_MIPS
        if nlev:
            l0 = level0(t, cols)
            for m in range(base, base + nlev):
                for c in (l0 if m == 0 else level(t, l0, m)):
                    data += c
        ci_off = 0
        if masked:
            flags |= F.TF_MASKED
            infos = []
            for j in range(t.w2 >> base):
                blob = encode_posts(cols[j << base][1])
                infos.append(F.colinfo(len(data) - start, len(blob)))
                data += blob
            ci = align(len(data), 4)
            data += b"\0" * (ci - len(data))
            ci_off = ci
            data += struct.pack("<%dI" % len(infos), *infos)
        ents.append(F.TEXENT.pack(data=start, width=t.width, height=t.height, flags=flags,
                                  log2_sh=t.log2_sh, log2_w=t.log2_w, nlevels=nlev, colinfo=ci_off))
        meta.append({"name": t.name, "base": base, "nlevels": nlev, "masked": masked,
                     "bytes": len(data) - start})
    texdir = F.TEXDIR_HDR.pack(count=len(plan)) + b"".join(ents)
    return texdir, bytes(data), meta


def opaque_size(t, base, nlev):
    return sum((t.w2 >> m) * (t.sh >> m) for m in range(base, base + nlev))
