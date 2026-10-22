#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Runs the Renode variant of the DOOM firmware (mk/fw.mk, renode) and checks it.

The board is tests/renode/dmc100.repl (128 KB of flash, 32 KB of SRAM) with the bootloader
stand-in blstub. DOOMREN.BIN, the meter's image but for the polled NOR and LCD drivers, goes to
the app slot, the NOR image is written by tests/renode/fwnor.py with the files at their places
on both FAT12 volumes. A hook
at app_main writes the test settings into the flash block dmc_test (port/game/dmc_fw.h: the
tic and frame limits of the passes, the demos that run, forced level exits), a loop in the
monitor runs the emulation until the firmware reaches a state. The firmware boots to its
results screen, MODE there plays the demos. Runs:
  demos    DEMO1 of the image and DEMO2.LMP (the E1M1 script, tools/mklmp.py): verification
           pass of 2,101 tics (DEMO1 stops at the limit, the 2,100 tics of DEMO2.LMP at the end
           marker) and 6 timed frames each. DOOMRES.DAT taken out of the NOR afterwards must
           decode (tests/device/doomres.py) with every chained hash equal to the host engine's
           (build/host/dmcdoom_skip, the masked hash of lockstep-skip), and the view hashes of
           the first timed frames equal to the host's frames (--frame-out, low detail). Then
           SAVE starts play mode: the title page of the title loop, the same as the host's
           title frame, its tics run in real time.
  install  no DOOM.DAT, folder UI with stock art: the installer screen, SAVE erases the data
           drive (a valid empty FAT12 volume with the folder UI, checked with fsck.fat and
           mtools, every other cluster erased, the firmware drive unchanged) and opens it on
           USB (the board has a stub of the USB controller, the device is never configured),
           SAVE there resets the meter into the installer again, MODE opens the drive as it is
  hold     DOOM.DAT and folder UI: HOLD on the results screen opens the data drive on USB, SAVE
           there resets the meter to the results screen. The boot made SAVE.DAT in the 16 free
           clusters of the data drive (fsck.fat, mdir, 65,536 B, both FATs equal), the NOR is
           unchanged but for its chain in both FATs, its root entry and its 4 slot headers
  uiback   DOOM.DAT and folder UI_OFF: UI_OFF becomes UI at boot, the data drive is unchanged
           otherwise and the demos run except DEMO3.LMP, a deathmatch demo, whose record says
           so. Then a panic (plat_panic) on the play screen writes the crash record into
           DOOMRES.DAT and the fault screen says so.
  doomx    DOOM.DAT without DOOMX.DAT: the folder goes back to UI and the error screen stays
  play     a boot straight into play mode, DEMO1 the only demo, test tics 300: the title page,
           LIGHT the main menu over it, LIGHT the skill page, LIGHT E1M1 on I'm Too Young To
           Die with a forced exit at level time 200. In the intermission the menu opens and
           closes (POWER held 1 s twice) while the statistics count, LIGHT completes them,
           LIGHT shows the next map, then E1M2. There POWER held 1 s opens the menu over the
           frozen view, HOLD and LIGHT the options, POWER closes it, POWER held 1 s opens it
           again, HOLD twice and LIGHT the Save Game page, LIGHT saves into slot 0. V-A held
           walks on, then POWER held 1 s, V-A and LIGHT the Load Game page, LIGHT loads slot 0.
           Then the benchmark: the options, HOLD to Benchmark, LIGHT, LIGHT (y). The screens of
           the title, the menu pages over
           it and the intermission are the host engine's frames of the same state (the skull
           and the "you are here" pointer blink, either frame), the pages over E1M2 the host's
           inside the menu's panel (Load Game with a save of the same level time) and outside
           it the LCD as it was when the menu opened (taken by a hook on M_Responder). Hooks
           on P_Ticker, G_DoSaveGame and G_DoLoadGame log the level time, the map and the
           player's object: the first tic after the load is the first tic after the save. The
           boot made SAVE.DAT (fsck.fat, mdir, 65,536 B, at most 8 runs, both FATs equal). The
           benchmark's record of DEMO1 has the host's chained hashes, SAVE on the results
           screen goes back to the title page, and the stack high-water stays at most 1,300 B.
  saveload two boots on the E1 pack with DEMO2.LMP and no folder UI (with the stock art the
           volume has no room for SAVE.DAT next to DEMO2.LMP): the first verifies 1,500 tics of
           DEMO2.LMP and saves into slot 0 after the display of tic 1,000 (dmc_test.save_tic),
           the second loads slot 0 right after the demo has set up its level (dmc_test.load).
           The first chain must equal the host's, the second the host's stream from tic 1,001
           on, chained from the start.
  finale   only with --only finale: as play, E1M1 to E1M8 on I'm Too Young To Die with a forced
           exit at level time 35 in each, LIGHT through every intermission, then the finale.
           A hook on I_FinishUpdate, where a frame is on the LCD whole, takes screenshots at
           three finale counts of the text stage and at the start of the art stage: they equal
           the host's frames of the same counts (-warp 1 8 with the same exit). POWER held 1 s
           opens the main menu over the art stage, its panel equals the host's and outside it
           the LCD is as it was, then Quit Game and n. The stack high-water stays at most
           1,300 B.
  attract  as play, but the attract demo plays when POWER is held 6 s (the menu at 1 s, the quit
           prompt at 4 s), POWER (n) answers the prompt, then the menu opens over the demo
           (POWER held 1 s, or LIGHT as any key) and POWER held 1 s closes it, 12 times. No fault
           and no panic, the demo goes on.
  multimap only with --only multimap (about 5 minutes): the multi-map demo of the
           lockstep workload exitlist (tests/lockstep/run_lockstep.py, made with the oracle)
           as DEMO2.LMP, alone, with the forced exits of that workload. Its verification pass
           runs through E1M1, E1M2, E1M3 (secret exit), E1M9, E1M4 to E1M8, the intermissions
           and the finale to its end marker, 100 tics after the art stage: every chained hash
           must equal the host's (dmcdoom_skip with the same exits), the counted events of
           all levels must be 0, the plans of the levels it set up must be those report.json
           gives the nine maps at skill 4, and the pages and the arena of the record those of
           E1M1.

