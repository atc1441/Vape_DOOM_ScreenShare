#!/usr/bin/env python3
"""
flash.py - put the firmware on the device, in chunks, with an honest check.

    python flash.py [image.bin]

Why not simply `JLink loadfile`:

The external watchdog hangs on PC6 and pulls NRST as soon as nothing feeds
it. While the old firmware runs it does - but `erase` takes that firmware
away, and from then on the watchdog resets the part about once a second. A
write across the whole image takes ten seconds and does not survive that:
measured, it breaks off after roughly 180 KB with a "T-bit of XPSR is 0 but
should be 1" in the middle, which is exactly such a reset. After that the
flash is half written, the CPU runs into lockup, and the next attempt fails
at the erase already ("Cannot read register ... while CPU is running").

In chunks it works. A 32 KiB chunk takes a little over a second and fits
inside a watchdog window; what is already in flash survives the reset; and
`r`/`h` before each chunk sets up cleanly again. If a chunk does not land,
only that one is repeated.

And why not `vapedoom.py flash`: at this size its loader reproducibly loses
pages at the end of the image, and `erase()` fails afterwards.

Verification runs over a *fresh* connection. Without that you are not
reading flash but the loader's buffer - which cheerfully reports what ought
to be there, so an empty flash looks exactly like a full one. That happened
here once.
"""

import io
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)

import vapedoom  # noqa: E402

JLINK = os.environ.get("VAPE_JLINK",
                       r"C:\Program Files\SEGGER\JLink_V870\JLink.exe")
SCRIPT = os.path.join(ROOT, "jlink", "script.js")
WORK = os.path.join(ROOT, "jlink", "parts")
BIN = os.path.join(ROOT, "out_ud", "DOOMVAPE.BIN")
BASE = 0x08000000
STEP = 0x8000
SPEED = 1000


def run(lines, timeout=400):
    path = os.path.join(WORK, "run.jlink")
    io.open(path, "w", newline="\n").write("\n".join(lines + ["exit", ""]))
    out = os.path.join(WORK, "run.out")
    with io.open(out, "wb") as fh:
        subprocess.run([JLINK, "-device", vapedoom.DEVICE, "-if", "SWD",
                        "-speed", str(SPEED), "-autoconnect", "1",
                        "-SelectEmuBySN", str(vapedoom.SN),
                        "-JLinkScriptFile", SCRIPT, "-CommanderScript", path],
                       stdout=fh, stderr=subprocess.STDOUT,
                       stdin=subprocess.DEVNULL, timeout=timeout)
    return io.open(out, encoding="latin1").read()


def bad_blocks(data, block=4096):
    """Compare flash against the file - fresh connection, no loader."""
    v = vapedoom.Vape().open()
    try:
        bad = []
        for off in range(0, len(data), block):
            n = min(block, len(data) - off)
            if bytes(v.jl.memory_read8(BASE + off, n)) != data[off:off + n]:
                bad.append(off)
        return bad
    finally:
        v.close()


def main(argv):
    path = argv[1] if len(argv) > 1 else BIN
    data = io.open(path, "rb").read()
    os.makedirs(WORK, exist_ok=True)
    parts = []
    for i, off in enumerate(range(0, len(data), STEP)):
        p = os.path.join(WORK, "part%02d.bin" % i)
        io.open(p, "wb").write(data[off:off + STEP])
        parts.append((off, p))
    print("[flash] %s, %d B in %d chunks" % (os.path.basename(path), len(data), len(parts)))

    lines = ["power on", "r", "h", "erase"]
    for off, p in parts:
        lines += ["r", "h", "loadbin %s 0x%08X" % (p, BASE + off)]
    run(lines)

    for attempt in range(1, 7):
        bad = bad_blocks(data)
        if not bad:
            break
        # Only the chunks that are missing something, once more.
        todo = sorted({b // STEP for b in bad})
        print("[flash] attempt %d: %d blocks wrong in chunk %s"
              % (attempt, len(bad), ", ".join(str(t) for t in todo)))
        lines = ["power on"]
        for i in todo:
            off, p = parts[i]
            lines += ["r", "h", "loadbin %s 0x%08X" % (p, BASE + off)]
        run(lines)
    else:
        print("[flash] failed, %d blocks remain wrong" % len(bad))
        return 1

    v = vapedoom.Vape().open()
    try:
        sp, pc = v.jl.memory_read32(BASE, 2)
        print("[flash] verified, reset vector SP 0x%08X  PC 0x%08X" % (sp, pc))
        v.jl.reset(halt=False)
        v.jl.restart()
    finally:
        v.close()
    print("[flash] started")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
