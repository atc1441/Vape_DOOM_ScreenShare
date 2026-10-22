#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""The game pad (port/game/pad.h) in the host engine, driven by --pad.

Each case runs the engine on the E1 image with a pad script and reads the game state of one
tic from its hash dump (--hash-dump): the player's position, angle, weapon, ammo and the game
state. Checked: a pad at rest and a pad during a demo change nothing (same hash stream as
without --pad), the left stick moves forward and strafes, the right stick turns at least at
the vanilla fast turn rate at full deflection, small deflections stay in the dead zone and
half deflection is slower than half speed, fire and use, the dial and the weapon buttons
change weapon, and from the title the pad opens the menu, moves its cursor with flicks (auto
repeat while held) and selects with Enter.

usage: test_pad.py [--engine BIN] [--nor E1.img]
Outputs go to build/padtest, removed when every case passes.
"""
import argparse
import math
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

FIRE, USE, MENU, MAP, RUN, NEXTW, PREVW, ENTER = (1 << i for i in range(8))
FRACUNIT = 1 << 16
FAST_TURN = 1280 << 16  # angleturn[1] of g_game.c as an angle
WP = {'fist': 0, 'pistol': 1, 'shotgun': 2, 'chaingun': 3, 'chainsaw': 7}
LEVEL = ['-warp', '1', '1', '-skill', '3']

failures = []


def check(cond, msg):
    print(('ok   ' if cond else 'FAIL ') + msg)
    if not cond:
        failures.append(msg)


class Engine:
    def __init__(self, engine, nor, out):
        self.engine, self.nor, self.out = engine, nor, out
        self.n = 0

    def run(self, args, pad=None, tics=40):
        """Runs tics tics. pad is a list of (tic, lx, ly, rx, ry, dial, buttons). Returns the
        dump of the last tic as a dict and the hash stream as a list of lines."""
        self.n += 1
        base = os.path.join(self.out, 'r%02d' % self.n)
        cmd = [self.engine, '--nor', self.nor] + args + [
            '--tics', str(tics), '--hash-dump', str(tics - 1), base + '.txt',
            '--hash-out', base + '.hash']
        if pad is not None:
            with open(base + '.pad', 'w') as f:
                for line in pad:
                    f.write('%d %d %d %d %d %d %#x\n' % line)
            cmd += ['--pad', base + '.pad']
        with open(base + '.log', 'w') as log:
            rc = subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT).returncode
        if rc:
            sys.exit('engine failed (%d): %s, see %s.log' % (rc, ' '.join(cmd), base))
        dump = {}
        with open(base + '.txt') as f:
            for line in f:
                k, v = line.split()
                dump[k] = v
        with open(base + '.hash') as f:
            return dump, f.read().splitlines()


def player(d):
    """x, y in map units, the angle as a signed 32-bit value, weapon, clip ammo"""
    m = d['pl.mo[0]']
    return (int(d['mo.x[%s]' % m]) / FRACUNIT, int(d['mo.y[%s]' % m]) / FRACUNIT,
            int(d['mo.angle[%s]' % m]), int(d['pl.readyweapon[0]']), int(d['pl.ammo[0]']))


def held(lx=0, ly=0, rx=0, ry=0, dial=0, buttons=0):
    """A pad held from tic 0 on"""
    return [(0, lx, ly, rx, ry, dial, buttons)]


def pulses(*steps, length=1):
    """A pad at rest with the given (tic, lx, ly, rx, ry, dial, buttons) states, each held for
    length tics"""
    pad = [(0, 0, 0, 0, 0, 0, 0)]
    for s in steps:
        pad += [s, (s[0] + length, 0, 0, 0, 0, 0, 0)]
    return pad


def moves(e):
    ref, ref_hash = e.run(LEVEL, held())
    _, plain_hash = e.run(LEVEL)
    check(ref_hash == plain_hash, 'a pad at rest gives the hash stream of a run without one')
    x0, y0, a0, _, ammo0 = player(ref)
    th = a0 / 2.0 ** 32 * 2 * math.pi

    def displacement(d):
        x, y, _, _, _ = player(d)
        dx, dy = x - x0, y - y0
        return dx * math.cos(th) + dy * math.sin(th), dy * math.cos(th) - dx * math.sin(th)

    fwd, side = displacement(e.run(LEVEL, held(ly=660))[0])
    check(fwd > 200 and abs(side) < fwd / 8,
          'left stick up moves forward (%.0f forward, %.0f to the left)' % (fwd, side))
    back, side = displacement(e.run(LEVEL, held(ly=-660))[0])
    # a wall stops it soon
    check(back < -40 and abs(side) < -back / 8,
          'left stick down moves back (%.0f forward, %.0f to the left)' % (back, side))
    fwd2, side = displacement(e.run(LEVEL, held(lx=660))[0])
    check(side < -100 and abs(fwd2) < -side / 8,
          'left stick right strafes right (%.0f forward, %.0f to the left)' % (fwd2, side))
    half, _ = displacement(e.run(LEVEL, held(ly=330))[0])
    check(0 < half < fwd / 2,
          'half deflection moves forward, slower than half speed (%.0f of %.0f)' % (half, fwd))
    dead, dead_hash = e.run(LEVEL, held(lx=-50, ly=50, rx=50))
    check(dead_hash == ref_hash, 'deflections inside the dead zone do nothing')

    tics = 20
    turn = e.run(LEVEL, held(rx=660), tics)[0]
    ref20 = e.run(LEVEL, held(), tics)[0]
    da = (player(turn)[2] - player(ref20)[2] + 2 ** 31) % 2 ** 32 - 2 ** 31
    check(da <= -FAST_TURN * (tics - 1),
          'right stick right turns right at full deflection, %.2f of the fast turn rate'
          % (-da / FAST_TURN / tics))
    left = e.run(LEVEL, held(rx=-300), tics)[0]
    da = (player(left)[2] - player(ref20)[2] + 2 ** 31) % 2 ** 32 - 2 ** 31
    check(0 < da < FAST_TURN * tics / 2,
          'right stick a bit left turns left slowly, %.2f of the fast turn rate'
          % (da / FAST_TURN / tics))

    ammo = player(e.run(LEVEL, held(buttons=FIRE))[0])[4]
    check(ammo < ammo0, 'FIRE held fires the pistol (clip %d -> %d)' % (ammo0, ammo))
    d = e.run(LEVEL, held(buttons=USE))[0]
    check(d['pl.usedown[0]'] == '1', 'USE held is the use button')


def weapons(e):
    god = LEVEL + ['--god']
    two = pulses((10, 0, 0, 0, 0, 660, 0), (30, 0, 0, 0, 0, 500, 0), length=10)
    w = player(e.run(god, two, 100)[0])[3]
    check(w == WP['chaingun'], 'two turns of the dial to the right, each held for 10 tics: '
          'pistol -> shotgun -> chaingun (weapon %d)' % w)
    w = player(e.run(god, pulses((10, 0, 0, 0, 0, -660, 0), length=10), 100)[0])[3]
    check(w == WP['chainsaw'], 'the dial to the left: the weapon before the pistol, the '
          'chainsaw (weapon %d)' % w)
    w = player(e.run(god, pulses((10, 0, 0, 0, 0, 200, 0), length=10), 100)[0])[3]
    check(w == WP['pistol'], 'the dial a bit to the right keeps the weapon (weapon %d)' % w)
    w = player(e.run(god, pulses((10, 0, 0, 0, 0, 0, NEXTW), (50, 0, 0, 0, 0, 0, NEXTW),
                                 (90, 0, 0, 0, 0, 0, PREVW)), 140)[0])[3]
    check(w == WP['shotgun'], 'NEXTW, NEXTW, PREVW: the shotgun (weapon %d)' % w)
    w = player(e.run(LEVEL, pulses((10, 0, 0, 0, 0, 660, 0), length=10), 100)[0])[3]
    check(w == WP['fist'], 'with the fist and the pistol, the dial to the right takes the fist '
          '(weapon %d)' % w)


def menu(e):
    # The title: Enter (or Escape) opens the menu, Enter takes New Game, the skill page opens
    # on the easiest skill
    for opener, name in ((ENTER, 'ENTER'), (MENU, 'MENU')):
        steps = ((20, 0, 0, 0, 0, 0, opener), (30, 0, 0, 0, 0, 0, ENTER),
                 (40, 0, -500, 0, 0, 0, 0), (50, 0, 0, 0, 0, 0, ENTER))
        d = e.run([], pulses(*steps), 120)[0]
        check(d['gamestate'] == '0' and d['gameskill'] == '1' and d['gamemap'] == '1',
              'title: %s, ENTER, one flick down, ENTER starts E1M1 at skill 1 '
              '(state %s, skill %s, map %s)' % (name, d['gamestate'], d['gameskill'],
                                                d['gamemap']))
    pad = pulses((20, 0, 0, 0, 0, 0, ENTER), (30, 0, 0, 0, 0, 0, ENTER), (55, 0, 0, 0, 0, 0, ENTER))
    pad += [(40, 0, -500, 0, 0, 0, 0), (50, 0, 0, 0, 0, 0, 0)]
    pad.sort(key=lambda s: s[0])
    d = e.run([], pad, 120)[0]
    check(d['gamestate'] == '0' and d['gameskill'] == '2',
          'the flick held for 10 tics repeats: skill 2 (state %s, skill %s)'
          % (d['gamestate'], d['gameskill']))
    pad = pulses((20, 0, 0, 0, 0, 0, ENTER), (30, 0, 0, 0, 0, 0, ENTER),
                 (40, 0, -500, 0, 0, 0, 0), (45, 0, 500, 0, 0, 0, 0), (50, 0, 0, 0, 0, 0, ENTER))
    d = e.run([], pad, 120)[0]
    check(d['gamestate'] == '0' and d['gameskill'] == '0',
          'a flick down and a flick up: skill 0 (state %s, skill %s)'
          % (d['gamestate'], d['gameskill']))


def demo(e):
    args = ['-playdemo', 'demo1']
    _, plain = e.run(args, tics=400)
    pad = [(0, 660, 660, 660, 0, 0, FIRE | USE | RUN), (100, -660, -400, -660, 0, 660, FIRE),
           (200, 0, 0, 0, 0, -660, NEXTW | MAP), (300, 0, 0, 0, 0, 0, PREVW)]
    _, padded = e.run(args, pad, 400)
    check(padded == plain, 'a pad used during DEMO1 leaves its hash stream as it is')


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--engine', default=os.path.join(ROOT, 'build', 'host', 'dmcdoom'))
    ap.add_argument('--nor', default=os.path.join(ROOT, 'build', 'nor', 'E1.img'))
    a = ap.parse_args()
    out = os.path.join(ROOT, 'build', 'padtest')
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    e = Engine(os.path.abspath(a.engine), os.path.abspath(a.nor), out)
    moves(e)
    weapons(e)
    menu(e)
    demo(e)
    if failures:
        sys.exit('%d failures, files in %s' % (len(failures), out))
    shutil.rmtree(out, ignore_errors=True)
    print('pad tests passed')


if __name__ == '__main__':
    main()
