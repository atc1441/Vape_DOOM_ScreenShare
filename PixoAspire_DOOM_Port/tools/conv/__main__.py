# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Converter entry point.

usage: python3 -I tools/conv/__main__.py [--zip PATH | --wad PATH] [options] {gen,packs,all}
       python3 -m tools.conv ... (from the repository root, without -I)
"""

import argparse
import hashlib
import os
import sys

if __package__ in (None, ""):
    sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
    __package__ = "tools.conv"

from tools.conv import fetch, format as F, info, pack, trig, upstream, wadread  # noqa: E402
from tools.conv.util import GEN, ConvError, write_if_changed  # noqa: E402


def load_wad(args):
    if args.wad:
        wad_bytes, notices = fetch.read_wad(args.wad), {}
    else:
        wad_bytes, notices = fetch.extract(args.zip)
    return wadread.Wad(wad_bytes), wad_bytes, notices


def load_tables(wad):
    up = upstream.load()
    deh = wadread.dehacked(wad)
    inf, dense = info.build(wad, up, deh)
    return up, deh, inf, dense


def write_gen(args, wad, wad_bytes, up, inf, dense):
    ih, ic, isz = info.emit(inf, dense, hashlib.sha256(wad_bytes).digest())
    source = os.path.basename(args.wad) if args.wad else None
    files, tsz = trig.emit(trig.build(up, wad.lump("COLORMAP")), source)
    files.update({"dmcdat.h": F.emit_header(), "info.h": ih, "info.c": ic})
    for name, text in sorted(files.items()):
        write_if_changed(os.path.join(args.gen, name), text)
    print("gen: %d states, %d mobj types (%d actors), %d sprites, %d actions" % (
        len(dense.state_list), len(dense.type_list), dense.nactors, len(dense.sprite_list),
        len(dense.actions)))
    print("gen: flash bytes info %s, tables %s" % (isz, tsz))


def main(argv=None):
    ap = argparse.ArgumentParser(prog="tools.conv")
    ap.add_argument("--zip", help="local freedoom-0.13.0.zip (default: $FREEDOOM_ZIP, dl/ cache, download)")
    ap.add_argument("--wad", help="local IWAD read instead of the Freedoom zip")
    ap.add_argument("--gen", default=GEN, help="directory of the generated sources (default gen)")
    ap.add_argument("--out", default=None, help="pack output directory (default build/packs)")
    ap.add_argument("--ram-layout", default=None, help="RAM layout JSON for the planner")
    ap.add_argument("--packs", default="E1M1,E1", help="comma separated pack ids")
    ap.add_argument("cmd", choices=("gen", "packs", "all"),
                    help="gen writes gen/, packs writes the packs only, all does both")
    args = ap.parse_args(argv)
    try:
        wad, wad_bytes, notices = load_wad(args)
        up, deh, inf, dense = load_tables(wad)
        if args.cmd in ("gen", "all"):
            write_gen(args, wad, wad_bytes, up, inf, dense)
        if args.cmd in ("packs", "all"):
            pack.build_all(args, wad, wad_bytes, notices, up, deh, inf, dense)
    except ConvError as e:
        print("conv: error: %s" % e, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
