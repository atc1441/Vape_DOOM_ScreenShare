# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Pack assembly: logical image, DOOM.DAT / DOOMX.DAT split, report.json, halfui.wad."""

import hashlib
import json
import os
import struct

from . import VERSION
from . import flats as FL
from . import format as F
from . import info as I
from . import maps as M
from . import plan as P
from . import sounds as SND
from . import sprites as SP
from . import textures as TX
from . import ui as UI
from . import wadread
from .util import BUILD, ConvError, align, check, crc32, mkdirs, pad, write_if_changed

E1 = I.E1_MAPS
# ui_half: UI pictures stored at half width (xshift 1)
PACKS = {
    "E1M1": {"maps": ["E1M1"], "base": 0, "mips": 3, "greedy": False, "xshift": 0,
             "ui_exclude": (), "ui_half": (), "demo": False, "doomx": False},
    "E1": {"maps": E1, "base": 1, "mips": 1, "greedy": True, "xshift": 1,
           "ui_exclude": ("HELP1", "CREDIT"), "ui_half": ("TITLEPIC", "WIMAP0"), "demo": True,
           "doomx": True},
    "E1A": {"maps": E1[:5], "base": 1, "mips": 1, "greedy": True, "xshift": 1,
            "ui_exclude": ("HELP1", "CREDIT"), "ui_half": ("TITLEPIC", "WIMAP0"), "demo": False,
            "doomx": False},
    "E1B": {"maps": E1[5:], "base": 1, "mips": 1, "greedy": True, "xshift": 1,
            "ui_exclude": ("HELP1", "CREDIT"), "ui_half": ("TITLEPIC", "WIMAP0"), "demo": True,
            "doomx": False},
}
# Steps that make a pack smaller, tried in order while it does not fit
LADDER = [
    (1, "drop DEMO1"),
    (2, "half-res monster sprites"),
    (3, "two half-episode packs E1A (E1M1-E1M5) and E1B (E1M6-E1M9)"),
]
GREEDY_MARGIN = 1024
SKY_NAME = "SKY1"
FINALE_DEFAULT = "FLOOR4_8"


class Ctx:
    """Data shared by every pack of one converter run."""

    def __init__(self, wad, wad_bytes, notices, up, deh, inf, dense, layout):
        self.wad, self.wad_bytes, self.notices = wad, wad_bytes, notices
        self.up, self.deh, self.inf, self.dense, self.layout = up, deh, inf, dense, layout
        self.ts = TX.TextureSet(wad)
        self.maps = {}
        self.frames = SP.sprite_frames(wad, inf, dense)
        last_ep = wadread.switch_episode(wad)
        self.switches = [(a.upper(), b.upper()) for a, b, ep in up["switches"] if ep <= last_ep]
        self.flat_first, self.flat_last = wad.num("F_START") + 1, wad.num("F_END") - 1

    def map(self, name):
        if name not in self.maps:
            self.maps[name] = M.analyze(self.wad, name, self.inf, self.dense, self.ts.first)
        return self.maps[name]

    def flatnum(self, name):
        n = self.wad.num(name)
        check(n >= 0, "flat %s missing", name)
        return n - self.flat_first


# ------------------------------------------------------------------ textures and flats

