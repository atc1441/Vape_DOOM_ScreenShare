# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Structure, CRC, bounds and alignment checks of a DMC DOOM pack.

usage: python3 -I tools/dmccheck.py [--save FILE]... [PACK_DIR ...]

A pack holds no SAVE.DAT, the firmware makes it. --save checks a SAVE.DAT read back from a
meter: 65,536 B, every slot empty (its header erased) or with the save magic, a body that
fits the slot and matches its CRC, and a description of the slot's map in STCFN characters.
"""

import argparse
import hashlib
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from tools.conv import format as F  # noqa: E402
from tools.conv.reader import Image, decode_posts, patch_columns  # noqa: E402
from tools.conv.util import ConvError, crc32  # noqa: E402


class Checker:
    def __init__(self, path):
        self.path = path
        self.errors = []
        self.checks = 0

    def ok(self, cond, msg, *args):
        self.checks += 1
        if not cond:
            self.errors.append(msg % args if args else msg)
        return cond

    def run(self):
        im = Image(self.path)
        h = im.hdr
        ok = self.ok
        ok(h["fmt_major"] == F.FMT_MAJOR, "fmt_major %d", h["fmt_major"])
        ok(im.header_crc_ok(), "header CRC")
        ok(h["image_size"] == len(im.img), "image_size %d vs %d", h["image_size"], len(im.img))
        ok(h["split"] <= h["image_size"], "split beyond image")
        ok(h["split"] == h["image_size"] or h["split"] % F.SPLIT_ALIGN == 0, "split alignment")
        ok(len(im.doom) == h["split"], "DOOM.DAT size %d vs split %d", len(im.doom), h["split"])
        ok(len(im.doom) <= F.DOOMDAT_CAP, "DOOM.DAT over capacity")
        ok(0 < h["nsect"] <= F.MAX_SECTS and 0 < h["nmaps"] <= F.MAX_MAPS, "table counts")
        ok(list(h["skill_group"]) == list(F.SKILL_GROUP), "skill group map")
        z = bytearray(im.img)
        for name, n in (("image_uuid", 16), ("header_crc", 4)):
            o = F.HEADER.offsets[name]
            z[o:o + n] = bytes(n)
        ok(hashlib.sha256(bytes(z)).digest()[:16] == bytes(h["image_uuid"]), "image_uuid")
        if im.doomx is not None:
            x = im.xhdr
            ok(x["magic"] == F.MAGIC_X, "DOOMX magic")
            ok(bytes(x["image_uuid"]) == bytes(h["image_uuid"]), "DOOMX uuid differs from DOOM.DAT")
            ok(x["logical_start"] == h["split"], "DOOMX logical_start")
            ok(x["length"] == len(im.doomx) - F.DOOMX_HDR, "DOOMX length")
            ok(crc32(im.doomx[F.DOOMX_HDR:]) == x["payload_crc"], "DOOMX payload CRC")
            ok(len(im.doomx) <= F.DOOMX_CAP, "DOOMX.DAT over capacity")
        else:
            ok(not os.path.exists(os.path.join(self.path, "DOOMX.DAT")), "stale DOOMX.DAT")
        ok(not os.path.exists(os.path.join(self.path, "SAVE.DAT")), "SAVE.DAT in the pack")

        # sections: known ids, alignment, bounds, CRC, no overlap
        spans = [(0, F.HEADER_SIZE, "header")]
        for sid, e in sorted(im.sects.items()):
            name = dict(F.SECTIONS).get(sid)
            ok(name is not None, "unknown section id %d", sid)
            ok(e["off"] % F.ALIGN == 0, "section %s misaligned", name)
            ok(e["off"] + e["size"] <= len(im.img), "section %s out of bounds", name)
            ok(crc32(im.img[e["off"]:e["off"] + e["size"]]) == e["crc"], "section %s CRC", name)
            spans.append((e["off"], e["off"] + e["size"], name))
        nums = {e["map"] for e in im.maps}
        for e in im.maps:
            # vanilla episode 1 order, a target outside the pack must be marked as such
            n = e["map"]
            for field, want in (("next", F.NEXT_END_EPISODE if n == 8 else 4 if n == 9 else n + 1),
                                ("secret_next", 9)):
                if want != F.NEXT_END_EPISODE and want not in nums:
                    want = F.NEXT_NOT_IN_PACK
                ok(e[field] == want, "map %s %s %d, expected %d", e["name"], field, e[field], want)
            ok(e["name"] == "E%dM%d" % (e["episode"], n), "map %s numbers", e["name"])
            ok(e["off"] % F.ALIGN == 0, "map %s misaligned", e["name"])
            ok(e["off"] + e["size"] <= len(im.img), "map %s out of bounds", e["name"])
            ok(crc32(im.img[e["off"]:e["off"] + e["size"]]) == e["crc"], "map %s CRC", e["name"])
            spans.append((e["off"], e["off"] + e["size"], e["name"]))
        spans.sort()
        for a, b in zip(spans, spans[1:]):
            ok(a[1] <= b[0], "%s overlaps %s", a[2], b[2])
        for name in dict(F.SECTIONS).values():
            ok(F.SECT[name] in im.sects, "section %s missing", name)
        if self.errors:
            return im
        self.sections(im)
        for e in im.maps:
            self.map(im, e)
        return im

    def save(self):
        """Checks of a SAVE.DAT. Returns the description for the summary line."""
        ok = self.ok
        with open(self.path, "rb") as f:
            sv = f.read()
        ok(len(sv) == F.SAVE_SIZE, "size %d, expected %d", len(sv), F.SAVE_SIZE)
        slot = F.SAVE_SIZE // F.SAVE_SLOTS
        used = 0
        for k in range(F.SAVE_SLOTS):
            o = k * slot + F.SAVE_HEADER.size
            raw = sv[k * slot:o]
            if len(raw) < F.SAVE_HEADER.size or raw == b"\xff" * len(raw):
                continue
            used += 1
            h = F.SAVE_HEADER.unpack(raw)
            if not ok(h["magic"] == F.MAGIC_SAVE, "slot %d magic %r", k, h["magic"]) or \
                    not ok(h["size"] <= slot - F.SAVE_HEADER.size, "slot %d size %d", k, h["size"]):
                continue
            body = sv[o:o + h["size"]]
            ok(len(body) == h["size"] and crc32(body) == h["crc"], "slot %d body CRC", k)
            desc = h["desc"].split(b"\0", 1)[0]
            ok(desc.startswith(b"E1M%d " % h["map"]) and all(c == 32 or 33 <= c <= 95 for c in desc),
               "slot %d description %r for map %d", k, desc, h["map"])
        return "%d B, %d of %d slots in use" % (len(sv), used, F.SAVE_SLOTS)

    def sections(self, im):
        ok = self.ok
        pal, _ = im.section("PALETTE")
        ok(len(pal) == F.PALETTE_COUNT * 512, "PALETTE size")
        texdir, _ = im.section("TEXDIR")
        texdata, _ = im.section("TEXDATA")
        ntex = F.TEXDIR_HDR.unpack(texdir)["count"]
        self.ntex = ntex
        ok(len(texdir) == 16 + 16 * ntex, "TEXDIR size")
        for k in range(ntex):
            t = F.TEXENT.unpack(texdir, 16 + 16 * k)
            base = (t["flags"] & F.TF_BASE_MASK) >> F.TF_BASE_SHIFT
            w2, sh = 1 << t["log2_w"], 1 << t["log2_sh"]
            want_sh = 1
            while want_sh < min(t["height"], 128):
                want_sh <<= 1
            ok(w2 <= t["width"] < 2 * w2 and sh == want_sh, "texture %d geometry", k)
            ok(t["data"] % F.ALIGN == 0, "texture %d data misaligned", k)
            size = sum((w2 >> m) * (sh >> m) for m in range(base, base + t["nlevels"]))
            ok(t["data"] + size <= len(texdata), "texture %d levels out of bounds", k)
            ok(bool(t["flags"] & F.TF_MIPS) == (t["nlevels"] > 1), "texture %d mips flag", k)
            if t["flags"] & F.TF_MASKED:
                ok(t["colinfo"] % 4 == 0 and t["colinfo"] + 4 * (w2 >> base) <= len(texdata),
                   "texture %d colinfo bounds", k)
                for j in range(w2 >> base):
                    ci = struct.unpack_from("<I", texdata, t["colinfo"] + 4 * j)[0]
                    off = t["data"] + (ci & F.COLINFO_OFF_MASK)
                    try:
                        decode_posts(texdata, off, ci >> F.COLINFO_LEN_SHIFT)
                    except ConvError as e:
                        ok(False, "texture %d column %d: %s", k, j, e)
            else:
                ok(t["nlevels"] > 0, "texture %d has no data", k)
        flats, fe = im.section("FLATS")
        nflat = fe["flags"]
        self.nflat = nflat
        ok(len(flats) == F.finale_flat_off(nflat) + F.FINALE_FLAT_SIZE, "FLATS size")
        pdir, _ = im.section("PATCHDIR")
        pdata, _ = im.section("PATCHDATA")
        npatch = struct.unpack_from("<I", pdir, 0)[0]
        self.npatch = npatch
        ok(len(pdir) == 4 + 4 * npatch, "PATCHDIR size")
        offs = struct.unpack_from("<%dI" % npatch, pdir, 4)
        for k, o in enumerate(offs):
            ok(o % F.ALIGN == 0 and o < len(pdata), "patch %d offset", k)
            end = offs[k + 1] if k + 1 < npatch else len(pdata)
            try:
                ph, cols = patch_columns(pdata[o:end])
                ok(ph["ncols"] == (ph["width"] + (1 << ph["xshift"]) - 1) >> ph["xshift"], "patch %d ncols", k)
                rows = (ph["height"] + 1) // 2 if ph["flags"] & F.PF_HALF_ROWS else ph["height"]
                if ph["flags"] & F.PF_OPAQUE:
                    ok(all(len(c) == 1 and c[0][0] == 0 and len(c[0][1]) == rows for c in cols),
                       "patch %d opaque flag", k)
            except (ConvError, struct.error) as e:
                ok(False, "patch %d: %s", k, e)
        spr, _ = im.section("SPRDEF")
        sh_ = F.SPRDEF_HDR.unpack(spr)
        ok(sh_["frames_off"] % F.ALIGN == 0 and sh_["frames_off"] + 16 * sh_["nframes"] == len(spr), "SPRDEF size")
        for s in range(sh_["nsprites"]):
            e = F.SPRITE.unpack(spr, 16 + 4 * s)
            ok(e["first"] + e["numframes"] <= sh_["nframes"], "sprite %d frames", s)
        for fidx in range(sh_["nframes"]):
            fr = F.SPRFRAME.unpack(spr, sh_["frames_off"] + 16 * fidx)
            for v in fr["lump"]:
                p = v & F.SPR_PATCH_MASK
                ok(p == F.SPR_NO_PATCH or p < npatch, "sprite frame %d patch %d", fidx, p)
        snd, _ = im.section("SOUNDS")
        cnt = F.SOUNDS_HDR.unpack(snd)["count"]
        for k in range(cnt):
            e = F.SOUND.unpack(snd, 8 + 8 * k)
            ok(e["off"] + e["len"] <= len(snd), "sound %d bounds", k)
        st, _ = im.section("STRINGS")
        n = F.LIST_HDR.unpack(st)["count"]
        ok(n == len(F.STRING_IDS), "STRINGS count %d", n)
        for k in range(n):
            o = struct.unpack_from("<I", st, 4 + 4 * k)[0]
            ok(o < len(st) and st.find(b"\0", o) >= 0, "string %d", k)
        uid, ue = im.section("UIDIR")
        ok(ue["flags"] == len(F.UI_NAMES) and len(uid) == 2 * len(F.UI_NAMES), "UIDIR size")
        for k, v in enumerate(struct.unpack_from("<%dH" % ue["flags"], uid)):
            ok(v == F.NONE16 or v < npatch, "UI %s patch", F.UI_NAMES[k])
        an, ae = im.section("ANIMS")
        ok(len(an) == 8 * ae["flags"], "ANIMS size")
        for k in range(ae["flags"]):
            a = F.ANIM.unpack(an, 8 * k)
            lim = nflat if a["kind"] == F.ANIM_FLAT else ntex
            ok(a["first"] < a["last"] < lim, "anim %d range", k)
            if a["kind"] == F.ANIM_SWITCH:
                ok(a["first"] % 2 == 0 and a["last"] == a["first"] + 1, "switch pair %d not tex ^ 1", k)
        dm, _ = im.section("DEMOS")
        n = F.LIST_HDR.unpack(dm)["count"]
        for k in range(n):
            o = struct.unpack_from("<I", dm, 4 + 4 * k)[0]
            ok(o < len(dm), "demo %d offset", k)
        ok(bool(im.hdr["flags"] & F.HF_DEMO1) == (n > 0), "DEMO1 flag")

    def map(self, im, e):
        ok = self.ok
        mb = im.map(e["name"])
        h = mb.h
        nm = e["name"]
        ok(h["magic"] == F.MAGIC_MAP, "%s magic", nm)
        ok(e["sky_texture"] < self.ntex, "%s sky", nm)
        for k, name in enumerate(F.STREAMS):
            o, n = h["stream_off"][k], h["stream_size"][k]
            ok(o % F.ALIGN == 0 and o >= F.MAPHDR.size and o + n <= len(mb.block), "%s stream %s", nm, name)
        sizes = {"NODES": 16 * h["nnodes"], "NODEBB": 16 * h["nnodes"], "SSECTORS": 8 * h["nssectors"],
                 "SEGS": 16 * h["nsegs"], "LINES": 16 * h["nlines"], "LINESIDES": 4 * h["nlines"],
                 "SIDES": 8 * h["nsides"], "SECTORS": 16 * h["nsectors"], "SECEXTRA": 8 * h["nsectors"],
                 "SECLINES": 2 * h["nseclines"], "GRID": 2 * h["bmap_cols"] * h["bmap_rows"],
                 "REJECT": (h["nsectors"] ** 2 + 7) // 8, "SNDADJ": 4 * h["nsectors"] + 2 * h["nsndadj"],
                 "THINGS": 12 * h["nthings"],
                 "CELLTHINGS": 2 * (h["bmap_cols"] * h["bmap_rows"] + 1) + 2 * h["ncellthings"],
                 "SECTHINGS": 2 * (h["nsectors"] + 1) + 2 * h["nsecthings"], "ZCLIP": 2 + 2 * h["nzclip"],
                 "TELEDEST": 4 * h["nteledest"], "SPECIALS": 2 * h["nscrollers"],
                 "MONSTERS": 2 * len(mb.u16s("MONSTERS")), "TEXXLAT": 2 * (h["ntex"] + 1),
                 "FLATXLAT": 2 * (h["nflat"] + 1)}
        for name, sz in sizes.items():
            ok(h["stream_size"][F.STREAM[name]] == sz, "%s stream %s size", nm, name)
        nsec, nl, nsd, nseg, nss, nn = (h["nsectors"], h["nlines"], h["nsides"], h["nsegs"], h["nssectors"],
                                        h["nnodes"])
        ok(h["root"] == nn - 1, "%s root", nm)

        def child_ok(c):
            return (c & ~F.NF_SUBSECTOR) < nss if c & F.NF_SUBSECTOR else c < nn
        for r in mb.records("NODES", F.NODE):
            ok(all(child_ok(c) for c in r["child"]), "%s node child", nm)
        for r in mb.records("SSECTORS", F.SSECTOR):
            ok(r["numsegs"] > 0 and r["firstseg"] + r["numsegs"] <= nseg and r["sector"] < nsec, "%s ssector", nm)
        for r in mb.records("SEGS", F.SEG):
            ok((r["line"] & F.SEG_LINE_MASK) < nl and (r["backsector"] == F.NONE16 or r["backsector"] < nsec),
               "%s seg", nm)
        nslot = 0
        for r in mb.records("LINES", F.LINE):
            ok(r["front"] < nsec and (r["back"] == F.NONE16 or r["back"] < nsec), "%s line sectors", nm)
            ok(r["tag"] <= h["ntags"], "%s line tag", nm)
            ok((r["slot"] == F.NONE8) == (r["special"] == 0), "%s line slot", nm)
            if r["slot"] != F.NONE8:
                ok(r["slot"] == nslot, "%s special slots not dense", nm)
                nslot += 1
        ok(nslot == h["nspeclines"], "%s special line count", nm)
        for r in mb.records("LINESIDES", F.LINESIDES):
            ok(r["side"][0] < nsd and (r["side"][1] == F.NONE16 or r["side"][1] < nsd), "%s linesides", nm)
        sw = set()
        for r in mb.records("SIDES", F.SIDE):
            ok(max(r["top"], r["bottom"], r["mid"]) <= h["ntex"], "%s side texture", nm)
            if r["switch_slot"] != F.NONE8:
                ok(r["switch_slot"] < h["nswitches"] and r["switch_slot"] not in sw, "%s switch slot", nm)
                sw.add(r["switch_slot"])
        ok(len(sw) == h["nswitches"], "%s switch count", nm)
        ov = set()
        for r in mb.records("SECTORS", F.SECTOR):
            ok(max(r["floorpic"], r["ceilingpic"]) <= h["nflat"], "%s sector flat", nm)
            ok(r["tag"] <= h["ntags"] and r["firstline"] + r["linecount"] <= h["nseclines"], "%s sector", nm)
            if r["overlay"] != F.NONE8:
                ov.add(r["overlay"])
        ok(ov == set(range(h["nmutable"])), "%s overlay slots", nm)
        ok(all(v < nl for v in mb.u16s("SECLINES")), "%s seclines", nm)
        rng, secs, vals = mb.tags()
        ok(all(s < nsec for s in secs) and vals[0] == 0 and vals[1:] == sorted(vals[1:]), "%s tags", nm)
        ok(all(r["first"] + r["count"] <= len(secs) for r in rng), "%s tag ranges", nm)
        ncell = h["bmap_cols"] * h["bmap_rows"]
        for g in mb.u16s("GRID"):
            ok(child_ok(g), "%s grid entry", nm)
        bm = mb.stream("BLOCKMAP")
        nbm = len(bm) // 2
        words = struct.unpack_from("<%dH" % nbm, bm, 0)
        for o in words[:ncell]:
            p = o - 4
            if not ok(ncell <= p < nbm, "%s blockmap offset", nm):
                continue
            while p < nbm and words[p] != F.NONE16:
                p += 1
            ok(p < nbm, "%s blockmap list at %d runs past the stream", nm, o)
            ok(all(v < nl for v in words[o - 4:p]), "%s blockmap list at %d line index", nm, o)
        rng, ents = mb.ranges_and_list("SNDADJ", nsec)
        ok(all(r["first"] + r["count"] <= len(ents) for r in rng), "%s sndadj ranges", nm)
        ok(all((v & ~F.SNDADJ_BLOCK) < nsec for v in ents), "%s sndadj entries", nm)
        nth = h["nthings"]
        for r in mb.records("THINGS", F.THING):
            ok(r["sector"] < nsec and r["angle45"] < 8 and r["options"] < 32, "%s thing", nm)
        for name, n in (("CELLTHINGS", ncell), ("SECTHINGS", nsec)):
            for L in mb.first_lists(name, n):
                ok(all(v < nth for v in L) and L == sorted(L, reverse=True), "%s %s order", nm, name)
        z = mb.u16s("ZCLIP")
        ok(z[0] == h["nzclip"] and all(v < nth for v in z[1:]), "%s zclip", nm)
        td = mb.records("TELEDEST", F.TELEDEST)
        ok([(r["sector"], r["thing"]) for r in td] == sorted((r["sector"], r["thing"]) for r in td), "%s teledest", nm)
        ok(all(v < nl for v in mb.u16s("SPECIALS")), "%s specials", nm)
        mon = mb.u16s("MONSTERS")
        ok(mon == sorted(mon) and all(v < nth for v in mon), "%s monsters", nm)
        tx, fx = mb.u16s("TEXXLAT"), mb.u16s("FLATXLAT")
        ok(tx[0] == F.NONE16 and all(v < self.ntex for v in tx[1:]), "%s texxlat", nm)
        ok(fx[0] == F.NONE16 and all(v < self.nflat for v in fx[1:]), "%s flatxlat", nm)
        for g, p in enumerate(mb.plans):
            ok(p["plan"] < 4 and p["pages"] >= p["need_play"] + p["reserve"], "%s plan %d", nm, g)
            ok(p["monsters"] == h["nmonsters"][g] and p["zclip_slots"] == h["nzclip"], "%s plan %d fields", nm, g)


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--save", action="append", default=[], metavar="FILE", help="a SAVE.DAT to check")
    ap.add_argument("packs", nargs="*", metavar="PACK_DIR")
    a = ap.parse_args(argv[1:])
    if not a.packs and not a.save:
        print(__doc__.strip())
        return 2
    bad = 0
    for path, is_save in [(p, True) for p in a.save] + [(p, False) for p in a.packs]:
        c = Checker(path)
        try:
            if is_save:
                desc = c.save()
            else:
                im = c.run()
                desc = "%d B, %d sections, %d maps" % (len(im.img), len(im.sects), len(im.maps))
        except (ConvError, OSError, struct.error) as e:
            c.errors.append(str(e))
            desc = "unreadable"
        if c.errors:
            bad += 1
            print("dmccheck %s: FAIL (%d errors of %d checks, %s)" % (path, len(c.errors), c.checks, desc))
            for e in c.errors[:50]:
                print("  " + e)
        else:
            print("dmccheck %s: OK (%d checks, %s)" % (path, c.checks, desc))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
