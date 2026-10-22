#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Installs DOOM on a FNIRSI DMC-100 from Linux, over the stock firmware or an older DOOM.

Step 1 flashes the firmware. Start upgrade mode on the meter when the script asks for it.
The script waits for the meter's 1 MB firmware drive (USB ID 2E3C:5720), deletes the older
DOOM files there, copies DOOMX.DAT (when the build has one), DOOMCODE.DMC, DOOMRES.DAT (with
--res) and DOOM.BIN, reads them back after a read only mount and unmounts the drive. Then
SAVE on the meter flashes DOOM.BIN.

Step 2 copies the game data. Switch the meter on: DOOM shows its installer, or a data error
after an update, and opens the meter's 3 MB data drive on USB. On the installer MODE opens it
as it is: the script copies the meter's own pictures (the folder UI) to backup/UI (--backup),
which tools/restore.py puts back, and deletes them, DOOM.DAT needs the room. Once that copy
exists, SAVE there is faster: it erases the drive first. The script replaces DOOM.DAT, reads
it back and ejects the drive. The meter restarts into DOOM.

The drives are mounted through udisksctl (udisks2), without root, and touched only through
their file systems. To make room the script deletes the System Volume Information folder of
Windows and, on the data drive, benchmark demos (DEMO*.LMP). Other files are deleted only
with --force and only when they are in the way: when they take the room DOOM needs, or when
they are .BIN files on the firmware drive, which the bootloader would flash. The meter's own
pictures are deleted only once the computer has a copy.

exit status: 0 done, 1 udisksctl or file system error, 2 wrong arguments, 3 a file is missing
in the build folder, 4 no udisksctl, 5 the meter did not show up or go away in time, 6 no
room or files in the way, 7 a copied file reads back different, 130 Ctrl-C
"""
import argparse
import errno
import os
import re
import shutil
import sys
import threading
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tests", "device"))
sys.dont_write_bytecode = True
import usbtest  # noqa: E402

E_IO, E_FILE, E_UDISKS, E_TIMEOUT, E_ROOM, E_VERIFY, E_INT = 1, 3, 4, 5, 6, 7, 130
CLUSTER = 4096
SAVE_CLUSTERS = 16  # SAVE.DAT, which the meter makes on the data drive at its next start
CHUNK = 64 * 1024
POLL = 0.5
FW_FILES = ("DOOM.BIN", "DOOMCODE.DMC", "DOOMX.DAT", "DOOMRES.DAT")
WINDOWS = "SYSTEM VOLUME INFORMATION"
PICTURES = ("UI", "UI_OFF")  # the meter's pictures, UI_OFF after DOOM's rename


class Kind:
    """One of the meter's two drives: its size on USB, its clusters of 4 KB, the folder that
    stands for it with --test-dir and about how fast it takes data (KB/s)."""

    def __init__(self, name, size, clusters, folder, speed):
        self.name, self.size, self.clusters, self.folder, self.speed = name, size, clusters, folder, speed


FIRMWARE = Kind("firmware drive", 1024 * 1024, 249, "firmware", 40)
DATA = Kind("data drive", usbtest.SIZE, 761, "data", 60)


class Fail(Exception):
    """An error that ends the script with an exit status."""

    def __init__(self, status, msg):
        super().__init__(msg)
        self.status = status


def secs(t):
    return f"{t:.1f} s" if t < 60 else f"{int(t // 60)} min {int(t % 60):02d} s"


def nclusters(size, cs):
    return -(-size // cs)


def sysfs_block(dev, name):
    """A sysfs attribute of a block device, None when it cannot be read."""
    try:
        with open(f"/sys/class/block/{os.path.basename(os.path.realpath(dev))}/{name}") as f:
            return f.read()
    except OSError:
        return None


def fat_clusters(path, cs):
    """Clusters a file, or a folder with all it holds, takes on a FAT volume. An entry of a
    folder is counted as 64 B, for its long name."""
    if not os.path.isdir(path):
        return nclusters(os.path.getsize(path), cs)
    names = os.listdir(path)
    return nclusters((len(names) + 2) * 64, cs) + sum(fat_clusters(os.path.join(path, n), cs) for n in names)


class Usb:
    """The meter's drives on USB, found in sysfs by their USB ID and size as
    tests/device/usbtest.py does, or the block devices of --device (by size only). They are
    mounted through udisksctl."""

    def __init__(self, devices):
        self.devices = devices

    def connected(self):
        """(device, size in bytes) of each drive that may be one of the meter's."""
        if self.devices:
            sizes = [(d, int(sysfs_block(d, "size") or 0) * 512) for d in self.devices]
            return [(d, size) for d, size in sizes if size]
        return [("/dev/" + b, size) for b, size, _ in usbtest.drives()]

    def mount(self, dev, ro):
        return usbtest.mount(dev, ro)

    def unmount(self, dev):
        usbtest.unmount(dev)

    def eject(self, dev):
        """Powers the drive off as the eject of a file manager does, which makes the meter
        close it. A loop device of --device is detached instead."""
        if os.path.basename(os.path.realpath(dev)).startswith("loop"):
            usbtest.udisks("loop-delete", "-b", dev)
        else:
            usbtest.udisks("power-off", "-b", dev)

    def space(self, mp, kind):
        """Cluster size, free and all clusters of the mounted volume."""
        st = os.statvfs(mp)
        return st.f_frsize, st.f_bavail, st.f_blocks

    def written(self, dev):
        """Bytes the kernel has written to the device so far."""
        st = (sysfs_block(dev, "stat") or "").split()
        return int(st[6]) * 512 if len(st) > 6 else None


