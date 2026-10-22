#!/usr/bin/env python3
"""
patch_upstream.py - the three spots in DMC100-Doom this port needs.

Apart from these the port leaves the original alone: it brings its own
platform, its own linker scripts and its own build rules. Three spots
cannot be reached from outside:

  port/ovl/ovl.h    The overlay tables are plain arrays only under
                    PLAT_HOST. This build is fully resident, so it needs the
                    same shape - but it is not a host build.
  port/data/cache.h RC_SETS is fixed at 32. On the DMC-100 that is right;
                    here there is twice the RAM. Without a guard the header
                    wins against -DRC_SETS=256, and does so silently: the
                    build goes on, just eight times smaller.
  tools/conv/info.py The converter walks all nine maps of episode 1 to work
                    out which object types have to end up in the tables. A
                    WAD with only E1M1 aborts there. Restricting it to the
                    maps actually present is not merely allowed, it produces
                    smaller tables.

All idempotent, and reversible with --revert.
"""

import argparse
import os
import sys

# --root points at the DMC100-Doom tree; the default is the tree this
# file sits in (tools/vape/patch_upstream.py -> three levels up).
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

PATCHES = [
    ("port/ovl/ovl.h",
     "#ifdef PLAT_HOST\n#define OVL_TABLE(name, ...)",
     "#if defined(PLAT_HOST) || defined(OVL_RESIDENT)\n#define OVL_TABLE(name, ...)"),
    # The line before it belongs in the pattern so that `old` is not a
    # substring of `new`: otherwise the idempotency check below finds the
    # pattern in its own result and lays another guard on top with every
    # run. Nested #ifndef stay functionally right - RC_SETS comes from -D
    # and the outer guard takes effect - the file just grows by two lines
    # on every sync.
    ("port/data/cache.h",
     "#define RC_WAYS 4\n#define RC_SETS 32",
     "#define RC_WAYS 4\n#ifndef RC_SETS\n#define RC_SETS 32\n#endif"),
    ("tools/conv/info.py",
     "    types, states, weapons = closure(info, roster_types(wad, info, E1_MAPS))",
     "    have = [m for m in E1_MAPS if wad.num(m) >= 0]\n"
     "    types, states, weapons = closure(info, roster_types(wad, info, have))"),
]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=ROOT)
    ap.add_argument("--revert", action="store_true")
    a = ap.parse_args()

    for rel, old, new in PATCHES:
        # See the comment at the cache.h entry: overlapping patterns make
        # the patch silently repeatable.
        assert old not in new and new not in old, rel
        p = os.path.join(a.root, rel.replace("/", os.sep))
        s = open(p, encoding="utf-8").read()
        frm, to = (new, old) if a.revert else (old, new)
        if to in s and frm not in s:
            print("[patch] %s %s" % (rel, "already reverted" if a.revert else "already applied"))
            continue
        if frm not in s:
            print("[patch] %s: pattern not found" % rel)
            return 1
        open(p, "w", encoding="utf-8", newline="").write(s.replace(frm, to))
        print("[patch] %s %s" % (rel, "reverted" if a.revert else "applied"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
