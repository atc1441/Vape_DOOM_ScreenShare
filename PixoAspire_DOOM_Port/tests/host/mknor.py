#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Builds a 4 MB NOR image of the meter from a pack.

NOR 0x000000: firmware drive, FAT12, 1 MB (APP.BIN of 25 clusters of zeros, DOOMX.DAT when
              the pack has one, DOOMCODE.DMC with --code)
NOR 0x100000: data drive, FAT12, 3 MB (DOOM.DAT, SAVE.DAT with --save, DOOMCODE.DMC with
              --code-main, the UI_OFF directory with --junk)

Both volumes use the geometry of the meter: 4096 B sectors, 1 sector per
cluster, 1 reserved sector, 2 FATs of 1 sector, 512 root entries.

usage: mknor.py PACK_DIR OUT [--frag N] [--xfrag N] [--code FILE] [--code-main FILE]
                [--save FILE] [--junk] [--sector B --spc N] [--doomx-bad uuid|length]
                [--chain-bad zero|loop]
  --frag N       DOOM.DAT in N cluster runs (gaps: free clusters, then SAVE.DAT)
  --xfrag N      DOOMX.DAT in N runs, interleaved with APP.BIN
  --code F       put F on the firmware drive as DOOMCODE.DMC
  --code-main F  put F on the data drive as DOOMCODE.DMC
  --save F       put F on the data drive as SAVE.DAT (a pack has none, the firmware
                 makes it)
  --junk         LFN entries, a deleted DOOM.DAT entry and the UI_OFF directory
                 in front of the real entries
  --sector B     sector size of both volumes (default 4096), with --spc N
                 sectors per cluster (default 1). The FAT takes as many
                 sectors as it needs.
  --doomx-bad    DOOMX.DAT with another image uuid or a wrong length
  --chain-bad    the FAT chain of DOOM.DAT ends early (a free entry) or loops
                 back to its first cluster
