# DOOM on the Aspire PIXO — natively

This is Doom running on the vape's own CPU. No PC, no screen share: the
PY32F403 renders every frame itself, reads the map data from the 16 MB SPI
flash and takes input from the touch panel.

![](../VAPE_DOOMed.jpg)

This repo is made together with this explanation video:(click on the image)

[https://www.youtube.com/watch?v=LNx4T7kC93w](https://www.youtube.com/watch?v=LNx4T7kC93w)

[![YoutubeVideo](https://img.youtube.com/vi/LNx4T7kC93w/0.jpg)](https://www.youtube.com/watch?v=LNx4T7kC93w)

## Credit where it belongs

**The engine is [DMC100-Doom](https://github.com/ArturBieniek4/DMC100-Doom)
by Artur Bieniek.** That port is the whole reason this is possible: it is a
Doom built to run on a meter with **32 KB of RAM**, with a two-pass strip
renderer that never holds a framebuffer, map data streamed from serial
flash, and a record cache in place of the usual level-sized allocations. A
stock Doom needs several megabytes. Without that work there is nothing to
put on a vape.

**The board, its bring-up and the screen-share firmware are
[@atc1441](https://github.com/atc1441/Vape_DOOM_ScreenShare)'s.** The pinout,
the panel init sequence, the external-watchdog finding on PC6 and the flash
dumps in this repo all come from there, and this port starts from that
register configuration rather than from the reference manual.

What is new here is the platform layer that puts the two together: panel
driver, SPI pump, NOR access, touch input, memory map, build rules and the
host-side tools in `tools/`.

Everything is GPL-2.0-or-later, like Doom itself.

## The hardware, and what it means

| | |
|---|---|
| CPU | Puya PY32F403xC, Cortex-M4F @ 144 MHz |
| RAM | 64 KB |
| Flash | 256 KB |
| External | 16 MB SPI NOR (JEDEC `0x852018`) |
| Panel | ST7789-style, 173 × 323 native portrait, column offset 34 |
| Touch | CST816D on I²C address `0x15` |
| Keys | one, on PE15 |
| Watchdogs | internal IWDG **and** an external chip on PC6 driving NRST |

Three of these shape the whole port.

**The panel and the NOR share one SPI bus and one data wire.** Not just the
bus — PB4 and PB5 are the same net, so reading the flash means releasing
MOSI from the pad first. While a frame goes out, no map data can be fetched,
and the other way round. A DMA pump would have nothing to overlap with, so
the pixel pump is polled; that is not a compromise here, it is the shape of
the hardware, and it removes an interrupt that would otherwise have to
arbitrate for the bus.

**The screen is portrait and the game is landscape.** MADCTL 0x40 was picked
by measurement, not from the datasheet: with it the panel's row index runs
the same way as the screen's x axis, so a column of pixels is one contiguous
run in panel memory and a frame streams without a single extra window
command. That is also the order the strip renderer produces columns in.
240 screen rows are squeezed into 173 panel columns, and that squeeze lives
in `lcd.c` and nowhere else.

**The external watchdog is not optional.** It pulls NRST whenever nothing
toggles PC6. Both watchdogs are fed from the SysTick at 1 kHz, never from
the game loop — a full frame takes over 50 ms, and a slow frame fed from the
loop would be a reset. It also makes flashing awkward; see below.

## Controls

The glass carries everything needed to play.

| | |
|---|---|
| **Left area** (x < 250) | Steering cross around (125, 120), dead zone ±45 × ±40. Up walks, down backs up, left and right turn, and the corners do both at once — a finger at the top left is *forward and turn left together*, which no pair of real keys on this device could do. |
| **Right strip** (x ≥ 250) | Top is use, bottom is fire. Use is the shift key plus fire, which is exactly what the chord resolver turns into `KEY_USE`. |
| **Top-left corner**, 44 × 36 | Menu |
| **Key (PE15)** | fires as well — a bonus, not a requirement |

The touch panel is read in the SysTick interrupt rather than in the game
loop: the CPU spends most of a frame inside the renderer, and input sampled
there would arrive in bursts.

**The menu corner is a button, not a key you hold.** The resolver gives
POWER three meanings — tap opens the menu, one second toggles it, four
seconds open the quit prompt whose yes calls `I_Quit`, i.e. powers the
device off. Under a thumb on a real switch that is a sensible ladder. On
glass it is not: a finger left lying in the corner walks through all three
without anyone's help. So the corner reports POWER for 150 ms and nothing
after that until the finger leaves. One press, one menu.

## Which pin the key is on

`Pinout.txt` names the connector pins, not their jobs. This port assumed
"PC5 fires, PE15 is shift and menu" for a long time, and that was wrong.

Counted out, the stock firmware's configuration leaves only **five digital
inputs in the entire chip** — everything else is output, alternate function
or analog, and analog cannot report anything because the Schmitt trigger is
off. Four of the five sit on the connector with no known job.
`tools/pins.py` watched them while somebody pressed the key:

| Pin | Pull | Idle | measured |
|---|---|---|---|
| **PE15** | up | 1 | goes to 0 on a press — **the key** |
| PE6 | down | 0 | flips on its own about twice a second — a signal |
| PC13 | — | 0 | never moved |
| PC5 | **none** | 1 | floats, changes by itself |

**PC5 with no pull resistor was the actual bug.** While `key_bits()` OR-ed it
in, it twitched in the middle of a press:

```
104.4s  PE15=0  PC5=1   buttons 0x00 -> 0x10
104.4s  PE15=0  PC5=0   buttons       0x10
104.5s  PE15=0  PC5=0   buttons       0x00     <- gone
104.5s  PE15=0  PC5=1   buttons       0x10
```

Every change resets the candidate in `buttons_sample()`, `cand_n` never
reaches three, no edge is produced, and the press disappears. Sometimes it
got through, sometimes it did not. The firmware now reads PE15 only.

Two things worth knowing if you test it yourself: a press **on the title
screen** does nothing of the sort — there `LIGHT` is menu-confirm, not fire.
And measure the key at `gamekeydown`, the array `g_game.c` builds the ticcmd
from, not at what you think you see on screen.

## Two bugs that were hiding under the table

**`--wrap=D_PostEvent` was missing from the build rules.** Under `DMC_FW`,
`engine/d_event.c` keeps a ring of *two* events, because `port/game/i_dmc.c`
drains each event the moment it is posted — through a `--wrap` redirect that
`mk/fw.mk` sets and these rules did not have. Without it nothing drained,
and two events in a row made the ring's head and tail coincide: the queue
looked empty and **both were gone**.

On the DMC-100 that never shows. Five switches and one thumb do not produce
two simultaneous presses, and two at once there are a chord, i.e. one key. A
glass surface produces them constantly — a finger at the top left is exactly
that. Measured: each input alone worked, both together did nothing, one
after the other worked again.

**`RC_SETS=256` was silently inactive.** `port/data/cache.h` has
`#define RC_SETS 32` with no guard, so the header quietly wins against
`-DRC_SETS=256` — the build goes on, just eight times smaller. Visible only
as NOR accesses per frame, never as an error. With the guard in place:
**4,770 → 483 accesses per frame, 57.8 ms → 9.6 ms**, frame rate 11–15 →
**19.8 fps measured, 25 fps in play**.

## Measurements

On the device, with `tools/prof.py`:

| | |
|---|---|
| Frame rate | 19.8 fps (50.6 ms per frame) |
| NOR | 9.6 ms/frame, 483 accesses, 19.6 KB |
| Panel | 22.7 ms/frame, 46,326 pixels |
| Engine | 18.3 ms/frame |
| Flash | 189 KB of 254 KB |
| RAM | 8.4 KB fixed, 18.2 KB cache, the rest arena |

**The panel is the bottleneck now, and it is at the floor.** 46,326 pixels
are 92,652 bytes, and at 36 MHz — PCLK2/2, the part's maximum — that is
22.7 ms. Nothing more comes off without a faster bus; the 9 % gap to the
theoretical maximum is the window commands and the byte loop.

That the data really is Doom's episode 1 is not read off a photograph but
out of RAM: the player starts at **(1056, −3616) facing 90°**, which is
E1M1's player-1 start, exact to the unit.

## Building

The port is the `platform/py32` layer plus build rules for
[DMC100-Doom](https://github.com/ArturBieniek4/DMC100-Doom). Clone that,
drop these files in, and build with clang/lld 14:

```sh
git clone https://github.com/ArturBieniek4/DMC100-Doom dmc
cp -r port/platform/py32   dmc/platform/
cp    port/link/*.ld       dmc/platform/link/
cp    port/mk/py32.mk      dmc/mk/
python port/tools/patch_upstream.py --root dmc   # three small upstream tweaks

cd dmc
make -j8 BUILD=build/ud GEN=build/ud/gen IWAD=/path/to/doom1.wad CONV_PACKS=E1 packs
make -j8 BUILD=build/ud GEN=build/ud/gen IWAD=/path/to/doom1.wad py32
```

`patch_upstream.py` applies three things and prints what it did:
`OVL_RESIDENT` support in `port/ovl/ovl.h` (this build has no overlays — the
flash is two and a half times larger and an overlay would live on the NOR
that shares its bus with the panel, so loading code would have to interrupt
a frame); the `#ifndef` guard around `RC_SETS`; and restricting the
converter's object-type scan to the maps actually present, so a WAD with
fewer than nine E1 maps does not abort.

No Doom data is shipped here. Bring your own `doom1.wad`.

## Flashing

```sh
python tools/mknor.py --doom out/DOOM.DAT --doomx out/DOOMX.DAT -o out/nor.bin
python tools/checknor.py out/nor.bin        # check the image without hardware
python ../PixoAspire_Vape_FW/tools/nortool.py write out/nor.bin 0   # once, ~2.5 min
python tools/flash.py                       # the firmware
```

**Do not use a single `loadfile` for the firmware.** The external watchdog
pulls NRST as soon as nothing feeds it, and `erase` takes away the firmware
that was doing so — from then on it resets the part about once a second. A
write across the whole image takes ten seconds and does not survive that:
measured, it breaks off after roughly 180 KB with a
`T-bit of XPSR is 0 but should be 1` in the middle, which is exactly such a
reset. After that the flash is half written, the CPU runs off an empty reset
vector into lockup, and the next attempt fails at the erase with
`Cannot read register ... while CPU is running`. That is a state you do not
get out of by trying again.

`tools/flash.py` writes in **32 KiB chunks**, each with its own `r`/`h`
first. A chunk takes about a second and fits inside a watchdog window, what
is already in flash survives the reset, and a chunk that does not land is
repeated on its own.

If you do end up there: resetting with `DEMCR.VC_CORERESET` set halts the
core **before** the first instruction, so it cannot trip over the empty
vector, and from there the chip can be erased again.

**Verify over a fresh connection.** Otherwise you are not reading flash, you
are reading the loader's buffer, which cheerfully reports what ought to be
there. A completely blank chip looked like a full one here, including
"0 blocks wrong".

## Tools

| | |
|---|---|
| `tools/flash.py` | firmware, in chunks, verified |
| `tools/mknor.py`, `checknor.py` | build and check the NOR image |
| `tools/vapedoom.py` | debug ring over SWD, progress, state |
| `tools/prof.py` | where a frame spends its time |
| `tools/touch.py` | inject input and prove its effect |
| `tools/pins.py` | watch which pin moves |
| `tools/shot.py` | photograph the display |

There is no serial port on this board and USB is the vape's own connector,
so progress and panics go into a ring in RAM that the host reads with the
debugger while the CPU runs.

`touch.py` is the interesting one. A finger cannot be put in the same place
twice, so the firmware carries an injection block that the debugger fills in
at run time, and the proof is not a camera frame but the engine's own state:
`players[0]` is in RAM, so position and facing before and after an input are
readable. "Forward" is then a distance in map units, not an impression.

```
input                forward   sideways     turn    expected  result
forward                +356.0       +0.1     +0.0    +    .   0     ok
back                   -321.5       -0.1     +0.0    -    .   0     ok
turn left                +0.0       -0.0   +107.2    .    .   +     ok
forward + left         +231.9      -63.7   +114.3    +    .   +     ok
strafe left              -0.1     +244.9     +0.0    .    +   0     ok
use (right strip)        +0.0       +0.0     +0.0    0    0   0     ok
```

What it cannot answer is whether a finger at a given spot on the glass
produces the raw value the controller ought to report there — that is the
one point where injection runs in a circle, and it is settled elsewhere.

## Safety

* **PB13 (load path / heating coil)** is driven LOW in the reset handler,
  before the clock is touched or `.data` is copied, and again in the fault
  handler before any C runs there. LOW is the state the stock firmware holds
  permanently. Nothing in this port ever drives it high.
* **Both watchdogs** are fed from the SysTick at 1 kHz.
* **The fault screen** turns off the load path, writes PC/LR/CFSR to the
  display and to the debug ring, and keeps feeding the watchdogs by hand
  instead of dropping into a reset loop.
* **The NOR is only written in its first 4 MB.** The vape's original content
  is still there above that, and it is backed up in this repo.

## Still open

* **Sound.** There is no speaker. The vibration motor is on the board but
  the stock firmware never drives it: all 35 `HAL_GPIO_WritePin` call sites
  and the single direct GPIO write in the dump cover the LCD, the backlight,
  PB13 and the idle levels, and no timer channel is configured as an
  alternate function. So the pin cannot be recovered from the program, and
  eleven connector-exposed outputs remain unnamed. Blind-pulsing one of them
  on a device with a heating coil and a sealed lipo is not a good idea.
* **Episodes beyond E1.** The converter handles them; the NOR has room.
* **The frame rate** is at the panel's bus limit. A faster SPI mode would be
  the only lever.