class TestDirs:
    """--test-dir DIR, for tests without the meter: the folder DIR/firmware stands for the
    mounted firmware drive and DIR/data for the data drive, each while it exists. The room
    is counted as on the meter's FAT12 volumes. Mounting, unmounting and ejecting do
    nothing."""

    def __init__(self, d):
        self.d = d

    def connected(self):
        return [(os.path.join(self.d, k.folder), k.size) for k in (FIRMWARE, DATA)
                if os.path.isdir(os.path.join(self.d, k.folder))]

    def mount(self, dev, ro):
        return dev

    def unmount(self, dev):
        pass

    def eject(self, dev):
        pass

    def space(self, mp, kind):
        used = sum(fat_clusters(os.path.join(mp, n), CLUSTER) for n in os.listdir(mp))
        return CLUSTER, kind.clusters - used, kind.clusters

    def written(self, dev):
        return None


class Session:
    """The drive in use, and the file being written, which is removed when anything fails."""

    def __init__(self, backend, force):
        self.b, self.force, self.dev, self.partial, self.worker = backend, force, None, None, None

    def cleanup(self):
        if self.dev is None:
            return
        if self.worker is not None and self.worker.is_alive():
            name = os.path.basename(self.partial) if self.partial else "A file"
            print(f"{name} is still being written, so the drive stays mounted. "
                  "Run the script again, it replaces the file.")
            return
        try:
            if self.partial:
                name, self.partial = os.path.basename(self.partial), None
                p = os.path.join(self.b.mount(self.dev, False), name)
                if os.path.exists(p):
                    os.remove(p)
                    os.sync()
                    print(f"Removed the incomplete {name}.")
        except (OSError, SystemExit) as e:
            print(f"Could not remove the incomplete file: {e}")
        try:
            self.b.unmount(self.dev)
        except (OSError, SystemExit) as e:
            print(f"Could not unmount {self.dev}: {e}")


def show(line, end=False):
    """A progress line, redrawn in place on a terminal. Elsewhere only the last one is
    printed."""
    if sys.stdout.isatty():
        print("\r" + line + "\x1b[K", end="\n" if end else "", flush=True)
    elif end:
        print(line)


def bar(name, done, total, t):
    frac = min(done / total, 1.0) if total else 1.0
    n = int(frac * 20)
    line = (f"  {name:12s} [{'#' * n}{'.' * (20 - n)}] {frac * 100:3.0f} %  "
            f"{done // 1024:5,d} of {total // 1024:,d} KB  {secs(t)}")
    if t >= 2 and done:
        line += f", {done / t / 1024:.0f} KB/s, {secs((total - done) * t / done)} left"
    return line


def rate(n, t):
    return f"{secs(t)}, {n / t / 1024:.0f} KB/s" if t >= 0.1 else secs(t)


