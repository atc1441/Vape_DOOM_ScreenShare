# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Decode every texture column, flat, sprite and UI picture of a pack and compare
it with an independent composite built from the WAD at the stored LOD. Also
checks SPRDEF against an independent R_InitSpriteDefs, UIDIR names, SOUNDS
against the DP lumps, PALETTE against PLAYPAL and ANIMS against the upstream
animation and switch lists. UI pictures stored at half width must draw (column
x from stored column x >> xshift) the pictures of the pack's halfui.wad, the
oracle's copy of them, column for column.

usage: python3 -I tools/dmcview.py [--zip FREEDOOM_ZIP | --wad IWAD] [--png OUT_DIR]
       PACK_DIR [PACK_DIR ...]

The WAD side below shares no code with the converter. The pack's names.json
says which WAD texture, flat, lump or sprite each id came from. The upstream
tables come from the dumper of the pinned upstream source.
"""

import argparse
import json
import os
import struct
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from tools.conv import fetch, format as F, upstream  # noqa: E402
from tools.conv.reader import Image, decode_posts, patch_columns  # noqa: E402
from tools.conv.util import ConvError  # noqa: E402

UI_OPTIONAL = ("HELP1", "CREDIT")
SWITCH_EPISODE = 2


# ------------------------------------------------------------------ independent WAD side

class Wad:
    def __init__(self, data):
        self.d = data
        n, off = struct.unpack_from("<ii", data, 4)
        self.dir = []
        for i in range(n):
            p, s, nm = struct.unpack_from("<ii8s", data, off + 16 * i)
            self.dir.append((nm.split(b"\0")[0].decode("latin-1").upper(), p, s))
        self.idx = {}
        for i, (nm, _, _) in enumerate(self.dir):
            self.idx[nm] = i

    def get(self, name):
        _, p, s = self.dir[self.idx[name.upper()]]
        return self.d[p:p + s]


def texture_order(wad):
    """Vanilla texture numbers: TEXTURE1 then TEXTURE2 entries in lump order."""
    out = []
    for lump in ("TEXTURE1", "TEXTURE2"):
        if lump not in wad.idx:
            continue
        b = wad.get(lump)
        for k in range(struct.unpack_from("<i", b, 0)[0]):
            o = struct.unpack_from("<i", b, 4 + 4 * k)[0]
            out.append(b[o:o + 8].split(b"\0")[0].decode("latin-1").upper())
    return out


def sprite_defs(wad, sname):
    """R_InitSpriteDefs for one sprite: [(rotate, [(lump name, flip)] * 8)] per frame."""
    first, last = wad.idx["S_START"] + 1, wad.idx["S_END"] - 1
    frames = {}

    def install(name, frame, rot, flip):
        if frame >= 29 or rot > 8:
            raise ConvError("sprite %s: bad frame or rotation" % name)
        fr = frames.setdefault(frame, [None, [None] * 8])
        if rot == 0:
            if fr[0] is not None:
                raise ConvError("sprite %s: rotation 0 conflict" % name)
            fr[0] = False
            fr[1] = [(name, flip)] * 8
            return
        if fr[0] is False or fr[1][rot - 1] is not None:
            raise ConvError("sprite %s: rotation conflict" % name)
        fr[0] = True
        fr[1][rot - 1] = (name, flip)
    for l in range(first, last + 1):
        nm = wad.dir[l][0]
        if nm[:4] != sname:
            continue
        install(nm, ord(nm[4]) - 65, ord(nm[5]) - 48, False)
        if len(nm) > 6:
            install(nm, ord(nm[6]) - 65, ord(nm[7]) - 48, True)
    out = []
    for f in range(max(frames) + 1 if frames else 0):
        fr = frames.get(f)
        if fr is None or (fr[0] and None in fr[1]):
            raise ConvError("sprite %s frame %c incomplete" % (sname, 65 + f))
        out.append(fr)
    return out


def patch(raw):
    w, h, lo, to = struct.unpack_from("<hhhh", raw, 0)
    cols = []
    for x in range(w):
        p = struct.unpack_from("<I", raw, 8 + 4 * x)[0]
        posts = []
        while raw[p] != 0xFF:
            posts.append((raw[p], raw[p + 3:p + 3 + raw[p + 1]]))
            p += raw[p + 1] + 4
        cols.append(posts)
    return {"w": w, "h": h, "left": lo, "top": to, "cols": cols, "raw": raw}


def texture_defs(wad):
    pn = wad.get("PNAMES")
    pnames = [pn[4 + 8 * i:12 + 8 * i].split(b"\0")[0].decode("latin-1").upper()
              for i in range(struct.unpack_from("<i", pn, 0)[0])]
    defs = {}
    for lump in ("TEXTURE1", "TEXTURE2"):
        if lump not in wad.idx:
            continue
        b = wad.get(lump)
        for k in range(struct.unpack_from("<i", b, 0)[0]):
            o = struct.unpack_from("<i", b, 4 + 4 * k)[0]
            name = b[o:o + 8].split(b"\0")[0].decode("latin-1").upper()
            w, h = struct.unpack_from("<hh", b, o + 12)
            np = struct.unpack_from("<h", b, o + 20)[0]
            pl = [struct.unpack_from("<hhh", b, o + 22 + 10 * j) for j in range(np)]
            if name not in defs:
                defs[name] = (w, h, [(ox, oy, pnames[i]) for ox, oy, i in pl])
    return defs


def wad_texture_columns(wad, tdef):
    """Per column: (bytes the wall drawer reads for rows 0..h-1, painted masked map {row: pixel})."""
    w, h, pl = tdef
    ps = [(ox, oy, patch(wad.get(n))) for ox, oy, n in pl]
    owners = [[] for _ in range(w)]
    for ox, oy, p in ps:
        for x in range(max(0, ox), min(w, ox + p["w"])):
            owners[x].append((ox, oy, p))
    out = []
    for x in range(w):
        if len(owners[x]) == 1:
            ox, oy, p = owners[x][0]
            start = struct.unpack_from("<I", p["raw"], 8 + 4 * (x - ox))[0] + 3
            col = bytes(p["raw"][start:start + h]).ljust(h, b"\0")
            masked = {}
            for td, px in p["cols"][x - ox]:
                for k, c in enumerate(px):
                    masked[td + k] = c
        else:
            canvas = bytearray(h)
            masked = {}
            for ox, oy, p in owners[x]:
                for td, px in p["cols"][x - ox]:
                    top = oy + td
                    cnt = len(px) + min(top, 0)
                    top = max(top, 0)
                    cnt = min(cnt, h - top)
                    for k in range(max(cnt, 0)):
                        canvas[top + k] = px[k]
                        masked[top + k] = px[k]
            col = bytes(canvas)
        out.append((col, masked))
    return out


def painted(posts, rows_step=1):
    m = {}
    for td, px in posts:
        for k, c in enumerate(px):
            m[(td + k) * rows_step] = c
    return m


# ------------------------------------------------------------------ PNG

def write_png(path, w, h, rgba):
    rows = b"".join(b"\0" + rgba[y * w * 4:(y + 1) * w * 4] for y in range(h))

    def chunk(t, d):
        return struct.pack(">I", len(d)) + t + d + struct.pack(">I", zlib.crc32(t + d) & 0xFFFFFFFF)
    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0)) +
                chunk(b"IDAT", zlib.compress(rows, 6)) + chunk(b"IEND", b""))


def palette_rgb(im):
    pal, _ = im.section("PALETTE")
    out = []
    for i in range(256):
        v = struct.unpack_from("<H", pal, 2 * i)[0]
        r, g, b = (v >> 11) & 31, (v >> 5) & 63, v & 31
        out.append(((r << 3) | (r >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2)))
    return out


def image_png(path, w, h, pix, pal):
    """pix: dict (x, y) -> index, missing pixels are transparent."""
    buf = bytearray(4 * w * h)
    for (x, y), c in pix.items():
        if 0 <= x < w and 0 <= y < h:
            o = 4 * (y * w + x)
            buf[o:o + 4] = bytes(pal[c]) + b"\xff"
    write_png(path, w, h, bytes(buf))


# ------------------------------------------------------------------ comparisons

class Viewer:
    def __init__(self, wad, path, png_dir):
        self.wad, self.path, self.png = wad, path, png_dir
        self.im = Image(path)
        with open(os.path.join(path, "names.json")) as f:
            self.names = json.load(f)
        self.errors = []
        self.counts = {"texture columns": 0, "masked columns": 0, "flats": 0, "patches": 0, "patch columns": 0,
                       "sprite frames": 0, "ui pictures": 0, "half-width ui pictures": 0, "sounds": 0,
                       "anims": 0}
        self.pal = palette_rgb(self.im)
        self.tdefs = texture_defs(wad)
        if png_dir:
            for sub in ("textures", "flats", "patches"):
                os.makedirs(os.path.join(png_dir, sub), exist_ok=True)

    def err(self, msg, *a):
        if len(self.errors) < 100:
            self.errors.append(msg % a)
        else:
            self.errors.append(None)

    def textures(self):
        texdir, _ = self.im.section("TEXDIR")
        data, _ = self.im.section("TEXDATA")
        n = F.TEXDIR_HDR.unpack(texdir)["count"]
        for k in range(n):
            t = F.TEXENT.unpack(texdir, 16 + 16 * k)
            name = self.names["textures"][k]
            tdef = self.tdefs[name]
            cols = wad_texture_columns(self.wad, tdef)
            w2, sh, base = 1 << t["log2_w"], 1 << t["log2_sh"], (t["flags"] & F.TF_BASE_MASK) >> F.TF_BASE_SHIFT
            if t["width"] != tdef[0] or t["height"] != tdef[1]:
                self.err("texture %s geometry", name)
            h = tdef[1]
            off = t["data"]
            shown = None
            for m in range(base, base + t["nlevels"]):
                for j in range(w2 >> m):
                    got = data[off:off + (sh >> m)]
                    off += sh >> m
                    src = cols[j << m][0]
                    want = bytes(src[((i << m) & 127) % h] for i in range(sh >> m))
                    self.counts["texture columns"] += 1
                    if got != want:
                        self.err("texture %s level %d column %d differs", name, m, j)
                if shown is None:
                    shown = (m, off - (w2 >> m) * (sh >> m))
            if t["flags"] & F.TF_MASKED:
                for j in range(w2 >> base):
                    ci = struct.unpack_from("<I", data, t["colinfo"] + 4 * j)[0]
                    posts = decode_posts(data, t["data"] + (ci & F.COLINFO_OFF_MASK), ci >> F.COLINFO_LEN_SHIFT)
                    self.counts["masked columns"] += 1
                    if painted(posts) != cols[j << base][1]:
                        self.err("texture %s masked column %d differs", name, j)
            if self.png and shown:
                m, o = shown
                pix = {}
                for j in range(w2 >> m):
                    for i in range(sh >> m):
                        pix[(j, i)] = data[o + j * (sh >> m) + i]
                image_png(os.path.join(self.png, "textures", "%03d_%s_L%d.png" % (k, name, m)),
                          w2 >> m, sh >> m, pix, self.pal)

    def flats(self):
        fl, fe = self.im.section("FLATS")
        n = fe["flags"]
        pp = self.wad.get("PLAYPAL")
        for k, name in enumerate(self.names["flats"]):
            src = self.wad.get(name)
            want = bytes(src[(2 * v) * 64 + 2 * u] for v in range(32) for u in range(32))
            got = fl[k * 1024:(k + 1) * 1024]
            self.counts["flats"] += 1
            if got != want:
                self.err("flat %s differs", name)
            rgb = [sum(pp[3 * c + i] for c in src[:4096]) for i in range(3)]
            rgb = [(v + 2048) // 4096 for v in rgb]
            dist = [sum((pp[3 * c + i] - rgb[i]) ** 2 for i in range(3)) for c in range(256)]
            if fl[n * 1024 + k] != dist.index(min(dist)):
                self.err("flat %s average colour", name)
            if self.png:
                image_png(os.path.join(self.png, "flats", "%03d_%s.png" % (k, name)), 32, 32,
                          {(i % 32, i // 32): c for i, c in enumerate(got)}, self.pal)
        fo = F.finale_flat_off(n)
        if fl[fo:fo + 4096] != self.wad.get(self.names["finale_flat"])[:4096]:
            self.err("finale flat differs")

    def patches(self):
        pdir, _ = self.im.section("PATCHDIR")
        pdata, _ = self.im.section("PATCHDATA")
        n = struct.unpack_from("<I", pdir, 0)[0]
        offs = list(struct.unpack_from("<%dI" % n, pdir, 4)) + [len(pdata)]
        for k in range(n):
            name = self.names["patches"][k]
            h, cols = patch_columns(pdata[offs[k]:offs[k + 1]])
            src = patch(self.wad.get(name))
            self.counts["patches"] += 1
            if (h["width"], h["height"], h["leftoffset"], h["topoffset"]) != \
                    (src["w"], src["h"], src["left"], src["top"]):
                self.err("patch %s header", name)
            half = bool(h["flags"] & F.PF_HALF_ROWS)
            pix = {}
            for j, posts in enumerate(cols):
                want = painted(src["cols"][j << h["xshift"]])
                if half:
                    want = {y: c for y, c in want.items() if y % 2 == 0 and y < src["h"]}
                got = painted(posts, 2 if half else 1)
                self.counts["patch columns"] += 1
                if got != want:
                    self.err("patch %s column %d differs", name, j)
                for y, c in got.items():
                    pix[(j, y)] = c
            if self.png:
                image_png(os.path.join(self.png, "patches", "%04d_%s.png" % (k, name)),
                          max(1, h["ncols"]), max(1, h["height"]), pix, self.pal)

    def palette(self):
        pal, _ = self.im.section("PALETTE")
        pp = self.wad.get("PLAYPAL")
        rgb = [pp[3 * i:3 * i + 3] for i in range(F.PALETTE_COUNT * 256)]
        want = b"".join(struct.pack("<H", ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)) for r, g, b in rgb)
        if pal != want:
            self.err("PALETTE differs from PLAYPAL")

    def sprdef(self):
        spr, _ = self.im.section("SPRDEF")
        h = F.SPRDEF_HDR.unpack(spr)
        names = self.names["sprites"]
        if h["nsprites"] != len(names):
            self.err("SPRDEF has %d sprites, names.json %d", h["nsprites"], len(names))
            return
        for k, sname in enumerate(names):
            e = F.SPRITE.unpack(spr, F.SPRDEF_HDR.size + F.SPRITE.size * k)
            want = sprite_defs(self.wad, sname)
            if e["numframes"] != len(want):
                self.err("sprite %s: %d frames, WAD %d", sname, e["numframes"], len(want))
                continue
            for f, (rotate, lumps) in enumerate(want):
                v = F.SPRFRAME.unpack(spr, h["frames_off"] + F.SPRFRAME.size * (e["first"] + f))["lump"]
                pids = [x & F.SPR_PATCH_MASK for x in v]
                if all(p == F.SPR_NO_PATCH for p in pids):
                    continue
                self.counts["sprite frames"] += 1
                got = [(self.names["patches"][p] if p != F.SPR_NO_PATCH else None, bool(x & F.SPR_FLIP))
                       for p, x in zip(pids, v)]
                rot = [bool(x & F.SPR_ROTATE) for x in v]
                if got != lumps or rot != [rotate] + [False] * 7:
                    self.err("sprite %s frame %c differs: %s", sname, 65 + f, got)

    def uidir(self):
        uid, ue = self.im.section("UIDIR")
        ids = struct.unpack_from("<%dH" % ue["flags"], uid)
        for name, v in zip(F.UI_NAMES, ids):
            if v == F.NONE16:
                if name in self.wad.idx and name not in UI_OPTIONAL:
                    self.err("UI picture %s missing", name)
                continue
            self.counts["ui pictures"] += 1
            if self.names["patches"][v] != name:
                self.err("UI %s points at patch %s", name, self.names["patches"][v])

    def half_ui(self):
        uid, ue = self.im.section("UIDIR")
        ids = struct.unpack_from("<%dH" % ue["flags"], uid)
        pdir, _ = self.im.section("PATCHDIR")
        pdata, _ = self.im.section("PATCHDATA")
        n = struct.unpack_from("<I", pdir, 0)[0]
        offs = list(struct.unpack_from("<%dI" % n, pdir, 4)) + [len(pdata)]
        half = []
        for name, v in zip(F.UI_NAMES, ids):
            if v != F.NONE16 and F.PATCH_HDR.unpack(pdata, offs[v])["xshift"]:
                half.append((name, v))
        if bool(half) != bool(self.im.hdr["flags"] & F.HF_HALF_UI):
            self.err("HF_HALF_UI flag does not match the half-width UI pictures %s", [k for k, _ in half])
        wp = os.path.join(self.path, "halfui.wad")
        if not os.path.exists(wp):
            if half:
                self.err("no halfui.wad for the half-width UI pictures")
            return
        with open(wp, "rb") as f:
            ref = Wad(f.read())
        if sorted(nm for nm, _, _ in ref.dir) != sorted(k for k, _ in half):
            self.err("halfui.wad holds %s, the half-width UI pictures are %s",
                     [nm for nm, _, _ in ref.dir], [k for k, _ in half])
            return
        for name, v in half:
            h, cols = patch_columns(pdata[offs[v]:offs[v + 1]])
            src = patch(ref.get(name))
            self.counts["half-width ui pictures"] += 1
            if (h["width"], h["height"], h["leftoffset"], h["topoffset"]) != \
                    (src["w"], src["h"], src["left"], src["top"]):
                self.err("half-width %s header differs from halfui.wad", name)
                continue
            for x in range(src["w"]):
                if painted(cols[x >> h["xshift"]]) != painted(src["cols"][x]):
                    self.err("half-width %s column %d differs from halfui.wad", name, x)

    def sounds(self, up):
        snd, _ = self.im.section("SOUNDS")
        n = F.SOUNDS_HDR.unpack(snd)["count"]
        if n != len(up["sfx"]):
            self.err("SOUNDS count %d, upstream %d", n, len(up["sfx"]))
            return
        for k, (name, _prio, link, _pitch, _vol) in enumerate(up["sfx"]):
            e = F.SOUND.unpack(snd, F.SOUNDS_HDR.size + F.SOUND.size * k)
            lump = "DP" + name.upper()
            if link >= 0 or lump not in self.wad.idx:
                if e["len"]:
                    self.err("sound %s should be empty", name)
                continue
            raw = self.wad.get(lump)
            cnt = struct.unpack_from("<H", raw, 2)[0]
            self.counts["sounds"] += 1
            if snd[e["off"]:e["off"] + e["len"]] != raw[4:4 + cnt]:
                self.err("sound %s differs from %s", name, lump)

    def anims(self, up):
        an, ae = self.im.section("ANIMS")
        got = [F.ANIM.unpack(an, F.ANIM.size * k) for k in range(ae["flags"])]
        tex = self.names["textures"]
        flats = self.names["flats"]
        torder = texture_order(self.wad)
        firstflat = self.wad.idx["F_START"] + 1
        want = []
        for istex, start, end, speed in up["animdefs"]:
            start, end = start.upper(), end.upper()
            if istex:
                if start not in torder:
                    continue
                s, e = torder.index(start), torder.index(end)
                cycle, pool = torder[s:e + 1], tex
            else:
                if start not in self.wad.idx:
                    continue
                s, e = self.wad.idx[start] - firstflat, self.wad.idx[end] - firstflat
                cycle, pool = [self.wad.dir[firstflat + j][0] for j in range(s, e + 1)], flats
            if set(cycle) & set(pool):
                want.append((F.ANIM_TEXTURE if istex else F.ANIM_FLAT, speed, cycle, s % len(cycle)))
        for a, b, ep in up["switches"]:
            if ep <= SWITCH_EPISODE and (a.upper() in tex or b.upper() in tex):
                want.append((F.ANIM_SWITCH, 0, [a.upper(), b.upper()], 0))
        have = []
        for a in got:
            names = tex if a["kind"] != F.ANIM_FLAT else flats
            have.append((a["kind"], a["speed"], names[a["first"]:a["last"] + 1], a["phase"]))
            if a["kind"] == F.ANIM_SWITCH and a["first"] % 2:
                self.err("switch pair %s at an odd id", names[a["first"]])
        self.counts["anims"] = len(have)
        if sorted(have) != sorted(want):
            self.err("ANIMS differ: extra %s, missing %s", [x for x in have if x not in want],
                     [x for x in want if x not in have])

    def run(self, up):
        self.textures()
        self.flats()
        self.patches()
        self.palette()
        self.sprdef()
        self.uidir()
        self.half_ui()
        self.sounds(up)
        self.anims(up)


def main(argv):
    ap = argparse.ArgumentParser(description="decode and compare a DMC DOOM pack with the WAD")
    ap.add_argument("--zip", help="freedoom-0.13.0.zip (default: $FREEDOOM_ZIP, dl/ cache, download)")
    ap.add_argument("--wad", help="local IWAD the packs were made from, instead of the Freedoom zip")
    ap.add_argument("--png", help="write PNG previews into this directory")
    ap.add_argument("packs", nargs="+")
    a = ap.parse_args(argv[1:])
    try:
        wad_bytes = fetch.read_wad(a.wad) if a.wad else fetch.extract(a.zip)[0]
        up = upstream.load()
    except ConvError as e:
        print("dmcview: error: %s" % e, file=sys.stderr)
        return 1
    wad = Wad(wad_bytes)
    bad = 0
    for p in a.packs:
        try:
            v = Viewer(wad, p, os.path.join(a.png, os.path.basename(p.rstrip("/"))) if a.png else None)
            v.run(up)
        except (ConvError, OSError, KeyError, ValueError, struct.error) as e:
            bad += 1
            print("dmcview %s: FAIL (%s: %s)" % (p, type(e).__name__, e))
            continue
        errs = [e for e in v.errors if e]
        if v.errors:
            bad += 1
            print("dmcview %s: FAIL (%d differences)" % (p, len(v.errors)))
            for e in errs[:30]:
                print("  " + e)
        else:
            print("dmcview %s: OK (%s)" % (p, ", ".join("%d %s" % (c, k) for k, c in v.counts.items())))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
