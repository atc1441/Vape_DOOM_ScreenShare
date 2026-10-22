# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Map blocks: records, streams and derived per-map sets.

analyze() parses one map with vanilla semantics and computes everything that
does not depend on the pack's texture and flat numbering. encode() writes the
block once the pack has assigned global texture and flat ids.
"""

import struct

from . import format as F
from . import info as I
from . import wadread
from .util import align, cdiv, check, fixed_mul, s32

FRACBITS = 16
MAXRADIUS = 32
ML_BLOCKING = 1
ML_TWOSIDED = 4
ML_SOUNDBLOCK = 64

# Vanilla line specials by what they touch.
MANUAL_DOORS = {1, 26, 27, 28, 31, 32, 33, 34, 117, 118}
TELEPORTS = {39, 97, 125, 126}
EXITS = {11, 51, 52, 124}
SCROLLERS = {48}
STAIRS = {7, 8, 100, 127}
DONUTS = {9}
# Specials that change the floor flat of their tagged sectors (EV_DoPlat and EV_DoFloor "change" types)
FLOORPIC_CHANGERS = {14, 15, 20, 22, 37, 47, 59, 66, 67, 68, 84, 93, 95}
STROBE_START = {17}
SHOOT = {24, 46, 47}
KNOWN_SPECIALS = (
    MANUAL_DOORS | TELEPORTS | EXITS | SCROLLERS | SHOOT |
    {2, 3, 4, 5, 6, 8, 10, 12, 13, 16, 17, 19, 22, 25, 30, 35, 36, 37, 38, 40, 44, 53, 54, 56,
     57, 58, 59, 100, 104, 108, 109, 110, 119, 121, 130, 141,
     72, 73, 74, 75, 76, 77, 79, 80, 81, 82, 83, 84, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
     98, 105, 106, 107, 120, 128, 129,
     7, 9, 14, 15, 18, 20, 21, 23, 29, 41, 49, 50, 55, 71, 101, 102, 103, 111, 112, 113, 122,
     127, 131, 133, 135, 137, 140,
     42, 43, 45, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 99, 114, 115, 116, 123, 132, 134,
     136, 138, 139})
TAGLESS = MANUAL_DOORS | EXITS | SCROLLERS
SECTOR_MUTABLE_SPECIALS = {1, 2, 3, 4, 8, 9, 10, 12, 13, 14, 17}
SECTOR_LIGHT_SPECIALS = {1, 2, 3, 4, 8, 12, 13, 17}
SECTOR_KNOWN_SPECIALS = SECTOR_MUTABLE_SPECIALS | {0, 5, 7, 11, 16}
BOSS_TAGS = {"E1M8": 666}
# Every mover speed in the game code, in fixed point: all on the 1/8 unit grid.
MOVER_SPEEDS = (1 << 16, 2 << 16, 8 << 16, (1 << 16) // 8, (1 << 16) // 4, (1 << 16) // 2, 4 << 16,
                8 << 16, 16 << 16)
MAX_TAGS = 255
MAX_BSP_DEPTH = 64
MAXLINEANIMS = 64


# ------------------------------------------------------------------ vanilla geometry

def point_on_side(x, y, node):
    """R_PointOnSide with fixed point inputs. node = (x, y, dx, dy) in map units."""
    nx, ny, ndx, ndy = (v << FRACBITS for v in node[:4])
    if not ndx:
        if x <= nx:
            return 1 if ndy > 0 else 0
        return 1 if ndy < 0 else 0
    if not ndy:
        if y <= ny:
            return 1 if ndx < 0 else 0
        return 1 if ndx > 0 else 0
    dx = s32(x - nx)
    dy = s32(y - ny)
    if (ndy ^ ndx ^ dx ^ dy) < 0:
        return 1 if (ndy ^ dx) < 0 else 0
    left = fixed_mul(ndy >> FRACBITS, dx)
    right = fixed_mul(dy, ndx >> FRACBITS)
    return 0 if right < left else 1


def point_in_subsector(nodes, x, y, start=None):
    n = len(nodes) - 1 if start is None else start
    if not nodes:
        return 0
    while not n & F.NF_SUBSECTOR:
        nd = nodes[n]
        n = nd[6][point_on_side(x, y, nd)]
    return n & ~F.NF_SUBSECTOR


def box_on_line_side(box, x1, y1, dx, dy):
    """P_BoxOnLineSide on map unit values. box = (top, bottom, left, right)."""
    top, bottom, left, right = box
    if dy == 0:
        p1 = 1 if top > y1 else 0
        p2 = 1 if bottom > y1 else 0
        if dx < 0:
            p1 ^= 1
            p2 ^= 1
    elif dx == 0:
        p1 = 1 if right < x1 else 0
        p2 = 1 if left < x1 else 0
        if dy < 0:
            p1 ^= 1
            p2 ^= 1
    elif (dy > 0) == (dx > 0):
        p1 = point_on_line_side(left, top, x1, y1, dx, dy)
        p2 = point_on_line_side(right, bottom, x1, y1, dx, dy)
    else:
        p1 = point_on_line_side(right, top, x1, y1, dx, dy)
        p2 = point_on_line_side(left, bottom, x1, y1, dx, dy)
    return p1 if p1 == p2 else -1


def point_on_line_side(x, y, x1, y1, ldx, ldy):
    """P_PointOnLineSide with map unit point and line, all converted to fixed."""
    x, y, x1, y1, ldx, ldy = (v << FRACBITS for v in (x, y, x1, y1, ldx, ldy))
    if not ldx:
        if x <= x1:
            return 1 if ldy > 0 else 0
        return 1 if ldy < 0 else 0
    if not ldy:
        if y <= y1:
            return 1 if ldx < 0 else 0
        return 1 if ldx > 0 else 0
    dx = s32(x - x1)
    dy = s32(y - y1)
    left = fixed_mul(ldy >> FRACBITS, dx)
    right = fixed_mul(dy, ldx >> FRACBITS)
    return 0 if right < left else 1


# ------------------------------------------------------------------ analysis

class MapInfo:
    pass


def tex_number(texidx, name):
    """R_TextureNumForName: '-' is 0, otherwise the first texture with that name."""
    if name.startswith("-"):
        return 0
    n = texidx.get(name.upper())
    check(n is not None, "texture %s not found", name)
    return n


def analyze(wad, mapname, inf, dense, tex_first):
    """Parse and derive everything for one map. tex_first: name -> first vanilla texture number."""
    m = MapInfo()
    m.name = mapname
    L = wad.map_lumps(mapname)
    verts = wadread.parse_vertexes(L["VERTEXES"])
    lines = wadread.parse_linedefs(L["LINEDEFS"])
    sides = wadread.parse_sidedefs(L["SIDEDEFS"])
    secs = wadread.parse_sectors(L["SECTORS"])
    ssecs = wadread.parse_ssectors(L["SSECTORS"])
    segs = wadread.parse_segs(L["SEGS"])
    nodes = wadread.parse_nodes(L["NODES"])
    m.verts, m.lines, m.sides, m.secs, m.ssecs, m.segs, m.nodes = verts, lines, sides, secs, ssecs, segs, nodes
    m.blockmap = L["BLOCKMAP"]
    m.ox, m.oy, m.cols, m.rows = wadread.parse_blockmap(m.blockmap)
    nsec = len(secs)

    check(len(lines) <= F.SEG_LINE_MASK + 1, "%s: too many lines", mapname)
    check(len(segs) <= 8191, "%s: more than 8191 segs", mapname)
    check(len(sides) < 0xFFFF and len(nodes) < 0x8000, "%s: counts", mapname)
    check(nsec <= 0x7FFF, "%s: %d sectors, SNDADJ uses bit 15 as a flag", mapname, nsec)
    check(len(ssecs) <= 0x7FFF, "%s: %d subsectors, NODES and GRID use bit 15 as a flag", mapname, len(ssecs))
    for fh, ch, fp, cp, light, special, tag in secs:
        for h in (fh, ch):
            check(-32768 <= h * 8 < 32768, "%s: height %d beyond the i16 1/8 unit range", mapname, h)
        check(special in SECTOR_KNOWN_SPECIALS, "%s: sector special %d", mapname, special)
        check(0 <= light <= 32767, "%s: light %d", mapname, light)
        check(tag >= 0, "%s: negative sector tag %d", mapname, tag)
    for v in MOVER_SPEEDS:
        check(v % (1 << 13) == 0, "mover speed not on the 1/8 unit grid")

    m.bsp_depth = bsp_depth(nodes)
    check(m.bsp_depth <= MAX_BSP_DEPTH, "%s: BSP depth %d", mapname, m.bsp_depth)

    # sides and line sectors (vanilla: line front/back from sidenum, -1 = none)
    def side_sector(sd):
        return sides[sd][5] if sd != -1 else None
    m.line_front = [side_sector(l[5]) for l in lines]
    m.line_back = [side_sector(l[6]) for l in lines]
    for i, l in enumerate(lines):
        check(l[5] != -1, "%s: line %d has no front side", mapname, i)
        check(l[3] == 0 or l[3] in KNOWN_SPECIALS, "%s: line %d special %d is not vanilla", mapname, i, l[3])
        check(l[3] < 256, "special range")
        check(not (l[2] & ML_TWOSIDED) or l[6] != -1, "%s: two-sided line %d without back side", mapname, i)
        check(l[2] < 256, "%s: line %d flags 0x%x use bits above 7", mapname, i, l[2])
        if l[3] and l[3] not in TAGLESS:
            check(l[4] != 0, "%s: line %d special %d with tag 0", mapname, i, l[3])
        if l[3]:
            check(l[4] >= 0, "%s: line %d has the negative tag %d", mapname, i, l[4])
        x1, y1 = verts[l[0]]
        x2, y2 = verts[l[1]]
        check(-32768 <= x2 - x1 < 32768 and -32768 <= y2 - y1 < 32768, "%s: line delta range", mapname)

    # texture numbers per WAD side
    m.side_tex = [(tex_number(tex_first, s[2]), tex_number(tex_first, s[3]), tex_number(tex_first, s[4]))
                  for s in sides]

    # P_GroupLines
    seclines = [[] for _ in range(nsec)]
    for i in range(len(lines)):
        f, b = m.line_front[i], m.line_back[i]
        seclines[f].append(i)
        if b is not None and b != f:
            seclines[b].append(i)
    m.seclines = seclines
    m.blockbox = []
    m.soundorg = []
    shift = FRACBITS + F.MAPBLOCKSHIFT
    oxf, oyf, rad = m.ox << FRACBITS, m.oy << FRACBITS, MAXRADIUS << FRACBITS
    for s in range(nsec):
        check(seclines[s], "%s: sector %d has no lines", mapname, s)
        # M_ClearBox and M_AddToBox in fixed point, with the else-if of the vanilla code
        top = right = -(1 << 31)
        bottom = left = (1 << 31) - 1
        for i in seclines[s]:
            for k in (0, 1):
                x, y = (v << FRACBITS for v in verts[lines[i][k]])
                if x < left:
                    left = x
                elif x > right:
                    right = x
                if y < bottom:
                    bottom = y
                elif y > top:
                    top = y
        # soundorg = (fixed sum) / 2 truncated toward zero, stored >> FRACBITS
        sx = cdiv(s32(right + left), 2)
        sy = cdiv(s32(top + bottom), 2)
        m.soundorg.append((sx >> FRACBITS, sy >> FRACBITS))
        bt = s32(top - oyf + rad) >> shift
        bt = m.rows - 1 if bt >= m.rows else bt
        bb = s32(bottom - oyf - rad) >> shift
        bb = 0 if bb < 0 else bb
        br = s32(right - oxf + rad) >> shift
        br = m.cols - 1 if br >= m.cols else br
        bl = s32(left - oxf - rad) >> shift
        bl = 0 if bl < 0 else bl
        for v in (bt, bb, br, bl):
            check(0 <= v < 256, "%s: blockbox range", mapname)
        m.blockbox.append((bt, bb, bl, br))

    # subsector sectors (P_GroupLines: sector of the first seg's sidedef)
    m.ssec_sector = []
    for numsegs, first in ssecs:
        v1, v2, ang, ld, side, off = segs[first]
        m.ssec_sector.append(sides[lines[ld][5 + side]][5])

    # tags
    tagvals = {s[6] for s in secs if s[6]}
    tagvals |= {l[4] for l in lines if l[3] and l[4]}
    if mapname in BOSS_TAGS:
        tagvals.add(BOSS_TAGS[mapname])
    m.tags = sorted(tagvals)
    check(len(m.tags) <= MAX_TAGS, "%s: %d tags", mapname, len(m.tags))
    m.tag_index = {t: i + 1 for i, t in enumerate(m.tags)}
    m.tag_sectors = {t: [s for s in range(nsec) if secs[s][6] == t] for t in m.tags}

    def tagged(tag):
        return m.tag_sectors.get(tag, [])

    # sectors whose floor flat can change at run time
    floorpic_mut = set()
    for l in lines:
        if l[3] in FLOORPIC_CHANGERS:
            floorpic_mut.update(tagged(l[4]))
        if l[3] in DONUTS:
            floorpic_mut.update(r for r in (donut_ring(m, s) for s in tagged(l[4])) if r is not None)

    # mutable sectors
    mut = set()
    m.stair_fallbacks = 0
    for s in range(nsec):
        if secs[s][5] in SECTOR_MUTABLE_SPECIALS:
            mut.add(s)
    for i, l in enumerate(lines):
        sp, tag = l[3], l[4]
        if not sp:
            continue
        if sp in MANUAL_DOORS:
            if m.line_back[i] is not None:
                mut.add(m.line_back[i])
            continue
        if sp in TAGLESS or sp in TELEPORTS:
            continue
        for s in tagged(tag):
            mut.add(s)
            if sp in STAIRS:
                chain, exact = stair_chain(m, s, floorpic_mut)
                mut.update(chain)
                m.stair_fallbacks += not exact
            if sp in DONUTS:
                s2 = donut_ring(m, s)
                if s2 is not None:
                    mut.add(s2)
    if mapname in BOSS_TAGS:
        mut.update(tagged(BOSS_TAGS[mapname]))
    m.mutable = sorted(mut)
    check(len(m.mutable) <= 254, "%s: %d mutable sectors", mapname, len(m.mutable))
    m.overlay = {s: k for k, s in enumerate(m.mutable)}

    # light thinkers: level start plus line-started strobes
    m.nlights = sum(1 for s in secs if s[5] in SECTOR_LIGHT_SPECIALS)
    m.light_slots = m.nlights + sum(len(tagged(l[4])) for l in lines if l[3] in STROBE_START)

    # special lines and switch sides
    m.special_lines = [i for i, l in enumerate(lines) if l[3]]
    check(len(m.special_lines) <= 254, "%s: %d special lines", mapname, len(m.special_lines))
    m.line_slot = {i: k for k, i in enumerate(m.special_lines)}
    m.scrollers = [i for i, l in enumerate(lines) if l[3] in SCROLLERS]
    check(len(m.scrollers) <= MAXLINEANIMS, "%s: %d scrolling lines, vanilla allows %d", mapname,
          len(m.scrollers), MAXLINEANIMS)

    # WAD sidedefs of special lines must not be shared (vanilla changes them in place)
    use = {}
    for i, l in enumerate(lines):
        for sd in (l[5], l[6]):
            if sd != -1:
                use.setdefault(sd, []).append(i)
    for i in m.special_lines:
        for sd in (lines[i][5], lines[i][6]):
            if sd != -1:
                check(len(use[sd]) == 1, "%s: special line %d shares sidedef %d", mapname, i, sd)

    # things
    things = I.sp_things(wad, inf, mapname)
    m.things = things
    m.thing_type = [dense.type_id[t[6]] for t in things]
    m.thing_sector = [m.ssec_sector[point_in_subsector(nodes, t[1] << FRACBITS, t[2] << FRACBITS)]
                      for t in things]
    m.rngk = []
    k = [0, 0, 0]
    player = inf.midx["MT_PLAYER"]
    for t in things:
        m.rngk.append(tuple(v & 255 for v in k))
        for g in range(3):
            if t[6] == player or t[5] & F.GROUP_BITS[g]:
                k[g] += 1
                if t[6] != player and inf.states[inf.mobjinfo[t[6]]["spawnstate"]][2] > 0:
                    k[g] += 1
    m.rng_total = tuple(k)

    # monsters (spawn state runs A_Look)
    m.monsters = []
    look = "A_Look"
    for i, t in enumerate(things):
        st = inf.mobjinfo[t[6]]["spawnstate"]
        if inf.states[st][3] == look:
            s1 = inf.states[st]
            s2 = inf.states[s1[4]]
            check(0 < s1[2] <= 15 and 0 < s2[2] <= 15 and s2[3] == look and s2[4] == st,
                  "%s: stand states of %s are not a 2-state A_Look cycle", mapname, inf.mnames[t[6]])
            m.monsters.append(i)
    # static things animate as f(type, spawn phase, leveltime): no actions in their spawn cycles
    for t in {th[6] for th in things} - {player}:
        st = inf.mobjinfo[t]["spawnstate"]
        if inf.states[st][3] == look:
            continue
        seen = set()
        while st and st not in seen:
            seen.add(st)
            check(not inf.states[st][3], "%s: %s spawn cycle runs %s", mapname, inf.mnames[t], inf.states[st][3])
            st = inf.states[st][4]
    m.monsters_by_group = [sum(1 for i in m.monsters if things[i][5] & F.GROUP_BITS[g]) for g in range(3)]
    m.drops_by_group = [sum(1 for i in m.monsters if things[i][5] & F.GROUP_BITS[g]
                            and inf.mnames[things[i][6]] in I.KILL_DROPS) for g in range(3)]
    for t in things:
        check(0 <= inf.mobjinfo[t[6]]["mass"] < 65536, "mass range")

    # blockmap and sector thing lists, reverse spawn order
    ncells = m.cols * m.rows
    cells = [[] for _ in range(ncells)]
    secth = [[] for _ in range(nsec)]
    for i in reversed(range(len(things))):
        fl = inf.mobjinfo[things[i][6]]["flags"]
        if not fl & I.MF_NOBLOCKMAP:
            cx = (things[i][1] - m.ox) >> 7
            cy = (things[i][2] - m.oy) >> 7
            if 0 <= cx < m.cols and 0 <= cy < m.rows:
                cells[cy * m.cols + cx].append(i)
        if not fl & I.MF_NOSECTOR:
            secth[m.thing_sector[i]].append(i)
    m.cellthings = cells
    m.secthings = secth

    # teleport destinations
    tele = inf.midx["MT_TELEPORTMAN"]
    m.teledest = sorted((m.thing_sector[i], i) for i, t in enumerate(things) if t[6] == tele)

    m.zclip = zclip_set(m, inf)
    m.grid = start_grid(m)
    m.sndadj = sound_adjacency(m)
    m.reject = reject_bytes(m, L["REJECT"])
    check(all(o < 32768 for o in struct.unpack_from("<%dH" % ncells, m.blockmap, 8)),
          "%s: blockmap offsets beyond 32767", mapname)
    return m


def next_sector(m, line, sec):
    """getNextSector."""
    if not m.lines[line][2] & ML_TWOSIDED:
        return None
    if m.line_front[line] == sec:
        return m.line_back[line]
    return m.line_front[line]


def donut_ring(m, sec):
    """EV_DoDonut s2: the sector across the first line of the tagged sector."""
    return next_sector(m, m.seclines[sec][0], sec) if m.seclines[sec] else None


def stair_closure(m, start, texture):
    """Sectors reachable over two-sided lines from front to back, with floor flat texture (None: any)."""
    seen = {start}
    todo = [start]
    while todo:
        sec = todo.pop()
        for li in m.seclines[sec]:
            if not m.lines[li][2] & ML_TWOSIDED or m.line_front[li] != sec:
                continue
            t = m.line_back[li]
            if (texture is None or m.secs[t][2] == texture) and t not in seen:
                seen.add(t)
                todo.append(t)
    return seen - {start}


def stair_chain(m, start, floorpic_mut):
    """Every sector EV_BuildStairs can reach from a tagged sector: (sorted list, exact).

    Vanilla follows two-sided lines whose front is the current sector to a back
    sector with the start floor flat, skipping sectors that already move. The
    set of all such paths is a superset of any run-time chain when no sector
    the walk can reach changes its floor flat at run time. Otherwise every
    reachable sector is taken, whatever its flat.
    """
    anyflat = stair_closure(m, start, None)
    if floorpic_mut & (anyflat | {start}):
        return sorted(anyflat), False
    return sorted(stair_closure(m, start, m.secs[start][2])), True


def bsp_depth(nodes):
    if not nodes:
        return 0
    depth = 0
    stack = [(len(nodes) - 1, 1)]
    while stack:
        n, d = stack.pop()
        depth = max(depth, d)
        for c in nodes[n][6]:
            if not c & F.NF_SUBSECTOR:
                stack.append((c, d + 1))
    return depth


def zclip_set(m, inf):
    """Things whose z can change through P_ChangeSector's height clipping."""
    mut = set(m.mutable)
    cells = set()
    for s in m.mutable:
        bt, bb, bl, br = m.blockbox[s]
        for cy in range(bb, bt + 1):
            for cx in range(bl, br + 1):
                cells.add((cx, cy))
    player = inf.midx["MT_PLAYER"]
    lines = []
    for i, l in enumerate(m.lines):
        x1, y1 = m.verts[l[0]]
        x2, y2 = m.verts[l[1]]
        lines.append((i, x1, y1, x2 - x1, y2 - y1, min(x1, x2), max(x1, x2), min(y1, y2), max(y1, y2)))
    out = []
    for i, t in enumerate(m.things):
        if t[6] == player:
            continue
        mi = inf.mobjinfo[t[6]]
        if mi["flags"] & I.MF_NOBLOCKMAP:
            continue
        cell = ((t[1] - m.ox) >> 7, (t[2] - m.oy) >> 7)
        if cell not in cells:
            continue
        own = m.thing_sector[i]
        if own in mut:
            out.append(i)
            continue
        r = mi["radius"] >> FRACBITS
        box = (t[2] + r, t[2] - r, t[1] - r, t[1] + r)
        hit = False
        for li, x1, y1, dx, dy, lx0, lx1, ly0, ly1 in lines:
            if box[3] <= lx0 or box[2] >= lx1 or box[0] <= ly0 or box[1] >= ly1:
                continue
            if box_on_line_side(box, x1, y1, dx, dy) != -1:
                continue
            f, b = m.line_front[li], m.line_back[li]
            if f in mut or (b is not None and b in mut):
                hit = True
                break
            if b is None:
                continue
            for s in (f, b):
                if m.secs[s][0] != m.secs[own][0] or m.secs[s][1] != m.secs[own][1]:
                    hit = True
            if hit:
                break
        if hit:
            out.append(i)
    return out