# ---- waiting for the meter

def hint(kind, dev, size):
    if kind is FIRMWARE and size == DATA.size:
        return (f"{dev} is the 3 MB data drive, not the firmware drive. If the firmware is flashed "
                "already, run the script with --data-only.")
    if kind is DATA and size == FIRMWARE.size:
        return (f"{dev} is the firmware drive: the meter is still in upgrade mode. Press SAVE on "
                "the meter to leave it, then switch the meter on.")
    return f"{dev} has USB ID 2E3C:5720 but {size:,d} B: not a drive of the meter."


def wait_for(s, kind, timeout, hint=hint):
    """Waits until exactly one drive of the kind is connected and returns its device. hint
    says what another drive of the meter's USB ID is."""
    print(f"Waiting for the {kind.name} (USB 2E3C:5720, {kind.size >> 20} MB)...")
    t0, seen = time.monotonic(), set()
    while True:
        found = s.b.connected()
        ours = [d for d, size in found if size == kind.size]
        if len(ours) == 1:
            print(f"Found the {kind.name} {ours[0]} after {secs(time.monotonic() - t0)}.")
            return ours[0]
        if len(ours) > 1:
            raise Fail(E_IO, f"more than one {kind.name}: {', '.join(ours)}. Unplug the others.")
        for d, size in found:
            if (d, size) not in seen:
                seen.add((d, size))
                print(hint(kind, d, size))
        if timeout and time.monotonic() - t0 > timeout:
            raise Fail(E_TIMEOUT, f"no {kind.name} after {timeout:g} s. " + (
                "Is the meter in upgrade mode with the cable in?" if kind is FIRMWARE
                else "Does the meter say \"the data drive on USB\"?"))
        time.sleep(POLL)


def wait_gone(s, kind, timeout):
    """Waits until the drive in use is gone, returns the time it took."""
    t0 = time.monotonic()
    while any(d == s.dev and size == kind.size for d, size in s.b.connected()):
        if timeout and time.monotonic() - t0 > timeout:
            raise Fail(E_TIMEOUT, f"the {kind.name} is still there after {timeout:g} s. The firmware is on "
                       "it: press SAVE on the meter to flash it.")
        time.sleep(POLL)
    return time.monotonic() - t0


# ---- the files on a drive

class Entry:
    """A file or folder in the root of a drive and what happens to it."""

    def __init__(self, kind, mp, name, cs):
        self.name, self.path = name, os.path.join(mp, name)
        self.dir = os.path.isdir(self.path)
        self.n = fat_clusters(self.path, cs)
        self.what, self.why = self.classify(kind, name.upper())
        self.gone = self.blocks = False

    def classify(self, kind, u):
        """'old': a DOOM file this script replaces or removes, 'spare': deleted when the room
        is needed, 'keep', 'other': deleted only with --force and only when it is in the
        way."""
        if self.dir and u == WINDOWS:
            return "spare", "Windows metadata"
        if kind is FIRMWARE:
            if not self.dir and u in FW_FILES:
                return "old", "an older DOOM file"
        elif self.dir and u in PICTURES:
            return "keep", "the meter needs this folder"
        elif not self.dir and u == "DOOM.DAT":
            return "old", "older game data"
        elif not self.dir and u == "SAVE.DAT":
            return "keep", "DOOM's save slots"
        elif not self.dir and re.fullmatch(r"DEMO\d\.LMP", u):
            return "spare", "a benchmark demo"
        return "other", "not a DOOM file"

    def line(self):
        act = "delete" if self.gone else "IN THE WAY" if self.blocks else "keep"
        size = "folder" if self.dir else f"{os.path.getsize(self.path):,d} B"
        name = self.name + ("/" if self.dir else "")
        cl = "cluster " if self.n == 1 else "clusters"
        return f"  {name:28s} {size:>11s} {self.n:4d} {cl}  {act}: {self.why}"


