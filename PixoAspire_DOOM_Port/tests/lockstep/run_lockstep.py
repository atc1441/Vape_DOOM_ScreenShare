#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Lockstep test: the engine against the unmodified doomgeneric oracle.

Both programs write one hash line per tic (tests/lockstep/hash.h). The
streams must be identical, and two runs of the oracle must be identical.
The oracle reads freedoom1.wad, the engine only the NOR image built from a
pack (tests/host/mknor.py).


Workloads:
  demo1    Freedoom's DEMO1 (E1M6, skill 3) played to its end marker
  script   E1M1 at skill 4 driven by a deterministic ticcmd stream
           (fire, move, strafe, turn, use, weapon changes) for 3200 tics,
           then a forced level exit, the intermission and E1M2. The oracle
           state at the last E1M1 tic must show kills and player damage.
  attract  no game arguments: title page, DEMO1 in the attract loop and the
           credit page, up to where the oracle starts DEMO2 (a 3-player
           demo the engine does not play)
  maps     E1M2 to E1M9 at skill 4 with the same ticcmd stream, 2500 tics
           each with a level exit forced at tic 2000
  e1m1pack the script workload up to the E1M1 exit on the E1M1 pack image
           (--nor-e1m1), the only map of that pack. Then, engine only, the
           script with the forced exit on that image: the intermission must
           match the oracle and the game must end in the title loop, because
           the map table of the pack has no E1M2 (e1m1pack-end)
  exits    every E1 map at skills 1 and 4 played from its start to its exit
           with the play aids of --refill and a stored route
           (tests/lockstep/routes, made by tools/routes.py): a part of the
           script stream, then a stop in front of the exit, use or a walk
           across it, then 600 tics in which the route fires every 8 tics
           outside a level. E1M3 also through its secret exit, 20 runs in
           all. The level must end in an intermission (the finale after
           E1M8) and the run on the next map (E1M9 after the secret exit)
  episode  the routes chained at skills 4 and 1: E1M1, E1M2, E1M3 through
           the secret exit, E1M9, E1M4 to E1M8, then 2000 tics of the
           finale, which switch to the art stage
  exitlist the E1M1 script as a v1.9 demo (tools/mklmp.py) with level exits
           forced by level time (--exit-list): E1M1 350, E1M2 350, E1M3 300
           secret, E1M9 300, E1M4 350, E1M5 350, E1M6 400, E1M7 400, E1M8
           200. The demo ends 100 tics after the finale's switch to the art
           stage, found by a first oracle run (the multi-map demo of the
           firmware test)
  menu     the keys of MENU_STEPS (--keys): on the title the menu, New Game,
           the Nightmare prompt answered n, then I'm Too Young To Die (the
           oracle also passes its episode page and moves its skill cursor,
           both select on the same tic). In E1M1 the menu opened and closed
           (the game pauses under it), Options and back, the End Game prompt
           and the quit prompt (F10) answered n, the empty Load Game page,
           then a save into slot 0 from Save Game (the oracle types a
           character as its description, both confirm on the same tic) and
           the Load Game page with it. The new game must start on the same
           tic and the level time stand still under the menu. Both start
           without saves: the engine makes SAVE.DAT in its NOR image in
           memory, the oracle has an empty save directory
  corpus   not in "all": every E1 map at skills 4, 3 and 2, two bot seeds,
           one life of 8000 tics each. The oracle's corpus bot
           (oracle/bot.c) plays with health and armor refilled, every E1
           weapon and no level exit (tests/lockstep/play.h): rallies
           through the map wake the monsters, fights hunt them, lost souls
           first, a tour of item spots, trigger lines and places beside
           monsters reaches them. Its ticcmds and tour are replayed by a
           second oracle run, which must give the same stream, and by the
           engines. A run whose counted events show that the bot woke more
           monsters than the plan holds is compared up to the first event
           and reported as beyond the plan. Every life must last the whole
           run, and the corpus must remove a lost soul that is still
           referenced (a zombie) and fire A_BossDeath. With --obj-stats the
           peaks are compared with the RAM plans of the image
  demos    not in "all": DEMO2 to DEMO4 of the WAD that are single player
           demos, played to their end marker as .lmp files on both sides
           (Freedoom has none, the shareware DOOM1.WAD has DEMO2 and DEMO3)


usage: run_lockstep.py [--zip FILE | --iwad FILE] [--workload NAME|all]
                       [--tics N] [--engine BIN]... [--oracle BIN] [--asan]
                       [--tag NAME] [--nor IMAGE] [--nor-e1m1 IMAGE]
                       [--engine-arg ARG] [--parity] [--obj-stats]
