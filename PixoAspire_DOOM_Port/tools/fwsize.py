#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Flash and RAM report of the DOOM firmware (mk/fw.mk) from its ld.lld link map.

usage: fwsize.py --map MAIN.map [--code DOOMCODE.DMC] [--ovl OVL.elf]... [--code-area B]

The map may come from a link that failed because the image or the fixed core is over its
limit (ld.lld writes the map first). Every input section is charged to the module (object
file) it came from, the modules to the groups below:
  flash  .text and .data (in the app slot)
  RAM    .data and .bss, the fixed core
The image size (DOOM.BIN) is .text plus the load image of .data, the bytes no module has are
alignment. Its limit is the code area of the app slot (--code-area). The fixed core is .data
and .bss with their padding, its limit FRAME_BASE (the address of .dynram) minus the end of
the stack, and the stack is .stack. The overlays (copy + bss) and DOOMCODE.DMC in 4 KB
clusters close the report.
"""
import argparse
import os
import re
import struct
import sys

# group, module patterns (object path, regex). The tables group holds COLORMAP, the trig
# and view tables, light, states, mobjinfo, the action, weapon and sfx tables and the font
# (rndtable, the input map and the strings are in the modules that use them).
GROUPS = [
    ("startup, vectors, fault screen", r"/(startup|fault)\.o$"),
    ("platform drivers", r"/plat/[^/]+/(plat_at32|systick|buttons|buzzer|settings|nor|lcd|pump|text|usbd)\.o$"),
    ("runtime library", r"/(rt|libc_dmc)\.o$"),
    ("data layer and overlay loader", r"/port/(data|ovl)/"),
    ("game logic p_*, object store", r"engine/(p_\w+|m_random|m_bbox|m_fixed|doomdef|doomstat|dstrings)\.o$|/port/obj/"),
    ("main loop g_game, d_*, s_sound", r"engine/(g_game|d_main|d_loop|d_event|s_sound|m_misc|m_controls)\.o$"),
    ("renderer and band compositor", r"engine/(r_\w+|v_video)\.o$|/port/render/|/port/math/"),
    ("status bar and message line", r"engine/(st_\w+|hu_\w+)\.o$"),
    ("tables: generated (gen/), action table, font", r"/gen/|engine/(info|tables)\.o$|/font8x8\.o$"),
    ("front end glue", r"/port/game/fe_(glue|play)\.o$"),
    ("firmware: boot, installer, demos, results, input, sound",
     r"/port/game/|/platform/apps/ui\.o$|/tests/lockstep/hash\.o$"),
    ("linker (merged strings, thunks)", r"^<internal>|^$"),
]

LINE_OUT = re.compile(r"^\s*([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+\d+ (\.\S+)\s*$")
LINE_SEC = re.compile(r"^\s*([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+\d+ (\.text|\.data|\.bss|\.stack|\.dynram)\s*$")
LINE_IN = re.compile(r"^\s*([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+\d+\s{9}(.*):\((\S+)\)\s*$")


def parse(path):
    out, mods, secs = None, {}, {}
    with open(path) as f:
        for line in f:
            m = LINE_SEC.match(line)
            if m:
                secs[m.group(4)] = (int(m.group(1), 16), int(m.group(3), 16), int(m.group(2), 16))
            m = LINE_OUT.match(line)
            if m:
                out = m.group(4)
                continue
            m = LINE_IN.match(line)
            if m and out:
                size = int(m.group(3), 16)
                obj = m.group(4)
                obj = obj[:obj.index("(")] if "(" in obj else obj
                mods.setdefault(obj, {}).setdefault(out, 0)
                mods[obj][out] += size
    return mods, secs


def group_of(obj):
    for i, (_, pat) in enumerate(GROUPS):
        if re.search(pat, obj):
            return i
    return len(GROUPS)


def short(obj):
    for cut in ("/obj/", "/plat/"):
        if cut in obj:
            return obj[obj.index(cut) + len(cut):]
    return obj or "<linker>"


def ovl_size(path):
    with open(path, "rb") as f:
        d = f.read()
    _, _, _, _, _, _, shoff, _, _, _, _, shentsize, shnum, shstrndx = struct.unpack_from(
        "<16sHHIIIIIHHHHHH", d, 0)
    secs = [struct.unpack_from("<10I", d, shoff + i * shentsize) for i in range(shnum)]
    names = secs[shstrndx][4]
    sizes = {}
    for s in secs:
        n = d[names + s[0]:d.index(b"\0", names + s[0])].decode()
        sizes[n] = s[5]
    return sizes.get(".ovl", 0), sizes.get(".ovl_bss", 0)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--map", required=True)
    ap.add_argument("--code")
    ap.add_argument("--ovl", action="append", default=[])
    ap.add_argument("--code-area", type=int, default=101376)
    ap.add_argument("--top", type=int, default=24)
    a = ap.parse_args()
    if not os.path.exists(a.map):
        sys.exit(f"{a.map} missing")
    mods, secs = parse(a.map)
    flash = lambda s: s.get(".text", 0) + s.get(".data", 0)
    ram = lambda s: s.get(".data", 0) + s.get(".bss", 0)
    g_flash = [0] * (len(GROUPS) + 1)
    g_ram = [0] * (len(GROUPS) + 1)
    for obj, s in mods.items():
        g = group_of(obj)
        g_flash[g] += flash(s)
        g_ram[g] += ram(s)

    print(f"DOOM firmware size report ({a.map})")
    print(f"{'group':58s} {'flash':>7s} {'RAM':>6s}")
    for i, (name, _) in enumerate(GROUPS + [("other", "")]):
        if not g_flash[i] and not g_ram[i]:
            continue
        print(f"{name:58s} {g_flash[i]:7d} {g_ram[i]:6d}")
    total = sum(g_flash)
    over = lambda v, b: f"over by {v - b}" if v > b else f"spare {b - v}"
    if ".text" in secs and ".data" in secs:
        image = secs[".data"][2] + secs[".data"][1] - secs[".text"][0]
        print(f"{'alignment':58s} {image - total:7d}")
        total = image
    print(f"{'flash (DOOM.BIN)':58s} {total:7d}  code area {a.code_area}: "
          f"{over(total, a.code_area)}")
    core = sum(g_ram)
    if all(k in secs for k in (".data", ".bss", ".stack", ".dynram")):
        sbase, ssize, _ = secs[".stack"]
        limit = secs[".dynram"][0] - sbase - ssize
        used = secs[".bss"][0] + secs[".bss"][1] - secs[".data"][0]
        print(f"{'fixed core .data + .bss, padding %d B included' % (used - core):58s} {used:7d}  "
              f"limit {limit}: {over(used, limit)}")
        core = used
        print(f"{'stack':58s} {ssize:7d}  fixed core with the stack {core + ssize} of {limit + ssize}")
    else:
        print(f"{'fixed core .data + .bss':58s} {core:7d}")
    for path in a.ovl:
        copy, bss = ovl_size(path)
        name = os.path.splitext(os.path.basename(path))[0]
        print(f"{name + ' overlay, copy + bss':58s} {copy + bss:7d}")
    if a.code:
        size = os.path.getsize(a.code)
        print(f"{'DOOMCODE.DMC':58s} {size:7d}  {(size + 4095) // 4096} clusters of 4 KB")

    print(f"\nfixed core by module (top {a.top}):")
    rows = sorted(((ram(s), obj) for obj, s in mods.items() if ram(s)), reverse=True)
    for n, obj in rows[:a.top]:
        print(f"  {n:6d}  {short(obj)}")
    print(f"\nflash by module (top {a.top}):")
    rows = sorted(((flash(s), obj) for obj, s in mods.items()), reverse=True)
    for n, obj in rows[:a.top]:
        print(f"  {n:6d}  {short(obj)}")


if __name__ == "__main__":
    main()
