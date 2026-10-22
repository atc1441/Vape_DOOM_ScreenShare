# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Small helpers shared by the converter modules."""

import os
import threading
import zlib

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
BUILD = os.path.join(REPO, "build")
GEN = os.path.join(REPO, "gen")
UPSTREAM = os.path.join(REPO, "oracle", "src", "doomgeneric-repo", "doomgeneric")


class ConvError(Exception):
    pass


def check(cond, msg, *args):
    if not cond:
        raise ConvError(msg % args if args else msg)


def align(n, a=16):
    return (n + a - 1) & ~(a - 1)


def pad(b, a=16, fill=b"\0"):
    return bytes(b) + fill * (align(len(b), a) - len(b))


def crc32(b):
    return zlib.crc32(b) & 0xFFFFFFFF


def mkdirs(path):
    os.makedirs(path, exist_ok=True)


def write_if_changed(path, data):
    """Write bytes or text, keeping the mtime when the content is unchanged."""
    if isinstance(data, str):
        data = data.encode("utf-8")
    try:
        with open(path, "rb") as f:
            if f.read() == data:
                return False
    except OSError:
        pass
    mkdirs(os.path.dirname(path))
    tmp = tmp_name(path)
    with open(tmp, "wb") as f:
        f.write(data)
    os.replace(tmp, path)
    return True


def tmp_name(path):
    """Temporary name next to path, unique per process and thread."""
    return "%s.%d.%d.tmp" % (path, os.getpid(), threading.get_ident())


def s32(v):
    v &= 0xFFFFFFFF
    return v - (1 << 32) if v & 0x80000000 else v


def u32(v):
    return v & 0xFFFFFFFF


def cdiv(a, b):
    """C99 integer division (truncates toward zero)."""
    q = abs(a) // abs(b)
    return q if (a >= 0) == (b >= 0) else -q


def fixed_mul(a, b):
    return s32((a * b) >> 16)


def fixed_div(a, b):
    """Vanilla FixedDiv with -fwrapv abs()."""
    def cabs(v):
        return s32(abs(v)) if v != -(1 << 31) else v
    if (cabs(a) >> 14) >= cabs(b):
        return -(1 << 31) if (a ^ b) < 0 else (1 << 31) - 1
    return s32(cdiv(a << 16, b))