usage: test_fw.py --build BUILD [--renode PATH] [--only NAME]
"""
import argparse
import json
import os
import re
import shutil
import struct
import subprocess
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(ROOT, "tests", "device"))
sys.dont_write_bytecode = True
import doomres  # noqa: E402
import fwnor  # noqa: E402

sys.path.insert(0, os.path.join(ROOT, "tests", "lockstep"))
import run_lockstep  # noqa: E402

sys.path.insert(0, os.path.join(ROOT, "tests", "host"))
import savefs  # noqa: E402

BUTTONS = {"HOLD": ("B", 4), "SAVE": ("B", 5), "MODE": ("B", 6), "VA": ("B", 7),
           "LIGHT": ("A", 15), "POWER": ("F", 7)}
DEFAULT_RENODE = "renode"
MAIN_ROOT = 0x100000 + 3 * 4096  # root directory of the data drive
# Stack high-water gate of the 1,536 B stack: Renode never runs the game pad's interrupt, which
# takes about 216 B more on the meter
STACK_GATE = 1300
GS_DEMOSCREEN = 3
D = doomres.D
# dmc_test_t (port/game/dmc_fw.h): field, offset, bytes per value
TEST_FIELDS = {"exits": (0, 2), "save_tic": (24, 4), "load": (28, 1), "boot_bench": (29, 1),
               "demos": (30, 1), "tics": (32, 4), "frames": (36, 4)}
failures = []


def check(cond, msg):
    print(("  ok   " if cond else "  FAIL ") + msg)
    if not cond:
        failures.append(msg)


def symbols(elf):
    out = subprocess.run(["llvm-nm", elf], check=True, capture_output=True, text=True).stdout
    syms = {}
    for line in out.splitlines():
        p = line.split()
        if len(p) == 3:
            syms[p[2]] = int(p[0], 16)
    return syms


def landscape(ppm):
    return Image.open(ppm).convert("RGB").transpose(Image.Transpose.TRANSPOSE)


class Run:
    """One Renode run: setup, a list of steps, dumps at the end. test: the test settings
    (TEST_FIELDS, exits a list of words), the others stay 0."""

    def __init__(self, renode, work, build, name, nor, **test):
        self.renode, self.work, self.name = renode, work, name
        b = os.path.join(build, "fwdoom", "renode")
        self.syms = symbols(os.path.join(b, "main.elf"))
        self.files = (os.path.join(build, "plat", "renode", "blstub.bin"), os.path.join(b, "DOOMREN.BIN"))
        self.nor_in = nor
        self.test = test
        self.py = []
        self.hooks = []

    def w(self, n):
        return os.path.join(self.work, f"{self.name}_{n}")

    def until(self, state, seconds, step=2, name="res_cur_state", size=4):
        """Run until the word (size 4) or half word (2) name reaches state, at most seconds of
        emulated time."""
        read = "ReadDoubleWord" if size == 4 else "ReadWord"
        self.py += [f"for i in range({seconds // step}):",
                    f"    monitor.Parse('emulation RunFor \"00:00:{step:02d}\"')",
                    f"    if bus.{read}({self.syms[name]:#x}) >= {state}:",
                    "        break"]

    def until_eq(self, value, seconds, addr, size=4, cmp="=="):
        """Run until the word (size 4) or byte (1) at addr compares to value, at most seconds
        of emulated time, a tenth of a second at a time."""
        read = "ReadDoubleWord" if size == 4 else "ReadByte"
        self.py += [f"for i in range({seconds * 10}):",
                    "    monitor.Parse('emulation RunFor \"00:00:00.100\"')",
                    f"    if bus.{read}({addr:#x}) {cmp} {value}:",
                    "        break"]

    def hold(self, button, seconds):
        """button held for seconds of emulated time."""
        p, n = BUTTONS[button]
        self.py += [f"monitor.Parse('gpioPort{p} OnGPIO {n} false')",
                    f"monitor.Parse('emulation RunFor \"{seconds}\"')",
                    f"monitor.Parse('gpioPort{p} OnGPIO {n} true')",
                    "monitor.Parse('emulation RunFor \"00:00:00.300\"')"]

    def menu(self, n, button, seconds=300):
        """Wait for the n-th results screen since boot, then press button on it."""
        self.until(n, seconds, 1, "res_menus")
        self.run_for("00:00:00.500")
        self.press(button)

    def run_for(self, seconds):
        self.py.append(f"monitor.Parse('emulation RunFor \"{seconds}\"')")

    def press(self, button):
        p, n = BUTTONS[button]
        self.py += [f"monitor.Parse('gpioPort{p} OnGPIO {n} false')",
                    "monitor.Parse('emulation RunFor \"00:00:00.150\"')",
                    f"monitor.Parse('gpioPort{p} OnGPIO {n} true')",
                    "monitor.Parse('emulation RunFor \"00:00:00.300\"')"]

    def panic(self, msg):
        # the message in the tic scratch, which nothing reads once plat_panic runs
        at = self.syms["dmc_tic"]
        for i, b in enumerate(msg.encode() + b"\0"):
            self.py.append(f"bus.WriteByte({at + i:#x}, {b:#x})")
        self.py += [f"monitor.Parse('cpu SetRegisterUnsafe 0 {at:#x}')",
                    f"monitor.Parse('cpu PC {self.syms['plat_panic'] & ~1:#x}')"]

    def point(self, tag, nor=False):
        self.py += [f"monitor.Parse('sram DumpBinary @{self.w(tag + '_sram.bin')}')",
                    f"monitor.Parse('spi2.lcd SaveScreenshot @{self.w(tag + '.ppm')}')"]
        if nor:
            self.py.append(f"monitor.Parse('nor_mem DumpBinary @{self.w(tag + '_nor.bin')}')")

    def go(self, timeout=7200):
        for old in os.listdir(self.work):
            if old.startswith(self.name + "_"):
                os.remove(os.path.join(self.work, old))
        with open(self.w("nor.bin"), "wb") as f:
            f.write(self.nor_in)
        s = self.syms
        write = {1: "WriteByte", 2: "WriteWord", 4: "WriteDoubleWord"}
        hook = []
        for field, value in self.test.items():
            off, size = TEST_FIELDS[field]
            for i, v in enumerate(value if isinstance(value, list) else [value]):
                hook.append(f"machine.SystemBus.{write[size]}({s['dmc_test'] + off + size * i:#x}, {v})")
        hook = "; ".join(hook) or "pass"
        rel = "; ".join(f"machine['sysbus.gpioPort{p}'].OnGPIO({n}, True)" for p, n in BUTTONS.values())
        lines = [f"include @{os.path.join(HERE, 'DMC100LCD.cs')}",
                 f'mach create "{self.name}"',
                 f"machine LoadPlatformDescription @{os.path.join(HERE, 'dmc100.repl')}",
                 "logLevel 3",
                 "sysbus UnhandledAccessBehaviour ThrowException",
                 f"sysbus LoadBinary @{self.files[0]} 0x08000000",
                 f"sysbus LoadBinary @{self.files[1]} 0x08007000",
                 f"sysbus LoadBinary @{self.w('nor.bin')} 0x90000000"]
        lines += [f"gpioPort{p} OnGPIO {n} true" for p, n in BUTTONS.values()]
        lines += [f'cpu AddHook 0x08000008 "{rel}"', f'cpu AddHook {s["app_main"] & ~1:#x} "{hook}"']
        lines += [f'cpu AddHook {s[fn] & ~1:#x} "{py}"' for fn, py in self.hooks]
        lines += ["cpu VectorTableOffset 0x08000000", f"include @{self.w('steps.py')}",
                  f"nor_mem DumpBinary @{self.w('nor_out.bin')}", "quit"]
        with open(self.w("steps.py"), "w") as f:
            f.write("\n".join(['bus = self.Machine["sysbus"]'] + self.py) + "\n")
        with open(self.w("run.resc"), "w") as f:
            f.write("\n".join(lines) + "\n")
        # no console: a long run with the console reading a closed stdin ends in a Renode error
        p = subprocess.run([self.renode, "--disable-gui", "-P", "-1", "--plain", self.w("run.resc")],
                           capture_output=True, text=True, timeout=timeout, cwd=self.work,
                           stdin=subprocess.DEVNULL)
        with open(self.w("log.txt"), "w") as f:
            f.write(p.stdout + p.stderr)
        out = {}
        if os.path.exists(self.w("nor_out.bin")):
            with open(self.w("nor_out.bin"), "rb") as f:
                out["nor"] = f.read()
        return out

    def sram(self, tag):
        path = self.w(tag + "_sram.bin")
        if not os.path.exists(path):
            return None
        with open(path, "rb") as f:
            return f.read()

    def word(self, sram, name, off=0):
        return struct.unpack_from("<I", sram, self.syms[name] + off - 0x20000000)[0]

    def half(self, sram, name):
        return struct.unpack_from("<H", sram, self.syms[name] - 0x20000000)[0]

    def nor(self, tag):
        path = self.w(tag + "_nor.bin")
        if not os.path.exists(path):
            return None
        with open(path, "rb") as f:
            return f.read()


def root_entry(nor, name):
    for i in range(512):
        e = nor[MAIN_ROOT + 32 * i:MAIN_ROOT + 32 * i + 32]
        if e[:11] == name:
            return MAIN_ROOT + 32 * i
    return None


def diff_main(a, b):
    """Byte offsets where the data drives of two NOR images differ."""
    return [i for i in range(0x100000, 0x400000) if a[i] != b[i]] if a[0x100000:] != b[0x100000:] else []


def volume(work, name, nor):
    """fsck.fat and mtools on the data drive of a NOR image: fsck.fat accepts it, the names
    in its root directory and in its folder UI."""
    path = os.path.join(work, name)
    with open(path, "wb") as f:
        f.write(nor[0x100000:])
    env = dict(os.environ, MTOOLS_SKIP_CHECK="1")
    ok = subprocess.run(["fsck.fat", "-n", path], capture_output=True).returncode == 0
    names = []
    for d in ("::/", "::/UI"):
        r = subprocess.run(["mdir", "-b", "-i", path, d], capture_output=True, text=True, env=env)
        names.append(sorted(os.path.basename(x.rstrip("/")) for x in r.stdout.split("\n") if x.strip())
                     if r.returncode == 0 else None)
    os.remove(path)
    return ok, names[0], names[1]


def save_dat(work, name, nor):
    """SAVE.DAT as the boot makes it on the data drive of a NOR image: fsck.fat accepts the
    volume, mdir lists the file, 65,536 B in at most 8 runs, both FATs equal. (ok, runs)."""
    ok, root, _ = volume(work, name, nor)
    v = savefs.Vol(nor)
    f = v.file(b"SAVE    DAT")
    good = ok and bool(root) and "SAVE.DAT" in root and f is not None and len(f[0]) == 65536 \
        and f[1] <= 8 and all(x == v.fats[0] for x in v.fats)
    return good, f[1] if f else None


def save_dat_bytes(nor):
    """NOR offsets the making of SAVE.DAT changes: its chain in both FATs, its root entry and
    the 4 sectors of its slot headers."""
    v = savefs.Vol(nor)
    e = v.find(b"SAVE    DAT")
    if not e:
        return set()
    out = set()
    at = MAIN_ROOT + v.v[v.root:v.data].index(e) // 32 * 32
    out.update(range(at, at + 32))
    fat = 0x100000 + v.root - len(v.fats) * len(v.fats[0])
    c, k = struct.unpack_from("<H", e, 26)[0], 0
    while 2 <= c < 0xFF8:
        for f in range(len(v.fats)):
            o = fat + f * len(v.fats[0]) + c * 3 // 2
            out.update((o, o + 1))
        if k % 4 == 0:
            o = 0x100000 + v.data + (c - 2) * v.cs
            out.update(range(o, o + v.cs))
        c, k = v.entry(c), k + 1
    return out


def view_hash(png):
    img = Image.open(png)
    px = img.load()
    h = doomres.FNV_OFFSET
    for x in range(320):
        for y in range(200):
            h = ((h ^ px[x, y]) * doomres.FNV_PRIME) & 0xFFFFFFFFFFFFFFFF
    return h


def ui_id(name):
    """A dmc_ui picture id of gen/dmcdat.h."""
    with open(os.path.join(ROOT, "gen", "dmcdat.h")) as f:
        return int(re.search(r"\b%s = (\d+)" % name, f.read()).group(1))


def host_title(engine, nor, work):
    """The host engine's frame of the title page (palette indices and its palette)."""
    fd = os.path.abspath(os.path.join(work, "title_frames"))
    shutil.rmtree(fd, ignore_errors=True)
    os.makedirs(fd)
    subprocess.run([os.path.abspath(engine), "--nor", os.path.abspath(nor), "--tics", "12",
                    "--frame-out", fd], check=False, stdout=subprocess.DEVNULL,
                   stderr=subprocess.DEVNULL, cwd=fd)
    png = os.path.join(fd, "f000010.png")
    img = Image.open(png) if os.path.exists(png) else None
    if img:
        img.load()
    shutil.rmtree(fd, ignore_errors=True)
    return img


