#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Puts the original firmware of a FNIRSI DMC-100 back from Linux, after DOOM.

It needs the original firmware (the .bin file of a FNIRSI update) and a copy of the meter's
pictures, which the original firmware shows and DOOM.DAT takes the room of. tools/install.py
copies them to backup/UI before it deletes them. --pictures names another copy: the folder
UI copied from the data drive, or an image of the whole data drive (its folder UI or UI_OFF
is used, read with mtools).

Step 1 puts the pictures back. In DOOM open the menu (POWER), Options, USB drive and answer
yes (LIGHT): the meter opens its 3 MB data drive on USB. The script deletes DOOM's files
there (DOOM.DAT, SAVE.DAT with the saved games, DEMO*.LMP), copies the pictures into the
folder UI, reads them back and ejects the drive.

Step 2 flashes the original firmware. Start upgrade mode when the script asks for it. The
script deletes DOOM's files on the 1 MB firmware drive, copies the firmware as STOCK.BIN,
reads it back and unmounts the drive. Then SAVE on the meter flashes it.

DOOM.BIN is longer than the original firmware and covers the flash page where that firmware
keeps its settings (language, backlight, auto power off, the saved readings). STOCK.BIN is
the firmware padded to the end of that page, so the flash also erases it, and the firmware
starts with its defaults and asks for the language. With --flash-dump, a copy of the
meter's internal flash from before DOOM, the page from the copy is put back instead.

The drives are handled as tools/install.py does: through udisksctl, without root, and only
through their file systems. Files that are neither DOOM's nor the pictures are deleted only
with --force and only when they are in the way.