def start_grid(m):
    """Per blockmap cell start node: descend while the cell grown by GRID_MARGIN is strictly on one side."""
    nodes = m.nodes
    root = len(nodes) - 1
    out = []
    g = F.GRID_MARGIN
    for cy in range(m.rows):
        for cx in range(m.cols):
            x0 = m.ox + (cx << 7) - g
            x1 = m.ox + (cx << 7) + 128 + g
            y0 = m.oy + (cy << 7) - g
            y1 = m.oy + (cy << 7) + 128 + g
            n = root
            while not n & F.NF_SUBSECTOR:
                nx, ny, ndx, ndy = nodes[n][:4]
                sides = set()
                for px, py in ((x0, y0), (x1, y0), (x0, y1), (x1, y1)):
                    c = ndx * (py - ny) - ndy * (px - nx)
                    sides.add(0 if c < 0 else 1 if c > 0 else 2)
                if len(sides) != 1 or 2 in sides:
                    break
                n = nodes[n][6][sides.pop()]
            out.append(n)
    return out


def sound_adjacency(m):
    """Per sector: [(other, blocked)] over two-sided lines, blocked if every joining line blocks."""
    out = []
    for s in range(len(m.secs)):
        pairs = {}
        for li in m.seclines[s]:
            l = m.lines[li]
            if not l[2] & ML_TWOSIDED:
                continue
            f = m.sides[l[5]][5]
            b = m.sides[l[6]][5]
            other = b if f == s else f
            if other == s:
                continue
            blk = bool(l[2] & ML_SOUNDBLOCK)
            pairs[other] = pairs.get(other, True) and blk
        out.append(sorted(pairs.items()))
    return out


