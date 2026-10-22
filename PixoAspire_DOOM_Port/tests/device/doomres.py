#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Decodes DOOMRES.DAT of the DOOM firmware and checks its demo hashes against the host.

usage: doomres.py DOOMRES.DAT [--host BIN --nor IMG [--lmp-dir DIR] [--work DIR]]
       doomres.py --nor-image NOR.img [same options]
       doomres.py --blank OUT
       doomres.py --self-test

The layout comes from port/game/dmc_res.h (parsed here). The report lists the header, every
demo record (verification pass: the level of its first tic and the plans of the levels it
set up, tics hashed, why it ended, last hash and chain, object store events, peak pages,
timed pass: frames, phases mean/p50/p95/max in ms, the frame time histogram, NOR traffic,
stack, view hashes) and the crash record, of this boot of the firmware or of an earlier one.

With --host (build/host/dmcdoom_skip, the engine with the masked hash of lockstep-skip) and
--nor (the NOR image of the same pack, build/nor/E1.img) every verified demo is played on the
host (-playdemo demo1, or DIR/DEMOn.LMP of --lmp-dir) for as many tics as the meter hashed,
and the chained values (their low words), the last hash and the last chain must be equal.
Exit status 1 on a difference or a crash record of the boot that wrote the results.
--nor-image takes DOOMRES.DAT out of a whole NOR image (the firmware drive at NOR 0, FAT12)
and --blank writes the zeroed file the user copies to the firmware drive.
"""
import argparse
import os
import re
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
FNV_OFFSET, FNV_PRIME = 0xcbf29ce484222325, 0x100000001b3
PHASES = ("tic", "pass1", "strips", "band", "NOR wait", "LCD wait", "total")
ENDS = {1: "end marker", 2: "level exit", 3: "test limit", 4: "stopped (MODE)"}
STATES = {1: "running demos", 2: "demos done", 3: "play mode"}


def defines():
    with open(os.path.join(ROOT, "port", "game", "dmc_res.h")) as f:
        return {m.group(1): int(m.group(2), 0) for m in
                re.finditer(r"^#define\s+([A-Z][A-Z0-9_]*)\s+(0x[0-9A-Fa-f]+|\d+)u?\b", f.read(), re.M)}


D = defines()


def feed(h, v):
    for i in range(4):
        h ^= (v >> (8 * i)) & 0xFF
        h = (h * FNV_PRIME) & 0xFFFFFFFFFFFFFFFF
    return h


def chain_of(hashes):
    """Chained values of a hash stream [(gametic, hash)]: stored ones (their low words as the
    firmware keeps them), last hash, last c."""
    c, stored, last = FNV_OFFSET, [], 0
    for tic, h in hashes:
        c = feed(feed(c, h & 0xFFFFFFFF), h >> 32)
        last = h
        if (tic + 1) % D["RES_CHAIN_TICS"] == 0:
            stored.append(c & 0xFFFFFFFF)
    return stored, last, c


def words(data, off, n):
    return list(struct.unpack_from(f"<{n}I", data, off))


def u64(w, i):
    return w[i] | w[i + 1] << 32


def decode(data):
    if len(data) < D["RES_SIZE"]:
        raise ValueError(f"{len(data)} bytes, DOOMRES.DAT has {D['RES_SIZE']}")
    h = words(data, D["RES_HDR_OFF"], D["H_WORDS"])
    out = {"header": h, "demos": [], "crash": None}
    c = words(data, D["RES_CRASH_OFF"], D["C_WORDS"])
    if c[D["C_MAGIC"]] == D["RES_CRASH_MAGIC"]:
        msg = struct.pack("<16I", *c[D["C_MSG"]:D["C_MSG"] + 16]).split(b"\0")[0].decode("latin-1")
        out["crash"] = (c, msg)
    if h[D["H_MAGIC"]] != D["RES_MAGIC"] or h[D["H_VERSION"]] != D["RES_VERSION"]:
        return out
    for k in range(D["RES_DEMOS"]):
        r = words(data, D["RES_DEMO_OFF"] + k * D["RES_DEMO_BYTES"], D["D_WORDS"])
        if not r[D["D_STATUS"]]:
            continue
        name = struct.pack("<3I", *r[D["D_NAME"]:D["D_NAME"] + 3]).split(b"\0")[0].decode("latin-1")
        first, n = r[D["D_V_FIRST"]], r[D["D_V_NCHAIN"]]
        n = max(0, min(n, D["RES_CHAIN_MAX"] - first))
        chain = list(struct.unpack_from(f"<{n}I", data, D["RES_CHAIN_OFF"] + 4 * first))
        hist = struct.unpack_from("<102H", data, D["RES_DEMO_OFF"] + k * D["RES_DEMO_BYTES"] + 4 * D["D_T_HIST"])
        view = [(r[D["D_T_VIEW"] + 3 * i], r[D["D_T_VIEW"] + 3 * i + 1] | r[D["D_T_VIEW"] + 3 * i + 2] << 32)
                for i in range(D["DMC_VIEW_FRAMES"])]
        out["demos"].append({"k": k, "name": name, "r": r, "chain": chain, "hist": hist[:101], "view": view})
    return out


def plans(lv):
    """The plans (letters) of the levels a verification pass set up, from its level word."""
    return "".join("ABCD"[p] for p in range(4) if lv >> (28 + p) & 1)


def crash_now(res):
    """The crash record belongs to the boot that wrote the results."""
    return bool(res["crash"]) and res["crash"][0][D["C_BOOT"]] == res["header"][D["H_BOOT"]]


def report(res):
    h = res["header"]
    if h[D["H_MAGIC"]] != D["RES_MAGIC"]:
        print("DOOMRES.DAT: no results yet (blank file)")
        return
    bid = struct.pack("<5I", *h[D["H_BUILD"]:D["H_BUILD"] + 5]).hex()
    print(f"DOOMRES.DAT v{h[D['H_VERSION']]}: {STATES.get(h[D['H_STATE']], h[D['H_STATE']])}, "
          f"{h[D['H_NDEMOS']]} demos, {h[D['H_SAVES']]} saves, uptime {h[D['H_UPTIME']] / 1000:.1f} s, "
          f"run {h[D['H_RUNS']]} of boot {h[D['H_BOOT']]}")
    print(f"  firmware build {bid} (variant {h[D['H_VARIANT']]}), fixed core {h[D['H_CORE']]} B, "
          f"FRAME_BASE {h[D['H_FRAME_BASE']]:#x}, RAM end {h[D['H_RAM_END']]:#x}, "
          f"stack high-water {h[D['H_STACK']]} B")
    if h[D["H_TEST_TICS"]] or h[D["H_TEST_FRAMES"]]:
        print(f"  test limits: {h[D['H_TEST_TICS']]} tics, {h[D['H_TEST_FRAMES']]} frames per pass")
    for d in res["demos"]:
        r = d["r"]
        lv = r[D["D_LEVEL"]]
        if r[D["D_STATUS"]] & D["DS_NODEMO"]:
            print(f"{d['name']}: not played (not a single player v1.9 demo of a map of the pack)")
            continue
        print(f"{d['name']}: E{lv & 255}M{(lv >> 8) & 255} skill {((lv >> 16) & 255) + 1} "
              f"plan {'ABCD'[(lv >> 24) & 3]}, levels set up in plans {plans(lv) or '-'}, "
              f"status {r[D['D_STATUS']]:#x}")
        print(f"  verification: {r[D['D_V_TICS']]} tics ({ENDS.get(r[D['D_V_END']], '?')}) in "
              f"{r[D['D_V_MS']]} ms, last hash {u64(r, D['D_V_HASH']):016x}, "
              f"chain {u64(r, D['D_V_CHAIN']):016x} ({r[D['D_V_NCHAIN']]} stored from slot "
              f"{r[D['D_V_FIRST']]})")
        ev = r[D["D_V_EVENTS"]:D["D_V_EVENTS"] + 5]
        pg = r[D["D_V_PAGES"]]
        print(f"  object store: events {ev}, pages {pg & 0xFFFF}, peak in use {pg >> 16}")
        ram = r[D["D_V_RAM"]]
        print(f"  RAM of the plan: frame region {ram & 0xFFFF} B, caches {ram >> 16} B, "
              f"level arena {r[D['D_V_ARENA']]} B")
        fr = r[D["D_T_FRAMES"]]
        if not fr:
            continue
        timed = max(fr - D["DMC_VIEW_FRAMES"], 0)
        ms = r[D["D_T_MS"]]
        print(f"  timed: {fr} frames ({timed} timed) in {ms} ms, stack {r[D['D_T_STACK']]} B, "
              f"peak pages {r[D['D_T_PAGES']]}")
        n = r[D["D_T_NOR"]:D["D_T_NOR"] + 4]
        print(f"  NOR: {n[0]} sync reads, {n[1]} queued reads, {n[2]} bytes, {n[3]} DMA chunks; "
              f"LCD pump interrupts {r[D['D_T_LCD']]}, restarts {r[D['D_T_LCD'] + 1]}")
        print("  phase       mean     p50     p95     max  (ms)")
        for p, name in enumerate(PHASES):
            w = r[D["D_T_PHASE"] + 4 * p:D["D_T_PHASE"] + 4 * p + 4]
            print(f"  {name:9s} {w[0] / 1000:7.2f} {w[1] / 1000:7.2f} {w[2] / 1000:7.2f} {w[3] / 1000:7.2f}")
        mean = r[D["D_T_PHASE"] + 4 * D["PH_TOTAL"]]
        if mean:
            print(f"  mean {1e6 / mean:.1f} fps")
        bins = [(i, c) for i, c in enumerate(d["hist"]) if c]
        print("  frame time histogram (ms: frames): " +
              ", ".join(f"{'100+' if i == 100 else i}: {c}" for i, c in bins))
        print("  view hashes: " + ", ".join(f"tic {t}: {v:016x}" for t, v in d["view"] if t))
    if res["crash"]:
        c, msg = res["crash"]
        print("CRASH of this boot" if crash_now(res) else f"crash of an earlier boot ({c[D['C_BOOT']]})",
              end=" ")
        print(f"at {c[D['C_UPTIME']] / 1000:.1f} s: exception {c[D['C_EXC']]}, PC {c[D['C_PC']]:#010x}, "
              f"LR {c[D['C_LR']]:#010x}, CFSR {c[D['C_CFSR']]:#x}, HFSR {c[D['C_HFSR']]:#x}, "
              f"BFAR {c[D['C_BFAR']]:#x}, state {c[D['C_STATE']]} demo {c[D['C_DEMO']]} "
              f"gametic {c[D['C_GAMETIC']]}, stack {c[D['C_STACK']]} B, message \"{msg}\"")


def host_stream(host, nor, demo, tics, work, extra=()):
    out = os.path.join(work, f"host_{os.path.basename(demo).replace('.', '_')}.txt")
    subprocess.run([os.path.abspath(host), "--nor", os.path.abspath(nor), "-playdemo", demo,
                    "--hash-out", out, "--tics", str(tics)] + list(extra), check=False,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, cwd=work)
    with open(out) as f:
        lines = [l.split() for l in f if l.strip()]
    os.remove(out)
    return [(int(t), int(h, 16)) for t, h in lines]


def compare(res, host, nor, lmp_dir, work, extra=()):
    """Every verified demo against the host engine, extra: more host arguments (the forced
    exits of the test settings)."""
    ok = True
    for d in res["demos"]:
        r = d["r"]
        tics = r[D["D_V_TICS"]]
        if r[D["D_STATUS"]] & D["DS_NODEMO"]:
            continue
        if not tics:
            print(f"{d['name']}: nothing verified")
            ok = False
            continue
        demo = "demo1" if d["name"] == "DEMO1" else os.path.join(os.path.abspath(lmp_dir or "."), d["name"])
        stream = host_stream(host, nor, demo, tics, work, extra)
        if len(stream) < tics:
            print(f"{d['name']}: the host played only {len(stream)} of {tics} tics")
            ok = False
            continue
        stored, last, c = chain_of(stream[:tics])
        n = len(d["chain"])
        bad = [i for i in range(n) if i >= len(stored) or d["chain"][i] != stored[i]]
        same = not bad and last == u64(r, D["D_V_HASH"]) and c == u64(r, D["D_V_CHAIN"])
        print(f"{d['name']}: {tics} tics, {n} chained values: "
              + ("equal to the host" if same else
                 f"DIFFER (first chained value at tic {(bad[0] + 1) * D['RES_CHAIN_TICS'] - 1})" if bad
                 else "DIFFER in the last hash or chain"))
        ok &= same
    if crash_now(res):
        ok = False
    return ok


def from_nor(path):
    """DOOMRES.DAT out of a NOR image: root directory of the firmware drive, its clusters."""
    with open(path, "rb") as f:
        nor = f.read()
    bps, spc, rsv, nfat, nroot = struct.unpack_from("<HBHBH", nor, 11)
    fatsz = struct.unpack_from("<H", nor, 22)[0]
    root = (rsv + nfat * fatsz) * bps
    data = root + nroot * 32
    fat = nor[rsv * bps:(rsv + fatsz) * bps]
    for i in range(nroot):
        e = nor[root + 32 * i:root + 32 * i + 32]
        if e[:11] == b"DOOMRES DAT":
            c, size, out = struct.unpack_from("<H", e, 26)[0], struct.unpack_from("<I", e, 28)[0], b""
            while 2 <= c < 0xFF8 and len(out) < size:
                o = data + (c - 2) * spc * bps
                out += nor[o:o + spc * bps]
                v = struct.unpack_from("<H", fat, c * 3 // 2)[0]
                c = v >> 4 if c & 1 else v & 0xFFF
            return out[:size]
    raise ValueError(f"{path}: no DOOMRES.DAT on the firmware drive")


def self_test():
    ok = D["RES_HDR_OFF"] + D["H_WORDS"] * 4 <= D["RES_DEMO_OFF"] and D["D_WORDS"] * 4 <= D["RES_DEMO_BYTES"]
    ok &= D["RES_DEMO_OFF"] + D["RES_DEMOS"] * D["RES_DEMO_BYTES"] <= D["RES_CRASH_OFF"]
    ok &= D["RES_CRASH_OFF"] + 4 * D["C_WORDS"] <= D["RES_CHAIN_OFF"]
    ok &= D["RES_CHAIN_OFF"] + 4 * D["RES_CHAIN_MAX"] <= D["RES_SIZE"] == D["RES_SECTOR"]
    ok &= D["D_T_HIST"] + 51 <= D["D_T_VIEW"] and D["D_T_PHASE"] + 4 * D["PH_COUNT"] <= D["D_T_HIST"]
    ok &= D["D_T_VIEW"] + 3 * D["DMC_VIEW_FRAMES"] <= D["D_WORDS"]
    # the chain of LS_Feed over two tics, computed byte by byte
    stored, last, c = chain_of([(t, 0x0123456789ABCDEF + t) for t in range(70)])
    ok &= len(stored) == 2 and stored[1] == c & 0xFFFFFFFF and last == 0x0123456789ABCDEF + 69
    h = FNV_OFFSET
    for t in range(70):
        for b in struct.pack("<Q", 0x0123456789ABCDEF + t):
            h = ((h ^ b) * FNV_PRIME) & 0xFFFFFFFFFFFFFFFF
    ok &= h == c
    blank = decode(bytes(D["RES_SIZE"]))
    ok &= blank["demos"] == [] and blank["crash"] is None
    ok &= plans(0x3 << 24 | 1) == "" and plans(0xB << 28 | 1 << 24) == "ABD"
    print("doomres self-test", "ok" if ok else "FAILED")
    return 0 if ok else 1


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("file", nargs="?")
    ap.add_argument("--nor-image")
    ap.add_argument("--host")
    ap.add_argument("--nor")
    ap.add_argument("--lmp-dir")
    ap.add_argument("--work", default=".")
    ap.add_argument("--blank")
    ap.add_argument("--self-test", action="store_true")
    a = ap.parse_args()
    if a.self_test:
        return self_test()
    if a.blank:
        with open(a.blank, "wb") as f:
            f.write(bytes(D["RES_SIZE"]))
        return 0
    if a.nor_image:
        data = from_nor(a.nor_image)
    elif a.file:
        with open(a.file, "rb") as f:
            data = f.read()
    else:
        ap.error("give DOOMRES.DAT or --nor-image")
    res = decode(data)
    report(res)
    if a.host:
        if not a.nor:
            ap.error("--host needs --nor")
        os.makedirs(a.work, exist_ok=True)
        return 0 if compare(res, a.host, a.nor, a.lmp_dir, a.work) else 1
    return 1 if crash_now(res) else 0


if __name__ == "__main__":
    sys.exit(main())
