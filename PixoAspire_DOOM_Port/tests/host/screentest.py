#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Front-end screens of the engine against the oracle.

Both programs write a hash of every frame outside a level (--screen-hash):
the oracle of its 320x200 screen, the engine of the LCD rows 0..199 and of
the band rows 200..239, palette indices both. A tic with a wipe gives the
oracle several frames, the last one is the frame of that tic. Every frame of
the oracle must be in the engine's list with the same hash, and the band of
every frame of the engine must be colour 0. The engine runs with --lcd-check
and --screen-full: every frame also shows its palette and equals the same
screen composed whole, which finds groups the record pass missed.

Workloads, on the E1 pack against the oracle with build/packs/E1/halfui.wad
(TITLEPIC and WIMAP0 at full width with every odd column a copy of the even
one before it, what the half-width pictures of the pack show), and on the
E1M1 pack (full width) against the plain oracle:
  title    no game arguments: the title page
  script   the lockstep E1M1 script with the exit at tic 3200: the
           intermission into E1M2. The E1M1 pack has no E1M2, there the
           engine goes on to the title loop after the intermission.
  count    E1 pack: the same with the script ending at the exit, so nothing
           speeds the intermission up: the whole count of the statistics
  exits    E1 pack: the 20 routes of the lockstep exits workload, each
           intermission and the finale text after E1M8
  finale   E1 pack: -warp 1 8 -skill 4 --exit-at 100 --tics 2100, the text
           stage, then the art stage against the title page of the oracle
           (the engine shows TITLEPIC where the oracle shows CREDIT, which
           the E1 pack does not have)
Then the oracle's hash streams of the lockstep attract and script workloads
must be the same with halfui.wad and without it.

The menu, on the E1 pack: the engine writes its frames (--frame-out) and
the menu's panel of every frame that shows the menu (--panel-out), the
oracle the menu alone drawn into a zeroed screen (--menu-out).
  menu     the keys of the lockstep menu workload (run_lockstep.py
           MENU_STEPS). Where both show the same page, inside the panel the
           frame equals the oracle's menu: the main menu above row 126 (the
           oracle has Read This in row 5), the skill page and the prompts
           whole, the options above row 68 (their title and first two
           rows), Load Game and Save Game above row 114 (their title and
           four slots, the oracle has six) while every slot is empty. Outside
           the panel rows 0..199 are the frame before the menu opened, the
           band rows are colour 0 outside a level. Load Game with the save
           of the workload is the engine's own content: its panel against
           the reviewed hash of tests/host/screens.txt.
  own      engine only, what the oracle does not have: the meter's options,
           its own messages and Nightmare answered y (a second message, then
           the skill page again), on an image with a SAVE.DAT of the wrong
           size (Load Game says the saves are off). Parts
           of frames against the reviewed hashes of tests/host/screens.txt,
           the page after the Nightmare note from the panel lines.
  over     engine only: the menu opened and closed over the intermission
           and over the finale text, which go on under it and are composed
           again when it closes (--screen-full).
  reopen   engine only: POWER held on the title (port/game/i_input.c), its
           Escape opens the menu, 3 s later Escape closes it and F10 opens
           the quit prompt in the same tic, no frame between them. Outside
           the panel every frame of the menu is the title before it opened.

On a difference both programs run again with --frame-out and the frames of
the first differing tics stay in the output directory.

usage: screentest.py --oracle BIN --engine BIN [--nor E1.img] [--nor-e1m1 E1M1.img]
                     [--tag NAME] [--jobs N]
