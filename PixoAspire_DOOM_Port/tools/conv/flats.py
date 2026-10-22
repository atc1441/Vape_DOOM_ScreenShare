# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Flats: 32x32 nearest decimation, average colours and the 64x64 finale flat."""

from . import format as F
from .util import check, pad


def decimate(flat64):
    """byte [v * 32 + u] = 64x64 texel (2u, 2v), matching u = (xfrac >> 17) & 31."""
    check(len(flat64) >= 4096, "flat shorter than 4096 bytes")
    return bytes(flat64[(2 * v) * 64 + 2 * u] for v in range(32) for u in range(32))


def average_colour(flat64, playpal):
    """Mean RGB of the 64x64 flat in palette 0, mapped to the nearest palette index."""
    r = g = b = 0
    for c in flat64[:4096]:
        r += playpal[3 * c]
        g += playpal[3 * c + 1]
        b += playpal[3 * c + 2]
    r, g, b = (r + 2048) // 4096, (g + 2048) // 4096, (b + 2048) // 4096
    best, bd = 0, None
    for i in range(256):
        d = (playpal[3 * i] - r) ** 2 + (playpal[3 * i + 1] - g) ** 2 + (playpal[3 * i + 2] - b) ** 2
        if bd is None or d < bd:
            best, bd = i, d
    return best


def build(wad, names, finale_name):
    """names: flat names in global id order. Returns the FLATS section bytes."""
    playpal = wad.lump("PLAYPAL")
    data = bytearray()
    avg = bytearray()
    for n in names:
        f = wad.lump(n)
        data += decimate(f)
        avg.append(average_colour(f, playpal))
    data += avg
    data = bytearray(pad(data))
    check(len(data) == F.finale_flat_off(len(names)), "finale flat offset")
    fin = wad.lump(finale_name)
    check(len(fin) >= 4096, "finale flat %s too short", finale_name)
    data += fin[:4096]
    return bytes(data)
