#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""NOR images of the meter for the DOOM firmware, and the cluster budget of its drives.

usage: fwnor.py -o NOR.img [--upd FILE]... [--main FILE]... [--ui UI|UI_OFF|none]
       fwnor.py --budget DIR
       fwnor.py --self-test --build BUILD/fwdoom

Image: both FAT12 volumes with the geometry of the meter (tests/host/mknor.py: 4 KB clusters,
2 FATs, 512 root entries), the firmware drive at NOR 0 (249 clusters, 996 KB) and the 3 MB
data drive at 0x100000 (761 clusters). The files go in the given order, each in one run, with
their 8.3 names from the file names. The data drive starts with the folder the bootloader
looks at (--ui, default UI) holding one file of stock art, then its files. The rest of the
NOR is 0xFF. The same arguments give the same image.

--budget: clusters of the device package of make fw-package on both drives, with DOOM.BIN
(at most 25 clusters, it waits there until the bootloader flashes it) and DOOMCODE.DMC (at
most 11) on the firmware drive, SAVE.DAT (16 clusters, the meter makes it) on the data drive
and a reserve of 3 clusters on each drive for the System Volume Information folder Windows
writes. DOOMRES.DAT and DEMO*.LMP are for developers only, users do not copy them: they must
fit when the reserve is not taken.
"""
import argparse
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "host"))
sys.dont_write_bytecode = True
import mknor  # noqa: E402

CLUSTER = 4096
UPD_CLUSTERS, MAIN_CLUSTERS = 249, 761
BIN_CLUSTERS, CODE_CLUSTERS, SAVE_CLUSTERS = 25, 11, 16
SVI_RESERVE = 3
STOCK_ART = bytes((i * 7 + 3) & 0xFF for i in range(3 * CLUSTER + 100))


def name83(path):
    base = os.path.basename(path).upper()
    stem, _, ext = base.partition(".")
    if not 1 <= len(stem) <= 8 or len(ext) > 3:
        sys.exit(f"{path}: not an 8.3 name")
    return mknor.name83(base)


def folder(vol, name, files):
    """A folder in the root holding files (name, data), each in its own clusters."""
    c = vol.take(1)
    d = bytearray(vol.csize)
    d[0:32] = mknor.dirent(b".          ", 0x10, c[0], 0)
    d[32:64] = mknor.dirent(b"..         ", 0x10, 0, 0)
    entries = []
    for i, (n, data) in enumerate(files):
        cl = vol.take(vol.nclusters(data))
        for k, x in enumerate(cl):
            vol.fat[x] = cl[k + 1] if k + 1 < len(cl) else 0xFFF
            o = vol.data_sec * vol.sector + (x - 2) * vol.csize
            vol.img[o:o + vol.csize] = data[k * vol.csize:(k + 1) * vol.csize].ljust(vol.csize, b"\0")
        d[64 + 32 * i:96 + 32 * i] = mknor.dirent(n, 0x20, cl[0], len(data))
    vol.put(name.ljust(11).encode(), bytes(d), c, attr=0x10)


def build(upd, main, ui):
    nor = bytearray(b"\xFF" * mknor.NOR_SIZE)
    m = mknor.Volume(mknor.MAIN_BYTES, 0x444D4331, 4096, 1)
    assert m.nclus == MAIN_CLUSTERS
    if ui != "none":
        folder(m, ui, [(b"LOGO    BIN", STOCK_ART)])
    for p in main:
        data = open(p, "rb").read()
        m.put(name83(p), data, m.take(m.nclusters(data)))
    img = m.finish()
    nor[mknor.MAIN_BASE:mknor.MAIN_BASE + len(img)] = img
    u = mknor.Volume(mknor.UPD_BYTES, 0x444D4332, 4096, 1)
    assert u.nclus == UPD_CLUSTERS
    for p in upd:
        data = open(p, "rb").read()
        u.put(name83(p), data, u.take(u.nclusters(data)))
    img = u.finish()
    nor[mknor.UPD_BASE:mknor.UPD_BASE + len(img)] = img
    return bytes(nor)


def clusters(path):
    return (os.path.getsize(path) + CLUSTER - 1) // CLUSTER


def developer(d, names, free):
    """Prints the developer files of names in d, True when they fit into free clusters."""
    n = 0
    for name in names:
        c = clusters(os.path.join(d, name))
        n += c
        print(f"  {name:13s} {c:4d} for developers only")
    if names:
        print(f"  {'':13s} {n:4d} of the {free} free without the Windows reserve"
              + ("" if n <= free else ", DO NOT FIT"))
    return n <= free


def budget(d):
    ok = True
    upd = [("DOOMX.DAT", None), ("DOOM.BIN", BIN_CLUSTERS), ("DOOMCODE.DMC", CODE_CLUSTERS)]
    print(f"firmware drive ({UPD_CLUSTERS} clusters of 4 KB):")
    n = SVI_RESERVE
    for name, cap in upd:
        p = os.path.join(d, name)
        c = clusters(p) if os.path.exists(p) else 0
        if cap is not None and c > cap:
            print(f"  {name} has {c} clusters, more than {cap}")
            ok = False
        n += c
        print(f"  {name:13s} {c:4d}" + (f" of at most {cap}" if cap else ""))
    print(f"  {'reserve':13s} {SVI_RESERVE:4d} for Windows (System Volume Information)")
    print(f"  {'total':13s} {n:4d}, {UPD_CLUSTERS - n} free")
    ok &= n <= UPD_CLUSTERS
    dev = ["DOOMRES.DAT"] if os.path.exists(os.path.join(d, "DOOMRES.DAT")) else []
    ok &= developer(d, dev, UPD_CLUSTERS - n + SVI_RESERVE)
    print(f"data drive ({MAIN_CLUSTERS} clusters of 4 KB):")
    c = clusters(os.path.join(d, "DOOM.DAT"))
    n = 1 + c + SAVE_CLUSTERS + SVI_RESERVE
    print(f"  {'UI folder':13s} {1:4d}")
    print(f"  {'DOOM.DAT':13s} {c:4d}")
    print(f"  {'SAVE.DAT':13s} {SAVE_CLUSTERS:4d} made by the meter")
    print(f"  {'reserve':13s} {SVI_RESERVE:4d} for Windows (System Volume Information)")
    print(f"  {'total':13s} {n:4d}, {MAIN_CLUSTERS - n} free")
    ok &= n <= MAIN_CLUSTERS
    dev = sorted(f for f in os.listdir(d) if f.startswith("DEMO") and f.endswith(".LMP"))
    ok &= developer(d, dev, MAIN_CLUSTERS - n + SVI_RESERVE)
    print("budget ok" if ok else "OVER BUDGET")
    return 0 if ok else 1


def self_test(b):
    work = os.path.join(b, "fwnor-test")
    os.makedirs(work, exist_ok=True)
    a, c = os.path.join(work, "A.DAT"), os.path.join(work, "C.LMP")
    open(a, "wb").write(bytes(range(256)) * 20)
    open(c, "wb").write(b"\x6d" * 9000)
    x = build([a], [c], "UI")
    y = build([a], [c], "UI")
    ok = x == y
    for base, name, data in ((mknor.UPD_BASE, b"A       DAT", open(a, "rb").read()),
                             (mknor.MAIN_BASE, b"C       LMP", open(c, "rb").read())):
        root = x[base + 3 * 4096:base + 7 * 4096]
        i = root.find(name)
        first = int.from_bytes(root[i + 26:i + 28], "little")
        size = int.from_bytes(root[i + 28:i + 32], "little")
        o = base + 7 * 4096 + (first - 2) * 4096
        ok &= i >= 0 and size == len(data) and x[o:o + size] == data
    ok &= x[mknor.MAIN_BASE + 3 * 4096:mknor.MAIN_BASE + 3 * 4096 + 11] == b"UI         "
    for p in (a, c):
        os.remove(p)
    os.rmdir(work)
    print("fwnor self-test", "ok" if ok else "FAILED")
    return 0 if ok else 1


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("-o", "--out")
    ap.add_argument("--upd", action="append", default=[])
    ap.add_argument("--main", action="append", default=[])
    ap.add_argument("--ui", default="UI", choices=("UI", "UI_OFF", "none"))
    ap.add_argument("--budget")
    ap.add_argument("--self-test", action="store_true")
    ap.add_argument("--build")
    a = ap.parse_args()
    if a.self_test:
        return self_test(a.build)
    if a.budget:
        return budget(a.budget)
    if not a.out:
        ap.error("-o is required")
    data = build(a.upd, a.main, a.ui)
    with open(a.out + ".tmp", "wb") as f:
        f.write(data)
    os.replace(a.out + ".tmp", a.out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
