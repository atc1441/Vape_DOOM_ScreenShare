#!/usr/bin/env python3
"""
touch.py - inject input and prove what it does in the game.

    python touch.py state                 game state, player pose
    python touch.py probe                 which glass area sets which button
    python touch.py axis                  raw values -> screen coordinates
    python touch.py tap X Y [ms]          touch at a screen point
    python touch.py bits NAME.. [ms]      button bits directly
    python touch.py motion                measure walk/back/turn objectively
    python touch.py enter                 from the title screen into a game
    python touch.py walk [ms]             walk forward and measure the way
    python touch.py goto X Y              drive to a point on the map
    python touch.py shots [dir]           screenshots from the game
    python touch.py off                   stop injecting

A finger cannot be put in the same place twice, and the device has neither a
keyboard nor a serial port. The Cortex-M4 does let the debugger write RAM
while the CPU runs, though, so this tool occupies the block `tinj` in
platform/py32/buttons.c; the firmware then takes its values instead of the
touch controller's.

The proof is not in the picture but in the engine's state: `players[0]` is
in RAM, so the player's position and facing before and after an input are
readable. "Forward" is then not an impression from a camera frame but a
distance in map units.

The three modes separate what would otherwise collapse into one test:

  1  screen coordinates - checks the zone layout, the chord resolver and the
     game logic. Does not touch the panel's axis question.
  2  controller raw values - checks the mapping map_raw as well.
  3  button bits directly - for menu paths.

What this tool *cannot* answer: whether a finger at a given spot on the
glass produces the raw value the controller ought to report there. That is
the one point where injection runs in a circle, and it is settled elsewhere
(tools/stocktouch.py reads the stock firmware's hit areas).
"""

import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)

import vapedoom  # noqa: E402

# Paths and probe serial live in vapedoom.py and are set there through
# environment variables.
ELF = vapedoom.ELF_LINUX
NM = vapedoom.NM_WSL

TINJ_MAGIC = 0x544F5543

# struct touch_inject, platform/py32/buttons.c
T_MAGIC, T_MODE, T_X, T_Y, T_BITS, T_DOWN, T_SEQ, T_OUT = (
    0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C)

BTN_HOLD, BTN_SAVE, BTN_MODE, BTN_VA, BTN_LIGHT, BTN_POWER = range(6)
BTN_NAME = {"HOLD": 0, "SAVE": 1, "MODE": 2, "VA": 3, "LIGHT": 4, "POWER": 5,
            "BACK": 0, "RIGHT": 1, "LEFT": 2, "FWD": 3, "FIRE": 4,
            "SHIFT": 5, "MENU": 5}
BTN_SHOW = ["HOLD/back", "SAVE/right", "MODE/left", "VA/fwd", "LIGHT/fire",
            "POWER/shift"]

# engine/d_player.h: mobj_t *mo comes first in player_t.
# engine/p_mobj.h: x, y, z, momx, momy, momz, angle - alles 32 Bit.
MO_X, MO_Y, MO_Z, MO_ANGLE = 0x00, 0x04, 0x08, 0x18
MO_MOMX, MO_MOMY, MO_MOMZ = 0x0C, 0x10, 0x14

GS = {0: "GS_LEVEL", 1: "GS_INTERMISSION", 2: "GS_FINALE", 3: "GS_DEMOSCREEN"}

FRACBITS = 16


def symbols():
    """Addresses from the ELF. llvm-nm runs in WSL, where the tree is."""
    env = dict(os.environ, MSYS2_ARG_CONV_EXCL="*")
    out = subprocess.run(["wsl.exe", "-d", vapedoom.WSL_DISTRO, "--", NM, ELF],
                         capture_output=True, text=True, env=env,
                         cwd=vapedoom.TREE).stdout
    syms = {}
    for line in out.splitlines():
        p = line.split()
        if len(p) == 3:
            syms[p[2]] = int(p[0], 16)
    for need in ("tinj", "dbg", "players", "gamestate", "gamemap", "leveltime",
                 "menuactive", "demoplayback", "usergame"):
        if need not in syms:
            raise SystemExit("symbol %s missing - is %s freshly built?" % (need, ELF))
    return syms


