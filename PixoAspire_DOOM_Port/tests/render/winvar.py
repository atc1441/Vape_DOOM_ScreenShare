#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""W-invariance of the strip renderer.

The engine writes per frame a hash of the palette indices of the 3D view and
of the status band as the emulated LCD shows them (--view-hash). Every strip
width must give the same streams as W = 1, for every frame of the workload:
  demo1  Freedoom's DEMO1 (E1M6) on the E1 pack image
  e1m1   the lockstep E1M1 script (3200 tics) on the E1M1 pack image
in high and low detail, with textured and with solid floors, walls with LOD.
Strips start cleared (--strip-clear), so a pixel that nothing draws (vanilla
HOM) reads the same for every W. On a difference the first differing frame
is written for both widths and the differing pixels are counted.

usage: winvar.py --engine BIN --nor E1.img --nor-e1m1 E1M1.img --out DIR
                 [--widths 1,2,4,8,16,160] [--jobs N]
"""
import argparse
import concurrent.futures
import os
import struct
import subprocess
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.join(ROOT, 'tests', 'lockstep'))
import run_lockstep as LS  # noqa: E402


def run(engine, args, out_file, log):
    with open(log, 'w') as f:
        r = subprocess.run([engine] + args + ['--view-hash', out_file], stdout=f,
                           stderr=subprocess.STDOUT)
    if r.returncode:
        sys.exit('%s exited with %d, see %s' % (engine, r.returncode, log))
    with open(out_file) as f:
        return [line.split() for line in f if line.strip()]


def png_pixels(path):
    """Palette indices of an indexed PNG written by oracle/png.c."""
    with open(path, 'rb') as f:
        data = f.read()
    pos, idat, w = 8, b'', 0
    while pos < len(data):
        n, kind = struct.unpack_from('>I4s', data, pos)
        body = data[pos + 8:pos + 8 + n]
        if kind == b'IHDR':
            w, h = struct.unpack_from('>II', body)
        elif kind == b'IDAT':
            idat += body
        pos += 12 + n
    raw = zlib.decompress(idat)
    return w, b''.join(raw[y * (w + 1) + 1:(y + 1) * (w + 1)] for y in range(len(raw) // (w + 1)))


def explain(engine, base_args, wa, wb, tic, outdir):
    shots = {}
    for w in (wa, wb):
        d = os.path.join(outdir, 'w%d' % w)
        os.makedirs(d, exist_ok=True)
        subprocess.run([engine] + base_args + ['--strip-w', str(w), '--frame-out', d,
                                               '--tics', str(tic + 2)],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        shots[w] = png_pixels(os.path.join(d, 'f%06d.png' % tic))
    (w, a), (_, b) = shots[wa], shots[wb]
    diff = [i for i in range(len(a)) if a[i] != b[i]]
    if diff:
        xs = [i % w for i in diff]
        ys = [i // w for i in diff]
        print('    tic %d: %d pixels differ between W%d and W%d, x %d..%d, y %d..%d (frames in %s)'
              % (tic, len(diff), wa, wb, min(xs), max(xs), min(ys), max(ys), outdir))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--engine', required=True)
    ap.add_argument('--nor', required=True)
    ap.add_argument('--nor-e1m1', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--widths', default='1,2,4,8,16,160')
    ap.add_argument('--jobs', type=int, default=min(16, os.cpu_count() or 1))
    a = ap.parse_args()
    engine = os.path.abspath(a.engine)
    out = os.path.abspath(os.path.join(a.out, 'winvar'))
    os.makedirs(out, exist_ok=True)
    widths = [int(w) for w in a.widths.split(',')]

    cmds = os.path.join(out, 'e1m1_script.cmds')
    LS.make_cmds(cmds, LS.E1M1_TICS)
    workloads = {
        'demo1': ['--nor', os.path.abspath(a.nor), '-playdemo', 'demo1'],
        'e1m1': ['--nor', os.path.abspath(a.nor_e1m1), '-warp', '1', '1', '-skill', '4',
                 '--cmds', cmds, '--tics', str(LS.E1M1_TICS)],
    }
    configs = []
    for name, wl in workloads.items():
        for detail in ('high', 'low'):
            for floors in ('textured', 'solid'):
                args = wl + ['--strip-clear', '--solid-floors', '1' if floors == 'solid' else '0',
                             '--flat-slots', '0' if floors == 'solid' else '4']
                if detail == 'low':
                    args.append('--low-detail')
                configs.append(('%s %s detail %s floors' % (name, detail, floors), args))

    jobs = {}
    with concurrent.futures.ThreadPoolExecutor(a.jobs) as pool:
        for k, (label, args) in enumerate(configs):
            for w in widths:
                f = os.path.join(out, 'c%d_w%d.txt' % (k, w))
                jobs[(k, w)] = pool.submit(run, engine, args + ['--strip-w', str(w)], f, f + '.log')
    ok = True
    for k, (label, args) in enumerate(configs):
        ref = jobs[(k, widths[0])].result()
        frames = sum(1 for r in ref if r[3] == '1')
        bad = []
        for w in widths[1:]:
            s = jobs[(k, w)].result()
            if s == ref:
                continue
            first = next((i for i in range(min(len(s), len(ref))) if s[i] != ref[i]), None)
            bad.append(w)
            if first is None:
                print('  %s: W%d gives %d frames, W%d %d' % (label, w, len(s), widths[0], len(ref)))
            else:
                part = 'view' if s[first][1] != ref[first][1] else 'band'
                print('  %s: W%d differs from W%d from tic %s (%s)' % (label, w, widths[0],
                                                                     s[first][0], part))
                explain(engine, args, widths[0], w, int(s[first][0]),
                        os.path.join(out, 'diff_c%d_w%d' % (k, w)))
        print('%-40s %5d view frames, W %s: %s' % (label, frames, a.widths,
                                                  'identical' if not bad else 'DIFFERENT'))
        ok &= not bad
    print('PASS' if ok else 'FAIL')
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
