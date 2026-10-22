# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Patch blobs (with the half-width option) and the SPRDEF section."""

import struct

from . import format as F
from . import info as I
from . import textures
from .util import align, check


def column_pixels(posts, height):
    """Paint posts in order into (cover, pixels) arrays of the given height."""
    cover = bytearray(height)
    pix = bytearray(height)
    for td, data in posts:
        for k, c in enumerate(data):
            y = td + k
            if 0 <= y < height:
                cover[y] = 1
                pix[y] = c
    return cover, pix


def encode_patch(pic, xshift=0, half_rows=False):
    ncols = (pic.width + (1 << xshift) - 1) >> xshift
    blobs = []
    opaque = True
    for k in range(ncols):
        posts = pic.columns[k << xshift]
        if half_rows:
            cover, pix = column_pixels(posts, pic.height)
            posts = textures.runs_to_posts(cover[::2], pix[::2])
            full = (pic.height + 1) // 2
        else:
            full = pic.height
        if not (len(posts) == 1 and posts[0][0] == 0 and len(posts[0][1]) == full):
            opaque = False
        blobs.append(textures.encode_posts(posts))
    flags = (F.PF_OPAQUE if opaque else 0) | (F.PF_HALF_ROWS if half_rows else 0)
    ci_off = F.PATCH_HDR.size
    pos = ci_off + 4 * ncols
    infos = []
    for b in blobs:
        infos.append(F.colinfo(pos, len(b)))
        pos += len(b)
    hdr = F.PATCH_HDR.pack(width=pic.width, height=pic.height, leftoffset=pic.left, topoffset=pic.top,
                           xshift=xshift, flags=flags, ncols=ncols, colinfo=ci_off)
    out = hdr + struct.pack("<%dI" % ncols, *infos) + b"".join(blobs)
    return out + b"\0" * (align(len(out)) - len(out))


def sprite_frames(wad, info, dense):
    """Vanilla R_InitSpriteDefs for the firmware's sprites.

    Returns per dense sprite: list of frames, each (rotate, [(lump, flip)] * 8) or None.
    """
    first = wad.num("S_START") + 1
    last = wad.num("S_END") - 1
    names = [(wad.name(l), l) for l in range(first, last + 1)]
    out = []
    for spr in dense.sprite_list:
        sname = info.sprnames[spr]
        tmp = {}
        maxframe = -1
        for nm, l in names:
            if nm[:4] != sname:
                continue
            for k in (4, 6):
                if len(nm) <= k + 1:
                    continue
                frame = ord(nm[k]) - ord("A")
                rot = ord(nm[k + 1]) - ord("0")
                check(0 <= frame < 29 and 0 <= rot <= 8, "sprite lump %s", nm)
                maxframe = max(maxframe, frame)
                fr = tmp.setdefault(frame, {"rotate": None, "lumps": [None] * 8})
                flip = k == 6
                if rot == 0:
                    check(fr["rotate"] is None, "sprite %s frame %c rotation conflict", sname, 65 + frame)
                    fr["rotate"] = False
                    fr["lumps"] = [(l, flip)] * 8
                else:
                    check(fr["rotate"] is not False, "sprite %s frame %c rotation conflict", sname, 65 + frame)
                    fr["rotate"] = True
                    check(fr["lumps"][rot - 1] is None, "sprite %s frame %c rot %d twice", sname, 65 + frame, rot)
                    fr["lumps"][rot - 1] = (l, flip)
        frames = []
        for f in range(maxframe + 1):
            fr = tmp.get(f)
            check(fr is not None, "sprite %s frame %c missing", sname, 65 + f)
            if fr["rotate"]:
                check(all(fr["lumps"]), "sprite %s frame %c missing rotations", sname, 65 + f)
            frames.append(fr)
        out.append(frames)
    return out


def needed_frames(info, dense, pack_types, pack_weapons):
    """{dense sprite: set(frame)} drawn in a pack: every type but the player, plus psprites."""
    types = set(pack_types) - {info.midx["MT_PLAYER"]}
    _, states, _ = I.closure_from(info, types, pack_weapons)
    spawn = {info.mobjinfo[t]["spawnstate"] for t in types}
    need = {}
    for s in states:
        if info.states[s][2] == 0 and s not in spawn:
            continue  # zero-tic states are passed through, never drawn
        spr, frame = info.states[s][0], info.states[s][1] & 0x7FFF
        need.setdefault(dense.sprite_id[spr], set()).add(frame)
    return need


def build_sprdef(frames, need, patch_id):
    """SPRDEF bytes. patch_id: lump -> patch id for every lump of a needed frame."""
    ents, recs = [], []
    for sid, fr in enumerate(frames):
        ents.append(F.SPRITE.pack(first=len(recs), numframes=len(fr)))
        for f, d in enumerate(fr):
            vals = []
            for r, (lump, flip) in enumerate(d["lumps"]):
                v = patch_id[lump] if f in need.get(sid, ()) else F.SPR_NO_PATCH
                if flip:
                    v |= F.SPR_FLIP
                if r == 0 and d["rotate"]:
                    v |= F.SPR_ROTATE
                vals.append(v)
            recs.append(F.SPRFRAME.pack(lump=vals))
    head = F.SPRDEF_HDR.size + 4 * len(ents)
    foff = align(head)
    out = F.SPRDEF_HDR.pack(nsprites=len(ents), nframes=len(recs), frames_off=foff) + b"".join(ents)
    out += b"\0" * (foff - len(out))
    return out + b"".join(recs)


def needed_lumps(frames, need):
    lumps = set()
    for sid, fr in enumerate(frames):
        for f in need.get(sid, ()):
            check(f < len(fr), "frame %d of sprite %d not in the WAD", f, sid)
            lumps.update(l for l, _ in fr[f]["lumps"])
    return lumps
