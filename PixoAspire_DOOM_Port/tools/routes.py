#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Makes the stored routes of the lockstep exits workload (tests/lockstep/routes).

usage: routes.py [--oracle BIN] [--engine BIN] [--nor IMAGE] [--jobs N] [NAME...]

A route (tests/lockstep/play.h) plays E1Mm at skill s from its start with the
play aids of --refill: the first N tics of the script stream (make_cmds of
tests/lockstep/run_lockstep.py), then a stop in front of an exit line, facing
it, and BT_USE for 4 tics (a switch) or 8 tics of forward movement across it
(a walk-over line). NAME is e1mM_sS, or e1m3secret_sS for the secret exit of
E1M3. Without a name every route of the workload is made.

The exit lines come from the WAD by special (11 and 52, 51 for the secret
exit). The stop is 32 units in front of the middle of the line, facing it
(the formula of oracle/bot.c), in a sector with room for the player (found
by a BSP walk, a switch often sits in an alcove less deep). When the exit
does not fire within 8 tics of the stop on the oracle, the next place is
tried: 24, 40 and 48 units away, then a quarter of the line from either
end, then the back side of a walk-over line. N starts at 1,200 tics and is
shortened until the whole workload run (the route, then the intermission
and the next map, run_lockstep.EXIT_TAIL tics) ends on the next map and
the engine counts no object store event (--obj-stats, in the normal mode
and with --promote-all). The search runs once, when a route is made. The
workload only replays the stored file.
"""
import argparse
import concurrent.futures
import math
import os
import shutil
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
sys.path.insert(0, os.path.join(ROOT, 'tests', 'lockstep'))
sys.dont_write_bytecode = True
import run_lockstep as L  # noqa: E402
from tools.conv import maps, wadread  # noqa: E402

COMBAT = (1200, 900, 600, 400, 250, 150, 70, 0)
DISTANCES = (32, 24, 40, 48)
ALONG = (0.5, 0.25, 0.75)
EXIT_WITHIN = 8
USE_TICS, WALK_TICS = 4, 8
BT_USE = 2
WALK_FORWARD = 50
ROOM = 56
SPECIALS = {False: {11: 'use', 52: 'walk'}, True: {51: 'use'}}
WORK = os.path.join(ROOT, 'build', 'tmp-routes')
SCRIPT = []


class Map:
    def __init__(self, wad, m):
        lumps = wad.map_lumps('E1M%d' % m)
        self.vertexes = wadread.parse_vertexes(lumps['VERTEXES'])
        self.lines = wadread.parse_linedefs(lumps['LINEDEFS'])
        self.sides = wadread.parse_sidedefs(lumps['SIDEDEFS'])
        self.sectors = wadread.parse_sectors(lumps['SECTORS'])
        self.segs = wadread.parse_segs(lumps['SEGS'])
        self.ssectors = wadread.parse_ssectors(lumps['SSECTORS'])
        self.nodes = wadread.parse_nodes(lumps['NODES'])

    def sector_at(self, x, y):
        """R_PointInSubsector, then the sector of the subsector's first seg."""
        ss = maps.point_in_subsector(self.nodes, x << 16, y << 16)
        _, first = self.ssectors[ss]
        line, side = self.segs[first][3], self.segs[first][4]
        return self.sides[self.lines[line][5 + side]][5]

    def stops(self, specials):
        """The places in front of the exit lines with these specials, in the
        order they are tried: (line, how, x, y, angle)."""
        for i, (v1, v2, _, special, _, _, s1) in enumerate(self.lines):
            how = specials.get(special)
            if not how:
                continue
            (x1, y1), (x2, y2) = self.vertexes[v1], self.vertexes[v2]
            dx, dy = x2 - x1, y2 - y1
            n = math.hypot(dx, dy)
            # the front side is on the right of v1 to v2
            nx, ny = dy / n, -dx / n
            for sign in (1, -1) if how == 'walk' and s1 >= 0 else (1,):
                for t in ALONG:
                    lx, ly = x1 + t * dx, y1 + t * dy
                    for d in DISTANCES:
                        x = math.floor(lx + sign * d * nx)
                        y = math.floor(ly + sign * d * ny)
                        floor, ceiling = self.sectors[self.sector_at(x, y)][:2]
                        if ceiling - floor < ROOM:
                            continue
                        angle = round(math.atan2(ly - y, lx - x) / (2 * math.pi) * 2 ** 32)
                        yield i, how, x, y, angle & 0xFFFFFFFF


def combat_lines(m, n):
    """The first n tics of the script stream as cmd lines, equal tics merged."""
    out, start = [], 0
    for k in range(1, n + 1):
        if k == n or SCRIPT[k] != SCRIPT[start]:
            out.append('cmd %d %d %d %d %d %d %d' % ((m, start, k - start) + SCRIPT[start]))
            start = k
    return out