def texture_plan(ctx, cfg, maps):
    """Global texture order, ANIMS entries and usage flags for a pack."""
    ts = ctx.ts
    opaque, masked, uses = set(), set(), {}
    for m in maps:
        o, k = M.texture_needs(m)
        opaque |= o
        masked |= k
        for sd in range(len(m.sides)):
            for v in m.side_tex[sd]:
                if v:
                    uses[v] = uses.get(v, 0) + 1
    sky = ts.first[SKY_NAME]
    opaque.add(sky)
    used = opaque | masked
    # switch pairs in alphSwitchList order
    pairs = []
    for a, b in ctx.switches:
        na, nb = ts.first.get(a), ts.first.get(b)
        check(na is not None and nb is not None, "switch texture %s/%s missing", a, b)
        if na in used or nb in used:
            pairs.append((na, nb))
    # animation cycles by vanilla texture number
    cycles = []
    for istex, start, end, speed in ctx.up["animdefs"]:
        if not istex:
            continue
        s, e = ts.first.get(start.upper()), ts.first.get(end.upper())
        if s is None:
            continue
        check(e is not None and e - s + 1 >= 2, "bad texture cycle %s", start)
        if any(v in used for v in range(s, e + 1)):
            cycles.append((s, e, speed))
    order = []
    anims = []
    for a, b in pairs:
        anims.append(F.ANIM.pack(kind=F.ANIM_SWITCH, speed=0, first=len(order), last=len(order) + 1, phase=0))
        order += [a, b]
    for s, e, speed in cycles:
        check(not set(range(s, e + 1)) & set(order), "texture cycle overlaps a switch")
        anims.append(F.ANIM.pack(kind=F.ANIM_TEXTURE, speed=speed, first=len(order),
                                 last=len(order) + e - s, phase=s % (e - s + 1)))
        order += list(range(s, e + 1))
    inset = set(order)
    order += sorted(v for v in used if v not in inset)
    # propagate draw modes across switch pairs and cycles
    for a, b in pairs:
        for x, y in ((a, b), (b, a)):
            if x in opaque:
                opaque.add(y)
            if x in masked:
                masked.add(y)
    for s, e, _ in cycles:
        grp = set(range(s, e + 1))
        if grp & opaque:
            opaque |= grp
        if grp & masked:
            masked |= grp
    return order, anims, opaque, masked, uses, sky


def flat_plan(ctx, maps, finale):
    names = set()
    for m in maps:
        names |= M.flat_names(m)
    nums = {ctx.flatnum(n) for n in names if n != "F_SKY1"}
    cycles = []
    for istex, start, end, speed in ctx.up["animdefs"]:
        if istex or ctx.wad.num(start) < 0:
            continue
        s, e = ctx.flatnum(start), ctx.flatnum(end)
        check(e - s + 1 >= 2, "bad flat cycle %s", start)
        if any(v in nums for v in range(s, e + 1)):
            cycles.append((s, e, speed))
    order, anims = [], []
    for s, e, speed in cycles:
        anims.append(F.ANIM.pack(kind=F.ANIM_FLAT, speed=speed, first=len(order), last=len(order) + e - s,
                                 phase=s % (e - s + 1)))
        order += list(range(s, e + 1))
    inset = set(order)
    order += sorted(v for v in nums if v not in inset)
    names_by_num = [ctx.wad.name(ctx.flat_first + v) for v in order]
    flatmap = {"F_SKY1": None}
    for n in names:
        if n != "F_SKY1":
            flatmap[n] = order.index(ctx.flatnum(n))
    return names_by_num, anims, flatmap


# ------------------------------------------------------------------ other sections