def make_room(s, kind, mp, need, save, entry=Entry):
    """Lists the drive with what happens to each entry (an entry class says that) and deletes
    what has to go for need clusters. Raises Fail, before deleting anything, when the room
    cannot be made."""
    cs, free, total = s.b.space(mp, kind)
    items = [entry(kind, mp, n, cs) for n in sorted(os.listdir(mp), key=str.upper)]
    avail = free
    for e in items:
        if e.what == "old":
            e.gone, avail = True, avail + e.n
    for e in items:
        if e.what == "spare" and avail < need:
            e.gone, avail = True, avail + e.n
    others = [e for e in items if e.what == "other"]
    short = avail < need
    for e in others:
        e.blocks = short or (kind is FIRMWARE and e.name.upper().endswith(".BIN"))
    blocking = [e for e in others if e.blocks]
    if s.force:
        for e in blocking:
            e.gone, e.blocks, avail = True, False, avail + e.n
    print(f"Files on the {kind.name} ({mp}):")
    for e in items:
        print(e.line())
    if not items:
        print("  (none)")
    print(f"  {free} of {total} clusters of {cs // 1024} KB free, {avail} after deleting, {need} needed")
    if save:
        print(f"  ({SAVE_CLUSTERS} of them for SAVE.DAT, which the meter makes at its next start)")
    if avail + sum(e.n for e in blocking if not s.force) < need:
        raise Fail(E_ROOM, f"the {kind.name} has {avail} clusters free after deleting, {need} are needed.")
    if blocking and not s.force:
        names = ", ".join(e.name for e in blocking)
        why = ("The bootloader flashes the last .BIN file it finds, so the one to flash must be the only "
               "one." if not short else "The files to copy need the room they take.")
        raise Fail(E_ROOM, f"files that are not DOOM's are in the way on the {kind.name}: {names}. {why} "
                   "Delete them, or run the script again with --force, which deletes them.")
    gone = [e for e in items if e.gone]
    if not gone:
        return
    t = time.monotonic()
    for e in gone:
        if e.dir:
            shutil.rmtree(e.path)
        else:
            os.remove(e.path)
    os.sync()
    cs, free, _ = s.b.space(mp, kind)
    print(f"Deleted {', '.join(e.name for e in gone)} ({secs(time.monotonic() - t)}).")
    if free < need:
        raise Fail(E_ROOM, f"the {kind.name} has {free} clusters free after deleting, {need} are needed.")


def write_watched(s, name, total, write):
    """Runs write(stop, handed) with a progress bar of total bytes under the name. A thread
    runs it: it writes and waits for the sync, keeps the bytes handed to the kernel in
    handed[0] and returns early once stop is set. The progress comes from the device's write
    counter, so it shows what has reached the meter, not what is in the page cache."""
    stop, err, handed = threading.Event(), [], [0]

    def work():
        try:
            write(stop, handed)
        except BaseException as e:
            err.append(e)

    base = s.b.written(s.dev)
    w = threading.Thread(target=work, daemon=True)
    s.worker = w
    t0 = time.monotonic()
    w.start()
    try:
        while w.is_alive():
            w.join(0.25)
            now = s.b.written(s.dev) if base is not None else None
            done = now - base if now is not None else handed[0]
            if w.is_alive():
                show(bar(name, max(0, min(done, total - 1)), total, time.monotonic() - t0))
    except KeyboardInterrupt:
        stop.set()
        print(("\n" if sys.stdout.isatty() else "") + "Ctrl-C: waiting until the drive has the data sent "
              "so far (Ctrl-C again quits at once).")
        w.join()
        raise
    s.worker = None
    if err:
        raise err[0]
    show(f"  {name:12s} {total:9,d} B in {rate(total, time.monotonic() - t0)}", end=True)


def read_tree(d):
    """The files in a folder and below it as (path in it, bytes), and its folders."""
    files, dirs = [], []
    for top, ds, fs in os.walk(d):
        ds.sort()
        rel = os.path.relpath(top, d)
        dirs += [os.path.normpath(os.path.join(rel, n)) for n in ds]
        for n in sorted(fs):
            with open(os.path.join(top, n), "rb") as f:
                files.append((os.path.normpath(os.path.join(rel, n)), f.read()))
    return files, dirs