class Pad(object):
    def __init__(self, v, syms):
        self.v = v
        self.s = syms

    # -- RAM --
    def r32(self, a, n=1):
        x = self.v.jl.memory_read32(a, n)
        return x[0] if n == 1 else list(x)

    def w32(self, a, vals):
        self.v.jl.memory_write32(a, vals if isinstance(vals, list) else [vals])

    # -- injection --
    def arm(self, mode):
        self.w32(self.s["tinj"] + T_MODE, mode)
        self.w32(self.s["tinj"] + T_MAGIC, TINJ_MAGIC)

    def off(self):
        self.w32(self.s["tinj"] + T_MAGIC, 0)
        self.w32(self.s["tinj"] + T_DOWN, 0)
        self.w32(self.s["tinj"] + T_MODE, 0)

    def set(self, mode, x=0, y=0, bits=0, down=1):
        t = self.s["tinj"]
        # Order: values first, then down. The firmware reads the block in
        # the SysTick interrupt, so it can read right into the middle of an
        # update; half a point with down already set would be a touch in the
        # wrong place.
        self.w32(t + T_MODE, [mode, x, y, bits])
        self.w32(t + T_DOWN, 1 if down else 0)
        self.w32(t + T_MAGIC, TINJ_MAGIC)

    def release(self):
        self.w32(self.s["tinj"] + T_DOWN, 0)

    def wait_seq(self, n=2, timeout=0.5):
        """Wait until the firmware has really read the injection.
        touch_poll only runs every 16th millisecond."""
        t = self.s["tinj"] + T_SEQ
        s0 = self.r32(t)
        t0 = time.time()
        while time.time() - t0 < timeout:
            if (self.r32(t) - s0) & 0xFFFFFFFF >= n:
                return True
            time.sleep(0.004)
        return False

    def out_bits(self):
        return self.r32(self.s["tinj"] + T_OUT)

    def press(self, mode, x=0, y=0, bits=0, ms=250):
        self.set(mode, x, y, bits, down=1)
        self.wait_seq(2)
        got = self.out_bits()
        time.sleep(max(0.0, ms / 1000.0 - 0.05))
        self.release()
        self.wait_seq(2)
        return got

    # -- engine --
    def player(self):
        mo = self.r32(self.s["players"])
        if not (0x20000000 <= mo < 0x20010000):
            return None
        x, y, z = self.r32(mo + MO_X, 3)
        ang = self.r32(mo + MO_ANGLE)
        return dict(mo=mo, x=sfix(x), y=sfix(y), z=sfix(z), ang=ang,
                    deg=ang * 360.0 / 2 ** 32)

    def flag(self, name):
        """boolean is one byte here; reading 32 bits would catch the neighbour."""
        return self.v.jl.memory_read8(self.s[name], 1)[0] != 0

    def menu(self):
        return self.flag("menuactive")

    def where(self):
        """Where the program stands. Without this every measurement runs
        into the void: during demo playback the player moves by himself, and
        an input has no effect there at all."""
        return dict(demo=self.flag("demoplayback"), menu=self.flag("menuactive"),
                    ingame=self.flag("usergame"),
                    gamestate=self.r32(self.s["gamestate"]),
                    gamemap=self.r32(self.s["gamemap"]),
                    leveltime=self.r32(self.s["leveltime"]))

    def playing(self):
        w = self.where()
        return (w["ingame"] and not w["demo"] and not w["menu"]
                and w["gamestate"] == 0 and self.player() is not None)

    def stop_player(self):
        """Take away leftover momentum. momx/momy are not link fields, so
        the debugger may touch them; otherwise every measurement drags the
        previous one's coasting along."""
        mo = self.r32(self.s["players"])
        if 0x20000000 <= mo < 0x20010000:
            self.w32(mo + MO_MOMX, [0, 0, 0])

    def turn_player(self, deg):
        mo = self.r32(self.s["players"])
        if 0x20000000 <= mo < 0x20010000:
            a = (self.r32(mo + MO_ANGLE) + int(deg / 360.0 * 2 ** 32)) & 0xFFFFFFFF
            self.w32(mo + MO_ANGLE, a)

    def game(self):
        return dict(gamestate=self.r32(self.s["gamestate"]),
                    gamemap=self.r32(self.s["gamemap"]),
                    episode=self.r32(self.s.get("gameepisode", self.s["gamemap"])),
                    leveltime=self.r32(self.s["leveltime"]))


def sfix(v):
    """fixed_t is signed; memory_read32 hands them over unsigned."""
    if v >= 1 << 31:
        v -= 1 << 32
    return v / float(1 << FRACBITS)


def names(bits):
    return " ".join(n for i, n in enumerate(BTN_SHOW) if bits & (1 << i)) or "-"