Outputs go to build/screens/NAME (default: default).
"""
import argparse
import concurrent.futures
import os
import shutil
import struct
import subprocess
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.join(ROOT, 'tests', 'lockstep'))
import run_lockstep as LS  # noqa: E402

FNV_OFFSET, FNV_PRIME = 0xcbf29ce484222325, 0x100000001b3
BAND_BYTES = 40 * 320
SCRIPT_TICS = 4500
FINALE = ['-warp', '1', '8', '-skill', '4', '--exit-at', '100', '--tics', '2100']
KEEP = 3  # differing frames kept per workload


def fnv(data):
    h = FNV_OFFSET
    for b in data:
        h = ((h ^ b) * FNV_PRIME) & 0xffffffffffffffff
    return h


BAND0 = '%016x' % fnv(bytes(BAND_BYTES))


def run(binary, args, cwd, log):
    os.makedirs(cwd, exist_ok=True)
    with open(log, 'w') as f:
        return subprocess.run([binary] + args, stdout=f, stderr=subprocess.STDOUT,
                              cwd=cwd).returncode


def read_hashes(path):
    """tic -> the fields after the tic of its last line."""
    out = {}
    if os.path.exists(path):
        with open(path) as f:
            for line in f:
                w = line.split()
                if w:
                    out[int(w[0])] = w[1:]
    return out


class Case:
    def __init__(self, name, pack, args, extra=False):
        self.name, self.pack, self.args, self.extra = name, pack, args, extra


def oracle_args(c, cfg):
    a = ['-iwad', LS.WAD]
    if c.pack == 'E1':
        a += ['-file', cfg.halfui]
    return a + c.args


def engine_args(c, cfg):
    return ['--nor', cfg.nor[c.pack], '--lcd-check', '--screen-full'] + c.args


def keep_frames(c, cfg, tics):
    """Runs both programs again with --frame-out, keeps the frames of tics."""
    d = os.path.join(cfg.out, c.name)
    for who, binary, args in (('oracle', cfg.oracle, oracle_args(c, cfg)),
                              ('engine', cfg.engine, engine_args(c, cfg))):
        fd = os.path.join(d, who + '.frames')
        shutil.rmtree(fd, ignore_errors=True)
        os.makedirs(fd)
        run(binary, args + ['--frame-out', fd], d, os.path.join(d, who + '.frames.log'))
        keep = {'f%06d.png' % t for t in tics}
        for f in os.listdir(fd):
            if f not in keep:
                os.remove(os.path.join(fd, f))
    return d


def check(c, cfg, title):
    """One workload. Returns (ok, report lines)."""
    d = os.path.join(cfg.out, c.name)
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    oh, eh, st = (os.path.join(d, n) for n in ('oracle.txt', 'engine.txt', 'oracle.states'))
    rc = run(cfg.oracle, oracle_args(c, cfg) + ['--screen-hash', oh, '--state-out', st], d,
             os.path.join(d, 'oracle.log'))
    if rc:
        return False, ['[%s] FAIL: the oracle exited with %d' % (c.name, rc)]
    rc = run(cfg.engine, engine_args(c, cfg) + ['--screen-hash', eh], d,
             os.path.join(d, 'engine.log'))
    if rc:
        return False, ['[%s] FAIL: the engine exited with %d, see %s' % (
            c.name, rc, os.path.join(d, 'engine.log'))]
    o, e = read_hashes(oh), read_hashes(eh)
    _, art = LS.finale_art(LS.read_states(st))
    states = {s[0]: s[1] for s in LS.read_states(st)}
    bad, missing, arts = [], [], 0
    for t in sorted(o):
        want = o[t][0]
        # the frame of tic t follows the tic numbered t - 1 in the states
        if art is not None and t > art and states.get(t - 1) == LS.GS_FINALE:
            if title is None:
                return False, ['[%s] FAIL: no title page to compare the art stage with' % c.name]
            want = title
            arts += 1
        if t not in e:
            missing.append(t)
        elif e[t][0] != want:
            bad.append(t)
    extra = sorted(set(e) - set(o))
    band = [t for t in sorted(e) if e[t][1] != BAND0]
    ok = bool(o) and not bad and not missing and not band and (c.extra or not extra)
    lines = ['[%s] %d frames, %d of them of the art stage, %s' % (
        c.name, len(o), arts, 'identical' if ok else 'FAIL')]
    if not o:
        lines.append('  FAIL: the oracle has no frame outside a level')
    if missing:
        lines.append('  FAIL: no engine frame for tics %s' % missing[:8])
    if extra:
        lines.append('  %sengine frames the oracle does not have: %d from tic %d' % (
            '' if c.extra else 'FAIL: ', len(extra), extra[0]))
    if band:
        lines.append('  FAIL: band rows not colour 0 in tics %s' % band[:8])
    if bad:
        kept = keep_frames(c, cfg, bad[:KEEP])
        lines.append('  FAIL: %d frames differ from tic %d, frames of tics %s in %s' % (
            len(bad), bad[0], bad[:KEEP], kept))
    for f in ('oracle.frames', 'engine.frames'):
        if not bad:
            shutil.rmtree(os.path.join(d, f), ignore_errors=True)
    return ok, lines


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


def read_panels(path):
    """tic -> (menuactive, message, page, item, message id, x0, x1, y0, y1, gamestate) of
    --panel-out."""
    out = {}
    if os.path.exists(path):
        with open(path) as f:
            for line in f:
                w = [int(v) for v in line.split()]
                if w:
                    out[w[0]] = w[1:]
    return out


# Rows of the screen where a page of the menu is the oracle's (the rest of the panel is ours)
MENU_ROWS = {'main': 126, 'new': 200, 'prompt': 200, 'options': 68, 'load': 114, 'save': 114}
# Pages of the menu workload that only the engine has: the reviewed hash of their panel in
# tests/host/screens.txt, by page
MENU_OWN = {'saved': 'loadsaved'}


def menu_check(cfg):
    """The menu workload: the menu inside the panel against the oracle's, the frozen picture
    outside it. Returns (ok, report lines)."""
    d = os.path.join(cfg.out, 'menu')
    shutil.rmtree(d, ignore_errors=True)
    home, ef, om = (os.path.join(d, n) for n in ('oracle.home', 'engine.frames', 'oracle.menu'))
    for x in (home, ef, om):
        os.makedirs(x)
    keys = {}
    for side in ('engine', 'oracle'):
        keys[side] = os.path.join(d, side + '.keys')
        LS.menu_keys(keys[side], side)
    tics = ['--tics', str(LS.MENU_TICS)]
    with open(os.path.join(d, 'oracle.log'), 'w') as f:
        rc = subprocess.run([cfg.oracle, '-iwad', LS.WAD, '-file', cfg.halfui, '--keys',
                             keys['oracle'], '--menu-out', om] + tics, stdout=f,
                            stderr=subprocess.STDOUT, cwd=home, env=dict(os.environ, HOME=home)).returncode
    if rc:
        return False, ['[menu] FAIL: the oracle exited with %d' % rc]
    panels = os.path.join(d, 'engine.panels')
    rc = run(cfg.engine, ['--nor', cfg.nor['E1'], '--lcd-check', '--screen-full', '--keys',
                          keys['engine'], '--frame-out', ef, '--panel-out', panels] + tics, d,
             os.path.join(d, 'engine.log'))
    if rc:
        return False, ['[menu] FAIL: the engine exited with %d, see %s' % (rc, os.path.join(d, 'engine.log'))]
    pan = read_panels(panels)
    page, steps = {}, list(LS.MENU_STEPS)
    for i, (tic, _, _, pg) in enumerate(steps):
        end = steps[i + 1][0] if i + 1 < len(steps) else LS.MENU_TICS
        for t in range(tic + 1, end + 1):
            page[t] = pg
    bad, seen, before, gold = [], {}, None, read_goldens()
    for t in range(1, LS.MENU_TICS):
        fp = os.path.join(ef, 'f%06d.png' % t)
        if t not in pan:
            before = fp
            continue
        pg = page.get(t)
        mp = os.path.join(om, 'm%06d.png' % t)
        if pg is None:
            continue
        if pg in MENU_OWN:
            x0, x1, y0, y1, want = gold[MENU_OWN[pg]]
            got = part_hash(png_pixels(fp)[1], x0, x1, y0, y1) if os.path.exists(fp) else None
            if got != want:
                bad.append((t, 'hash %s of %s, screens.txt has %s' % (got, MENU_OWN[pg], want)))
            seen[pg] = seen.get(pg, 0) + 1
            continue
        if not os.path.exists(mp) or not os.path.exists(fp) or before is None:
            bad.append((t, 'no oracle menu or engine frame'))
            continue
        _, e = png_pixels(fp)
        _, o = png_pixels(mp)
        _, b = png_pixels(before)
        x0, x1, y0, y1 = pan[t][5:9]
        rows = MENU_ROWS[pg]
        why = None
        for y in range(200):
            for x in range(320):
                inside = x0 <= x <= x1 and y0 <= y <= y1
                if inside and y < rows and e[y * 320 + x] != o[y * 320 + x]:
                    why = 'pixel %d,%d is %d in the panel, the oracle has %d' % (
                        x, y, e[y * 320 + x], o[y * 320 + x])
                elif not inside and e[y * 320 + x] != b[y * 320 + x]:
                    why = 'pixel %d,%d is %d outside the panel, %d before the menu' % (
                        x, y, e[y * 320 + x], b[y * 320 + x])
                if why:
                    break
            if why:
                break
        if not why and t < LS.MENU_GAME and any(e[320 * 200:]):
            why = 'band rows not colour 0'
        if why:
            bad.append((t, why))
        seen[pg] = seen.get(pg, 0) + 1
    pages = set(MENU_ROWS) | set(MENU_OWN)
    ok = not bad and set(seen) == pages
    lines = ['[menu] frames compared per page: %s, %s' % (
        ', '.join('%s %d' % kv for kv in sorted(seen.items())), 'identical' if ok else 'FAIL')]
    if set(seen) != pages:
        lines.append('  FAIL: no frame of %s' % sorted(pages - set(seen)))
    for t, why in bad[:KEEP]:
        lines.append('  FAIL: tic %d (%s): %s' % (t, page.get(t), why))
    if ok:
        shutil.rmtree(d, ignore_errors=True)
    else:
        lines.append('  frames in %s' % d)
    return ok, lines


# The engine's own menu content, engine only: (tic, key, the golden part of the frame after
# that tic, from tests/host/screens.txt by name)
OWN_STEPS = (
    (20, 'KEY_ESCAPE', None),
    (25, 'KEY_ENTER', None),           # New Game, the cursor on I'm Too Young To Die
    (30, 'KEY_UPARROW', None),         # Nightmare
    (35, 'KEY_ENTER', None),           # its prompt
    (45, 'y', 'nonightmare'),          # the note
    (60, 'KEY_ENTER', None),           # the skill page again, on Nightmare
    (70, 'KEY_BACKSPACE', None),
    (75, 'KEY_DOWNARROW', None),
    (80, 'KEY_ENTER', 'options'),      # the options, sound low
    (85, 'KEY_DOWNARROW', None),
    (90, 'KEY_DOWNARROW', None),
    (95, 'KEY_RIGHTARROW', 'soundhigh'),
    (100, 'KEY_RIGHTARROW', 'soundoff'),
    (105, 'KEY_RIGHTARROW', None),     # low again
    (110, 'KEY_DOWNARROW', None),
    (115, 'KEY_ENTER', 'usb'),         # the USB drive prompt
    (130, 'n', None),
    (140, 'KEY_ESCAPE', None),
    (145, 'KEY_ENTER', None),          # the options, on the USB drive
    (150, 'KEY_DOWNARROW', None),
    (155, 'KEY_ENTER', 'bench'),       # the benchmark prompt
    (170, 'n', None),
    (180, 'KEY_ESCAPE', None),
    (185, 'KEY_DOWNARROW', None),      # Load Game
    (190, 'KEY_ENTER', 'nosave'),      # SAVE.DAT of the wrong size
    (205, 'KEY_ENTER', None),
)
OWN_TICS = 220
OWN_NOTE = 60  # the tic of the key that closes the Nightmare note
SCREENS = os.path.join(ROOT, 'tests', 'host', 'screens.txt')
MKNOR = os.path.join(ROOT, 'tests', 'host', 'mknor.py')
BAD_SAVE_BYTES = 69632  # a SAVE.DAT of the wrong size, 17 clusters


def part_hash(px, x0, x1, y0, y1):
    """FNV-1a 64 of the palette indices of x0..x1, y0..y1 of a 320 wide frame, row by row."""
    return '%016x' % fnv(b''.join(px[y * 320 + x0:y * 320 + x1 + 1] for y in range(y0, y1 + 1)))


def read_goldens():
    """name -> (x0, x1, y0, y1, hash) of tests/host/screens.txt."""
    out = {}
    with open(SCREENS) as f:
        for line in f:
            w = line.split('#')[0].split()
            if w:
                out[w[0]] = tuple(int(v) for v in w[1:5]) + (w[5],)
    return out


def own_check(cfg):
    """The engine's own menu content against the goldens. Returns (ok, report lines)."""
    d = os.path.join(cfg.out, 'own')
    shutil.rmtree(d, ignore_errors=True)
    ef = os.path.join(d, 'engine.frames')
    os.makedirs(ef)
    keys, panels = os.path.join(d, 'engine.keys'), os.path.join(d, 'engine.panels')
    with open(keys, 'w') as f:
        for tic, key, _ in OWN_STEPS:
            f.write('%d %s down\n%d %s up\n' % (tic, key, tic + 1, key))
    bad, nor = os.path.join(d, 'badsave.dat'), os.path.join(d, 'badsave.img')
    with open(bad, 'wb') as f:
        f.write(bytes(BAD_SAVE_BYTES))
    subprocess.run([sys.executable, '-I', MKNOR, os.path.join(ROOT, 'build', 'packs', 'E1'), nor,
                    '--save', bad], check=True)
    rc = run(cfg.engine, ['--nor', nor, '--lcd-check', '--screen-full', '--keys', keys,
                          '--frame-out', ef, '--panel-out', panels, '--tics', str(OWN_TICS)], d,
             os.path.join(d, 'engine.log'))
    if rc:
        return False, ['[own] FAIL: the engine exited with %d, see %s' % (rc, os.path.join(d, 'engine.log'))]
    gold, pan, lines, ok = read_goldens(), read_panels(panels), [], True
    for i, (tic, _, name) in enumerate(OWN_STEPS):
        if not name:
            continue
        # the frame before the next key
        t = OWN_STEPS[i + 1][0]
        x0, x1, y0, y1, want = gold[name]
        got = part_hash(png_pixels(os.path.join(ef, 'f%06d.png' % t))[1], x0, x1, y0, y1)
        good = got == want
        ok &= good
        lines.append('  %s, tic %d: %s' % (name, t, 'as reviewed' if good else
                                            'FAIL: hash %s, screens.txt has %s' % (got, want)))
    # Nightmare answered y: the note, then the skill page with the cursor on Nightmare
    after = pan.get(OWN_NOTE + 2)
    good = after is not None and after[:4] == [1, 0, 1, 4]
    ok &= good
    lines.append('  after the Nightmare note: %s' % ('the skill page on Nightmare' if good else
                                                    'FAIL: panel line %s' % after))
    lines.insert(0, '[own] %d goldens, %s' % (sum(1 for s_ in OWN_STEPS if s_[2]),
                                            'identical' if ok else 'FAIL'))
    if ok:
        shutil.rmtree(d, ignore_errors=True)
    else:
        lines.append('  frames in %s' % d)
    return ok, lines


