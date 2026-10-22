#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Status band against the oracle's status bar.

The oracle (screenblocks 10) draws the status bar into rows 168..199 of its
screen. The engine composes it in the status band, LCD rows 208..239, from
dirty widgets only, with the palette tints. Every frame of the lockstep
E1M1 script (stride N) must give the same palette indices there, in high
and low detail and for several strip widths (the band is composed in pieces
that fit a strip buffer), and every frame of DEMO1 on the E1 image in the
device configuration. The engine also checks after every frame that each
pixel of its screen shows the current palette (--lcd-check), which needs the
whole band sent again on every palette change, and that the HUD message
line (LCD rows 200..207) is the message widget as the vanilla code draws it
into a screen (the oracle's text comes from d_englsh.h, the image has
Freedoom's strings, so the line is not compared with the oracle).

usage: bandtest.py --oracle BIN --engine BIN --nor E1M1.img --nor-e1 E1.img --out DIR
                   [--stride N]
"""
import argparse
import os
import struct
import subprocess
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.join(ROOT, 'tests', 'lockstep'))
import run_lockstep as LS  # noqa: E402


def png_pixels(path):
    """Width and palette indices of an indexed PNG written by oracle/png.c."""
    with open(path, 'rb') as f:
        data = f.read()
    pos, idat, w = 8, b'', 0
    while pos < len(data):
        n, kind = struct.unpack_from('>I4s', data, pos)
        body = data[pos + 8:pos + 8 + n]
        if kind == b'IHDR':
            w = struct.unpack_from('>I', body)[0]
        elif kind == b'IDAT':
            idat += body
        pos += 12 + n
    raw = zlib.decompress(idat)
    return w, b''.join(raw[y * (w + 1) + 1:(y + 1) * (w + 1)] for y in range(len(raw) // (w + 1)))


def run(binary, args, log):
    # the oracle writes its save directory into the working directory
    with open(log, 'w') as f:
        r = subprocess.run([binary] + args, stdout=f, stderr=subprocess.STDOUT,
                           cwd=os.path.dirname(log))
    if r.returncode:
        sys.exit('%s exited with %d, see %s' % (binary, r.returncode, log))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--oracle', required=True)
    ap.add_argument('--engine', required=True)
    ap.add_argument('--nor', required=True)
    ap.add_argument('--nor-e1', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--stride', type=int, default=4)
    ap.add_argument('--zip')
    a = ap.parse_args()
    out = os.path.abspath(os.path.join(a.out, 'bandtest'))
    os.makedirs(out, exist_ok=True)
    LS.OUT = os.path.join(ROOT, 'build', 'lockstep')
    os.makedirs(LS.OUT, exist_ok=True)
    wad = LS.get_wad(a.zip)
    cmds = os.path.join(out, 'e1m1_script.cmds')
    LS.make_cmds(cmds, LS.E1M1_TICS)
    e1m1 = ['-warp', '1', '1', '-skill', '4', '--cmds', cmds, '--tics', str(LS.E1M1_TICS),
            '--frame-stride', str(a.stride)]
    demo1 = ['-playdemo', 'demo1', '--frame-stride', str(a.stride)]
    oracle = {}
    for name, game in (('e1m1', e1m1), ('demo1', demo1)):
        od = oracle[name] = os.path.join(out, 'oracle_' + name)
        os.makedirs(od, exist_ok=True)
        run(os.path.abspath(a.oracle), ['-iwad', wad, '--frame-out', od] + game, od + '.log')
    ok = True
    for name, detail, w in (('e1m1', 'high', 8), ('e1m1', 'low', 8), ('e1m1', 'low', 1),
                            ('e1m1', 'low', 16), ('demo1', 'low', 0)):
        od = oracle[name]
        ed = os.path.join(out, 'engine_%s_%s_w%d' % (name, detail, w))
        os.makedirs(ed, exist_ok=True)
        eargs = ['--nor', os.path.abspath(a.nor if name == 'e1m1' else a.nor_e1),
                 '--frame-out', ed, '--lcd-check'] + (e1m1 if name == 'e1m1' else demo1)
        if w:
            eargs += ['--strip-w', str(w)]
        if detail == 'low':
            eargs.append('--low-detail')
        run(os.path.abspath(a.engine), eargs, ed + '.log')
        with open(ed + '.log') as f:
            for line in f:
                if line.startswith('lcd-check'):
                    print('  ' + line.strip())
        frames = diff = bad_frames = 0
        first = None
        for n in sorted(os.listdir(od)):
            if not n.endswith('.png'):
                continue
            e = os.path.join(ed, n)
            if not os.path.exists(e):
                continue
            ow, op = png_pixels(os.path.join(od, n))
            ew, ep = png_pixels(e)
            o = op[168 * ow:200 * ow]
            g = ep[208 * ew:240 * ew]
            d = sum(1 for i in range(len(o)) if o[i] != g[i])
            frames += 1
            if d:
                diff += d
                bad_frames += 1
                first = first or (n, d)
        print('%-5s %s detail %-4s %d frames compared, %d with differences (%d pixels)%s' % (
            name, detail, 'W%d' % w if w else 'plan', frames, bad_frames, diff,
            ', first %s (%d pixels)' % first if first else ''))
        ok &= frames > 0 and not diff
    print('PASS' if ok else 'FAIL')
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