def reject_bytes(m, lump):
    n = (len(m.secs) * len(m.secs) + 7) // 8
    check(len(lump) >= n, "%s: REJECT shorter than %d bytes", m.name, n)
    return lump[:n]


# ------------------------------------------------------------------ encoding

def side_key(m, sd, tx):
    s = m.sides[sd]
    t = m.side_tex[sd]
    return (s[0], s[1], tx(t[0]), tx(t[1]), tx(t[2]))


def encode(m, texmap, flatmap, switch_tex, plans):
    """Return the map block bytes and a size summary.

    texmap: vanilla texture number -> global id. flatmap: flat name -> global id
    (F_SKY1 -> None). switch_tex: set of vanilla texture numbers in the switch list.
    plans: three dmc_plan_t dicts.
    """
    # per-map texture and flat indices, ordered by global id
    used_t = set()
    for sd in range(len(m.sides)):
        used_t.update(v for v in m.side_tex[sd] if v)
    tex_ids = sorted({texmap[v] for v in used_t})
    check(len(tex_ids) <= 255, "%s: %d textures", m.name, len(tex_ids))
    tex_local = {g: i + 1 for i, g in enumerate(tex_ids)}

    def tx(v):
        return tex_local[texmap[v]] if v else 0

    flat_ids = sorted({flatmap[n] for s in m.secs for n in (s[2], s[3]) if flatmap[n] is not None})
    check(len(flat_ids) <= 255, "%s: %d flats", m.name, len(flat_ids))
    flat_local = {g: i + 1 for i, g in enumerate(flat_ids)}

    def fl(name):
        g = flatmap[name]
        return 0 if g is None else flat_local[g]

    # unique sides
    side_rec = []
    side_index = {}
    dedup = {}
    special_side = set()
    switch_slot = {}
    for i in m.special_lines:
        for sd in (m.lines[i][5], m.lines[i][6]):
            if sd != -1:
                special_side.add(sd)
        sd0 = m.lines[i][5]
        if any(v in switch_tex for v in m.side_tex[sd0]):
            switch_slot[sd0] = len(switch_slot)
    check(len(switch_slot) <= 254, "%s: switch sides", m.name)
    for li, l in enumerate(m.lines):
        for sd in (l[5], l[6]):
            if sd == -1 or sd in side_index:
                continue
            key = side_key(m, sd, tx)
            if sd in special_side:
                side_index[sd] = len(side_rec)
                side_rec.append(key + (switch_slot.get(sd, F.NONE8),))
                continue
            if key not in dedup:
                dedup[key] = len(side_rec)
                side_rec.append(key + (F.NONE8,))
            side_index[sd] = dedup[key]
    m.unique_sides = len(side_rec)
    m.switch_sides = len(switch_slot)

    S = {}
    S["NODES"] = b"".join(F.NODE.pack(x=n[0], y=n[1], dx=n[2], dy=n[3], child=list(n[6])) for n in m.nodes)
    S["NODEBB"] = b"".join(F.NODEBB.pack(bbox=list(n[4]) + list(n[5])) for n in m.nodes)
    S["SSECTORS"] = b"".join(F.SSECTOR.pack(firstseg=first, numsegs=num, sector=m.ssec_sector[i])
                             for i, (num, first) in enumerate(m.ssecs))
    segs = []
    for v1, v2, ang, ld, side, off in m.segs:
        l = m.lines[ld]
        sd = l[5 + side]
        back = F.NONE16
        if l[2] & ML_TWOSIDED:
            back = m.sides[l[5 + (side ^ 1)]][5]
        line = ld | (F.SEG_SIDE if side else 0) | (F.SEG_HASMID if m.side_tex[sd][2] else 0)
        segs.append(F.SEG.pack(v1x=m.verts[v1][0], v1y=m.verts[v1][1], v2x=m.verts[v2][0], v2y=m.verts[v2][1],
                               angle=ang & 0xFFFF, offset=off, line=line, backsector=back))
    S["SEGS"] = b"".join(segs)
    lrec, lsides = [], []
    for i, l in enumerate(m.lines):
        x1, y1 = m.verts[l[0]]
        x2, y2 = m.verts[l[1]]
        back = m.line_back[i]
        lrec.append(F.LINE.pack(x1=x1, y1=y1, dx=x2 - x1, dy=y2 - y1, front=m.line_front[i],
                                back=F.NONE16 if back is None else back, flags=l[2] & 0xFF,
                                special=l[3], tag=m.tag_index.get(l[4], 0) if l[3] else 0,
                                slot=m.line_slot.get(i, F.NONE8)))
        lsides.append(F.LINESIDES.pack(side=[side_index[sd] if sd != -1 else F.NONE16 for sd in (l[5], l[6])]))
    S["LINES"] = b"".join(lrec)
    S["LINESIDES"] = b"".join(lsides)
    S["SIDES"] = b"".join(F.SIDE.pack(textureoffset=r[0], rowoffset=r[1], top=r[2], bottom=r[3], mid=r[4],
                                      switch_slot=r[5]) for r in side_rec)
    secrec, secx, seclines = [], [], []
    for s, (fh, ch, fp, cp, light, special, tag) in enumerate(m.secs):
        secrec.append(F.SECTOR.pack(floor=fh, ceiling=ch, light=light, floorpic=fl(fp), ceilingpic=fl(cp),
                                    special=special, tag=m.tag_index.get(tag, 0),
                                    overlay=m.overlay.get(s, F.NONE8), linecount=len(m.seclines[s]),
                                    firstline=len(seclines)))
        seclines.extend(m.seclines[s])
        bt, bb, bl, br = m.blockbox[s]
        secx.append(F.SECEXTRA.pack(blockbox=[bt, bb, bl, br], soundorg_x=m.soundorg[s][0],
                                    soundorg_y=m.soundorg[s][1]))
    check(len(seclines) < 65536, "seclines")
    S["SECTORS"] = b"".join(secrec)
    S["SECEXTRA"] = b"".join(secx)
    S["SECLINES"] = struct.pack("<%dH" % len(seclines), *seclines)
    ranges, lists = [F.RANGE.pack(first=0, count=0)], []
    for t in m.tags:
        ss = m.tag_sectors[t]
        ranges.append(F.RANGE.pack(first=len(lists), count=len(ss)))
        lists.extend(ss)
    S["TAGS"] = b"".join(ranges) + struct.pack("<%dH" % len(lists), *lists) + \
        struct.pack("<%dH" % (len(m.tags) + 1), 0, *m.tags)
    S["BLOCKMAP"] = m.blockmap[8:]
    S["GRID"] = struct.pack("<%dH" % len(m.grid), *m.grid)
    S["REJECT"] = bytes(m.reject)
    ranges, ents = [], []
    for s in range(len(m.secs)):
        ranges.append(F.RANGE.pack(first=len(ents), count=len(m.sndadj[s])))
        ents.extend(o | (F.SNDADJ_BLOCK if blk else 0) for o, blk in m.sndadj[s])
    check(len(ents) < 65536, "sndadj")
    m.nsndadj = len(ents)
    S["SNDADJ"] = b"".join(ranges) + struct.pack("<%dH" % len(ents), *ents)
    th = []
    for i, t in enumerate(m.things):
        th.append(F.THING.pack(x=t[1], y=t[2], sector=m.thing_sector[i], type=m.thing_type[i],
                               angle45=cdiv(t[3], 45) & 7,
                               options=t[5] & 0x1F, rngk=list(m.rngk[i])))
    S["THINGS"] = b"".join(th)
    S["CELLTHINGS"] = _lists(m.cellthings)
    S["SECTHINGS"] = _lists(m.secthings)
    S["ZCLIP"] = struct.pack("<%dH" % (len(m.zclip) + 1), len(m.zclip), *m.zclip)
    S["TELEDEST"] = b"".join(F.TELEDEST.pack(sector=s, thing=t) for s, t in m.teledest)
    S["SPECIALS"] = struct.pack("<%dH" % len(m.scrollers), *m.scrollers)
    S["MONSTERS"] = struct.pack("<%dH" % len(m.monsters), *m.monsters)
    S["TEXXLAT"] = struct.pack("<%dH" % (len(tex_ids) + 1), F.NONE16, *tex_ids)
    S["FLATXLAT"] = struct.pack("<%dH" % (len(flat_ids) + 1), F.NONE16, *flat_ids)

    body = bytearray()
    offs, sizes = [], []
    pos = F.MAPHDR.size
    for name in F.STREAMS:
        data = S[name]
        offs.append(pos)
        sizes.append(len(data))
        body += data + b"\0" * (align(len(data)) - len(data))
        pos += align(len(data))
    hdr = F.MAPHDR.pack(
        magic=F.MAGIC_MAP, nlines=len(m.lines), nsides=len(side_rec), nsegs=len(m.segs),
        nssectors=len(m.ssecs), nnodes=len(m.nodes), nsectors=len(m.secs), nthings=len(m.things),
        nmonsters=m.monsters_by_group, nspeclines=len(m.special_lines), nswitches=len(switch_slot),
        nmutable=len(m.mutable), ntags=len(m.tags), nzclip=len(m.zclip), nlights=m.nlights,
        nseclines=len(seclines), nsndadj=len(ents), nteledest=len(m.teledest), nscrollers=len(m.scrollers),
        ncellthings=sum(len(c) for c in m.cellthings), nsecthings=sum(len(c) for c in m.secthings),
        bmap_ox=m.ox, bmap_oy=m.oy, bmap_cols=m.cols, bmap_rows=m.rows, root=len(m.nodes) - 1,
        ntex=len(tex_ids), nflat=len(flat_ids), stream_off=offs, stream_size=sizes,
        plans=[F.PLAN.pack(**p) for p in plans])
    block = hdr + bytes(body)
    summary = {n: len(S[n]) for n in F.STREAMS}
    return block, summary


def _lists(lists):
    first = []
    flat = []
    for L in lists:
        first.append(len(flat))
        flat.extend(L)
    first.append(len(flat))
    check(len(flat) < 65536, "thing list too long")
    return struct.pack("<%dH" % len(first), *first) + struct.pack("<%dH" % len(flat), *flat)


def texture_needs(m):
    """(vanilla texture numbers drawn opaque, numbers drawn masked) by this map."""
    opaque, masked = set(), set()
    for li, l in enumerate(m.lines):
        two = bool(l[2] & ML_TWOSIDED)
        for sd in (l[5], l[6]):
            if sd == -1:
                continue
            top, bot, mid = m.side_tex[sd]
            for v in (top, bot):
                if v:
                    opaque.add(v)
            if mid:
                (masked if two else opaque).add(mid)
    return opaque, masked


def flat_names(m):
    return {n for s in m.secs for n in (s[2], s[3])}
