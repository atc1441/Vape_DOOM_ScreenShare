#!/usr/bin/env python3
"""
mknor.py - builds the vape's NOR image out of the game data.

The port finds its files through two FAT12 volumes, exactly as on the
DMC-100: the firmware volume at 0x000000 (1 MiB) and the data volume at
0x100000 (3 MiB). Those addresses are constants in port/data/extents.h;
keeping them here means not a line of the port's data layer has to change.

The vape's flash is 16 MiB, so 12 MiB stay free. A larger layout would be
possible, but it costs a change in the port and buys nothing as long as the
data fits.

    python mknor.py --doom DOOM.DAT --doomx DOOMX.DAT -o nor.bin
"""

import argparse
import os
import struct
import sys

SECTOR = 512
CLUSTER = 4096
SPC = CLUSTER // SECTOR  # 8

VOL_UPDATE = 0x000000  # DMC_VOL_UPDATE
VOL_MAIN = 0x100000    # DMC_VOL_MAIN
UPDATE_SIZE = 0x100000
MAIN_SIZE = 0x300000
IMAGE_SIZE = VOL_MAIN + MAIN_SIZE

SAVE_SIZE = 64 * 1024


def name83(name):
    base, _, ext = name.partition(".")
    return ("%-8s%-3s" % (base[:8], ext[:3])).encode("ascii")


