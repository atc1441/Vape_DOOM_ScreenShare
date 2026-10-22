# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""UI pictures (status bar, faces, font, menu, intermission, full screens) and UIDIR."""

import struct

from . import format as F


def select(wad, exclude=()):
    """[(enum index, name, lump)] for every UI picture present in the WAD and not excluded."""
    out = []
    for i, n in enumerate(F.UI_NAMES):
        if n in exclude:
            continue
        lump = wad.num(n)
        if lump >= 0:
            out.append((i, n, lump))
    return out


def uidir(selected, patch_id):
    ids = [F.NONE16] * len(F.UI_NAMES)
    for i, n, lump in selected:
        ids[i] = patch_id[lump]
    return struct.pack("<%dH" % len(ids), *ids)