def back_up_pictures(s, a):
    """Copies the meter's pictures (a folder UI or UI_OFF that is not empty) to the computer
    and empties the folder on the drive. The first copy goes to BACKUP/UI, one that differs
    from it next to it, named by the date and time."""
    mp = s.b.mount(s.dev, False)
    for n in sorted(os.listdir(mp)):
        src = os.path.join(mp, n)
        if n.upper() not in PICTURES or not os.path.isdir(src) or not os.listdir(src):
            continue
        t = time.monotonic()
        tree = read_tree(src)
        size = sum(len(d) for _, d in tree[0])
        print(f"The meter's pictures ({n}/, {len(tree[0])} files, {size:,d} B) read in "
              f"{rate(size, time.monotonic() - t)}.")
        dest = os.path.join(a.backup, "UI")
        if os.path.isdir(dest) and read_tree(dest) == tree:
            print(f"  The same as the copy in {dest}.")
        else:
            if os.path.exists(dest):
                dest += time.strftime("-%Y%m%d-%H%M%S")
            os.makedirs(dest)
            for d in tree[1]:
                os.makedirs(os.path.join(dest, d), exist_ok=True)
            for rel, data in tree[0]:
                with open(os.path.join(dest, rel), "wb") as f:
                    f.write(data)
            os.sync()
            if read_tree(dest) != tree:
                raise Fail(E_VERIFY, f"the copy in {dest} reads back different. The pictures are still on "
                           "the meter.")
            print(f"  Copied to {dest}, tools/restore.py puts them back.")
        for e in os.listdir(src):
            p = os.path.join(src, e)
            if os.path.isdir(p):
                shutil.rmtree(p)
            else:
                os.remove(p)
        os.sync()
        print(f"  Deleted them on the meter, the folder {n} stays: the meter needs it.")


def copy_file(s, mp, name, data):
    """Writes one file with a progress bar."""
    path = os.path.join(mp, name)

    def write(stop, handed):
        with open(path, "wb") as f:
            for i in range(0, len(data), CHUNK):
                if stop.is_set():
                    return
                f.write(data[i:i + CHUNK])
                handed[0] = min(i + CHUNK, len(data))
            f.flush()
            os.fsync(f.fileno())

    s.partial = path
    write_watched(s, name, len(data), write)
    s.partial = None


def verify_file(mp, name, data):
    """Reads one file back and compares it, True when it is the same."""
    t0, got = time.monotonic(), bytearray()
    try:
        with open(os.path.join(mp, name), "rb") as f:
            while True:
                b = f.read(CHUNK)
                if not b:
                    break
                got += b
                show(bar(name, len(got), len(data), time.monotonic() - t0))
    except FileNotFoundError:
        show(f"  {name:12s} MISSING", end=True)
        return False
    t = time.monotonic() - t0
    if got == data:
        show(f"  {name:12s} same, read in {rate(len(got), t)}", end=True)
        return True
    n = min(len(got), len(data))
    at = next((i for i in range(n) if got[i] != data[i]), n)
    show(f"  {name:12s} DIFFERENT: {len(got):,d} of {len(data):,d} B read, first difference at "
         f"byte {at:,d}", end=True)
    return False


def install_files(s, kind, files, entry=Entry):
    """Mounts the drive, makes room (make_room with the entry class), copies the files in
    their order and compares them after a read only mount, then unmounts it. Returns the
    names that read back different and those that were on the drive."""
    t = time.monotonic()
    mp = s.b.mount(s.dev, False)
    print(f"Mounted {s.dev} at {mp} ({secs(time.monotonic() - t)}).")
    names = os.listdir(mp)
    cs = s.b.space(mp, kind)[0]
    need = sum(nclusters(len(d), cs) for _, d in files)
    save = kind is DATA and not any(n.upper() == "SAVE.DAT" for n in names)
    make_room(s, kind, mp, need + (SAVE_CLUSTERS if save else 0), save, entry)
    size = sum(len(d) for _, d in files)
    print(f"Copying {size // 1024:,d} KB, about {secs(size / 1024 / kind.speed)} at the {kind.name}'s "
          f"{kind.speed} KB/s:")
    for name, data in files:
        copy_file(s, mp, name, data)
    t = time.monotonic()
    os.sync()
    s.b.unmount(s.dev)
    mp = s.b.mount(s.dev, True)
    print(f"Reading back after a read only mount ({secs(time.monotonic() - t)}):")
    bad = [name for name, data in files if not verify_file(mp, name, data)]
    s.b.unmount(s.dev)
    return bad, names


