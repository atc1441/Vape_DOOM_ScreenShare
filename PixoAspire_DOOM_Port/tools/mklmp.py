#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Writes the E1M1 script workload of the lockstep test as a vanilla v1.9 demo (.lmp).

usage: mklmp.py -o DEMO2.LMP [--tics N]

The ticcmds are those of tests/lockstep/run_lockstep.py (make_cmds, the deterministic
stream of the "script" workload), on E1M1 at skill 4 (Ultra-Violence), single player. A
v1.9 demo keeps the turn in 8 bits, so each angleturn is stored the way vanilla's
G_WriteDemoTiccmd does it, (angleturn + 128) >> 8: the game played back differs from the
script run with --cmds, the engine and the oracle both play the .lmp file
(-playdemo FILE.lmp). The default length is 2,100 tics (one minute).
"""
import argparse
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "tests", "lockstep"))
sys.dont_write_bytecode = True
import run_lockstep  # noqa: E402

VERSION = 109
SKILL = 3  # sk_hard, -skill 4
CMD = struct.Struct("<bbhB3x")  # make_cmds record: forwardmove, sidemove, angleturn, buttons


def lmp(cmds):
    out = bytearray([VERSION, SKILL, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0])
    for fwd, side, turn, buttons in CMD.iter_unpack(cmds):
        out += struct.pack("<bbBB", fwd, side, ((turn + 128) >> 8) & 0xFF, buttons)
    out.append(0x80)
    return bytes(out)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("-o", "--out", required=True)
    ap.add_argument("--tics", type=int, default=2100)
    a = ap.parse_args()
    tmp = a.out + ".cmds"
    run_lockstep.make_cmds(tmp, a.tics)
    with open(tmp, "rb") as f:
        cmds = f.read()
    os.remove(tmp)
    data = lmp(cmds[:a.tics * CMD.size])
    with open(a.out + ".tmp", "wb") as f:
        f.write(data)
    os.replace(a.out + ".tmp", a.out)


if __name__ == "__main__":
    main()
