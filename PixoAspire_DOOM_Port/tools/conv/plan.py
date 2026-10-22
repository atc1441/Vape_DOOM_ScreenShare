# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""RAM plans per map and skill group.

A plan (A to D) sets the strip width of the renderer, the number of flat
cache slots and textured or solid floors, and with them the size of the frame
region. What the stack, the fixed core, the frame region and the caches leave
of the 32 KB of SRAM is the level arena: the level-fixed state of the map and
the object pages.

Reads build/ram_layout.json when there is one, otherwise
tools/conv/ram_layout_default.json. Plans are tried in order A, B, C, D and
the first with pages >= need_play + reserve is taken. No fit is an error.
"""

import json
import os

from . import format as F
from .util import BUILD, ConvError

DEFAULT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ram_layout_default.json")


def load_layout(path=None):
    if path is None:
        cand = os.path.join(BUILD, "ram_layout.json")
        path = cand if os.path.exists(cand) else DEFAULT
    with open(path) as f:
        lay = json.load(f)
    with open(DEFAULT) as f:
        want = json.load(f)
    missing = [k for k in want if k != "comment" and k not in lay]
    missing += ["level_fixed." + k for k in want["level_fixed"] if k not in lay.get("level_fixed", {})]
    missing += ["plans." + p for p in F.PLANS if p not in lay.get("plans", {})]
    if missing:
        raise ConvError("%s: missing keys %s" % (path, ", ".join(missing)))
    lay["_path"] = os.path.relpath(path, os.path.dirname(BUILD))
    return lay


def ceil_div(a, b):
    return -(-a // b)


def level_fixed(lay, m, g):
    """Level-fixed arena bytes for map info m in skill group g, itemised."""
    c = lay["level_fixed"]
    nmut = len(m.mutable)
    items = [
        ("sectors", c["sector_bytes"] * len(m.secs)),
        ("mutable overlay", c["mutable_bytes"] * nmut),
        ("line bits", ceil_div(c["line_bits"] * len(m.lines), 8)),
        ("special bits", ceil_div(c["special_bits"] * len(m.special_lines), 8)),
        ("switch bits", c["switch_bytes"]),
        ("buttons", c["button_bytes"]),
        ("tier bits", ceil_div(c["tier_bits"] * len(m.things), 8)),
        ("sleeping monster bytes", c["sleep_bytes"] * m.monsters_by_group[g]),
        ("monster bitmap", ceil_div(c["monster_bits"] * len(m.things), 8)),
        ("z-clip slots", c["zclip_bytes"] * len(m.zclip)),
        ("blockmap hash", c["blockmap_hash_bytes"]),
        ("sector hash", c["sector_hash_bytes"]),
        ("lights", c["light_bytes"] * m.light_slots),
        ("movers", c["mover_bytes"] * min(nmut, c["mover_max"])),
        ("plat/ceiling lists", c["platlist_bytes"]),
        ("drops", c["drop_bytes"] * m.drops_by_group[g]),
    ]
    return items


def need_play(lay, mon):
    awake = min(mon, max(lay["awake_min"], ceil_div(lay["awake_percent"] * mon, 100)))
    return 1 + lay["transients"] + awake


def arena_bytes(lay, p):
    pl = lay["plans"][p]
    caches = lay["map_cache_bytes"] + pl["k"] * lay["flat_slot_bytes"]
    return lay["ram_total"] - lay["stack"] - lay["fixed_core"] - pl["frame_region"] - caches


def plan_map(lay, m):
    """Return [plan dict per skill group] and a report list."""
    out, rep = [], []
    for g in range(3):
        items = level_fixed(lay, m, g)
        lf = sum(b for _, b in items)
        mon = m.monsters_by_group[g]
        need = need_play(lay, mon)
        chosen = None
        tried = []
        for p in F.PLANS:
            arena = arena_bytes(lay, p)
            pages = (arena - lf) // lay["page_bytes"]
            tried.append((p, arena, pages))
            if pages >= need + lay["reserve_pages"]:
                chosen = (p, arena, pages)
                break
        if chosen is None:
            raise ConvError("%s group %d: no RAM plan fits (level fixed %d B, need %d pages, tried %s)" % (
                m.name, g, lf, need + lay["reserve_pages"], tried))
        p, arena, pages = chosen
        check16 = [pages, m.drops_by_group[g], m.light_slots, len(m.zclip), mon, need, arena]
        if any(v < 0 or v > 0xFFFF for v in check16):
            raise ConvError("%s: plan field out of range" % m.name)
        pl = lay["plans"][p]
        rec = {"plan": F.PLANS.index(p), "w": pl["w"], "k": pl["k"], "reserve": lay["reserve_pages"],
               "cache_lines": lay["map_cache_lines"], "pages": pages, "drop_slots": m.drops_by_group[g],
               "mover_slots": min(len(m.mutable), lay["level_fixed"]["mover_max"]),
               "light_slots": m.light_slots, "zclip_slots": len(m.zclip), "monsters": mon,
               "need_play": need, "arena_bytes": arena}
        out.append(rec)
        corpses = min(mon, 7 * max(0, pages - need))
        rep.append({"group": g, "plan": p, "arena": arena, "level_fixed": lf, "pages": pages,
                    "awake_cap": min(mon, pages - 1 - lay["transients"]), "need_play": need,
                    "monsters": mon, "corpses_kept_at_need": corpses,
                    "all_awake_fits": pages >= 1 + lay["transients"] + mon,
                    "level_fixed_items": dict(items)})
    return out, rep