def route_text(name, n, stop):
    m, secret, sk = L.route_map(name)
    line, how, x, y, angle = stop
    text = ['# E1M%d at skill %d to its %sexit, made by tools/routes.py: the first %d tics of'
            % (m, sk, 'secret ' if secret else '', n),
            '# the script stream (run_lockstep.make_cmds), then line %d from (%d, %d)' % (line, x, y)]
    text += combat_lines(m, n)
    text.append('stop %d %d %d %d %d' % (m, n, x, y, angle))
    if how == 'use':
        text.append('cmd %d %d %d 0 0 0 %d' % (m, n, USE_TICS, BT_USE))
    else:
        text.append('cmd %d %d %d %d 0 0 0' % (m, n, WALK_TICS, WALK_FORWARD))
    return '\n'.join(text) + '\n'


def events(path):
    """Counted object store events and hazards of an --obj-stats file, None when missing."""
    if not os.path.exists(path):
        return None
    total = 0
    with open(path) as f:
        for line in f:
            w = line.split()
            if w and w[0] == 'level':
                d = dict(zip(w[2::2], w[3::2]))
                total += sum(int(d.get(k, '0')) for k in L.OBJ_EVENTS + L.OBJ_HAZARDS)
    return total


def attempt(name, n, stop, a, work):
    """Runs one candidate route. Returns (note, exit tic): note None when it works."""
    m, secret, _ = L.route_map(name)
    path = os.path.join(work, name + '.route')
    with open(path, 'w') as f:
        f.write(route_text(name, n, stop))
    game = L.route_args(name, path)
    states = os.path.join(work, 'states.txt')
    with open(os.path.join(work, 'oracle.log'), 'w') as log:
        subprocess.run([a.oracle, '-iwad', L.WAD, '--state-out', states] + game, stdout=log,
                       stderr=subprocess.STDOUT, cwd=work)
    note, tic = L.check_exit(L.read_states(states), m, secret)
    if note:
        return note, None
    if tic <= n or tic > n + EXIT_WITHIN:
        return 'the level ends at tic %d, the stop is at %d' % (tic, n), None
    for mode in ([], ['--promote-all']):
        obj = os.path.join(work, 'engine.obj')
        with open(os.path.join(work, 'engine.log'), 'w') as log:
            subprocess.run([a.engine, '--nor', a.nor, '--obj-stats', obj] + mode + game, stdout=log,
                           stderr=subprocess.STDOUT, cwd=work)
        ev = events(obj)
        if ev is None:
            return 'the engine wrote no statistics, see %s' % os.path.join(work, 'engine.log'), None
        if ev:
            return 'events', None
    return None, tic


def make(name, a, wad):
    """Searches the route of name and stores it. Returns a report line."""
    m, secret, _ = L.route_map(name)
    work = os.path.join(WORK, name)
    os.makedirs(work, exist_ok=True)
    stops = list(Map(wad, m).stops(SPECIALS[secret]))
    if not stops:
        return '%s: FAIL, no place in front of an exit line' % name
    for n in COMBAT:
        for stop in stops:
            note, tic = attempt(name, n, stop, a, work)
            if note == 'events':
                break
            if note:
                continue
            shutil.copy(os.path.join(work, name + '.route'), L.route_path(name))
            shutil.rmtree(work)
            return '%s: %d tics of the script, line %d from (%d, %d), level ends at tic %d' % (
                name, n, stop[0], stop[2], stop[3], tic)
    shutil.rmtree(work)
    return '%s: FAIL, no length of the script part gives an exit without events' % name


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('names', nargs='*')
    ap.add_argument('--oracle', default=os.path.join(ROOT, 'build', 'oracle', 'dg_oracle'))
    ap.add_argument('--engine', default=os.path.join(ROOT, 'build', 'host', 'dmcdoom_skip'))
    ap.add_argument('--nor', default=os.path.join(ROOT, 'build', 'nor', 'E1.img'))
    ap.add_argument('--jobs', type=int, default=L.JOBS)
    a = ap.parse_args()
    for b in (a.oracle, a.engine, a.nor):
        if not os.path.exists(b):
            sys.exit('%s missing, run make -j$(nproc) oracle build/host/dmcdoom_skip nor-images' % b)
    a.oracle, a.engine, a.nor = (os.path.abspath(p) for p in (a.oracle, a.engine, a.nor))
    names = a.names or L.route_names()
    for name in names:
        if name not in L.route_names():
            sys.exit('%s: not a route of the workload (%s)' % (name, ' '.join(L.route_names())))
    L.WAD = L.get_wad(None)
    with open(L.WAD, 'rb') as f:
        wad = wadread.Wad(f.read())
    os.makedirs(WORK, exist_ok=True)
    os.makedirs(L.ROUTES, exist_ok=True)
    path = os.path.join(WORK, 'script.cmds')
    L.make_cmds(path, COMBAT[0])
    with open(path, 'rb') as f:
        SCRIPT.extend(struct.iter_unpack('<bbhB3x', f.read()))
    ok = True
    with concurrent.futures.ThreadPoolExecutor(a.jobs) as ex:
        for line in ex.map(lambda n: make(n, a, wad), names):
            print(line)
            ok &= 'FAIL' not in line
    shutil.rmtree(WORK)
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
