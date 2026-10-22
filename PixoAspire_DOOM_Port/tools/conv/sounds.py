# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""PC speaker sounds (DP lumps), indexed by the vanilla sfx number."""

import struct

from . import format as F
from .util import check


def build(wad, sfx):
    """sfx: upstream S_sfx rows [name, priority, link, pitch, volume]. Returns (bytes, stats)."""
    hdr = F.SOUNDS_HDR.size + F.SOUND.size * len(sfx)
    ents, data = [], bytearray()
    present = 0
    for name, _prio, link, _p, _v in sfx:
        lump = wad.num("DP" + name.upper()) if name and link < 0 else -1
        if lump < 0:
            ents.append(F.SOUND.pack(off=0, len=0))
            continue
        b = wad.read(lump)
        check(len(b) >= 4, "DP%s too short", name)
        kind, n = struct.unpack_from("<HH", b, 0)
        check(kind == 0 and 4 + n <= len(b) and n < 65536, "DP%s header", name)
        ents.append(F.SOUND.pack(off=hdr + len(data), len=n))
        data += b[4:4 + n]
        present += 1
    out = F.SOUNDS_HDR.pack(count=len(sfx)) + b"".join(ents) + bytes(data)
    return out, {"sounds": present, "bytes": len(data)}
