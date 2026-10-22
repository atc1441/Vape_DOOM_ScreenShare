#!/bin/sh
# Builds data and firmware from your own IWAD.
#
# The generated tables (palette, colormap, object and state lists,
# visibility tables) come out of the WAD. Firmware and data must therefore
# come from the same GEN tree, or colours and object numbers will not match.
# That is why every WAD build gets its own BUILD and GEN branch; the
# Freedoom state in build/ stays untouched.
set -e
WAD=${WAD:-$HOME/dmc/doom1.wad}
B=${B:-build/ud}
PACKS=${PACKS:-E1}
export PATH="$HOME/opt/usr/lib/llvm-14/bin:/usr/lib/llvm-14/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
export LD_LIBRARY_PATH="$HOME/opt/usr/lib/x86_64-linux-gnu:$HOME/opt/usr/lib/llvm-14/lib:$LD_LIBRARY_PATH"
cd "$HOME/dmc"
make -j8 BUILD="$B" GEN="$B/gen" IWAD="$WAD" CONV_PACKS="$PACKS" packs
make -j8 BUILD="$B" GEN="$B/gen" IWAD="$WAD" py32
ls -l "$B/py32/DOOMVAPE.BIN" "$B/packs/$PACKS/"