def same_screen(ppm, frame):
    """The screenshot of the panel (RGB565 widened) shows the host frame: the RGB565 of every
    pixel equal to the colour its palette index has in the frame."""
    if frame is None:
        return False
    shot = landscape(ppm)
    pal, px = frame.getpalette(), frame.load()
    for y in range(240):
        for x in range(320):
            r, g, b = shot.getpixel((x, y))
            c = pal[3 * px[x, y]:3 * px[x, y] + 3]
            if (r >> 3, g >> 2, b >> 3) != (round(c[0] * 31 / 255), round(c[1] * 63 / 255),
                                             round(c[2] * 31 / 255)):
                return False
    return True


def text_on(ppm):
    img = landscape(ppm)
    return sum(1 for x in range(0, 320, 2) for y in range(0, 240, 2) if sum(img.getpixel((x, y))) > 300)


def text_at(ppm, x, y, s):
    """s in the 8x8 font of platform/font8x8.c at x, y, light on a dark background."""
    with open(os.path.join(ROOT, "platform", "font8x8.c")) as f:
        rows = re.findall(r"\{((?:0x[0-9A-Fa-f]{2},?\s*){8})\}", f.read())
    font = [[int(v, 16) for v in r.replace(",", " ").split()] for r in rows]
    img = landscape(ppm)
    for i, ch in enumerate(s):
        g = font[ord(ch) - 32]
        for gy in range(8):
            for gx in range(8):
                if (sum(img.getpixel((x + 8 * i + gx, y + gy))) > 500) != bool(g[gy] & 0x80 >> gx):
                    return False
    return True


def test_demos(rn, build, args):
    print("demos: DEMO1 and DEMO2.LMP, verification and timed passes")
    pk = os.path.join(build, "packs", "E1")
    res = os.path.join(rn.work, "DOOMRES.DAT")
    doomres_blank = bytes(D["RES_SIZE"])
    with open(res, "wb") as f:
        f.write(doomres_blank)
    lmp = os.path.join(build, "fwdoom", "DEMO2.LMP")
    nor = fwnor.build([os.path.join(pk, "DOOMX.DAT"), os.path.join(build, "fwdoom", "renode", "DOOMCODE.DMC"), res],
                      [os.path.join(pk, "DOOM.DAT"), lmp], "UI")
    r = Run(rn.renode, rn.work, build, "demos", nor, boot_bench=1, tics=2101, frames=6)
    r.menu(1, "MODE")
    r.until(D["RS_DONE"], 600)
    r.until(2, 60, 1, "res_menus")
    r.run_for("00:00:00.500")
    r.point("done")
    r.press("SAVE")
    r.run_for("00:00:01")
    r.point("play0")
    r.run_for("00:00:03")
    r.point("play1")
    out = r.go()
    done = r.sram("done")
    if done is None or "nor" not in out:
        check(False, "the run reached its checkpoints, see demos_log.txt")
        return
    check(r.word(done, "res_cur_state") == D["RS_DONE"], "the demos ran to the end (DOOMRES.DAT state done)")
    data = doomres.from_nor(r.w("nor_out.bin"))
    with open(r.w("DOOMRES.DAT"), "wb") as f:
        f.write(data)
    dec = doomres.decode(data)
    doomres.report(dec)
    check(len(dec["demos"]) == 2 and [d["name"] for d in dec["demos"]] == ["DEMO1", "DEMO2.LMP"],
          "DOOMRES.DAT decodes with the records of DEMO1 and DEMO2.LMP")
    if len(dec["demos"]) != 2:
        return
    d1, d2 = dec["demos"]
    check(d1["r"][D["D_V_TICS"]] == 2101 and d1["r"][D["D_V_END"]] == D["DE_LIMIT"],
          f"DEMO1 verified for {d1['r'][D['D_V_TICS']]} tics, up to the test limit of 2,101")
    check(d2["r"][D["D_V_TICS"]] == 2100 and d2["r"][D["D_V_END"]] == D["DE_MARKER"],
          f"DEMO2.LMP verified to its end marker, {d2['r'][D['D_V_TICS']]} tics")
    for d in dec["demos"]:
        ev = d["r"][D["D_V_EVENTS"]:D["D_V_EVENTS"] + 3]
        check(ev == [0, 0, 0], f"{d['name']}: no deferred wake-up, skipped transient or deferred mover {ev}")
    hw = os.path.join(rn.work, "host")
    os.makedirs(hw, exist_ok=True)
    ok = doomres.compare(dec, args.host, args.nor, os.path.dirname(lmp), hw)
    check(ok, "every chained hash, the last hash and the last chain equal the host's (lockstep-skip hash)")
    # view hashes of the first timed frames against the host's frames
    for d, demo in ((d1, "demo1"), (d2, lmp)):
        fd = os.path.join(hw, "frames")
        shutil.rmtree(fd, ignore_errors=True)
        os.makedirs(fd)
        subprocess.run([os.path.abspath(args.engine), "--nor", os.path.abspath(args.nor), "-playdemo", demo,
                        "--low-detail", "--frame-out", fd, "--tics", "8"], check=False,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, cwd=hw)
        got = [(t, v) for t, v in d["view"] if t]
        same = len(got) == D["DMC_VIEW_FRAMES"]
        for t, v in got:
            png = os.path.join(fd, f"f{t:06d}.png")
            same &= os.path.exists(png) and view_hash(png) == v
        check(same, f"{d['name']}: the 3D view of timed frames {[t for t, _ in got]} equals the host's frames")
        shutil.rmtree(fd, ignore_errors=True)
        check(d["r"][D["D_T_FRAMES"]] == 6 and d["r"][D["D_T_PHASE"] + 4 * D["PH_TOTAL"] + 3] > 0,
              f"{d['name']}: 6 frames, the last 2 timed")
    img = landscape(r.w("done.ppm"))
    img.save(r.w("done.png"))
    check(text_on(r.w("done.ppm")) > 200, "the results and play screen is shown")
    p0, p1 = r.sram("play0"), r.sram("play1")
    if p0 and p1:
        g0, g1 = r.word(p0, "gametic"), r.word(p1, "gametic")
        check(r.word(p1, "res_cur_state") == D["RS_PLAY"] and r.word(p1, "gamestate") == GS_DEMOSCREEN
              and r.word(p1, "pagepic") == ui_id("DMC_UI_TITLEPIC"),
              "SAVE: play mode on the title page")
        check(90 <= g1 - g0 <= 120, f"play mode runs 35 tics per second ({g1 - g0} tics in 3 s)")
        check(same_screen(r.w("play1.ppm"), host_title(args.engine, args.nor, hw)),
              "the title page is the host's title frame")
        landscape(r.w("play1.ppm")).save(r.w("play1.png"))
    else:
        check(False, "play mode checkpoints reached")


def test_install(rn, build, args):
    print("install: no DOOM.DAT, folder UI with stock art")
    pk = os.path.join(build, "packs", "E1")
    res = os.path.join(rn.work, "DOOMRES.DAT")
    junk = os.path.join(rn.work, "README.TXT")
    with open(junk, "wb") as f:
        f.write(bytes((i * 7) & 0xFF for i in range(9000)))
    nor = fwnor.build([os.path.join(pk, "DOOMX.DAT"), os.path.join(build, "fwdoom", "renode", "DOOMCODE.DMC"), res],
                      [junk], "UI")
    os.remove(junk)
    r = Run(rn.renode, rn.work, build, "install", nor, boot_bench=1)
    r.until(1, 30, 1, "ovl_loads", 2)
    r.run_for("00:00:01")
    r.point("screen")
    r.press("SAVE")
    r.until(2, 30, 1, "ovl_loads", 2)
    r.run_for("00:00:03")
    r.point("usb", nor=True)
    r.press("SAVE")
    r.run_for("00:00:05")
    r.point("again")
    r.press("MODE")
    r.run_for("00:00:03")
    r.point("usb2")
    out = r.go()
    usb, again, usb2 = r.sram("usb"), r.sram("again"), r.sram("usb2")
    if "nor" not in out or None in (usb, again, usb2) or r.nor("usb") is None:
        check(False, "the run reached its checkpoints, see install_log.txt")
        return
    for tag in ("screen", "usb", "again", "usb2"):
        landscape(r.w(tag + ".ppm")).save(r.w(tag + ".png"))
    check(text_at(r.w("screen.ppm"), 0, 33, "DOOM.DAT is not on the 3 MB data drive.")
          and text_at(r.w("screen.ppm"), 0, 103, "SAVE: erase the data drive for DOOM,"),
          "the installer screen is shown")
    after = r.nor("usb")
    ok, root, ui = volume(rn.work, "install_vol.img", after)
    check(ok and root == ["UI"] and ui == [], f"SAVE erased the data drive: fsck.fat accepts it, root {root}, UI {ui}")
    check(after[0x108000:0x400000] == b"\xff" * 0x2F8000, "every cluster but the folder's reads 0xFF")
    check(after[0x100000:0x100000 + 512] == nor[0x100000:0x100000 + 512], "its boot sector is kept")
    check(after[:0x100000] == nor[:0x100000], "the firmware drive is unchanged")
    check(r.half(usb, "ovl_loads") == 2 and text_at(r.w("usb.ppm"), 0, 33, "Connect the meter to a computer.")
          and text_at(r.w("usb.ppm"), 0, 53, "Copy DOOM.DAT to the drive. Keep the"),
          "then the USB overlay shows the data drive screen")
    check(r.half(again, "ovl_loads") < r.half(usb, "ovl_loads")
          and text_at(r.w("again.ppm"), 0, 33, "DOOM.DAT is not on the 3 MB data drive."),
          "SAVE in USB mode: a reset, the installer again")
    check(text_at(r.w("usb2.ppm"), 0, 33, "Connect the meter to a computer."), "MODE opens the data drive on USB")
    check(out["nor"] == after, "as it is: the NOR is unchanged since the erase")