class Fat12(object):
    """A FAT12 volume with one reserved sector, one FAT and one root
    directory. Files are laid down contiguously: the port reads them
    through extents, and one run per file is the best case for it."""

    def __init__(self, size, label, nroot=16):
        self.size = size
        self.total_sec = size // SECTOR
        self.nroot = nroot
        self.label = label
        root_sectors = (nroot * 32 + SECTOR - 1) // SECTOR
        # FAT large enough for all clusters, found iteratively
        fatsz = 1
        while True:
            datasec = 1 + fatsz + root_sectors
            nclus = (self.total_sec - datasec) // SPC
            need = (nclus + 2) * 3 // 2
            if need <= fatsz * SECTOR:
                break
            fatsz += 1
        self.fatsz = fatsz
        self.root_sectors = root_sectors
        self.datasec = 1 + fatsz + root_sectors
        self.nclus = (self.total_sec - self.datasec) // SPC
        if self.nclus >= 4085:
            raise SystemExit("too many clusters for FAT12: %d" % self.nclus)
        self.fat = [0] * (self.nclus + 2)
        self.fat[0] = 0xFF8
        self.fat[1] = 0xFFF
        self.dir = []
        self.data = bytearray(b"\xFF" * (self.nclus * CLUSTER))
        self.next_cluster = 2

    def add(self, name, blob):
        n = (len(blob) + CLUSTER - 1) // CLUSTER
        if self.next_cluster + n > self.nclus + 2:
            raise SystemExit("%s does not fit in %s (%d of %d clusters free)" %
                             (name, self.label, self.nclus + 2 - self.next_cluster, self.nclus))
        start = self.next_cluster
        for i in range(n):
            c = start + i
            self.fat[c] = 0xFFF if i == n - 1 else c + 1
        off = (start - 2) * CLUSTER
        self.data[off:off + len(blob)] = blob
        pad = n * CLUSTER - len(blob)
        if pad:
            self.data[off + len(blob):off + n * CLUSTER] = b"\x00" * pad
        self.next_cluster = start + n
        self.dir.append((name83(name), start, len(blob)))
        return start, n

    def build(self):
        img = bytearray(b"\xFF" * self.size)

        bpb = bytearray(SECTOR)
        bpb[0:3] = b"\xEB\x3C\x90"
        bpb[3:11] = b"VAPEDOOM"
        struct.pack_into("<H", bpb, 11, SECTOR)
        bpb[13] = SPC
        struct.pack_into("<H", bpb, 14, 1)          # Reservesektoren
        bpb[16] = 1                                  # one FAT
        struct.pack_into("<H", bpb, 17, self.nroot)
        struct.pack_into("<H", bpb, 19, self.total_sec if self.total_sec < 0x10000 else 0)
        bpb[21] = 0xF8
        struct.pack_into("<H", bpb, 22, self.fatsz)
        struct.pack_into("<H", bpb, 24, 32)          # Sektoren je Spur
        struct.pack_into("<H", bpb, 26, 2)           # Koepfe
        struct.pack_into("<I", bpb, 28, 0)           # versteckte Sektoren
        struct.pack_into("<I", bpb, 32, 0 if self.total_sec < 0x10000 else self.total_sec)
        bpb[36] = 0x80
        bpb[38] = 0x29
        struct.pack_into("<I", bpb, 39, 0x56415045)
        bpb[43:54] = ("%-11s" % self.label).encode("ascii")
        bpb[54:62] = b"FAT12   "
        bpb[510] = 0x55
        bpb[511] = 0xAA
        img[0:SECTOR] = bpb

        fat = bytearray(self.fatsz * SECTOR)
        for i in range(0, len(self.fat) - 1, 2):
            a, b = self.fat[i], self.fat[i + 1]
            j = i * 3 // 2
            fat[j] = a & 0xFF
            fat[j + 1] = ((a >> 8) & 0x0F) | ((b & 0x0F) << 4)
            fat[j + 2] = (b >> 4) & 0xFF
        if len(self.fat) % 2:
            a = self.fat[-1]
            j = (len(self.fat) - 1) * 3 // 2
            fat[j] = a & 0xFF
            fat[j + 1] = (fat[j + 1] & 0xF0) | ((a >> 8) & 0x0F)
        img[SECTOR:SECTOR + len(fat)] = fat

        root = bytearray(self.root_sectors * SECTOR)
        for k, (nm, start, size) in enumerate(self.dir):
            e = bytearray(32)
            e[0:11] = nm
            e[11] = 0x20
            struct.pack_into("<H", e, 22, 0x6000)   # Zeit
            struct.pack_into("<H", e, 24, 0x5A21)   # Datum
            struct.pack_into("<H", e, 26, start)
            struct.pack_into("<I", e, 28, size)
            root[k * 32:(k + 1) * 32] = e
        off = (1 + self.fatsz) * SECTOR
        img[off:off + len(root)] = root

        off = self.datasec * SECTOR
        img[off:off + len(self.data)] = self.data
        return bytes(img)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--doom", required=True)
    ap.add_argument("--doomx", default=None,
                    help="omitted when the image fits entirely into DOOM.DAT")
    ap.add_argument("--save", type=int, default=SAVE_SIZE)
    ap.add_argument("-o", "--out", required=True)
    a = ap.parse_args()

    doom = open(a.doom, "rb").read()
    # DOOMX.DAT exists only when the image does not fit into DOOM.DAT. The
    # data layer only checks for it then (img_check.c: if split < size).
    doomx = open(a.doomx, "rb").read() if a.doomx and os.path.exists(a.doomx) else b""

    up = Fat12(UPDATE_SIZE, "VAPE-FW")
    if doomx:
        up.add("DOOMX.DAT", doomx)

    main_vol = Fat12(MAIN_SIZE, "VAPE-DATA")
    main_vol.add("DOOM.DAT", doom)
    main_vol.add("SAVE.DAT", b"\x00" * a.save)

    img = bytearray(b"\xFF" * IMAGE_SIZE)
    img[VOL_UPDATE:VOL_UPDATE + UPDATE_SIZE] = up.build()
    img[VOL_MAIN:VOL_MAIN + MAIN_SIZE] = main_vol.build()
    open(a.out, "wb").write(bytes(img))

    print("[nor] %s  %d B" % (a.out, len(img)))
    print("  Firmware-Laufwerk 0x%06X: %s, %d von %d Clustern frei" %
          (VOL_UPDATE, ("DOOMX.DAT %d B" % len(doomx)) if doomx else "empty (not needed)",
           up.nclus + 2 - up.next_cluster, up.nclus))
    print("  Daten-Laufwerk    0x%06X: DOOM.DAT %d B + SAVE.DAT %d B, %d von %d frei" %
          (VOL_MAIN, len(doom), a.save, main_vol.nclus + 2 - main_vol.next_cluster,
           main_vol.nclus))
    return 0


if __name__ == "__main__":
    sys.exit(main())