def palette_section(wad):
    pp = wad.lump("PLAYPAL")
    check(len(pp) >= F.PALETTE_COUNT * 768, "PLAYPAL short")
    out = []
    for i in range(F.PALETTE_COUNT * 256):
        r, g, b = pp[3 * i], pp[3 * i + 1], pp[3 * i + 2]
        out.append(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
    return struct.pack("<%dH" % len(out), *out)


def string_values(up, deh):
    vals = []
    for n in F.STRING_IDS:
        if n in deh["strings"]:
            v = deh["strings"][n]
        elif n in F.QUITMSG_IDS:
            v = up["endmsg"][F.QUITMSG_IDS.index(n)]
        else:
            v = up["strings"][n]
        vals.append(v)
    return vals


def list_section(blobs):
    head = F.LIST_HDR.size + 4 * len(blobs)
    offs, data = [], bytearray()
    for b in blobs:
        offs.append(head + len(data))
        data += b
    return F.LIST_HDR.pack(count=len(blobs)) + struct.pack("<%dI" % len(offs), *offs) + bytes(data)


def strings_section(up, deh):
    return list_section([v.encode("latin-1", "replace") + b"\0" for v in string_values(up, deh)])


def par_seconds(up, deh, e, m):
    if (e, m) in deh["pars"]:
        return deh["pars"][(e, m)]
    return up["pars"][e * 10 + m]


def monster_sprites(ctx):
    """Dense sprite ids used by monster states (spawn state runs A_Look)."""
    inf, d = ctx.inf, ctx.dense
    out = set()
    for t in d.type_list:
        mi = inf.mobjinfo[t]
        if inf.states[mi["spawnstate"]][3] != "A_Look":
            continue
        _, st, _ = I.closure_from(inf, {t}, set(), code_roots=False)
        out.update(d.sprite_id[inf.states[s][0]] for s in st)
    return out


# ------------------------------------------------------------------ image

class Built:
    pass


def build_image(ctx, pack_id, cfg, opts):
    """Build one logical image. opts: drop_demo, half_monsters, upgrades (set of vanilla numbers)."""
    wad, inf, dense = ctx.wad, ctx.inf, ctx.dense
    maps = [ctx.map(n) for n in cfg["maps"]]
    order, tex_anims, opaque, masked, uses, sky = texture_plan(ctx, cfg, maps)
    texmap = {v: i for i, v in enumerate(order)}
    tplan = []
    for v in order:
        t = ctx.ts.list[v]
        if v not in opaque:
            base, nlev = min(cfg["base"], t.log2_w), 0
        elif v == sky:
            base, nlev = 0, 1
        elif v in opts.get("upgrades", ()):
            base, nlev = 0, min(2, TX.max_levels(t, 0))
        else:
            base = min(cfg["base"], t.log2_w, t.log2_sh)
            nlev = max(1, min(cfg["mips"], TX.max_levels(t, base)))
        tplan.append((t, base, nlev, v in masked))
    texdir, texdata, tmeta = TX.build(ctx.ts, tplan)
    medusa = sum(ctx.ts.multi_patch_masked_columns(ctx.ts.list[v]) for v in order if v in masked)

    finale = ctx.deh["strings"].get("BGFLATE1", FINALE_DEFAULT)
    flat_names, flat_anims, flatmap = flat_plan(ctx, maps, finale)
    flats = FL.build(wad, flat_names, finale)

    # sprites and UI pictures
    ptypes = I.roster_types(wad, inf, cfg["maps"])
    types, _, weapons = I.closure(inf, ptypes)
    need = SP.needed_frames(inf, dense, types, weapons)
    lumps = sorted(SP.needed_lumps(ctx.frames, need))
    half_spr = monster_sprites(ctx) if opts.get("half_monsters") else set()
    half_lumps = set()
    for sid in half_spr:
        for fr in ctx.frames[sid]:
            half_lumps.update(l for l, _ in fr["lumps"])
    uisel = UI.select(wad, cfg["ui_exclude"])
    patch_id, blobs = {}, []
    for l in lumps:
        patch_id[l] = len(blobs)
        blobs.append(SP.encode_patch(ctx.ts.pic(l), cfg["xshift"], l in half_lumps))
    nspr_patches = len(blobs)
    half_ui = [n for _, n, _ in uisel if n in cfg["ui_half"]]
    for _, n, l in uisel:
        if l not in patch_id:
            patch_id[l] = len(blobs)
            blobs.append(SP.encode_patch(ctx.ts.pic(l), 1 if n in half_ui else 0, False))
    check(len(blobs) < F.SPR_NO_PATCH, "too many patches")
    pdata = bytearray()
    poffs = []
    for b in blobs:
        poffs.append(len(pdata))
        pdata += b
    patchdir = struct.pack("<I%dI" % len(poffs), len(poffs), *poffs)
    sprdef = SP.build_sprdef(ctx.frames, need, patch_id)
    uidir = UI.uidir(uisel, patch_id)

    sounds, snd_stats = SND.build(wad, ctx.up["sfx"])
    anims = tex_anims + flat_anims
    demos = []
    if cfg["demo"] and not opts.get("drop_demo") and wad.num("DEMO1") >= 0:
        demos.append(wad.lump("DEMO1"))

    sections = [
        ("PALETTE", palette_section(wad), 0),
        ("TEXDIR", texdir, 0),
        ("SPRDEF", sprdef, 0),
        ("SOUNDS", sounds, 0),
        ("STRINGS", strings_section(ctx.up, ctx.deh), 0),
        ("UIDIR", uidir, len(F.UI_NAMES)),
        ("ANIMS", b"".join(anims), len(anims)),
        ("PATCHDIR", patchdir, 0),
        ("FLATS", flats, len(flat_names)),
        ("TEXDATA", texdata, 0),
        ("PATCHDATA", bytes(pdata), 0),
        ("DEMOS", list_section(demos), 0),
    ]

    # map blocks
    switch_tex = set()
    for a, b in ctx.switches:
        switch_tex.update((ctx.ts.first[a], ctx.ts.first[b]))
    blocks = []
    plan_rep = {}
    for m in maps:
        plans, rep = P.plan_map(ctx.layout, m)
        block, streams = M.encode(m, texmap, flatmap, switch_tex, plans)
        blocks.append((m, block, streams))
        plan_rep[m.name] = rep

    # layout
    img = bytearray(F.HEADER_SIZE)
    sect_ents = []
    sect_rep = {}
    for name, data, flags in sections:
        off = len(img)
        img += pad(data)
        sect_ents.append(F.SECTENT.pack(id=F.SECT[name], flags=flags, off=off, size=len(data), crc=crc32(data)))
        sect_rep[name] = len(data)
    map_ents = []
    map_rep = {}
    sky_id = texmap[sky]
    in_pack = {int(m.name[3]) for m in maps}

    def next_value(n):
        return n if n == F.NEXT_END_EPISODE or n in in_pack else F.NEXT_NOT_IN_PACK
    for m, block, streams in blocks:
        off = len(img)
        img += pad(block)
        n = int(m.name[3])
        # G_DoCompleted for episode 1: E1M8 ends the episode, E1M9 returns to E1M4,
        # a secret exit always leads to E1M9
        nxt = F.NEXT_END_EPISODE if n == 8 else 4 if n == 9 else n + 1
        map_ents.append(F.MAPENT.pack(
            name=m.name.encode(), off=off, size=len(block), crc=crc32(block), episode=1, map=n,
            par_seconds=par_seconds(ctx.up, ctx.deh, 1, n), name_string=F.STRING["HUSTR_E1M%d" % n],
            sky_texture=sky_id, next=next_value(nxt), secret_next=next_value(9)))
        map_rep[m.name] = {"bytes": len(block), "streams": streams, "unique_sides": m.unique_sides,
                           "mutable": len(m.mutable), "zclip": len(m.zclip), "lights": m.light_slots,
                           "monsters": m.monsters_by_group, "drops": m.drops_by_group,
                           "switch_sides": m.switch_sides, "tags": len(m.tags),
                           "special_lines": len(m.special_lines), "things": len(m.things),
                           "bsp_depth": m.bsp_depth, "stair_fallbacks": m.stair_fallbacks}
    check(len(sect_ents) <= F.MAX_SECTS and len(map_ents) <= F.MAX_MAPS, "header tables full")

    b = Built()
    b.img = img
    b.sect_ents, b.map_ents = sect_ents, map_ents
    b.flags = (F.HF_WALL_MIPS if any(t[2] > 1 for t in tplan) else 0) | \
        (F.HF_HALF_SPRITES if cfg["xshift"] else 0) | \
        (F.HF_HALF_WALLS if any(t[1] > 0 for t in tplan) else 0) | \
        (F.HF_DEMO1 if demos else 0) | (F.HF_HALF_MONSTERS if half_lumps else 0) | \
        (F.HF_HALF_UI if half_ui else 0)
    b.report = {
        "sections": sect_rep, "maps": map_rep, "plans": plan_rep,
        "textures": {"count": len(order), "full_res": sum(1 for t in tplan if t[1] == 0 and t[2]),
                     "half_res": sum(1 for t in tplan if t[1] == 1 and t[2]),
                     "masked": sum(1 for t in tplan if t[3]), "with_mips": sum(1 for t in tplan if t[2] > 1),
                     "masked_multi_patch_columns": medusa, "bytes": len(texdata)},
        "flats": {"count": len(flat_names), "finale": finale},
        "patches": {"sprite": nspr_patches, "ui": len(blobs) - nspr_patches, "bytes": len(pdata),
                    "half_rows": len(half_lumps & set(lumps))},
        "sounds": snd_stats, "demos": len(demos), "anims": len(anims),
    }
    b.names = {"textures": [ctx.ts.list[v].name for v in order], "flats": flat_names,
               "patches": [None] * len(blobs), "finale_flat": finale,
               "sprites": [inf.sprnames[s] for s in dense.sprite_list]}
    for l, k in patch_id.items():
        b.names["patches"][k] = wad.name(l)
    b.tex_candidates = [(v, ctx.ts.list[v]) for v, t in zip(order, tplan)
                        if t[2] and t[1] > 0 and not t[3] and v != sky]
    b.uses = uses
    b.half_ui = half_ui
    return b


def finish(ctx, pack_id, b):
    """Header, uuid, CRCs. Returns the logical image bytes."""
    img = b.img
    hdr_fields = dict(magic=F.MAGIC, fmt_major=F.FMT_MAJOR, fmt_minor=F.FMT_MINOR, image_size=len(img),
                      split=len(img), image_uuid=[0] * 16,
                      wad_sha256=list(hashlib.sha256(ctx.wad_bytes).digest()),
                      conv_version=VERSION.encode(), pack_id=pack_id.encode(),
                      nsect=len(b.sect_ents), nmaps=len(b.map_ents), flags=b.flags, header_crc=0,
                      skill_group=list(F.SKILL_GROUP))
    cfg = PACKS[pack_id]
    if len(img) > F.DOOMDAT_CAP:
        check(cfg["doomx"], "%s: image %d B exceeds DOOM.DAT capacity", pack_id, len(img))
        hdr_fields["split"] = F.DOOMDAT_CAP // F.SPLIT_ALIGN * F.SPLIT_ALIGN

    def put_header(uuid):
        hdr_fields["image_uuid"] = list(uuid)
        hdr_fields["header_crc"] = 0
        h = bytearray(F.HEADER_SIZE)
        h[0:F.HEADER.size] = F.HEADER.pack(**hdr_fields)
        for k, e in enumerate(b.sect_ents):
            h[F.SECT_TABLE_OFF + 16 * k:F.SECT_TABLE_OFF + 16 * (k + 1)] = e
        for k, e in enumerate(b.map_ents):
            h[F.MAP_TABLE_OFF + 32 * k:F.MAP_TABLE_OFF + 32 * (k + 1)] = e
        return h
    h = put_header(b"\0" * 16)
    uuid = hashlib.sha256(bytes(h) + bytes(img[F.HEADER_SIZE:])).digest()[:16]
    h = put_header(uuid)
    hdr_fields["header_crc"] = crc32(bytes(h))
    h[0:F.HEADER.size] = F.HEADER.pack(**hdr_fields)
    img[0:F.HEADER_SIZE] = h
    return bytes(img), hdr_fields["split"], uuid


def half_ui_wad(wad, names):
    """Test PWAD for the oracle, never shipped: the half-width UI pictures at full width with
    every odd column a copy of the even column before it, the picture the engine draws."""
    lumps = []
    for n in names:
        pic = wadread.Picture(wad.lump(n))
        cols = []
        for x in range(pic.width):
            o = p = pic.colofs[x & ~1]
            while pic.raw[p] != 0xFF:
                p += pic.raw[p + 1] + 4
            cols.append(pic.raw[o:p + 1])
        offs, pos = [], 8 + 4 * pic.width
        for c in cols:
            offs.append(pos)
            pos += len(c)
        lumps.append((n, struct.pack("<hhhh%dI" % pic.width, pic.width, pic.height, pic.left, pic.top,
                                     *offs) + b"".join(cols)))
    data = b"".join(d for _, d in lumps)
    out = struct.pack("<4sii", b"PWAD", len(lumps), 12 + len(data)) + data
    pos = 12
    for n, d in lumps:
        out += struct.pack("<ii8s", pos, len(d), n.encode())
        pos += len(d)
    return out


def files_for(img, split, uuid):
    files = {"DOOM.DAT": img[:split]}
    if split < len(img):
        payload = img[split:]
        check(len(payload) + F.DOOMX_HDR <= F.DOOMX_CAP, "DOOMX.DAT exceeds its capacity")
        x = F.DOOMX_HEADER.pack(magic=F.MAGIC_X, fmt_major=F.FMT_MAJOR, fmt_minor=F.FMT_MINOR,
                                image_uuid=list(uuid), logical_start=split, length=len(payload),
                                payload_crc=crc32(payload))
        files["DOOMX.DAT"] = x + payload
    return files


def capacity(cfg):
    return F.DOOMDAT_CAP + (F.DOOMX_CAP - F.DOOMX_HDR if cfg["doomx"] else 0)


def build_pack(ctx, pack_id, out_dir):
    cfg = PACKS[pack_id]
    cap = capacity(cfg)
    opts = {}
    steps = []
    b = build_image(ctx, pack_id, cfg, opts)
    for step, what in LADDER:
        if len(b.img) <= cap:
            break
        if step == 1 and cfg["demo"]:
            opts["drop_demo"] = True
        elif step == 2:
            opts["half_monsters"] = True
        elif step == 3:
            raise ConvError("%s: %d B does not fit %d B after the ladder: build packs E1A and E1B instead"
                            % (pack_id, len(b.img), cap))
        else:
            steps.append({"step": step, "what": what, "applied": False})
            continue
        steps.append({"step": step, "what": what, "applied": True})
        b = build_image(ctx, pack_id, cfg, opts)
    check(len(b.img) <= cap, "%s does not fit", pack_id)
    greedy_slack = cap - len(b.img) - GREEDY_MARGIN
    upgrades = []
    if cfg["greedy"]:
        slack = greedy_slack
        cand = sorted(b.tex_candidates, key=lambda vt: (-b.uses.get(vt[0], 0), vt[0]))
        for v, t in cand:
            extra = align(TX.opaque_size(t, 0, 2)) - align(TX.opaque_size(t, 1, 1))
            if extra <= slack:
                upgrades.append(v)
                slack -= extra
        if upgrades:
            opts["upgrades"] = set(upgrades)
            b = build_image(ctx, pack_id, cfg, opts)
            while len(b.img) > cap - GREEDY_MARGIN // 2 and upgrades:
                upgrades.pop()
                opts["upgrades"] = set(upgrades)
                b = build_image(ctx, pack_id, cfg, opts)
    img, split, uuid = finish(ctx, pack_id, b)
    files = files_for(img, split, uuid)
    aux = {"halfui.wad": half_ui_wad(ctx.wad, b.half_ui)} if b.half_ui else {}
    d = os.path.join(out_dir, pack_id)
    mkdirs(d)
    # A file this run does not make is removed (SAVE.DAT comes from the firmware)
    for name in ("DOOM.DAT", "DOOMX.DAT", "SAVE.DAT", "halfui.wad"):
        p = os.path.join(d, name)
        data = files.get(name, aux.get(name))
        if data is not None:
            write_if_changed(p, data)
        elif os.path.exists(p):
            os.remove(p)
    rep = {"pack": pack_id, "converter": VERSION, "format": [F.FMT_MAJOR, F.FMT_MINOR],
           "image_size": len(img), "split": split, "capacity": cap,
           "doomdat_capacity": F.DOOMDAT_CAP, "doomx_capacity": F.DOOMX_CAP if cfg["doomx"] else 0,
           "free": cap - len(img), "greedy_slack": greedy_slack, "uuid": uuid.hex(), "flags": b.flags,
           "ram_layout": ctx.layout["_path"], "ladder": steps,
           "greedy_upgrades": [ctx.ts.list[v].name for v in upgrades],
           "files": {n: {"size": len(v), "sha256": hashlib.sha256(v).hexdigest()} for n, v in files.items()}}
    rep.update(b.report)
    write_if_changed(os.path.join(d, "report.json"), json.dumps(rep, indent=1, sort_keys=True) + "\n")
    write_if_changed(os.path.join(d, "names.json"), json.dumps(b.names, indent=0) + "\n")
    return rep


def build_all(args, wad, wad_bytes, notices, up, deh, inf, dense):
    out = args.out or os.path.join(BUILD, "packs")
    layout = P.load_layout(args.ram_layout)
    ctx = Ctx(wad, wad_bytes, notices, up, deh, inf, dense, layout)
    for pid in [p for p in args.packs.split(",") if p]:
        check(pid in PACKS, "unknown pack %s", pid)
        rep = build_pack(ctx, pid, out)
        print("pack %s: %d B of %d B (%.1f KB free), split %d, maps %s" % (
            pid, rep["image_size"], rep["capacity"], rep["free"] / 1024.0, rep["split"],
            " ".join("%s=%d" % (k, v["bytes"]) for k, v in sorted(rep["maps"].items()))))
    for name, data in notices.items():
        write_if_changed(os.path.join(out, "notices", name), data)
