# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Info tables: reachable states, split mobjinfo, actions, weapons, sfx.

The tables come from the upstream dumper. Only states reachable from the E1
roster are kept and renumbered densely. The engine-side knowledge below
(which actions spawn which types, drops, weapon pickups) mirrors the
vanilla game code.
"""

from . import wadread
from .util import check

FRACUNIT = 1 << 16
MF_SPECIAL = 0x1
MF_SOLID = 0x2
MF_SHOOTABLE = 0x4
MF_NOBLOCKMAP = 0x10
MF_NOSECTOR = 0x8
MF_SPAWNCEILING = 0x100
MF_DROPPED = 0x20000
MF_MISSILE = 0x10000
MF_COUNTKILL = 0x400000
MF_COUNTITEM = 0x800000

ACTION_SPAWNS = {
    "A_TroopAttack": ["MT_TROOPSHOT"], "A_HeadAttack": ["MT_HEADSHOT"],
    "A_BruisAttack": ["MT_BRUISERSHOT"], "A_CyberAttack": ["MT_ROCKET"],
    "A_FireMissile": ["MT_ROCKET"], "A_FirePlasma": ["MT_PLASMA"], "A_FireBFG": ["MT_BFG"],
    "A_BFGSpray": ["MT_EXTRABFG"],
}
KILL_DROPS = {"MT_WOLFSS": "MT_CLIP", "MT_POSSESSED": "MT_CLIP", "MT_SHOTGUY": "MT_SHOTGUN",
              "MT_CHAINGUY": "MT_CHAINGUN"}
WEAPON_PICKUPS = {"MT_SHOTGUN": "wp_shotgun", "MT_CHAINGUN": "wp_chaingun", "MT_MISC25": "wp_bfg",
                  "MT_MISC26": "wp_chainsaw", "MT_MISC27": "wp_missile", "MT_MISC28": "wp_plasma",
                  "MT_SUPERSHOTGUN": "wp_supershotgun"}
FLASH_PLUS_ONE = ("wp_chaingun", "wp_plasma")
ALWAYS_TYPES = ("MT_PLAYER", "MT_PUFF", "MT_BLOOD", "MT_TFOG")
ALWAYS_WEAPONS = ("wp_fist", "wp_pistol")
CODE_STATES = ("S_NULL", "S_LIGHTDONE", "S_GIBS", "S_PLAY", "S_PLAY_RUN1", "S_PLAY_ATK1",
               "S_PLAY_ATK2", "S_PUFF3", "S_BLOOD2", "S_BLOOD3")
DOOM2_ACTIONS = {
    "A_VileChase", "A_VileStart", "A_VileTarget", "A_VileAttack", "A_StartFire", "A_Fire",
    "A_FireCrackle", "A_Tracer", "A_SkelWhoosh", "A_SkelFist", "A_SkelMissile", "A_FatRaise",
    "A_FatAttack1", "A_FatAttack2", "A_FatAttack3", "A_BspiAttack", "A_BabyMetal", "A_PainAttack",
    "A_PainDie", "A_KeenDie", "A_BrainPain", "A_BrainScream", "A_BrainDie", "A_BrainAwake",
    "A_BrainSpit", "A_SpawnSound", "A_SpawnFly", "A_BrainExplode", "A_FireShotgun2",
    "A_CheckReload", "A_OpenShotgun2", "A_LoadShotgun2", "A_CloseShotgun2"}
DOOM2_THINGS = {68, 64, 88, 89, 69, 67, 71, 65, 66, 84}
# Vanilla state runs the engine indexes by arithmetic (P_XYMovement,
# A_FireCGun, A_FirePlasma): consecutive dense ids as well when reachable.
CODE_RUNS = (("S_PLAY_RUN1", 4), ("S_CHAIN1", 2), ("S_CHAINFLASH1", 2), ("S_PLASMAFLASH1", 2))
# Types and sprites the engine code names that only some rosters have (the
# shareware E1 has no plasma rifle, cells, berserk or lost souls, Freedoom's E1
# has no light amplification visor). info.h defines DMC_HAVE_<name> for those
# present.
OPTIONAL_TYPES = ("MT_PLASMA", "MT_SKULL")
OPTIONAL_SPRITES = ("PSTR", "CELL", "CELP", "PLAS", "PVIS")
ACTOR_STATES = ("seestate", "painstate", "meleestate", "missilestate", "deathstate", "xdeathstate")
ACTOR_SOUNDS = ("seesound", "attacksound", "painsound", "deathsound", "activesound")
E1_MAPS = ["E1M%d" % i for i in range(1, 10)]


class Info:
    """Upstream tables plus name lookups."""

    def __init__(self, up, deh=None):
        self.up = up
        self.states = [list(s) for s in up["states"]]
        self.snames = up["state_names"][:up["numstates"]]
        self.mnames = up["mobj_names"][:up["nummobjtypes"]]
        self.sprnames = up["sprnames"]
        self.mobjinfo = up["mobjinfo"]
        self.wnames = up["weapon_names"][:up["numweapons"]]
        self.sidx = {n: i for i, n in enumerate(self.snames)}
        self.midx = {n: i for i, n in enumerate(self.mnames)}
        self.widx = {n: i for i, n in enumerate(self.wnames)}
        self.by_ednum = {}
        for i, m in enumerate(self.mobjinfo):
            if m["doomednum"] != -1 and m["doomednum"] not in self.by_ednum:
                self.by_ednum[m["doomednum"]] = i
        # DEHACKED frame changes: state -> upstream frame value
        self.deh_changed = {}
        for n, fields in sorted((deh or {}).get("frames", {}).items()):
            if "Sprite subnumber" in fields:
                self.deh_changed[n] = self.states[n][1]
                self.states[n][1] = fields["Sprite subnumber"]
        self.deh_other = {n: f for n, f in (deh or {}).get("frames", {}).items()
                          if set(f) - {"Sprite subnumber"}}

    def thing_type(self, ednum):
        """Vanilla P_SpawnMapThing type lookup, or None for things never spawned in SP."""
        return self.by_ednum.get(ednum)


def sp_things(wad, info, mapname):
    """Single-player spawnable map things in WAD order: (wad index, x, y, angle, ednum, options, mobjtype)."""
    out = []
    starts = 0
    for i, (x, y, ang, ed, opt) in enumerate(wadread.parse_things(wad.map_lumps(mapname)["THINGS"])):
        check(ed not in DOOM2_THINGS, "%s: Doom 2 thing %d stops vanilla P_LoadThings", mapname, ed)
        if ed == 11 or ed <= 0 or 2 <= ed <= 4:
            continue
        if ed == 1:
            starts += 1
            out.append((i, x, y, ang, ed, opt, info.midx["MT_PLAYER"]))
            continue
        if opt & 16:
            continue
        t = info.thing_type(ed)
        check(t is not None, "%s: unknown thing type %d", mapname, ed)
        out.append((i, x, y, ang, ed, opt, t))
    check(starts == 1, "%s: %d player 1 starts", mapname, starts)
    return out


def closure(info, map_types):
    """Return (types, states, weapons) reachable from the given mobj types."""
    types = set(map_types) | {info.midx[n] for n in ALWAYS_TYPES}
    weapons = {info.widx[n] for n in ALWAYS_WEAPONS}
    states = set()
    while True:
        n0 = (len(types), len(states), len(weapons))
        for t in list(types):
            name = info.mnames[t]
            if name in WEAPON_PICKUPS:
                weapons.add(info.widx[WEAPON_PICKUPS[name]])
            if name in KILL_DROPS:
                types.add(info.midx[KILL_DROPS[name]])
        roots = {info.sidx[n] for n in CODE_STATES}
        for t in types:
            m = info.mobjinfo[t]
            roots.add(m["spawnstate"])
            roots.update(m[k] for k in ACTOR_STATES)
        for w in weapons:
            wi = info.up["weaponinfo"][w]
            roots.update(wi[1:6])
            if info.wnames[w] in FLASH_PLUS_ONE:
                roots.add(wi[5] + 1)
        stack = list(roots)
        while stack:
            s = stack.pop()
            if s in states:
                continue
            states.add(s)
            stack.append(info.states[s][4])
        for s in states:
            act = info.states[s][3]
            check(act not in DOOM2_ACTIONS, "Doom 2 action %s reachable from %s", act, info.snames[s])
            for tn in ACTION_SPAWNS.get(act, ()):
                types.add(info.midx[tn])
        if (len(types), len(states), len(weapons)) == n0:
            return types, states, weapons


def closure_from(info, types, weapons, code_roots=True):
    states = set()
    roots = set()
    for t in types:
        m = info.mobjinfo[t]
        roots.add(m["spawnstate"])
        roots.update(m[k] for k in ACTOR_STATES)
    for w in weapons:
        wi = info.up["weaponinfo"][w]
        roots.update(wi[1:6])
        if info.wnames[w] in FLASH_PLUS_ONE:
            roots.add(wi[5] + 1)
    if code_roots:
        roots.update(info.sidx[n] for n in ("S_PUFF3", "S_BLOOD2", "S_BLOOD3", "S_LIGHTDONE"))
    stack = list(roots)
    while stack:
        s = stack.pop()
        if s in states or s == 0:
            continue
        states.add(s)
        stack.append(info.states[s][4])
    return types, states, weapons


def roster_types(wad, info, maps):
    t = set()
    for m in maps:
        t.update(th[6] for th in sp_things(wad, info, m))
    return t


class Dense:
    """Firmware numbering of states, mobj types and sprites for the E1 roster."""

    def __init__(self, info, types, states, weapons):
        self.info = info
        self.weapons = weapons
        self.state_list = sorted(states | {0})
        self.state_id = {s: i for i, s in enumerate(self.state_list)}
        actors, others = [], []
        for t in sorted(types):
            (actors if self.is_actor(t) else others).append(t)
        self.type_list = actors + others
        self.nactors = len(actors)
        self.type_id = {t: i for i, t in enumerate(self.type_list)}
        sprs = sorted({info.states[s][0] for s in self.state_list if s})
        self.sprite_list = sprs
        self.sprite_id = {s: i for i, s in enumerate(sprs)}
        self.actions = sorted({info.states[s][3] for s in self.state_list if info.states[s][3]})
        self.action_id = {a: i + 1 for i, a in enumerate(self.actions)}

    def is_actor(self, t):
        m = self.info.mobjinfo[t]
        if self.info.mnames[t] == "MT_PLAYER":
            return True
        if m["flags"] & (MF_SHOOTABLE | MF_MISSILE | MF_COUNTKILL):
            return True
        return any(m[k] for k in ACTOR_STATES) or any(m[k] for k in ACTOR_SOUNDS)


def build(wad, up, deh):
    info = Info(up, deh)
    have = [m for m in E1_MAPS if wad.num(m) >= 0]
    types, states, weapons = closure(info, roster_types(wad, info, have))
    for n, f in info.deh_other.items():
        check(n not in states, "DEHACKED changes %s of reachable state %s", sorted(f), info.snames[n])
    return info, Dense(info, types, states, weapons)


# ------------------------------------------------------------------ C emission

def _c_array(ctype, name, rows, per_line=1):
    out = ["const %s %s[%d] = {" % (ctype, name, len(rows))]
    for i in range(0, len(rows), per_line):
        out.append("    " + " ".join(r + "," for r in rows[i:i + per_line]))
    out.append("};")
    return out


SFXNAME_LEN = 7


def emit(info, d, wad_sha256):
    """Return (info.h text, info.c text, sizes dict)."""
    up = info.up
    for name, n in CODE_RUNS:
        s0 = info.sidx[name]
        if s0 not in d.state_id:
            continue
        check(all(s0 + k in d.state_id and d.state_id[s0 + k] == d.state_id[s0] + k for k in range(n)),
              "states from %s are not consecutive dense ids", name)
    H = ["// SPDX-License-Identifier: GPL-2.0-or-later",
         "// Copyright (C) 1993-1996 Id Software, Inc.",
         "// Copyright (C) 2005-2014 Simon Howard",
         "// Copyright (C) 2026 Artur Bieniek",
         "// Generated by tools/conv/info.py from the upstream info tables. Do not edit.",
         "", "#ifndef DMC_INFO_H", "#define DMC_INFO_H", "", "#include <stdint.h>", ""]
    H.append("typedef enum {")
    for i, s in enumerate(d.state_list):
        H.append("    %s = %d," % (info.snames[s], i))
    H += ["    NUMSTATES = %d" % len(d.state_list), "} statenum_t;", ""]
    H.append("typedef enum {")
    for i, s in enumerate(d.sprite_list):
        H.append("    SPR_%s = %d," % (info.sprnames[s], i))
    H += ["    NUMSPRITES = %d" % len(d.sprite_list), "} spritenum_t;", ""]
    H.append("typedef enum {")
    for i, t in enumerate(d.type_list):
        H.append("    %s = %d," % (info.mnames[t], i))
    H += ["    NUMMOBJTYPES = %d" % len(d.type_list), "} mobjtype_t;", "",
          "// Types below DMC_NUMACTORS have a dmc_actorinfo_t entry",
          "#define DMC_NUMACTORS %d" % d.nactors, ""]
    have = [n for n in OPTIONAL_TYPES if info.midx[n] in d.type_id]
    have += ["SPR_" + n for n in OPTIONAL_SPRITES if info.sprnames.index(n) in d.sprite_id]
    H += ["// The optional types and sprites of tools/conv/info.py in this roster"]
    H += ["#define DMC_HAVE_%s 1" % n for n in have] + [""]
    H += ["// The first 4 bytes of the WAD's SHA-256 (little endian), as DOOM.DAT's header has them:",
          "// the boot check refuses a DOOM.DAT made from another WAD",
          "#define DMC_WAD_ID 0x%08Xu" % int.from_bytes(wad_sha256[:4], "little"), ""]
    H.append("#define DMC_ACTIONS(X) \\")
    H += ["    X(%s) \\" % a for a in d.actions]
    H.append("")
    H.append("enum dmc_action {")
    H.append("    DMC_ACT_NONE = 0,")
    for a in d.actions:
        H.append("    DMC_ACT_%s = %d," % (a, d.action_id[a]))
    H += ["    DMC_NUMACTIONS = %d" % (len(d.actions) + 1), "};", ""]
    H += ["// frame: bit 7 fullbright, bits 0-6 frame index. tics: 0xFF = -1.",
          "typedef struct {",
          "    uint8_t sprite;",
          "    uint8_t frame;",
          "    uint8_t tics;",
          "    uint8_t action;",
          "    uint16_t nextstate;",
          "} dmc_state_t;",
          "",
          "// dmc_states applies the Freedoom DEHACKED frame changes. Define",
          "// DMC_ORACLE_FRAMES for the upstream frames of the unmodified oracle.",
          "#define DMC_STATE_TICS(st) ((st)->tics == 0xFF ? -1 : (int)(st)->tics)",
          "#define DMC_STATE_FRAME(st) ((((st)->frame & 0x80) << 8) | ((st)->frame & 0x7F))",
          "",
          "// radius and height in map units",
          "typedef struct {",
          "    uint16_t flags_lo;",
          "    uint16_t flags_hi;",
          "    uint16_t spawnstate;",
          "    uint16_t spawnhealth;",
          "    uint8_t radius;",
          "    uint8_t height;",
          "} dmc_mobjinfo_t;",
          "",
          "#define DMC_MOBJ_FLAGS(mi) ((uint32_t)(mi)->flags_lo | ((uint32_t)(mi)->flags_hi << 16))",
          "",
          "// speed in map units per tic (shifted by FRACBITS for MF_MISSILE types)",
          "typedef struct {",
          "    uint16_t seestate;",
          "    uint16_t painstate;",
          "    uint16_t meleestate;",
          "    uint16_t missilestate;",
          "    uint16_t deathstate;",
          "    uint16_t xdeathstate;",
          "    uint16_t painchance;",
          "    uint16_t mass;",
          "    uint8_t speed;",
          "    uint8_t damage;",
          "    uint8_t reactiontime;",
          "    uint8_t seesound;",
          "    uint8_t attacksound;",
          "    uint8_t painsound;",
          "    uint8_t deathsound;",
          "    uint8_t activesound;",
          "} dmc_actorinfo_t;",
          "",
          "#define DMC_NONACTOR_REACTIONTIME 8",
          "",
          "typedef struct {",
          "    uint8_t ammo;",
          "    uint8_t pad;",
          "    uint16_t upstate;",
          "    uint16_t downstate;",
          "    uint16_t readystate;",
          "    uint16_t atkstate;",
          "    uint16_t flashstate;",
          "} dmc_weaponinfo_t;",
          "",
          "// link: sfx number or 0, pitch and volume only for links",
          "typedef struct {",
          "    uint8_t priority;",
          "    uint8_t link;",
          "    uint8_t pitch;",
          "    int8_t volume;",
          "} dmc_sfxinfo_t;",
          "",
          "#define DMC_NUMWEAPONS %d" % len(info.wnames),
          "#define DMC_NUMSFX %d" % up["numsfx"],
          "",
          "extern const dmc_state_t dmc_states[NUMSTATES];",
          "extern const dmc_mobjinfo_t dmc_mobjinfo[NUMMOBJTYPES];",
          "extern const dmc_actorinfo_t dmc_actorinfo[DMC_NUMACTORS];",
          "extern const dmc_weaponinfo_t dmc_weaponinfo[DMC_NUMWEAPONS];",
          "extern const dmc_sfxinfo_t dmc_sfxinfo[DMC_NUMSFX];",
          "",
          "// Vanilla numbers of the dense ids, for lockstep comparisons",
          "extern const uint16_t dmc_state_vanilla[NUMSTATES];",
          "extern const uint16_t dmc_mobj_vanilla[NUMMOBJTYPES];",
          "extern const uint16_t dmc_sprite_vanilla[NUMSPRITES];",
          "",
          "#ifdef DMC_HOST",
          "// Editor numbers and lump names for the host tools",
          "extern const int16_t dmc_mobj_doomednum[NUMMOBJTYPES];",
          "extern const char dmc_sprnames[NUMSPRITES][5];",
          "extern const char dmc_sfxnames[DMC_NUMSFX][%d];" % SFXNAME_LEN,
          "#endif",
          "",
          "#endif", ""]

    def sid(v):
        check(v in d.state_id, "state %s not reachable", info.snames[v])
        return d.state_id[v]

    def state_row(s, frame):
        spr, _, tics, act, nxt, m1, m2 = info.states[s]
        check(m1 == 0 and m2 == 0, "state %s uses misc1/misc2", info.snames[s])
        check((frame & 0x7FFF) < 0x80 and not frame & ~0x807F, "state %s frame %d", info.snames[s], frame)
        check(-1 <= tics < 255, "state %s tics %d", info.snames[s], tics)
        f8 = (frame & 0x7F) | (0x80 if frame & 0x8000 else 0)
        sp = d.sprite_id[spr] if s else 0
        return "{%d, 0x%02x, %d, %d, %d}, // %s" % (
            sp, f8, tics & 0xFF, d.action_id.get(act, 0), sid(nxt), info.snames[s])

    rows = []
    for s in d.state_list:
        frame = info.states[s][1]
        up_frame = info.deh_changed.get(s, frame)
        if up_frame == frame:
            rows.append(state_row(s, frame))
            continue
        # sprite lumps are chosen by frame index, so only the fullbright bit may differ
        check((up_frame ^ frame) & 0x7FFF == 0, "DEHACKED changes the frame index of %s", info.snames[s])
        rows += ["#ifdef DMC_ORACLE_FRAMES", state_row(s, up_frame), "#else", state_row(s, frame), "#endif"]

    C = ["// SPDX-License-Identifier: GPL-2.0-or-later",
         "// Copyright (C) 1993-1996 Id Software, Inc.",
         "// Copyright (C) 2005-2014 Simon Howard",
         "// Copyright (C) 2026 Artur Bieniek",
         "// Generated by tools/conv/info.py from the upstream info tables. Do not edit.",
         "", "#include \"info.h\"", ""]
    C += ["const dmc_state_t dmc_states[NUMSTATES] = {"]
    C += [r if r.startswith("#") else "    " + r for r in rows] + ["};", ""]
    mrows, arows = [], []
    for i, t in enumerate(d.type_list):
        m = info.mobjinfo[t]
        check(m["radius"] % FRACUNIT == 0 and 0 <= m["radius"] >> 16 < 256, "%s radius", info.mnames[t])
        check(m["height"] % FRACUNIT == 0 and 0 <= m["height"] >> 16 < 256, "%s height", info.mnames[t])
        check(0 <= m["spawnhealth"] < 65536, "%s health", info.mnames[t])
        mrows.append("{0x%04x, 0x%04x, %d, %d, %d, %d}, // %s" % (
            m["flags"] & 0xFFFF, (m["flags"] >> 16) & 0xFFFF, sid(m["spawnstate"]), m["spawnhealth"], m["radius"] >> 16,
            m["height"] >> 16, info.mnames[t]))
        if i < d.nactors:
            sp = m["speed"]
            if m["flags"] & MF_MISSILE:
                check(sp % FRACUNIT == 0 and sp >> 16 < 256, "%s missile speed", info.mnames[t])
                sp >>= 16
            check(0 <= sp < 256, "%s speed %d", info.mnames[t], m["speed"])
            check(0 <= m["mass"] < 65536, "%s mass %d", info.mnames[t], m["mass"])
            check(0 <= m["painchance"] <= 256, "%s painchance", info.mnames[t])
            check(0 <= m["damage"] < 256 and 0 <= m["reactiontime"] < 256, "%s damage", info.mnames[t])
            arows.append("{%s, %d, %d, %d, %d, %d, %s}, // %s" % (
                ", ".join(str(sid(m[k])) for k in ACTOR_STATES), m["painchance"], m["mass"], sp,
                m["damage"], m["reactiontime"], ", ".join(str(m[k]) for k in ACTOR_SOUNDS),
                info.mnames[t]))
        else:
            # the engine's p_noactor values
            check(m["reactiontime"] == 8 and m["speed"] == 0 and m["mass"] == 100 and m["damage"] == 0
                  and m["painchance"] == 0, "%s actor fields", info.mnames[t])
    C += ["const dmc_mobjinfo_t dmc_mobjinfo[NUMMOBJTYPES] = {"] + ["    " + r for r in mrows] + ["};", ""]
    C += ["const dmc_actorinfo_t dmc_actorinfo[DMC_NUMACTORS] = {"] + ["    " + r for r in arows] + ["};", ""]
    wrows = []
    for w, wi in enumerate(up["weaponinfo"]):
        have = w in d.weapons
        st = [sid(v) if have else 0 for v in wi[1:6]]
        wrows.append("{%d, 0, %s}, // %s" % (wi[0], ", ".join(map(str, st)), info.wnames[w]))
    C += ["const dmc_weaponinfo_t dmc_weaponinfo[DMC_NUMWEAPONS] = {"] + ["    " + r for r in wrows] + ["};", ""]
    srows = []
    for name, prio, link, pitch, vol in up["sfx"]:
        check(0 <= prio < 256, "sfx %s priority", name)
        if link >= 0:
            check(0 < link < 256 and 0 <= pitch < 256 and -128 <= vol < 128, "sfx %s link", name)
            srows.append("{%d, %d, %d, %d}, // %s" % (prio, link, pitch, vol, name))
        else:
            srows.append("{%d, 0, 0, 0}, // %s" % (prio, name))
    C += ["const dmc_sfxinfo_t dmc_sfxinfo[DMC_NUMSFX] = {"] + ["    " + r for r in srows] + ["};", ""]
    C += _c_array("uint16_t", "dmc_state_vanilla", [str(s) for s in d.state_list], 16)
    C += _c_array("uint16_t", "dmc_mobj_vanilla", [str(t) for t in d.type_list], 16)
    C += _c_array("uint16_t", "dmc_sprite_vanilla", [str(s) for s in d.sprite_list], 16)
    C += ["", "#ifdef DMC_HOST"]
    C += _c_array("int16_t", "dmc_mobj_doomednum", [str(info.mobjinfo[t]["doomednum"]) for t in d.type_list],
                  16)
    for sfx in up["sfx"]:
        check(len(sfx[0]) < SFXNAME_LEN, "sfx name %s", sfx[0])
    for ctype, rows in (("char dmc_sprnames[NUMSPRITES][5]", [info.sprnames[s] for s in d.sprite_list]),
                        ("char dmc_sfxnames[DMC_NUMSFX][%d]" % SFXNAME_LEN, [sfx[0] for sfx in up["sfx"]])):
        C += ["const %s = {" % ctype]
        C += ["    " + " ".join('"%s",' % r for r in rows[k:k + 12]) for k in range(0, len(rows), 12)]
        C += ["};"]
    C += ["#endif", ""]
    sizes = {"states": 6 * len(d.state_list), "mobjinfo": 10 * len(d.type_list),
             "actorinfo": 24 * d.nactors, "weaponinfo": 12 * len(info.wnames),
             "sfxinfo": 4 * up["numsfx"], "actions": 4 * (len(d.actions) + 1)}
    return "\n".join(H), "\n".join(C), sizes
