#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
#
# Prints the unified diff of engine/ against the pinned doomgeneric files
# listed in engine/UPSTREAM.txt. Files that were imported and later removed
# show as deletions. Files in engine/ that are not from upstream are listed
# at the end. Options: --stat for a per-file summary only.

set -eu

ROOT=$(cd "$(dirname "$0")/.." && pwd)
UP=$ROOT/oracle/src/doomgeneric-repo/doomgeneric
LIST=$ROOT/engine/UPSTREAM.txt
STAT=0
[ "${1:-}" = "--stat" ] && STAT=1

[ -d "$UP" ] || { echo "missing $UP, run oracle/fetch.sh" >&2; exit 1; }
[ -f "$LIST" ] || { echo "missing $LIST, run tools/import_upstream.py" >&2; exit 1; }

cd "$ROOT"
files=$(grep -v '^#' "$LIST" | awk 'NF == 2 { print $2 }')
for f in $files; do
    up=oracle/src/doomgeneric-repo/doomgeneric/$f
    if [ ! -e "engine/$f" ]; then
        if [ $STAT = 1 ]; then
            echo "deleted   $f"
        else
            diff -u --label "a/engine/$f" --label /dev/null "$up" /dev/null || true
        fi
    elif ! cmp -s "$up" "engine/$f"; then
        if [ $STAT = 1 ]; then
            n=$(diff "$up" "engine/$f" | grep -c '^[<>]' || true)
            echo "modified  $f ($n lines)"
        else
            diff -u --label "a/engine/$f" --label "b/engine/$f" "$up" "engine/$f" || true
        fi
    fi
done

echo "# not from upstream:"
find engine -type f ! -name UPSTREAM.txt | sed 's|^engine/||' | sort | while read -r f; do
    echo "$files" | grep -qx "$f" || echo "#   engine/$f"
done
