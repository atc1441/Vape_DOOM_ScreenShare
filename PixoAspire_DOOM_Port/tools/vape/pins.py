#!/usr/bin/env python3
"""
pins.py - watch which pin moves.

    python pins.py [seconds]      count once and summarise
    python pins.py live [seconds] report as soon as something stirs

Which pin the key hangs on is written nowhere: `Pinout.txt` names the
connector pins, not their jobs, and the stock firmware reads it somewhere
that is not easy to find in the program image. The pin configuration allows
a guess (an input with a pull-up looks like a key), but guessing is not
necessary here.

The CPU keeps running meanwhile - the Cortex-M4 allows reads without a halt.
So: read all five IDR registers in a loop and report which bits changed.
Press the key while it runs and exactly one pin flips.

Pins the program drives itself (SPI, I2C, chip selects, backlight, watchdog)
are named and marked as such; they change constantly and are not the ones
you are looking for.
"""

import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import vapedoom  # noqa: E402

PORTS = [("A", 0x48000000), ("B", 0x48000400), ("C", 0x48000800),
         ("D", 0x48000C00), ("E", 0x48001000)]
IDR = 0x10

# What the firmware drives itself (py32_hw.h, lcd.c, nor.c, buttons.c)
KNOWN = {
    "A13": "SWDIO", "A14": "SWCLK", "A9": "BLE?", "A10": "BLE?",
    "B3": "SPI SCK", "B4": "SPI MISO", "B5": "SPI MOSI",
    "B6": "LCD CS", "B7": "LCD RST", "B8": "I2C SCL", "B9": "I2C SDA",
    "B13": "Lastpfad", "B15": "Touch INT",
    "C6": "external watchdog",
    "D5": "LCD DC", "D6": "NOR CS",
    "E12": "Touch RST", "E14": "Backlight",
}


def live(secs):
    """Report every change on a pin with no known job, immediately.

    A one-minute measuring window only works if you are sitting in front of
    it. This one runs long and reports by itself - so the key may be
    pressed whenever it suits.
    """
    v = vapedoom.Vape().open()
    try:
        last = {n: v.jl.memory_read32(a + IDR, 1)[0] for n, a in PORTS}
        print("[pins] %s" % ", ".join("%s=0x%04X" % (n, last[n] & 0xFFFF) for n, _ in PORTS))
        print("[pins] ready - press the key, it will report by itself")
        sys.stdout.flush()
        t0 = time.time()
        while time.time() - t0 < secs:
            for n, a in PORTS:
                cur = v.jl.memory_read32(a + IDR, 1)[0]
                ch = (cur ^ last[n]) & 0xFFFF
                last[n] = cur
                for b in range(16):
                    if not (ch >> b & 1):
                        continue
                    k = "%s%d" % (n, b)
                    if k in KNOWN:
                        continue
                    print("[pins] %6.1fs  %-4s -> %d" % (time.time() - t0, k, cur >> b & 1))
                    sys.stdout.flush()
        print("[pins] window closed, nothing unknown seen" if time.time() - t0 >= secs else "")
        sys.stdout.flush()
    finally:
        v.close()
    return 0


def main(argv):
    if len(argv) > 1 and argv[1] == "live":
        return live(float(argv[2]) if len(argv) > 2 else 300.0)
    secs = float(argv[1]) if len(argv) > 1 else 20.0
    v = vapedoom.Vape().open()
    try:
        base = {n: v.jl.memory_read32(a + IDR, 1)[0] for n, a in PORTS}
        print("idle:")
        for n, a in PORTS:
            print("  GPIO%s IDR 0x%08X" % (n, base[n]))
        print("")
        print("watching for %.0f s - press the key now and let go again." % secs)
        flips = {}
        lo = {}
        last = dict(base)
        t0 = time.time()
        reads = 0
        while time.time() - t0 < secs:
            for n, a in PORTS:
                cur = v.jl.memory_read32(a + IDR, 1)[0]
                ch = cur ^ last[n]
                if ch:
                    for b in range(16):
                        if ch >> b & 1:
                            k = "%s%d" % (n, b)
                            flips[k] = flips.get(k, 0) + 1
                            if not (cur >> b & 1):
                                lo[k] = lo.get(k, 0) + 1
                    last[n] = cur
            reads += 1
        print("")
        print("%d passes." % reads)
        if not flips:
            print("Not a single pin changed.")
            return 1
        print("pin   changes  of those to low  note")
        for k in sorted(flips, key=lambda x: -flips[x]):
            print("  %-4s %6d %12d        %s" % (k, flips[k], lo.get(k, 0), KNOWN.get(k, "")))
        unknown = [k for k in flips if k not in KNOWN]
        print("")
        if unknown:
            print("With no known job: %s" % ", ".join(sorted(unknown)))
        else:
            print("Only pins the program moves itself - the key was not among them.")
        return 0
    finally:
        v.close()


if __name__ == "__main__":
    sys.exit(main(sys.argv))
