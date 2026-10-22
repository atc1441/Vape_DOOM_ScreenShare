#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Saves and loads of the host engine against itself: the test saves of the meter.

A test save (--save-at TIC SLOT) puts the game into a slot of SAVE.DAT after the display of
tic TIC, where a save from the menu is taken. A test load (--load SLOT) puts it back over the
game the arguments set up, gametic and a demo's place included, before the first tic. The run
that loads must go on exactly as the run that saved.

Cases, each with the default object store and with a small one (SMALL: fewer pages and one
mover slot, so wakes and movers are deferred):
  e1m1-s4, e1m3-s4, e1m6-s3, e1m7-s4, e1m8-s4, e1m9-s1
           the map at the skill with the lockstep script stream (run_lockstep.make_cmds), on
           E1M1 the player is moved into a secret area at tic SECRET_TIC (a tour stop,
           tests/lockstep/play.h): the script finds no secret
  demo1    DEMO1 of the image
  demo2    build/fwdoom/DEMO2.LMP, the E1M1 script as a demo
  demo2-exit
           DEMO2.LMP with E1M1 ended at level time EXIT_AT (--exit-list), then E1M2: a load
           of a save on E1M2 sets up that map over the demo's E1M1 (G_DoNewGame)
each for TICS tics. Saves at every 100th tic of a level, and at the first tic probes
(--hash-dump every PROBE_STEP tics, the exact tic found tic by tic) show a mover in motion, a
missile in flight, the player dead, a mover started (a switch or a door), a secret found, the
level loaded again for a reborn player, the next map loaded, at the first counted event of
the store (--obj-stats), at tic 1 (the level loaded, its wipe taken by the first display, which
follows tic 1) and at the tic before the first one after tic 1 that loads a level again or ends
it (--stats-out), so that the display after the first tic of the load has a wipe.
  run P    the case without saves
  runs A   up to 4 saves each (--nor-out): the hash stream must equal P's, and the data drive
           of the image must hold SAVE.DAT of 65,536 B (fsck.fat, mdir) with both FATs equal
  runs B   one per save, on its image A with --load and the exits A has not taken yet: the
           first line is tic TIC + 1 and the lines equal A's from there on
With --ref the stream of P must also equal the one of that engine (the frame model against
dmcdoom_skip). --engine-arg gives every run of the engine an argument (--verify-model of the
frame model: the creation order under the tic scratch, as the verification pass of the meter
keeps it). Every kind of save tic must turn up in some case.

Without --case also: the route of E1M1 at skill 4 (the exits workload of run_lockstep.py)
leaves the level by its exit, a save at the end of the tic before that one is taken and a save
at the end of that tic is refused (the end of the level waits for the next tic).

usage: savetest.py [--engine BIN] [--engine-arg ARG]... [--ref BIN] [--nor E1.img]
                   [--case NAME]... [--jobs N] [--tag NAME]