With --obj-stats every engine run writes the object store statistics
(--obj-stats of the engine): every counted event (port/obj/things.h) must be 0.
The exits, episode and exitlist workloads always check them. An oracle run
with key input gets an emptied directory of its own as working directory
and HOME, where it keeps its configuration and saves.
With --parity the engine is the accessor-parity build: it also gets the WAD
and the names.json of the pack (build/packs/<image name>/names.json).
Outputs go to build/lockstep/NAME (default: default).
The Freedoom zip comes from --zip, $FREEDOOM_ZIP, dl/ or the official URL.
--iwad gives the oracle another IWAD instead of freedoom1.wad, for the NOR images of a
pack converted from it (make IWAD=FILE BUILD=DIR, give --nor and --nor-e1m1 too).
"""
import argparse
import concurrent.futures
import hashlib
import os
import shutil
import struct
import subprocess
import sys
import urllib.request
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
OUT = os.path.join(ROOT, 'build', 'lockstep')
ZIP_NAME = 'freedoom-0.13.0.zip'
ZIP_URL = 'https://github.com/freedoom/freedoom/releases/download/v0.13.0/' + ZIP_NAME
ZIP_SHA256 = '3f9b264f3e3ce503b4fb7f6bdcb1f419d93c7b546f4df3e874dd878db9688f59'
WAD_MEMBER = 'freedoom-0.13.0/freedoom1.wad'
WAD = NOR = None
ENGINE_ARGS = []
PARITY = False

# ticcmd button bits (d_event.h)
BT_ATTACK, BT_USE, BT_CHANGE, BT_WEAPONSHIFT = 1, 2, 4, 3

E1M1_TICS = 3200
ATTRACT_TICS = 7490
MAP_TICS, MAP_EXIT = 2500, 2000
WORKLOADS = ('demo1', 'script', 'attract', 'maps', 'e1m1pack', 'exits', 'episode', 'exitlist',
             'menu')
ROUTES = os.path.join(ROOT, 'tests', 'lockstep', 'routes')
ROUTE_SKILLS = (1, 4)
# tics after the end of a route: the intermission and the next map
EXIT_TAIL = 600
EPISODE = ('e1m1', 'e1m2', 'e1m3secret', 'e1m9', 'e1m4', 'e1m5', 'e1m6', 'e1m7', 'e1m8')
EPISODE_SKILLS = (4, 1)
EPISODE_FINALE = 2000
EXITLIST = '350,350,300s,300,350,350,400,400,200'
EXITLIST_PROBE_TICS = 9000
EXITLIST_AFTER_ART = 100
MKLMP = os.path.join(ROOT, 'tools', 'mklmp.py')
# The menu workload: (tic, key, side, page). The key goes down at the tic and up at the next
# one, on both runners or on one side ('engine', 'oracle'). page is what both show from the
# next tic on, for the menu screens of tests/host/screentest.py: 'main', 'new' (the skill
# page), 'options', 'prompt', 'load' and 'save' (every slot empty), or what only the engine
# shows ('saved', Load Game with the save), or None (no menu, or the two differ).
MENU_STEPS = (
    (20, 'KEY_ESCAPE', 'both', 'main'),       # over the title page
    (30, 'KEY_ENTER', 'both', None),          # New Game, the oracle's episode page
    (35, 'KEY_ENTER', 'oracle', None),        # episode 1, the oracle's skill cursor on row 2
    (40, 'KEY_UPARROW', 'oracle', None),
    (45, 'KEY_UPARROW', 'oracle', 'new'),     # both on row 0
    (55, 'KEY_UPARROW', 'both', 'new'),       # Nightmare
    (65, 'KEY_ENTER', 'both', 'prompt'),
    (80, 'n', 'both', None),                  # the menu closes
    (90, 'KEY_ESCAPE', 'both', 'main'),
    (100, 'KEY_ENTER', 'both', None),
    (105, 'KEY_ENTER', 'oracle', 'new'),      # both on Nightmare again
    (115, 'KEY_DOWNARROW', 'both', 'new'),
    (125, 'KEY_ENTER', 'both', None),         # I'm Too Young To Die: E1M1 set up in tic 125
    (200, 'KEY_ESCAPE', 'both', 'main'),      # the game pauses
    (215, 'KEY_ESCAPE', 'both', None),
    (230, 'KEY_ESCAPE', 'both', 'main'),
    (240, 'KEY_DOWNARROW', 'both', 'main'),
    (250, 'KEY_ENTER', 'both', 'options'),
    (265, 'KEY_BACKSPACE', 'both', 'main'),
    (275, 'KEY_ENTER', 'both', 'options'),
    (285, 'KEY_ENTER', 'both', 'prompt'),     # End Game
    (300, 'n', 'both', None),
    (315, 'KEY_F10', 'both', 'prompt'),       # quit
    (330, 'n', 'both', None),
    (340, 'KEY_ESCAPE', 'both', 'main'),      # on Options
    (345, 'KEY_DOWNARROW', 'both', 'main'),
    (350, 'KEY_ENTER', 'both', 'load'),       # Load Game
    (360, 'KEY_BACKSPACE', 'both', 'main'),
    (365, 'KEY_DOWNARROW', 'both', 'main'),
    (370, 'KEY_ENTER', 'both', 'save'),       # Save Game
    (380, 'KEY_ENTER', 'oracle', None),       # the oracle's description of slot 0
    (385, 'y', 'oracle', None),
    (390, 'KEY_ENTER', 'both', None),         # the save of slot 0 in tic 391
    (400, 'KEY_ESCAPE', 'both', 'main'),      # on Save Game
    (405, 'KEY_UPARROW', 'both', 'main'),
    (410, 'KEY_ENTER', 'both', 'saved'),      # Load Game with the save
    (420, 'KEY_ESCAPE', 'both', None),
)
MENU_TICS = 440
MENU_GAME = 125
GS_LEVEL, GS_INTERMISSION, GS_FINALE = 0, 1, 2
# M_Random calls of a screen wipe (f_wipe.c wipe_initMelt), rndindex is a byte
WIPE_RND = 320 % 256
JOBS = max(1, min(8, (os.cpu_count() or 2) // 2))
CORPUS_TICS = 8000
CORPUS_SKILLS = (4, 3, 2)
CORPUS_SEEDS = (1, 2)
CORPUS_PLAY = ['--refill', '--no-exit']
OBJ_STATS = False
OBJ_EVENTS = ('ev_wake', 'ev_transient', 'ev_mover', 'ev_evicted', 'ev_overrun')
# traces that vanilla's corpses at rest would take past 128 intercepts
# (the engine's corpse records are outside the blockmap)
OBJ_HAZARDS = ('corpse_trace',)
obj_levels = []


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(1 << 20), b''):
            h.update(chunk)
    return h.hexdigest()


def given_zip(arg):
    """The zip named by --zip or $FREEDOOM_ZIP, which must exist, or None."""
    path = arg or os.environ.get('FREEDOOM_ZIP')
    if path and not os.path.isfile(path):
        sys.exit('%s: no such file' % path)
    return path


def find_zip(arg):
    path = given_zip(arg)
    if not path:
        path = os.path.join(ROOT, 'dl', ZIP_NAME)
        if not os.path.exists(path):
            os.makedirs(os.path.dirname(path), exist_ok=True)
            print('downloading', ZIP_URL)
            tmp = path + '.part'
            try:
                urllib.request.urlretrieve(ZIP_URL, tmp)
                digest = sha256_file(tmp)
                if digest != ZIP_SHA256:
                    sys.exit('download: SHA-256 %s, expected %s' % (digest, ZIP_SHA256))
                os.replace(tmp, path)
            finally:
                if os.path.exists(tmp):
                    os.remove(tmp)
    digest = sha256_file(path)
    if digest != ZIP_SHA256:
        sys.exit('%s: SHA-256 %s, expected %s' % (path, digest, ZIP_SHA256))
    return path


def get_wad(zip_arg):
    given_zip(zip_arg)
    wad = os.path.join(OUT, 'freedoom1.wad')
    stamp = wad + '.sha256'
    if os.path.exists(wad) and os.path.exists(stamp):
        with open(stamp) as f:
            if f.read().strip() == sha256_file(wad):
                return wad
    with zipfile.ZipFile(find_zip(zip_arg)) as z:
        data = z.read(WAD_MEMBER)
    tmp = '%s.%d' % (wad, os.getpid())
    with open(tmp, 'wb') as f:
        f.write(data)
    os.replace(tmp, wad)
    with open(tmp, 'w') as f:
        f.write(hashlib.sha256(data).hexdigest() + '\n')
    os.replace(tmp, stamp)
    return wad


def wad_lump(path, name):
    """The last lump of that name in the WAD at path, or None."""
    with open(path, 'rb') as f:
        data = f.read()
    _, n, off = struct.unpack_from('<4sii', data, 0)
    found = None
    for i in range(n):
        pos, size, lname = struct.unpack_from('<ii8s', data, off + 16 * i)
        if lname.split(b'\0', 1)[0].upper() == name.encode():
            found = data[pos:pos + size]
    return found


def make_cmds(path, tics, seed=0x1d0f, fire_one_in=3):
    """Deterministic input: segments of 8..40 tics with one behaviour each."""
    state = seed

    def rnd():
        nonlocal state
        state ^= (state << 13) & 0xffffffff
        state ^= state >> 17
        state ^= (state << 5) & 0xffffffff
        return state

    out = bytearray()
    t = 0
    while t < tics:
        n = 8 + rnd() % 33
        fwd = (0, 25, 50, 50, -25, -50)[rnd() % 6]
        side = (0, 0, 24, -24, 40, -40)[rnd() % 6]
        turn = (0, 0, 320, -320, 640, -640, 1280, -1280)[rnd() % 8] + (rnd() % 64) - 32
        fire = rnd() % fire_one_in == 0
        use_every = (0, 0, 7)[rnd() % 3]
        weapon = rnd() % 4 if rnd() % 5 == 0 else -1
        for k in range(min(n, tics - t)):
            buttons = BT_ATTACK if fire else 0
            if use_every and k % use_every == 0:
                buttons |= BT_USE
            if weapon >= 0 and k == 0:
                buttons |= BT_CHANGE | (weapon << BT_WEAPONSHIFT)
            out += struct.pack('<bbhB3x', fwd, side, turn, buttons)
        t += n
    with open(path, 'wb') as f:
        f.write(out)


def run(binary, args, hash_out, log, dump=None, stats=None, home=None, states=None):
    """home: an emptied directory of its own as the working directory and
    HOME (the oracle keeps its configuration and saves there). states: the
    oracle's per-tic game state (--state-out)."""
    cmd = [binary, '--hash-out', hash_out] + args
    if stats:
        cmd += ['--obj-stats', stats]
    if dump:
        cmd += ['--hash-dump', str(dump[0]), dump[1]]
    if states:
        cmd += ['--state-out', states]
    env = None
    if home:
        shutil.rmtree(home, ignore_errors=True)
        os.makedirs(home)
        env = dict(os.environ, HOME=home)
    with open(log, 'w') as f:
        r = subprocess.run(cmd, stdout=f, stderr=subprocess.STDOUT, cwd=home or OUT, env=env)
    if r.returncode != 0:
        print('  %s exited with %d, see %s' % (os.path.basename(binary), r.returncode, log))
    return r.returncode


