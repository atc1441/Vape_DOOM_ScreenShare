# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Minimal WAD reader: directory, map lumps, pictures, textures, flats, DEHACKED."""

import struct

from .util import check

MAP_LUMPS = ("THINGS", "LINEDEFS", "SIDEDEFS", "VERTEXES", "SEGS", "SSECTORS", "NODES",
             "SECTORS", "REJECT", "BLOCKMAP")


def lname(b):
    return b.split(b"\0", 1)[0].decode("latin-1").upper()


class Wad:
    def __init__(self, data):
        self.data = data
        ident, n, off = struct.unpack_from("<4sii", data, 0)
        check(ident in (b"IWAD", b"PWAD"), "not a WAD")
        self.lumps = []
        for i in range(n):
            pos, size, name = struct.unpack_from("<ii8s", data, off + 16 * i)
            self.lumps.append((lname(name), pos, size))
        self.last = {}
        for i, (nm, _, _) in enumerate(self.lumps):
            self.last[nm] = i

    def num(self, name):
        """Vanilla W_CheckNumForName: the last lump with that name, or -1."""
        return self.last.get(name.upper(), -1)

    def read(self, i):
        _, pos, size = self.lumps[i]
        return self.data[pos:pos + size]

    def lump(self, name):
        i = self.num(name)
        check(i >= 0, "lump %s missing", name)
        return self.read(i)

    def name(self, i):
        return self.lumps[i][0]

    def map_lumps(self, mapname):
        i = self.num(mapname)
        check(i >= 0, "map %s missing", mapname)
        out = {}
        for k, want in enumerate(MAP_LUMPS):
            check(self.lumps[i + 1 + k][0] == want, "map %s: lump %s out of order", mapname, want)
            out[want] = self.read(i + 1 + k)
        return out

    def range(self, start, end):
        """Lump indices strictly between the markers (vanilla style, by name)."""
        return list(range(self.num(start) + 1, self.num(end)))


def switch_episode(wad):
    """Last alphSwitchList episode of vanilla P_InitSwitchList: 1 for a shareware IWAD
    (D_IdentifyVersion finds no E3M1 and no E4M1), 2 for registered and retail."""
    return 2 if wad.num("E3M1") >= 0 or wad.num("E4M1") >= 0 else 1


# ---------------------------------------------------------------- map lumps

def parse_vertexes(b):
    return [struct.unpack_from("<hh", b, i) for i in range(0, len(b) - 3, 4)]


def parse_linedefs(b):
    # v1, v2, flags, special, tag, side0, side1 (sides as signed shorts like vanilla)
    out = []
    for i in range(0, len(b) - 13, 14):
        v1, v2, fl, sp, tag, s0, s1 = struct.unpack_from("<HHHHhhh", b, i)
        out.append((v1, v2, fl, sp, tag, s0, s1))
    return out


def parse_sidedefs(b):
    out = []
    for i in range(0, len(b) - 29, 30):
        xo, yo, t, bt, m, sec = struct.unpack_from("<hh8s8s8sh", b, i)
        out.append((xo, yo, lname(t), lname(bt), lname(m), sec))
    return out


def parse_sectors(b):
    out = []
    for i in range(0, len(b) - 25, 26):
        fh, ch, fp, cp, light, special, tag = struct.unpack_from("<hh8s8shhh", b, i)
        out.append((fh, ch, lname(fp), lname(cp), light, special, tag))
    return out


def parse_ssectors(b):
    return [struct.unpack_from("<HH", b, i) for i in range(0, len(b) - 3, 4)]


def parse_segs(b):
    # v1, v2, angle, linedef, side, offset
    return [struct.unpack_from("<HHhHhh", b, i) for i in range(0, len(b) - 11, 12)]


def parse_nodes(b):
    out = []
    for i in range(0, len(b) - 27, 28):
        v = struct.unpack_from("<hhhh8hHH", b, i)
        out.append((v[0], v[1], v[2], v[3], tuple(v[4:8]), tuple(v[8:12]), (v[12], v[13])))
    return out


def parse_things(b):
    return [struct.unpack_from("<hhhhh", b, i) for i in range(0, len(b) - 9, 10)]


def parse_blockmap(b):
    ox, oy, cols, rows = struct.unpack_from("<hhhh", b, 0)
    return ox, oy, cols, rows


# ---------------------------------------------------------------- pictures

