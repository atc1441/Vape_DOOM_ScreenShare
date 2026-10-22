#!/usr/bin/env python3
"""Checks the NOR image with the same logic as port/data/fat12.c."""
import struct, sys, hashlib

VOL = [("firmware volume", 0x000000, 0x100000), ("data volume", 0x100000, 0x400000)]

def log2_exact(v):
    return v.bit_length() - 1 if v and not (v & (v - 1)) else -1

def mount(img, base, limit):
    b = img[base:base + 36]
    bshift = log2_exact(struct.unpack_from("<H", b, 11)[0])
    sshift = log2_exact(b[13])
    rsv = struct.unpack_from("<H", b, 14)[0]; nfat = b[16]
    nroot = struct.unpack_from("<H", b, 17)[0]
    total = struct.unpack_from("<H", b, 19)[0] or struct.unpack_from("<I", b, 32)[0]
    fatsz = struct.unpack_from("<H", b, 22)[0]
    assert 9 <= bshift <= 12 and sshift >= 0 and bshift + sshift <= 15, "BPB shifts"
    assert rsv and nfat and nfat <= 2 and nroot and fatsz, "BPB counts"
    assert base < limit and total <= (limit - base) >> bshift, "volume does not fit"
    rootsec = rsv + nfat * fatsz
    datasec = rootsec + ((nroot * 32 + (1 << bshift) - 1) >> bshift)
    assert total > datasec, "keine Datensektoren"
    nclus = (total - datasec) >> sshift
    assert nclus < 4085, "not FAT12"
    assert (nclus + 2) * 3 // 2 <= fatsz << bshift, "FAT zu klein"
    return dict(fat=base + (rsv << bshift), root=base + (rootsec << bshift),
                data=base + (datasec << bshift), nroot=nroot, nclus=nclus,
                cshift=bshift + sshift)

def fat_get(img, v, c):
    o = v["fat"] + c * 3 // 2
    w = img[o] | (img[o + 1] << 8)
    return (w >> 4) if (c & 1) else (w & 0xFFF)

def listdir(img, v):
    out = []
    for i in range(v["nroot"]):
        e = img[v["root"] + i * 32:v["root"] + i * 32 + 32]
        if e[0] in (0x00, 0xE5) or e[11] & 0x08 or e[11] & 0x10:
            if e[0] == 0x00: break
            continue
        name = e[0:11].decode("ascii")
        start = struct.unpack_from("<H", e, 26)[0]
        size = struct.unpack_from("<I", e, 28)[0]
        out.append((name, start, size))
    return out

def chain(img, v, start, size):
    runs, c, n = [], start, 0
    need = (size + (1 << v["cshift"]) - 1) >> v["cshift"]
    while 2 <= c < 0xFF8 and n < need + 2:
        if runs and runs[-1][0] + runs[-1][1] == c:
            runs[-1][1] += 1
        else:
            runs.append([c, 1])
        c = fat_get(img, v, c); n += 1
    return runs, n, need, c

img = open(sys.argv[1], "rb").read()
print("image %d B, sha256 %s" % (len(img), hashlib.sha256(img).hexdigest()[:24]))
ok = True
for label, base, limit in VOL:
    v = mount(img, base, limit)
    print("\n%s @0x%06X: %d clusters of %d B, root %d entries" %
          (label, base, v["nclus"], 1 << v["cshift"], v["nroot"]))
    for name, start, size in listdir(img, v):
        runs, n, need, last = chain(img, v, start, size)
        good = (n == need and last >= 0xFF8)
        ok &= good
        print("   %-12s %8d B  cluster %4d  runs %d  chain %s" %
              (name.strip(), size, start, len(runs), "ok" if good else "BROKEN"))
        if name.startswith("DOOM "):
            off = v["data"] + (start - 2) * (1 << v["cshift"])
            print("        first 16 B: %s" % img[off:off+16].hex(" "))
print("\n%s" % ("all chains consistent" if ok else "AT LEAST ONE CHAIN IS BROKEN"))
sys.exit(0 if ok else 1)