exit status: 0 done, 1 udisksctl or file system error, 2 wrong arguments, 3 a file to copy is
missing or not usable, 4 no udisksctl or mtools, 5 the meter did not show up or go away in
time, 6 no room or files in the way, 7 a copied file reads back different, 130 Ctrl-C
"""
import argparse
import os
import struct
import subprocess
import sys
import time

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import install  # noqa: E402
from install import (DATA, E_FILE, E_UDISKS, E_VERIFY, FIRMWARE, FW_FILES, WINDOWS, Fail,  # noqa: E402
                     Session, TestDirs, Usb, nclusters, secs, show, bar, rate)

FLASH = 0x08000000
FLASH_SIZE = 128 * 1024
APP = 0x08007000            # where the bootloader flashes a .BIN file
SETTINGS = 0x0801F400       # the settings page of the original firmware
PAGE = 1024
APP_MAX = SETTINGS - APP
RAM_END = 0x20008000
NAME = "STOCK.BIN"


def hint(kind, dev, size):
    if kind is FIRMWARE and size == DATA.size:
        return (f"{dev} is the 3 MB data drive, not the firmware drive. Upgrade mode shows it when the "
                "data drive has no folder UI: run the script with --pictures-only first.")
    if kind is DATA and size == FIRMWARE.size:
        return (f"{dev} is the firmware drive: the meter is in upgrade mode. Unplug the cable and press "
                "SAVE on the meter (it switches off), then switch it on and open the USB drive in DOOM.")
    return install.hint(kind, dev, size)


class Entry(install.Entry):
    """What happens to a file or folder in the root of a drive, as install.Entry."""

    def classify(self, kind, u):
        if self.dir and u == WINDOWS:
            return "spare", "Windows metadata"
        if kind is FIRMWARE:
            if not self.dir and u in FW_FILES:
                return "old", "a DOOM file"
            if not self.dir and u == NAME:
                return "old", "an earlier copy"
        elif self.dir and u == "UI":
            return "keep", "the pictures go here"
        elif self.dir and u == "UI_OFF":
            ui = os.path.isdir(os.path.join(os.path.dirname(self.path), "UI"))
            return "keep", "not used, UI is there" if ui else "becomes UI"
        elif not self.dir and u == "DOOM.DAT":
            return "old", "DOOM's game data"
        elif not self.dir and u == "SAVE.DAT":
            return "old", "DOOM's saved games"
        elif not self.dir and u.startswith("DEMO") and u.endswith(".LMP"):
            return "old", "a benchmark demo"
        return "other", "not a DOOM file"


# ---- the files to copy

def mtools(img, *args):
    try:
        r = subprocess.run(args, capture_output=True, env=dict(os.environ, MTOOLS_SKIP_CHECK="1"))
    except FileNotFoundError:
        raise Fail(E_UDISKS, f"{args[0]} is missing. It reads the image {img}: install mtools (on Ubuntu: "
                   "sudo apt install mtools), or copy the folder UI out of the image and name the folder.")
    if r.returncode:
        raise Fail(E_FILE, f"{args[0]} could not read {img}: {r.stderr.decode(errors='replace').strip()}")
    return r.stdout


def from_image(img):
    """The files and folders of the folder UI (or UI_OFF) in an image of the data drive."""
    root = [ln.rstrip("/").rsplit("/", 1)[-1] for ln in mtools(img, "mdir", "-b", "-i", img, "::/")
            .decode(errors="replace").splitlines() if ln.endswith("/")]
    ui = next((n for want in ("UI", "UI_OFF") for n in root if n.upper() == want), None)
    if ui is None:
        raise Fail(E_FILE, f"the image {img} has no folder UI or UI_OFF.")
    top = f"::/{ui}/"
    files, dirs = [], []
    for ln in mtools(img, "mdir", "-/", "-b", "-i", img, top).decode(errors="replace").splitlines():
        if not ln.startswith(top):
            continue
        rel = ln[len(top):]
        if rel.endswith("/"):
            dirs.append(rel.rstrip("/"))
        elif rel:
            files.append((rel, mtools(img, "mtype", "-i", img, ln)))
    return files, dirs


def from_folder(d):
    """The files and folders of a copy of the folder UI. A folder that holds UI (or UI_OFF)
    stands for it."""
    for want in ("UI", "UI_OFF"):
        sub = next((n for n in os.listdir(d) if n.upper() == want and os.path.isdir(os.path.join(d, n))), None)
        if sub:
            return install.read_tree(os.path.join(d, sub))
    return install.read_tree(d)


def load_pictures(src):
    if not os.path.exists(src):
        raise Fail(E_FILE, f"{src} is missing. tools/install.py copies the pictures there before it deletes "
                   "them, name another copy with --pictures.")
    files, dirs = from_folder(src) if os.path.isdir(src) else from_image(src)
    if not files:
        raise Fail(E_FILE, f"no pictures in {src}.")
    size = sum(len(d) for _, d in files)
    print(f"Pictures {src}: {len(files)} files in {len(dirs)} folders, {size:,d} B")
    return files, dirs


def load_firmware(path):
    """The original firmware, checked to be an app for the meter that ends before the
    settings page."""
    try:
        with open(path, "rb") as f:
            fw = f.read()
    except OSError as e:
        raise Fail(E_FILE, f"cannot read {path}: {e.strerror}.")
    if not 1024 <= len(fw) <= APP_MAX:
        raise Fail(E_FILE, f"{path} has {len(fw):,d} B, the original firmware has at most {APP_MAX:,d} B.")
    sp, reset = struct.unpack_from("<II", fw)
    if not (0x20000000 < sp <= RAM_END and reset & 1 and APP <= reset - 1 < APP + len(fw)):
        raise Fail(E_FILE, f"{path} does not start as firmware for the meter (stack {sp:#x}, start {reset:#x}).")
    print(f"Firmware {path}: {len(fw):,d} B")
    return fw


def load_settings(path):
    """The settings page of a copy of the internal flash, None without one."""
    if path is None:
        print("Settings: erased by the flash, the firmware starts with its defaults and asks for the language")
        return None
    try:
        with open(path, "rb") as f:
            d = f.read()
    except OSError as e:
        raise Fail(E_FILE, f"cannot read {path}: {e.strerror}.")
    if len(d) != FLASH_SIZE:
        raise Fail(E_FILE, f"{path} has {len(d):,d} B, a copy of the internal flash has {FLASH_SIZE:,d} B.")
    page = d[SETTINGS - FLASH:SETTINGS - FLASH + PAGE]
    # the firmware takes the page when its first halfword is 0, else it starts with its defaults
    if page[:2] != b"\0\0":
        raise Fail(E_FILE, f"{path} has no settings of the original firmware (or DOOM was on the meter "
                   "already). Leave out --flash-dump: the firmware then starts with its defaults.")
    print(f"Settings: put back from {path}")
    return page


def stock_image(fw, page):
    """The firmware padded to the end of the settings page, which is page or erased."""
    return fw + b"\xff" * (SETTINGS - APP - len(fw)) + (page or b"\xff" * PAGE)


# ---- the two steps

def pictures_size(files, dirs, cs):
    """Clusters the pictures take in the folder UI, a folder's entry counted as 64 B."""
    entries = {}
    for p in [f for f, _ in files] + dirs:
        parent = os.path.dirname(p)
        entries[parent] = entries.get(parent, 0) + 1
    return sum(nclusters(len(d), cs) for _, d in files) + sum(nclusters((n + 2) * 64, cs)
                                                             for n in entries.values())