def ang_delta(a, b):
    """Shortest rotation from a to b, in degrees, signed."""
    d = ((b - a) & 0xFFFFFFFF) * 360.0 / 2 ** 32
    return d - 360.0 if d > 180.0 else d


# ---------------------------------------------------------------- Befehle

def cmd_state(p):
    g = p.game()
    print("gamestate %d (%s)  Episode %d Karte %d  leveltime %d tics (%.1f s)" %
          (g["gamestate"], GS.get(g["gamestate"], "?"), g["episode"],
           g["gamemap"], g["leveltime"], g["leveltime"] / 35.0))
    pl = p.player()
    if pl is None:
        print("player: no object (no level loaded)")
    else:
        print("player at x %.1f  y %.1f  z %.1f  facing %.1f deg" %
              (pl["x"], pl["y"], pl["z"], pl["deg"]))
    w = p.where()
    print("demo %d  menu %d  own game %d" % (w["demo"], w["menu"], w["ingame"]))
    s = p.v.state()
    print("stage %d (%s)  %.1f s  %d frames  %.2f fps  buttons %s" %
          (s["stage"], s["stage_name"], s["ms"] / 1000.0, s["frames"], s["fps"],
           names(s["buttons"])))


# The areas from buttons.c, one point in the middle of each.
PAD_RIGHT, PAD_CX, PAD_CY, PAD_DX, PAD_DY = 250, 125, 120, 45, 40
# The points come from the constants in platform/py32/buttons.c: steering
# cross left of 250 around (125,120) with a dead zone of +-45/+-40, action
# strip to the right of it split at 120, menu corner 44x36. The last two
# rows check that the corner stays small.
REGIONS = [
    ("top left", 40, 48, "MODE/left VA/fwd"),
    ("top", 125, 40, "VA/fwd"),
    ("top right", 210, 40, "SAVE/right VA/fwd"),
    ("left", 40, 120, "MODE/left"),
    ("centre (dead zone)", 125, 120, "-"),
    ("right", 210, 120, "SAVE/right"),
    ("bottom left", 40, 200, "MODE/left HOLD/back"),
    ("bottom", 125, 200, "HOLD/back"),
    ("bottom right", 210, 200, "SAVE/right HOLD/back"),
    ("strip top (USE)", 290, 60, "LIGHT/fire POWER/shift"),
    ("strip bottom (FIRE)", 290, 180, "LIGHT/fire"),
    ("menu corner", 20, 16, "POWER/shift"),
    ("just right of it", 60, 16, "MODE/left VA/fwd"),
    ("just below it", 20, 50, "MODE/left VA/fwd"),
]




def cmd_probe(p):
    """Touch each area once and read back from the firmware which button
    bits came of it. That checks touch_bits on the device instead of in your
    head."""
    print("area                    point      expected                      measured")
    bad = 0
    for nm, x, y, want in REGIONS:
        got = p.press(1, x, y, ms=120)
        have = names(got)
        ok = sorted(have.split()) == sorted(want.split())
        if not ok:
            bad += 1
        print("%-22s (%3d,%3d)  %-28s  %-28s %s" %
              (nm, x, y, want, have, "ok" if ok else "MISMATCH"))
    print("\n%d of %d areas as intended" % (len(REGIONS) - bad, len(REGIONS)))
    return 1 if bad else 0


def cmd_axis(p):
    """Send controller raw values through map_raw and read off where they
    land. That documents the mapping; which end of the glass the controller
    counts as ry=0 is not in here."""
    corners = [("ry 0   (one end of the long axis)", 86, 0),
               ("ry max (the other end)", 86, 322),
               ("rx 0   (one edge of the short axis)", 0, 161),
               ("rx max (the other edge)", 172, 161),
               ("centre", 86, 161)]
    print("raw (rx,ry)          ->  screen (sx,sy)       button bits")
    for nm, rx, ry in corners:
        got = p.press(2, rx, ry, ms=80)
        raw = p.r32(p.s["dbg"] + vapedoom.O_TOUCH_RAW)
        sx = p.r32(p.s["dbg"] + vapedoom.O_TOUCH_SX)
        sy = p.r32(p.s["dbg"] + vapedoom.O_TOUCH_SY)
        print("%-34s (%3d,%3d) -> (%3d,%3d)   %s" %
              (nm, raw >> 16, raw & 0xFFFF, sx, sy, names(got)))
    return 0


