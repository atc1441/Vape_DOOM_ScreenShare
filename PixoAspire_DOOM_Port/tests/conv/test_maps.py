# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Round trip of every map record of a pack against an independent parse of the WAD.

Checks vertexes, linedefs, sidedefs, segs, ssectors, nodes, sectors, things,
blockmap and reject, the GRID start node property on a dense set of points per
cell, SNDADJ symmetry and block bits, TAGS lists, thing list order and rngk,
SECEXTRA against vanilla P_GroupLines, the mutable sector set and the ZCLIP
superset property.

usage: python3 -I tests/conv/test_maps.py [--zip FREEDOOM_ZIP | --wad IWAD] [--gen GEN] [PACK_DIR]
       (default build/packs/E1 and gen)
"""

import argparse
import json
import os
import random
import re
import struct
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, REPO)

from tools.conv import fetch, format as F  # noqa: E402
from tools.conv.reader import Image  # noqa: E402
from tools.conv.util import ConvError  # noqa: E402

MF_NOSECTOR, MF_NOBLOCKMAP = 0x8, 0x10
INT_MIN, INT_MAX = -(1 << 31), (1 << 31) - 1

# Vanilla specials (p_spec.c, p_switch.c) as this test reads them
SEC_CHANGING = {1, 2, 3, 4, 8, 9, 10, 12, 13, 14, 17}
LINE_MANUAL_DOORS = {1, 26, 27, 28, 31, 32, 33, 34, 117, 118}
LINE_NO_SECTOR_CHANGE = {11, 51, 52, 124, 48, 39, 97, 125, 126}
LINE_STAIRS = {7, 8, 100, 127}
LINE_DONUT = 9


class Fail(Exception):
    pass


def eq(a, b, what):
    if a != b:
        raise Fail("%s: %r != %r" % (what, a, b))


# ------------------------------------------------------------------ independent WAD parse

def wad_dir(data):
    n, off = struct.unpack_from("<ii", data, 4)
    out = []
    for i in range(n):
        p, s, nm = struct.unpack_from("<ii8s", data, off + 16 * i)
        out.append((nm.split(b"\0")[0].decode("latin-1").upper(), p, s))
    return out


def wad_map(data, dirs, name):
    i = [k for k, d in enumerate(dirs) if d[0] == name][-1]
    out = {}
    for k in range(1, 11):
        nm, p, s = dirs[i + k]
        out[nm] = data[p:p + s]
    return out


def rec(b, fmt):
    sz = struct.calcsize(fmt)
    return [struct.unpack_from(fmt, b, i) for i in range(0, len(b) - sz + 1, sz)]


def name8(b):
    return b.split(b"\0")[0].decode("latin-1").upper()


def mul16(a, b):
    v = (a * b) >> 16
    return ((v + (1 << 31)) & 0xFFFFFFFF) - (1 << 31)


def wrap32(v):
    return ((v + (1 << 31)) & 0xFFFFFFFF) - (1 << 31)


def side_of(x, y, n):
    """R_PointOnSide, fixed point point, node in map units."""
    nx, ny, ndx, ndy = n[0] << 16, n[1] << 16, n[2] << 16, n[3] << 16
    if ndx == 0:
        return int(ndy > 0) if x <= nx else int(ndy < 0)
    if ndy == 0:
        return int(ndx < 0) if y <= ny else int(ndx > 0)
    dx, dy = wrap32(x - nx), wrap32(y - ny)
    if (ndy ^ ndx ^ dx ^ dy) < 0:
        return int((ndy ^ dx) < 0)
    return 0 if mul16(dy, ndx >> 16) < mul16(ndy >> 16, dx) else 1


def sector_bbox(verts, lines, seclines):
    """P_GroupLines: M_ClearBox, then M_AddToBox with its else-if, in fixed point."""
    box = {"top": INT_MIN, "right": INT_MIN, "bottom": INT_MAX, "left": INT_MAX}
    for i in seclines:
        for v in (lines[i][0], lines[i][1]):
            x, y = verts[v][0] << 16, verts[v][1] << 16
            if x < box["left"]:
                box["left"] = x
            elif x > box["right"]:
                box["right"] = x
            if y < box["bottom"]:
                box["bottom"] = y
            elif y > box["top"]:
                box["top"] = y
    return box


def cdiv2(v):
    return -((-v) // 2) if v < 0 else v // 2


def crosses(box, x1, y1, x2, y2):
    """The line strictly separates two corners of box (top, bottom, left, right), map units."""
    top, bottom, left, right = box
    if right <= min(x1, x2) or left >= max(x1, x2) or top <= min(y1, y2) or bottom >= max(y1, y2):
        return False
    sides = {(x2 - x1) * (py - y1) - (y2 - y1) * (px - x1) > 0
             for px in (left, right) for py in (bottom, top)
             if (x2 - x1) * (py - y1) - (y2 - y1) * (px - x1) != 0}
    return len(sides) == 2


def walk(nodes, x, y, start):
    n, visits = start, 0
    while not n & 0x8000:
        nd = nodes[n]
        n = nd[12 + side_of(x, y, nd)]
        visits += 1
    return n & 0x7FFF, visits


# ------------------------------------------------------------------ checks

def check_map(im, mapname, data, dirs, names, up, mtype_names, stats):
    mb = im.map(mapname)
    h = mb.h
    L = wad_map(data, dirs, mapname)
    verts = rec(L["VERTEXES"], "<hh")
    lines = rec(L["LINEDEFS"], "<HHHHhhh")
    sides = [(r[0], r[1], name8(r[2]), name8(r[3]), name8(r[4]), r[5]) for r in rec(L["SIDEDEFS"], "<hh8s8s8sh")]
    secs = [(r[0], r[1], name8(r[2]), name8(r[3]), r[4], r[5], r[6]) for r in rec(L["SECTORS"], "<hh8s8shhh")]
    ssecs = rec(L["SSECTORS"], "<HH")
    segs = rec(L["SEGS"], "<HHhHhh")
    nodes = rec(L["NODES"], "<hhhhhhhhhhhhHH")
    things = rec(L["THINGS"], "<hhhhh")
    texxlat = mb.u16s("TEXXLAT")
    flatxlat = mb.u16s("FLATXLAT")
    tex_names = names["textures"]
    flat_names = names["flats"]
    first_tex = names["_first_texture"]

    def texname(idx):
        return "-" if idx == 0 else tex_names[texxlat[idx]]

    def wadtex(n):
        return "-" if n.startswith("-") or n == first_tex else n

    def flatname(idx):
        return "F_SKY1" if idx == 0 else flat_names[flatxlat[idx]]

    eq((h["nlines"], h["nsegs"], h["nssectors"], h["nnodes"], h["nsectors"]),
       (len(lines), len(segs), len(ssecs), len(nodes), len(secs)), mapname + " counts")

    # nodes and bboxes
    for i, (n, nb) in enumerate(zip(mb.records("NODES", F.NODE), mb.records("NODEBB", F.NODEBB))):
        w = nodes[i]
        eq((n["x"], n["y"], n["dx"], n["dy"], tuple(n["child"])), (w[0], w[1], w[2], w[3], (w[12], w[13])),
           "%s node %d" % (mapname, i))
        eq(tuple(nb["bbox"]), tuple(w[4:12]), "%s node bbox %d" % (mapname, i))

    # lines, sides, vertexes
    _, tag_secs, tag_vals = mb.tags()
    lrec = mb.records("LINES", F.LINE)
    lsides = mb.records("LINESIDES", F.LINESIDES)
    srec = mb.records("SIDES", F.SIDE)
    for i, l in enumerate(lines):
        r = lrec[i]
        x1, y1 = verts[l[0]]
        x2, y2 = verts[l[1]]
        eq((r["x1"], r["y1"], r["dx"], r["dy"]), (x1, y1, x2 - x1, y2 - y1), "%s line %d coords" % (mapname, i))
        front = sides[l[5]][5]
        back = sides[l[6]][5] if l[6] != -1 else 0xFFFF
        eq((r["front"], r["back"], r["flags"], r["special"]), (front, back, l[2], l[3]), "%s line %d" % (mapname, i))
        eq(tag_vals[r["tag"]] if r["tag"] else 0, l[4] if l[3] else 0, "%s line %d tag" % (mapname, i))
        for k in (0, 1):
            sd = l[5 + k]
            u = lsides[i]["side"][k]
            if sd == -1:
                eq(u, 0xFFFF, "%s line %d side %d" % (mapname, i, k))
                continue
            s = srec[u]
            w = sides[sd]
            eq((s["textureoffset"], s["rowoffset"], texname(s["top"]), texname(s["bottom"]), texname(s["mid"])),
               (w[0], w[1], wadtex(w[2]), wadtex(w[3]), wadtex(w[4])), "%s line %d side %d" % (mapname, i, k))
    for i, s in enumerate(mb.records("SEGS", F.SEG)):
        v1, v2, ang, ld, side, off = segs[i]
        l = lines[ld]
        back = sides[l[5 + (side ^ 1)]][5] if l[2] & 4 else 0xFFFF
        hasmid = wadtex(sides[l[5 + side]][4]) != "-"
        eq((s["v1x"], s["v1y"], s["v2x"], s["v2y"], s["angle"], s["offset"], s["line"] & 0x1FFF,
            bool(s["line"] & F.SEG_SIDE), bool(s["line"] & F.SEG_HASMID), s["backsector"]),
           (verts[v1][0], verts[v1][1], verts[v2][0], verts[v2][1], ang & 0xFFFF, off, ld, bool(side), hasmid,
            back), "%s seg %d" % (mapname, i))
    for i, s in enumerate(mb.records("SSECTORS", F.SSECTOR)):
        num, first = ssecs[i]
        _, _, _, ld, side, _ = segs[first]
        eq((s["firstseg"], s["numsegs"], s["sector"]), (first, num, sides[lines[ld][5 + side]][5]),
           "%s ssector %d" % (mapname, i))

    # sectors, line lists, tags
    secl = [[] for _ in secs]
    for i, l in enumerate(lines):
        f = sides[l[5]][5]
        secl[f].append(i)
        if l[6] != -1 and sides[l[6]][5] != f:
            secl[sides[l[6]][5]].append(i)
    sl = mb.u16s("SECLINES")
    for i, s in enumerate(mb.records("SECTORS", F.SECTOR)):
        w = secs[i]
        eq((s["floor"], s["ceiling"], flatname(s["floorpic"]), flatname(s["ceilingpic"]), s["light"], s["special"],
            tag_vals[s["tag"]] if s["tag"] else 0), w, "%s sector %d" % (mapname, i))
        eq(sl[s["firstline"]:s["firstline"] + s["linecount"]], secl[i], "%s sector %d lines" % (mapname, i))
    rng, tsecs, vals = mb.tags()
    for k in range(1, len(vals)):
        r = rng[k]
        eq(tsecs[r["first"]:r["first"] + r["count"]], [i for i, s in enumerate(secs) if s[6] == vals[k]],
           "%s tag %d list" % (mapname, vals[k]))

    # SECEXTRA: blockbox and soundorg of vanilla P_GroupLines
    ox, oy, cols, rows = struct.unpack_from("<hhhh", L["BLOCKMAP"], 0)
    blockbox = []
    for i, x in enumerate(mb.records("SECEXTRA", F.SECEXTRA)):
        b = sector_bbox(verts, lines, secl[i])
        top = wrap32(b["top"] - (oy << 16) + (32 << 16)) >> 23
        bottom = wrap32(b["bottom"] - (oy << 16) - (32 << 16)) >> 23
        right = wrap32(b["right"] - (ox << 16) + (32 << 16)) >> 23
        left = wrap32(b["left"] - (ox << 16) - (32 << 16)) >> 23
        bb = (min(top, rows - 1), max(bottom, 0), max(left, 0), min(right, cols - 1))
        blockbox.append(bb)
        so = (cdiv2(wrap32(b["right"] + b["left"])) >> 16, cdiv2(wrap32(b["top"] + b["bottom"])) >> 16)
        eq((tuple(x["blockbox"]), (x["soundorg_x"], x["soundorg_y"])), (bb, so), "%s secextra %d" % (mapname, i))

    # mutable sectors: every sector the game can change has an overlay slot
    overlay = {i for i, s in enumerate(mb.records("SECTORS", F.SECTOR)) if s["overlay"] != F.NONE8}
    need = {i for i, s in enumerate(secs) if s[5] in SEC_CHANGING}
    if mapname == "E1M8":
        need |= {i for i, s in enumerate(secs) if s[6] == 666}

    def other(li, sec):
        l = lines[li]
        if not l[2] & 4:
            return None
        f, b = sides[l[5]][5], sides[l[6]][5]
        return b if f == sec else f

    for li, l in enumerate(lines):
        if not l[3] or l[3] in LINE_NO_SECTOR_CHANGE:
            continue
        if l[3] in LINE_MANUAL_DOORS:
            if l[6] != -1:
                need.add(sides[l[6]][5])
            continue
        for t in [i for i, s in enumerate(secs) if s[6] == l[4]]:
            need.add(t)
            if l[3] == LINE_DONUT and secl[t]:
                ring = other(secl[t][0], t)
                if ring is not None:
                    need.add(ring)
            if l[3] in LINE_STAIRS:
                seen, todo = {t}, [t]
                while todo:
                    cur = todo.pop()
                    for k in secl[cur]:
                        n = lines[k]
                        if n[2] & 4 and sides[n[5]][5] == cur:
                            nxt = sides[n[6]][5]
                            if secs[nxt][2] == secs[t][2] and nxt not in seen:
                                seen.add(nxt)
                                todo.append(nxt)
                need |= seen
    eq(sorted(need - overlay), [], mapname + " sectors that change without an overlay slot")

    # blockmap, reject
    eq((h["bmap_ox"], h["bmap_oy"], h["bmap_cols"], h["bmap_rows"]), (ox, oy, cols, rows), mapname + " bmap hdr")
    eq(mb.stream("BLOCKMAP"), L["BLOCKMAP"][8:], mapname + " blockmap")
    nrej = (len(secs) ** 2 + 7) // 8
    eq(mb.stream("REJECT"), L["REJECT"][:nrej], mapname + " reject")

    # things (single-player spawnable, WAD order)
    ed_to_type = {}
    for i, mi in enumerate(up["mobjinfo"]):
        ed_to_type.setdefault(mi["doomednum"], i)
    sp = []
    for i, t in enumerate(things):
        x, y, ang, ed, opt = t
        if ed == 11 or ed <= 0 or 2 <= ed <= 4:
            continue
        if ed != 1 and opt & 16:
            continue
        sp.append((x, y, ang, ed, opt, 0 if ed == 1 else ed_to_type[ed]))
    trec = mb.records("THINGS", F.THING)
    eq(len(trec), len(sp), mapname + " thing count")
    root = len(nodes) - 1
    ssec_sector = [s["sector"] for s in mb.records("SSECTORS", F.SSECTOR)]
    kk = [0, 0, 0]
    for i, (t, r) in enumerate(zip(sp, trec)):
        x, y, ang, ed, opt, vt = t
        q = int(ang / 45)
        eq((r["x"], r["y"], r["angle45"], r["options"]), (x, y, q & 7, opt & 31), "%s thing %d" % (mapname, i))
        eq(mtype_names[r["type"]], up["mobj_names"][vt], "%s thing %d type" % (mapname, i))
        ss, _ = walk(nodes, x << 16, y << 16, root)
        eq(r["sector"], ssec_sector[ss], "%s thing %d sector" % (mapname, i))
        eq(tuple(r["rngk"]), tuple(v & 255 for v in kk), "%s thing %d rngk" % (mapname, i))
        for g, bit in enumerate((1, 2, 4)):
            if ed == 1 or opt & bit:
                kk[g] += 1
                st = up["states"][up["mobjinfo"][vt]["spawnstate"]]
                if ed != 1 and st[2] > 0:
                    kk[g] += 1

    # cell and sector thing lists in reverse spawn order
    cells = [[] for _ in range(cols * rows)]
    seclist = [[] for _ in secs]
    for i in reversed(range(len(sp))):
        fl = up["mobjinfo"][sp[i][5]]["flags"]
        if not fl & MF_NOBLOCKMAP:
            cx, cy = (sp[i][0] - ox) >> 7, (sp[i][1] - oy) >> 7
            if 0 <= cx < cols and 0 <= cy < rows:
                cells[cy * cols + cx].append(i)
        if not fl & MF_NOSECTOR:
            seclist[trec[i]["sector"]].append(i)
    eq(mb.first_lists("CELLTHINGS", cols * rows), cells, mapname + " cell things")
    eq(mb.first_lists("SECTHINGS", len(secs)), seclist, mapname + " sector things")
    look = [i for i, t in enumerate(sp) if up["states"][up["mobjinfo"][t[5]]["spawnstate"]][3] == "A_Look"]
    eq(mb.u16s("MONSTERS"), look, mapname + " monsters")
    tele = [(trec[i]["sector"], i) for i, t in enumerate(sp) if up["mobj_names"][t[5]] == "MT_TELEPORTMAN"]
    eq([(r["sector"], r["thing"]) for r in mb.records("TELEDEST", F.TELEDEST)], sorted(tele), mapname + " teledest")
    eq(mb.u16s("SPECIALS"), [i for i, l in enumerate(lines) if l[3] == 48], mapname + " scrollers")

    # ZCLIP holds every thing P_ChangeSector can reach (blockbox cells of a changing sector)
    # that stands in a changing sector or crosses a line of one
    zclip = set(mb.u16s("ZCLIP")[1:])
    cells = set()
    for sec in overlay:
        top, bottom, left, right = blockbox[sec]
        cells |= {(cx, cy) for cx in range(left, right + 1) for cy in range(bottom, top + 1)}
    mlines = [(verts[l[0]], verts[l[1]]) for l in lines
              if sides[l[5]][5] in overlay or (l[6] != -1 and sides[l[6]][5] in overlay)]
    missing = []
    nz = 0
    for i, t in enumerate(sp):
        mi = up["mobjinfo"][t[5]]
        if t[3] == 1 or mi["flags"] & MF_NOBLOCKMAP or ((t[0] - ox) >> 7, (t[1] - oy) >> 7) not in cells:
            continue
        r = mi["radius"] >> 16
        box = (t[1] + r, t[1] - r, t[0] - r, t[0] + r)
        if trec[i]["sector"] in overlay or any(crosses(box, a[0], a[1], b[0], b[1]) for a, b in mlines):
            nz += 1
            if i not in zclip:
                missing.append(i)
    eq(missing, [], mapname + " things missing from ZCLIP")

    # SNDADJ: symmetric, block bit set iff every joining two-sided line blocks sound
    rng, ents = mb.ranges_and_list("SNDADJ", len(secs))
    adj = [dict((v & 0x7FFF, bool(v & 0x8000)) for v in ents[r["first"]:r["first"] + r["count"]]) for r in rng]
    want = [dict() for _ in secs]
    for l in lines:
        if not l[2] & 4:
            continue
        a, b = sides[l[5]][5], sides[l[6]][5]
        if a == b:
            continue
        blk = bool(l[2] & 64)
        for p, q in ((a, b), (b, a)):
            want[p][q] = want[p].get(q, True) and blk
    for s in range(len(secs)):
        eq(adj[s], want[s], "%s sndadj sector %d" % (mapname, s))
        for o, blk in adj[s].items():
            eq(adj[o].get(s), blk, "%s sndadj symmetry %d-%d" % (mapname, s, o))

    # GRID: walking from the start node gives the root walk's subsector for dense points per cell
    grid = mb.u16s("GRID")
    rnd = random.Random(1234 + int(mapname[3]))
    npts = vroot = vgrid = 0
    for cy in range(rows):
        for cx in range(cols):
            g = grid[cy * cols + cx]
            x0 = (ox + (cx << 7)) << 16
            y0 = (oy + (cy << 7)) << 16
            pts = [(x0 + (i * ((128 << 16) - 1)) // 5, y0 + (j * ((128 << 16) - 1)) // 5)
                   for i in range(6) for j in range(6)]
            pts += [(x0 + rnd.randrange(128 << 16), y0 + rnd.randrange(128 << 16)) for _ in range(12)]
            for x, y in pts:
                a, va = walk(nodes, x, y, root)
                b, vb = walk(nodes, x, y, g)
                if a != b:
                    raise Fail("%s grid cell %d,%d point %x,%x: %d != %d" % (mapname, cx, cy, x, y, a, b))
                npts += 1
                vroot += va
                vgrid += vb
    stats[mapname] = {"grid_points": npts, "root_visits": vroot / npts, "grid_visits": vgrid / npts,
                      "zclip": len(zclip), "need_zclip": nz, "mutable": len(overlay), "need_mutable": len(need)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--zip")
    ap.add_argument("--wad", help="local IWAD the pack was made from, instead of the Freedoom zip")
    ap.add_argument("--gen", default=os.path.join(REPO, "gen"), help="generated sources of its build")
    ap.add_argument("pack", nargs="?", default=os.path.join(REPO, "build", "packs", "E1"))
    a = ap.parse_args()
    try:
        data = fetch.read_wad(a.wad) if a.wad else fetch.extract(a.zip)[0]
    except ConvError as e:
        raise Fail(str(e))
    dirs = wad_dir(data)
    with open(os.path.join(a.pack, "names.json")) as f:
        names = json.load(f)
    t1 = [d for d in dirs if d[0] == "TEXTURE1"][-1]
    o = struct.unpack_from("<i", data, t1[1] + 4)[0]
    names["_first_texture"] = name8(data[t1[1] + o:t1[1] + o + 8])
    with open(os.path.join(REPO, "build", "conv", "upstream.json")) as f:
        up = json.load(f)
    with open(os.path.join(a.gen, "info.h")) as f:
        txt = f.read()
    enum = txt[txt.index("} spritenum_t;"):txt.index("} mobjtype_t;")]
    mtype_names = {int(v): n for n, v in re.findall(r"(MT_\w+) = (\d+),", enum)}
    im = Image(a.pack)
    stats = {}
    for e in im.maps:
        check_map(im, e["name"], data, dirs, names, up, mtype_names, stats)
        s = stats[e["name"]]
        print("%s: records OK, grid %d points, node visits per lookup %.2f root -> %.2f grid, "
              "mutable %d (%d required), zclip %d (%d required)" % (
                  e["name"], s["grid_points"], s["root_visits"], s["grid_visits"], s["mutable"],
                  s["need_mutable"], s["zclip"], s["need_zclip"]))
    print("PASS test_maps (%s, %d maps)" % (a.pack, len(im.maps)))


if __name__ == "__main__":
    try:
        main()
    except Fail as e:
        print("FAIL: %s" % e)
        sys.exit(1)