# (name, game arguments, the tics Escape opens and closes the menu, the gamestate under it)
OVER = (('intermission', ['-warp', '1', '1', '-skill', '4', '--exit-at', '50'], 80, 120,
         LS.GS_INTERMISSION),
        ('finale', ['-warp', '1', '8', '-skill', '4', '--exit-at', '100'], 200, 260, LS.GS_FINALE))


def over_check(cfg):
    """The menu over the intermission and the finale. Returns (ok, report lines)."""
    ok, lines = True, []
    for name, args, t0, t1, gs in OVER:
        d = os.path.join(cfg.out, 'over-' + name)
        shutil.rmtree(d, ignore_errors=True)
        os.makedirs(d)
        keys, panels = os.path.join(d, 'engine.keys'), os.path.join(d, 'engine.panels')
        with open(keys, 'w') as f:
            for t in (t0, t1):
                f.write('%d KEY_ESCAPE down\n%d KEY_ESCAPE up\n' % (t, t + 1))
        log = os.path.join(d, 'engine.log')
        rc = run(cfg.engine, ['--nor', cfg.nor['E1'], '--lcd-check', '--screen-full', '--keys', keys,
                              '--panel-out', panels, '--tics', str(t1 + 40)] + args, d, log)
        pan = read_panels(panels)
        up = sorted(pan)
        good = rc == 0 and up == list(range(t0 + 1, t1 + 1)) and all(pan[t][9] == gs for t in up)
        ok &= good
        lines.append('  %s: %s' % (name, 'the menu over it from tic %d to %d' % (t0 + 1, t1) if good
                                   else 'FAIL: exit %d, menu frames %s, see %s' % (
                                       rc, '%d..%d' % (up[0], up[-1]) if up else 'none', log)))
        if good:
            shutil.rmtree(d, ignore_errors=True)
    lines.insert(0, '[over] %s' % ('composed again and in its palette' if ok else 'FAIL'))
    return ok, lines


