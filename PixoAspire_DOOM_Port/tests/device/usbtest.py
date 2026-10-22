#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Checks the DOOM firmware's USB mode on Linux: the meter's 3 MB data drive.

Usage: python3 -I tests/device/usbtest.py [--size KB] [--copy FILE]... [--no-test] [--wait S]
Run it on the PC while the meter shows "the data drive on USB". No root needed: it mounts
through udisksctl and touches the drive only through its file system.
  1. finds the block device of USB ID 2e3c:5720 with 3 MB (3,145,728 B) and refuses anything
     else (the bootloader's firmware drive has the same ID but 1 MB)
  2. prints the descriptors Linux saw (sysfs, lsusb -v when it runs without root) and the
     SCSI identity
  3. mounts the drive (or takes the mount the desktop made) and lists it
  4. speed test: writes a test file of --size KB (default 1024, less when the drive has less
     room) of random data, fsync, unmounts, mounts read only, reads it back and compares,
     MB/s of both, then removes it
  5. --copy FILE (DOOM.DAT, DEMO*.LMP, any number) copies the files to the root of the drive,
     each only when it fits, and compares them after a read only mount, they stay there
  6. unmounts. On an error the file being written is removed and the drive unmounted. Eject
     the drive on the PC (or press SAVE on the meter) to leave USB mode.
"""

import argparse
import glob
import os
import shutil
import subprocess
import sys
import time

VID, PID = "2e3c", "5720"
SIZE = 3 * 1024 * 1024
TEST_NAME = "USBTEST.BIN"


def usb_dir(block):
    """The sysfs directory of the USB device behind a block device, or None."""
    p = os.path.realpath(os.path.join("/sys/block", block, "device"))
    while p != "/":
        if os.path.exists(os.path.join(p, "idVendor")):
            return p
        p = os.path.dirname(p)
    return None


def attr(d, name):
    try:
        with open(os.path.join(d, name)) as f:
            return f.read().strip()
    except OSError:
        return ""


def drives():
    """(block name, size in bytes, USB sysfs directory) of every USB disk with our ID."""
    out = []
    for b in sorted(os.listdir("/sys/block")):
        u = usb_dir(b)
        if u and attr(u, "idVendor").lower() == VID and attr(u, "idProduct").lower() == PID:
            out.append((b, int(attr(os.path.join("/sys/block", b), "size") or 0) * 512, u))
    return out


def find(wait):
    t0 = time.monotonic()
    while True:
        found = drives()
        ours = [d for d in found if d[1] == SIZE]
        if len(ours) == 1:
            return ours[0]
        if len(ours) > 1:
            sys.exit(f"more than one drive {VID}:{PID} of 3 MB: {[d[0] for d in ours]}, unplug the others")
        if time.monotonic() - t0 > wait:
            for b, size, _ in found:
                print(f"/dev/{b} is {VID}:{PID} but {size} B: not the DOOM data drive (the bootloader's "
                      "firmware drive is 1 MB)")
            sys.exit(f"no {VID}:{PID} drive of {SIZE} B found, is the meter in USB mode?")
        time.sleep(1)


def describe(block, u):
    b = os.path.join("/sys/block", block)
    print(f"device /dev/{block}: {int(attr(b, 'size')) * 512} B, logical block "
          f"{attr(b, 'queue/logical_block_size')} B, removable {attr(b, 'removable')}")
    print(f"USB {attr(u, 'idVendor')}:{attr(u, 'idProduct')} at {os.path.basename(u)}: "
          f"manufacturer \"{attr(u, 'manufacturer')}\", product \"{attr(u, 'product')}\", "
          f"serial \"{attr(u, 'serial')}\"")
    print(f"  USB {attr(u, 'version')}, {attr(u, 'speed')} Mb/s, bcdDevice {attr(u, 'bcdDevice')}, "
          f"{attr(u, 'bNumConfigurations')} configuration, MaxPower {attr(u, 'bMaxPower')}")
    for i in sorted(glob.glob(u + "/*:1.*")):
        print(f"  interface {os.path.basename(i)}: class {attr(i, 'bInterfaceClass')} subclass "
              f"{attr(i, 'bInterfaceSubClass')} protocol {attr(i, 'bInterfaceProtocol')}, "
              f"{attr(i, 'bNumEndpoints')} endpoints, driver {os.path.basename(os.path.realpath(i + '/driver'))}")
        for e in sorted(glob.glob(i + "/ep_*")):
            print(f"    endpoint {attr(e, 'bEndpointAddress')}: {attr(e, 'type')} {attr(e, 'direction')}, "
                  f"{int(attr(e, 'wMaxPacketSize'), 16)} B")
    print(f"SCSI: vendor \"{attr(b, 'device/vendor')}\", model \"{attr(b, 'device/model')}\", "
          f"revision \"{attr(b, 'device/rev')}\"")
    if shutil.which("lsusb"):
        r = subprocess.run(["lsusb", "-v", "-s", f"{attr(u, 'busnum')}:{attr(u, 'devnum')}"],
                           capture_output=True, text=True)
        print("lsusb -v:")
        for line in (r.stdout + r.stderr).rstrip().splitlines():
            print("  " + line)


def mount_point(dev):
    with open("/proc/mounts") as f:
        for line in f:
            p = line.split()
            if p[0] == dev:
                return p[1].replace("\\040", " ").replace("\\011", "\t").replace("\\134", "\\"), p[3]
    return None, None


def udisks(*args):
    r = subprocess.run(["udisksctl"] + list(args) + ["--no-user-interaction"], capture_output=True, text=True)
    if r.returncode:
        sys.exit(f"udisksctl {' '.join(args)} failed: {(r.stdout + r.stderr).strip()}")


def mount(dev, ro, wait=10.0):
    """Mounts dev, read only or not, or takes the mount that is there, and returns the mount
    point. udisks may not know a drive yet that has just shown up (or one that took over the
    name of a drive that went away), and the desktop may mount it at the same time, so a
    failed udisksctl is tried again for up to wait seconds."""
    t0 = time.monotonic()
    while True:
        mp, opts = mount_point(dev)
        if mp and ("ro" in opts.split(",")) == ro:
            return mp
        args = ["unmount", "-b", dev] if mp else ["mount", "-b", dev] + (["-o", "ro"] if ro else [])
        r = subprocess.run(["udisksctl"] + args + ["--no-user-interaction"], capture_output=True, text=True)
        if r.returncode:
            if time.monotonic() - t0 > wait:
                sys.exit(f"udisksctl {' '.join(args)} failed: {(r.stdout + r.stderr).strip()}")
            time.sleep(0.25)


def unmount(dev):
    if mount_point(dev)[0]:
        udisks("unmount", "-b", dev)


def listing(mp):
    st = os.statvfs(mp)
    print(f"volume at {mp}: {st.f_blocks * st.f_frsize} B, {st.f_bavail * st.f_frsize} B free, "
          f"cluster {st.f_frsize} B")
    for root, dirs, files in os.walk(mp):
        dirs.sort()
        rel = os.path.relpath(root, mp)
        depth = 0 if rel == "." else rel.count(os.sep) + 1
        if rel != ".":
            print("  " + "  " * (depth - 1) + os.path.basename(root) + "/")
        for n in sorted(files):
            print("  " + "  " * depth + f"{n} {os.path.getsize(os.path.join(root, n))}")
        if depth >= 2:
            dirs[:] = []


def write_file(path, data):
    t0 = time.monotonic()
    with open(path, "wb") as f:
        f.write(data)
        f.flush()
        os.fsync(f.fileno())
    os.sync()
    return time.monotonic() - t0


def read_file(path):
    t0 = time.monotonic()
    with open(path, "rb") as f:
        data = f.read()
    return data, time.monotonic() - t0


def rate(n, t):
    return f"{t:.2f} s, {n / t / 1e6:.3f} MB/s" if t > 0 else "0 s"


def room(mp, name):
    """Bytes a file called name may take in the root of the volume: the free clusters, and
    those of a file of that name that it replaces. Also the cluster size."""
    st = os.statvfs(mp)
    free, cl = st.f_bavail * st.f_frsize, st.f_frsize
    old = os.path.join(mp, name)
    if os.path.isfile(old):
        free += -(-os.path.getsize(old) // cl) * cl
    return free, cl


class Drive:
    """The mounted drive, and the file being written, which is removed when anything fails."""

    def __init__(self, dev):
        self.dev, self.mp, self.ro, self.partial = dev, None, False, None

    def mount(self, ro):
        self.mp, self.ro = mount(self.dev, ro), ro
        return self.mp

    def unmount(self):
        unmount(self.dev)
        self.mp = None

    def write(self, name, data):
        path = os.path.join(self.mp, name)
        self.partial = path
        t = write_file(path, data)
        self.partial = None
        return t

    def remove(self, name):
        if self.ro:
            self.unmount()
            self.mount(False)
        os.remove(os.path.join(self.mp, name))
        os.sync()

    def cleanup(self):
        """After an error: the partial file removed, the drive unmounted, as far as it goes."""
        try:
            if self.partial:
                name, self.partial = os.path.basename(self.partial), None
                if self.mp is None:
                    self.mount(False)
                if os.path.exists(os.path.join(self.mp, name)):
                    self.remove(name)
                    print(f"removed the partial {name}")
        except (OSError, SystemExit) as e:
            print(f"could not remove the partial file: {e}")
        try:
            unmount(self.dev)
        except (OSError, SystemExit) as e:
            print(f"could not unmount {self.dev}: {e}")


def speed_test(d, kb):
    """The test file written, read back from a read only mount and compared, then removed.
    True when it was the same."""
    free, cl = room(d.mp, TEST_NAME)
    size = min(kb * 1024, (free // cl - 1) * cl)
    if size < 64 * 1024:
        print(f"speed test skipped: {free} B free on the drive")
        return True
    if size < kb * 1024:
        print(f"test file of {size // 1024} KB, the drive has {free} B free")
    test = os.urandom(size)
    t = d.write(TEST_NAME, test)
    print(f"wrote {TEST_NAME}, {size} B: {rate(size, t)}")
    d.unmount()
    data, t = read_file(os.path.join(d.mount(True), TEST_NAME))
    same = data == test
    print(f"read {TEST_NAME}, {len(data)} B (read only mount): {rate(len(data), t)}, "
          f"{'same' if same else 'DIFFERENT'}")
    d.remove(TEST_NAME)
    print(f"removed {TEST_NAME}")
    return same


def copy_files(d, paths):
    """The files copied when they fit, then read back from a read only mount and compared.
    True when every file was copied and is the same."""
    ok, done = True, []
    for p in paths:
        with open(p, "rb") as f:
            data = f.read()
        name = os.path.basename(p).upper()
        free, cl = room(d.mp, name)
        if len(data) > free:
            print(f"{name}: {len(data)} B does not fit, {free} B free on the drive: not copied")
            ok = False
            continue
        t = d.write(name, data)
        done.append((name, data))
        print(f"wrote {name}, {len(data)} B: {rate(len(data), t)}")
    if done:
        d.unmount()
        mp = d.mount(True)
        for name, want in done:
            data, t = read_file(os.path.join(mp, name))
            same = data == want
            ok &= same
            print(f"read {name}, {len(data)} B (read only mount): {rate(len(data), t)}, "
                  f"{'same' if same else 'DIFFERENT'}")
    return ok


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--size", type=int, default=1024, help="test file in KB (default 1024)")
    ap.add_argument("--copy", action="append", default=[], metavar="FILE", help="file to copy and verify")
    ap.add_argument("--no-test", action="store_true", help="skip the test file")
    ap.add_argument("--wait", type=int, default=30, help="seconds to wait for the drive (default 30)")
    a = ap.parse_args()
    for p in a.copy:
        if not os.path.isfile(p):
            sys.exit(f"{p}: no such file")
    names = [os.path.basename(p).upper() for p in a.copy]
    if TEST_NAME in names or len(set(names)) != len(names):
        sys.exit("--copy: file names must differ from each other and from " + TEST_NAME)

    block, size, u = find(a.wait)
    dev = "/dev/" + block
    if size != SIZE or attr(u, "idVendor").lower() != VID or attr(u, "idProduct").lower() != PID:
        sys.exit(f"{dev} is not the {VID}:{PID} drive of {SIZE} B, refusing")
    describe(block, u)

    d = Drive(dev)
    ok = False
    try:
        listing(d.mount(False))
        ok = True
        if not a.no_test and a.size > 0:
            ok &= speed_test(d, a.size)
        if a.copy:
            if d.ro:
                d.unmount()
                d.mount(False)
            ok &= copy_files(d, a.copy)
        if d.mp:
            listing(d.mp)
        d.unmount()
    except OSError as e:
        print(f"error: {e}")
        d.cleanup()
        print("FAILED")
        return 1
    except BaseException:
        d.cleanup()
        raise
    print(f"{dev} unmounted. Eject it on the PC or press SAVE on the meter to leave USB mode.")
    print("ALL OK" if ok else "FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
