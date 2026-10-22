#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Host tests of the USB drive of the DOOM firmware (port/usb) with build/usb/usb_host.

NOR images come from tests/renode/fwnor.py (both volumes with the meter's geometry), the 3 MB
volume is checked as a file system with fsck.fat and mtools.
  protocol  usb_host suite: enumeration, the attach and mount sequence, reads and writes over
            the whole volume, out of range LBAs, unsupported commands and their sense data,
            the cases of the Bulk-Only Transport, invalid CBWs and resets, the host letting go
            (stop, eject, configuration 0)
  erase     the quick format of a volume with the meter's stock art: fsck.fat accepts it, the
            root holds an empty folder UI only, the boot sector is kept, every cluster but the
            folder's reads 0xFF, erased in 64 KB blocks where a block is whole, and a block
            written there with WRITE(10) needs no erase. Also a volume without a FAT12 boot
            sector (the meter's geometry is written) and one of another FAT12 geometry (kept).
  copy      DOOM.DAT of the E1 pack copied onto the erased volume: mtools copies it onto an
            image of the volume, the 4 KB sectors that changed go to the drive with WRITE(10).
            Only those of the FATs and the root directory need an erase. The volume must equal
            mtools' image, fsck.fat accepts it, mtools reads DOOM.DAT back unchanged, the folder
            UI is there, img_init and img_verify of the firmware's data layer accept the data,
            and the NOR outside the volume is unchanged.
  ui        the folder UI when the drive is left: UI_OFF renamed back, a new folder when there
            is none, nothing done when there is one, errors without a FAT12 volume and on a full
            volume.

usage: test_usb.py --build BUILD
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
sys.path.insert(0, os.path.join(ROOT, "tests", "renode"))
sys.dont_write_bytecode = True
import fwnor  # noqa: E402

VOL, VOL_END, SECTOR = 0x100000, 0x400000, 4096
ENV = dict(os.environ, MTOOLS_SKIP_CHECK="1", MTOOLS_NO_VFAT="1")
failures = []


def check(cond, msg):
    print(("  ok   " if cond else "  FAIL ") + msg)
    if not cond:
        failures.append(msg)


class Work:
    def __init__(self, build):
        self.dir = os.path.join(build, "usb", "work")
        shutil.rmtree(self.dir, ignore_errors=True)
        os.makedirs(self.dir)
        self.tool = os.path.join(build, "usb", "usb_host")
        self.pack = os.path.join(build, "packs", "E1")

    def p(self, name):
        return os.path.join(self.dir, name)

    def write(self, name, data):
        with open(self.p(name), "wb") as f:
            f.write(data)
        return self.p(name)

    def read(self, name):
        with open(self.p(name), "rb") as f:
            return f.read()

    def usb(self, *args):
        r = subprocess.run([self.tool] + [self.p(a) if a.endswith((".img", ".rec")) else a for a in args],
                           capture_output=True, text=True)
        return r.returncode, r.stdout + r.stderr

    def volume(self, nor_name, out_name):
        """The data drive of a NOR image as an image file of its own."""
        return self.write(out_name, self.read(nor_name)[VOL:VOL_END])


def fsck(path):
    r = subprocess.run(["fsck.fat", "-n", path], capture_output=True, text=True)
    return r.returncode == 0, (r.stdout + r.stderr).strip().splitlines()[-1:]


def mtools(*args):
    r = subprocess.run(list(args), capture_output=True, env=ENV)
    return r.returncode, r.stdout


def root_names(vol, path="::/"):
    """Names in a directory of a volume image (mdir -b gives one path per line)."""
    rc, out = mtools("mdir", "-b", "-i", vol, path)
    if rc:
        return None
    return sorted(os.path.basename(line.rstrip("/")) for line in out.decode().split("\n") if line.strip())


def changed_sectors(a, b, lo=0, hi=None):
    hi = len(a) if hi is None else hi
    return [o for o in range(lo, hi, SECTOR) if a[o:o + SECTOR] != b[o:o + SECTOR]]


def bpb(nor):
    return struct.unpack_from("<HBHBHHBH", nor, VOL + 11)


def clusters(nor):
    """NOR addresses of the first cluster, of the end of the clusters and the cluster size."""
    bps, spc, rsv, nfat, nroot, total, _, fatsz = bpb(nor)
    data = VOL + (rsv + nfat * fatsz) * bps + (nroot * 32 + bps - 1) // bps * bps
    size = spc * bps
    return data, data + (VOL + total * bps - data) // size * size, size


def check_erased(w, name, rc, out):
    """Checks of a volume after vol_format: every whole 4 KB sector of the clusters but the
    folder's reads 0xFF, erased in 64 KB blocks where a block is whole, and the checks of
    usb_host format after it (the progress, a WRITE(10) of a block of the clusters without an
    erase) pass."""
    for line in out.splitlines():
        if "FAIL" in line:
            print("    " + line)
    check(rc == 0, "the progress reaches its total, which covers the clusters, then WRITE(10) writes a "
          "block of the clusters")
    nor = w.read(name)
    data, end, size = clusters(nor)
    start, end = -(-data // SECTOR) * SECTOR, end // SECTOR * SECTOR
    first = -(-(data + size) // SECTOR) * SECTOR  # behind the folder's cluster
    left = [o for o in range(first, end, SECTOR) if nor[o:o + SECTOR] != b"\xff" * SECTOR]
    check(not left, f"every cluster but the folder's reads 0xFF ({len(left)} sectors do not)")
    blocks = max(0, end // 0x10000 - -(-start // 0x10000))
    m = re.search(r"vol_format: 0, (\d+) sector and (\d+) block erases", out)
    check(m and int(m.group(2)) == blocks,
          f"the clusters are erased in 64 KB blocks where whole ({m and m.group(1)} sector and "
          f"{m and m.group(2)} block erases)")
    m = re.search(r"WRITE\(10\) of block (\d+): (\d+) erases", out)
    check(m and int(m.group(2)) == 0, "a block of the clusters written then needs no erase")


def test_protocol(w):
    print("protocol: the transport and the SCSI commands (usb_host suite)")
    w.write("proto.img", fwnor.build([os.path.join(w.pack, "DOOMX.DAT")], [], "UI"))
    rc, out = w.usb("suite", "proto.img")
    for line in out.splitlines():
        if "FAIL" in line or not line.startswith("  "):
            print("    " + line)
    check(rc == 0, "usb_host suite passes")


def meter_volume(w):
    """A NOR image as the meter has it before the install: DOOMX.DAT on the firmware drive, the
    folder UI with stock art and a file in the root of the data drive."""
    junk = w.write("README.TXT", bytes((i * 13) & 0xFF for i in range(10000)))
    data = fwnor.build([os.path.join(w.pack, "DOOMX.DAT")], [junk], "UI")
    os.remove(junk)
    return w.write("meter.img", data)


def test_erase(w):
    print("erase: quick format of the data drive")
    meter_volume(w)
    a = w.read("meter.img")
    rc, out = w.usb("format", "meter.img", "erased.img")
    saved = "vol_format: 0," in out
    b = w.read("erased.img") if saved else a
    check(saved, "vol_format succeeds")
    ok, msg = fsck(w.volume("erased.img", "erased_vol.img"))
    check(ok, f"fsck.fat accepts the erased volume {msg}")
    check(root_names(w.p("erased_vol.img")) == ["UI"], "the root holds the folder UI only")
    check(root_names(w.p("erased_vol.img"), "::/UI") == [], "the folder UI is empty")
    check(a[VOL:VOL + 512] == b[VOL:VOL + 512], "the boot sector is kept")
    check(a[:VOL] == b[:VOL], "the firmware drive is unchanged")
    if saved:
        check_erased(w, "erased.img", rc, out)
    # no FAT12 boot sector: the meter's geometry
    bad = bytearray(a)
    bad[VOL:VOL + SECTOR] = bytes(SECTOR)
    w.write("nobpb.img", bad)
    rc, out = w.usb("format", "nobpb.img", "nobpb_erased.img")
    saved = "vol_format: 0," in out
    check(saved, "vol_format of a volume without a FAT12 boot sector")
    if saved:
        c = w.read("nobpb_erased.img")
        check(bpb(c) == (4096, 1, 1, 2, 512, 768, 0xF8, 1),
              "it gets the meter's geometry: 4 KB sectors, 2 FATs, 512 root entries, 768 sectors")
        ok, msg = fsck(w.volume("nobpb_erased.img", "nobpb_vol.img"))
        check(ok and root_names(w.p("nobpb_vol.img")) == ["UI"], f"fsck.fat accepts it, folder UI {msg}")
        check(c[:VOL] == a[:VOL], "the firmware drive is unchanged")
        check_erased(w, "nobpb_erased.img", rc, out)
    # another FAT12 geometry, made by mkfs.fat: kept
    other = w.p("other_vol.img")
    if os.path.exists(other):
        os.remove(other)
    subprocess.run(["mkfs.fat", "-F", "12", "-S", "4096", "-s", "2", "-f", "1", "-r", "256", "-n", "OTHER",
                    "-C", other, "3072"], check=True, capture_output=True)
    rc, _ = mtools("mcopy", "-i", other, w.write("SOME.BIN", bytes(range(256)) * 100), "::/SOME.BIN")
    check(rc == 0 and root_names(other) == ["SOME.BIN"], "mkfs.fat and mcopy make a volume with a file")
    with open(other, "rb") as f:
        vol = f.read()
    m = bytearray(a)
    m[VOL:VOL_END] = vol
    w.write("other.img", m)
    rc_fmt, fmt = w.usb("format", "other.img", "other_erased.img")
    saved = "vol_format: 0," in fmt
    check(saved, "vol_format of that volume (1 FAT, 256 root entries, 8 KB clusters)")
    if saved:
        c = w.read("other_erased.img")
        check(c[VOL:VOL + 512] == vol[:512], "its boot sector is kept")
        ok, msg = fsck(w.volume("other_erased.img", "other_erased_vol.img"))
        check(ok and root_names(w.p("other_erased_vol.img")) == ["UI"], f"fsck.fat accepts it, folder UI only {msg}")
        rc, out = mtools("mlabel", "-s", "-i", w.p("other_erased_vol.img"), "::")
        check(rc == 0 and b"OTHER" in out, f"its volume label stays ({out.decode().strip()})")
        check_erased(w, "other_erased.img", rc_fmt, fmt)


def test_copy(w):
    print("copy: DOOM.DAT through WRITE(10) as mtools writes it")
    doom = os.path.join(w.pack, "DOOM.DAT")
    erased = w.read("erased.img")
    target = w.volume("erased.img", "mtools_vol.img")
    rc, _ = mtools("mcopy", "-i", target, doom, "::/DOOM.DAT")
    check(rc == 0, "mcopy DOOM.DAT onto an image of the erased volume")
    with open(target, "rb") as f:
        want = f.read()
    ch = changed_sectors(erased[VOL:VOL_END], want)
    with open(w.p("copy.rec"), "wb") as f:
        for o in ch:
            f.write(struct.pack("<I", o // SECTOR) + want[o:o + SECTOR])
    rc, out = w.usb("replay", "erased.img", "copy.rec", "copied.img")
    print("    " + out.strip().splitlines()[-1])
    check(rc == 0, f"{len(ch)} sectors written with WRITE(10)")
    if rc:
        return
    head = [o for o in ch if o < clusters(erased)[0] - VOL]
    m = re.search(r"(\d+) sector and (\d+) block erases", out)
    check(m and int(m.group(1)) == len(head) and int(m.group(2)) == 0,
          f"only the {len(head)} sectors of the FATs and the root directory needed an erase")
    c = w.read("copied.img")
    check(c[VOL:VOL_END] == want, "the volume equals mtools' image")
    ok, msg = fsck(w.volume("copied.img", "copied_vol.img"))
    check(ok, f"fsck.fat accepts it {msg}")
    back = w.p("DOOM.OUT")
    rc, _ = mtools("mcopy", "-n", "-i", w.p("copied_vol.img"), "::/DOOM.DAT", back)
    with open(doom, "rb") as f:
        same = rc == 0 and open(back, "rb").read() == f.read()
    check(same, "mtools reads DOOM.DAT back unchanged")
    check(root_names(w.p("copied_vol.img")) == ["DOOM.DAT", "UI"] and root_names(w.p("copied_vol.img"), "::/UI") == [],
          "the root holds DOOM.DAT and the folder UI")
    rc, out = w.usb("img", "copied.img")
    check(rc == 0, f"the firmware's img_init and img_verify accept the data ({out.strip()})")
    rc, out = w.usb("img", "erased.img")
    check("img_init -2" in out, "on the erased volume img_init finds no DOOM.DAT")
    check(c[:VOL] == erased[:VOL] == w.read("meter.img")[:VOL], "nothing outside 0x100000-0x3FFFFF changed")


def test_ui(w):
    print("ui: the folder UI when the drive is left")
    doom = os.path.join(w.pack, "DOOM.DAT")
    a = w.write("uioff.img", fwnor.build([], [doom], "UI_OFF"))
    rc, out = w.usb("ui", "uioff.img", "uioff_out.img")
    b, a = w.read("uioff_out.img"), w.read("uioff.img")
    ch = [i for i in range(len(a)) if a[i] != b[i]] if a != b else []
    check(rc == 0 and ch and all(VOL + 3 * SECTOR <= i < VOL + 3 * SECTOR + 11 for i in ch),
          f"UI_OFF renamed back to UI in its directory entry ({len(ch)} bytes)")
    ok, msg = fsck(w.volume("uioff_out.img", "uioff_vol.img"))
    check(ok and "UI" in root_names(w.p("uioff_vol.img")), f"fsck.fat accepts it {msg}")
    w.write("hasui.img", fwnor.build([], [doom], "UI"))
    rc, out = w.usb("ui", "hasui.img", "hasui_out.img")
    check(rc == 0 and w.read("hasui.img") == w.read("hasui_out.img"), "a folder UI is left as it is")
    w.write("noui.img", fwnor.build([], [doom], "none"))
    rc, out = w.usb("ui", "noui.img", "noui_out.img")
    check(rc == 0, "no folder UI: a new one")
    ok, msg = fsck(w.volume("noui_out.img", "noui_vol.img"))
    check(ok and root_names(w.p("noui_vol.img")) == ["DOOM.DAT", "UI"]
          and root_names(w.p("noui_vol.img"), "::/UI") == [], f"fsck.fat accepts it, DOOM.DAT and the empty UI {msg}")
    back = w.p("DOOM.OUT")
    rc, _ = mtools("mcopy", "-o", "-n", "-i", w.p("noui_vol.img"), "::/DOOM.DAT", back)
    with open(doom, "rb") as f:
        check(rc == 0 and open(back, "rb").read() == f.read(), "DOOM.DAT is unchanged")
    check(w.read("noui.img")[:VOL] == w.read("noui_out.img")[:VOL], "the firmware drive is unchanged")
    bad = bytearray(w.read("noui.img"))
    bad[VOL:VOL + SECTOR] = bytes(SECTOR)
    w.write("nofs.img", bad)
    rc, out = w.usb("ui", "nofs.img", "nofs_out.img")
    check(rc != 0 and "vol_ui: -2" in out and w.read("nofs_out.img") == bytes(bad),
          "no FAT12 volume: VOL_E_FS, nothing written")
    full = w.write("FULL.BIN", bytes(761 * SECTOR))
    w.write("full.img", fwnor.build([], [full], "none"))
    os.remove(full)
    rc, out = w.usb("ui", "full.img", "full_out.img")
    check(rc != 0 and "vol_ui: -3" in out and w.read("full.img") == w.read("full_out.img"),
          "a full volume: VOL_E_FULL, nothing written")


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--build", default=os.path.join(ROOT, "build"))
    a = ap.parse_args()
    w = Work(os.path.abspath(a.build))
    for t in (test_protocol, test_erase, test_copy, test_ui):
        t(w)
    if not failures:
        shutil.rmtree(w.dir, ignore_errors=True)
    print("ALL PASSED" if not failures else f"{len(failures)} FAILED")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