def read_stream(path):
    if not os.path.exists(path):
        return []
    with open(path) as f:
        return [line.split() for line in f if line.strip()]


def read_dump(path):
    """First value of every name in a --hash-dump file."""
    d = {}
    with open(path) as f:
        for line in f:
            k, v = line.split()
            d.setdefault(k, v)
    return d


def first_diff(a, b):
    for i in range(min(len(a), len(b))):
        if a[i] != b[i]:
            return i
    return None if len(a) == len(b) else min(len(a), len(b))


def explain(name, oracle, engine, oargs, eargs, tic, say=print):
    """Dump both states at the first differing tic and print the fields."""
    da = os.path.join(OUT, name + '.oracle.dump')
    db = os.path.join(OUT, name + '.engine.dump')
    run(oracle, oargs, os.devnull, os.path.join(OUT, name + '.oracle.dump.log'), (tic, da),
        home=oracle_home(name + '.oracle.dump', oargs))
    run(engine, eargs, os.devnull, os.path.join(OUT, name + '.engine.dump.log'), (tic, db))
    if not (os.path.exists(da) and os.path.exists(db)):
        return
    with open(da) as f:
        la = f.read().splitlines()
    with open(db) as f:
        lb = f.read().splitlines()
    shown = 0
    for x, y in zip(la, lb):
        if x != y and shown < 20:
            say('    oracle %-32s engine %s' % (x, y))
            shown += 1
    if len(la) != len(lb):
        say('    dump lengths differ: %d vs %d values' % (len(la), len(lb)))


