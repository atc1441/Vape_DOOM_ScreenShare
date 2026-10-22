#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Renderer work counters, pool high-water marks and a device timing model.

Runs the engine with --rstats-out (one line of counters per rendered frame,
port/render/r_strip.h) on DEMO1 (E1M6, E1 pack) and on the lockstep E1M1
script (E1M1 pack), in the device configuration (low detail, strip width,
floors and walls with LOD as the map's plan says) and in variants, then
prints per workload:
  - mean / p95 / max per frame of every counter
  - per-strip pool high-water marks (p50 / p95 / p99.9 / max over frames)
    against the pool sizes, strip splits and frames that split
  - a frame-time estimate from the counters with the costs measured on the
    meter (col8 18 cycles/px, span8 30 cycles/px, NOR 2.8 us + 8 MB/s per
    sync read, pump ISR 744 cycles per 100 px) and estimated per-item costs
    (node 350, seg 550, fragment replay 1,400, column setup 260, plane 425,
    span setup 130, sprite 1,800, strip 1,500 cycles). It is an estimate,
    not a measurement.
    Column reads are sent one column ahead (the request of a column goes out
    before the column queued ahead of it is drawn): the engine counts the
    cycles the drawers still wait for them (colwait), every other NOR read
    (records, flats) counts as blocking. The NOR line without prefetch is
    printed too.
  - the calls of the exact table decoders (port/math/trig.h) per frame:
    finesine and finecosine, the inline finetangent of the wall columns and
    tantoangle during the view, and those of the game logic in the tic
    before it (the host draws every tic). Their device time is estimated
    from the code each one runs in the ARM -Oz link, at 1.25 cycles per
    byte of code, 2 per taken branch and 4 per load from flash (this fits
    call loops timed on the meter: empty call 19 cycles, FixedMul as a call
    33): finesine 90 cycles on its common path and about 250 more in the
    16 % of the indices whose block has a fix-up, so about 130 (the
    interpolated quarter wave of DMC_SINE_L1 adds about 80), tantoangle
    about 120, the inline finetangent about 175 on its interpolated path
    (64 on its full-table path, counted as 175). A vanilla table read
    (literal and indexed load from flash) is about 16 cycles.

usage: rstats.py --engine BIN --nor E1.img --nor-e1m1 E1M1.img --out DIR
"""
import argparse
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.join(ROOT, 'tests', 'lockstep'))
import run_lockstep as LS  # noqa: E402

HZ = 144e6
POOLS = {8: (56, 40, 20, 256), 16: (80, 48, 24, 480)}
SHOW = ('nodes', 'segs', 'frags', 'replays', 'wallpx', 'skypx', 'planes', 'spans', 'spanpx',
        'solidpx', 'sprites', 'vissprites', 'sprcols', 'maskedcols', 'colpx', 'fuzzpx',
        'strips', 'splits', 'nor_tx', 'nor_bytes', 'colreq', 'colhit', 'coltx', 'colbytes',
        'colwait', 'band_cols', 'band_pieces', 'band_nor_tx', 'band_nor_bytes', 'trig_sin',
        'trig_tan', 'trig_atan', 'logic_sin', 'logic_atan', 'ctx_fills', 'post_fills', 'col_direct',
        'col_wrap', 'col_lv2', 'fragc_hits', 'solid_runs', 'solid_fillpx')
# Cycles of a decoder call on the meter (see above) and of a vanilla table read
TRIG_SIN, TRIG_TAN, TRIG_ATAN, TRIG_VANILLA = 130, 175, 120, 16


def pct(v, p):
    v = sorted(v)
    return v[min(len(v) - 1, int(p / 100.0 * len(v)))] if v else 0


def model(r, detail):
    """Frame time estimate in ms: CPU, NOR, pump ISR, total, NOR without prefetch."""
    cols = r['wallcols'] + r['skycols'] + r['sprcols'] + r['maskedcols'] + r['pspcols']
    cpu = (350 * r['nodes'] + 550 * r['segs'] + 1400 * r['replays'] + 260 * cols +
           18 * r['colpx'] + 425 * r['planes'] + 130 * r['spans'] + 30 * r['spanpx'] +
           100 * r['solidcols'] + 9 * r['solidpx'] + 1800 * r['vissprites'] +
           1500 * r['strips'] + 18 * r['fuzzpx'])
    sync = r['nor_tx'] * 2.8e-6 + r['nor_bytes'] / 8e6
    nor = ((r['nor_tx'] - r['coltx']) * 2.8e-6 + (r['nor_bytes'] - r['colbytes']) / 8e6 +
           r['colwait'] / HZ)
    pump = 7.44 * 64000 / HZ
    cpu /= HZ
    return cpu * 1e3, nor * 1e3, pump * 1e3, (cpu + nor + pump) * 1e3, sync * 1e3


def trig_model(r):
    """Decoder time in ms: view, tic before it, both over vanilla table reads."""
    view = TRIG_SIN * r['trig_sin'] + TRIG_TAN * r['trig_tan'] + TRIG_ATAN * r['trig_atan']
    logic = TRIG_SIN * r['logic_sin'] + TRIG_ATAN * r['logic_atan']
    calls = r['trig_sin'] + r['trig_tan'] + r['trig_atan'] + r['logic_sin'] + r['logic_atan']
    return view / HZ * 1e3, logic / HZ * 1e3, (view + logic - TRIG_VANILLA * calls) / HZ * 1e3


def load(path):
    """Frame rows and the per-strip histograms (name: {value: strips})."""
    with open(path) as f:
        lines = f.read().split('\n')
    names = lines[0].split()[1:]
    rows, hist = [], {}
    for line in lines[1:]:
        if line.startswith('H '):
            t = line.split()
            hist[t[1]] = {int(t[i]): int(t[i + 1]) for i in range(2, len(t), 2)}
        elif line.strip():
            rows.append(dict(zip(names, map(int, line.split()))))
    return rows, hist


def hpct(h, p):
    total = sum(h.values())
    acc = 0
    for v in sorted(h):
        acc += h[v]
        if acc >= p / 100.0 * total:
            return v
    return 0


def report(label, rows, hist):
    if not rows:
        print('%s: no frames' % label)
        return True
    w = rows[0]['w']
    print('== %s: %d frames, W%d, %s detail, %s floors' % (
        label, len(rows), w, 'low' if rows[0]['detail'] else 'high',
        'textured' if rows[0]['textured'] else 'solid'))
    print('  %-12s %10s %10s %10s' % ('per frame', 'mean', 'p95', 'max'))
    for k in SHOW:
        v = [r[k] for r in rows]
        print('  %-12s %10.1f %10d %10d' % (k, sum(v) / len(v), pct(v, 95), max(v)))
    pools = POOLS[8 if w <= 8 else 16]
    print('  pool use              per strip: p50   p95 p99.9   max | frame high-water: p50   p95'
          ' p99.9   max | pool')
    for i, k in enumerate(('drawsegs', 'visplanes', 'vissprites', 'openings')):
        v = [r['hw_' + k] for r in rows]
        h = hist.get(k, {0: 1})
        print('  %-18s %16d %5d %5d %5d | %21d %5d %5d %5d | %4d' % (
            k, hpct(h, 50), hpct(h, 95), hpct(h, 99.9), max(h), pct(v, 50), pct(v, 95),
            pct(v, 99.9), max(v), pools[i] if rows[0]['textured'] or i != 1 else 0))
    splits = [r['splits'] for r in rows]
    drops = [r['vanilla_drops'] for r in rows]
    print('  strip splits %d in %d frames (%.2f %%), vanilla drops at W1 %d, fragments dropped %d,'
          ' sprites dropped %d' % (sum(splits), sum(1 for s in splits if s),
                                   100.0 * sum(1 for s in splits if s) / len(rows), sum(drops),
                                   sum(r['frags_dropped'] for r in rows),
                                   sum(r['sprites_dropped'] for r in rows)))
    m = [model(r, rows[0]['detail']) for r in rows]
    for i, name in enumerate(('CPU', 'NOR', 'pump ISR', 'total', 'NOR sync')):
        v = [x[i] for x in m]
        print('  model %-9s mean %6.1f ms  p95 %6.1f ms  max %6.1f ms' % (
            name, sum(v) / len(v), pct(v, 95), max(v)))
    m = [trig_model(r) for r in rows]
    for i, name in enumerate(('view', 'tic', 'over vanilla')):
        v = [x[i] for x in m]
        print('  decoders %-12s mean %5.2f ms  p95 %5.2f ms  max %5.2f ms' % (
            name, sum(v) / len(v), pct(v, 95), max(v)))
    return True


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--engine', required=True)
    ap.add_argument('--nor', required=True)
    ap.add_argument('--nor-e1m1', required=True)
    ap.add_argument('--out', required=True)
    a = ap.parse_args()
    out = os.path.abspath(os.path.join(a.out, 'rstats'))
    os.makedirs(out, exist_ok=True)
    cmds = os.path.join(out, 'e1m1_script.cmds')
    LS.make_cmds(cmds, LS.E1M1_TICS)
    demo1 = ['--nor', os.path.abspath(a.nor), '-playdemo', 'demo1']
    e1m1 = ['--nor', os.path.abspath(a.nor_e1m1), '-warp', '1', '1', '-skill', '4', '--cmds', cmds,
            '--tics', str(LS.E1M1_TICS)]
    runs = [
        ('DEMO1, device configuration', demo1 + ['--low-detail']),
        ('DEMO1, W16 textured, 4 flats', demo1 + ['--low-detail', '--strip-w', '16',
                                                  '--solid-floors', '0', '--flat-slots', '4']),
        ('DEMO1, W8 textured, 3 flats', demo1 + ['--low-detail', '--strip-w', '8',
                                                 '--solid-floors', '0', '--flat-slots', '3']),
        ('E1M1 script, device configuration', e1m1 + ['--low-detail']),
        ('E1M1 script, W16', e1m1 + ['--low-detail', '--strip-w', '16']),
        ('E1M1 script, high detail', e1m1),
    ]
    procs = []
    for k, (label, args) in enumerate(runs):
        f = os.path.join(out, 'r%d.txt' % k)
        log = open(f + '.log', 'w')
        procs.append((label, f, log, subprocess.Popen([os.path.abspath(a.engine)] + args +
                                                       ['--rstats-out', f], stdout=log,
                                                       stderr=subprocess.STDOUT)))
    ok = True
    for label, f, log, p in procs:
        p.wait()
        log.close()
        if p.returncode:
            print('%s: engine exited with %d, see %s.log' % (label, p.returncode, f))
            ok = False
            continue
        report(label, *load(f))
    print('PASS' if ok else 'FAIL')
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
