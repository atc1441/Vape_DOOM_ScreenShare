#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
#
# Fetches the pinned doomgeneric commit into oracle/src/doomgeneric-repo.
# ORACLE_MIRROR may name a local clone or another URL to fetch from.
# The checkout is never modified after the fetch: the script refuses to
# touch a tree with local changes and does nothing when the tree is already
# clean and at the pinned commit.

set -eu

COMMIT=dcb7a8dbc7a16ce3dda29382ac9aae9d77d21284
URL=https://github.com/ozkl/doomgeneric.git
HERE=$(cd "$(dirname "$0")" && pwd)
DEST=$HERE/src/doomgeneric-repo
SCRATCH=$(cd "$HERE/.." && pwd)/build
SRC=${ORACLE_MIRROR:-$URL}

die() { echo "oracle/fetch.sh: $*" >&2; exit 1; }

check_clean() {
    st=$(git -C "$DEST" status --porcelain --untracked-files=all --ignored) || die "git status failed in $DEST"
    [ -z "$st" ] || { echo "$st" >&2; die "local modifications in $DEST, refusing to continue"; }
}

verify() {
    head=$(git -C "$DEST" rev-parse --verify HEAD) || die "no HEAD in $DEST"
    [ "$head" = "$COMMIT" ] || die "HEAD is $head, expected $COMMIT"
    git -C "$DEST" cat-file -e "$COMMIT^{commit}" || die "commit object missing"
    git -C "$DEST" fsck --no-dangling --connectivity-only >/dev/null 2>&1 || die "git fsck failed"
    check_clean
    echo "oracle: doomgeneric $COMMIT verified in $DEST"
}

if [ -e "$DEST" ] && [ ! -d "$DEST/.git" ]; then
    die "$DEST exists but is not a git checkout"
fi

if [ -d "$DEST/.git" ]; then
    check_clean
    head=$(git -C "$DEST" rev-parse --verify HEAD 2>/dev/null || true)
    if [ "$head" != "$COMMIT" ]; then
        if ! git -C "$DEST" cat-file -e "$COMMIT^{commit}" 2>/dev/null; then
            git -C "$DEST" fetch --quiet "$SRC" "$COMMIT" 2>/dev/null ||
                git -C "$DEST" fetch --quiet "$SRC" ||
                die "cannot fetch $COMMIT from $SRC"
        fi
        git -C "$DEST" -c advice.detachedHead=false checkout --quiet --detach "$COMMIT"
    fi
    verify
    exit 0
fi

mkdir -p "$HERE/src" "$SCRATCH"
TMP=$SCRATCH/oracle-fetch.$$
rm -rf "$TMP"
trap 'rm -rf "$TMP"' EXIT INT TERM
git clone --quiet --no-checkout "$SRC" "$TMP" || die "clone of $SRC failed"
git -C "$TMP" cat-file -e "$COMMIT^{commit}" 2>/dev/null ||
    git -C "$TMP" fetch --quiet origin "$COMMIT" ||
    die "$SRC does not have $COMMIT"
git -C "$TMP" -c advice.detachedHead=false checkout --quiet --detach "$COMMIT"
mv "$TMP" "$DEST"
trap - EXIT INT TERM
verify