def engine_args(nor, args):
    eargs = ['--nor', nor] + ENGINE_ARGS + args
    if PARITY:
        pack = os.path.splitext(os.path.basename(nor))[0]
        eargs += ['-iwad', WAD, '-parnames', os.path.join(ROOT, 'build', 'packs', pack, 'names.json')]
    return eargs


def read_obj_stats(path, name, tag, say=print):
    """Object store lines of an engine run: checks the events, keeps the levels."""
    ok = True
    if not os.path.exists(path):
        say('  FAIL: %s wrote no object statistics' % tag)
        return False
    with open(path) as f:
        for line in f:
            w = line.split()
            if not w or w[0] != 'level':
                continue
            d = {'level': w[1]}
            for k, v in zip(w[2::2], w[3::2]):
                d[k] = v
            d['workload'] = name
            obj_levels.append(d)
            bad = [k for k in OBJ_EVENTS + OBJ_HAZARDS if d.get(k, '0') != '0']
            if bad:
                say('  FAIL: %s %s skill %s: counted events %s' % (
                    tag, d['level'], d['skill'], ', '.join('%s %s' % (k, d[k]) for k in bad)))
                ok = False
    return ok


def oracle_home(run_name, oargs):
    """An oracle run that may save (it takes key input) gets a save directory of its own."""
    return os.path.join(OUT, run_name + '.home') if '--keys' in oargs else None


def workload(name, args, oracle, engines, dump=None, nor=None, states=None, events=False, say=print,
             keys=None):
    """args: game arguments. The oracle gets the WAD, the engines the NOR image.
    states: the first oracle run writes its game states there. events: the
    engines always write object statistics and a counted event fails. keys:
    the key files of the oracle and of the engines, when they differ."""
    oargs = ['-iwad', WAD] + args
    eargs = engine_args(nor or NOR, args)
    if keys:
        oargs += ['--keys', keys['oracle']]
        eargs += ['--keys', keys['engine']]
    say('[%s] %s' % (name, ' '.join(args)))
    ha = os.path.join(OUT, name + '.oracle1.txt')
    hb = os.path.join(OUT, name + '.oracle2.txt')
    ok = True
    rc = run(oracle, oargs, ha, os.path.join(OUT, name + '.oracle1.log'), dump,
             home=oracle_home(name + '.oracle1', oargs), states=states)
    rc |= run(oracle, oargs, hb, os.path.join(OUT, name + '.oracle2.log'),
              home=oracle_home(name + '.oracle2', oargs))
    sa, sb = read_stream(ha), read_stream(hb)
    if rc or not sa:
        say('  FAIL: oracle run failed')
        return False
    d = first_diff(sa, sb)
    if d is None:
        say('  oracle twice: identical, %d tics' % len(sa))
    else:
        say('  FAIL: oracle runs differ at line %d' % d)
        ok = False
    for engine in engines:
        tag = os.path.basename(engine)
        he = os.path.join(OUT, '%s.%s.txt' % (name, tag))
        stats = os.path.join(OUT, '%s.%s.obj' % (name, tag)) if OBJ_STATS or events else None
        rc = run(engine, eargs, he, os.path.join(OUT, '%s.%s.log' % (name, tag)), stats=stats)
        se = read_stream(he)
        d = first_diff(sa, se)
        if stats:
            ok &= read_obj_stats(stats, name, tag, say)
        if rc == 0 and d is None:
            say('  %s: identical every tic, %d tics, last %s' % (tag, len(se), ' '.join(se[-1])))
            continue
        ok = False
        if d is None:
            say('  FAIL: %s exited with %d' % (tag, rc))
            continue
        tic = int(sa[d][0]) if d < len(sa) else int(se[d][0])
        say('  FAIL: %s differs from tic %d (oracle %d lines, engine %d lines)'
            % (tag, tic, len(sa), len(se)))
        if d < len(sa) and d < len(se):
            explain(name, oracle, engine, oargs, eargs, tic, say)
    return ok


def pack_end(oracle, engines, cmds, nor, tics):
    """E1M1 exit on the E1M1 pack: same stream as the oracle through the
    intermission, then the title loop (gamestate GS_DEMOSCREEN) at the last
    tic instead of E1M2."""
    args = ['-warp', '1', '1', '-skill', '4', '--cmds', cmds, '--tics', str(tics),
            '--exit-at', str(E1M1_TICS)]
    print('[e1m1pack-end] %s' % ' '.join(args))
    ho = os.path.join(OUT, 'e1m1pack-end.oracle.txt')
    if run(oracle, ['-iwad', WAD] + args, ho, os.path.join(OUT, 'e1m1pack-end.oracle.log')):
        print('  FAIL: oracle run failed')
        return False
    so = read_stream(ho)
    ok = True
    for engine in engines:
        tag = os.path.basename(engine)
        he = os.path.join(OUT, 'e1m1pack-end.%s.txt' % tag)
        dump = os.path.join(OUT, 'e1m1pack-end.%s.dump' % tag)
        rc = run(engine, engine_args(nor, args), he,
                 os.path.join(OUT, 'e1m1pack-end.%s.log' % tag), (tics - 1, dump))
        se = read_stream(he)
        d = first_diff(so, se)
        state = read_dump(dump).get('gamestate') if os.path.exists(dump) else None
        good = rc == 0 and len(se) == tics and d is not None and d > E1M1_TICS and state == '3'
        print('  %s: exit %d, %d tics, same as the oracle up to tic %s, gamestate %s at tic %d%s'
              % (tag, rc, len(se), int(so[d][0]) - 1 if d is not None and d < len(so) else '-',
                 state, tics - 1, '' if good else ', FAIL: no clean end of the pack'))
        ok &= good
    return ok