# Name, mode, x, y, bits, expectation per axis.
# "+"/"-" means: must move clearly in that direction, "0": must not move
# appreciably, None: anything goes.
#
# That the sideways axis is not asked about when turning is not slackness:
# walking forward *and* turning describes a curve, and reckoned against the
# previous facing a curve has a large sideways component. Checking that
# against zero would report correct behaviour as a fault - the first run did
# exactly that.
MOVES = [
    ("forward",           1, 125,  40, 0, "+",  None, "0"),
    ("back",              1, 125, 200, 0, "-",  None, "0"),
    ("turn left",         1,  40, 120, 0, None, None, "+"),
    ("turn right",        1, 210, 120, 0, None, None, "-"),
    ("forward + left",    1,  40,  40, 0, "+",  None, "+"),
    ("forward + right",   1, 210,  40, 0, "+",  None, "-"),
    # Strafing is not checked against "forward equals zero" but against
    # "sideways dominates". Sliding along an angled wall drags you along in
    # the facing direction too - that is Doom's collision handling, not a
    # fault of the input.
    ("strafe left",       3, 0, 0, (1 << BTN_MODE) | (1 << BTN_POWER), None, "+", "0"),
    ("strafe right",      3, 0, 0, (1 << BTN_SAVE) | (1 << BTN_POWER), None, "-", "0"),
    # Use cannot be read off the player's pose. What is checked is what went
    # wrong here: the strip sets POWER and LIGHT at once, and if the
    # resolver took POWER for a single tap, the menu would open on release.
    ("use (right strip)", 1, 290,  60, 0, "0",  "0",  "0"),
]



# Press briefly. Doom turns about 7 degrees per tic while running; half a
# second is already 120 degrees, a whole one more than a half turn - and
# then the measured rotation no longer says which way it went. 250 ms stay
# unambiguous.
PRESS_MS = 250
MOVE_EPS = 15.0   # map units that still count as "standing still"
MOVE_MIN = 40.0   # below this a movement counts as blocked
TURN_EPS = 2.0    # degrees


def axis_ok(want, got, eps, lo):
    if want is None:
        return True
    if want == "0":
        return abs(got) <= eps
    return got >= lo if want == "+" else got <= -lo


def decompose(a, b):
    """Path from a to b, decomposed along the *mean* facing.

    Not along the earlier one: walking forward while turning describes an
    arc, and an arc's chord lies on the bisector between start and end
    facing, not on the start facing. Reckoned against the start facing,
    "forward and right" can therefore look like backward motion - the
    previous run reported exactly that, and it was the arithmetic, not the
    device."""
    import math
    dx, dy = b["x"] - a["x"], b["y"] - a["y"]
    tn = ang_delta(a["ang"], b["ang"])
    th = (a["ang"] * 360.0 / 2 ** 32 + tn / 2.0) * math.pi / 180.0
    return (dx * math.cos(th) + dy * math.sin(th),      # forward
            -dx * math.sin(th) + dy * math.cos(th),     # to the left
            tn)