class Picture:
    """Vanilla patch: header plus per-column post lists [(topdelta, bytes)]."""

    def __init__(self, data):
        self.raw = data
        self.width, self.height, self.left, self.top = struct.unpack_from("<hhhh", data, 0)
        check(0 < self.width <= 4096 and 0 <= self.height <= 4096, "bad picture header")
        self.colofs = list(struct.unpack_from("<%dI" % self.width, data, 8))
        self.columns = []
        for x in range(self.width):
            p = self.colofs[x]
            posts = []
            while True:
                check(p < len(data), "picture column runs past the lump")
                td = data[p]
                if td == 0xFF:
                    break
                n = data[p + 1]
                posts.append((td, data[p + 3:p + 3 + n]))
                p += n + 4
            self.columns.append(posts)


# ---------------------------------------------------------------- textures

def parse_pnames(b):
    n = struct.unpack_from("<i", b, 0)[0]
    return [lname(b[4 + 8 * i:12 + 8 * i]) for i in range(n)]


def parse_texture_lump(b):
    """[(name, width, height, [(originx, originy, pname index)])] in lump order."""
    n = struct.unpack_from("<i", b, 0)[0]
    offs = struct.unpack_from("<%di" % n, b, 4)
    out = []
    for o in offs:
        name = lname(b[o:o + 8])
        _masked, w, h, _cd, npatch = struct.unpack_from("<ihhih", b, o + 8)
        patches = []
        for k in range(npatch):
            ox, oy, pn, _sd, _cm = struct.unpack_from("<hhhhh", b, o + 22 + 10 * k)
            patches.append((ox, oy, pn))
        out.append((name, w, h, patches))
    return out


def textures(wad):
    """Vanilla texture list: TEXTURE1 then TEXTURE2, with patch lump numbers resolved."""
    pnames = parse_pnames(wad.lump("PNAMES"))
    plumps = [wad.num(p) for p in pnames]
    out = []
    for tl in ("TEXTURE1", "TEXTURE2"):
        if wad.num(tl) < 0:
            continue
        for name, w, h, patches in parse_texture_lump(wad.lump(tl)):
            pl = []
            for ox, oy, pn in patches:
                check(0 <= pn < len(plumps) and plumps[pn] >= 0,
                      "texture %s: missing patch %d", name, pn)
                pl.append((ox, oy, plumps[pn]))
            out.append((name, w, h, pl))
    return out


# ---------------------------------------------------------------- DEHACKED

def parse_dehacked(text):
    """Return {'frames': {n: {field: value}}, 'pars': {(e, m): s}, 'strings': {k: v}}."""
    frames, pars, strings = {}, {}, {}
    section = None
    cur = None
    lines = text.replace("\r", "").split("\n")
    i = 0
    while i < len(lines):
        ln = lines[i]
        i += 1
        s = ln.strip()
        if not s or s.startswith("#"):
            continue
        if s.startswith("["):
            section = s.strip("[]").upper()
            cur = None
            continue
        words = s.split()
        if words[0] in ("Frame", "Thing", "Sound", "Weapon", "Ammo", "Misc", "Cheat", "Pointer",
                        "Text", "Sprite") and len(words) >= 2 and words[1].isdigit():
            section = None
            cur = frames.setdefault(int(words[1]), {}) if words[0] == "Frame" else {}
            continue
        if section == "PARS":
            if words[0].lower() == "par" and len(words) >= 4:
                pars[(int(words[1]), int(words[2]))] = int(words[3])
            continue
        if section == "STRINGS":
            if "=" not in s:
                continue
            key, val = s.split("=", 1)
            val = val.strip()
            while val.endswith("\\") and i < len(lines):
                val = val[:-1] + lines[i].strip()
                i += 1
            strings[key.strip().upper()] = bex_unescape(val)
            continue
        if cur is not None and "=" in s:
            key, val = s.split("=", 1)
            try:
                cur[key.strip()] = int(val.strip())
            except ValueError:
                pass
    return {"frames": frames, "pars": pars, "strings": strings}


def bex_unescape(v):
    out = []
    k = 0
    while k < len(v):
        c = v[k]
        if c == "\\" and k + 1 < len(v):
            n = v[k + 1]
            out.append({"n": "\n", "t": "\t", "\\": "\\", '"': '"'}.get(n, n))
            k += 2
            continue
        out.append(c)
        k += 1
    return "".join(out)


def dehacked(wad):
    i = wad.num("DEHACKED")
    if i < 0:
        return {"frames": {}, "pars": {}, "strings": {}}
    return parse_dehacked(wad.read(i).decode("latin-1"))
