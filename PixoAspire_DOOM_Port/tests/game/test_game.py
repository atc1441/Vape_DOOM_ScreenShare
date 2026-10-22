#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright(C) 2005-2014 Simon Howard
# Copyright (C) 2026 Artur Bieniek
"""Host tests of port/game/: the button chord resolver and the PC-speaker sound.

Input: scripted button sequences through build/game/input_test, checked against the
expected key events (vanilla default key codes).
Sound: sounds started through the engine's s_sound.c in build/game/pcspk_test, which runs
the platform's 140 Hz stepper on a TMR4 register model. The buzzer frequency of every step
is checked against the DP lump decoded straight from freedoom1.wad. Square-wave WAVs of the
traces are written for listening.
"""

import argparse
import os
import random
import struct
import subprocess
import sys
import wave
import zipfile

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
WAD_MEMBER = "freedoom-0.13.0/freedoom1.wad"

BTN = {"HOLD": 0, "SAVE": 1, "MODE": 2, "VA": 3, "LIGHT": 4, "POWER": 5}
ACT = {name: i for i, name in enumerate(
    "NONE FWD BACK TURNL TURNR STRAFEL STRAFER FIRE USE RUN STRAFE MAP NEXTW PREVW".split())}
WP = {name: i for i, name in enumerate(
    "fist pistol shotgun chaingun missile plasma bfg chainsaw supershotgun".split())}
KEY_RSHIFT = 0x80 + 0x36
TICRATE = 35

# Chocolate Doom src/i_pcsound.c divisors, tone byte -> PIT divisor
DIVISORS = [
    0,
    6818, 6628, 6449, 6279, 6087, 5906, 5736, 5575,
    5423, 5279, 5120, 4971, 4830, 4697, 4554, 4435,
    4307, 4186, 4058, 3950, 3836, 3728, 3615, 3519,
    3418, 3323, 3224, 3131, 3043, 2960, 2875, 2794,
    2711, 2633, 2560, 2485, 2415, 2348, 2281, 2213,
    2153, 2089, 2032, 1975, 1918, 1864, 1810, 1757,
    1709, 1659, 1612, 1565, 1521, 1478, 1435, 1395,
    1355, 1316, 1280, 1242, 1207, 1173, 1140, 1107,
    1075, 1045, 1015, 986, 959, 931, 905, 879,
    854, 829, 806, 783, 760, 739, 718, 697,
    677, 658, 640, 621, 604, 586, 570, 553,
    538, 522, 507, 493, 479, 465, 452, 439,
    427, 415, 403, 391, 380, 369, 359, 348,
    339, 329, 319, 310, 302, 293, 285, 276,
    269, 261, 253, 246, 239, 232, 226, 219,
    213, 207, 201, 195, 190, 184, 179,
]
PIT_HZ = 1193181
TMR_HZ = 9000000  # TMR4 counter clock on the meter, 144 MHz / 16
# Sounds Chocolate Doom's PC speaker driver never plays
DISABLED = {"posact", "bgact", "dmact", "dmpain", "popain", "sawidl"}

failures = []


def check(cond, msg):
    if not cond:
        failures.append(msg)
        print("FAIL: " + msg)
    return cond


# ---------------------------------------------------------------- input


class Script:
    """Button script for input_test. t is the index of the next tic line, unbuilt holds the
    tics whose I_StartTic builds no ticcmd."""

    def __init__(self):
        self.lines = []
        self.mask = 0
        self.ctx = "g"
        self.t = 0
        self.unbuilt = set()

    def cmd(self, line):
        self.lines.append(line)

    def tics(self, n=1, extra="", built=True):
        for _ in range(n):
            self.lines.append("%s %s %d %s" % ("t" if built else "u", self.ctx, self.mask, extra))
            if not built:
                self.unbuilt.add(self.t)
            self.t += 1

    def down(self, *names):
        for n in names:
            self.mask |= 1 << BTN[n]

    def up(self, *names):
        for n in names:
            self.mask &= ~(1 << BTN[n])

    def run(self, exe):
        p = subprocess.run([exe], input="\n".join(self.lines) + "\n", capture_output=True,
                           text=True)
        if p.returncode:
            sys.exit("%s failed: %s" % (exe, p.stderr))
        events, speed = [], []
        for line in p.stdout.splitlines():
            f = line.split()
            if f[0] == "speed":
                speed.append(int(f[1]))
            else:
                events.append((int(f[0]), f[1]))
        return events, speed


def balanced(events, unbuilt=()):
    """Every key that goes up is down, and was down in at least one built ticcmd: events
    come only in built tics (the ticcmd of a tic sees its events), a keyup comes in a later
    tic than the last keydown. No key stays down at the end."""
    down = {}
    for t, e in events:
        k = e[1:]
        if t in unbuilt:
            return "%s at tic %d, which builds no ticcmd" % (e, t)
        if e[0] == "+":
            down[k] = t
        elif k not in down:
            return "%s at tic %d without a keydown" % (e, t)
        elif down.pop(k) == t:
            return "%s at tic %d in the tic of its keydown" % (e, t)
    return "keys left down: %s" % sorted(down) if down else None