def cmd_motion(p, ms=PRESS_MS):
    """Measure the effect objectively: position and facing before and
    after the input. A rotation in degrees and a path in map units cannot
    be misread the way a camera frame can."""
    if not p.playing():
        w = p.where()
        print("No own game (demo %d, menu %d, game %d) - run 'enter' first."
              % (w["demo"], w["menu"], w["ingame"]))
        return 1
    print("input                 forward   sideways      turn   expected   result")
    bad = 0
    for nm, mode, x, y, bits, wf, ws, wt in MOVES:
        res = None
        for attempt in (0, 1):
            # Every step starts in the game, not in a menu: a menu opened
            # by accident would swallow everything that follows.
            if p.menu():
                p.press(3, bits=(1 << BTN_POWER), ms=120)
                time.sleep(0.5)
                if p.menu():
                    print("menu will not close - giving up.")
                    return 1
            p.stop_player()
            a = p.player()
            p.press(mode, x, y, bits, ms=ms)
            time.sleep(0.7)  # let inertia run out, Doom brakes over ~0.5 s
            b = p.player()
            if a is None or b is None:
                res = ("player object vanished", None)
                break
            fwd, side, tn = decompose(a, b)
            ok = (axis_ok(wf, fwd, MOVE_EPS, MOVE_MIN) and
                  axis_ok(ws, side, MOVE_EPS, MOVE_MIN) and
                  axis_ok(wt, tn, TURN_EPS, 30.0) and not p.menu())
            # The *other* translation axis is not checked against zero but
            # against the intended one dominating. Walking along an angled
            # wall makes you slide sideways with it - Doom's collision
            # handling, not a fault of the input. Measured: +252 forward
            # with +33 across, and -58 forward with +68 sideways, both
            # close to the wall of the starting room.
            # Only for pure translations. Turning at the same time walks an
            # arc and scrapes along walls - the ratio of the two axes then
            # says nothing any more.
            if wt == "0":
                if wf in ("+", "-") and ws is None and abs(fwd) <= abs(side):
                    ok = False
                if ws in ("+", "-") and wf is None and abs(side) <= abs(fwd):
                    ok = False
            res = ((fwd, side, tn), ok)
            if ok:
                break
            # A movement can end at a wall - that is not a fault of the
            # input. Turn 180 degrees and try again; facing is not a link
            # field, it can be set from outside.
            blocked = ((wf in ("+", "-") and abs(fwd) < MOVE_MIN) or
                       (ws in ("+", "-") and abs(side) < MOVE_MIN))
            if attempt == 0 and blocked and not p.menu():
                p.turn_player(180.0)
                time.sleep(0.2)
                continue
            break
        vals, ok = res
        if vals is None or isinstance(vals, str):
            print("%-20s %s" % (nm, vals))
            bad += 1
            continue
        fwd, side, tn = vals
        if not ok:
            bad += 1
        print("%-20s %+8.1f   %+8.1f   %+8.1f   %-4s %-4s %-4s  %s" %
              (nm, fwd, side, tn, wf or ".", ws or ".", wt or ".",
               "ok" if ok else ("MENU" if p.menu() else "MISMATCH")))
    print("")
    print("%d of %d inputs as intended" % (len(MOVES) - bad, len(MOVES)))
    return 1 if bad else 0


def cmd_enter(p, steps=10):
    """From the title screen or a demo into a game of your own.

    Not as a fixed key sequence but by state: where the program stands
    right now is read from `demoplayback`, `menuactive` and `usergame`. A
    fixed sequence would have made the same mistake as the first attempt -
    it went through during demo playback, because the demo already shows a
    level, and every measurement after that applied to the demo."""
    for i in range(steps):
        w = p.where()
        if p.playing():
            print("own game running: map %d, %d tics" % (w["gamemap"], w["leveltime"]))
            return 0
        if w["demo"]:
            what, bits = "Demo abbrechen", 1 << BTN_LIGHT
        elif w["menu"]:
            what, bits = "confirm menu", 1 << BTN_LIGHT
        else:
            what, bits = "open menu", 1 << BTN_POWER
        print("  %d. %-18s demo %d menu %d game %d gamestate %d map %d"
              % (i + 1, what, w["demo"], w["menu"], w["ingame"], w["gamestate"],
                 w["gamemap"]))
        p.press(3, bits=bits, ms=150)
        time.sleep(0.8)
    w = p.where()
    print("no own game reached: demo %d menu %d game %d gamestate %d"
          % (w["demo"], w["menu"], w["ingame"], w["gamestate"]))
    return 1


def cmd_shots(p, out, names=("start", "vor", "links", "rechts")):
    """Screenshots with an estimated reflection subtracted.

    The reflection only comes off if the picture changes - and only turning
    in the game changes it. That is why this command sits here and not in
    shot.py: it needs both, camera and input."""
    import shot as shotlib
    if not p.playing():
        print("No own game - run 'enter' first.")
        return 1
    os.makedirs(out, exist_ok=True)
    poses = [("start", None), ("vor", (1, 125, 40, 600)),
             ("links", (1, 40, 120, 400)), ("rechts", (1, 210, 120, 800))]
    for nm, act in poses:
        if act:
            p.press(act[0], act[1], act[2], ms=act[3])
            time.sleep(0.5)
        p.stop_player()
        time.sleep(0.4)
        path = os.path.join(out, "G_%s.png" % nm)
        shotlib.shot(path, n=10)
        pl = p.player()
        print("  %-24s map %d  x %.0f y %.0f  facing %.0f"
              % (path, p.game()["gamemap"], pl["x"], pl["y"], pl["deg"]))
    shotlib.cam.release()
    return 0