def script_combat(dump):
    """The oracle state at the last E1M1 tic of the script workload."""
    if not os.path.exists(dump):
        print('  FAIL: no state dump at tic %d' % (E1M1_TICS - 1))
        return False
    d = read_dump(dump)
    kills, health = int(d['pl.killcount[0]']), int(d['pl.health[0]'])
    good = d['gamemap'] == '1' and kills > 0 and health < 100
    print('  E1M1 at tic %d: %d kills, health %d%s' % (E1M1_TICS - 1, kills, health,
                                                     '' if good else ', FAIL: no combat on E1M1'))
    return good


def route_names():
    """Every stored route: e1mM_sS, and e1m3secret_sS for the secret exit of E1M3."""
    names = []
    for sk in ROUTE_SKILLS:
        for m in range(1, 10):
            names.append('e1m%d_s%d' % (m, sk))
            if m == 3:
                names.append('e1m3secret_s%d' % sk)
    return names


def route_map(name):
    """(map, secret exit, skill) of a route name."""
    level, sk = name.split('_s')
    return int(level[3]), level.endswith('secret'), int(sk)


def route_next(m, secret):
    """The map that follows map m of E1, 0 for the finale."""
    if secret:
        return 9
    return {8: 0, 9: 4}.get(m, m + 1)


def route_path(name):
    return os.path.join(ROUTES, name + '.route')


def route_end(path):
    """The level time at which the last cmd line of a route ends."""
    end = 0
    with open(path) as f:
        for line in f:
            w = line.split('#')[0].split()
            if w and w[0] == 'cmd':
                end = max(end, int(w[2]) + int(w[3]))
    return end


def route_args(name, path=None):
    """Game arguments of the exits run of a route: the route, then EXIT_TAIL tics."""
    m, _, sk = route_map(name)
    path = path or route_path(name)
    return ['-warp', '1', str(m), '-skill', str(sk), '--refill', '--route', path,
            '--tics', str(route_end(path) + EXIT_TAIL)]


def read_states(path):
    """The oracle's --state-out lines: (gametic, gamestate, gamemap, leveltime, rndindex)."""
    if not os.path.exists(path):
        return []
    with open(path) as f:
        return [tuple(int(v) for v in line.split()) for line in f if line.strip()]


def check_exit(states, m, secret):
    """The level of map m, where the run starts, ends in the intermission (the
    finale after E1M8) and the run ends on the next map. Returns (None, tic of
    the first tic after the level) or (what is wrong, None)."""
    if not states or states[0][1] != GS_LEVEL or states[0][2] != m:
        return 'the run does not start on E1M%d' % m, None
    ended = [st for st in states if st[1] != GS_LEVEL]
    if not ended:
        return 'E1M%d never ends' % m, None
    nxt, last = route_next(m, secret), states[-1]
    if nxt and ended[0][1] != GS_INTERMISSION:
        return 'E1M%d ends in gamestate %d, not in the intermission' % (m, ended[0][1]), None
    if nxt and (last[1] != GS_LEVEL or last[2] != nxt):
        return 'the run ends in gamestate %d on map %d, not on E1M%d' % (last[1], last[2], nxt), None
    if not nxt and (ended[0][1] != GS_FINALE or last[1] != GS_FINALE):
        return 'E1M8 does not end in the finale', None
    return None, ended[0][0]


def finale_art(states):
    """(first tic of the finale, tic of its switch to the art stage), None for
    each that the run does not reach. No game code in the finale draws random
    numbers, so rndindex moves only by the wipe after a tic: the one into the
    finale and the one into the art stage."""
    fin = [st for st in states if st[1] == GS_FINALE]
    if not fin:
        return None, None
    steps = [a[0] for a, b in zip(fin, fin[1:]) if b[0] == a[0] + 1 and b[4] != a[4]]
    wipes = [a[0] for a, b in zip(fin, fin[1:]) if b[0] == a[0] + 1 and (b[4] - a[4]) % 256 == WIPE_RND]
    if len(steps) < 2 or steps[:2] != wipes[:2] or steps[0] != fin[0][0]:
        return fin[0][0], None
    return fin[0][0], steps[1]


def level_maps(states):
    """The maps a run plays, in order (a map played again at once counts once)."""
    maps = []
    for st in states:
        if st[1] == GS_LEVEL and (not maps or maps[-1] != st[2]):
            maps.append(st[2])
    return maps


def exits_job(name, oracle, engines):
    """One run of the exits workload. Returns (ok, report lines)."""
    out = []
    m, secret, _ = route_map(name)
    states = os.path.join(OUT, name + '.states')
    ok = workload(name, route_args(name), oracle, engines, states=states, events=True,
                  say=out.append)
    note, tic = check_exit(read_states(states), m, secret)
    if note:
        out.append('  FAIL: ' + note)
        return False, out
    nxt = route_next(m, secret)
    out.append('  E1M%d ends at tic %d, the run ends %s' % (
        m, tic, 'on E1M%d' % nxt if nxt else 'in the finale'))
    return ok, out


def exits(oracle, engines):
    """Every stored route, in parallel."""
    names = route_names()
    missing = [n for n in names if not os.path.exists(route_path(n))]
    if missing:
        print('[exits] FAIL: no route %s, run tools/routes.py' % ', '.join(missing))
        return False
    ok = True
    with concurrent.futures.ThreadPoolExecutor(JOBS) as ex:
        for good, lines in ex.map(lambda n: exits_job(n, oracle, engines), names):
            ok &= good
            for line in lines:
                print(line)
    return ok


