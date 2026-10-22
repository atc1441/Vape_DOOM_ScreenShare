#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Runs the overlay demo firmware (OVLDEMO.BIN, polled NOR and LCD) in Renode and checks it.

The board is tests/renode/dmc100.repl with the bootloader stand-in blstub. The NOR holds the
image of norimg.py: DOOMCODE.DMC is read through the fixed-offset locator (port/ovl/ovl_fixed.c)
from the DEMO_NOR_* offsets, there is no FAT volume. The firmware runs the call checks on the
thunk and the -mlong-calls packs and the error checks, then calls overlay A with a pack of
another build: its fail hook draws the mismatch screen and stops. The test reads demo_res[]
from SRAM, compares the screen and checks that the NOR is unchanged.

usage: test_renode.py --build build/ovl [--renode PATH]
Renode comes from --renode, else from env RENODE, else from PATH.
"""
import argparse
import os
import re
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

BUTTONS = (("B", 4), ("B", 5), ("B", 6), ("B", 7), ("A", 15), ("F", 7))
failures = []


def check(cond, msg):
    print(("  ok   " if cond else "  FAIL ") + msg)
    if not cond:
        failures.append(msg)


def names(macro):
    with open(os.path.join(HERE, "ovl_demo.h")) as f:
        text = f.read()
    body = text[text.index(f"#define {macro}(X)"):]
    body = body[:body.index("\n\n")]
    return re.findall(r'X\((\w+), "([^"]*)"\)', body)


def ppm_landscape(path):
    """Panel memory (240 x 320 as sent) as a function of the landscape (x, y)."""
    with open(path, "rb") as f:
        data = f.read()
    m = re.match(rb"P6\s+(\d+)\s+(\d+)\s+255\s", data)
    w, h, pix = int(m.group(1)), int(m.group(2)), data[m.end():]
    assert (w, h) == (240, 320)
    return lambda x, y: tuple(pix[3 * (x * 240 + y):3 * (x * 240 + y) + 3])


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--build", required=True)
    ap.add_argument("--renode", default=os.environ.get("RENODE") or "renode")
    a = ap.parse_args()
    b = os.path.abspath(a.build)
    d = os.path.join(b, "demo")
    work = os.path.join(b, "renode-run")
    os.makedirs(work, exist_ok=True)
    renode = shutil.which(a.renode)
    if renode is None:
        sys.exit(f"Renode not found at {a.renode} (set RENODE)")
    D = norimg.defines(os.path.join(HERE, "ovl_demo.h"))
    O = norimg.defines(os.path.join(ROOT, "port", "ovl", "ovl.h"))
    res_at = po.Elf(os.path.join(d, "main.elf")).sym("demo_res")["value"]
    w = lambda n: os.path.join(work, n)
    for n in ("sram.bin", "nor_out.bin", "screen.ppm", "log.txt"):
        if os.path.exists(w(n)):
            os.remove(w(n))
    rd = os.path.join(ROOT, "tests", "renode")
    lines = [f"include @{os.path.join(rd, 'DMC100LCD.cs')}",
             'mach create "ovl"',
             f"machine LoadPlatformDescription @{os.path.join(rd, 'dmc100.repl')}",
             "logLevel 3",
             "sysbus UnhandledAccessBehaviour ThrowException",
             f"sysbus LoadBinary @{os.path.join(b, 'blstub.bin')} 0x08000000",
             f"sysbus LoadBinary @{os.path.join(d, 'OVLDEMO.BIN')} 0x08007000",
             f"sysbus LoadBinary @{os.path.join(d, 'nor.img')} 0x90000000"]
    lines += [f"gpioPort{p} OnGPIO {n} true" for p, n in BUTTONS]
    lines += ["cpu VectorTableOffset 0x08000000",
              'emulation RunFor "00:00:02"',
              f"sram DumpBinary @{w('sram.bin')}",
              f"spi2.lcd SaveScreenshot @{w('screen.ppm')}",
              f"nor_mem DumpBinary @{w('nor_out.bin')}",
              'echo "DONE"', "cpu PC", "quit"]
    with open(w("run.resc"), "w") as f:
        f.write("\n".join(lines) + "\n")
    p = subprocess.run([renode, "--disable-gui", "--console", "--plain", w("run.resc")],
                       capture_output=True, text=True, timeout=900, cwd=work, stdin=subprocess.DEVNULL)
    with open(w("log.txt"), "w") as f:
        f.write(p.stdout + p.stderr)
    if not os.path.exists(w("sram.bin")):
        sys.exit(f"Renode did not finish, see {w('log.txt')}")

    print("overlay demo in Renode (polled NOR, DOOMCODE.DMC at fixed NOR offsets)")
    with open(w("sram.bin"), "rb") as f:
        sram = f.read()
    res = struct.unpack_from(f"<{D['R_COUNT']}I", sram, res_at - 0x20000000)
    check(res[D["R_MAGIC"]] == D["DEMO_MAGIC"], f"demo_res found at {res_at:#x}")
    check(res[D["R_STAGE"]] == 4, f"all checks ran and the fail hook stopped the app (stage {res[D['R_STAGE']]})")
    for tag, slot in (("linker thunks", "R_FAIL0"), ("-mlong-calls", "R_FAIL1")):
        for k, (_, desc) in enumerate(names("DEMO_CHECKS")):
            check(not res[D[slot]] >> k & 1, f"{tag}: {desc}")
    for k, (_, desc) in enumerate(names("DEMO_ERRORS")):
        code = struct.unpack("<i", struct.pack("<I", res[D["R_CODES"] + k]))[0]
        check(not res[D["R_FAILE"]] >> k & 1, f"{desc} ({code})")
    check(res[D["R_LOADS"]] == 12, f"{res[D['R_LOADS']]} completed loads (4 per call pass, 4 in the error checks)")
    hid, herr = res[D["R_HOOK_ID"]], struct.unpack("<i", struct.pack("<I", res[D["R_HOOK_ERR"]]))[0]
    check(hid == O["OVL_TEST_A"] and herr == -3, f"OVL_CALL with another build: fail hook got overlay {hid}, error {herr} (OVL_EBUILD)")

    px = ppm_landscape(w("screen.ppm"))
    bg = sum(1 for x in range(320) for y in range(240) if px(x, y) == (0, 0, 132))
    white = sum(1 for x in range(16, 304) for y in range(40, 56) if px(x, y) == (255, 255, 255))
    yellow = sum(1 for x in range(16, 272) for y in range(80, 88) if px(x, y) == (255, 255, 0))
    check(bg > 60000 and white > 300 and yellow > 200,
          f"mismatch screen drawn by the hook ({bg} background, {white} title, {yellow} text px)")
    with open(w("nor_out.bin"), "rb") as f, open(os.path.join(d, "nor.img"), "rb") as g:
        check(f.read() == g.read(), "NOR image unchanged")

    if not failures:
        for n in ("sram.bin", "nor_out.bin"):
            os.remove(w(n))
    print("ALL PASSED" if not failures else f"{len(failures)} FAILED")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