def test_hold(rn, build, args):
    print("hold: HOLD on the results screen")
    pk = os.path.join(build, "packs", "E1")
    res = os.path.join(rn.work, "DOOMRES.DAT")
    nor = fwnor.build([os.path.join(pk, "DOOMX.DAT"), os.path.join(build, "fwdoom", "renode", "DOOMCODE.DMC"), res],
                      [os.path.join(pk, "DOOM.DAT")], "UI")
    r = Run(rn.renode, rn.work, build, "hold", nor, boot_bench=1)
    r.menu(1, "HOLD")
    r.run_for("00:00:02")
    r.point("usb")
    r.press("SAVE")
    r.run_for("00:00:10")
    r.point("again")
    out = r.go()
    usb, again = r.sram("usb"), r.sram("again")
    if "nor" not in out or None in (usb, again):
        check(False, "the run reached its checkpoints, see hold_log.txt")
        return
    for tag in ("usb", "again"):
        landscape(r.w(tag + ".ppm")).save(r.w(tag + ".png"))
    check(text_at(r.w("usb.ppm"), 0, 33, "Connect the meter to a computer."), "HOLD opens the data drive on USB")
    check(r.word(again, "res_menus") == 1 and r.half(again, "ovl_loads") < r.half(usb, "ovl_loads")
          and text_at(r.w("again.ppm"), 0, 153, "HOLD: open the data drive on USB")
          and text_at(r.w("again.ppm"), 0, 163, "      (add DEMO*.LMP)"),
          "SAVE there: a reset, the results screen again with its HOLD option")
    ok, runs = save_dat(rn.work, "hold_vol.img", out["nor"])
    check(ok, f"the boot made SAVE.DAT in {runs} runs, fsck.fat and mdir accept it, both FATs equal")
    allowed = save_dat_bytes(out["nor"])
    d = [i for i in range(len(nor)) if nor[i] != out["nor"][i]]
    check(all(i in allowed for i in d), f"the NOR is unchanged but for the making of SAVE.DAT ({len(d)} bytes)")


def test_uiback(rn, build, args):
    print("uiback: DOOM.DAT and folder UI_OFF, a deathmatch DEMO3.LMP, then a panic")
    pk = os.path.join(build, "packs", "E1")
    res = os.path.join(rn.work, "DOOMRES.DAT")
    dm = os.path.join(rn.work, "DEMO3.LMP")
    with open(dm, "wb") as f:
        # v1.9, skill 3, E1M1, deathmatch, two players, one tic, the end marker
        f.write(bytes([109, 2, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0x80]))
    nor = fwnor.build([os.path.join(pk, "DOOMX.DAT"), os.path.join(build, "fwdoom", "renode", "DOOMCODE.DMC"), res],
                      [os.path.join(pk, "DOOM.DAT"), os.path.join(build, "fwdoom", "DEMO2.LMP"), dm], "UI_OFF")
    os.remove(dm)
    r = Run(rn.renode, rn.work, build, "uiback", nor, boot_bench=1, tics=70, frames=5)
    r.menu(1, "MODE")
    r.until(D["RS_DONE"], 300)
    r.point("done")
    r.panic("RENODE CRASH TEST")
    r.run_for("00:00:02")
    r.point("panic")
    out = r.go()
    if "nor" not in out or r.sram("panic") is None:
        check(False, "the run reached its checkpoints, see uiback_log.txt")
        return
    a = root_entry(nor, b"UI_OFF     ")
    d = diff_main(nor, out["nor"])
    check(a is not None and root_entry(out["nor"], b"UI         ") == a,
          "boot renamed UI_OFF back to UI")
    check(d and all(a + 2 <= i < a + 6 for i in d), f"nothing else on the data drive changed ({len(d)} bytes)")
    dec = doomres.decode(doomres.from_nor(r.w("nor_out.bin")))
    ok = dec["header"][D["H_STATE"]] == D["RS_DONE"] and [d["name"] for d in dec["demos"]] == [
        "DEMO1", "DEMO2.LMP", "DEMO3.LMP"]
    check(ok, "the demos ran with the folder UI back in place")
    check(ok and dec["demos"][2]["r"][D["D_STATUS"]] == D["DS_NODEMO"],
          "DEMO3.LMP (deathmatch) has a record that says it is not played, no panic")
    c = dec["crash"]
    check(c is not None and c[1] == "RENODE CRASH TEST" and c[0][D["C_EXC"]] == 0 and doomres.crash_now(dec),
          f"the panic wrote the crash record of this boot with its message ({c[1] if c else None})")
    check(text_at(r.w("panic.ppm"), 8 + 8 * 16, 158, "1"), "the fault screen says the crash record was written")
    landscape(r.w("panic.ppm")).save(r.w("panic.png"))


def test_doomx(rn, build, args):
    print("doomx: DOOM.DAT without DOOMX.DAT")
    pk = os.path.join(build, "packs", "E1")
    res = os.path.join(rn.work, "DOOMRES.DAT")
    nor = fwnor.build([os.path.join(build, "fwdoom", "renode", "DOOMCODE.DMC"), res],
                      [os.path.join(pk, "DOOM.DAT")], "UI_OFF")
    r = Run(rn.renode, rn.work, build, "doomx", nor, boot_bench=1)
    r.run_for("00:00:05")
    r.point("screen")
    out = r.go()
    if "nor" not in out or r.sram("screen") is None:
        check(False, "the run reached its checkpoints, see doomx_log.txt")
        return
    a = root_entry(nor, b"UI_OFF     ")
    check(root_entry(out["nor"], b"UI         ") == a, "UI_OFF went back to UI for the firmware drive")
    check(r.word(r.sram("screen"), "res_cur_state") == 0 and text_on(r.w("screen.ppm")) > 200,
          "the error screen stays, no demo runs")
    landscape(r.w("screen.ppm")).save(r.w("screen.png"))


# fe_state_t and the intermission state of fe_scr_t (port/game/fe.h): byte offsets
FE_MENU, FE_ITEM, FE_PX0, FE_MSG = 0, 1, 13, 9
MENU_OPTIONS, MENU_LOAD, MENU_SAVE = 2, 3, 4  # engine/m_menu.c
WI_STATE, WI_SP_STATE = 238, 239
KEY_ESCAPE = 27
MSG_BENCH = 0x83  # engine/m_menu.c
PLAY_EXIT = 200  # level time of the forced exit of the play run


def stack_used(r, sram):
    """Stack high-water of an SRAM dump: the bytes below __stack_top no longer painted
    (platform/startup.c)."""
    lo, top = r.syms["__stack_bottom"], r.syms["__stack_top"]
    for a in range(lo, top, 4):
        if struct.unpack_from("<I", sram, a - 0x20000000)[0] != 0x5AC3A55C:
            return top - a
    return 0


def host_frames(engine, nor, work, name, args, keys, tics, keep=None):
    """Frames (tic -> image, only the tics of keep when given) and the menu's panel lines (tic ->
    fields of --panel-out) of a host engine run with the keys (tic, key) pressed for one tic
    each."""
    d = os.path.join(work, name)
    shutil.rmtree(d, ignore_errors=True)
    fd = os.path.join(d, "frames")
    os.makedirs(fd)
    kf, pf = os.path.join(d, "keys"), os.path.join(d, "panels")
    with open(kf, "w") as f:
        for t, k in keys:
            f.write(f"{t} {k} down\n{t + 1} {k} up\n")
    subprocess.run([os.path.abspath(engine), "--nor", os.path.abspath(nor), "--keys", kf, "--tics", str(tics),
                    "--frame-out", fd, "--panel-out", pf] + args, check=False, stdout=subprocess.DEVNULL,
                   stderr=subprocess.DEVNULL, cwd=d)
    frames = {}
    for f in os.listdir(fd):
        if keep is not None and int(f[1:7]) not in keep:
            continue
        img = Image.open(os.path.join(fd, f))
        img.load()
        frames[int(f[1:7])] = img
    panels = {}
    with open(pf) as f:
        for line in f:
            w = [int(v) for v in line.split()]
            panels[w[0]] = w[1:]
    shutil.rmtree(d, ignore_errors=True)
    return frames, panels


def rgb565(img, x, y):
    if img.mode == "P":
        c = img.getpalette()[3 * img.getpixel((x, y)):3 * img.getpixel((x, y)) + 3]
        return round(c[0] * 31 / 255), round(c[1] * 63 / 255), round(c[2] * 31 / 255)
    r, g, b = img.getpixel((x, y))[:3]
    return r >> 3, g >> 2, b >> 3