# The keys of POWER held on the title from tic 0 on: (tic, key, down or up)
REOPEN_KEYS = ((20, 'KEY_ESCAPE', 'down'), (21, 'KEY_ESCAPE', 'up'),
               (125, 'KEY_ESCAPE', 'down'), (125, 'KEY_F10', 'down'),
               (126, 'KEY_ESCAPE', 'up'), (127, 'KEY_F10', 'up'))
REOPEN_TICS = 150


def reopen_check(cfg):
    """The menu closed and opened again in one tic. Returns (ok, report lines)."""
    d = os.path.join(cfg.out, 'reopen')
    shutil.rmtree(d, ignore_errors=True)
    ef = os.path.join(d, 'engine.frames')
    os.makedirs(ef)
    keys, panels = os.path.join(d, 'engine.keys'), os.path.join(d, 'engine.panels')
    with open(keys, 'w') as f:
        for k in REOPEN_KEYS:
            f.write('%d %s %s\n' % k)
    log = os.path.join(d, 'engine.log')
    rc = run(cfg.engine, ['--nor', cfg.nor['E1'], '--lcd-check', '--screen-full', '--keys', keys,
                          '--frame-out', ef, '--panel-out', panels, '--tics', str(REOPEN_TICS)], d,
             log)
    if rc:
        return False, ['[reopen] FAIL: the engine exited with %d, see %s' % (rc, log)]
    pan = read_panels(panels)
    t0, t1 = REOPEN_KEYS[0][0], REOPEN_KEYS[2][0]
    _, before = png_pixels(os.path.join(ef, 'f%06d.png' % t0))
    why = None
    if sorted(pan) != list(range(t0 + 1, REOPEN_TICS)):
        why = 'menu frames %s' % sorted(pan)[:8]
    elif any(not pan[t][1] for t in range(t1 + 1, REOPEN_TICS)):
        why = 'no quit prompt after tic %d' % t1
    for t in sorted(pan):
        if why:
            break
        _, e = png_pixels(os.path.join(ef, 'f%06d.png' % t))
        x0, x1, y0, y1 = pan[t][5:9]
        for i in range(320 * 200):
            x, y = i % 320, i // 320
            if e[i] != before[i] and not (x0 <= x <= x1 and y0 <= y <= y1):
                why = 'tic %d: pixel %d,%d is %d outside the panel, %d before the menu' % (
                    t, x, y, e[i], before[i])
                break
    if not why:
        shutil.rmtree(d, ignore_errors=True)
    return not why, ['[reopen] %s' % ('the quit prompt over the title as it was before the menu'
                                      if not why else 'FAIL: %s, frames in %s' % (why, d))]