def episode(oracle, engines, sk):
    """The routes of EPISODE chained at skill sk, then EPISODE_FINALE tics of
    the finale. A first oracle run finds the tic the finale starts."""
    name = 'episode-s%d' % sk
    paths = [route_path('%s_s%d' % (r, sk)) for r in EPISODE]
    game = ['-warp', '1', '1', '-skill', str(sk), '--refill']
    for path in paths:
        game += ['--route', path]
    bound = sum(route_end(path) + EXIT_TAIL for path in paths) + EPISODE_FINALE
    probe = os.path.join(OUT, name + '.probe.states')
    run(oracle, ['-iwad', WAD] + game + ['--tics', str(bound)], os.devnull,
        os.path.join(OUT, name + '.probe.log'), states=probe)
    first, _ = finale_art(read_states(probe))
    if first is None:
        print('[%s] FAIL: the oracle does not reach the finale in %d tics' % (name, bound))
        return False
    states = os.path.join(OUT, name + '.states')
    ok = workload(name, game + ['--tics', str(first + EPISODE_FINALE)], oracle, engines,
                  states=states, events=True)
    st = read_states(states)
    first, art = finale_art(st)
    want = [route_map(r + '_s1')[0] for r in EPISODE]
    maps = level_maps(st)
    good = maps == want and art is not None
    print('  maps %s, finale from tic %s, art stage from tic %s%s' % (
        ' '.join('E1M%d' % m for m in maps), first, art,
        '' if good else ', FAIL: expected %s and the art stage' % ' '.join('E1M%d' % m for m in want)))
    return ok and good


def mklmp(path, tics):
    """The E1M1 script as a vanilla demo of tics tics (tools/mklmp.py)."""
    subprocess.run([sys.executable, '-I', MKLMP, '-o', path, '--tics', str(tics)], check=True)


def exitlist_lmp(oracle, lmp, probe):
    """The demo of the exitlist workload (also the multi-map demo of the
    firmware test) into lmp: the script demo through E1 with the exits of
    EXITLIST, up to EXITLIST_AFTER_ART tics after the art stage of the
    finale. A first oracle run on a longer demo finds that tic, its files
    start with probe. Returns the length in tics, None when the oracle does
    not reach the art stage."""
    mklmp(probe + '.lmp', EXITLIST_PROBE_TICS)
    run(oracle, ['-iwad', WAD, '-playdemo', probe + '.lmp', '--exit-list', EXITLIST], os.devnull,
        probe + '.log', states=probe + '.states')
    os.remove(probe + '.lmp')
    _, art = finale_art(read_states(probe + '.states'))
    if art is None:
        return None
    mklmp(lmp, art + EXITLIST_AFTER_ART)
    return art + EXITLIST_AFTER_ART


def exitlist(oracle, engines):
    """The demo of exitlist_lmp on every engine."""
    lmp = os.path.join(OUT, 'exitlist.lmp')
    if exitlist_lmp(oracle, lmp, os.path.join(OUT, 'exitlist.probe')) is None:
        print('[exitlist] FAIL: the oracle does not reach the art stage of the finale in %d tics'
              % EXITLIST_PROBE_TICS)
        return False
    states = os.path.join(OUT, 'exitlist.states')
    ok = workload('exitlist', ['-playdemo', lmp, '--exit-list', EXITLIST], oracle, engines,
                  states=states, events=True)
    st = read_states(states)
    first, art = finale_art(st)
    want = [1, 2, 3, 9, 4, 5, 6, 7, 8]
    maps = level_maps(st)
    good = maps == want and art is not None
    print('  maps %s, finale from tic %s, art stage from tic %s, demo of %d tics%s' % (
        ' '.join('E1M%d' % m for m in maps), first, art, (art or 0) + EXITLIST_AFTER_ART,
        '' if good else ', FAIL: expected %s and the art stage' % ' '.join('E1M%d' % m for m in want)))
    return ok and good


def menu_keys(path, side):
    """The key file of MENU_STEPS for side ('engine' or 'oracle')."""
    with open(path, 'w') as f:
        for tic, key, who, _ in MENU_STEPS:
            if who in ('both', side):
                f.write('%d %s down\n%d %s up\n' % (tic, key, tic + 1, key))


def menu(oracle, engines):
    """The menu workload: each side with its key file. The game starts on the
    same tic and stands still under the menu."""
    keys = {}
    for side in ('engine', 'oracle'):
        keys[side] = os.path.join(OUT, 'menu.%s.keys' % side)
        menu_keys(keys[side], side)
    states = os.path.join(OUT, 'menu.states')
    ok = workload('menu', ['--tics', str(MENU_TICS)], oracle, engines, states=states, keys=keys)
    st = {s_[0]: s_ for s_ in read_states(states)}
    game = [t for t in sorted(st) if st[t][1] == GS_LEVEL]
    paused = [st[t][3] for t in range(202, 215) if t in st]
    good = (bool(game) and game[0] == MENU_GAME and st[game[0]][2] == 1
            and len(set(paused)) == 1 and len(paused) == 13)
    print('  E1M1 from tic %s, level time %s under the menu%s' % (
        game[0] if game else None, sorted(set(paused)),
        '' if good else ', FAIL: expected E1M1 from tic %d and the game paused' % MENU_GAME))
    return ok and good


def read_level(path):
    """The level line of an --obj-stats file as a dict, or None."""
    if not os.path.exists(path):
        return None
    d = None
    with open(path) as f:
        for line in f:
            w = line.split()
            if w and w[0] == 'level':
                d = {'level': w[1]}
                d.update(zip(w[2::2], w[3::2]))
    return d


