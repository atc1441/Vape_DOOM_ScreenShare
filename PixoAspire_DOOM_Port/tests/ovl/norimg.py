#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Writes the 4 MB NOR image of the overlay tests.

The three packs from make go to the DEMO_NOR_* offsets of tests/ovl/ovl_demo.h, followed by
changed copies of the good pack: a flipped code byte of overlay A (blob CRC), a reserved
header byte (header CRC), another build ID, a bss of A just over its window and a bss of
DEMO_BSS_SIZE for A (the last three with the CRCs fixed), and an erased area.
port/ovl/ovl_fixed.c reads the file at ovl_fixed_base, the tests move it between these
offsets. For test_ext.c the good pack is also stored as a fragmented file in 128 B clusters
(extents only) and as DOOMCODE.DMC on a small FAT12 volume. The rest of the image is 0xFF.

usage: norimg.py --good DMC --lc DMC --noent DMC -o NOR.img [--args FILE]
--args writes the host test's --build-id and --vma options for this image.
"""
import argparse
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "tools"))
sys.dont_write_bytecode = True
import pack_overlays as po

NOR_SIZE = 0x400000


def defines(path):
    with open(path) as f:
        return {m.group(1): int(m.group(2), 0) for m in
                re.finditer(r"^#define\s+(\w+)\s+(0x[0-9A-Fa-f]+|\d+)u?\b", f.read(), re.M)}


def fix_header_crc(d):
    count = struct.unpack_from("<H", d, 10)[0]
    end = po.HDR_SIZE + po.ENT_SIZE * count
    struct.pack_into("<I", d, 36, 0)
    struct.pack_into("<I", d, 36, po.crc32(bytes(d[po.HDR_SIZE:end]), po.crc32(bytes(d[:po.HDR_SIZE]))))


def entry(data, oid):
    for i, e in enumerate(po.parse_file(data)["entries"]):
        if e["id"] == oid:
            return i, e
    sys.exit(f"overlay {oid} not in the pack")


def damaged(good, kind, oid, window, bss=0):
    d = bytearray(good)
    i, e = entry(good, oid)
    if kind == "crc":
        d[e["off"] + 16 + 4 * e["nent"] + 2] ^= 0xFF
    elif kind == "hdr":
        d[50] ^= 0x01
    elif kind == "build":
        d[12] ^= 0xFF
        fix_header_crc(d)
    elif kind in ("blob", "bss"):
        struct.pack_into("<I", d, e["off"] + 12, window - e["size"] + 4 if kind == "blob" else bss)
        blob = bytes(d[e["off"]:e["off"] + e["size"]])
        struct.pack_into("<I", d, po.HDR_SIZE + po.ENT_SIZE * i + 12, po.crc32(blob))
        fix_header_crc(d)
    return bytes(d)


def fat_volume(name, data):
    """FAT12 volume of 64 sectors of 512 B (1 per cluster, 2 FATs, 16 root entries) with an
    LFN entry, a deleted entry and APP.BIN in front of name. A file of more than one cluster
    runs backwards with gaps, so that every cluster is an extent of its own."""
    sec, nroot, total = 512, 16, 64
    data_sec = 1 + 2 + nroot * 32 // sec
    vol = bytearray(total * sec)
    vol[0:3] = b"\xEB\x3C\x90"
    vol[3:11] = b"DMCDOOM "
    struct.pack_into("<HBHBHHBHHHII", vol, 11, sec, 1, 1, 2, nroot, total, 0xF8, 1, 32, 2, 0, 0)
    vol[510:512] = b"\x55\xAA"
    n = (len(data) + sec - 1) // sec
    assert 1 <= n <= 4, "DOOMCODE.DMC fits in at most 4 clusters of 512 B, one extent each"
    chain = [4 + 2 * (n - 1 - k) for k in range(n)]
    fat = {2: 0xFFF}
    for k, c in enumerate(chain):
        fat[c] = chain[k + 1] if k + 1 < n else 0xFFF
        at = (data_sec + c - 2) * sec
        vol[at:at + sec] = data[k * sec:(k + 1) * sec].ljust(sec, b"\0")
    tab = bytearray(sec)
    for c, v in {0: 0xFF8, 1: 0xFFF, **fat}.items():
        o = c * 3 // 2
        if c & 1:
            tab[o] = (tab[o] & 0x0F) | (v << 4 & 0xF0)
            tab[o + 1] = v >> 4 & 0xFF
        else:
            tab[o] = v & 0xFF
            tab[o + 1] = (tab[o + 1] & 0xF0) | v >> 8
    vol[sec:2 * sec] = vol[2 * sec:3 * sec] = tab
    lfn = bytearray(32)
    lfn[0], lfn[11] = 0x41, 0x0F
    ent = lambda nm, first, size: struct.pack("<11sB14xHI", nm, 0x20, first, size)
    root = lfn + ent(b"\xE5" + name[1:], chain[0], len(data)) + ent(b"APP     BIN", 2, 100) \
        + ent(name, chain[0], len(data))
    vol[3 * sec:3 * sec + len(root)] = root
    return bytes(vol)


def image(good, lc, noent):
    D = defines(os.path.join(HERE, "ovl_demo.h"))
    O = defines(os.path.join(ROOT, "port", "ovl", "ovl.h"))
    oid, win = O["OVL_TEST_A"], O["OVL_WIN_FRAME"]
    parts = {"GOOD": good, "GOOD_LC": lc, "NOENT": noent,
             "CRC": damaged(good, "crc", oid, win), "HDR": damaged(good, "hdr", oid, win),
             "BUILD": damaged(good, "build", oid, win), "BLOB": damaged(good, "blob", oid, win),
             "BSS": damaged(good, "bss", oid, win, D["DEMO_BSS_SIZE"]),
             "FAT": fat_volume(b"DOOMCODEDMC", good)}
    cl = 1 << D["DEMO_FRAG_SHIFT"]
    assert len(good) <= 4 * cl, "the fragmented copy has at most 4 extents"
    used = sorted([(D["DEMO_NOR_" + n], len(d), n) for n, d in parts.items()]
                  + [(D["DEMO_NOR_EMPTY"], 1, "EMPTY"), (D["DEMO_NOR_FRAG"], (D["DEMO_FRAG_PCL0"] - 1) * cl, "FRAG")])
    for (at, n, name), nxt in zip(used, used[1:] + [(NOR_SIZE, 0, "")]):
        assert at + n <= nxt[0], f"{name} runs into {nxt[2]}"
    nor = bytearray(b"\xff" * NOR_SIZE)
    for name, data in parts.items():
        at = D["DEMO_NOR_" + name]
        nor[at:at + len(data)] = data
    for k in range(0, len(good), cl):
        at = D["DEMO_NOR_FRAG"] + (D["DEMO_FRAG_PCL0"] - D["DEMO_FRAG_STEP"] * (k // cl) - 2) * cl
        nor[at:at + cl] = good[k:k + cl].ljust(cl, b"\xff")
    return bytes(nor)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--good", required=True)
    ap.add_argument("--lc", required=True)
    ap.add_argument("--noent", required=True)
    ap.add_argument("-o", "--out", required=True)
    ap.add_argument("--args")
    a = ap.parse_args()
    packs = []
    for p in (a.good, a.lc, a.noent):
        with open(p, "rb") as f:
            packs.append(f.read())
    with open(a.out, "wb") as f:
        f.write(image(*packs))
    if a.args:
        h = po.parse_file(packs[0])
        with open(a.args, "w") as f:
            f.write(f"--build-id {h['build_id'].hex()} --vma {h['entries'][0]['vma']:x}\n")


if __name__ == "__main__":
    main()
