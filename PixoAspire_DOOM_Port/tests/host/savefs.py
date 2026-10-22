#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""SAVE.DAT on the data drive of the host engine's NOR images (tests/host/mknor.py): the
promises of the slot header and of the creation.

  header   a save whose header has another build ID or image uuid: the Load Game page shows
           the slot empty (its slot rows equal those of an image without saves) and --load
           says it is empty
  crc      a save with a flipped body byte: --load says it is damaged, Load Game then the slot
           shows the message (--panel-out) and the game is as before (the same hash stream as
           with an empty slot)
  badsize  a SAVE.DAT of the wrong size (69,632 B, mknor --save): the run says the saves
           are off, a save fails, the image stays as it was (BOOT neither deletes nor makes
           SAVE.DAT)
  frag N   the free clusters in N runs (mknor --frag N): SAVE.DAT made in at most 8 extents
           (fsck.fat clean, both FATs equal, dmccheck --save clean) and a save and its load
           work on it. With the free clusters in runs whose 8 longest hold fewer than 16, the
           saves are off and the image stays as it was
  full     15 free clusters (a 6 cluster DOOMCODE.DMC on the volume): the saves are off, the
           image stays as it was
  cutsave  --nor-fail-after N for every N through one save: the slot is empty or holds the
           whole save, never one with a wrong body (dmccheck --save, --load)
  cutmake  --nor-fail-after N for every N through the creation: between whole sector writes
           only lost clusters (fsck.fat), and the next start makes SAVE.DAT or, with fewer than
           16 free clusters left, says the saves are off. A cut between the erase and the
           program of a sector loses that sector (NOR has no atomic sector write): the run
           reports which cuts those are.

usage: savefs.py [--engine BIN] [--tag NAME]
Outputs go to build/savefs/NAME (default: default), removed when everything passes.
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
sys.path.insert(0, os.path.join(ROOT, 'tests', 'host'))
import run_lockstep as LS  # noqa: E402
import screentest  # noqa: E402

MKNOR = os.path.join(ROOT, 'tests', 'host', 'mknor.py')
DMCCHECK = os.path.join(ROOT, 'tools', 'dmccheck.py')
PACK = os.path.join(ROOT, 'build', 'packs', 'E1')
MAIN = 0x100000
SLOT = 16384
HDR_BUILD, HDR_UUID, HDR_SIZE = 40, 48, 64
NOSAVES = 'SAVE.DAT is missing or not usable'
SAVE_TIC = 200
TICS = 400
MSG_BADSAVE = 0x84  # engine/m_menu.c
# The title, then Load Game, then its slot 0 in tic 45
LOAD_PAGE = ((20, 'KEY_ESCAPE'), (25, 'KEY_DOWNARROW'), (30, 'KEY_DOWNARROW'), (35, 'KEY_ENTER'))
LOAD_SLOT = (45, 'KEY_ENTER')
SLOT_ROWS = (72, 279, 28, 113)  # the title and the slots of the Load Game page
failures = []


def check(cond, msg):
    print(('  ok   ' if cond else '  FAIL ') + msg)
    if not cond:
        failures.append(msg)
    return cond


class Work:
    def __init__(self, engine, d):
        self.engine, self.d = engine, d
        os.makedirs(d, exist_ok=True)
        self.cmds = self.p('script.cmds')
        LS.make_cmds(self.cmds, TICS)
        self.game = ['-warp', '1', '1', '-skill', '4', '--cmds', self.cmds, '--tics', str(TICS)]

    def p(self, name):
        return os.path.join(self.d, name)

    def run(self, nor, args, log='run.log'):
        """(exit status, output) of the engine on image nor."""
        with open(self.p(log), 'w') as f:
            rc = subprocess.run([self.engine, '--nor', nor] + args, stdout=f, stderr=subprocess.STDOUT,
                                cwd=self.d).returncode
        with open(self.p(log)) as f:
            return rc, f.read()

    def mknor(self, name, *args):
        out = self.p(name)
        subprocess.run([sys.executable, '-I', MKNOR, PACK, out] + list(args), check=True)
        return out

    def keys(self, name, steps):
        path = self.p(name)
        with open(path, 'w') as f:
            for tic, key in steps:
                f.write('%d %s down\n%d %s up\n' % (tic, key, tic + 1, key))
        return path


def read(path):
    with open(path, 'rb') as f:
        return f.read()


def write(path, data):
    with open(path, 'wb') as f:
        f.write(data)
    return path