def expect(name, exe, s, want):
    got, _ = s.run(exe)
    bad = balanced(got, s.unbuilt)
    check(bad is None, "%s: %s" % (name, bad))
    ok = check(got == want, "%s:\n  want %s\n  got  %s" % (name, want, got))
    if ok:
        print("ok   input: %s (%d events)" % (name, len(got)))
    return got


def tap_events(t, key):
    return [(t, "+" + key), (t + 1, "-" + key)]


def next_weapon_ref(ready, pending, owned, strength, direction):
    """Chocolate Doom's G_NextWeapon for Doom 1, returns the weapon number key."""
    order = [("fist", "fist"), ("chainsaw", "fist"), ("pistol", "pistol"),
             ("shotgun", "shotgun"), ("supershotgun", "shotgun"), ("chaingun", "chaingun"),
             ("missile", "missile"), ("plasma", "plasma"), ("bfg", "bfg")]

    def selectable(w):
        if w == "supershotgun" or w not in owned:
            return False
        return not (w == "fist" and "chainsaw" in owned and not strength)

    w = ready if pending is None else pending
    i = [o[0] for o in order].index(w)
    start = i
    while True:
        i = (i + direction) % len(order)
        if i == start or selectable(order[i][0]):
            break
    return str(WP[order[i][1]] + 1)