def corpus_job(oracle, engines, m, sk, seed):
    """One corpus run: the bot on the oracle, its replay on the oracle and
    on every engine. Returns (ok, report lines, level dicts)."""
    name = 'corpus-s%d-e1m%d-k%d' % (seed, m, sk)
    pre = os.path.join(OUT, name)
    game = CORPUS_PLAY + ['-warp', '1', str(m), '-skill', str(sk), '--tics', str(CORPUS_TICS)]
    replay = ['--cmds', pre + '.cmds', '--tour', pre + '.tour'] + game
    out, levels, ok = [], [], True
    ha, hb = pre + '.oracle1.txt', pre + '.oracle2.txt'
    rc = run(oracle, ['--bot-out', pre, '--bot-seed', str(seed), '-iwad', WAD] + game, ha,
             pre + '.oracle1.log')
    rc |= run(oracle, ['-iwad', WAD] + replay, hb, pre + '.oracle2.log')
    sa, sb = read_stream(ha), read_stream(hb)
    if rc or not sa or first_diff(sa, sb) is not None:
        return False, ['%s: FAIL, the oracle replay of the bot differs or failed' % name], levels
    for engine in engines:
        tag = os.path.basename(engine)
        he, st = '%s.%s.txt' % (pre, tag), '%s.%s.obj' % (pre, tag)
        rc = run(engine, engine_args(NOR, replay), he, '%s.%s.log' % (pre, tag), stats=st)
        se = read_stream(he)
        d = read_level(st)
        if rc or d is None:
            out.append('%s %s: FAIL, exit %d' % (name, tag, rc))
            ok = False
            continue
        d['workload'] = 'corpus'
        d['seed'] = seed
        levels.append(d)
        first = int(d.get('first_event', '-1'))
        if first < 0:
            diff = first_diff(sa, se)
            exact = diff is None
            note = 'identical every tic' if exact else 'differs from tic %s' % sa[min(diff, len(sa) - 1)][0]
        else:
            # a counted event changes the game from its tic on: the same game up to that tic
            pa = [x for x in sa if int(x[0]) < first]
            pe = [x for x in se if int(x[0]) < first]
            exact = pa == pe
            note = ('beyond the plan from tic %d (%s), identical before' % (
                first, ', '.join('%s %s' % (k, d[k]) for k in OBJ_EVENTS if d.get(k, '0') != '0'))
                if exact else 'differs before the first event at tic %d' % first)
        life = int(d['leveltime'])
        hazard = int(d.get('corpse_trace', '0'))
        good = exact and life >= CORPUS_TICS and not hazard
        if life < CORPUS_TICS:
            note += ', life of %d tics only' % life
        if hazard:
            note += ', %d traces past 128 intercepts with the corpses' % hazard
        ok &= good
        mon = int(d['monsters'])
        out.append('%s %s: %s, life %d, peak full %s/%s pages (need %s), awake %s of %d (%.0f%%), '
                   'corpses %s, drops %s, zombies %s, boss %s, clip %s%s' % (
                       name, tag, note, life, d['peak_full'], d['pages'], d['need_play'],
                       d['peak_awake'], mon, 100.0 * int(d['peak_awake']) / max(mon, 1),
                       d['peak_corpses'], d['peak_drops'], d['zombies'], d['boss'],
                       d['clip_promoted'], '' if good else ', FAIL'))
    return ok, out, levels