class Vol:
    """The data drive of a NOR image: its FATs, root directory and files."""

    def __init__(self, nor):
        self.v = nor[MAIN:]
        bps, spc, rsv, nfat, nroot = struct.unpack_from('<HBHBH', self.v, 11)
        self.cs = bps * spc
        fsz = struct.unpack_from('<H', self.v, 22)[0] * bps
        self.fats = [self.v[rsv * bps + k * fsz:rsv * bps + (k + 1) * fsz] for k in range(nfat)]
        self.root = rsv * bps + nfat * fsz
        self.data = self.root + nroot * 32
        self.nroot = nroot
        self.nclus = (len(self.v) - self.data) // self.cs

    def entry(self, c, fat=0):
        v = struct.unpack_from('<H', self.fats[fat], c * 3 // 2)[0]
        return v >> 4 if c & 1 else v & 0xFFF

    def free(self):
        return sum(1 for c in range(2, self.nclus + 2) if not self.entry(c))

    def find(self, name83):
        for i in range(self.nroot):
            e = self.v[self.root + 32 * i:self.root + 32 * i + 32]
            if not e[0]:
                return None
            if e[:11] == name83 and e[0] != 0xE5:
                return e
        return None

    def file(self, name83):
        """(bytes, extents) of a file, None when absent."""
        e = self.find(name83)
        if not e:
            return None
        c, size = struct.unpack_from('<H', e, 26)[0], struct.unpack_from('<I', e, 28)[0]
        out, runs, last = b'', 0, None
        while 2 <= c < 0xFF8 and len(out) < size:
            runs += last is None or c != last + 1
            o = self.data + (c - 2) * self.cs
            out += self.v[o:o + self.cs]
            last, c = c, self.entry(c)
        return out[:size], runs


def fsck(w, nor, name='vol.img'):
    """fsck.fat -n of the data drive: (clean, its report lines)."""
    path = write(w.p(name), nor[MAIN:])
    r = subprocess.run(['fsck.fat', '-n', path], capture_output=True, text=True)
    os.remove(path)
    lines = [x.strip() for x in (r.stdout + r.stderr).splitlines()[1:] if x.strip()]
    clean = r.returncode == 0 and len(lines) == 1
    return clean, lines


def lost_only(lines):
    """fsck.fat reports at most that the FATs differ and some unused clusters."""
    ok = ('FATs differ but appear to be intact.', 'Using first FAT.', 'Leaving filesystem unchanged.')
    return all(x in ok or x.startswith('Reclaimed ') or x.endswith(' clusters') for x in lines)


def dmccheck(w, data):
    path = write(w.p('SAVE.DAT'), data)
    r = subprocess.run([sys.executable, '-I', DMCCHECK, '--save', path], capture_output=True, text=True)
    os.remove(path)
    return r.returncode == 0, (r.stdout + r.stderr).strip().splitlines()[-1:]


def save_dat(nor):
    f = Vol(nor).file(b'SAVE    DAT')
    return f[0] if f else None


def patch_slot(w, src, name, slot, off, xor):
    """A copy of image src with byte off of slot slot of SAVE.DAT changed."""
    nor = bytearray(read(src))
    vol = Vol(bytes(nor))
    e = vol.find(b'SAVE    DAT')
    c, k = struct.unpack_from('<H', e, 26)[0], (slot * SLOT + off) // vol.cs
    for _ in range(k):
        c = vol.entry(c)
    nor[MAIN + vol.data + (c - 2) * vol.cs + (slot * SLOT + off) % vol.cs] ^= xor
    return write(w.p(name), bytes(nor))


def frames_slots(w, nor, keys, tic, tag):
    """The slot rows of the Load Game page in the frame of tic, and the panel lines."""
    fd = w.p(tag + '.frames')
    shutil.rmtree(fd, ignore_errors=True)
    os.makedirs(fd)
    panels = w.p(tag + '.panels')
    w.run(nor, ['--keys', keys, '--tics', str(tic + 5), '--frame-out', fd, '--panel-out', panels,
                '--hash-out', w.p(tag + '.txt')], tag + '.log')
    _, px = screentest.png_pixels(os.path.join(fd, 'f%06d.png' % tic))
    x0, x1, y0, y1 = SLOT_ROWS
    shutil.rmtree(fd, ignore_errors=True)
    pan = screentest.read_panels(panels)
    return b''.join(px[y * 320 + x0:y * 320 + x1 + 1] for y in range(y0, y1 + 1)), pan


def test_header(w, base, plain):
    print('header: another build ID or image uuid shows the slot empty')
    keys = w.keys('page.keys', LOAD_PAGE)
    empty, _ = frames_slots(w, plain, keys, 40, 'plain')
    full, _ = frames_slots(w, base, keys, 40, 'base')
    check(full != empty, 'the Load Game page shows the save of the base image')
    for what, off in (('build ID', HDR_BUILD), ('image uuid', HDR_UUID)):
        img = patch_slot(w, base, 'hdr.img', 0, off, 0x01)
        rows, _ = frames_slots(w, img, keys, 40, 'hdr')
        rc, out = w.run(img, w.game + ['--load', '0'])
        check(rows == empty and rc != 0 and 'the slot is empty' in out,
              'another %s: Load Game shows the slot empty, --load says so (exit %d)' % (what, rc))


def test_crc(w, base, plain):
    print('crc: a flipped body byte')
    img = patch_slot(w, base, 'crc.img', 0, HDR_SIZE + 1000, 0x10)
    rc, out = w.run(img, w.game + ['--load', '0'])
    check(rc != 0 and 'damaged (CRC)' in out, '--load says the save is damaged (exit %d)' % rc)
    keys = w.keys('slot.keys', LOAD_PAGE + (LOAD_SLOT,))
    _, pan = frames_slots(w, img, keys, 55, 'crc')
    _, pan0 = frames_slots(w, plain, keys, 55, 'empty')
    msg = pan.get(55)
    same = LS.read_stream(w.p('crc.txt')) == LS.read_stream(w.p('empty.txt'))
    check(msg is not None and msg[1] == 1 and msg[4] == MSG_BADSAVE,
          'Load Game, slot 0: the message that the save is damaged (panel %s)' % msg)
    check(same and pan0.get(55, [0, 0])[1] == 0,
          'the game as with an empty slot (the same hash stream)')


def test_badsize(w):
    print('badsize: a SAVE.DAT of the wrong size (69,632 B)')
    bad = write(w.p('bad.dat'), bytes(range(256)) * 272)
    img = w.mknor('badsize.img', '--save', bad)
    out_img = w.p('badsize.out.img')
    rc, out = w.run(img, w.game + ['--save-at', str(SAVE_TIC), '0', '--nor-out', out_img])
    check(NOSAVES + ': no saves' in out and rc != 0 and '--save-at %d 0: ' % SAVE_TIC + NOSAVES in out,
          'the run says the saves are off and the save fails (exit %d)' % rc)
    check(read(out_img) == read(img), 'the image is as it was, SAVE.DAT kept')


def round_trip(w, img, slot, tag):
    a, b = w.p(tag + '.a.txt'), w.p(tag + '.b.txt')
    out = w.p(tag + '.saved.img')
    rc = w.run(img, w.game + ['--save-at', str(SAVE_TIC), str(slot), '--nor-out', out,
                              '--hash-out', a], tag + '.a.log')[0]
    rc |= w.run(out, w.game + ['--load', str(slot), '--hash-out', b], tag + '.b.log')[0]
    sa, sb = LS.read_stream(a), LS.read_stream(b)
    os.remove(out)
    return rc == 0 and sb and sb == [x for x in sa if int(x[0]) > SAVE_TIC]


def test_frag(w):
    for n in (2, 8, 9, 16):
        print('frag %d: the free clusters in %d runs' % (n, n))
        img = w.mknor('frag%d.img' % n, '--frag', str(n))
        out_img = w.p('frag%d.out.img' % n)
        rc, out = w.run(img, ['--tics', '2', '--nor-out', out_img])
        vol = Vol(read(img))
        runs, c, longest = [], 2, []
        while c < vol.nclus + 2:
            if not vol.entry(c):
                s = c
                while c < vol.nclus + 2 and not vol.entry(c):
                    c += 1
                runs.append(c - s)
            c += 1
        fits = sum(sorted(runs, reverse=True)[:8]) >= 16
        nor = read(out_img)
        f = Vol(nor).file(b'SAVE    DAT')
        if not fits:
            check(f is None and NOSAVES in out and nor == read(img),
                  'free runs %s: the 8 longest hold fewer than 16 clusters, the saves are off and '
                  'the image is as it was' % sorted(runs, reverse=True))
            continue
        clean, lines = fsck(w, nor)
        good, msg = dmccheck(w, f[0]) if f else (False, [])
        check(f is not None and len(f[0]) == 65536 and f[1] <= 8 and clean and NOSAVES not in out
              and Vol(nor).fats[0] == Vol(nor).fats[1] and good,
              'free runs %s: SAVE.DAT in %s extents, fsck.fat %s, dmccheck %s' % (
                  sorted(runs, reverse=True), f[1] if f else None, 'clean' if clean else lines, msg))
        check(round_trip(w, out_img, 2, 'frag%d' % n), 'a save into slot 2 and its load')
        os.remove(out_img)


def test_full(w):
    print('full: 15 free clusters')
    code = write(w.p('code.bin'), bytes(6 * 4096))
    img = w.mknor('full.img', '--code-main', code)
    out_img = w.p('full.out.img')
    rc, out = w.run(img, ['--tics', '2', '--nor-out', out_img])
    check(Vol(read(img)).free() == 15 and NOSAVES in out and read(out_img) == read(img),
          'the saves are off, the image is as it was')


def test_cutsave(w, base):
    print('cutsave: the power goes during a save into slot 1')
    a = w.p('cut.a.txt')
    w.run(base, w.game + ['--hash-out', a], 'cut.a.log')
    want = [x for x in LS.read_stream(a) if int(x[0]) > SAVE_TIC]
    n, results = 0, []
    while True:
        cut = w.p('cutsave.img')
        rc, out = w.run(base, w.game + ['--save-at', str(SAVE_TIC), '1', '--nor-fail-after', str(n),
                                        '--nor-out', cut])
        good, msg = dmccheck(w, save_dat(read(cut)))
        b = w.p('cut.b.txt')
        rl, lout = w.run(cut, w.game + ['--load', '1', '--hash-out', b], 'cut.b.log')
        if rl == 0 and LS.read_stream(b) == want:
            state = 'whole'
        elif rl != 0 and 'the slot is empty' in lout:
            state = 'empty'
        else:
            state = 'BROKEN'
        results.append(state)
        if not good or state == 'BROKEN':
            check(False, 'cut after %d erases and programs: %s, dmccheck %s' % (n, state, msg))
        if rc != 3:
            break
        n += 1
    check(results[-1] == 'whole' and all(r == 'empty' for r in results[:-1]),
          'cuts after 0..%d operations leave the slot empty, the whole save loads after %d' % (
              n - 1, n))


def test_cutmake(w, plain_pack):
    print('cutmake: the power goes while SAVE.DAT is made')
    src = read(plain_pack)
    n, window = 0, []
    erases, last_erase = 0, None
    while True:
        cut = w.p('cutmake.img')
        rc, out = w.run(plain_pack, ['--tics', '2', '--nor-fail-after', str(n), '--nor-out', cut])
        nor = read(cut)
        vol = Vol(nor)
        # the first FAT and the root directory, which the meter reads, erased
        fat1 = vol.root - len(vol.fats) * len(vol.fats[0])
        meta = [o for o in range(MAIN + fat1, MAIN + vol.data, 4096)
                if (o - MAIN < fat1 + len(vol.fats[0]) or o - MAIN >= vol.root)
                and nor[o:o + 4096] == b'\xff' * 4096 and src[o:o + 4096] != b'\xff' * 4096]
        clean, lines = fsck(w, nor, 'cutvol.img')
        if rc == 0:
            check(clean and vol.file(b'SAVE    DAT') is not None,
                  'no cut after %d operations: SAVE.DAT made, fsck.fat clean' % n)
            break
        if meta:
            window.append(n)
        else:
            nxt = w.p('cutnext.img')
            _, nout = w.run(cut, ['--tics', '2', '--nor-out', nxt])
            room = vol.free() >= 16
            made = Vol(read(nxt)).file(b'SAVE    DAT') is not None
            check(lost_only(lines) and made == room and (NOSAVES in nout) != room,
                  'cut after %d: fsck.fat %s, %d free clusters, the next start %s' % (
                      n, 'clean' if clean else 'finds lost clusters only' if lost_only(lines)
                      else lines, vol.free(), 'makes SAVE.DAT' if made else 'says the saves are off'))
        n += 1
    check(len(window) <= 2, 'cuts between the erase and the program of the first FAT or the root '
          'directory, which read erased then: after %s operations' % window)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--engine', default=os.path.join(ROOT, 'build', 'host', 'dmcdoom_skip'))
    ap.add_argument('--tag', default='default')
    a = ap.parse_args()
    d = os.path.join(ROOT, 'build', 'savefs', a.tag)
    shutil.rmtree(d, ignore_errors=True)
    w = Work(os.path.abspath(a.engine), d)
    plain = w.mknor('plain.img')
    base = w.p('base.img')
    w.run(plain, w.game + ['--save-at', str(SAVE_TIC), '0', '--nor-out', base])
    if not check(save_dat(read(base)) is not None, 'the base image: SAVE.DAT made, a save in slot 0'):
        sys.exit(1)
    test_header(w, base, plain)
    test_crc(w, base, plain)
    test_badsize(w)
    test_frag(w)
    test_full(w)
    test_cutsave(w, base)
    test_cutmake(w, plain)
    if not failures:
        shutil.rmtree(d, ignore_errors=True)
    print('PASS' if not failures else '%d FAILED, files in %s' % (len(failures), d))
    sys.exit(1 if failures else 0)


if __name__ == '__main__':
    main()