def same_part(shot, frame, x0, x1, y0, y1):
    """Pixels x0..x1, y0..y1 of a screenshot (landscape) as the host frame shows them."""
    return all(rgb565(shot, x, y) == rgb565(frame, x, y) for y in range(y0, y1 + 1) for x in range(x0, x1 + 1))


def one_of(ppm, frames, part=(0, 319, 0, 239)):
    """The screenshot equals one of the host frames in part."""
    if not os.path.exists(ppm):
        return False
    shot, seen = landscape(ppm), set()
    for img in frames:
        key = img.tobytes()
        if key in seen:
            continue
        seen.add(key)
        if same_part(shot, img, *part):
            return True
    return False


def test_play(rn, build, args):
    print("play: title, menu, E1M1, intermission, E1M2, options, benchmark")
    pk = os.path.join(build, "packs", "E1")
    res = os.path.join(rn.work, "DOOMRES.DAT")
    with open(res, "wb") as f:
        f.write(bytes(D["RES_SIZE"]))
    nor = fwnor.build([os.path.join(pk, "DOOMX.DAT"), os.path.join(build, "fwdoom", "renode", "DOOMCODE.DMC"), res],
                      [os.path.join(pk, "DOOM.DAT")], "UI")
    r = Run(rn.renode, rn.work, build, "play", nor, demos=1, tics=300, frames=6, exits=[PLAY_EXIT])
    s = r.syms
    # the LCD when Escape opens the menu, by gametic: the frame before it
    r.hooks.append(("M_Responder", "b = machine.SystemBus; ev = self.GetRegisterUnsafe(0).RawValue; "
                    f"b.ReadDoubleWord(ev) == 0 and b.ReadDoubleWord(ev + 4) == {KEY_ESCAPE} and "
                    f"b.ReadDoubleWord({s['menuactive']:#x}) == 0 and "
                    f"machine['sysbus.spi2.lcd'].SaveScreenshot('{r.w('opened_')}%d.ppm' % "
                    f"b.ReadDoubleWord({s['gametic']:#x}))"))
    # every tic of the game: gametic, level time, map and the player's object, and where the
    # saves and loads run
    log = r.w("tics.txt")
    r.hooks.append(("P_Ticker", f"b = machine.SystemBus; mo = b.ReadDoubleWord({s['players']:#x}); "
                    f"f = open('{log}', 'a'); f.write('tic %d %d %d ' % (b.ReadDoubleWord({s['gametic']:#x}), "
                    f"b.ReadDoubleWord({s['leveltime']:#x}), b.ReadDoubleWord({s['gamemap']:#x})) + "
                    "' '.join(['%x' % b.ReadDoubleWord(mo + 4 * i) for i in range(15)]) + chr(10)); f.close()"))
    for fn, what in (("G_DoSaveGame", "save"), ("G_DoLoadGame", "load")):
        r.hooks.append((fn, f"f = open('{log}', 'a'); f.write('{what}' + chr(10)); f.close()"))
    # the menu opens and the game starts before the title page's 170 tics end
    r.until(1, 300, 1, "gametic")
    r.run_for("00:00:00.500")
    r.point("title")
    r.press("LIGHT")
    r.run_for("00:00:00.500")
    r.point("main")
    r.press("LIGHT")
    r.run_for("00:00:00.500")
    r.point("skill")
    r.press("LIGHT")
    r.until(35, 30, 1, "leveltime")
    r.point("e1m1")
    r.until_eq(1, 30, s["gamestate"])
    r.hold("POWER", "00:00:01.200")
    r.point("wimenu")
    r.hold("POWER", "00:00:01.200")
    r.point("wiclosed")
    # LIGHT completes the count, unless it is complete already
    p, n = BUTTONS["LIGHT"]
    r.py += [f"if bus.ReadByte({s['dmc_tic'] + WI_SP_STATE:#x}) < 10:",
             f"    monitor.Parse('gpioPort{p} OnGPIO {n} false')",
             "    monitor.Parse('emulation RunFor \"00:00:00.150\"')",
             f"    monitor.Parse('gpioPort{p} OnGPIO {n} true')"]
    r.until_eq(10, 30, s["dmc_tic"] + WI_SP_STATE, 1, ">=")
    r.run_for("00:00:00.500")
    r.point("stats")
    r.press("LIGHT")
    r.until_eq(1, 10, s["dmc_tic"] + WI_STATE, 1)
    r.run_for("00:00:00.500")
    r.point("entering")
    r.until(2, 30, 1, "gamemap")
    r.until(70, 30, 1, "leveltime")
    r.hold("POWER", "00:00:01.200")
    r.press("HOLD")
    r.press("LIGHT")
    r.run_for("00:00:00.500")
    r.point("options")
    # POWER closes the menu, POWER held 1 s opens it again on Options, HOLD twice: Save Game,
    # its page, slot 0
    r.press("POWER")
    r.hold("POWER", "00:00:01.200")
    r.press("HOLD")
    r.press("HOLD")
    r.press("LIGHT")
    r.run_for("00:00:00.500")
    r.point("savepage")
    r.press("LIGHT")
    r.run_for("00:00:00.500")
    # on a few steps, then Load Game (the menu opens on Save Game, V-A goes up), slot 0
    r.hold("VA", "00:00:01.500")
    r.run_for("00:00:00.300")
    r.point("walked")
    r.hold("POWER", "00:00:01.200")
    r.press("VA")
    r.press("LIGHT")
    r.run_for("00:00:00.500")
    r.point("loadpage")
    r.press("LIGHT")
    r.run_for("00:00:00.500")
    r.point("loaded")
    # the options (the menu opens on Load Game), HOLD to Benchmark, LIGHT
    r.hold("POWER", "00:00:01.200")
    r.press("VA")
    r.press("LIGHT")
    for _ in range(4):
        r.press("HOLD")
    r.press("LIGHT")
    r.run_for("00:00:00.500")
    r.point("prompt")
    r.press("LIGHT")
    r.menu(1, "SAVE", 600)
    r.run_for("00:00:01")
    r.point("title2")
    r.run_for("00:00:03")
    r.point("title3")
    out = r.go()
    pts = {t: r.sram(t) for t in ("title", "main", "skill", "e1m1", "wimenu", "wiclosed", "stats", "entering",
                                  "options", "savepage", "walked", "loadpage", "loaded", "prompt", "title2",
                                  "title3")}
    if "nor" not in out or None in pts.values():
        check(False, "the run reached its checkpoints, see play_log.txt")
        return
    for t in pts:
        landscape(r.w(t + ".ppm")).save(r.w(t + ".png"))
    hw = os.path.join(rn.work, "host")
    os.makedirs(hw, exist_ok=True)
    fe = s["fe"]

    def byte(t, off):
        return pts[t][fe + off - 0x20000000]

    # the title and the menu over it: the host with the same keys (LIGHT is fire in the game,
    # enter in the menu)
    frames, _ = host_frames(args.engine, args.nor, hw, "title", [], [(20, "KEY_FIRE"), (40, "KEY_ENTER")], 60)
    check(r.word(pts["title"], "gamestate") == GS_DEMOSCREEN
          and r.word(pts["title"], "res_cur_state") == D["RS_PLAY"] and one_of(r.w("title.ppm"), [frames[15]]),
          "boot to the title page in play mode, the host's title frame")
    check(r.word(pts["main"], "menuactive") == 1 and byte("main", FE_MENU) == 0
          and one_of(r.w("main.ppm"), [frames[t] for t in range(23, 40)]),
          "LIGHT: the main menu over the title, as the host shows it")
    check(byte("skill", FE_MENU) == 1 and byte("skill", FE_ITEM) == 0
          and one_of(r.w("skill.ppm"), [frames[t] for t in range(43, 60)]),
          "LIGHT: the skill page on I'm Too Young To Die, as the host shows it")
    check(r.word(pts["e1m1"], "gamemap") == 1 and r.word(pts["e1m1"], "gameskill") == 0
          and r.word(pts["e1m1"], "menuactive") == 0, "LIGHT: E1M1 on I'm Too Young To Die")
    check(r.word(pts["wimenu"], "gamestate") == 1 and r.word(pts["wimenu"], "menuactive") == 1
          and r.word(pts["wiclosed"], "menuactive") == 0,
          f"the exit at level time {PLAY_EXIT}, the menu over the intermission opened and closed")
    # the intermission of the same exit on the host, LIGHT after the count
    wi = ["-warp", "1", "1", "-skill", "1", "--exit-list", str(PLAY_EXIT)]
    frames, _ = host_frames(args.engine, args.nor, hw, "wi", wi, [(600, "KEY_FIRE")], 700)
    stats = [frames[t] for t in range(580, 600)]
    check(len({f.tobytes() for f in stats}) == 1, "the host's statistics are complete from tic 580")
    check(pts["stats"][s["dmc_tic"] + WI_SP_STATE - 0x20000000] == 10
          and pts["stats"][s["dmc_tic"] + WI_STATE - 0x20000000] == 0 and one_of(r.w("stats.ppm"), stats[-1:]),
          "LIGHT: the statistics complete, the host's frame")
    check(pts["entering"][s["dmc_tic"] + WI_STATE - 0x20000000] == 1
          and one_of(r.w("entering.ppm"), [frames[t] for t in range(605, 700)]),
          "LIGHT: the next map, a frame of the host's")
    def over_level(tag, page, frames, hp, label):
        """A menu page over E1M2: the panel and inside it the host's frames, outside it the
        LCD when the menu opened."""
        rp = [2 * byte(tag, FE_PX0), 2 * byte(tag, FE_PX0 + 1) + 1, byte(tag, FE_PX0 + 2),
              byte(tag, FE_PX0 + 3)]
        check(r.word(pts[tag], "gamemap") == 2 and byte(tag, FE_MENU) == page and rp == hp,
              f"{label} on the panel {rp}, the host's {hp}")
        check(one_of(r.w(tag + ".ppm"), frames, tuple(rp)), "inside the panel the host's page")
        gt = r.word(pts[tag], "gametic")
        opened = sorted(int(f[12:-4]) for f in os.listdir(rn.work)
                        if f.startswith("play_opened_") and int(f[12:-4]) <= gt)
        shot = landscape(r.w(tag + ".ppm"))
        before = landscape(r.w("opened_%d.ppm" % opened[-1])) if opened else None
        frozen = before is not None and all(rgb565(shot, x, y) == rgb565(before, x, y) for y in range(200)
                                            for x in range(320)
                                            if not (rp[0] <= x <= rp[1] and rp[2] <= y <= rp[3]))
        check(frozen, "outside the panel the view as the LCD showed it when the menu opened")

    # the options and Save Game over E1M2 on the host: the panel and what is in it
    frames, panels = host_frames(args.engine, args.nor, hw, "options", ["-warp", "1", "2", "-skill", "1"],
                                 [(60, "KEY_ESCAPE"), (70, "KEY_DOWNARROW"), (80, "KEY_ENTER"),
                                  (100, "KEY_ESCAPE"), (102, "KEY_ESCAPE"), (105, "KEY_DOWNARROW"),
                                  (110, "KEY_DOWNARROW"), (115, "KEY_ENTER")], 140)
    over_level("options", MENU_OPTIONS, [frames[t] for t in range(85, 100)], panels.get(99, [0] * 9)[5:9],
               "E1M2, POWER held 1 s, HOLD, LIGHT: the options")
    over_level("savepage", MENU_SAVE, [frames[t] for t in range(120, 140)], panels.get(139, [0] * 9)[5:9],
               "POWER, POWER held 1 s, HOLD twice, LIGHT: Save Game with every slot empty")
    # the save and the load: the first tic after either
    with open(log) as f:
        ticks = f.read().split("\n")

    def tic_after(marker):
        if marker not in ticks:
            return None
        return next((x.split()[1:] for x in ticks[ticks.index(marker) + 1:] if x.startswith("tic ")), None)

    saved, loaded = tic_after("save"), tic_after("load")
    walked = [x.split()[1:] for x in ticks[:ticks.index("load")] if x.startswith("tic ")][-1:] \
        if "load" in ticks else []
    check(saved is not None and loaded is not None and saved[1:] == loaded[1:] and walked
          and walked[0][3:] != saved[3:],
          "LIGHT on slot 0 saves, V-A walks on, the load brings back level time, map and the player's "
          f"object ({' '.join(saved[1:4]) if saved else None} at gametic {saved[0] if saved else None} and "
          f"{loaded[0] if loaded else None})")
    # Load Game on the host with a save of the same level time: the menu opens at level time e
    # and the save follows the tic after it closes
    e = int(saved[1]) - 1 if saved else 60
    frames, panels = host_frames(args.engine, args.nor, hw, "load", ["-warp", "1", "2", "-skill", "1"],
                                 [(e, "KEY_ESCAPE"), (e + 5, "KEY_DOWNARROW"), (e + 10, "KEY_DOWNARROW"),
                                  (e + 15, "KEY_DOWNARROW"), (e + 20, "KEY_ENTER"), (e + 30, "KEY_ENTER"),
                                  (e + 50, "KEY_ESCAPE"), (e + 55, "KEY_UPARROW"), (e + 60, "KEY_ENTER")],
                                 e + 80)
    over_level("loadpage", MENU_LOAD, [frames[t] for t in range(e + 65, e + 80)],
               panels.get(e + 79, [0] * 9)[5:9], "POWER held 1 s, V-A, LIGHT: Load Game with the save")
    check(r.word(pts["loaded"], "menuactive") == 0 and r.word(pts["loaded"], "gamemap") == 2,
          "LIGHT on slot 0: the menu closes on E1M2")
    ok, runs = save_dat(rn.work, "play_vol.img", out["nor"])
    check(ok, f"the boot made SAVE.DAT in {runs} runs, fsck.fat and mdir accept it, both FATs equal")
    check(pts["prompt"][s["messageToPrint"] - 0x20000000] == 1 and byte("prompt", FE_MSG) == MSG_BENCH,
          "HOLD to Benchmark, LIGHT: its prompt")
    dec = doomres.decode(doomres.from_nor(r.w("nor_out.bin")))
    doomres.report(dec)
    names = [d["name"] for d in dec["demos"]]
    rec = dec["demos"][0]["r"] if names == ["DEMO1"] else None
    check(rec is not None and rec[D["D_V_TICS"]] == 300 and rec[D["D_V_END"]] == D["DE_LIMIT"],
          f"LIGHT (y): the benchmark ran DEMO1 for 300 verified tics {names}")
    # the forced exit of the test settings applies to the verification pass too
    check(rec is not None
          and doomres.compare(dec, args.host, args.nor, None, hw, ["--exit-list", str(PLAY_EXIT)]),
          "every chained hash, the last hash and the last chain equal the host's (lockstep-skip hash, "
          "the same exit)")
    t2, t3 = pts["title2"], pts["title3"]
    g2, g3 = r.word(t2, "gametic"), r.word(t3, "gametic")
    check(r.word(t3, "res_cur_state") == D["RS_PLAY"] and r.word(t3, "gamestate") == GS_DEMOSCREEN
          and r.word(t3, "pagepic") == ui_id("DMC_UI_TITLEPIC") and 90 <= g3 - g2 <= 120,
          f"SAVE on the results screen: the title page in play mode ({g3 - g2} tics in 3 s)")
    check(same_screen(r.w("title3.ppm"), host_title(args.engine, args.nor, hw)),
          "the title page is the host's title frame")
    used = stack_used(r, t3)
    check(used <= STACK_GATE, f"stack high-water {used} B (at most {STACK_GATE:,})")