"""
import argparse
import os
import struct
import sys

NROOT = 512
RSV, NFAT = 1, 2
MAIN_BASE, MAIN_BYTES = 0x100000, 0x300000
UPD_BASE, UPD_BYTES = 0, 0x100000
NOR_SIZE = 0x400000


def boot_sector(sector, spc, total, fatsz, serial):
    b = bytearray(sector)
    b[0:3] = b"\xEB\x3C\x90"
    b[3:11] = b"DMCDOOM "
    struct.pack_into("<HBHBHHBHHHII", b, 11, sector, spc, RSV, NFAT, NROOT, total, 0xF8, fatsz,
                     32, 2, 0, 0)
    struct.pack_into("<BBBI11s8s", b, 36, 0x80, 0, 0x29, serial, b"NO NAME    ", b"FAT12   ")
    b[510:512] = b"\x55\xAA"
    return b


def name83(name):
    base, _, ext = name.partition(".")
    return (base.ljust(8) + ext.ljust(3)).encode("ascii")


def dirent(name, attr, first, size):
    return struct.pack("<11sBBBHHHHHHHI", name, attr, 0, 0, 0, 0, 0, 0, 0, 0, first, size)


def lfn_entries(long_name, short):
    """VFAT long name entries for long_name, in directory order."""
    csum = 0
    for c in short:
        csum = (((csum & 1) << 7) + (csum >> 1) + c) & 0xFF
    chars = [ord(c) for c in long_name] + [0]
    while len(chars) % 13:
        chars.append(0xFFFF)
    parts = [chars[i:i + 13] for i in range(0, len(chars), 13)]
    out = []
    for i, p in enumerate(parts):
        seq = i + 1 + (0x40 if i == len(parts) - 1 else 0)
        e = bytearray(32)
        e[0] = seq
        struct.pack_into("<5H", e, 1, *p[0:5])
        e[11], e[12], e[13] = 0x0F, 0, csum
        struct.pack_into("<6H", e, 14, *p[5:11])
        struct.pack_into("<2H", e, 28, *p[11:13])
        out.append(bytes(e))
    return list(reversed(out))


class Volume:
    def __init__(self, nbytes, serial, sector, spc):
        self.sector, self.spc = sector, spc
        self.csize = sector * spc
        sectors = nbytes // sector
        root_sec = NROOT * 32 // sector
        # FAT sectors for the clusters left after the FATs themselves
        self.fatsz = 1
        while True:
            nclus = (sectors - RSV - NFAT * self.fatsz - root_sec) // spc
            if (nclus + 2) * 3 // 2 <= self.fatsz * sector:
                break
            self.fatsz += 1
        assert nclus < 4085, "not a FAT12 volume"
        self.data_sec = RSV + NFAT * self.fatsz + root_sec
        self.nclus = nclus
        self.img = bytearray(sectors * sector)
        self.img[0:sector] = boot_sector(sector, spc, sectors, self.fatsz, serial)
        self.fat = [0] * (self.nclus + 2)
        self.fat[0], self.fat[1] = 0xFF8, 0xFFF
        self.root = []
        self.free = list(range(2, self.nclus + 2))

    def take(self, n):
        if n > len(self.free):
            sys.exit("volume full")
        got, self.free = self.free[:n], self.free[n:]
        return got

    def nclusters(self, data):
        return (len(data) + self.csize - 1) // self.csize

    def put(self, name, data, clusters, attr=0x20):
        """Store data in the given clusters (chain in list order)."""
        need = self.nclusters(data)
        assert len(clusters) == need, (name, len(clusters), need)
        cs = self.csize
        for i, c in enumerate(clusters):
            self.fat[c] = clusters[i + 1] if i + 1 < len(clusters) else 0xFFF
            o = self.data_sec * self.sector + (c - 2) * cs
            chunk = data[i * cs:(i + 1) * cs]
            self.img[o:o + len(chunk)] = chunk
        size = 0 if attr & 0x10 else len(data)
        self.root.append(dirent(name, attr, clusters[0] if clusters else 0, size))

    def finish(self):
        fat = bytearray(self.fatsz * self.sector)
        for n, v in enumerate(self.fat):
            o = n * 3 // 2
            if n & 1:
                fat[o] = (fat[o] & 0x0F) | ((v << 4) & 0xF0)
                fat[o + 1] = (v >> 4) & 0xFF
            else:
                fat[o] = v & 0xFF
                fat[o + 1] = (fat[o + 1] & 0xF0) | (v >> 8)
        for k in range(NFAT):
            o = (RSV + k * self.fatsz) * self.sector
            self.img[o:o + len(fat)] = fat
        root = b"".join(self.root)
        assert len(root) <= NROOT * 32
        o = (RSV + NFAT * self.fatsz) * self.sector
        self.img[o:o + len(root)] = root
        return bytes(self.img)


def runs(total, n):
    """Split total clusters into n runs of nearly equal length."""
    n = max(1, min(n, total))
    q, r = divmod(total, n)
    return [q + (1 if i < r else 0) for i in range(n)]


def place_interleaved(vol, a_len, b_len, nruns):
    """Cluster lists of file A in nruns runs and of file B, B filling the gaps.

    Free clusters fill the gaps first, so B stays in as few runs as possible.
    """
    pieces = runs(a_len, nruns)
    gaps = len(pieces) - 1
    spare = len(vol.free) - a_len - b_len
    a, b = [], []
    use_free = min(gaps, max(spare, 0))
    b_gaps = gaps - use_free
    b_pieces = runs(b_len, b_gaps + 1) if b_gaps else [b_len]
    bi = 0
    for i, p in enumerate(pieces):
        a += vol.take(p)
        if i < gaps:
            if i < use_free:
                vol.take(1)  # left free
            else:
                b += vol.take(b_pieces[bi])
                bi += 1
    while bi < len(b_pieces):
        b += vol.take(b_pieces[bi])
        bi += 1
    return a, b


def build(pack, frag, xfrag, code, code_main, save, junk, sector, spc, doomx_bad, chain_bad):
    rd = lambda n: open(os.path.join(pack, n), "rb").read() if os.path.exists(os.path.join(pack, n)) else None
    doom, doomx = rd("DOOM.DAT"), rd("DOOMX.DAT")
    if doom is None:
        sys.exit("%s: no DOOM.DAT" % pack)
    if doomx is not None and doomx_bad:
        x = bytearray(doomx)
        if doomx_bad == "uuid":
            x[12] ^= 0xFF  # image_uuid[0]
        else:
            struct.pack_into("<I", x, 32, struct.unpack_from("<I", x, 32)[0] + 1)  # length
        doomx = bytes(x)

    main = Volume(MAIN_BYTES, 0x444D4331, sector, spc)
    if junk:
        uidir = bytearray(main.csize)
        c = main.take(1)
        uidir[0:32] = dirent(b".          ", 0x10, c[0], 0)
        uidir[32:64] = dirent(b"..         ", 0x10, 0, 0)
        main.put(b"UI_OFF     ", bytes(uidir), c, attr=0x10)
        main.root += lfn_entries("doom-old.dat", name83("DOOM~1.DAT"))
        main.root.append(dirent(b"\xE5OOM    DAT", 0x20, 2, 12345))
        main.root += lfn_entries("doom.dat", name83("DOOM.DAT"))
    a, b = place_interleaved(main, main.nclusters(doom), main.nclusters(save) if save else 0, frag)
    main.put(name83("DOOM.DAT"), doom, a)
    if chain_bad == "zero":
        main.fat[a[len(a) // 2]] = 0
    elif chain_bad == "loop":
        main.fat[a[-1]] = a[0]
    if save:
        main.put(name83("SAVE.DAT"), save, b)
    if code_main is not None:
        main.put(name83("DOOMCODE.DMC"), code_main, main.take(main.nclusters(code_main)))
    nor = bytearray(b"\xFF" * NOR_SIZE)
    m = main.finish()
    nor[MAIN_BASE:MAIN_BASE + len(m)] = m

    upd = Volume(UPD_BYTES, 0x444D4332, sector, spc)
    app = b"\0" * (25 * 4096)
    if doomx is not None:
        a, b = place_interleaved(upd, upd.nclusters(doomx), upd.nclusters(app), xfrag)
        upd.put(name83("APP.BIN"), app, b)
        upd.put(name83("DOOMX.DAT"), doomx, a)
    else:
        upd.put(name83("APP.BIN"), app, upd.take(upd.nclusters(app)))
    if code is not None:
        upd.put(name83("DOOMCODE.DMC"), code, upd.take(upd.nclusters(code)))
    u = upd.finish()
    nor[UPD_BASE:UPD_BASE + len(u)] = u
    return bytes(nor)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("pack")
    ap.add_argument("out")
    ap.add_argument("--frag", type=int, default=1)
    ap.add_argument("--xfrag", type=int, default=1)
    ap.add_argument("--code")
    ap.add_argument("--code-main")
    ap.add_argument("--save")
    ap.add_argument("--junk", action="store_true")
    ap.add_argument("--sector", type=int, default=4096, choices=(512, 1024, 2048, 4096))
    ap.add_argument("--spc", type=int, default=1, choices=(1, 2, 4, 8))
    ap.add_argument("--doomx-bad", choices=("uuid", "length"))
    ap.add_argument("--chain-bad", choices=("zero", "loop"))
    a = ap.parse_args()
    code = open(a.code, "rb").read() if a.code else None
    code_main = open(a.code_main, "rb").read() if a.code_main else None
    save = open(a.save, "rb").read() if a.save else None
    data = build(a.pack, a.frag, a.xfrag, code, code_main, save, a.junk, a.sector, a.spc, a.doomx_bad,
                 a.chain_bad)
    tmp = a.out + ".tmp"
    os.makedirs(os.path.dirname(os.path.abspath(a.out)), exist_ok=True)
    with open(tmp, "wb") as f:
        f.write(data)
    os.replace(tmp, a.out)


if __name__ == "__main__":
    main()
