#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Tests of tools/pack_overlays.py on the overlay demo build (make ovl-test).

Checks the build ID against the objcopy image, re-stamping, the DOOMCODE.DMC layout against
the overlay ELFs, determinism, the MOVW/MOVT decoding of the call check, and that the packer
or the overlay link refuses: an unstamped firmware, an overlay over its window, an overlay
over FRAME_MIN, an overlay with its own .data and .bss, an overlay calling the loader
directly, through resident code, through an alias or through an address loaded with
MOVW/MOVT, an overlay reaching a --forbid function, an overlay using another overlay's
symbol, duplicate ids, a firmware with an object in the frame region, an overlay linked
against another firmware, an overlay linked without relocations and damaged or truncated
files. An overlay using __frame_start is accepted.

usage: test_pack.py --build build/ovl --ld "ld.lld -T port/ovl/ovl.ld ..."
"""
import argparse
import hashlib
import os
import shlex
import shutil
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "tools"))
sys.path.insert(0, HERE)
sys.dont_write_bytecode = True
import norimg
import pack_overlays as po

PACKER = [sys.executable, "-I", os.path.join(ROOT, "tools", "pack_overlays.py")]
failures = []


def check(cond, msg):
    print(("  ok   " if cond else "  FAIL ") + msg)
    if not cond:
        failures.append(msg)


def run(cmd):
    p = subprocess.run(cmd, capture_output=True, text=True, cwd=ROOT)
    return p.returncode, p.stdout + p.stderr


def refused(what, cmd, needle):
    rc, out = run(cmd)
    first = next((l for l in out.splitlines() if needle in l), out.strip().splitlines()[0] if out.strip() else "")
    check(rc != 0 and needle in out and "Traceback" not in out, f"{what}: {first.strip()}")
    return out


def write(path, data):
    with open(path, "wb") as f:
        f.write(data)


def read(path):
    with open(path, "rb") as f:
        return f.read()


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--build", required=True)
    ap.add_argument("--ld", required=True)
    a = ap.parse_args()
    b = os.path.abspath(a.build)
    d, neg = os.path.join(b, "demo"), os.path.join(b, "neg")
    tmp = os.path.join(b, "tmp-pack")
    os.makedirs(tmp, exist_ok=True)
    ld = shlex.split(a.ld)
    main_elf, unstamped = os.path.join(d, "main.elf"), os.path.join(d, "main.unstamped.elf")
    m = po.Elf(main_elf)

    print("build ID")
    bid, cur = po.build_id(m)
    check(cur == bid, f"main.elf carries its build ID {bid.hex()}")
    img = bytearray(read(os.path.join(d, "OVLDEMO.BIN")))
    at = m.sym("dmc_build_id")["value"] - 0x08007000
    check(img[at:at + 20] == bid, "OVLDEMO.BIN (llvm-objcopy) carries the same 20 bytes")
    img[at:at + 20] = bytes(20)
    check(hashlib.sha1(img).digest() == bid, "build ID = SHA-1 of OVLDEMO.BIN with the field zeroed")
    check(po.build_id(po.Elf(unstamped))[1] == po.PLACEHOLDER, "unstamped link holds the placeholder")
    again = os.path.join(tmp, "again.elf")
    rc, _ = run(PACKER + ["stamp", main_elf, again])
    check(rc == 0 and read(again) == read(main_elf), "stamping a stamped firmware changes nothing")
    bad = bytearray(read(unstamped))
    off = m.file_offset(m.sym("dmc_build_id")["value"], 20)
    bad[off:off + 20] = b"X" * 20
    with open(os.path.join(tmp, "bad.elf"), "wb") as f:
        f.write(bad)
    refused("stamp refuses a firmware with an unknown ID field",
            PACKER + ["stamp", os.path.join(tmp, "bad.elf"), again], "neither the placeholder")

    print("DOOMCODE.DMC layout")
    base = m.sym("__frame_start")["value"]
    for name, parts in (("DOOMCODE.DMC", ("a", "b")), ("DOOMCODE_LC.DMC", ("a_lc", "b_lc")),
                        ("NOENT.DMC", ("b",))):
        path = os.path.join(d, name)
        data = read(path)
        h = po.parse_file(data)
        ok = h["crc_ok"] and h["size_ok"] and h["build_id"] == bid and len(h["entries"]) == len(parts)
        for e, p in zip(h["entries"], parts):
            o = po.Elf(os.path.join(d, p + ".elf"))
            s, bs = o.section(".ovl"), o.section(".ovl_bss")
            blob = o.data[s["off"]:s["off"] + s["size"]]
            nent = (o.sym("__ovl_entries_end")["value"] - base - 16) // 4
            ok = ok and e["id"] == o.sym("__ovl_id")["value"] and e["off"] % 16 == 0
            ok = ok and data[e["off"]:e["off"] + e["size"]] == blob and e["crc_ok"]
            ok = ok and e["vma"] == base and e["copy"] == e["size"] and e["nent"] == nent
            ok = ok and e["bss"] == 0 and not (bs and bs["size"])
        ids = [e["id"] for e in h["entries"]]
        check(ok and ids == sorted(ids),
              f"{name}: {len(data)} B, header and blob CRCs, blobs equal the ELF sections, vma {base:#x}, no bss")
        rc, _ = run(PACKER + ["info", path])
        check(rc == 0, f"{name}: pack_overlays.py info verifies it")
    redo = os.path.join(tmp, "redo.dmc")
    rc, out = run(PACKER + ["pack", "--report", "--main", main_elf, "-o", redo,
                            os.path.join(d, "a.elf"), os.path.join(d, "b.elf")])
    check(rc == 0 and read(redo) == read(os.path.join(d, "DOOMCODE.DMC")), "packing again gives the same bytes")
    print("\n".join("    " + l for l in out.strip().splitlines()))
    rc, out = run(PACKER + ["pack", "--report", "--main", main_elf, "-o", redo,
                            os.path.join(d, "a_lc.elf"), os.path.join(d, "b_lc.elf")])
    print("\n".join("    " + l for l in out.strip().splitlines()[1:]))

    print("refusals")
    good = read(os.path.join(d, "DOOMCODE.DMC"))
    for kind in ("crc", "hdr", "build", "blob"):
        path = os.path.join(tmp, kind + ".dmc")
        with open(path, "wb") as f:
            f.write(norimg.damaged(good, kind, 6, 8368))
        rc, _ = run(PACKER + ["info", path])
        if kind in ("crc", "hdr"):
            check(rc == 1, f"info reports the damaged {kind} copy")
    for n, needle in ((100, "runs past the end of the file (WRONG)"),
                      (70, "directory of 2 entries runs past the end")):
        path = os.path.join(tmp, "short.dmc")
        write(path, good[:n])
        rc, msg = run(PACKER + ["info", path])
        line = next((l for l in msg.splitlines() if needle in l), msg.strip())
        check(rc == 1 and needle in msg and "Traceback" not in msg, f"info on the first {n} B: {line.strip()}")
    a_elf, b_elf = os.path.join(d, "a.elf"), os.path.join(d, "b.elf")
    out = os.path.join(tmp, "x.dmc")
    refused("unstamped firmware", PACKER + ["pack", "--main", unstamped, "-o", out, a_elf, b_elf],
            "not stamped")
    refused("automap overlay over its window", PACKER + ["pack", "--main", main_elf, "-o", out,
            os.path.join(neg, "neg_big.elf")], "over its window of 5312 B")
    refused("overlay calling the loader", PACKER + ["pack", "--main", main_elf, "-o", out,
            os.path.join(neg, "neg_loader.elf")], "loader symbol ovl_load")
    refused("overlay calling resident code that calls the loader", PACKER + ["pack", "--main",
            main_elf, "-o", out, os.path.join(neg, "neg_indirect.elf")], "calls demo_calls -> ovl_")
    refused("overlay calling an alias of resident code that calls ovl_invalidate", PACKER + ["pack",
            "--main", main_elf, "-o", out, os.path.join(neg, "neg_frame.elf")],
            "calls demo_drop_frame -> ovl_invalidate")
    refused("overlay calling resident code that stores a loader caller with MOVW/MOVT", PACKER +
            ["pack", "--main", main_elf, "-o", out, os.path.join(neg, "neg_movt.elf")],
            "calls demo_defer -> demo_later_load -> ovl_load")
    refused("overlay B reaching a --forbid function", PACKER + ["pack", "--forbid", "demo_twice",
            "--main", main_elf, "-o", out, b_elf], "calls demo_twice")
    refused("--forbid of a function main.elf lacks", PACKER + ["pack", "--forbid", "no_such_fn",
            "--main", main_elf, "-o", out, a_elf], "--forbid no_such_fn: no such function")
    msg = refused("overlay with its own .data and .bss", PACKER + ["pack", "--main", main_elf, "-o",
                  out, os.path.join(neg, "neg_state.elf")], "has its own .data")
    check("neg_calls" in msg and "neg_acc" in msg, "the refusal names neg_calls (.data) and neg_acc (.bss)")
    st = po.Elf(os.path.join(neg, "neg_state.elf"))
    sec, bs = st.section(".ovl"), st.section(".ovl_bss")
    hdr_bss = struct.unpack_from("<I", st.data, sec["off"] + 12)[0]
    gap = bs["addr"] - sec["addr"] - sec["size"]
    check(gap > 0 and hdr_bss == gap + bs["size"],
          f"its blob bss word ({hdr_bss} B) covers .ovl_bss ({bs['size']} B) and the {gap} B gap before it")
    refused("duplicate overlay ids", PACKER + ["pack", "--main", main_elf, "-o", out, a_elf,
            os.path.join(d, "a_lc.elf")], "also used by")
    refused("firmware with an object in the frame region", PACKER + ["pack", "--main",
            os.path.join(neg, "main_alt.elf"), "-o", out, os.path.join(neg, "b_alt.elf")],
            "neg_dyn_buf")
    refused("overlay linked against another firmware", PACKER + ["pack", "--main", main_elf, "-o",
            out, os.path.join(neg, "b_alt.elf")], "relink against")
    check(not os.path.exists(out), "no file is written when a check fails")
    rc, msg = run(PACKER + ["pack", "--main", main_elf, "-o", os.path.join(tmp, "ok.dmc"),
                            os.path.join(neg, "ok_frame.elf")])
    check(rc == 0, "an automap overlay using __frame_start (a linker symbol) is accepted")

    def link(src, flags, needle, what):
        cmd = flags + ["--just-symbols=" + main_elf, os.path.join(neg if src.startswith("neg") else d, src),
                       "-o", os.path.join(tmp, "l.elf")]
        refused(what, cmd, needle)

    link("neg_huge.r.o", ld, "larger than FRAME_MIN", "overlay over FRAME_MIN does not link")
    link("neg_cross.r.o", ld, "undefined symbol: ovl_tab_TEST_A",
         "overlay using a symbol of another overlay does not link")
    plain = [x for x in ld if x not in ("--emit-relocs", "-q")]
    rc, msg = run(plain + ["--just-symbols=" + main_elf, os.path.join(d, "a.r.o"), "-o",
                           os.path.join(tmp, "norel.elf")])
    check(rc == 0, "overlay A links without --emit-relocs")
    refused("overlay linked without relocations", PACKER + ["pack", "--main", main_elf, "-o", out,
            os.path.join(tmp, "norel.elf")], "no relocations")

    print("resident image")
    ws = m.sym("ovl_win")
    win = struct.unpack("<12H", m.read(ws["value"], 24))
    check(win == (0, 8368, 5696, 5696, 5696, 5312, 8368, 8368, 0, 0, 0, 5696), f"ovl_win in main.elf {win}")
    prod = po.Elf(os.path.join(b, "obj", "prod", "port", "ovl", "ovl.o"))
    check(prod.sym("ovl_win")["size"] == 24, "the loader without OVL_DEMO has ids up to SAVEG (ovl_win 24 B)")
    check(not m.undefined(), "main.elf has no undefined symbols")
    n = bad = 0
    for _, off, typ, _, sym in m.relocations(po.SHF_ALLOC | po.SHF_EXECINSTR):
        if typ in (po.R_ARM_THM_MOVW_ABS_NC, po.R_ARM_THM_MOVT_ABS):
            imm = po.thumb_imm(*struct.unpack("<HH", m.read(off, 4)), typ)
            n += 1
            bad += imm != (sym["value"] >> 16 if typ == po.R_ARM_THM_MOVT_ABS else sym["value"] & 0xFFFF)
    check(n >= 2 and not bad, f"{n} MOVW/MOVT immediates in main.elf decode to their symbols")
    g = po.CallGraph(m)
    defer, later = m.sym("demo_defer")["value"] & ~1, m.sym("demo_later_load")["value"] & ~1
    check(later in g.edges.get(defer, ()), "call graph: demo_defer takes the address of demo_later_load")
    check(not g.unknown, "call graph: every relocation in main.elf's code is understood")

    shutil.rmtree(tmp)
    print("ALL PASSED" if not failures else f"{len(failures)} FAILED")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