def corpus(oracle, engines):
    """Every E1 map and corpus skill with every bot seed, in parallel."""
    jobs = [(m, sk, seed) for seed in CORPUS_SEEDS for m in range(1, 10) for sk in CORPUS_SKILLS]
    print('[corpus] %d runs of %d tics: bot on the oracle, replays on the oracle and the engines'
          % (len(jobs), CORPUS_TICS))
    ok = True
    with concurrent.futures.ThreadPoolExecutor(max(1, (os.cpu_count() or 2) // 2)) as ex:
        for good, lines, levels in ex.map(lambda j: corpus_job(oracle, engines, *j), jobs):
            ok &= good
            for line in lines:
                print('  ' + line)
            obj_levels.extend(levels)
    cov = {k: sum(int(d[k]) for d in obj_levels if d['workload'] == 'corpus')
           for k in ('zombies', 'boss', 'clip_promoted')}
    beyond = sorted({(d['level'], d['skill']) for d in obj_levels
                     if d['workload'] == 'corpus' and int(d.get('first_event', '-1')) >= 0})
    print('  coverage: zombies %d, A_BossDeath %d, height clip promotions %d' % (
        cov['zombies'], cov['boss'], cov['clip_promoted']))
    if beyond:
        print('  beyond the plan (counted events, the bot woke more than the plan holds): %s' % (
            ', '.join('%s skill %s' % b for b in beyond)))
    if not cov['zombies'] or not cov['boss']:
        print('  FAIL: no zombie or no A_BossDeath in the corpus')
        ok = False
    return ok


def corpus_report():
    """Peaks per map and skill over the corpus runs against the plans."""
    peaks = {}
    for d in obj_levels:
        if not d['workload'].startswith('corpus'):
            continue
        key = (d['level'], d['skill'])
        p = peaks.setdefault(key, dict(d))
        for k in ('peak_full', 'peak_awake', 'peak_corpses', 'peak_drops', 'peak_movers',
                  'peak_pages', 'leveltime'):
            p[k] = str(max(int(p[k]), int(d[k])))
    if not peaks:
        return
    print('object store peaks of the corpus, every seed (need_play = 1 + 24 + '
          'max(24, 30% of the monsters), pages = arena pages of the plan, reserve 8 of them)')
    print('%-5s %5s %4s %5s %5s %9s %6s %7s %6s %5s %6s %10s %8s %7s' % (
        'map', 'skill', 'plan', 'pages', 'need', 'peak full', 'awake', 'corpses', 'drops',
        'movers', 'pages', 'full/need', 'monsters', 'awake%'))
    for key in sorted(peaks):
        p = peaks[key]
        print('%-5s %5s %4s %5s %5s %9s %6s %7s %6s %5s %6s %9.0f%% %8s %6.0f%%' % (
            key[0], key[1], p['plan'], p['pages'], p['need_play'], p['peak_full'],
            p['peak_awake'], p['peak_corpses'], p['peak_drops'], p['peak_movers'],
            p['peak_pages'], 100.0 * int(p['peak_full']) / int(p['need_play']), p['monsters'],
            100.0 * int(p['peak_awake']) / max(int(p['monsters']), 1)))


def demos(oracle, engines):
    """DEMO2 to DEMO4 of the WAD that are single player demos (v1.9 header), from files."""
    ok = True
    for n in range(2, 5):
        data = wad_lump(WAD, 'DEMO%d' % n)
        if data is None:
            continue
        if len(data) < 13 or any(data[10:13]):
            print('[demo%d] not a single player demo, skipped' % n)
            continue
        path = os.path.join(OUT, 'demo%d.lmp' % n)
        with open(path, 'wb') as f:
            f.write(data)
        ok &= workload('demo%d' % n, ['-playdemo', path], oracle, engines)
    return ok


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--zip')
    ap.add_argument('--iwad', help='IWAD of the oracle instead of freedoom1.wad of the zip')
    ap.add_argument('--workload', default='all', choices=WORKLOADS + ('all', 'corpus', 'demos'))
    ap.add_argument('--tics', type=int, default=4500, help='length of the script workload')
    ap.add_argument('--oracle', default=os.path.join(ROOT, 'build', 'oracle', 'dg_oracle'))
    ap.add_argument('--engine', action='append', default=None,
                    help='engine binary, repeatable (default build/host/dmcdoom)')
    ap.add_argument('--asan', action='store_true', help='also run dmcdoom-asan next to the engine')
    ap.add_argument('--tag', default='default')
    ap.add_argument('--nor', default=os.path.join(ROOT, 'build', 'nor', 'E1.img'),
                    help='NOR image of the E1 pack for the engine')
    ap.add_argument('--nor-e1m1', default=os.path.join(ROOT, 'build', 'nor', 'E1M1.img'),
                    help='NOR image of the E1M1 pack (workload e1m1pack)')
    ap.add_argument('--engine-arg', action='append', default=[],
                    help='extra engine argument, repeatable')
    ap.add_argument('--parity', action='store_true',
                    help='the engine is dmcdoom-parity: give it the WAD and names.json too')
    ap.add_argument('--obj-stats', action='store_true',
                    help='object store statistics: events must be 0, corpus peaks reported')
    a = ap.parse_args()
    global OUT, WAD, NOR, ENGINE_ARGS, PARITY, OBJ_STATS
    os.makedirs(OUT, exist_ok=True)
    if a.iwad and not os.path.isfile(a.iwad):
        sys.exit('%s: no such file' % a.iwad)
    wad = os.path.abspath(a.iwad) if a.iwad else get_wad(a.zip)
    WAD = wad
    NOR = os.path.abspath(a.nor)
    ENGINE_ARGS = a.engine_arg
    PARITY = a.parity
    OBJ_STATS = a.obj_stats
    for img in (a.nor, a.nor_e1m1):
        if not os.path.exists(img):
            sys.exit('%s missing, run make -j$(nproc) nor-images' % img)
    OUT = os.path.join(OUT, a.tag)
    os.makedirs(OUT, exist_ok=True)
    a.oracle = os.path.abspath(a.oracle)
    engines = [os.path.abspath(e) for e in (a.engine or [os.path.join(ROOT, 'build', 'host', 'dmcdoom')])]
    if a.asan:
        engines.append(os.path.join(os.path.dirname(engines[0]), 'dmcdoom-asan'))
    for b in [a.oracle] + engines:
        if not os.path.exists(b):
            sys.exit('%s missing, run make -j$(nproc) oracle host' % b)
    ok = True
    sel = WORKLOADS if a.workload == 'all' else (a.workload,)
    cmds = os.path.join(OUT, 'e1m1_script.cmds')
    end_tics = max(a.tics, E1M1_TICS + 1000)
    make_cmds(cmds, max(a.tics, MAP_TICS, end_tics))
    if 'demo1' in sel:
        ok &= workload('demo1', ['-playdemo', 'demo1'], a.oracle, engines)
    if 'script' in sel:
        args = ['-warp', '1', '1', '-skill', '4', '--cmds', cmds, '--tics', str(a.tics)]
        dump = None
        if a.tics > E1M1_TICS:
            args += ['--exit-at', str(E1M1_TICS)]
            dump = (E1M1_TICS - 1, os.path.join(OUT, 'script.e1m1.dump'))
        ok &= workload('script', args, a.oracle, engines, dump)
        if dump:
            ok &= script_combat(dump[1])
    if 'attract' in sel:
        ok &= workload('attract', ['--tics', str(ATTRACT_TICS)], a.oracle, engines)
    if 'maps' in sel:
        for m in range(2, 10):
            args = ['-warp', '1', str(m), '-skill', '4', '--cmds', cmds,
                    '--tics', str(MAP_TICS), '--exit-at', str(MAP_EXIT)]
            ok &= workload('e1m%d' % m, args, a.oracle, engines)
    if 'e1m1pack' in sel:
        args = ['-warp', '1', '1', '-skill', '4', '--cmds', cmds, '--tics', str(E1M1_TICS)]
        ok &= workload('e1m1pack', args, a.oracle, engines, nor=os.path.abspath(a.nor_e1m1))
        ok &= pack_end(a.oracle, engines, cmds, os.path.abspath(a.nor_e1m1), end_tics)
    if 'exits' in sel:
        ok &= exits(a.oracle, engines)
    if 'episode' in sel:
        for sk in EPISODE_SKILLS:
            ok &= episode(a.oracle, engines, sk)
    if 'exitlist' in sel:
        ok &= exitlist(a.oracle, engines)
    if 'menu' in sel:
        ok &= menu(a.oracle, engines)
    if 'corpus' in sel:
        ok &= corpus(a.oracle, engines)
        corpus_report()
    if 'demos' in sel:
        ok &= demos(a.oracle, engines)
    print('PASS' if ok else 'FAIL')
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
