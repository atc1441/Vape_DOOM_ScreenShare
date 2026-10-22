#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""A save and a load from the menu of the host engine, the way a player makes them.

Run A plays a stored route of the lockstep exits workload (tests/lockstep/routes) from the
start of its map, opens the menu with keys (--keys) at tic SAVE_KEYS, goes to Save Game and
saves into slot 0: the save travels in the ticcmd (BTS_SAVEGAME) and is written at the start of
the next tic. Its NOR image is written out (--nor-out). Run B starts from the title on that
image with the same route, opens the menu, goes to Load Game and loads slot 0: the level of the
save is set up and the slot put over it, the running gametic is kept. gametic is hashed as 0
(--hash-no-gametic) and the route follows the level time, so from the load on B must give the
lines of A from the save on, in order, to the end of A. Maps E1M1, E1M4 and E1M6.

usage: savemenu.py [--engine BIN] [--nor E1.img] [--tag NAME]
Outputs go to build/savemenu/NAME (default: default), removed when a case passes.
"""
import argparse
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.join(ROOT, 'tests', 'lockstep'))
import run_lockstep as LS  # noqa: E402

CASES = (('e1m1', 1, 4), ('e1m4', 4, 4), ('e1m6', 6, 1))
TICS = 900
# The menu over the level: Save Game (the fourth row), slot 0 in tic 330, saved in tic 331
SAVE_KEYS = ((300, 'KEY_ESCAPE'), (305, 'KEY_DOWNARROW'), (310, 'KEY_DOWNARROW'),
             (315, 'KEY_DOWNARROW'), (320, 'KEY_ENTER'), (330, 'KEY_ENTER'))
# The menu over the title: Load Game (the third row), slot 0 loaded in tic 45
LOAD_KEYS = ((20, 'KEY_ESCAPE'), (25, 'KEY_DOWNARROW'), (30, 'KEY_DOWNARROW'),
             (35, 'KEY_ENTER'), (45, 'KEY_ENTER'))


def keys(path, steps):
    with open(path, 'w') as f:
        for tic, key in steps:
            f.write('%d %s down\n%d %s up\n' % (tic, key, tic + 1, key))


def run(engine, args, d, log):
    with open(os.path.join(d, log), 'w') as f:
        return subprocess.run([engine] + args, stdout=f, stderr=subprocess.STDOUT, cwd=d).returncode


def case(a, out, name, m, sk):
    d = os.path.join(out, name)
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    route = os.path.join(ROOT, 'tests', 'lockstep', 'routes', 'e1m%d_s%d.route' % (m, sk))
    ka, kb = os.path.join(d, 'a.keys'), os.path.join(d, 'b.keys')
    keys(ka, SAVE_KEYS)
    keys(kb, LOAD_KEYS)
    img, at, bt = (os.path.join(d, n) for n in ('a.img', 'a.txt', 'b.txt'))
    save, load = SAVE_KEYS[-1][0] + 1, LOAD_KEYS[-1][0]
    rc = run(a.engine, ['--nor', a.nor, '-warp', '1', str(m), '-skill', str(sk), '--route', route,
                        '--keys', ka, '--hash-no-gametic', '--tics', str(TICS), '--nor-out', img,
                        '--hash-out', at], d, 'a.log')
    rc |= run(a.engine, ['--nor', img, '--route', route, '--keys', kb, '--hash-no-gametic',
                         '--tics', str(TICS - save + load), '--hash-out', bt], d, 'b.log')
    sa = [h for t, h in LS.read_stream(at) if int(t) >= save]
    sb = [h for t, h in LS.read_stream(bt) if int(t) >= load]
    good = rc == 0 and len(sa) == TICS - save and sa == sb
    if good:
        shutil.rmtree(d, ignore_errors=True)
        return True, '[%s] skill %d: the save of tic %d loaded in tic %d, %d tics identical' % (
            name, sk, save, load, len(sa))
    diff = LS.first_diff(sa, sb)
    return False, '[%s] FAIL: exit %d, %d and %d tics from the save, they differ from the %s-th, ' \
        'files in %s' % (name, rc, len(sa), len(sb), diff, d)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--engine', default=os.path.join(ROOT, 'build', 'host', 'dmcdoom_skip'))
    ap.add_argument('--nor', default=os.path.join(ROOT, 'build', 'nor', 'E1.img'))
    ap.add_argument('--tag', default='default')
    a = ap.parse_args()
    a.engine, a.nor = os.path.abspath(a.engine), os.path.abspath(a.nor)
    out = os.path.join(ROOT, 'build', 'savemenu', a.tag)
    os.makedirs(out, exist_ok=True)
    ok = True
    for name, m, sk in CASES:
        good, line = case(a, out, name, m, sk)
        ok &= good
        print(line)
    print('PASS' if ok else 'FAIL')
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