def pwad_check(cfg, cmds):
    """The oracle with halfui.wad gives the same hash streams as without it."""
    ok = True
    d = os.path.join(cfg.out, 'pwad')
    os.makedirs(d, exist_ok=True)
    for name, args in (('attract', ['--tics', str(LS.ATTRACT_TICS)]),
                       ('script', ['-warp', '1', '1', '-skill', '4', '--cmds', cmds, '--tics',
                                   str(SCRIPT_TICS), '--exit-at', str(LS.E1M1_TICS)])):
        streams = []
        for tag, pwad in (('plain', []), ('halfui', ['-file', cfg.halfui])):
            h = os.path.join(d, '%s.%s.txt' % (name, tag))
            run(cfg.oracle, ['-iwad', LS.WAD] + pwad + args + ['--hash-out', h], d,
                os.path.join(d, '%s.%s.log' % (name, tag)))
            streams.append(LS.read_stream(h))
        same = streams[0] and streams[0] == streams[1]
        print('[pwad %s] the oracle with halfui.wad: %d tics, %s' % (
            name, len(streams[0]), 'same hash stream' if same else 'FAIL: the streams differ'))
        ok &= bool(same)
    return ok


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--oracle', required=True)
    ap.add_argument('--engine', required=True)
    ap.add_argument('--nor', default=os.path.join(ROOT, 'build', 'nor', 'E1.img'))
    ap.add_argument('--nor-e1m1', default=os.path.join(ROOT, 'build', 'nor', 'E1M1.img'))
    ap.add_argument('--tag', default='default')
    ap.add_argument('--jobs', type=int, default=LS.JOBS)
    a = ap.parse_args()
    os.makedirs(LS.OUT, exist_ok=True)
    LS.WAD = LS.get_wad(None)
    cfg = argparse.Namespace(oracle=os.path.abspath(a.oracle), engine=os.path.abspath(a.engine),
                             nor={'E1': os.path.abspath(a.nor),
                                  'E1M1': os.path.abspath(a.nor_e1m1)},
                             halfui=os.path.join(ROOT, 'build', 'packs', 'E1', 'halfui.wad'),
                             out=os.path.join(ROOT, 'build', 'screens', a.tag))
    for f in (cfg.oracle, cfg.engine, cfg.halfui, cfg.nor['E1'], cfg.nor['E1M1']):
        if not os.path.exists(f):
            sys.exit('%s missing, run make -j$(nproc) oracle host nor-images' % f)
    os.makedirs(cfg.out, exist_ok=True)
    cmds = os.path.join(cfg.out, 'e1m1_script.cmds')
    LS.make_cmds(cmds, SCRIPT_TICS)
    script = ['-warp', '1', '1', '-skill', '4', '--cmds', cmds, '--tics', str(SCRIPT_TICS),
              '--exit-at', str(LS.E1M1_TICS)]
    short = os.path.join(cfg.out, 'e1m1_exit.cmds')
    LS.make_cmds(short, LS.E1M1_TICS)
    count = ['-warp', '1', '1', '-skill', '4', '--cmds', short, '--tics', str(LS.E1M1_TICS + 600),
             '--exit-at', str(LS.E1M1_TICS)]
    title = ['--tics', '170']

    ok = True
    titles = {}
    for pack in ('E1', 'E1M1'):
        c = Case('title-%s' % pack, pack, title)
        good, lines = check(c, cfg, None)
        ok &= good
        print('\n'.join(lines))
        frames = read_hashes(os.path.join(cfg.out, c.name, 'oracle.txt'))
        hashes = set(v[0] for v in frames.values())
        titles[pack] = hashes.pop() if len(hashes) == 1 else None

    cases = [Case('script-E1', 'E1', script), Case('script-E1M1', 'E1M1', script, extra=True),
             Case('count-E1', 'E1', count), Case('finale-E1', 'E1', FINALE)]
    cases += [Case('exits-%s' % n, 'E1', LS.route_args(n)) for n in LS.route_names()]
    with concurrent.futures.ThreadPoolExecutor(a.jobs) as ex:
        for good, lines in ex.map(lambda c: check(c, cfg, titles[c.pack]), cases):
            ok &= good
            print('\n'.join(lines))
    for fn in (menu_check, own_check, over_check, reopen_check):
        good, lines = fn(cfg)
        ok &= good
        print('\n'.join(lines))
    ok &= pwad_check(cfg, cmds)
    print('PASS' if ok else 'FAIL')
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