Outputs go to build/savetest/NAME (default: default), removed when a case passes.
"""
import argparse
import concurrent.futures
import os
import shutil
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.join(ROOT, 'tests', 'lockstep'))
import run_lockstep as LS  # noqa: E402

TICS = 2000
SWEEP = 100
PROBE_STEP = 20
SMALL = ['--obj-pages', '16', '--obj-movers', '1']
NO_VIEW = ['--render-every', '1000000']
MF_MISSILE = 0x10000
PST_DEAD = 1
MAIN = 0x100000
KINDS = ('sweep', 'level', 'mover', 'missile', 'dead', 'switch', 'secret', 'reborn', 'nextmap',
         'event', 'wipe')
# A point of E1M1's secret sector 86 (Freedoom 0.13.0), in its subsector 45
SECRET_TIC, SECRET_AT = 300, (870, 1542)
EXIT_AT = 350


def cases(cmds, out_dir):
    tour = os.path.join(out_dir, 'secret.tour')
    with open(tour, 'w') as f:
        f.write('%d %d %d 0\n' % ((SECRET_TIC,) + SECRET_AT))
    out = {}
    for m, sk in ((1, 4), (3, 4), (6, 3), (7, 4), (8, 4), (9, 1)):
        out['e1m%d-s%d' % (m, sk)] = ['-warp', '1', str(m), '-skill', str(sk), '--cmds', cmds]
    out['e1m1-s4'] += ['--tour', tour]
    out['demo1'] = ['-playdemo', 'demo1']
    out['demo2'] = ['-playdemo', os.path.join(ROOT, 'build', 'fwdoom', 'DEMO2.LMP')]
    out['demo2-exit'] = out['demo2'] + ['--exit-list', str(EXIT_AT)]
    return out


def run(cfg, args, d, log, engine=None, nor=None):
    extra = [] if engine else cfg.engine_args
    with open(os.path.join(d, log), 'w') as f:
        return subprocess.run([engine or cfg.engine, '--nor', nor or cfg.nor] + extra + args,
                              stdout=f, stderr=subprocess.STDOUT, cwd=d).returncode


def stream(path):
    return LS.read_stream(path)


def probe(cfg, game, d, t):
    """The state at the end of tic t: (leveltime, playerstate, secretcount, sectors with a
    mover, missiles, gamemap), None outside a level."""
    dump = os.path.join(d, 'probe.%d' % t)
    run(cfg, game + ['--tics', str(t + 1), '--hash-dump', str(t), dump] + NO_VIEW, d, 'probe.log')
    v = {}
    movers = missiles = 0
    if not os.path.exists(dump):
        return None
    with open(dump) as f:
        for line in f:
            k, x = line.split()
            if k.startswith('sec.mover['):
                movers += x != '0'
            elif k.startswith('mo.flags['):
                missiles += bool(int(x) & MF_MISSILE)
            else:
                v[k] = x
    os.remove(dump)
    return (int(v['leveltime']), int(v['pl.playerstate[0]']), int(v['pl.secretcount[0]']),
            movers, missiles, int(v['gamemap']))


def targets(cfg, game, d, plain):
    """kind -> the save tic of each kind this case has."""
    found = {'level': 1}
    probes = {}
    for t in range(PROBE_STEP // 2, TICS - 1, PROBE_STEP):
        probes[t] = probe(cfg, game, d, t)
    first = {'mover': lambda s: s[3] > 0, 'missile': lambda s: s[4] > 0,
             'dead': lambda s: s[1] == PST_DEAD}
    rise = {'switch': lambda a, b: b[3] > a[3], 'secret': lambda a, b: b[2] > a[2],
            'reborn': lambda a, b: b[0] < a[0] and b[5] == a[5],
            'nextmap': lambda a, b: b[5] != a[5]}
    ts = sorted(t for t in probes if probes[t])
    for kind, fn in first.items():
        hit = [t for t in ts if fn(probes[t])]
        if hit:
            found[kind] = hit[0]
    for kind, fn in rise.items():
        for a, b in zip(ts, ts[1:]):
            if fn(probes[a], probes[b]):
                # the tic it happened in
                prev = probes[a]
                for t in range(a + 1, b + 1):
                    s = probes[t] if t in probes else probe(cfg, game, d, t)
                    if s and fn(prev, s):
                        found[kind] = t
                        break
                    prev = s or prev
                break
    for lv in plain:
        if int(lv.get('first_event', '-1')) >= 0:
            found['event'] = int(lv['first_event'])
            break
    return found


def tic_flags(path):
    """tic -> flag of a run's --stats-out: 1 for a tic that loaded a level, 2 for one that ended
    outside a level."""
    with open(path) as f:
        return {int(w[1]): int(w[5]) for w in (line.split() for line in f)
                if len(w) == 6 and w[0] == 'T'}


def level_tics(path):
    """The tics of a run that end in a level."""
    return {t for t, f in tic_flags(path).items() if f != 2}


def exits_taken(level, t):
    """The level exits of a run by the end of tic t: its tics up to t that end outside a level
    after one that ended in it."""
    return sum(1 for x in range(1, t + 1) if x - 1 in level and x not in level)


def after_exits(game, k):
    """The arguments of game for a run that starts after its first k forced exits."""
    if '--exit-list' not in game:
        return game
    i = game.index('--exit-list')
    rest = game[i + 1].split(',')[k:]
    return game[:i] + (['--exit-list', ','.join(rest)] if rest else []) + game[i + 2:]


def fat_check(img):
    """fsck.fat and mdir on the data drive of a NOR image: (ok, what is wrong)."""
    with open(img, 'rb') as f:
        vol = f.read()[MAIN:]
    path = img + '.vol'
    with open(path, 'wb') as f:
        f.write(vol)
    env = dict(os.environ, MTOOLS_SKIP_CHECK='1')
    fsck = subprocess.run(['fsck.fat', '-n', path], capture_output=True, text=True).returncode == 0
    r = subprocess.run(['mdir', '-b', '-i', path, '::/SAVE.DAT'], capture_output=True, text=True,
                       env=env)
    os.remove(path)
    bps, spc, rsv, nfat = struct.unpack_from('<HBHB', vol, 11)
    fatsz = struct.unpack_from('<H', vol, 22)[0] * bps
    fats = [vol[(rsv * bps) + k * fatsz:(rsv * bps) + (k + 1) * fatsz] for k in range(nfat)]
    size = None
    for i in range(512):
        e = vol[(rsv * bps) + nfat * fatsz + 32 * i:][:32]
        if e[:11] == b'SAVE    DAT':
            size = struct.unpack_from('<I', e, 28)[0]
    why = []
    if not fsck:
        why.append('fsck.fat finds errors')
    if r.returncode or size != 65536:
        why.append('SAVE.DAT %s' % size)
    if any(f != fats[0] for f in fats):
        why.append('the FATs differ')
    return not why, ', '.join(why)


def job(cfg, name, game, store):
    """One case with one object store. Returns (ok, report lines, kinds found)."""
    tag = '%s%s' % (name, '-small' if store else '')
    d = os.path.join(cfg.out, tag)
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    game = game + store
    lines, ok = [], True
    pt = os.path.join(d, 'plain.txt')
    obj = os.path.join(d, 'plain.obj')
    st = os.path.join(d, 'plain.stats')
    run(cfg, game + ['--tics', str(TICS), '--hash-out', pt, '--obj-stats', obj, '--stats-out', st],
        d, 'plain.log')
    plain = stream(pt)
    if len(plain) < TICS // 2:
        return False, ['[%s] FAIL: the run without saves has %d tics, see %s' % (
            tag, len(plain), d)], set()
    if cfg.ref:
        rt = os.path.join(d, 'ref.txt')
        run(cfg, game + ['--tics', str(TICS), '--hash-out', rt], d, 'ref.log', cfg.ref)
        if stream(rt) != plain:
            ok = False
            lines.append('  FAIL: the stream differs from %s' % os.path.basename(cfg.ref))
    levels = []
    with open(obj) as f:
        for line in f:
            w = line.split()
            if w and w[0] == 'level':
                levels.append(dict(zip(w[2::2], w[3::2])))
    found = targets(cfg, game, d, levels)
    last = int(plain[-1][0])
    flags = tic_flags(st)
    level = {t for t, f in flags.items() if f != 2}
    wipe = [t for t in sorted(flags) if t > 1 and t - 1 in level and flags[t]]
    if wipe:
        found['wipe'] = wipe[0] - 1
    saves = {}
    for kind, t in found.items():
        if t + 1 < last and t in level:
            saves.setdefault(t, kind)
    kinds = {'sweep'} | set(saves.values())
    for t in range(SWEEP, last - 1, SWEEP):
        if t in level:
            saves.setdefault(t, 'sweep')
    tics = sorted(saves)
    good = bad = 0
    for i in range(0, len(tics), 4):
        group = tics[i:i + 4]
        img, at = os.path.join(d, 'a%d.img' % i), os.path.join(d, 'a%d.txt' % i)
        args = list(game) + ['--tics', str(TICS), '--nor-out', img, '--hash-out', at]
        for k, t in enumerate(group):
            args += ['--save-at', str(t), str(k)]
        rc = run(cfg, args, d, 'a%d.log' % i)
        a = stream(at)
        if rc or a != plain:
            ok = False
            lines.append('  FAIL: run A with saves at %s: exit %d, %s' % (
                group, rc, 'stream differs from the run without saves' if a != plain else
                'same stream'))
            continue
        fat_ok, why = fat_check(img)
        if not fat_ok:
            ok = False
            lines.append('  FAIL: image of the saves at %s: %s' % (group, why))
        for k, t in enumerate(group):
            bt = os.path.join(d, 'b%d.txt' % t)
            rc = run(cfg, after_exits(game, exits_taken(level, t)) + [
                '--tics', str(TICS), '--load', str(k), '--hash-out', bt], d, 'b%d.log' % t, nor=img)
            b = stream(bt)
            want = [x for x in a if int(x[0]) > t]
            if rc == 0 and b and int(b[0][0]) == t + 1 and b == want:
                good += 1
                os.remove(bt)
                continue
            bad += 1
            ok = False
            diff = LS.first_diff(want, b)
            lines.append('  FAIL: load of the %s save at tic %d: exit %d, first tic %s, %s' % (
                saves[t], t, rc, b[0][0] if b else None,
                'differs from tic %s' % (b[diff][0] if diff is not None and diff < len(b)
                                         else want[diff][0] if diff is not None and diff < len(want)
                                         else '-')))
        if ok:
            os.remove(img)
    lines.insert(0, '[%s] %d saves loaded and identical%s, at %s' % (
        tag, good, ', %d FAILED' % bad if bad else '',
        ', '.join('%s %d' % (saves[t], t) for t in tics if saves[t] != 'sweep') or 'the sweep only'))
    if ok:
        shutil.rmtree(d, ignore_errors=True)
    else:
        lines.append('  files in %s' % d)
    return ok, lines, kinds


def exit_save(cfg):
    """The saves around the tic in which the route of E1M1 leaves the level by its exit.
    Returns (ok, report lines)."""
    d = os.path.join(cfg.out, 'exit')
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    game = LS.route_args('e1m1_s4') + NO_VIEW
    st = os.path.join(d, 'plain.stats')
    run(cfg, game + ['--stats-out', st], d, 'plain.log')
    level = level_tics(st)
    n = int(game[game.index('--tics') + 1])
    ends = sorted(x for x in level if x + 1 < n and x + 1 not in level)
    if not ends:
        return False, ['[exit] FAIL: E1M1 does not end, files in %s' % d]
    t = ends[0]
    rc = run(cfg, game + ['--save-at', str(t - 1), '0', '--save-at', str(t), '1'], d, 'save.log')
    with open(os.path.join(d, 'save.log')) as f:
        log = f.read()
    ok = (rc != 0 and '--save-at %d 1: not in a level' % t in log
          and '--save-at %d 0' % (t - 1) not in log)
    if ok:
        shutil.rmtree(d, ignore_errors=True)
    return ok, ['[exit] E1M1 ends in tic %d: the save at tic %d is taken, the save at tic %d %s' % (
        t, t - 1, t, 'is refused' if ok else 'FAILED (exit %d), files in %s' % (rc, d))]


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--engine', default=os.path.join(ROOT, 'build', 'host', 'dmcdoom_skip'))
    ap.add_argument('--engine-arg', action='append', default=[])
    ap.add_argument('--ref')
    ap.add_argument('--nor', default=os.path.join(ROOT, 'build', 'nor', 'E1.img'))
    ap.add_argument('--case', action='append', default=[])
    ap.add_argument('--jobs', type=int, default=LS.JOBS)
    ap.add_argument('--tag', default='default')
    a = ap.parse_args()
    out = os.path.join(ROOT, 'build', 'savetest', a.tag)
    os.makedirs(out, exist_ok=True)
    cmds = os.path.join(out, 'script.cmds')
    LS.make_cmds(cmds, TICS)
    cfg = argparse.Namespace(engine=os.path.abspath(a.engine), nor=os.path.abspath(a.nor), out=out,
                             ref=os.path.abspath(a.ref) if a.ref else None,
                             engine_args=a.engine_arg)
    all_cases = cases(cmds, out)
    names = a.case or list(all_cases)
    unknown = [n for n in names if n not in all_cases]
    if unknown:
        sys.exit('unknown case %s, the cases are %s' % (unknown, ', '.join(all_cases)))
    jobs = [(n, all_cases[n], s) for n in names for s in ([], SMALL)]
    ok, kinds = True, set()
    with concurrent.futures.ThreadPoolExecutor(a.jobs) as ex:
        for good, lines, k in ex.map(lambda j: job(cfg, *j), jobs):
            ok &= good
            kinds |= k
            print('\n'.join(lines))
    if not a.case:
        good, lines = exit_save(cfg)
        ok &= good
        print('\n'.join(lines))
        missing = [k for k in KINDS if k not in kinds]
        print('kinds of saves: %s%s' % (', '.join(k for k in KINDS if k in kinds),
                                         ', FAIL: none of %s' % missing if missing else ''))
        ok &= not missing
    print('PASS' if ok else 'FAIL')
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