# ---- the two steps

def firmware_step(s, a, files, title):
    t0 = time.monotonic()
    print(f"\n{title}: flash the firmware")
    print("  1. Unplug the USB cable and switch the meter off (in DOOM: Quit Game in the menu, on")
    print("     its installer and error screens hold POWER for 2 s).")
    print("  2. Hold HOLD and SAVE and press POWER: the meter starts in upgrade mode.")
    print("  3. Plug the cable in.")
    s.dev = wait_for(s, FIRMWARE, a.timeout)
    bad, _ = install_files(s, FIRMWARE, files)
    if bad:
        mp = s.b.mount(s.dev, False)
        if os.path.exists(os.path.join(mp, "DOOM.BIN")):
            os.remove(os.path.join(mp, "DOOM.BIN"))
            os.sync()
        s.b.unmount(s.dev)
        raise Fail(E_VERIFY, f"{', '.join(bad)} read back different. DOOM.BIN was deleted, so SAVE on the "
                   "meter flashes nothing. The firmware drive is still there: run the script again.")
    print("Press SAVE on the meter to flash.")
    t = wait_gone(s, FIRMWARE, a.timeout)
    s.dev = None
    print(f"The firmware drive went away after {secs(t)}, step 1 took {secs(time.monotonic() - t0)}.")
    print("DONE: firmware flashed")


def data_step(s, a, doom, title):
    t0 = time.monotonic()
    backup = os.path.join(a.backup, "UI")
    print(f"\n{title}: copy the game data")
    print("Switch the meter on.")
    if os.path.isdir(backup) and os.listdir(backup):
        print(f"On the installer screen press SAVE: it erases the meter's pictures, {backup} has a copy.")
    else:
        print("On the installer screen press MODE: the script first copies the meter's pictures to")
        print(f"{backup}, the original firmware needs them.")
    print("On a data error screen press SAVE.")
    print("If DOOM starts right away, its game data is in place already: stop the script with Ctrl-C.")
    s.dev = wait_for(s, DATA, a.timeout)
    back_up_pictures(s, a)
    bad, names = install_files(s, DATA, [("DOOM.DAT", doom)])
    if bad:
        raise Fail(E_VERIFY, "DOOM.DAT read back different. The data drive is still open: run "
                   "python3 -I tools/install.py --data-only to copy it again.")
    t = time.monotonic()
    try:
        s.b.eject(s.dev)
    except SystemExit as e:
        s.dev = None
        print(f"Could not eject the data drive: {e.code}")
        print("DONE: DOOM installed. Press SAVE on the meter, it restarts into DOOM.")
        return
    s.dev = None
    print(f"Ejected the data drive ({secs(time.monotonic() - t)}), step 2 took "
          f"{secs(time.monotonic() - t0)}.")
    print("DONE: DOOM installed, the meter restarts into DOOM")
    if any(n.upper() == "UI_OFF" for n in names):
        print("The drive has the folder UI_OFF: if the meter is in upgrade mode, unplug the cable and press "
              "SAVE, then switch the meter on.")


def load_build(a):
    """The files of the firmware drive in the order they are copied, and DOOM.DAT."""
    d = os.path.abspath(a.build)
    names = []
    if not a.data_only:
        x = ["DOOMX.DAT"] if os.path.lexists(os.path.join(d, "DOOMX.DAT")) else []
        names = x + ["DOOMCODE.DMC"] + (["DOOMRES.DAT"] if a.res else []) + ["DOOM.BIN"]
    if not a.firmware_only:
        names.append("DOOM.DAT")
    missing = [n for n in names if not os.path.isfile(os.path.join(d, n))]
    if missing:
        raise Fail(E_FILE, f"missing in {d}: {', '.join(missing)}. Build them with make -j$(nproc) "
                   "fw-package, or name the folder with --build.")
    files = []
    print(f"Build {d}:")
    for n in names:
        with open(os.path.join(d, n), "rb") as f:
            files.append((n, f.read()))
        print(f"  {n:12s} {len(files[-1][1]):9,d} B  {'data' if n == 'DOOM.DAT' else 'firmware'} drive")
    if not a.data_only and "DOOMX.DAT" not in names:
        print("  no DOOMX.DAT, all the game data is in DOOM.DAT")
    doom = None if a.firmware_only else files.pop()[1]
    return files, doom


