#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Frame test: the 3D view of the engine (NOR image) against the oracle (WAD).

The oracle is linked with tests/lockstep/attr.c, which records for every
finished 3D view the pixels and, per pixel, the drawer that wrote it last:
the column drawer (walls, sky, sprites, masked middles and the weapon), the
span drawer (floors and ceilings) or the fuzz drawer (spectres). The engine's
strip renderer is built with DMC_ATTR and writes the same files from its
strips (host/rattr_host.c). Plane pixels are known from the oracle's own span
drawer, so no extra run is needed to find them.

The oracle runs with screenblocks 11 (DMC_ATTR_VIEW, attr.c), so its 3D view
fills the 320x200 screen as the engine's does (the engine's status bar is in
the band below the view), and with the engine's detail level. The engine
draws walls at their stored base level (--no-lod).

A pixel is compared when both programs drew it with the column drawer. Those
must be equal: the E1M1 pack keeps walls and sprites at full resolution. Plane
pixels differ (the pack's flats are 32x32) and are only counted, as are fuzz
pixels (they read the plane pixels around them). Two kinds of column over-read are
counted apart, the engine marks them (r_draw.c, r_things.c):
  class 4, walls and sky: the drawer reads past the texture height (vanilla
    rows & 127: one row from rounding at the bottom of a wall, or the
    tutti-frutti of short textures). Vanilla reads the bytes that follow the
    column in its patch or composite, the image repeats the rows.
  class 5, sprites and masked middles: the drawer reads past the post column
    (row 127 from rounding at the top edge of a post, post + 3 + 127).
    Vanilla reads on into its patch lump, the engine reads zeros after its
    rebuilt column. These pixels are an accepted deviation from bit
    exactness.
Pixels whose class differs otherwise would be a geometry error.

usage: frametest.py --oracle BIN --engine BIN --nor IMAGE [--stride N]
                    [--tics N] [--zip FILE | --iwad FILE] [--keep] [--low-detail]
                    [--strip-w W] [--engine-arg ARG]...
The workload is the lockstep E1M1 script up to the exit (tests/lockstep).
--low-detail runs both programs in low detail (160 columns doubled),
--strip-w is the engine's strip width. --engine-arg passes an option to the
engine, e.g. --nor-defer.
"""
import argparse
import os
import shutil
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.join(ROOT, 'tests', 'lockstep'))
import run_lockstep as LS  # noqa: E402

COLUMN, SPAN, FUZZ, BEYOND, POSTREAD = 1, 2, 3, 4, 5


def run(binary, args, outdir, stride, log, view=None):
    os.makedirs(outdir, exist_ok=True)
    env = dict(os.environ, DMC_ATTR_DIR=outdir, DMC_ATTR_STRIDE=str(stride))
    if view:
        env['DMC_ATTR_VIEW'] = view
    with open(log, 'w') as f:
        r = subprocess.run([binary] + args, env=env, stdout=f, stderr=subprocess.STDOUT, cwd=outdir)
    if r.returncode:
        sys.exit('%s exited with %d, see %s' % (binary, r.returncode, log))


def load(path):
    with open(path, 'rb') as f:
        data = f.read()
    w, h = struct.unpack_from('<HH', data, 0)
    n = w * h
    return w, h, data[4:4 + n], data[4 + n:4 + 2 * n]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--oracle', required=True)
    ap.add_argument('--engine', required=True)
    ap.add_argument('--nor', required=True)
    ap.add_argument('--zip')
    ap.add_argument('--iwad', help='IWAD of the oracle instead of freedoom1.wad of the zip')
    ap.add_argument('--stride', type=int, default=8)
    ap.add_argument('--tics', type=int, default=LS.E1M1_TICS)
    ap.add_argument('--keep', action='store_true', help='keep the frame files')
    ap.add_argument('--low-detail', action='store_true', help='low detail in both programs')
    ap.add_argument('--strip-w', type=int, default=8, help='strip width of the engine')
    ap.add_argument('--engine-arg', action='append', default=[], help='engine option, repeatable')
    a = ap.parse_args()
    out = os.path.join(ROOT, 'build', 'frametest')
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    LS.OUT = os.path.join(ROOT, 'build', 'lockstep')
    os.makedirs(LS.OUT, exist_ok=True)
    wad = os.path.abspath(a.iwad) if a.iwad else LS.get_wad(a.zip)
    cmds = os.path.join(out, 'e1m1_script.cmds')
    LS.make_cmds(cmds, a.tics)
    game = ['-warp', '1', '1', '-skill', '4', '--cmds', cmds, '--tics', str(a.tics)]
    eargs = ['--nor', os.path.abspath(a.nor), '--no-lod', '--strip-w', str(a.strip_w)]
    if a.low_detail:
        eargs.append('--low-detail')
    print('engine: %s detail, strips of %d columns%s' % (
        'low' if a.low_detail else 'high', a.strip_w,
        ', ' + ' '.join(a.engine_arg) if a.engine_arg else ''))
    run(os.path.abspath(a.oracle), ['-iwad', wad] + game, os.path.join(out, 'oracle'), a.stride,
        os.path.join(out, 'oracle.log'), '11,%d' % (1 if a.low_detail else 0))
    run(os.path.abspath(a.engine), eargs + a.engine_arg + game,
        os.path.join(out, 'engine'), a.stride, os.path.join(out, 'engine.log'))

    names = sorted(n for n in os.listdir(os.path.join(out, 'oracle')) if n.endswith('.bin'))
    tot = dict(frames=0, compared=0, mismatch=0, plane=0, fuzz=0, classdiff=0, beyond=0,
               beyond_diff=0, postread=0, postread_diff=0, undrawn=0)
    worst = []
    for n in names:
        pe = os.path.join(out, 'engine', n)
        if not os.path.exists(pe):
            sys.exit('engine has no frame %s' % n)
        w, h, po, co = load(os.path.join(out, 'oracle', n))
        w2, h2, pg, cg = load(pe)
        if (w, h) != (w2, h2):
            sys.exit('%s: frame sizes differ' % n)
        mism = comp = 0
        for i in range(w * h):
            c1, c2 = co[i], cg[i]
            if c1 == COLUMN and c2 == BEYOND:
                tot['beyond'] += 1
                tot['beyond_diff'] += po[i] != pg[i]
            elif c1 == COLUMN and c2 == POSTREAD:
                tot['postread'] += 1
                tot['postread_diff'] += po[i] != pg[i]
            elif c1 != c2:
                tot['classdiff'] += 1
            elif c1 == COLUMN:
                comp += 1
                if po[i] != pg[i]:
                    mism += 1
            elif c1 == SPAN:
                tot['plane'] += 1
            elif c1 == FUZZ:
                tot['fuzz'] += 1
            elif c1 == 0:
                tot['undrawn'] += 1
        tot['frames'] += 1
        tot['compared'] += comp
        tot['mismatch'] += mism
        if mism:
            worst.append((mism, n))
    print('frames %d, wall/sky/sprite pixels compared %d, different %d' % (
        tot['frames'], tot['compared'], tot['mismatch']))
    print('plane pixels (not compared) %d, fuzz pixels %d, pixels with a different drawer %d' % (
        tot['plane'], tot['fuzz'], tot['classdiff']))
    print('pixels drawn by nothing in both (vanilla HOM) %d' % tot['undrawn'])
    print('wall/sky pixels read past the texture height (not compared) %d, of them different %d' % (
        tot['beyond'], tot['beyond_diff']))
    print('sprite/masked pixels read past the post column (not compared, accepted deviation) %d, '
          'of them different %d' % (tot['postread'], tot['postread_diff']))
    for m, n in sorted(worst, reverse=True)[:10]:
        print('  %s: %d different pixels' % (n, m))
    if not a.keep:
        shutil.rmtree(out, ignore_errors=True)
    ok = tot['frames'] > 0 and tot['mismatch'] == 0 and tot['classdiff'] == 0
    print('PASS' if ok else 'FAIL')
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