SAVE_TIC, SAVELOAD_TICS = 1000, 1500  # the saveload run


def chain_from(dec, host, nor, demo, first, work):
    """The record of demo against the host's stream from tic first on, chained from the start,
    as a load at that tic gives it."""
    d = dec["demos"][0]
    r = d["r"]
    tics = r[D["D_V_TICS"]]
    stream = [x for x in doomres.host_stream(host, nor, demo, first + tics, work) if x[0] >= first][:tics]
    stored, last, c = doomres.chain_of(stream)
    n = len(d["chain"])
    return len(stream) == tics and d["chain"] == stored[:n] and last == doomres.u64(r, D["D_V_HASH"]) \
        and c == doomres.u64(r, D["D_V_CHAIN"])


def test_saveload(rn, build, args):
    print("saveload: a test save in the verification pass of DEMO2.LMP, its load after a reboot")
    pk = os.path.join(build, "packs", "E1")
    res = os.path.join(rn.work, "DOOMRES.DAT")
    with open(res, "wb") as f:
        f.write(bytes(D["RES_SIZE"]))
    lmp = os.path.join(build, "fwdoom", "DEMO2.LMP")
    nor = fwnor.build([os.path.join(pk, "DOOMX.DAT"), os.path.join(build, "fwdoom", "renode", "DOOMCODE.DMC"), res],
                      [os.path.join(pk, "DOOM.DAT"), lmp], "none")
    hw = os.path.join(rn.work, "host")
    os.makedirs(hw, exist_ok=True)
    r = Run(rn.renode, rn.work, build, "saveload1", nor, boot_bench=1, demos=1 << 1, tics=SAVELOAD_TICS,
            frames=6, save_tic=SAVE_TIC)
    r.menu(1, "MODE")
    r.until(D["RS_DONE"], 900)
    out = r.go()
    if "nor" not in out:
        check(False, "the first boot ran to its end, see saveload1_log.txt")
        return
    dec = doomres.decode(doomres.from_nor(r.w("nor_out.bin")))
    doomres.report(dec)
    rec = dec["demos"][0]["r"] if [d["name"] for d in dec["demos"]] == ["DEMO2.LMP"] else None
    check(rec is not None and rec[D["D_V_TICS"]] == SAVELOAD_TICS
          and doomres.compare(dec, args.host, args.nor, os.path.dirname(lmp), hw),
          f"boot 1: {SAVELOAD_TICS} tics of DEMO2.LMP verified with a save at tic {SAVE_TIC}, every chained "
          "hash equal to the host's")
    ok, runs = save_dat(rn.work, "saveload_vol.img", out["nor"])
    sv = savefs.Vol(out["nor"]).file(b"SAVE    DAT")
    hdr = sv[0][:64] if sv else bytes(64)
    check(ok and hdr[:8] == b"DMCSAVE1" and hdr[8] == 1 and hdr[11] == 7,
          f"the boot made SAVE.DAT ({runs} runs), slot 0 holds the save of E1M1 with its order, the demo "
          f"cursor and gametic (flags {hdr[11]})")
    r = Run(rn.renode, rn.work, build, "saveload2", out["nor"], boot_bench=1, demos=1 << 1,
            tics=SAVELOAD_TICS, frames=6, load=1)
    r.menu(1, "MODE")
    r.until(D["RS_DONE"], 900)
    out = r.go()
    if "nor" not in out:
        check(False, "the second boot ran to its end, see saveload2_log.txt")
        return
    dec = doomres.decode(doomres.from_nor(r.w("nor_out.bin")))
    doomres.report(dec)
    names = [d["name"] for d in dec["demos"]]
    check(names == ["DEMO2.LMP"] and chain_from(dec, args.host, args.nor, lmp, SAVE_TIC + 1, hw),
          f"boot 2: the load of slot 0, then {dec['demos'][0]['r'][D['D_V_TICS']] if names else 0} tics "
          f"verified, every chained hash equal to the host's stream from tic {SAVE_TIC + 1} on")