def copy_tree(s, ui, files, dirs):
    """Writes the pictures into the folder ui with one progress bar."""
    def write(stop, handed):
        for d in dirs:
            os.makedirs(os.path.join(ui, d), exist_ok=True)
        for rel, data in files:
            if stop.is_set():
                return
            with open(os.path.join(ui, rel), "wb") as f:
                f.write(data)
            handed[0] += len(data)
        os.sync()

    install.write_watched(s, "pictures", sum(len(d) for _, d in files), write)


def verify_tree(ui, files):
    """Reads the pictures back, returns those that are missing or different."""
    t0, done, total, bad = time.monotonic(), 0, sum(len(d) for _, d in files), []
    for rel, data in files:
        try:
            with open(os.path.join(ui, rel), "rb") as f:
                got = f.read()
        except FileNotFoundError:
            got = None
        if got != data:
            bad.append(rel)
        done += len(data)
        show(bar("pictures", done, total, time.monotonic() - t0))
    t = time.monotonic() - t0
    show(f"  {'pictures':12s} " + (f"{len(bad)} of {len(files)} files DIFFERENT: {', '.join(bad[:5])}" if bad
                                    else f"same, read in {rate(total, t)}"), end=True)
    return bad


def find_dir(mp, name):
    return next((n for n in os.listdir(mp) if n.upper() == name and os.path.isdir(os.path.join(mp, n))), None)


def pictures_step(s, a, pics, title):
    files, dirs = pics
    t0 = time.monotonic()
    print(f"\n{title}: put the pictures back")
    print("In DOOM open the menu (POWER), Options, USB drive and answer yes (LIGHT). Unplug the game")
    print("pad first, the meter has one USB port. On DOOM's installer screen press MODE instead, on a")
    print("data error screen SAVE.")
    s.dev = install.wait_for(s, DATA, a.timeout, hint)
    t = time.monotonic()
    mp = s.b.mount(s.dev, False)
    print(f"Mounted {s.dev} at {mp} ({secs(time.monotonic() - t)}).")
    cs = s.b.space(mp, DATA)[0]
    install.make_room(s, DATA, mp, pictures_size(files, dirs, cs), False, Entry)
    ui, off = find_dir(mp, "UI"), find_dir(mp, "UI_OFF")
    if ui is None and off is not None:
        os.rename(os.path.join(mp, off), os.path.join(mp, "UI"))
        print(f"Renamed {off} to UI.")
    elif ui is None:
        os.mkdir(os.path.join(mp, "UI"))
    ui = os.path.join(mp, find_dir(mp, "UI"))
    size = sum(len(d) for _, d in files)
    print(f"Copying {size // 1024:,d} KB, about {secs(size / 1024 / DATA.speed)} at the data drive's "
          f"{DATA.speed} KB/s:")
    copy_tree(s, ui, files, dirs)
    t = time.monotonic()
    s.b.unmount(s.dev)
    mp = s.b.mount(s.dev, True)
    print(f"Reading back after a read only mount ({secs(time.monotonic() - t)}):")
    bad = verify_tree(os.path.join(mp, find_dir(mp, "UI")), files)
    s.b.unmount(s.dev)
    if bad:
        raise Fail(E_VERIFY, "pictures read back different. The data drive is still open: run "
                   "python3 -I tools/restore.py --pictures-only to copy them again.")
    t = time.monotonic()
    try:
        s.b.eject(s.dev)
    except SystemExit as e:
        s.dev = None
        print(f"Could not eject the data drive: {e.code}")
        print("Press SAVE on the meter to close the drive.")
    else:
        s.dev = None
        print(f"Ejected the data drive ({secs(time.monotonic() - t)}).")
    print(f"Step took {secs(time.monotonic() - t0)}.")
    print("DONE: pictures back. The meter restarts into DOOM's installer: do NOT press SAVE there,")
    print("it erases the pictures again.")


