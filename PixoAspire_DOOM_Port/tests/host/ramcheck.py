#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""RAM layout check per map and skill group.

For every map of the image and every skill group (skills 2, 3 and 4) the
engine loads the level (two tics) and reports its level arena: the plan,
the level-fixed bytes it carved and the pages left. The check adds

  stack + fixed core + excess + frame region of the plan + caches of the
  plan + level-fixed bytes + (need_play + reserve) pages of 60 B

and fails when the sum is above 32,768 B, when the arena has fewer pages
than need_play + reserve, or fewer than the converter planned, or when the
plan's arena does not fit between the caches and the end of SRAM. Stack,
fixed core, frame regions and caches come from the RAM layout file of the
converter (build/ram_layout.json or tools/conv/ram_layout_default.json).

The save slot column is what a SAVE.DAT slot of the level holds: the 64 B
header, the globals block at its cap, the arena and the order block (a u16
per page, per drop slot and one more). It fails above the slot size of
16,384 B.

The fixed core holds 400 B for the static globals of the game logic
(the engine globals p_*). The ARM link map (--map, required)
gives the measured .data + .bss of the game logic modules (p_*, p_mapdata,
port/obj): what is above 400 B is the excess added to every total. The work
areas of the game logic are in the tic scratch of the frame region, not in
the fixed core.

usage: ramcheck.py --engine BIN --nor IMAGE --map FILE [--layout FILE]
"""
import argparse
import json
import os
import re
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, ROOT)
from tools.conv import format as F  # noqa: E402

RAM = 32768
SAVE_SLOT = F.SAVE_SIZE // F.SAVE_SLOTS
SKILLS = (2, 3, 4)
PLANS = 'ABCD'
GAME_MODULES = ('things', 'pages', 'chains', 'movers', 'lights', 'rview', 'level', 'p_mapdata')
GAME_GLOBALS = 400      # bytes the fixed core of the layout holds for the globals p_*


def layout(path):
    if path is None:
        cand = os.path.join(ROOT, 'build', 'ram_layout.json')
        path = cand if os.path.exists(cand) else os.path.join(ROOT, 'tools', 'conv',
                                                              'ram_layout_default.json')
    with open(path) as f:
        return json.load(f), path


def level_line(engine, nor, m, skill, tmpdir):
    out = os.path.join(tmpdir, 'e1m%d-%d.txt' % (m, skill))
    cmd = [engine, '--nor', nor, '-warp', '1', str(m), '-skill', str(skill), '--tics', '2',
           '--obj-stats', out]
    r = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, cwd=tmpdir)
    if r.returncode or not os.path.exists(out):
        sys.exit('%s failed: %s' % (' '.join(cmd), r.stderr.decode(errors='replace').strip()))
    with open(out) as f:
        for line in f:
            w = line.split()
            if w and w[0] == 'level':
                d = {'level': w[1]}
                d.update(zip(w[2::2], w[3::2]))
                return d
    sys.exit('%s: no level line' % out)


def static_ram(mapfile):
    """.data + .bss per module of the ARM link map (sizegate.py parser)."""
    sys.path.insert(0, os.path.join(ROOT, 'tools', 'gates'))
    import sizegate
    per_mod, _ = sizegate.parse_map(mapfile)
    return per_mod


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--engine', required=True)
    ap.add_argument('--nor', required=True)
    ap.add_argument('--layout')
    ap.add_argument('--map', required=True, help='ARM link map for the static RAM of the game logic')
    a = ap.parse_args()
    lay, lpath = layout(a.layout)
    page = lay['page_bytes']
    reserve = lay['reserve_pages']
    print('RAM layout check, %s: stack %d, fixed core %d, page %d B, reserve %d pages' % (
        os.path.relpath(lpath, ROOT), lay['stack'], lay['fixed_core'], page, reserve))
    per_mod = static_ram(a.map)
    game = {m: v['data'] + v['bss'] for m, v in per_mod.items()
            if m.startswith('p_') or m in GAME_MODULES}
    measured = sum(game.values())
    excess = max(0, measured - GAME_GLOBALS)
    print('static RAM of the game logic on ARM (.data + .bss of p_*, port/obj): %d B against '
          '%d B in the fixed core, excess %d B (%s)' % (
              measured, GAME_GLOBALS, excess,
              ', '.join('%s %d' % (m, v) for m, v in sorted(game.items(), key=lambda x: -x[1]) if v)))
    core = lay['fixed_core'] + excess
    print('fixed: level-fixed arena bytes carved by the engine, pages: pages left in the arena, '
          'conv: pages the converter planned, need: need_play + reserve, total: RAM with need '
          'pages in use, slot: save slot bytes')
    print('%-5s %5s %4s %6s %6s %6s %6s %6s %6s %5s %5s %6s %6s %6s' % (
        'map', 'skill', 'plan', 'frame', 'caches', 'arena', 'fixed', 'pages', 'conv', 'need',
        'spare', 'total', 'free', 'slot'))
    ok = True
    tmpdir = tempfile.mkdtemp(dir=os.path.join(ROOT, 'build'))
    try:
        for m in range(1, 10):
            for sk in SKILLS:
                d = level_line(os.path.abspath(a.engine), os.path.abspath(a.nor), m, sk, tmpdir)
                plan = d['plan']
                pl = lay['plans'][plan]
                caches = lay['map_cache_bytes'] + pl['k'] * lay['flat_slot_bytes']
                fixed = int(d['fixed'])
                pages = int(d['pages'])
                arena = int(d['arena'])
                need = int(d['need_play']) + reserve
                used = ((fixed + 3) & ~3) + need * page
                total = lay['stack'] + core + pl['frame_region'] + caches + used
                conv = int(d['plan_pages'])
                slot = (F.SAVE_HEADER.size + F.SAVE_GLOBALS_MAX + arena
                        + 2 * (pages + int(d['drops']) + 1))
                bad = []
                if total > RAM:
                    bad.append('over 32 KB')
                if pages < need:
                    bad.append('fewer pages than need_play + reserve')
                if pages < conv:
                    bad.append('fewer pages than the converter planned')
                if lay['stack'] + core + pl['frame_region'] + caches + arena > RAM:
                    bad.append('arena beyond the end of RAM')
                if slot > SAVE_SLOT:
                    bad.append('save slot over %d B' % SAVE_SLOT)
                ok &= not bad
                print('%-5s %5d %4s %6d %6d %6d %6d %6d %6s %5d %5d %6d %6d %6d%s' % (
                    d['level'], sk, plan, pl['frame_region'], caches, arena, fixed, pages,
                    conv, need, pages - need, total, RAM - total, slot,
                    '  FAIL: ' + ', '.join(bad) if bad else ''))
    finally:
        for n in os.listdir(tmpdir):
            os.remove(os.path.join(tmpdir, n))
        os.rmdir(tmpdir)
    print('PASS' if ok else 'FAIL')
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
