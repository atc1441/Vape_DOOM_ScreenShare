#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Firmware build ID and DOOMCODE.DMC, the code overlay file.

usage: pack_overlays.py stamp IN.elf OUT.elf
       pack_overlays.py pack --main main.elf -o DOOMCODE.DMC [--report] [--forbid FUNC]...
                             OVERLAY.elf...
       pack_overlays.py info DOOMCODE.DMC

Build ID: SHA-1 of the resident flat image (llvm-objcopy -O binary of main.elf, gaps 0)
with the 20 bytes of dmc_build_id set to 0. "stamp" writes it into dmc_build_id, which holds
the placeholder "DMCDOOM NOT STAMPED" until then. DOOMCODE.DMC carries the same 20 bytes and
the loader (port/ovl/ovl.c) refuses a file of another build. SAVE.DAT keeps the first 8.

DOOMCODE.DMC, little-endian:
  header 64 B   char[8] "DMCCODE\\0", u16 fmt = 1, u16 count, u8 build_id[20],
                u32 total size, u32 crc, 24 zero bytes. crc is the CRC-32 of the header
                (crc field 0) and the directory.
  directory     count x 16 B, ascending id: u8 id, u8 0, u16 entry count, u32 off,
                u32 size, u32 crc (CRC-32 of the blob)
  blobs         16 B aligned: u32 magic "OVL1", u32 vma (FRAME_BASE), u32 copy size (= size),
                u32 bss size (cleared from the end of the blob), u32 entry[n], then code,
                rodata and data

Each overlay is the ELF of port/ovl/ovl.ld, linked with --just-symbols=main.elf and
--emit-relocs. "pack" fails unless all of these hold:
  main.elf  stamped with its own build ID, linked with --emit-relocs, no undefined
            symbols, no function or object inside the frame region [__frame_start,
            __frame_start + __frame_max), every window (ovl_win) within __frame_min
  overlay   __ovl_id known to the firmware (ovl_win[id] != 0) and unique, only the .ovl and
            .ovl_bss sections and NOBITS sections .res_N outside the frame region (the input
            sections of objects that main.elf links too, placed where main.elf has them by
            tools/ovl_resident.py), blob header consistent, entries point at Thumb code in
            the blob, no .data or .bss (state lives in resident structs, so the target and
            the host behave the same), copy + bss <= ovl_win[id], no undefined symbols, every
            resident symbol it refers to has the same value in main.elf (linked against this
            build), no reference to the loader (ovl_* symbols) or to resident functions or
            objects inside the frame region (linker symbols such as __frame_start are
            fine), and no resident function it uses reaches ovl_load, ovl_entry,
            ovl_invalidate or a --forbid function (resident code that writes the frame
            region) through calls or address loads in resident code. Calls through function
            pointers read from data tables or set at run time are not seen. So no overlay
            loads another overlay over itself or returns into an overwritten window.
  file      at most 11 clusters of 4 KB (45,056 B): with DOOMX.DAT (210), DOOM.BIN (25) and the
            Windows reserve (3) it fills the 249 clusters of the firmware drive