def test_input(exe):
    # Each gameplay button alone, held 40 tics (LIGHT held = autofire, one keydown)
    for btn, key in (("VA", "UP"), ("HOLD", "DOWN"), ("MODE", "LEFT"), ("SAVE", "RIGHT"),
                     ("LIGHT", "FIRE")):
        s = Script()
        s.tics(2)
        t0 = s.t
        s.down(btn)
        s.tics(40)
        t1 = s.t
        s.up(btn)
        s.tics(3)
        expect("%s alone" % btn, exe, s, [(t0, "+" + key), (t1, "-" + key)])

    # Holding any gameplay button for 2 s never opens the menu (a long turn must not open it)
    for btn in ("HOLD", "VA", "MODE", "SAVE", "LIGHT"):
        s = Script()
        s.down(btn)
        s.tics(2 * TICRATE)
        s.up(btn)
        s.tics(2)
        got, _ = s.run(exe)
        check(len(got) == 2 and "+ESC" not in [e for _, e in got],
              "%s held 2 s: %s" % (btn, got))
    print("ok   input: gameplay buttons held 2 s send no menu key")

    # POWER tap: the menu key on release, one tic long
    s = Script()
    s.tics(2)
    s.down("POWER")
    s.tics(5)
    t1 = s.t
    s.up("POWER")
    s.tics(3)
    expect("POWER tap = menu on release", exe, s, tap_events(t1, "ESC"))

    # POWER 0.9 s is still a tap
    s = Script()
    s.down("POWER")
    s.tics(31)
    t1 = s.t
    s.up("POWER")
    s.tics(3)
    expect("POWER 0.9 s = menu on release", exe, s, tap_events(t1, "ESC"))

    # POWER chords: shifted actions, no menu key on release
    s = Script()
    s.cmd("weap %d -1 %d 0" % (WP["pistol"], 0b1111))  # fist pistol shotgun chaingun
    s.tics(1)
    s.down("POWER")
    s.tics(3)
    ev = []
    for btn, key, held in (("MODE", "STRAFE_L", True), ("SAVE", "STRAFE_R", True),
                           ("VA", "3", False), ("HOLD", "1", False),
                           ("LIGHT", "USE", True)):
        t0 = s.t
        s.down(btn)
        s.tics(6)
        t1 = s.t
        s.up(btn)
        s.tics(2)
        ev += [(t0, "+" + key), (t1 if held else t0 + 1, "-" + key)]
    s.up("POWER")
    s.tics(3)
    expect("POWER chords (strafe, next and previous weapon, held use)", exe, s, ev)

    # POWER alone 1 s opens the menu, release sends nothing
    s = Script()
    s.tics(1)
    t0 = s.t
    s.down("POWER")
    s.tics(50)
    s.up("POWER")
    s.tics(3)
    got, _ = s.run(exe)
    # The tic of the press counts as the first of the 35
    check(balanced(got) is None and got == tap_events(t0 + TICRATE - 1, "ESC"),
          "POWER 1 s: %s (pressed at %d)" % (got, t0))
    print("ok   input: POWER alone 1 s = menu in the 35th tic held, nothing on release")

    # POWER alone 4 s: menu at 1 s, then (menu open) close it and ask to quit, LIGHT says yes
    s = Script()
    t0 = s.t
    s.down("POWER")
    s.tics(TICRATE + 1)
    s.ctx = "m"
    s.tics(4 * TICRATE + 10 - s.t)
    s.ctx = "p"
    s.up("POWER")
    s.tics(3)
    t2 = s.t
    s.down("LIGHT")
    s.tics(3)
    s.up("LIGHT")
    s.tics(2)
    got, _ = s.run(exe)
    t1 = t0 + TICRATE - 1
    t4 = t0 + 4 * TICRATE - 1
    ok = got == (tap_events(t1, "ESC") + [(t4, "+ESC"), (t4, "+F10"), (t4 + 1, "-ESC"),
                                         (t4 + 1, "-F10")] + tap_events(t2, "y"))
    check(ok and balanced(got) is None, "POWER 4 s: %s (pressed at %d)" % (got, t0))
    if ok:
        print("ok   input: POWER alone 4 s = quit prompt in the 140th tic held, LIGHT confirms")

    # POWER while walking: not alone, so no menu at 1 s, the menu key on release
    s = Script()
    t0 = s.t
    s.down("VA")
    s.tics(3)
    s.down("POWER")
    s.tics(TICRATE * 3 // 2)
    t1 = s.t
    s.up("POWER")
    s.tics(3)
    t2 = s.t
    s.up("VA")
    s.tics(2)
    expect("POWER held 1.5 s while walking = menu on release", exe, s,
           [(t0, "+UP")] + tap_events(t1, "ESC") + [(t2, "-UP")])

    # A press keeps the action it started with
    s = Script()
    t0 = s.t
    s.down("MODE")
    s.tics(3)
    s.down("POWER")
    s.tics(3)
    t1 = s.t
    s.up("MODE")
    s.tics(3)
    t2 = s.t
    s.up("POWER")
    s.tics(3)
    s.down("POWER")
    s.tics(2)
    t4 = s.t
    s.down("SAVE")
    s.tics(2)
    s.up("POWER")
    s.tics(3)
    t5 = s.t
    s.up("SAVE")
    s.tics(2)
    expect("latched actions (turn stays turn, strafe stays strafe)", exe, s,
           [(t0, "+LEFT"), (t1, "-LEFT")] + tap_events(t2, "ESC") +
           [(t4, "+STRAFE_R"), (t5, "-STRAFE_R")])

    # Flip swaps HOLD and V-A in game and menu
    s = Script()
    s.cmd("cfg 0 1 1")
    s.cmd("weap %d -1 %d 0" % (WP["pistol"], 0b1111))  # fist pistol shotgun chaingun
    ev = []
    for btn, key in (("HOLD", "UP"), ("VA", "DOWN")):
        t0 = s.t
        s.down(btn)
        s.tics(3)
        ev.append((t0, "+" + key))
        ev.append((s.t, "-" + key))
        s.up(btn)
        s.tics(1)
    for btn, key in (("HOLD", "3"), ("VA", "1")):
        s.down("POWER")
        s.tics(1)
        t0 = s.t
        s.down(btn)
        s.tics(3)
        ev += tap_events(t0, key)
        s.up(btn, "POWER")
        s.tics(1)
    s.ctx = "m"
    for btn, key in (("HOLD", "UP"), ("VA", "DOWN")):
        t0 = s.t
        s.down(btn)
        s.tics(3)
        ev += tap_events(t0, key)
        s.up(btn)
        s.tics(2)
    expect("flip", exe, s, ev)

    # Menu mapping
    s = Script()
    s.ctx = "m"
    ev = []
    for btn, key in (("VA", "UP"), ("HOLD", "DOWN"), ("MODE", "LEFT"), ("SAVE", "RIGHT"),
                     ("LIGHT", "ENTER")):
        t0 = s.t
        s.down(btn)
        s.tics(10)
        s.up(btn)
        s.tics(1)
        ev += tap_events(t0, key)
    s.down("POWER")
    s.tics(4)
    t0 = s.t
    s.up("POWER")
    s.tics(2)
    ev += tap_events(t0, "ESC")
    s.down("POWER")
    t0 = s.t
    s.tics(TICRATE + 3)
    s.up("POWER")
    s.tics(2)
    ev += tap_events(t0 + TICRATE - 1, "ESC")
    expect("menu mapping (move, adjust, select, close by tap and by hold)", exe, s, ev)

    # Prompt: LIGHT yes, POWER no, the rest nothing
    s = Script()
    s.ctx = "p"
    for btn in ("MODE", "SAVE", "HOLD", "VA"):
        s.down(btn)
        s.tics(2)
        s.up(btn)
        s.tics(1)
    t0 = s.t
    s.down("LIGHT")
    s.tics(2)
    s.up("LIGHT")
    s.tics(1)
    s.down("POWER")
    s.tics(2)
    t1 = s.t
    s.up("POWER")
    s.tics(2)
    expect("prompt (LIGHT yes, POWER no)", exe, s, tap_events(t0, "y") + tap_events(t1, "n"))

    # Press and release inside one tic still give one tic of key down
    s = Script()
    s.tics(2)
    t0 = s.t
    s.tics(1, "+4 -4")
    s.tics(3)
    t1 = s.t
    s.tics(1, "+5 -5")
    s.tics(3)
    got, _ = s.run(exe)
    # POWER's release waits one tic, so the menu key follows it
    ok = got == tap_events(t0, "FIRE") + tap_events(t1 + 1, "ESC")
    check(ok and balanced(got) is None, "short taps: %s" % got)
    if ok:
        print("ok   input: press and release inside one tic (fire, menu)")

    # Edges lost to a full ring: the debounced state catches up in the same tic
    s = Script()
    s.tics(1)
    t0 = s.t
    s.down("MODE", "LIGHT")
    s.tics(1, "drop")
    s.tics(3)
    t1 = s.t
    s.up("MODE")
    s.tics(1, "drop")
    t2 = s.t
    s.up("LIGHT")
    s.tics(2)
    expect("full ring resync", exe, s,
           [(t0, "+LEFT"), (t0, "+FIRE"), (t1, "-LEFT"), (t2, "-FIRE")])

    # Always-run: key_speed past the key table, off restores the run key
    s = Script()
    s.cmd("speed")
    s.tics(1)
    s.cmd("speed")
    s.cmd("cfg 0 0 0")
    s.cmd("cfg 0 0 1")
    _, speed = s.run(exe)
    check(speed[1:] == [256, KEY_RSHIFT, 256], "always-run key_speed: %s" % speed)
    print("ok   input: always-run on by default (key_speed %d), off = %d" % (speed[1], speed[2]))

    # Preset 1 and a custom map
    s = Script()
    s.cmd("cfg 1 0 1")
    ev = []
    for btn, alone, shifted in (("MODE", "STRAFE_L", "LEFT"), ("SAVE", "STRAFE_R", "RIGHT")):
        t0 = s.t
        s.down(btn)
        s.tics(2)
        s.up(btn)
        s.tics(1)
        s.down("POWER")
        s.tics(1)
        t1 = s.t
        s.down(btn)
        s.tics(2)
        s.up(btn, "POWER")
        s.tics(2)
        ev += [(t0, "+" + alone), (t0 + 2, "-" + alone), (t1, "+" + shifted),
               (t1 + 2, "-" + shifted)]
    expect("preset 1 (MODE and SAVE strafe, turn with POWER)", exe, s, ev)

    custom = [ACT["USE"], ACT["NONE"], ACT["FWD"], ACT["FWD"], ACT["FIRE"]] + [ACT["NONE"]] * 5
    s = Script()
    s.cmd("map 0 1 " + " ".join(map(str, custom)))
    t0 = s.t
    s.down("HOLD")
    s.tics(4)
    s.up("HOLD")
    s.tics(1)
    s.down("SAVE")
    s.tics(2)
    s.up("SAVE")
    s.tics(1)
    t1 = s.t
    s.down("MODE")
    s.tics(2)
    s.down("VA")
    s.tics(2)
    s.up("MODE")
    s.tics(2)
    t2 = s.t
    s.up("VA")
    s.tics(2)
    expect("custom map (held use, unbound button, two buttons on one key)", exe, s,
           [(t0, "+USE"), (t0 + 4, "-USE"), (t1, "+UP"), (t1 + 2, "+UP"), (t2, "-UP")])

    # Calls that build no ticcmd leave the edges in the ring: a release in such a call, or a
    # whole tap, still reaches the next built ticcmds
    s = Script()
    s.tics(2)
    s.down("POWER")
    s.tics(3)
    s.up("POWER")
    s.tics(3, built=False)
    t1 = s.t
    s.tics(1)
    s.tics(2, built=False)
    t2 = s.t
    s.tics(2)
    s.down("LIGHT")
    s.tics(1, built=False)
    s.up("LIGHT")
    s.tics(2, built=False)
    t3 = s.t
    s.tics(1)
    s.tics(1, built=False)
    t4 = s.t
    s.tics(1)
    s.tics(1, "+5 -5", built=False)
    t5 = s.t
    s.tics(2)
    s.tics(1, built=False)
    t6 = s.t
    s.tics(3)
    expect("unbuilt tics (POWER release, fire tap, POWER tap)", exe, s,
           [(t1, "+ESC"), (t2, "-ESC"), (t3, "+FIRE"), (t4, "-FIRE"), (t5 + 1, "+ESC"),
            (t6, "-ESC")])

    # The hold timer counts built tics only
    s = Script()
    s.down("POWER")
    built = []
    while len(built) < TICRATE + 2:
        built.append(s.t)
        s.tics(1)
        s.tics(2, built=False)
    s.up("POWER")
    s.tics(2)
    expect("POWER 1 s over unbuilt tics = menu in the 35th built tic", exe, s,
           [(built[TICRATE - 1], "+ESC"), (built[TICRATE], "-ESC")])

    # POWER pressed and released in one tic, then another button in the same tic: no chord
    s = Script()
    s.tics(2)
    t0 = s.t
    s.down("VA")
    s.tics(1, "+5 -5 +3")
    s.tics(3)
    t1 = s.t
    s.up("VA")
    s.tics(2)
    expect("POWER tap then V-A in one tic = menu and forward", exe, s,
           [(t0, "+UP")] + tap_events(t0 + 1, "ESC") + [(t1, "-UP")])

    # POWER pressed while another button is down never counts as alone: no menu at 1 s, no
    # quit, the menu key on release
    s = Script()
    t0 = s.t
    s.down("VA")
    s.tics(3)
    s.down("POWER")
    s.tics(2)
    t1 = s.t
    s.up("VA")
    s.tics(5 * TICRATE)
    t2 = s.t
    s.up("POWER")
    s.tics(2)
    expect("POWER pressed during V-A, held 5 s after V-A = menu on release", exe, s,
           [(t0, "+UP"), (t1, "-UP")] + tap_events(t2, "ESC"))

    # A tap of a key that a held button also holds (turn left on HOLD in a custom map, held
    # in game, then MODE's left in the menu): the key goes up once, after the tap's tic, in
    # either edge order
    custom = [ACT["TURNL"]] + [ACT["NONE"]] * 9
    for order in ("+2 -0", "-0 +2"):
        s = Script()
        s.cmd("map 0 1 " + " ".join(map(str, custom)))
        t0 = s.t
        s.down("HOLD")
        s.tics(2)
        s.ctx = "m"
        s.tics(2)
        t1 = s.t
        s.up("HOLD")
        s.down("MODE")
        s.tics(1, order)
        s.tics(2)
        s.up("MODE")
        s.tics(2)
        want = [(t0, "+LEFT")] + ([] if order[0] == "+" else [(t1, "-LEFT")])
        expect("menu LEFT tap while a held LEFT is released (%s)" % order, exe, s,
               want + tap_events(t1, "LEFT"))

    # Context changes while a button is held: a key held in game goes up in the menu, a
    # menu tap gives nothing in game when its button is released
    s = Script()
    t0 = s.t
    s.down("VA")
    s.tics(3)
    s.ctx = "m"
    s.tics(3)
    t1 = s.t
    s.up("VA")
    s.tics(2)
    t2 = s.t
    s.down("HOLD")
    s.tics(2)
    s.ctx = "g"
    s.tics(3)
    s.up("HOLD")
    s.tics(2)
    t3 = s.t
    s.down("HOLD")
    s.tics(2)
    t4 = s.t
    s.up("HOLD")
    s.tics(2)
    expect("context change while held (game to menu, menu to game)", exe, s,
           [(t0, "+UP"), (t1, "-UP")] + tap_events(t2, "DOWN") + [(t3, "+DOWN"), (t4, "-DOWN")])

    # Next and previous weapon against Chocolate's G_NextWeapon
    rng = random.Random(7)
    weapons = ["fist", "pistol", "shotgun", "chaingun", "missile", "plasma", "bfg", "chainsaw"]
    s = Script()
    cases = []
    for _ in range(400):
        owned = {w for w in weapons if rng.random() < 0.5} | {"fist", "pistol"}
        ready = rng.choice(sorted(owned))
        pending = rng.choice([None, None, rng.choice(sorted(owned))])
        strength = rng.random() < 0.3
        mask = sum(1 << WP[w] for w in owned)
        s.cmd("weap %d %d %d %d" % (WP[ready], -1 if pending is None else WP[pending], mask,
                                    strength))
        for direction, btn in ((1, "VA"), (-1, "HOLD")):
            s.down("POWER")
            s.tics(1)
            s.down(btn)
            s.tics(1)
            s.up(btn, "POWER")
            s.tics(2)
            cases.append(next_weapon_ref(ready, pending, owned, strength, direction))
    got, _ = s.run(exe)
    keys = [e[1:] for _, e in got if e[0] == "+"]
    check(keys == cases and balanced(got) is None,
          "weapon cycling differs from G_NextWeapon in %d of %d cases"
          % (sum(a != b for a, b in zip(keys, cases)) + abs(len(keys) - len(cases)), len(cases)))
    print("ok   input: next/previous weapon equal to G_NextWeapon in %d random states" % len(cases))

    # The game pad's ENTER (port/game/pad.h) in a level, in a demo, on the title, in the menu
    # and in a prompt. Buttons held when the pad comes back are no new presses.
    present, menu, enter = 0x100, 0x04, 0x80
    s = Script()
    s.cmd("pad %d" % present)
    s.tics(1)
    want = []
    for ctx, state, demo, key in (("g", 0, 0, None), ("g", 0, 1, "ENTER"), ("g", 3, 0, "ENTER"),
                                  ("m", 0, 0, "ENTER"), ("p", 0, 0, "y")):
        s.ctx = ctx
        s.cmd("state %d %d" % (state, demo))
        if key:
            want += tap_events(s.t, key)
        s.cmd("pad %d" % (present | enter))
        s.tics(1)
        s.cmd("pad %d" % present)
        s.tics(2)
    s.ctx = "g"
    s.cmd("state 0 0")
    want += tap_events(s.t, "ESC")
    s.cmd("pad %d" % (present | menu))
    s.tics(2)
    s.cmd("pad 0")
    s.tics(2)
    s.cmd("pad %d" % (present | menu))
    s.tics(2)
    s.cmd("pad %d" % present)
    s.tics(1)
    want += tap_events(s.t, "ESC")
    s.cmd("pad %d" % (present | menu))
    s.tics(2)
    expect("pad ENTER not in a level, MENU held through a dropout", exe, s, want)


# ---------------------------------------------------------------- sound


def find_zip(arg):
    for p in (arg, os.environ.get("FREEDOOM_ZIP"),
              os.path.join(REPO, "dl", "freedoom-0.13.0.zip")):
        if p and os.path.exists(p):
            return p
    sys.exit("freedoom-0.13.0.zip not found: set FREEDOOM_ZIP")


def wad_dp_lumps(zip_path):
    """DP lumps of freedoom1.wad: name -> tone bytes (length from the lump header)."""
    with zipfile.ZipFile(zip_path) as z:
        wad = z.read(WAD_MEMBER)
    kind, n, dirofs = struct.unpack_from("<4sii", wad, 0)
    if kind != b"IWAD":
        sys.exit("%s is not an IWAD" % WAD_MEMBER)
    out = {}
    for i in range(n):
        pos, size, name = struct.unpack_from("<ii8s", wad, dirofs + 16 * i)
        name = name.rstrip(b"\0").decode("ascii").lower()
        if not name.startswith("dp"):
            continue
        b = wad[pos:pos + size]
        zero, count = struct.unpack_from("<HH", b, 0)
        if zero == 0 and 4 + count <= size:
            out[name[2:]] = b[4:4 + count]
    return out


def expected_steps(tones):
    """Reference frequency per 140 Hz step, 0 = silent."""
    return [PIT_HZ / DIVISORS[t] if t < len(DIVISORS) and DIVISORS[t] else 0 for t in tones]


class Sound:
    def __init__(self, exe, image):
        self.exe, self.image = exe, image
        self.lines = []

    def cmd(self, line):
        self.lines.append(line)

    def run(self):
        p = subprocess.run([self.exe, self.image], input="\n".join(self.lines) + "\n",
                           capture_output=True, text=True)
        if p.returncode:
            sys.exit("%s failed: %s" % (self.exe, p.stderr))
        self.lines = []
        segs, cur, info = {}, None, {}
        for line in p.stdout.splitlines():
            f = line.split()
            if f[0] == "mark":
                cur = segs.setdefault(f[1], {"steps": [], "out": []})
            elif f[0] == "step":
                cur["steps"].append((int(f[1]), int(f[2])))
            elif f[0] in ("handle", "playing", "busy"):
                cur["out"].append(int(f[-1]))
            elif f[0] == "channels":
                info["channels"] = int(f[1])
            elif f[0] == "sounds":
                info["sounds"] = (int(f[1]), int(f[2]))
            elif f[0] == "sfx":
                info.setdefault("sfx", []).append((int(f[1]), f[2], int(f[3]), f[4] == "1"))
        return segs, info


def freq(period):
    return TMR_HZ / period if period else 0


def match(steps, tones):
    """Steps play tones exactly, then go silent. Returns (ok, max relative error)."""
    ref = expected_steps(tones)
    if len(steps) <= len(ref) or steps[len(ref)][0] != 0:
        return False, 0
    err = 0.0
    for (period, _), f in zip(steps, ref):
        if (period == 0) != (f == 0):
            return False, 0
        if f:
            err = max(err, abs(freq(period) - f) / f)
    return err < 1e-3, err


def is_prefix_then(steps, a, b):
    """steps = a non-empty prefix of a's tones, then all of b."""
    ra = expected_steps(a)
    for k in range(1, len(ra) + 1):
        ok, _ = match(steps[k:], b)
        same = all((p == 0) == (f == 0) and (not f or abs(freq(p) - f) / f < 1e-3)
                   for (p, _), f in zip(steps[:k], ra[:k]))
        if ok and same:
            return k
    return 0


def render_wav(path, steps, rate=44100):
    """Square wave of a trace: one 1/140 s step per entry, duty from the timer registers."""
    data = bytearray()
    phase = 0.0
    n_total = 0
    for i, (period, duty) in enumerate(steps):
        n = round((i + 1) * rate / 140) - n_total
        n_total += n
        f = freq(period)
        hi = duty / period if period else 0
        for _ in range(n):
            if f:
                phase = (phase + f / rate) % 1.0
                v = 8000 if phase < hi else -8000
            else:
                v = 0
            data += struct.pack("<h", v)
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(rate)
        w.writeframes(bytes(data))


def test_sound(exe, image, zip_path, wav_dir, wav_all):
    dp = wad_dp_lumps(zip_path)
    snd = Sound(exe, image)
    snd.cmd("names")
    _, info = snd.run()
    sfx = info["sfx"]
    channels = info["channels"]
    prio = {name: p for _, name, p, _ in sfx}
    num = {name: n for n, name, _, _ in sfx}
    playable = [name for n, name, _, link in sfx if not link and name in dp]

    # Every sound through S_StartSound, one per frame
    for name in playable:
        snd.cmd("frame")
        snd.cmd("mark %s" % name)
        snd.cmd("start 0 %d" % num[name])
        snd.cmd("frame")
        snd.cmd("run")
        snd.cmd("iplaying 0")
    segs, _ = snd.run()
    worst, nsteps, nchecked = 0.0, 0, 0
    for name in playable:
        steps = segs[name]["steps"]
        if name in DISABLED:
            check(all(p == 0 for p, _ in steps), "disabled sound %s plays" % name)
            continue
        ok, err = match(steps, dp[name])
        check(ok, "sound %s: steps differ from DP%s" % (name, name.upper()))
        check(segs[name]["out"] == [0], "sound %s still playing after it ended" % name)
        worst = max(worst, err)
        nsteps += len(dp[name])
        nchecked += 1
    print("ok   sound: %d DP sounds, %d steps equal to the WAD decode, max frequency error %.4f %%"
          ", %d disabled sounds silent" % (nchecked, nsteps, worst * 100,
                                          len([n for n in playable if n in DISABLED])))
    check(len(playable) - len(DISABLED & set(playable)) == nchecked, "sound count")

    # Choose sounds by priority (vanilla: lower number = more important)
    usable = [n for n in playable if n not in DISABLED and len(dp[n]) >= 10]
    base = sorted(n for n in usable if prio[n] == 64)
    low = max(usable, key=lambda n: (prio[n], n))
    high = min(usable, key=lambda n: (prio[n], n))
    a, b = base[0], low
    check(prio[low] > 64 > prio[high] and len(base) >= channels + 1, "priority choice")

    # A free channel: a less important sound still replaces the current one
    snd.cmd("mark replace")
    snd.cmd("start 1 %d" % num[a])
    snd.cmd("frame")
    snd.cmd("ms 50")
    snd.cmd("start 2 %d" % num[b])
    snd.cmd("frame")
    snd.cmd("run")
    # Same origin: the new sound replaces the old one
    snd.cmd("frame")
    snd.cmd("mark origin")
    snd.cmd("start 3 %d" % num[b])
    snd.cmd("frame")
    snd.cmd("ms 30")
    snd.cmd("start 3 %d" % num[a])
    snd.cmd("frame")
    snd.cmd("run")
    # All channels busy within one tic: the less important sound is refused, the more
    # important one kicks a channel out
    for tag, extra in (("full_low", low), ("full_high", high)):
        snd.cmd("frame")
        snd.cmd("mark %s" % tag)
        for i in range(channels):
            snd.cmd("start %d %d" % (i + 1, num[base[i]]))
        snd.cmd("start %d %d" % (channels + 1, num[extra]))
        snd.cmd("frame")
        snd.cmd("run")
    # Stop by origin
    snd.cmd("frame")
    snd.cmd("mark stop")
    snd.cmd("start 4 %d" % num[a])
    snd.cmd("frame")
    snd.cmd("ms 30")
    snd.cmd("stop 4")
    snd.cmd("mark stopped")
    snd.cmd("ms 30")
    # The I_* interface: handles are channels, only the current handle plays or stops
    snd.cmd("frame")
    snd.cmd("mark direct")
    snd.cmd("istart %d 3" % num[a])
    snd.cmd("iplaying 3")
    snd.cmd("frame")
    snd.cmd("ms 20")
    snd.cmd("iplaying 3")
    snd.cmd("iplaying 0")
    snd.cmd("istop 0")
    snd.cmd("iplaying 3")
    snd.cmd("istop 3")
    snd.cmd("iplaying 3")
    snd.cmd("mark direct_end")
    snd.cmd("istart %d 1" % num["sawidl"])
    snd.cmd("ms 20")
    # Volume: low 1/8 duty, high 1/2, off silent while the sound still runs
    for v in (1, 2, 0):
        snd.cmd("frame")
        snd.cmd("vol %d" % v)
        snd.cmd("mark vol%d" % v)
        snd.cmd("start 0 %d" % num[a])
        snd.cmd("frame")
        snd.cmd("ms 30")
        snd.cmd("iplaying 0")
        snd.cmd("run")
    snd.cmd("vol 2")
    segs, _ = snd.run()

    k = is_prefix_then(segs["replace"]["steps"], dp[a], dp[b])
    check(k > 0, "replace: %s then %s not found" % (a, b))
    print("ok   sound: %s (priority %d) replaced %s (priority %d) after %d steps"
          % (b, prio[b], a, prio[a], k))
    k = is_prefix_then(segs["origin"]["steps"], dp[b], dp[a])
    check(k > 0, "same origin: %s then %s not found" % (b, a))
    print("ok   sound: same origin replaces after %d steps" % k)
    ok, _ = match(segs["full_low"]["steps"], dp[base[channels - 1]])
    check(ok, "full channels: %s (priority %d) should be refused" % (low, prio[low]))
    ok2, _ = match(segs["full_high"]["steps"], dp[high])
    check(ok2, "full channels: %s (priority %d) should play" % (high, prio[high]))
    if ok and ok2:
        print("ok   sound: %d busy channels refuse %s (%d), %s (%d) kicks one out"
              % (channels, low, prio[low], high, prio[high]))
    st = segs["stop"]["steps"]
    check(any(p for p, _ in st) and all(p == 0 for p, _ in segs["stopped"]["steps"]),
          "stop by origin")
    out = segs["direct"]["out"]
    if check(out == [3, 1, 1, 0, 1, 0] and segs["direct_end"]["out"] == [-1] and
             all(p == 0 for p, _ in segs["direct_end"]["steps"]),
             "I_* interface: %s %s" % (out, segs["direct_end"]["out"])):
        print("ok   sound: S_StopSound and the I_* handle rules")
    for v, div in ((1, 8), (2, 2)):
        steps = [(p, d) for p, d in segs["vol%d" % v]["steps"] if p]
        check(steps and all(d == p // div for p, d in steps), "volume %d duty" % v)
    check(all(p == 0 for p, _ in segs["vol0"]["steps"]) and segs["vol0"]["out"] == [1],
          "volume off")
    print("ok   sound: volume low 1/8 duty, high 1/2 duty, off silent")

    # Handle -1 (a refused start) never matches, even before the first real start: a
    # sound of another module keeps playing
    snd.cmd("mark minus1")
    snd.cmd("raw 20")
    snd.cmd("iplaying -1")
    snd.cmd("istop -1")
    snd.cmd("busy")
    snd.cmd("istart %d 1" % num["sawidl"])
    snd.cmd("iplaying -1")
    snd.cmd("istop -1")
    snd.cmd("busy")
    snd.cmd("ms 50")
    segs, _ = snd.run()
    st = segs["minus1"]["steps"]
    if check(segs["minus1"]["out"] == [0, 1, -1, 0, 1] and len(st) >= 6 and
             all(p for p, _ in st[:6]), "handle -1: %s %s" % (segs["minus1"]["out"], st[:8])):
        print("ok   sound: handle -1 neither plays nor stops")

    # SOUNDS section checks: absent (size 0, nothing is read), a table longer than the
    # section, an entry past the end of the section. Reads outside the section given to
    # S_PcsInit fail in the harness.
    off, size = info["sounds"]
    with open(image, "rb") as f:
        f.seek(off)
        sect = f.read(size)
    count = struct.unpack_from("<H", sect, 0)[0]
    ent = [struct.unpack_from("<IH", sect, 8 + 8 * i) for i in range(count)]
    audible = [n for n in playable if n not in DISABLED and dp[n]]
    last = max(audible, key=lambda n: sum(ent[num[n]]))
    cut = sum(ent[num[last]]) - 1
    keep = [n for n in audible if sum(ent[num[n]]) <= cut][0]
    # The short section ends inside the table entry of keep
    for tag, sz in (("absent", 0), ("short", 8 + 8 * num[keep] + 4), ("cut", cut)):
        snd.cmd("frame")
        snd.cmd("init %d %d" % (0 if tag == "absent" else off, sz))
        for name in (last, keep):
            snd.cmd("mark %s_%s" % (tag, name))
            snd.cmd("start 0 %d" % num[name])
            snd.cmd("frame")
            snd.cmd("run")
    segs, _ = snd.run()
    ok = all(all(p == 0 for p, _ in segs["%s_%s" % (t, n)]["steps"])
             for t in ("absent", "short", "cut") for n in (last, keep) if (t, n) != ("cut", keep))
    ok2, _ = match(segs["cut_%s" % keep]["steps"], dp[keep])
    if check(ok and ok2, "SOUNDS section checks"):
        print("ok   sound: absent section and short table silent, %s past the cut silent, "
              "%s plays" % (last, keep))

    # WAVs of the device traces
    if wav_dir:
        os.makedirs(wav_dir, exist_ok=True)
        audible = [n for n in playable if n not in DISABLED]
        pick = audible if wav_all else [n for n in ("pistol", "shotgn", "itemup", "pldeth", "bfg")
                                        if n in audible]
        for name in pick:
            snd.cmd("frame")
            snd.cmd("mark %s" % name)
            snd.cmd("start 0 %d" % num[name])
            snd.cmd("frame")
            snd.cmd("run")
        segs, _ = snd.run()
        for name in pick:
            path = os.path.join(wav_dir, "dp%s.wav" % name)
            render_wav(path, segs[name]["steps"])
            with wave.open(path, "rb") as w:
                check(w.getnframes() >= len(dp[name]) * 44100 // 140, "WAV %s too short" % path)
        print("ok   sound: %d WAVs in %s" % (len(pick), wav_dir))


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--build", default=os.path.join(REPO, "build", "game"))
    ap.add_argument("--pack", default=os.path.join(REPO, "build", "packs", "E1M1"))
    ap.add_argument("--zip", help="freedoom-0.13.0.zip (default $FREEDOOM_ZIP or dl/)")
    ap.add_argument("--wav-dir", help="write square-wave WAVs of the traces here")
    ap.add_argument("--wav-all", action="store_true", help="a WAV for every sound")
    ap.add_argument("--sound-only", action="store_true")
    args = ap.parse_args()
    if not args.sound_only:
        test_input(os.path.join(args.build, "input_test"))
    test_sound(os.path.join(args.build, "pcspk_test"), os.path.join(args.pack, "DOOM.DAT"),
               find_zip(args.zip), args.wav_dir, args.wav_all)
    if failures:
        sys.exit("%d failures" % len(failures))
    print("game tests passed")


if __name__ == "__main__":
    main()