def need_udisks(a):
    if not a.test_dir and not shutil.which("udisksctl"):
        raise Fail(E_UDISKS, "udisksctl is missing. It mounts the meter's drives without root: install "
                   "udisks2 (on Ubuntu: sudo apt install udisks2).")
    if a.device:
        print(f"Looking only at {', '.join(a.device)}, without checking the USB ID.")


def run(s, title, body):
    """Prints the title, runs body() and turns its errors into an exit status."""
    t0 = time.monotonic()
    print(title)
    try:
        if not sys.platform.startswith("linux"):
            raise Fail(E_IO, "this script runs on Linux only.")
        body()
    except Fail as e:
        print(f"ERROR: {e}")
        s.cleanup()
        return e.status
    except SystemExit as e:
        # the udisksctl helpers of usbtest.py end with a message
        if not isinstance(e.code, str):
            raise
        busy = " Close file manager windows on the drive and try again." if "busy" in e.code else ""
        print(f"ERROR: {e.code}{busy}")
        s.cleanup()
        return E_IO
    except OSError as e:
        print(f"\nERROR: {e}")
        s.cleanup()
        return E_ROOM if e.errno == errno.ENOSPC else E_IO
    except KeyboardInterrupt:
        print("\nStopped (Ctrl-C).")
        s.cleanup()
        return E_INT
    print(f"Total time {secs(time.monotonic() - t0)}.")
    return 0


def main(argv=None):
    sys.stdout.reconfigure(line_buffering=True)
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--build", metavar="DIR", default=os.path.join(ROOT, "build", "device"),
                    help="the files of make fw-package (default: build/device of this checkout, "
                    "build/sw/device for a build of your own WAD)")
    one = ap.add_mutually_exclusive_group()
    one.add_argument("--firmware-only", action="store_true", help="only step 1, flash the firmware")
    one.add_argument("--data-only", action="store_true", help="only step 2, copy DOOM.DAT")
    ap.add_argument("--backup", metavar="DIR", default=os.path.join(ROOT, "backup"),
                    help="where the meter's pictures are copied before they are deleted, in the folder UI "
                    "(default: backup of this checkout)")
    ap.add_argument("--res", action="store_true",
                    help="also copy DOOMRES.DAT to the firmware drive, which keeps the Benchmark's results")
    ap.add_argument("--force", action="store_true",
                    help="delete files that are not DOOM's when they are in the way (they are listed first)")
    ap.add_argument("--timeout", type=float, default=600, metavar="S",
                    help="give up when the meter does not show up or go away within S seconds, 0 waits "
                    "for ever (default 600)")
    ap.add_argument("--device", action="append", default=[], metavar="DEV",
                    help="use this block device, for example a loop device with a copy of a drive, "
                    "instead of looking for the meter's USB ID. The size must match. Give it twice for "
                    "both drives.")
    ap.add_argument("--test-dir", metavar="DIR",
                    help="test without the meter: the folders DIR/firmware and DIR/data stand for the "
                    "mounted drives")
    a = ap.parse_args(argv)
    if a.res and a.data_only:
        ap.error("--res copies to the firmware drive, it does not go with --data-only")
    if a.test_dir and a.device:
        ap.error("--test-dir and --device do not go together")
    if a.timeout < 0:
        ap.error("--timeout must not be negative")

    s = Session(TestDirs(a.test_dir) if a.test_dir else Usb(a.device), a.force)

    def body():
        files, doom = load_build(a)
        need_udisks(a)
        both = not a.firmware_only and not a.data_only
        if not a.data_only:
            firmware_step(s, a, files, "Step 1 of 2" if both else "Firmware")
        if not a.firmware_only:
            data_step(s, a, doom, "Step 2 of 2" if both else "Game data")

    return run(s, "DOOM for the FNIRSI DMC-100: install", body)


if __name__ == "__main__":
    sys.exit(main())
