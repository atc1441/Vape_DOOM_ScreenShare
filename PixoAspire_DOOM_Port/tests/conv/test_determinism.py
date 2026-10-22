# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Two converter runs produce byte-identical gen/ sources and packs (same SHA-256).

usage: python3 -I tests/conv/test_determinism.py [--zip FREEDOOM_ZIP]
"""

import argparse
import hashlib
import os
import shutil
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
OUT = os.path.join(REPO, "build", "conv", "determinism")


def digest_tree(root):
    out = {}
    for d, _, files in os.walk(root):
        for f in files:
            p = os.path.join(d, f)
            with open(p, "rb") as fh:
                out[os.path.relpath(p, root)] = hashlib.sha256(fh.read()).hexdigest()
    return out


def run(out, zip_path):
    cache = os.path.join(REPO, "build", "conv", "upstream.json")
    if os.path.exists(cache):
        os.remove(cache)
    cmd = [sys.executable, "-I", os.path.join(REPO, "tools", "conv", "__main__.py"), "--out", out]
    if zip_path:
        cmd += ["--zip", zip_path]
    r = subprocess.run(cmd + ["all"], capture_output=True, text=True)
    if r.returncode:
        sys.stdout.write(r.stdout + r.stderr)
        raise SystemExit("FAIL: converter run failed")
    return digest_tree(out), digest_tree(os.path.join(REPO, "gen"))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--zip")
    a = ap.parse_args()
    shutil.rmtree(OUT, ignore_errors=True)
    p1, g1 = run(os.path.join(OUT, "a"), a.zip)
    p2, g2 = run(os.path.join(OUT, "b"), a.zip)
    bad = [k for k in sorted(set(p1) | set(p2)) if p1.get(k) != p2.get(k)]
    bad += ["gen/" + k for k in sorted(set(g1) | set(g2)) if g1.get(k) != g2.get(k)]
    for k in sorted(p1):
        if k.endswith(".DAT"):
            print("%-22s %s" % (k, p1[k]))
    shutil.rmtree(OUT, ignore_errors=True)
    if bad:
        raise SystemExit("FAIL: differs between runs: %s" % bad)
    print("PASS test_determinism (%d pack files, %d gen files identical)" % (len(p1), len(g1)))


if __name__ == "__main__":
    main()
