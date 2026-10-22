# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Exhaustive host test of the compressed tables and of FixedMulI (port/math/fixed.h), plus
the ARM build of port/math and gen/ (warning free, no undefined helpers, flash sizes).
The decoders are tested as built and with the compact sine table (DMC_SINE_L1).

usage: python3 -I tests/conv/test_trig.py [--zip FREEDOOM_ZIP]   (after make gen)
"""

import argparse
import os
import re
import struct
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, REPO)

from tools.conv import fetch, upstream  # noqa: E402
from tools.conv.util import UPSTREAM, ConvError, write_if_changed  # noqa: E402

OUT = os.path.join(REPO, "build", "conv", "test")
GEN = os.path.join(REPO, "gen")
MATH = os.path.join(REPO, "port", "math")
CC = os.environ.get("HOSTCC", "clang")
ARMCC = os.environ.get("CC", "clang")
ARMFLAGS = ["--target=thumbv7em-none-eabi", "-mcpu=cortex-m4", "-mthumb", "-mfloat-abi=soft", "-Oz",
            "-ffunction-sections", "-fdata-sections", "-ffreestanding", "-fwrapv", "-fsigned-char"]
WARN = ["-std=c11", "-Wall", "-Wextra", "-Werror", "-Wconversion", "-Wno-sign-conversion"]


def run(cmd):
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode:
        sys.stdout.write(r.stdout + r.stderr)
        raise SystemExit("FAIL: %s" % " ".join(cmd))
    return r.stdout


def arr(ctype, name, vals):
    return "const %s %s[%d] = {%s};\n" % (ctype, name, len(vals), ",".join(str(v) for v in vals))


def ref_source(up, colormap):
    v = {x["detail"]: x for x in up["view"]}
    s = "#include <stdint.h>\n"
    s += arr("int", "ref_finesine", up["finesine"])
    s += arr("int", "ref_finetangent", up["finetangent"])
    s += arr("uint32_t", "ref_tantoangle", ["%du" % (t & 0xFFFFFFFF) for t in up["tantoangle"]])
    s += "const int ref_vatx[2][4096] = {{%s},{%s}};\n" % tuple(
        ",".join(map(str, v[d]["viewangletox"])) for d in (0, 1))
    s += "const uint32_t ref_xtva[2][321] = {{%s},{%s}};\n" % tuple(
        ",".join("%du" % (t & 0xFFFFFFFF) for t in v[d]["xtoviewangle"]) for d in (0, 1))
    s += "const int ref_viewwidth[2] = {%d, %d};\n" % (v[0]["viewwidth"], v[1]["viewwidth"])
    s += arr("int", "ref_yslope", v[0]["yslope"])
    s += "const int ref_scalelight[2][768] = {{%s},{%s}};\n" % tuple(
        ",".join(map(str, v[d]["scalelight"])) for d in (0, 1))
    s += arr("int", "ref_zlight", up["zlight"])
    s += arr("uint8_t", "ref_colormap", list(colormap[:33 * 256]))
    return s


def wad_colormap(data):
    """COLORMAP lump of the WAD (the last one with that name), parsed here."""
    n, off = struct.unpack_from("<ii", data, 4)
    found = None
    for i in range(n):
        p, size, name = struct.unpack_from("<ii8s", data, off + 16 * i)
        if name.rstrip(b"\0") == b"COLORMAP":
            found = data[p:p + size]
    if found is None or len(found) < 33 * 256:
        raise SystemExit("FAIL: no COLORMAP in the WAD")
    return found


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--zip")
    a = ap.parse_args()
    for f in ("trig_tables.h", "trig_tables.c", "view.c", "colormap.c", "info.c", "info.h", "dmcdat.h"):
        if not os.path.exists(os.path.join(GEN, f)):
            raise SystemExit("FAIL: gen/%s missing, run make gen first" % f)
    try:
        up = upstream.load()
        cm = wad_colormap(fetch.extract(a.zip)[0])
    except ConvError as e:
        raise SystemExit("FAIL: %s" % e)
    os.makedirs(OUT, exist_ok=True)
    ref_c = os.path.join(OUT, "trig_ref.c")
    write_if_changed(ref_c, ref_source(up, cm))
    srcs = [os.path.join(REPO, "tests", "conv", "trig_test.c"), os.path.join(GEN, "trig_tables.c"),
            os.path.join(GEN, "view.c"), os.path.join(GEN, "colormap.c")]
    objs = []
    for s in srcs:
        o = os.path.join(OUT, os.path.basename(s) + ".o")
        run([CC, "-O2", "-fwrapv", "-fsigned-char"] + WARN + ["-I", GEN, "-I", MATH, "-c", s, "-o", o])
        objs.append(o)
    trig_objs = []
    for name, defs in (("trig.c.o", []), ("trig_l1.c.o", ["-DDMC_SINE_L1"])):
        o = os.path.join(OUT, name)
        run([CC, "-O2", "-fwrapv", "-fsigned-char"] + WARN + defs + ["-I", GEN, "-I", MATH, "-c",
                                                                     os.path.join(MATH, "trig.c"), "-o", o])
        trig_objs.append(o)
    o = os.path.join(OUT, "trig_ref.o")
    run([CC, "-O1", "-c", ref_c, "-o", o])
    objs.append(o)
    o = os.path.join(OUT, "m_fixed_ref.o")
    run([CC, "-O2", "-w", "-fwrapv", "-fsigned-char", "-DFixedMul=ref_FixedMul", "-DFixedDiv=ref_FixedDiv",
         "-I", UPSTREAM, "-c", os.path.join(UPSTREAM, "m_fixed.c"), "-o", o])
    objs.append(o)
    for t, label in zip(trig_objs, ("", "DMC_SINE_L1: ")):
        exe = os.path.join(OUT, "trig_test")
        run([CC, "-o", exe] + objs + [t])
        sys.stdout.write(label + run([exe]))

    # ARM: warning-free, no compiler-rt or libc references, sizes per object and table
    arm = []
    for s, defs in [(os.path.join(MATH, "trig.c"), []),
                    (os.path.join(MATH, "trig.c"), ["-DDMC_SINE_L1"]), (os.path.join(GEN, "trig_tables.c"), []),
                    (os.path.join(GEN, "view.c"), []), (os.path.join(GEN, "colormap.c"), []),
                    (os.path.join(GEN, "info.c"), [])]:
        o = os.path.join(OUT, "arm_" + os.path.basename(s)[:-2] + ("_l1" if defs else "") + ".c.o")
        run([ARMCC] + ARMFLAGS + WARN + defs + ["-I", GEN, "-I", MATH, "-c", s, "-o", o])
        arm.append(o)
    und = set()
    for o in arm[:2]:
        und |= set(re.findall(r"U (\S+)", run(["llvm-nm", "-u", o])))
    bad = sorted(u for u in und if not u.startswith("dmc_"))
    if bad:
        raise SystemExit("FAIL: port/math references %s" % bad)
    print("ARM: port/math undefined symbols are table data only (%d), no helpers" % len(und))
    for o in arm:
        print("ARM size %-22s %s" % (os.path.basename(o)[4:-2], run(["llvm-size", o]).splitlines()[1].split()[0]))
    tables = run(["llvm-nm", "-S", "--size-sort"] + arm[2:5])
    for ln in tables.splitlines():
        p = ln.split()
        if len(p) == 4 and p[3].startswith("dmc_"):
            print("  %-28s %6d" % (p[3], int(p[1], 16)))
    print("PASS test_trig")


if __name__ == "__main__":
    main()