# fin_state_t in fe_scr_t (port/game/fe.h): byte offsets
FIN_COUNT, FIN_STAGE = 160, 166
FINALE_EXIT = 35  # level time of the forced exits of the finale run
FINALE_TEXT = (100, 500, 1200)  # finale counts of the text stage screenshots
FINALE_ART = 30  # finale count of the art stage screenshot
DMC_STR_QUITMSG = 2  # gen/dmcdat.h


def test_finale(rn, build, args):
    print("finale: E1M1 to E1M8 through the intermissions, the finale, the menu over it")
    pk = os.path.join(build, "packs", "E1")
    res = os.path.join(rn.work, "DOOMRES.DAT")
    with open(res, "wb") as f:
        f.write(bytes(D["RES_SIZE"]))
    nor = fwnor.build([os.path.join(pk, "DOOMX.DAT"), os.path.join(build, "fwdoom", "renode", "DOOMCODE.DMC"), res],
                      [os.path.join(pk, "DOOM.DAT")], "UI")
    r = Run(rn.renode, rn.work, build, "finale", nor, demos=1, exits=[FINALE_EXIT] * 8)
    s = r.syms
    count, stage = s["dmc_tic"] + FIN_COUNT, s["dmc_tic"] + FIN_STAGE
    r.hooks.append(("M_Responder", "b = machine.SystemBus; ev = self.GetRegisterUnsafe(0).RawValue; "
                    f"b.ReadDoubleWord(ev) == 0 and b.ReadDoubleWord(ev + 4) == {KEY_ESCAPE} and "
                    f"b.ReadDoubleWord({s['menuactive']:#x}) == 0 and "
                    f"machine['sysbus.spi2.lcd'].SaveScreenshot('{r.w('opened.ppm')}')"))
    # the frames of the finale counts, each once it is on the LCD whole
    want = " or ".join(f"{c} <= c <= {c + 2}" for c in FINALE_TEXT)
    r.hooks.append(("I_FinishUpdate", f"b = machine.SystemBus; c = b.ReadDoubleWord({count:#x}); "
                    f"g = b.ReadDoubleWord({s['gamestate']:#x}); t = b.ReadByte({stage:#x}); "
                    f"g == 2 and b.ReadDoubleWord({s['menuactive']:#x}) == 0 and "
                    f"((t == 0 and ({want})) or (t == 1 and {FINALE_ART} <= c <= {FINALE_ART + 2})) and "
                    f"machine['sysbus.spi2.lcd'].SaveScreenshot('{r.w('shot')}%d_%d.ppm' % (t, c))"))
    r.until(1, 300, 1, "gametic")
    r.run_for("00:00:00.500")
    for _ in range(3):
        r.press("LIGHT")
        r.run_for("00:00:00.500")
    p, n = BUTTONS["LIGHT"]
    for m in range(2, 9):
        # the intermission: LIGHT completes the count, unless it is complete already, LIGHT the
        # next map
        r.until_eq(1, 60, s["gamestate"])
        r.run_for("00:00:00.300")
        r.py += [f"if bus.ReadByte({s['dmc_tic'] + WI_SP_STATE:#x}) < 10:",
                 f"    monitor.Parse('gpioPort{p} OnGPIO {n} false')",
                 "    monitor.Parse('emulation RunFor \"00:00:00.150\"')",
                 f"    monitor.Parse('gpioPort{p} OnGPIO {n} true')"]
        r.until_eq(10, 30, s["dmc_tic"] + WI_SP_STATE, 1, ">=")
        r.run_for("00:00:00.300")
        r.press("LIGHT")
        r.until(m, 30, 1, "gamemap")
        r.until_eq(0, 10, s["gamestate"])
    r.until_eq(2, 60, s["gamestate"])
    r.point("finale")
    r.until_eq(1, 120, stage, 1)
    r.until_eq(FINALE_ART + 5, 10, count, 4, ">=")
    r.point("art")
    r.hold("POWER", "00:00:01.200")
    r.run_for("00:00:00.500")
    r.point("menu")
    for _ in range(4):
        r.press("HOLD")
    r.press("LIGHT")
    r.run_for("00:00:00.500")
    r.point("quit")
    r.press("POWER")
    r.run_for("00:00:01")
    r.point("end")
    out = r.go(timeout=10800)
    pts = {t: r.sram(t) for t in ("finale", "art", "menu", "quit", "end")}
    if "nor" not in out or None in pts.values():
        check(False, "the run reached its checkpoints, see finale_log.txt")
        return
    for t in ("art", "menu", "quit", "end"):
        landscape(r.w(t + ".ppm")).save(r.w(t + ".png"))
    check(r.word(pts["finale"], "gamestate") == 2 and r.word(pts["finale"], "gamemap") == 8,
          "E1M1 to E1M8, each left at level time 35, LIGHT through the intermissions: the finale")
    hw = os.path.join(rn.work, "host")
    os.makedirs(hw, exist_ok=True)
    shots = sorted((int(m.group(1)), int(m.group(2))) for m in
                   (re.match(r"finale_shot(\d)_(\d+)\.ppm$", f) for f in os.listdir(rn.work)) if m)
    # the host's finale: the frame after tic FINALE_EXIT + c - 1 shows finale count c of the text
    # stage, the art stage restarts the count at its first tic
    end = 35 + 2000
    frames, panels = host_frames(args.engine, args.nor, hw, "finale",
                                 ["-warp", "1", "8", "-skill", "1", "--exit-list", str(FINALE_EXIT)],
                                 [(end - 60, "KEY_ESCAPE")], end, keep=set(range(1700, end)) | {
                                     FINALE_EXIT + c + d for c in FINALE_TEXT for d in range(-1, 5)})
    art0 = next((t for t in range(1700, end) if t in frames and t - 1 in frames
                 and frames[t].tobytes() != frames[t - 1].tobytes()
                 and all(frames[u].tobytes() == frames[t].tobytes() for u in range(t, t + 40))), None)
    for t, c in shots:
        base = FINALE_EXIT + c if t == 0 else (art0 or 0) + c
        good = one_of(r.w(f"shot{t}_{c}.ppm"), [frames[u] for u in range(base - 1, base + 2) if u in frames])
        check(good, f"{'the text stage' if t == 0 else 'the art stage'} at finale count {c}: the host's frame")
    stages = {t for t, _ in shots}
    check(len([1 for t, _ in shots if t == 0]) >= len(FINALE_TEXT) and 1 in stages,
          f"screenshots at {len(shots)} finale counts of both stages")
    fe = s["fe"]
    rp = [2 * pts["menu"][fe + FE_PX0 - 0x20000000], 2 * pts["menu"][fe + FE_PX0 + 1 - 0x20000000] + 1,
          pts["menu"][fe + FE_PX0 + 2 - 0x20000000], pts["menu"][fe + FE_PX0 + 3 - 0x20000000]]
    hp = panels.get(end - 1, [0] * 9)[5:9]
    check(r.word(pts["menu"], "menuactive") == 1 and rp == hp
          and one_of(r.w("menu.ppm"), [frames[t] for t in range(end - 50, end) if t in frames], tuple(rp)),
          f"POWER held 1 s: the main menu over the art stage on the panel {rp}, the host's {hp}")
    shot = landscape(r.w("menu.ppm"))
    before = landscape(r.w("opened.ppm")) if os.path.exists(r.w("opened.ppm")) else None
    check(before is not None and all(rgb565(shot, x, y) == rgb565(before, x, y) for y in range(200)
                                     for x in range(320) if not (rp[0] <= x <= rp[1] and rp[2] <= y <= rp[3])),
          "outside the panel the art stage as the LCD showed it when the menu opened")
    check(pts["quit"][s["messageToPrint"] - 0x20000000] == 1 and pts["quit"][fe + FE_MSG - 0x20000000]
          == DMC_STR_QUITMSG and r.word(pts["end"], "menuactive") == 0 and r.word(pts["end"], "gamestate") == 2,
          "HOLD four times, LIGHT: the quit prompt, POWER (n): the finale goes on")
    used = stack_used(r, pts["end"])
    check(used <= STACK_GATE, f"stack high-water {used} B (at most {STACK_GATE:,})")


ATTRACT_CYCLES = 12  # the menu opened and closed again over the attract demo


