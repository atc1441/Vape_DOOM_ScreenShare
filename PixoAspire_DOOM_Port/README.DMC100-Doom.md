# DOOM for the FNIRSI DMC-100
<a href="https://www.youtube.com/watch?v=KMTd9A_3Dr4"><img src="https://github.com/user-attachments/assets/0527e898-3196-4cc4-b1ae-fabf02e03812" alt="DOOM running on the FNIRSI DMC-100" width="480"></a>


DOOM running on the FNIRSI DMC-100 clamp meter: the DOOM engine (doomgeneric, derived from id
Software's GPL source release) on an Artery AT32F415 with 32 KB of RAM, 99 KB (101,376 bytes)
of flash for the program and a 4 MB SPI NOR flash without execute in place. It plays episode 1
of [Freedoom: Phase 1](https://freedoom.github.io/) (E1M1 to E1M9 with the title screen,
intermissions, the finale, the menu and four save slots) on the meter's 320x240 screen, with
sound effects on the meter's buzzer. A DJI RC-N1 drone controller can be used as a game pad.

The game logic is the vanilla one: Freedoom's DEMO1 and scripted runs through every map of
episode 1 play tic for tic as in an unmodified doomgeneric, which the host test suite checks.

**Before you start:** flashing DOOM replaces FNIRSI's firmware and may void the meter's
warranty. You do it at your own risk. While DOOM is installed the meter does not measure
anything: do not connect test leads and do not clamp it around a conductor. The original
firmware can be flashed back (see [Going back](#going-back-to-the-original-firmware)).

## What you need

- A FNIRSI DMC-100 and a USB-C cable to a computer. Linux was used throughout, any system
  that can copy files to a USB drive should do.
- To build: GNU make 4.3 or newer, clang and ld.lld 20 or newer, the LLVM tools llvm-objcopy,
  llvm-nm, llvm-size and llvm-dwarfdump, python3 and git, and for the first build
  ca-certificates and network access. With clang 19 or older the firmware does not fit
  (`ld.lld: error: the image runs into the settings page`). clang and lld 23 and Python 3.14
  were used.
- `make` without a target and the tests also need clang's sanitizer runtime (compiler-rt, on
  Ubuntu `libclang-rt-20-dev`). The tests need mtools and dosfstools (fsck.fat), the Renode
  tests [Renode](https://renode.io/) 1.17 and Pillow. `tools/install.py`, `tools/restore.py`
  and `tests/device/usbtest.py` (optional) need udisks2.
- The path of the checkout must not contain spaces.
- Optional: a DJI RC-N1 controller (see [controller/rc-n1](controller/rc-n1/README.md)).

On Ubuntu 24.04 this installs everything but Renode:

```sh
sudo apt install make clang-20 lld-20 llvm-20 libclang-rt-20-dev python3 git ca-certificates \
    mtools dosfstools python3-pil
export PATH=/usr/lib/llvm-20/bin:$PATH
```

## Building

```sh
git clone https://github.com/ArturBieniek4/DMC100-Doom.git
cd DMC100-Doom
make -j$(nproc) fw-package
```

The build downloads two things and checks them:

- the Freedoom 0.13.0 release,
  `https://github.com/freedoom/freedoom/releases/download/v0.13.0/freedoom-0.13.0.zip`,
  SHA-256 `3f9b264f3e3ce503b4fb7f6bdcb1f419d93c7b546f4df3e874dd878db9688f59`, into `dl/`.
  Set `FREEDOOM_ZIP=/path/to/freedoom-0.13.0.zip` to use a copy you already have, or put it
  into `dl/`.
- doomgeneric at commit `dcb7a8dbc7a16ce3dda29382ac9aae9d77d21284` from
  `https://github.com/ozkl/doomgeneric.git` into `oracle/src/` (`oracle/fetch.sh`). The data
  converter reads the original tables from it, and the tests build it unmodified as the
  reference engine. `ORACLE_MIRROR` may name a local clone to fetch from.

The converter (`tools/conv`) turns `freedoom1.wad` into the game data files, so the
repository holds no game data. The results are in `build/device/`:

| File | Size | Goes to |
|---|---|---|
| `DOOM.BIN` | 99 KB | firmware drive, the bootloader flashes it into the meter's microcontroller |
| `DOOMCODE.DMC` | 42 KB | firmware drive, the parts of the program that are loaded into RAM when needed |
| `DOOMX.DAT` | 839 KB | firmware drive, the part of the game data that does not fit on the data drive |
| `DOOM.DAT` | 2,960 KB | data drive, the game data |
| `DOOMRES.DAT` | 4 KB | only for the [Benchmark](#benchmark): firmware drive, keeps its results |
| `DEMO2.LMP` | 8 KB | only for the [Benchmark](#benchmark): data drive, a run through E1M1 |

`DOOM.DAT` and `DOOMX.DAT` are relative symbolic links into `build/packs/E1/`: copy them with
a tool that follows links (`cp` and file managers do). `DOOM.BIN` and `DOOMCODE.DMC` must come
from the same build, and `DOOM.DAT` and `DOOMX.DAT` from the same conversion: the firmware
refuses anything else and says so on the screen. Freedoom's licence and credits for the
converted data are in `build/packs/notices/`.

`make -j$(nproc)` without a target builds the firmware and the main test programs (the host
engine, the reference engine, the test tools and RCPROBE). It does not fill `build/device/`
and runs no tests.

## Installing

The meter has two drives on its NOR flash, both reachable over USB:

- the **firmware drive** (1 MB, label DMC-100), which the bootloader shows in upgrade mode
- the **data drive** (3 MB), where the original firmware keeps its pictures in a folder `UI`

Both show up as USB ID 2E3C:5720. With DOOM installed both drives are full apart from a few
clusters, which is why the files are split the way they are.

While the USB cable powers the meter, switching it off restarts it instead (DOOM starts
again). To start upgrade mode, always unplug the cable first:

1. Unplug the USB cable and switch the meter off. In DOOM use Quit Game in the menu, on its
   installer and error screens hold POWER for 2 s.
2. Hold HOLD and SAVE and press POWER: the meter starts in upgrade mode.
3. Plug the cable in. The computer sees the firmware drive.

Some file managers move deleted files to a trash folder on the drive itself (for example
`.Trash-1000`), where they still take up room. Empty the trash or delete the files with `rm`.

### Quick install with tools/install.py

On Linux, `tools/install.py` does the two steps below and says what to press on the meter in
between. It needs Python 3 and udisks2 (`udisksctl`), not root.

```sh
python3 -I tools/install.py
```

1. Start upgrade mode as above. The script waits for the firmware drive, deletes older DOOM
   files there (and the System Volume Information folder when it needs the room), copies
   `DOOMX.DAT`, `DOOMCODE.DMC` and `DOOM.BIN` from `build/device/` and reads them back. Then
   press SAVE on the meter to flash.
2. Switch the meter on. On the installer screen press MODE the first time: the data drive
   opens as it is, the script copies the meter's pictures (the folder `UI`) to `backup/UI`
   and deletes them on the meter, `DOOM.DAT` needs the room. Once `backup/UI` exists the
   script asks for SAVE instead, which erases the pictures and makes the copy faster. On a
   data error screen (after an update) press SAVE. The script waits for the data drive,
   replaces `DOOM.DAT`, reads it back and ejects the drive. The meter restarts into DOOM.

After the installer's erase (about 8 s) copying `DOOM.DAT` takes about 15 s, over an older
`DOOM.DAT` (an update) about 50 s. `backup/UI` is what
[Going back](#going-back-to-the-original-firmware) needs, `--backup DIR` puts it elsewhere.
Files that are not DOOM's stay on the drives unless they are in the way, and then the script
deletes them only with `--force`. `--build build/sw/device` installs a build of your own WAD,
`--firmware-only` and `--data-only` run one of the two steps and `--res` also copies
`DOOMRES.DAT` for the [Benchmark](#benchmark). `python3 -I tools/install.py -h` lists all
options.

### 1. Flash the firmware

1. Start upgrade mode as above.
2. Delete everything on the firmware drive. DOOM needs all of it apart from about 12 KB, the
   room Windows takes for its System Volume Information folder. Systems that write more
   metadata to a USB drive (macOS) may leave too little room, this was not tried.
3. Copy `DOOM.BIN`, `DOOMCODE.DMC` and `DOOMX.DAT` to it. `DOOM.BIN` must be the only `.BIN`
   file on the drive. Wait until the copy has finished (eject or `sync`).
4. Press SAVE on the meter. The bootloader flashes `DOOM.BIN`, deletes it from the drive and
   switches the meter off.

### 2. Copy the game data

Switch the meter on. DOOM starts, finds no `DOOM.DAT` and shows the installer. `DOOM.DAT`
does not fit next to the meter's own pictures (the folder `UI`, about 1.2 MB), so they have
to go. The original firmware needs these pictures: keep a copy if you want to go back to it
later. The installer offers:

- **SAVE** erases the data drive (a quick format that keeps an empty folder `UI`, then an erase
  of the free space that takes about 8 s and makes the copy faster) and opens it on USB.
- **MODE** opens the data drive on USB as it is. Copy the folder `UI` to the computer, then
  delete everything in it but keep the folder.

On the computer, copy `DOOM.DAT` to the data drive and nothing else. The meter makes
`SAVE.DAT` (64 KB) for the save slots there at the next start, which needs the room that is
left. On Linux, `python3 -I tests/device/usbtest.py --no-test --copy build/device/DOOM.DAT`
finds the drive, copies the file and reads it back.

Then eject the drive on the computer, or press SAVE on the meter, with the cable still in.
The meter restarts, checks the new data once (about a second), creates `SAVE.DAT` and starts
DOOM. Without USB power it switches off instead: switch it on again.

### When the game data has a problem

When the game data has a problem later (a damaged, missing or fragmented file, or the data of
another build), the meter says what is wrong:

- A problem of `DOOM.DAT` or of the data drive: **SAVE** opens the data drive on USB again.
  **MODE** renames the folder `UI` to `UI_OFF`, which makes the bootloader show the data
  drive in upgrade mode in place of the firmware drive, as another way to reach it. A
  checksum error may also be one of `DOOMX.DAT`, so that screen also says how to reach upgrade
  mode.
- `DOOMX.DAT` missing or of another `DOOM.DAT`: the screen says how to reach upgrade mode for
  the firmware drive. An old `DOOM.DAT` (after an update) gives the same error, so SAVE and
  MODE are offered as above.
- Another problem of a file on the firmware drive (damaged or in too many pieces): the screen
  says how to reach upgrade mode and offers only to switch off.
- `DOOMCODE.DMC` missing, damaged or of another build: a red screen says which, for example
  "Copy DOOMCODE.DMC from the same release". Copy it again in upgrade mode.

After MODE, start upgrade mode as above, copy `DOOM.DAT` to the data drive the computer now
sees and eject it. Then unplug the cable and press SAVE: with no `.BIN` file to flash, the
bootloader switches the meter off. Switch it on, DOOM starts and the first good start renames
`UI_OFF` back to `UI`.

### Updating

1. Start upgrade mode, delete the old `DOOMCODE.DMC` and `DOOMX.DAT` (and `DOOMRES.DAT`, copy
   it again after the flash) from the firmware drive and flash the new `DOOM.BIN` with the new
   `DOOMCODE.DMC` and `DOOMX.DAT` as in [Flash the firmware](#1-flash-the-firmware).
2. Switch the meter on. If the new `DOOMX.DAT` belongs to a new `DOOM.DAT`, the meter says
   "DOOMX.DAT is missing on the firmware drive or of another DOOM.DAT." Press SAVE to open
   the data drive on USB, delete the old `DOOM.DAT` there (the drive has room for one) and
   copy the new one. An old `DOOM.DAT` in a format the new firmware does not read gives
   "DOOM.DAT is not DMC DOOM data of this release: copy the DOOM.DAT of DOOM.BIN." with the
   same choices.

Saves belong to the firmware build and to `DOOM.DAT`, so after an update the save slots show
empty.

### Going back to the original firmware

You need the original firmware (the `.bin` file of a FNIRSI update) and the meter's pictures,
which `tools/install.py` copied to `backup/UI`. On Linux, `tools/restore.py` does the two
steps below:

```sh
python3 -I tools/restore.py --firmware APP_DMC-100_V1.4_250524.bin
```

1. In DOOM, unplug the game pad, open the menu, Options, USB drive and answer yes (LIGHT).
   The script deletes `DOOM.DAT`, `SAVE.DAT` and the benchmark demos on the data drive,
   copies the pictures back into the folder `UI`, reads them back and ejects the drive.
2. The meter restarts into the installer. Do not press SAVE there, it erases the pictures
   again: unplug the cable and hold POWER for 2 s.
3. Start upgrade mode as above. The script deletes DOOM's files on the firmware drive and
   copies the firmware as `STOCK.BIN`. Press SAVE on the meter to flash it.

`DOOM.BIN` is longer than the original firmware and covers the flash page where that firmware
keeps its settings (language, backlight, auto power off, the saved readings, no calibration,
as read from firmware V1.4).
`STOCK.BIN` is the firmware padded with 0xFF bytes to 100,352 B, the end of that page, so the
flash also erases it: the firmware starts with its defaults and asks for the language.
`--pictures` takes another copy of the folder `UI` or an image of the data drive (read with
mtools), `--flash-dump` a copy of the internal flash (128 KB) from before DOOM, whose
settings page is then put back. `--pictures-only` and `--firmware-only` run one step.

By hand: delete `DOOM.DAT` and `SAVE.DAT` on the data drive (Options, USB drive) and copy the
pictures back into `UI`, then flash the padded firmware the same way as `DOOM.BIN`, after
deleting `DOOMCODE.DMC`, `DOOMX.DAT` and `DOOMRES.DAT` on the firmware drive. To pad it:

```sh
python3 -c 'import sys; d = open(sys.argv[1], "rb").read(); open("STOCK.BIN", "wb").write(d + b"\xff" * (100352 - len(d)))' APP_DMC-100_V1.4_250524.bin
```

## Building with your own DOOM WAD

Freedoom is the default and the recommended choice. If you own DOOM, episode 1 of the
shareware `DOOM1.WAD` or of the registered `DOOM.WAD` can be converted instead (only the
shareware `DOOM1.WAD` 1.9 was tried). The WAD is never part of this repository: keep it
outside the tree and point the build at it.

```sh
make -j$(nproc) IWAD=/path/to/DOOM1.WAD BUILD=build/sw fw-package
```

`IWAD` needs a build directory of its own, the default `build/` stays the Freedoom build. The
Freedoom zip is not needed for it, doomgeneric is still fetched. The files are in
`build/sw/device/`: `DOOM.BIN` and `DOOMCODE.DMC` go to the firmware drive and `DOOM.DAT` to
the data drive. The shareware data fits in `DOOM.DAT`, so there is no `DOOMX.DAT`. These files
contain data from your WAD: they are for your own meter only, do not share them.

The firmware is made for the WAD its build converted. At start it checks that `DOOMCODE.DMC`
belongs to `DOOM.BIN`, that `DOOM.DAT` was converted from the WAD of the firmware and that
`DOOMX.DAT` belongs to `DOOM.DAT`, so a `DOOM.DAT` of another WAD is refused. To switch
between builds:

1. Start upgrade mode. The firmware drive is full: delete `DOOMCODE.DMC` and `DOOMX.DAT`
   (and `DOOMRES.DAT`) first. Then copy the new `DOOM.BIN` and `DOOMCODE.DMC`, and the new
   `DOOMX.DAT` if the build has one, and press SAVE to flash.
2. Switch the meter on. It says "DOOM.DAT is not DMC DOOM data of this release: copy the
   DOOM.DAT of DOOM.BIN." Press SAVE to open the data drive on USB (or MODE to show it in
   upgrade mode, see [When the game data has a problem](#when-the-game-data-has-a-problem)).
   The data drive holds one `DOOM.DAT`: delete the old one, then copy the new one.

## Playing

Hold the meter sideways with the clamp to the left and the buttons on the right. SAVE is the
button labelled SAVE/MENU.

| Button | Alone | With POWER held first |
|---|---|---|
| V-A | forward | next weapon |
| HOLD | backward | previous weapon |
| MODE | turn left | strafe left |
| SAVE | turn right | strafe right |
| LIGHT | fire | use (doors, switches) |
| POWER (tap) | menu | |

POWER is a shift key only when it is pressed before the other button. Held on its own for
1 s it opens or closes the menu, held on to 4 s it opens the quit prompt. Always-run is on. On
the title screen and during the demos any button opens the menu.

In the menu V-A moves up, HOLD down, LIGHT selects and a POWER tap closes the menu. MODE and
SAVE only change the sound level. There is no key to go back one page. In a yes or no prompt
LIGHT is yes and a POWER tap is no.

The menu has New Game, Options, Load Game, Save Game and Quit Game. Quit Game switches the
meter off, or starts DOOM again while USB powers the meter. The skill page starts on the
easiest skill and then remembers the last pick, Nightmare is not available. Options has End
Game, Messages, Sound (off, low, high), USB drive and Benchmark. USB drive asks first, then
ends the game and opens the data drive on USB, and the meter restarts after the eject.

With a DJI RC-N1 connected to the meter (see [controller/rc-n1](controller/rc-n1/README.md))
the sticks move and turn with analog control, its buttons fire, use and open the menu, and
its dial changes the weapon. It works on the title screen, in the menu and in the game. The
meter has one USB port: unplug the controller before Options, USB drive.

### Benchmark

Options, Benchmark asks first, then ends the game and plays each demo twice with the buzzer
muted: once without drawing, with a hash of the game state of every tic that can be compared
with the host engine, and once drawing every frame, timed. MODE stops the run, POWER held for
2 s switches the meter off. The results screen shows the frame rate of each demo when the
firmware drive has a `DOOMRES.DAT` (see below), without one it only says that the results are
not saved. On it SAVE goes back to the title screen, MODE runs the demos again and HOLD opens
the data drive on USB.

DEMO1 is part of the game data. `DEMO2.LMP` to `DEMO4.LMP` are played as well when they are
in the root folder of the data drive. They must be v1.9 demos of one player on a map of
episode 1, at most Ultra-Violence, without `-fast` or `-respawn`. `build/device/DEMO2.LMP` is
a run through E1M1. With the Freedoom data there is room for it only when Windows has not
taken its reserve on the drive.

The results are kept only when the firmware drive has a `DOOMRES.DAT` of 4 KB: copy
`build/device/DOOMRES.DAT` there in upgrade mode after `DOOM.BIN` has been flashed (it takes
the room `DOOM.BIN` had). To read them, copy the file back in upgrade mode and run

```sh
python3 -I tests/device/doomres.py DOOMRES.DAT
```

With `--host build/host/dmcdoom_skip --nor build/nor/E1.img --lmp-dir build/device` it also
plays the demos on the host and compares the hashes (`make -j$(nproc) build/host/dmcdoom_skip
nor-images` builds both files).

### Limitations

- Episode 1 of Freedoom: Phase 1 or of your own WAD only, single player.
- Low detail (160 columns, each drawn twice), no automap, no Nightmare skill.
- No music. Sound effects are the PC speaker versions, one at a time, on the buzzer.
- 29 frames per second on average with the default build in the DEMO1 of the shareware
  `DOOM1.WAD` (E1M5 on Hurt Me Plenty). The heaviest case measured, Freedoom's DEMO1 (E1M6 on
  Hurt Me Plenty), ran at 24 frames per second on average (half of the frames under 39 ms)
  with the NOR flash read at 72 MHz. The default build reads it at 36 MHz instead
  (`PLAT_NOR_SLOW=0` selects 72 MHz): at 72 MHz 2 of the 4 KB blocks in 1 GB read on the meter
  came back wrong, at 36 MHz none, and a wrong byte stops the game with a data error.
  Freedoom's DEMO1 at 36 MHz has not been measured yet, the Benchmark shows the figure of your
  build. Lighter areas run faster, large open ones slower.

## Tests

```sh
make -j$(nproc) test
```

runs the host test suite (a few minutes on a desktop, under 1 GB of disk in `build/`). Among
others it plays demos and scripted routes through every map in lockstep with the unmodified
doomgeneric and compares the game state every tic, compares the 3D view and the front end
screens pixel for pixel with it, and checks the data converter, the data layer on damaged and
fragmented drives, the overlay packer, the saves, the USB drive and the controller driver.

With Renode 1.17 installed, `make -j$(nproc) fw-renode-test` boots the firmware in an emulated
meter and plays the demos against the host engine, and `make -j$(nproc) ovl-renode-test` runs
the overlay loader there. Both take `renode` from the `PATH`, or `RENODE=/path/to/renode`.

## Layout

| Path | Contents |
|---|---|
| `engine/` | doomgeneric's DOOM engine, modified for the meter (`engine/UPSTREAM.txt` lists the imported files, `tools/upstream_diff.sh` shows the changes), plus `engine/p_mapdata.*` (the map data read from the NOR flash) and `engine/stub/` (stub C library and platform of the ARM size build), written for the port |
| `port/` | the port: data layer on the NOR flash (`data`), strip renderer (`render`), object store (`obj`), overlay loader (`ovl`), USB drive (`usb`), fixed point and table decoders (`math`), firmware glue, menu and input (`game`) |
| `platform/` | drivers of the meter: clock, buttons, buzzer, LCD, NOR flash, USB device controller, link scripts, text screens (`platform/apps`) and host models for their tests |
| `controller/rc-n1/` | DJI RC-N1 game pad driver and its host test, the probe app RCPROBE with its log printer, protocol notes |
| `tools/` | installer (`tools/install.py`) and its way back (`tools/restore.py`), data converter (`tools/conv`), overlay packer, size reports and gates, checkers of the converted data, demo and route makers, font generator, upstream import and diff |
| `oracle/` | the reference engine for the tests, built from unmodified doomgeneric |
| `host/`, `tests/` | host build of the engine and the test suite |
| `mk/` | build rules, one file per part |

## Licence

The firmware and the tools are GPL-2.0-or-later (`COPYING`, `LICENSES/`). Some parts carry
other compatible licences, and the game data built from Freedoom is under Freedoom's BSD
licence (the engine strings and tables in it under the GPL). This project publishes source only. See [CREDITS.md](CREDITS.md) for the details and
for what anyone who distributes binaries has to ship with them.

## USB IDs and trademarks

The data drive of DOOM uses USB ID 2E3C:5720, Artery's vendor ID with the product ID that the
meter's own bootloader uses. DOOM keeps it so that both modes look the same to the computer.
For the controller the meter enumerates as 18D1:4EE1 and then as 18D1:2D00, Google's IDs, the
second prescribed by the Android Open Accessory protocol. They are used only for
compatibility with the controller.

This project is not affiliated with, endorsed or sponsored by id Software, ZeniMax Media,
Microsoft, FNIRSI, Artery Technology, DJI, Google or the Freedoom project. DOOM is a
registered trademark of id Software LLC, a ZeniMax Media company. FNIRSI, Artery, DJI, RC-N1,
Google and Android are trademarks of their owners, used here only to name the hardware and
protocols this project works with.