"""
import argparse
import bisect
import hashlib
import struct
import sys
import zlib

PLACEHOLDER = b"DMCDOOM NOT STAMPED\0"
CODE_MAGIC = b"DMCCODE\0"
CODE_FMT = 1
BLOB_MAGIC = 0x314C564F
HDR_SIZE = 64
ENT_SIZE = 16
MAX_COUNT = 32
FILE_CAP = 11 * 4096

SHT_SYMTAB, SHT_NOBITS, SHT_REL = 2, 8, 9
SHF_ALLOC, SHF_EXECINSTR = 2, 4
SHN_UNDEF, SHN_ABS = 0, 0xFFF1
PT_LOAD = 1
STT_NOTYPE, STT_OBJECT, STT_FUNC = 0, 1, 2
R_ARM_NONE, R_ARM_ABS32, R_ARM_THM_CALL, R_ARM_TARGET1 = 0, 2, 10, 38
R_ARM_THM_JUMP24, R_ARM_THM_MOVW_ABS_NC, R_ARM_THM_MOVT_ABS, R_ARM_THM_JUMP19 = 30, 47, 48, 51
LOADER = ("ovl_load", "ovl_entry", "ovl_invalidate")


class Error(Exception):
    pass


class Elf:
    """The parts of a little-endian ELF32 ARM file this tool needs."""

    def __init__(self, path):
        self.path = path
        with open(path, "rb") as f:
            self.data = d = f.read()
        if d[:4] != b"\x7fELF" or d[4] != 1 or d[5] != 1:
            raise Error(f"{path}: not a little-endian ELF32 file")
        (_, _, machine, _, _, phoff, shoff, _, _, phentsize, phnum, shentsize, shnum,
         shstrndx) = struct.unpack_from("<16sHHIIIIIHHHHHH", d, 0)
        if machine != 40:
            raise Error(f"{path}: not an ARM ELF file")
        self.sections = []
        for i in range(shnum):
            (name, typ, flags, addr, off, size, link, info, _, entsize) = struct.unpack_from(
                "<10I", d, shoff + i * shentsize)
            self.sections.append(dict(name=name, type=typ, flags=flags, addr=addr, off=off,
                                      size=size, link=link, info=info, entsize=entsize))
        names = self.sections[shstrndx]
        for s in self.sections:
            s["name"] = self._str(names["off"], s["name"])
        self.segments = [dict(zip(("type", "off", "vaddr", "paddr", "filesz", "memsz"),
                                  struct.unpack_from("<6I", d, phoff + i * phentsize)))
                         for i in range(phnum)]
        self.symbols = []
        for s in self.sections:
            if s["type"] == SHT_SYMTAB:
                strtab = self.sections[s["link"]]["off"]
                for k in range(s["size"] // 16):
                    name, value, size, info, _, shndx = struct.unpack_from(
                        "<IIIBBH", d, s["off"] + 16 * k)
                    self.symbols.append(dict(name=self._str(strtab, name), value=value, size=size,
                                             bind=info >> 4, type=info & 15, shndx=shndx))
                self.symtab = s
        self.globals = {}
        for s in self.symbols:
            if s["bind"] != 0 and s["name"]:
                self.globals.setdefault(s["name"], s)
        self.named = {(s["name"], s["value"]) for s in self.symbols if s["name"]}

    def _str(self, base, off):
        end = self.data.index(b"\0", base + off)
        return self.data[base + off:end].decode()

    def section(self, name):
        return next((s for s in self.sections if s["name"] == name), None)

    def sym(self, name):
        s = self.globals.get(name)
        if s is None:
            s = next((x for x in self.symbols if x["name"] == name), None)
        if s is None:
            raise Error(f"{self.path}: no symbol {name}")
        return s

    def file_offset(self, addr, n):
        for s in self.sections:
            if (s["flags"] & SHF_ALLOC and s["type"] != SHT_NOBITS
                    and s["addr"] <= addr and addr + n <= s["addr"] + s["size"]):
                return s["off"] + addr - s["addr"]
        raise Error(f"{self.path}: no file data at {addr:#x}+{n}")

    def read(self, addr, n):
        off = self.file_offset(addr, n)
        return self.data[off:off + n]

    def undefined(self):
        return sorted({s["name"] for s in self.symbols if s["shndx"] == SHN_UNDEF and s["name"]})

    def flat(self):
        """(load address, bytes) of the loaded sections, as llvm-objcopy -O binary writes them."""
        parts = []
        for s in self.sections:
            if not s["flags"] & SHF_ALLOC or s["type"] == SHT_NOBITS or not s["size"]:
                continue
            seg = next((p for p in self.segments if p["type"] == PT_LOAD
                        and p["off"] <= s["off"] < p["off"] + p["filesz"]), None)
            if seg is None:
                raise Error(f"{self.path}: section {s['name']} is in no loadable segment")
            parts.append((seg["paddr"] + s["off"] - seg["off"], s))
        if not parts:
            raise Error(f"{self.path}: no loadable data")
        base = min(a for a, _ in parts)
        img = bytearray(max(a + s["size"] for a, s in parts) - base)
        for a, s in parts:
            img[a - base:a - base + s["size"]] = self.data[s["off"]:s["off"] + s["size"]]
        return base, img

    def relocations(self, flags=SHF_ALLOC):
        """(target section, offset, type, symbol index, symbol) of every relocation in the
        sections that have all of flags (debug sections have none)."""
        out = []
        for s in self.sections:
            target = self.sections[s["info"]] if s["type"] == SHT_REL else None
            if target is not None and target["flags"] & flags == flags:
                for k in range(s["size"] // 8):
                    off, info = struct.unpack_from("<II", self.data, s["off"] + 8 * k)
                    out.append((target, off, info & 0xFF, info >> 8, self.symbols[info >> 8]))
        return out


def crc32(data, crc=0):
    return zlib.crc32(data, crc) & 0xFFFFFFFF


def build_id(elf):
    """Build ID of a resident image: SHA-1 of the flat image with dmc_build_id zeroed."""
    s = elf.sym("dmc_build_id")
    if s["size"] != 20:
        raise Error(f"{elf.path}: dmc_build_id is {s['size']} bytes, not 20")
    base, img = elf.flat()
    at = s["value"] - base
    if not 0 <= at <= len(img) - 20:
        raise Error(f"{elf.path}: dmc_build_id is not in the loaded image")
    current = bytes(img[at:at + 20])
    img[at:at + 20] = bytes(20)
    return hashlib.sha1(img).digest(), current


def stamp(src, dst):
    elf = Elf(src)
    bid, current = build_id(elf)
    if current not in (PLACEHOLDER, bid):
        raise Error(f"{src}: dmc_build_id holds neither the placeholder nor this build's ID")
    data = bytearray(elf.data)
    off = elf.file_offset(elf.sym("dmc_build_id")["value"], 20)
    data[off:off + 20] = bid
    with open(dst, "wb") as f:
        f.write(data)
    print(f"{dst}: build ID {bid.hex()}")


class Overlay:
    pass


def check_main(m):
    errs = []
    bid, current = build_id(m)
    if current == PLACEHOLDER:
        errs.append(f"{m.path}: not stamped (pack_overlays.py stamp)")
    elif current != bid:
        errs.append(f"{m.path}: dmc_build_id {current.hex()} is not this image's {bid.hex()}")
    for name in m.undefined():
        errs.append(f"{m.path}: undefined symbol {name}")
    base = m.sym("__frame_start")["value"]
    top = base + m.sym("__frame_max")["value"]
    for s in m.symbols:
        if s["type"] in (STT_OBJECT, STT_FUNC) and base <= s["value"] < top:
            errs.append(f"{m.path}: resident {s['name']} at {s['value']:#x} is inside the frame "
                        f"region, overlays overwrite it")
    ws = m.sym("ovl_win")
    win = struct.unpack(f"<{ws['size'] // 2}H", m.read(ws["value"], ws["size"]))
    fmin = m.sym("__frame_min")["value"]
    if max(win) > fmin:
        errs.append(f"{m.path}: ovl_win {max(win)} B is larger than FRAME_MIN {fmin} B")
    return current, base, top, win, errs


def thumb_imm(hw1, hw2, typ):
    """Branch offset of a Thumb-2 BL/B.W/B<c>.W, or the imm16 of a MOVW/MOVT."""
    if typ in (R_ARM_THM_MOVW_ABS_NC, R_ARM_THM_MOVT_ABS):
        return (hw1 & 0xF) << 12 | (hw1 >> 10 & 1) << 11 | (hw2 >> 12 & 7) << 8 | (hw2 & 0xFF)
    s_, j1, j2 = hw1 >> 10 & 1, hw2 >> 13 & 1, hw2 >> 11 & 1
    if typ == R_ARM_THM_JUMP19:
        imm = s_ << 20 | j2 << 19 | j1 << 18 | (hw1 & 0x3F) << 12 | (hw2 & 0x7FF) << 1
        return imm - (s_ << 21)
    imm = (s_ << 24 | (1 - (j1 ^ s_)) << 23 | (1 - (j2 ^ s_)) << 22 | (hw1 & 0x3FF) << 12
           | (hw2 & 0x7FF) << 1)
    return imm - (s_ << 25)


class CallGraph:
    """Resident functions, keyed by address (Thumb bit clear) so aliases and local functions
    with a global's name stay apart, and the functions each one calls or loads the address
    of. Built from the relocations main.elf keeps (--emit-relocs) in all its executable
    sections: BL/B.W/B<c>.W, literal words and MOVW/MOVT pairs. Calls through function
    pointers read from data tables or set at run time are not seen. Functions with other
    relocations are listed in unknown, a path through them may be missed."""

    def __init__(self, m):
        self.names, extent = {}, {}
        for s in m.symbols:
            if s["type"] != STT_FUNC or s["shndx"] in (SHN_UNDEF, SHN_ABS) or s["shndx"] >= len(m.sections):
                continue
            a = s["value"] & ~1
            self.names.setdefault(a, set()).add((s["bind"] == 0, s["name"]))
            sec = extent.setdefault(s["shndx"], {})
            sec[a] = max(sec.get(a, 0), s["size"])
        spans = []
        for shndx, funcs in extent.items():
            sec, starts = m.sections[shndx], sorted(funcs)
            for k, a in enumerate(starts):
                nxt = starts[k + 1] if k + 1 < len(starts) else sec["addr"] + sec["size"]
                spans.append((a, a + funcs[a] if funcs[a] else nxt))
        spans.sort()
        self.starts, self.ends = [a for a, _ in spans], [e for _, e in spans]
        self.edges, self.unknown, movw = {}, {}, {}
        for _, off, typ, idx, sym in m.relocations(SHF_ALLOC | SHF_EXECINSTR):
            f = self.func_at(off)
            if f is None:
                continue
            out = self.edges.setdefault(f, set())
            if sym["type"] == STT_FUNC and sym["shndx"] != SHN_UNDEF:
                out.add(sym["value"] & ~1)
            if typ in (R_ARM_THM_CALL, R_ARM_THM_JUMP24, R_ARM_THM_JUMP19):
                out.add(off + 4 + thumb_imm(*struct.unpack("<HH", m.read(off, 4)), typ))
            elif typ in (R_ARM_ABS32, R_ARM_TARGET1):
                out.add(struct.unpack("<I", m.read(off, 4))[0] & ~1)
            elif typ in (R_ARM_THM_MOVW_ABS_NC, R_ARM_THM_MOVT_ABS):
                hw1, hw2 = struct.unpack("<HH", m.read(off, 4))
                key = (f, hw2 >> 8 & 15, idx)
                movw.setdefault(key, ([], []))[typ == R_ARM_THM_MOVT_ABS].append(
                    (off, thumb_imm(hw1, hw2, typ)))
            elif typ != R_ARM_NONE:
                self.unknown.setdefault(f, set()).add(f"type {typ}")
        for (f, _, _), (lo, hi) in movw.items():
            if len(lo) != len(hi):
                self.unknown.setdefault(f, set()).add("unpaired MOVW/MOVT")
            for (_, low), (_, high) in zip(sorted(lo), sorted(hi)):
                self.edges[f].add((high << 16 | low) & ~1)
        for f in self.edges:
            self.edges[f] &= self.names.keys()

    def func_at(self, addr):
        i = bisect.bisect_right(self.starts, addr) - 1
        return self.starts[i] if i >= 0 and addr < self.ends[i] else None

    def name(self, a):
        return min(self.names[a])[1] if a in self.names else f"{a:#x}"

    def path(self, start, goal):
        """Shortest chain of addresses from one of start to one of goal, or None, and the
        functions with unknown relocations met on the way."""
        prev = {a: None for a in sorted(start)}
        todo, seen_unknown = list(prev), set()
        while todo:
            a = todo.pop(0)
            if a in self.unknown:
                seen_unknown.add(a)
            if a in goal:
                chain = []
                while a is not None:
                    chain.append(a)
                    a = prev[a]
                return chain[::-1], seen_unknown
            for t in sorted(self.edges.get(a, ())):
                if t not in prev:
                    prev[t] = a
                    todo.append(t)
        return None, seen_unknown


def check_overlay(m, path, base, top, win, graph, forbid):
    o, errs = Overlay(), []
    e = Elf(path)
    o.path, o.warnings = path, []
    o.id = e.sym("__ovl_id")["value"]
    tabs = [s["name"][8:] for s in e.symbols if s["name"].startswith("ovl_tab_") and s["shndx"] != SHN_ABS]
    o.name = tabs[0] if len(tabs) == 1 else "?"
    tag = f"{path} (overlay {o.id} {o.name})"
    if len(tabs) != 1:
        errs.append(f"{tag}: needs exactly one OVL_TABLE, found {len(tabs)}")
    o.window = win[o.id] if 0 < o.id < len(win) else 0
    if not o.window:
        errs.append(f"{tag}: id {o.id} is unknown to the firmware (ovl_win)")
    res = set()
    for i, s in enumerate(e.sections):
        if not s["flags"] & SHF_ALLOC or not s["size"] or s["name"] in (".ovl", ".ovl_bss"):
            continue
        if s["name"].startswith(".res_") and s["type"] == SHT_NOBITS and not (
                base < s["addr"] + s["size"] and s["addr"] < top):
            res.add(i)
        else:
            errs.append(f"{tag}: unexpected section {s['name']}")
    sec, bss = e.section(".ovl"), e.section(".ovl_bss")
    if sec is None:
        raise Error(f"{tag}: no .ovl section, link it with port/ovl/ovl.ld")
    o.blob = e.data[sec["off"]:sec["off"] + sec["size"]]
    magic, vma, copy, o.bss = struct.unpack_from("<4I", o.blob)
    o.copy = len(o.blob)
    if magic != BLOB_MAGIC or vma != base or sec["addr"] != base or copy != o.copy:
        errs.append(f"{tag}: blob header {magic:#x} {vma:#x} {copy} does not match "
                    f"magic {BLOB_MAGIC:#x}, vma {base:#x} (main.elf __frame_start), size {o.copy}")
    copy_end, bss_end = e.sym("__ovl_copy_end")["value"], e.sym("__ovl_bss_end")["value"]
    if copy_end != base + o.copy or o.bss != bss_end - copy_end:
        errs.append(f"{tag}: blob bss {o.bss} B does not span the {bss_end - copy_end} B from the "
                    f"end of the blob to the end of .ovl_bss")
    d0, d1 = e.sym("__ovl_data_start")["value"], e.sym("__ovl_data_end")["value"]
    b0, b1 = (bss["addr"], bss["addr"] + bss["size"]) if bss else (0, 0)
    if d1 > d0 or b1 > b0:
        objs = sorted(s["name"] for s in e.symbols if s["type"] == STT_OBJECT and s["shndx"] not in
                      (SHN_UNDEF, SHN_ABS) and (d0 <= s["value"] < d1 or b0 <= s["value"] < b1))
        errs.append(f"{tag}: has its own .data ({d1 - d0} B) and .bss ({b1 - b0} B)"
                    f"{' ' + ', '.join(objs) if objs else ''}, overlay state lives in resident structs")
    o.nent = (e.sym("__ovl_entries_end")["value"] - base - 16) // 4
    if o.nent < 1 or 16 + 4 * o.nent > o.copy:
        errs.append(f"{tag}: bad entry table ({o.nent} entries)")
    else:
        for i, a in enumerate(struct.unpack_from(f"<{o.nent}I", o.blob, 16)):
            if not a & 1 or not base + 16 + 4 * o.nent <= a - 1 < base + o.copy:
                errs.append(f"{tag}: entry {i} = {a:#x} is not Thumb code in the blob")
    if o.copy + o.bss > o.window:
        errs.append(f"{tag}: copy {o.copy} + bss {o.bss} = {o.copy + o.bss} B is over its "
                    f"window of {o.window} B")
    for name in e.undefined():
        errs.append(f"{tag}: undefined symbol {name}")
    rels = e.relocations()
    if not rels:
        errs.append(f"{tag}: no relocations, link it with --emit-relocs")
    o.imports, start = set(), set()
    for _, _, _, _, s in rels:
        if (s["shndx"] != SHN_ABS and s["shndx"] not in res) or s["name"] == "" or s["name"] in o.imports:
            continue
        name = s["name"]
        o.imports.add(name)
        ms = m.globals.get(name) if s["bind"] else None
        if ((ms is None or ms["value"] != s["value"]) and (name, s["value"]) not in m.named):
            errs.append(f"{tag}: {name} = {s['value']:#x} is not in main.elf or has another value "
                        f"there, relink against {m.path}")
        if name.startswith("ovl_"):
            errs.append(f"{tag}: refers to the loader symbol {name}, overlays never load overlays")
        if base <= s["value"] < top and s["type"] != STT_NOTYPE:
            errs.append(f"{tag}: refers to resident {name} at {s['value']:#x} inside the frame region")
        if s["value"] & ~1 in graph.names:
            start.add(s["value"] & ~1)
    chain, unknown = graph.path(start, forbid)
    if chain and (len(chain) > 1 or not graph.name(chain[0]).startswith("ovl_")):
        errs.append(f"{tag}: calls {' -> '.join(graph.name(a) for a in chain)}, which can load "
                    f"another overlay or overwrite the window it runs from")
    for a in sorted(unknown):
        o.warnings.append(f"{tag}: calls {graph.name(a)}, whose relocations "
                          f"({', '.join(sorted(graph.unknown[a]))}) the call check does not follow")
    return o, errs


def make_file(build_id_bytes, ovls):
    ovls = sorted(ovls, key=lambda o: o.id)
    off = HDR_SIZE + ENT_SIZE * len(ovls)
    dirent, blobs = b"", b""
    for o in ovls:
        off = (off + 15) & ~15
        pad = off - HDR_SIZE - ENT_SIZE * len(ovls) - len(blobs)
        blobs += bytes(pad) + o.blob
        dirent += struct.pack("<BBHIII", o.id, 0, o.nent, off, len(o.blob), crc32(o.blob))
        off += len(o.blob)
    total = HDR_SIZE + len(dirent) + len(blobs)
    hdr = struct.pack("<8sHH20sII24s", CODE_MAGIC, CODE_FMT, len(ovls), build_id_bytes, total, 0, bytes(24))
    crc = crc32(dirent, crc32(hdr))
    hdr = hdr[:36] + struct.pack("<I", crc) + hdr[40:]
    return hdr + dirent + blobs


def pack(main_path, paths, out, report, forbid_names=()):
    m = Elf(main_path)
    bid, base, top, win, errs = check_main(m)
    if not m.relocations(SHF_ALLOC | SHF_EXECINSTR):
        errs.append(f"{m.path}: no relocations, link it with --emit-relocs")
    graph = CallGraph(m)
    forbid = {m.globals[n]["value"] & ~1 for n in LOADER if n in m.globals}
    for n in forbid_names:
        s = m.globals.get(n)
        if s is None or s["type"] != STT_FUNC:
            errs.append(f"{m.path}: --forbid {n}: no such function")
        else:
            forbid.add(s["value"] & ~1)
    ovls = []
    for p in paths:
        o, e = check_overlay(m, p, base, top, win, graph, forbid)
        ovls.append(o)
        errs += e
    for o in ovls:
        for w in o.warnings:
            print(f"pack_overlays.py: warning: {w}", file=sys.stderr)
    seen = {}
    for o in ovls:
        if o.id in seen:
            errs.append(f"{o.path}: id {o.id} also used by {seen[o.id]}")
        seen[o.id] = o.path
    data = make_file(bid, ovls) if not errs else b""
    if len(data) > FILE_CAP:
        errs.append(f"{out}: {len(data)} B is over the {FILE_CAP} B cap")
    if errs:
        raise Error("\n".join(errs))
    with open(out, "wb") as f:
        f.write(data)
    if report:
        print(f"{out}: {len(data)} B, build ID {bid.hex()}, FRAME_BASE {base:#x}")
        print("  id  name       entries   copy    bss  window   free  resident symbols used")
        for o in sorted(ovls, key=lambda o: o.id):
            print(f"  {o.id:2}  {o.name:<9} {o.nent:7} {o.copy:6} {o.bss:6} {o.window:7} "
                  f"{o.window - o.copy - o.bss:6}  {len(o.imports)}")


def parse_file(data):
    """Header fields and directory of a DOOMCODE.DMC image, with the CRCs and the bounds of
    the directory and of every blob checked."""
    if len(data) < HDR_SIZE:
        raise Error(f"file of {len(data)} B is shorter than its {HDR_SIZE} B header")
    magic, fmt, count, bid, total, crc, _ = struct.unpack_from("<8sHH20sII24s", data)
    if magic != CODE_MAGIC or fmt != CODE_FMT or count > MAX_COUNT:
        raise Error("bad magic, format or count")
    end = HDR_SIZE + ENT_SIZE * count
    if end > len(data):
        raise Error(f"directory of {count} entries runs past the end of the {len(data)} B file")
    want = crc32(data[HDR_SIZE:end], crc32(data[:36] + bytes(4) + data[40:HDR_SIZE]))
    ents = []
    for i in range(count):
        oid, _, nent, off, size, bcrc = struct.unpack_from("<BBHIII", data, HDR_SIZE + ENT_SIZE * i)
        in_file = off <= len(data) and size <= len(data) - off
        blob = data[off:off + size] if in_file else b""
        fields = struct.unpack_from("<4I", blob) if len(blob) >= 16 else (0, 0, 0, 0)
        ents.append(dict(id=oid, nent=nent, off=off, size=size, crc=bcrc, in_file=in_file,
                         crc_ok=in_file and crc32(blob) == bcrc, magic=fields[0], vma=fields[1],
                         copy=fields[2], bss=fields[3]))
    return dict(build_id=bid, total=total, crc=crc, crc_ok=crc == want, size_ok=total == len(data),
                entries=ents)


def info(path):
    with open(path, "rb") as f:
        data = f.read()
    h = parse_file(data)
    print(f"{path}: {len(data)} B, total {h['total']} ({'ok' if h['size_ok'] else 'WRONG'}), "
          f"header CRC {h['crc']:08x} ({'ok' if h['crc_ok'] else 'WRONG'})")
    print(f"  build ID {h['build_id'].hex()}")
    for e in h["entries"]:
        if not e["in_file"]:
            print(f"  id {e['id']:2}: {e['size']} B at {e['off']} runs past the end of the file (WRONG)")
            continue
        print(f"  id {e['id']:2}: {e['nent']} entries, {e['size']} B at {e['off']}, bss {e['bss']}, "
              f"vma {e['vma']:#x}, CRC {e['crc']:08x} ({'ok' if e['crc_ok'] else 'WRONG'})")
    return 0 if h["crc_ok"] and h["size_ok"] and all(e["crc_ok"] for e in h["entries"]) else 1


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("stamp", help="write the build ID into a resident ELF")
    p.add_argument("src")
    p.add_argument("dst")
    p = sub.add_parser("pack", help="check overlay ELFs and write DOOMCODE.DMC")
    p.add_argument("--main", required=True)
    p.add_argument("-o", "--out", required=True)
    p.add_argument("--report", action="store_true")
    p.add_argument("--forbid", action="append", default=[], metavar="FUNC",
                   help="resident function no overlay may reach (it writes the frame region)")
    p.add_argument("overlays", nargs="+")
    p = sub.add_parser("info", help="print and verify a DOOMCODE.DMC")
    p.add_argument("file")
    a = ap.parse_args()
    try:
        if a.cmd == "stamp":
            stamp(a.src, a.dst)
        elif a.cmd == "pack":
            pack(a.main, a.overlays, a.out, a.report, a.forbid)
        else:
            return info(a.file)
    except (Error, OSError) as e:
        print(f"pack_overlays.py: {e}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