def test_attract(rn, build, args):
    print("attract: POWER held 6 s over the attract demo, the menu opened and closed while it plays")
    pk = os.path.join(build, "packs", "E1")
    res = os.path.join(rn.work, "DOOMRES.DAT")
    with open(res, "wb") as f:
        f.write(bytes(D["RES_SIZE"]))
    nor = fwnor.build([os.path.join(pk, "DOOMX.DAT"), os.path.join(build, "fwdoom", "renode", "DOOMCODE.DMC"), res],
                      [os.path.join(pk, "DOOM.DAT")], "UI")
    r = Run(rn.renode, rn.work, build, "attract", nor, demos=1)
    s = r.syms
    # the exception, PC and LR of a fault, and a panic
    log = r.w("fault.txt")
    r.hooks.append(("fault_c", f"f = open('{log}', 'a'); f.write('exception %d PC %x LR %x' % tuple("
                    "self.GetRegisterUnsafe(i).RawValue for i in range(3)) + chr(10)); f.close()"))
    r.hooks.append(("plat_panic", f"f = open('{log}', 'a'); f.write('panic LR %x' % "
                    "self.GetRegisterUnsafe(14).RawValue + chr(10)); f.close()"))
    r.until(1, 300, 1, "gametic")
    r.until_eq(1, 30, s["demoplayback"])
    r.run_for("00:00:01")
    r.point("demo")
    # the menu at 1 s, Escape and F10 at 4 s: the quit prompt, POWER (n) back to the demo
    r.hold("POWER", "00:00:06")
    r.point("quit")
    r.press("POWER")
    r.run_for("00:00:00.500")
    r.point("back")
    # POWER held 1 s opens the menu and closes it again, LIGHT opens it as any key in the demo
    for i in range(ATTRACT_CYCLES):
        if i % 2:
            r.press("LIGHT")
        else:
            r.hold("POWER", "00:00:01.200")
        r.point(f"open{i}")
        r.hold("POWER", "00:00:01.200")
        r.point(f"closed{i}")
    # a jump outside RAM and flash stops the machine in Renode, the run then waits for its timeout
    try:
        out = r.go(timeout=1800)
    except subprocess.TimeoutExpired:
        out = {}
    faults = ""
    if os.path.exists(log):
        with open(log) as f:
            faults = f.read().strip()
    check(not faults, f"no fault and no panic {faults.splitlines()}")
    tags = ["demo", "quit", "back"] + [f"{t}{i}" for i in range(ATTRACT_CYCLES) for t in ("open", "closed")]
    pts = {t: r.sram(t) for t in tags}
    if "nor" not in out or None in pts.values():
        check(False, "the run reached its checkpoints, see attract_log.txt")
        return
    fe = s["fe"]
    check(r.word(pts["demo"], "demoplayback") == 1 and r.word(pts["demo"], "gamestate") == 0
          and r.word(pts["demo"], "menuactive") == 0, "the attract demo plays")
    check(pts["quit"][s["messageToPrint"] - 0x20000000] == 1
          and pts["quit"][fe + FE_MSG - 0x20000000] == DMC_STR_QUITMSG,
          "POWER held 6 s: the quit prompt")
    check(r.word(pts["back"], "menuactive") == 0 and r.word(pts["back"], "messageToPrint") == 0
          and r.word(pts["back"], "demoplayback") == 1, "POWER (n): the attract demo goes on")
    opened = sum(r.word(pts[f"open{i}"], "menuactive") == 1 for i in range(ATTRACT_CYCLES))
    closed = sum(r.word(pts[f"closed{i}"], "menuactive") == 0 for i in range(ATTRACT_CYCLES))
    check(opened == closed == ATTRACT_CYCLES,
          f"the menu opened over the demo and closed again {ATTRACT_CYCLES} times ({opened} opened, {closed} closed)")


def exit_words(spec):
    """dmc_test.exits of an --exit-list (tests/lockstep/play.h): level times, bit 15 for a
    secret exit."""
    return [int(e.rstrip("s")) | (0x8000 if e.endswith("s") else 0) for e in spec.split(",")]


def multimap_lmp(work, oracle):
    """The demo of the lockstep workload exitlist as work/DEMO2.LMP: (path, tics), or None."""
    shutil.rmtree(work, ignore_errors=True)
    os.makedirs(work)
    run_lockstep.OUT = os.path.join(ROOT, "build", "lockstep")
    os.makedirs(run_lockstep.OUT, exist_ok=True)
    run_lockstep.WAD = run_lockstep.get_wad(None)
    # the oracle runs in work
    run_lockstep.OUT = work
    lmp = os.path.join(work, "DEMO2.LMP")
    tics = run_lockstep.exitlist_lmp(oracle, lmp, os.path.join(work, "probe"))
    return (lmp, tics) if tics else None


def test_multimap(rn, build, args):
    print("multimap: the E1 episode in one demo with forced exits, verification pass")
    pk = os.path.join(build, "packs", "E1")
    res = os.path.join(rn.work, "DOOMRES.DAT")
    with open(res, "wb") as f:
        f.write(bytes(D["RES_SIZE"]))
    md = os.path.join(rn.work, "multimap")
    made = multimap_lmp(md, args.oracle)
    if not made:
        check(False, "the oracle reaches the art stage of the finale in the exitlist demo")
        return
    lmp, tics = made
    exits = exit_words(run_lockstep.EXITLIST)
    nor = fwnor.build([os.path.join(pk, "DOOMX.DAT"), os.path.join(build, "fwdoom", "renode", "DOOMCODE.DMC"), res],
                      [os.path.join(pk, "DOOM.DAT"), lmp], "UI")
    r = Run(rn.renode, rn.work, build, "multimap", nor, boot_bench=1, demos=1 << 1, frames=6, exits=exits)
    r.menu(1, "MODE")
    r.until(D["RS_DONE"], 1200)
    r.point("done")
    out = r.go(timeout=10800)
    done = r.sram("done")
    if done is None or "nor" not in out:
        check(False, "the run reached its checkpoints, see multimap_log.txt")
        return
    check(r.word(done, "res_cur_state") == D["RS_DONE"], "the demo ran to the end (DOOMRES.DAT state done)")
    dec = doomres.decode(doomres.from_nor(r.w("nor_out.bin")))
    doomres.report(dec)
    names = [d["name"] for d in dec["demos"]]
    check(names == ["DEMO2.LMP"], f"DOOMRES.DAT has the record of DEMO2.LMP only {names}")
    if names != ["DEMO2.LMP"]:
        return
    d = dec["demos"][0]
    rec = d["r"]
    check(rec[D["D_V_TICS"]] == tics and rec[D["D_V_END"]] == D["DE_MARKER"],
          f"the {tics} tics of the demo verified to its end marker ({rec[D['D_V_TICS']]})")
    ev = rec[D["D_V_EVENTS"]:D["D_V_EVENTS"] + 3]
    check(ev == [0, 0, 0], f"no deferred wake-up, skipped transient or deferred mover {ev}")
    with open(os.path.join(pk, "report.json")) as f:
        rep = json.load(f)
    want = "".join(sorted({rep["plans"][f"E1M{m}"][2]["plan"] for m in range(1, 10)}))
    got = doomres.plans(rec[D["D_LEVEL"]])
    check(got == want, f"the levels set up in plans {got}, report.json gives {want} for the nine maps at skill 4")
    first = rep["plans"]["E1M1"][2]
    got = (rec[D["D_V_PAGES"]] & 0xFFFF, rec[D["D_V_ARENA"]])
    check(got == (first["pages"], first["arena"]),
          f"pages and level arena {got} are those of E1M1, report.json gives {(first['pages'], first['arena'])}")
    hw =os.path.join(rn.work, "host")
    os.makedirs(hw, exist_ok=True)
    ok = doomres.compare(dec, args.host, args.nor, md, hw, ["--exit-list", run_lockstep.EXITLIST])
    check(ok, "every chained hash, the last hash and the last chain equal the host's (lockstep-skip hash, "
          "the same exits)")
    shutil.rmtree(md, ignore_errors=True)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--renode", default=os.environ.get("RENODE", DEFAULT_RENODE))
    ap.add_argument("--build", default=os.path.join(ROOT, "build"))
    ap.add_argument("--only", choices=["demos", "install", "hold", "uiback", "doomx", "play", "saveload",
                                       "attract", "finale", "multimap"])
    a = ap.parse_args()
    build = os.path.abspath(a.build)
    a.host = os.path.join(build, "host", "dmcdoom_skip")
    a.engine = os.path.join(build, "host", "dmcdoom")
    a.oracle = os.path.join(build, "oracle", "dg_oracle")
    a.nor = os.path.join(build, "nor", "E1.img")
    work = os.path.join(build, "fwdoom", "renode-run")
    os.makedirs(work, exist_ok=True)
    renode = shutil.which(a.renode)
    if renode is None:
        sys.exit(f"Renode not found at {a.renode} (set RENODE)")
    rn = argparse.Namespace(renode=os.path.abspath(renode), work=work)
    with open(os.path.join(work, "DOOMRES.DAT"), "wb") as f:
        f.write(bytes(D["RES_SIZE"]))
    tests = {"install": test_install, "hold": test_hold, "doomx": test_doomx, "uiback": test_uiback,
             "demos": test_demos, "play": test_play, "saveload": test_saveload, "attract": test_attract,
             "finale": test_finale, "multimap": test_multimap}
    for name, fn in tests.items():
        if a.only == name or (a.only is None and name not in ("finale", "multimap")):
            fn(rn, build, a)
    if not failures:
        for f in os.listdir(work):
            if f.endswith(".bin"):
                os.remove(os.path.join(work, f))
    print("ALL PASSED" if not failures else f"{len(failures)} FAILED")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