def firmware_step(s, a, fw, title):
    t0 = time.monotonic()
    print(f"\n{title}: flash the original firmware")
    print("  1. Unplug the USB cable and switch the meter off (on DOOM's installer hold POWER for 2 s,")
    print("     in DOOM use Quit Game in the menu).")
    print("  2. Hold HOLD and SAVE and press POWER: the meter starts in upgrade mode.")
    print("  3. Plug the cable in.")
    s.dev = install.wait_for(s, FIRMWARE, a.timeout, hint)
    bad, _ = install.install_files(s, FIRMWARE, [(NAME, fw)], Entry)
    if bad:
        mp = s.b.mount(s.dev, False)
        if os.path.exists(os.path.join(mp, NAME)):
            os.remove(os.path.join(mp, NAME))
            os.sync()
        s.b.unmount(s.dev)
        raise Fail(E_VERIFY, f"{NAME} read back different. It was deleted, so SAVE on the meter flashes "
                   "nothing. The firmware drive is still there: run the script again.")
    print("Press SAVE on the meter to flash.")
    t = install.wait_gone(s, FIRMWARE, a.timeout)
    s.dev = None
    print(f"The firmware drive went away after {secs(t)}, step took {secs(time.monotonic() - t0)}.")
    print("DONE: original firmware flashed. Switch the meter on.")


def main(argv=None):
    sys.stdout.reconfigure(line_buffering=True)
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--firmware", metavar="FILE",
                    help="the original firmware, the .bin file of a FNIRSI update")
    ap.add_argument("--pictures", metavar="PATH", default=os.path.join(install.ROOT, "backup", "UI"),
                    help="the meter's pictures: the folder UI copied from the data drive, or an image of "
                    "the data drive (default: backup/UI of this checkout, where tools/install.py copies "
                    "them)")
    ap.add_argument("--flash-dump", metavar="FILE",
                    help="a copy of the meter's internal flash (128 KB) from before DOOM: the original "
                    "firmware's settings are put back from it")
    one = ap.add_mutually_exclusive_group()
    one.add_argument("--pictures-only", action="store_true", help="only step 1, put the pictures back")
    one.add_argument("--firmware-only", action="store_true", help="only step 2, flash the firmware")
    ap.add_argument("--force", action="store_true",
                    help="delete files that are neither DOOM's nor the pictures when they are in the way "
                    "(they are listed first)")
    ap.add_argument("--timeout", type=float, default=600, metavar="S",
                    help="give up when the meter does not show up or go away within S seconds, 0 waits "
                    "for ever (default 600)")
    ap.add_argument("--device", action="append", default=[], metavar="DEV",
                    help="use this block device instead of looking for the meter's USB ID, as in "
                    "tools/install.py")
    ap.add_argument("--test-dir", metavar="DIR",
                    help="test without the meter: the folders DIR/firmware and DIR/data stand for the "
                    "mounted drives")
    a = ap.parse_args(argv)
    if not a.pictures_only and not a.firmware:
        ap.error("name the original firmware with --firmware (or use --pictures-only)")
    if a.pictures_only and a.flash_dump:
        ap.error("--flash-dump goes with the firmware, not with --pictures-only")
    if a.test_dir and a.device:
        ap.error("--test-dir and --device do not go together")
    if a.timeout < 0:
        ap.error("--timeout must not be negative")

    s = Session(TestDirs(a.test_dir) if a.test_dir else Usb(a.device), a.force)

    def body():
        fw = None if a.pictures_only else stock_image(load_firmware(a.firmware), load_settings(a.flash_dump))
        pics = None if a.firmware_only else load_pictures(a.pictures)
        install.need_udisks(a)
        both = not a.firmware_only and not a.pictures_only
        if pics:
            pictures_step(s, a, pics, "Step 1 of 2" if both else "Pictures")
        if fw:
            firmware_step(s, a, fw, "Step 2 of 2" if both else "Firmware")

    return install.run(s, "DOOM for the FNIRSI DMC-100: back to the original firmware", body)


if __name__ == "__main__":
    sys.exit(main())