def cmd_goto(p, tx, ty, tries=40):
    """Drive the player to a point on the map using injected touches alone.

    This is where the input check ends: no longer single buttons against
    single readings but a closed loop - read the pose, work out the
    bearing, touch the area, read again. Arriving at all would survive
    neither a sign laid the wrong way round nor a swallowed button."""
    import math
    if not p.playing():
        print("No own game - run 'enter' first.")
        return 1
    for i in range(tries):
        a = p.player()
        dx, dy = tx - a["x"], ty - a["y"]
        dist = math.hypot(dx, dy)
        if dist < 40.0:
            print("Angekommen: x %.1f  y %.1f  (%.1f Schritte vom Ziel)"
                  % (a["x"], a["y"], dist))
            return 0
        want = math.degrees(math.atan2(dy, dx)) % 360.0
        err = (want - a["deg"] + 180.0) % 360.0 - 180.0
        if abs(err) > 12.0:
            # Doom turns about 7 degrees per tic while running; a tic is 28.6 ms.
            ms = max(60, min(400, int(abs(err) / 7.0 * 28.6)))
            p.press(1, 40 if err > 0 else 210, 120, ms=ms)
            act = "drehe %+6.1f Grad" % err
        else:
            ms = max(80, min(500, int(dist / 1.2)))
            p.press(1, 125, 40, ms=ms)
            act = "gehe %5.0f Schritte" % dist
        time.sleep(0.25)
        p.stop_player()
        print("  %2d. x %7.1f y %8.1f facing %6.1f  left %6.1f  %s"
              % (i + 1, a["x"], a["y"], a["deg"], dist, act))
    a = p.player()
    print("did not arrive: x %.1f y %.1f" % (a["x"], a["y"]))
    return 1


def cmd_walk(p, ms=1500):
    if not p.playing():
        print("No own game - run 'enter' first.")
        return 1
    a = p.player()
    print("before  x %.1f  y %.1f  facing %.1f deg" % (a["x"], a["y"], a["deg"]))
    p.press(1, 125, 40, ms=ms)
    time.sleep(0.4)
    b = p.player()
    print("after   x %.1f  y %.1f  facing %.1f deg" % (b["x"], b["y"], b["deg"]))
    d = ((b["x"] - a["x"]) ** 2 + (b["y"] - a["y"]) ** 2) ** 0.5
    print("distance %.1f map units in %d ms" % (d, ms))
    return 0


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 1
    what = argv[1]
    syms = symbols()
    v = vapedoom.Vape().open()
    try:
        v.find()
        p = Pad(v, syms)
        print("[sym] tinj 0x%08X  dbg 0x%08X  players 0x%08X" %
              (syms["tinj"], syms["dbg"], syms["players"]))
        if what == "state":
            return cmd_state(p)
        if what == "off":
            p.off()
            print("injection off, the controller counts again.")
            return 0
        if what == "probe":
            return cmd_probe(p)
        if what == "axis":
            return cmd_axis(p)
        if what == "motion":
            return cmd_motion(p, int(argv[2]) if len(argv) > 2 else 600)
        if what == "enter":
            return cmd_enter(p)
        if what == "shots":
            return cmd_shots(p, argv[2] if len(argv) > 2
                             else os.path.join(ROOT, "captures"))
        if what == "goto":
            return cmd_goto(p, float(argv[2]), float(argv[3]))
        if what == "walk":
            return cmd_walk(p, int(argv[2]) if len(argv) > 2 else 1500)
        if what == "tap":
            x, y = int(argv[2]), int(argv[3])
            ms = int(argv[4]) if len(argv) > 4 else 250
            got = p.press(1, x, y, ms=ms)
            print("(%d,%d) %d ms -> %s" % (x, y, ms, names(got)))
            return 0
        if what == "bits":
            rest = argv[2:]
            ms = 250
            if rest and rest[-1].isdigit():
                ms = int(rest[-1])
                rest = rest[:-1]
            bits = 0
            for n in rest:
                if n.upper() not in BTN_NAME:
                    raise SystemExit("unknown button %s (%s)" %
                                     (n, ", ".join(sorted(BTN_NAME))))
                bits |= 1 << BTN_NAME[n.upper()]
            got = p.press(3, bits=bits, ms=ms)
            print("%s %d ms -> %s" % (names(bits), ms, names(got)))
            return 0
        print(__doc__)
        return 1
    except KeyboardInterrupt:
        return 130
    finally:
        try:
            p.release()
        except Exception:
            pass
        v.close()


if __name__ == "__main__":
    sys.exit(main(sys.argv))
