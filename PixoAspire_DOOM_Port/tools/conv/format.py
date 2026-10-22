# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""NOR image format, the single source of truth for every layout.

The converter packs records with these definitions, dmccheck and dmcview read
them back, and emit_header() writes gen/dmcdat.h for the firmware. All values
are little-endian. Offsets are logical image offsets unless a field says
otherwise. Records are naturally aligned so the C structs need no packing.
"""

import struct

FMT_MAJOR = 1
FMT_MINOR = 0

MAGIC = b"DMCDOOM\0"
MAGIC_X = b"DMCDOOMX"
MAGIC_SAVE = b"DMCSAVE1"
MAGIC_MAP = b"MAP1"

HEADER_SIZE = 4096
SECT_TABLE_OFF = 256
MAP_TABLE_OFF = 1792
MAX_SECTS = (MAP_TABLE_OFF - SECT_TABLE_OFF) // 16
MAX_MAPS = (HEADER_SIZE - MAP_TABLE_OFF) // 32
ALIGN = 16

# Volume capacities in bytes. DOOM.DAT takes 740 clusters of the 3 MB
# volume, DOOMX.DAT the 210 the firmware drive keeps after DOOM.BIN (25), DOOMCODE.DMC (11) and
# the Windows reserve (3).
DOOMDAT_CAP = 2960 * 1024
DOOMX_CAP = 840 * 1024
DOOMX_HDR = 64
SPLIT_ALIGN = 4096

# SAVE.DAT, made by the firmware: SAVE_SLOTS slots of SAVE_SLOT_SECTORS sectors. A slot holds
# a dmc_save_hdr_t, the globals block (at most SAVE_GLOBALS_MAX bytes), the order block when
# SF_ORDER is set and the arena.
SAVE_SIZE = 16 * 4096
SAVE_SLOTS = 4
SAVE_SLOT_SECTORS = 4
SAVE_GLOBALS_MAX = 896
SF_ORDER = 1 << 0             # creation order block present
SF_DEMO = 1 << 1              # demo cursor saved
SF_EXACT = 1 << 2             # gametic restored on load

# Header flags
HF_WALL_MIPS = 1 << 0
HF_HALF_SPRITES = 1 << 1
HF_HALF_WALLS = 1 << 2
HF_DEMO1 = 1 << 3
HF_HALF_MONSTERS = 1 << 4     # ladder step 2 (pack.py): monster sprites with rows halved too
HF_HALF_UI = 1 << 5           # the pack's ui_half pictures (TITLEPIC, WIMAP0) at half width

# Skill to rngk group (sk_baby, sk_easy -> 0, sk_medium -> 1, sk_hard, sk_nightmare -> 2)
SKILL_GROUP = (0, 0, 1, 2, 2)
GROUP_BITS = (1, 2, 4)        # mapthing option bit per group

# ------------------------------------------------------------------ sections

SECTIONS = [
    (1, "PALETTE"), (2, "TEXDIR"), (3, "TEXDATA"), (4, "FLATS"), (5, "PATCHDIR"),
    (6, "PATCHDATA"), (7, "SPRDEF"), (8, "SOUNDS"), (9, "STRINGS"), (10, "UIDIR"),
    (11, "ANIMS"), (12, "DEMOS"),
]
SECT = {n: i for i, n in SECTIONS}

PALETTE_COUNT = 14
FLAT_SIZE = 1024
FLAT_DIM = 32
FINALE_FLAT_SIZE = 4096

# TEXDIR flags
TF_MASKED = 1 << 0
TF_MIPS = 1 << 1
TF_BASE_SHIFT = 2
TF_BASE_MASK = 3 << 2

# Patch flags
PF_OPAQUE = 1 << 0            # every stored column is one post from row 0 to height-1
PF_HALF_ROWS = 1 << 1         # stored rows are original rows 2k (posts in stored rows)

COLINFO_OFF_MASK = 0x00FFFFFF
COLINFO_LEN_SHIFT = 24

# SPRDEF frame lump entries
SPR_PATCH_MASK = 0x1FFF
SPR_NO_PATCH = 0x1FFF
SPR_FLIP = 1 << 14
SPR_ROTATE = 1 << 15

# MAPENT next and secret_next values besides map numbers
NEXT_END_EPISODE = 0
NEXT_NOT_IN_PACK = 0xFF

# ANIMS kinds
ANIM_TEXTURE = 0
ANIM_FLAT = 1
ANIM_SWITCH = 2

# ------------------------------------------------------------------ map streams

STREAMS = ["NODES", "NODEBB", "SSECTORS", "SEGS", "LINES", "LINESIDES", "SIDES", "SECTORS",
           "SECEXTRA", "SECLINES", "TAGS", "BLOCKMAP", "GRID", "REJECT", "SNDADJ", "THINGS",
           "CELLTHINGS", "SECTHINGS", "ZCLIP", "TELEDEST", "SPECIALS", "MONSTERS", "TEXXLAT",
           "FLATXLAT"]
STREAM = {n: i for i, n in enumerate(STREAMS)}
assert len(STREAMS) == 24

NF_SUBSECTOR = 0x8000
NONE16 = 0xFFFF
NONE8 = 0xFF
SEG_LINE_MASK = 0x1FFF
SEG_SIDE = 1 << 13
SEG_HASMID = 1 << 14
SNDADJ_BLOCK = 0x8000
MAPBLOCKSHIFT = 7
GRID_MARGIN = 2
OPT_SKILL_MASK = 7
OPT_AMBUSH = 8
OPT_NOT_SP = 16

PLANS = "ABCD"

# ------------------------------------------------------------------ string and UI enums

STRING_IDS = (
    ["PRESSKEY", "PRESSYN", "QUITMSG"] + ["QUITMSG%d" % i for i in range(1, 8)] +
    ["QSPROMPT", "QLPROMPT", "SAVEDEAD", "QSAVESPOT", "NEWGAME", "NIGHTMARE", "SWSTRING",
     "MSGOFF", "MSGON", "ENDGAME", "DOSY", "DETAILHI", "DETAILLO", "EMPTYSTRING", "GGSAVED",
     "GOTARMOR", "GOTMEGA", "GOTHTHBONUS", "GOTARMBONUS", "GOTSTIM", "GOTMEDINEED", "GOTMEDIKIT",
     "GOTSUPER", "GOTBLUECARD", "GOTYELWCARD", "GOTREDCARD", "GOTBLUESKUL", "GOTYELWSKUL",
     "GOTREDSKULL", "GOTINVUL", "GOTBERSERK", "GOTINVIS", "GOTSUIT", "GOTMAP", "GOTVISOR",
     "GOTMSPHERE", "GOTCLIP", "GOTCLIPBOX", "GOTROCKET", "GOTROCKBOX", "GOTCELL", "GOTCELLBOX",
     "GOTSHELLS", "GOTSHELLBOX", "GOTBACKPACK", "GOTBFG9000", "GOTCHAINGUN", "GOTCHAINSAW",
     "GOTLAUNCHER", "GOTPLASMA", "GOTSHOTGUN", "PD_BLUEO", "PD_REDO", "PD_YELLOWO", "PD_BLUEK",
     "PD_REDK", "PD_YELLOWK", "HUSTR_MSGU"] +
    ["HUSTR_E1M%d" % i for i in range(1, 10)] +
    ["AMSTR_FOLLOWON", "AMSTR_FOLLOWOFF", "AMSTR_GRIDON", "AMSTR_GRIDOFF", "AMSTR_MARKEDSPOT",
     "AMSTR_MARKSCLEARED", "E1TEXT"])
STRING = {n: i for i, n in enumerate(STRING_IDS)}
QUITMSG_IDS = ["QUITMSG"] + ["QUITMSG%d" % i for i in range(1, 8)]

UI_NAMES = (
    ["STBAR", "STARMS"] + ["STTNUM%d" % i for i in range(10)] + ["STTPRCNT", "STTMINUS"] +
    ["STYSNUM%d" % i for i in range(10)] + ["STGNUM%d" % i for i in range(2, 8)] +
    ["STKEYS%d" % i for i in range(6)] +
    ["STFST%d%d" % (i, j) for i in range(5) for j in range(3)] +
    ["STFTR%d0" % i for i in range(5)] + ["STFTL%d0" % i for i in range(5)] +
    ["STFOUCH%d" % i for i in range(5)] + ["STFEVL%d" % i for i in range(5)] +
    ["STFKILL%d" % i for i in range(5)] + ["STFGOD0", "STFDEAD0"] +
    ["STCFN%03d" % i for i in range(33, 96)] + ["STCFN121"] +
    ["M_DOOM", "M_RDTHIS", "M_OPTION", "M_QUITG", "M_NGAME", "M_SKULL1", "M_SKULL2", "M_THERML",
     "M_THERMR", "M_THERMM", "M_THERMO", "M_ENDGAM", "M_PAUSE", "M_MESSG", "M_MSGON", "M_MSGOFF",
     "M_EPISOD", "M_EPI1", "M_EPI2", "M_EPI3", "M_EPI4", "M_HURT", "M_JKILL", "M_ROUGH",
     "M_SKILL", "M_NEWG", "M_ULTRA", "M_NMARE", "M_SVOL", "M_OPTTTL", "M_SAVEG", "M_LOADG",
     "M_DISP", "M_MSENS", "M_GDHIGH", "M_GDLOW", "M_DETAIL", "M_DISOPT", "M_SCRNSZ", "M_SGTTL",
     "M_MUSVOL", "M_SFXVOL", "M_LSLEFT", "M_LSCNTR", "M_LSRGHT", "M_CELL1", "M_CELL2"] +
    ["WIMAP0", "WIURH0", "WIURH1", "WISPLAT", "WIOSTK", "WIOSTI", "WIF", "WIMSTT", "WIOSTS",
     "WITIME", "WIPAR", "WIMINUS", "WIPCNT"] + ["WINUM%d" % i for i in range(10)] +
    ["WICOLON", "WISUCKS", "WISCRT2", "WIENTER"] + ["WILV0%d" % i for i in range(9)] +
    ["AMMNUM%d" % i for i in range(10)] +
    ["TITLEPIC", "HELP1", "CREDIT"])
UI = {n: i for i, n in enumerate(UI_NAMES)}
UI_FULLSCREEN = ("TITLEPIC", "HELP1", "CREDIT")

# ------------------------------------------------------------------ record structs

_FMT = {"u8": "B", "i8": "b", "u16": "H", "i16": "h", "u32": "I", "i32": "i", "char": "s"}
_SIZE = {"u8": 1, "i8": 1, "u16": 2, "i16": 2, "u32": 4, "i32": 4, "char": 1}
_CTYPE = {"u8": "uint8_t", "i8": "int8_t", "u16": "uint16_t", "i16": "int16_t",
          "u32": "uint32_t", "i32": "int32_t", "char": "char"}


class Struct:
    """Fixed-size little-endian record. fields: (name, type) or (name, type, count)."""

    def __init__(self, cname, size, fields, doc=""):
        self.cname, self.size, self.doc = cname, size, doc
        self.fields = [(f[0], f[1], f[2] if len(f) > 2 else 1) for f in fields]
        fmt = "<"
        off = 0
        self.offsets = {}
        for name, t, n in self.fields:
            if isinstance(t, Struct):
                assert off % t.align == 0, (cname, name, "misaligned")
                self.offsets[name] = off
                fmt += "%ds" % (t.size * n)
                off += t.size * n
                continue
            assert off % _SIZE[t] == 0, (cname, name, "misaligned")
            self.offsets[name] = off
            fmt += ("%ds" % n) if t == "char" else ((str(n) if n > 1 else "") + _FMT[t])
            off += _SIZE[t] * n
        assert off <= size, (cname, off, size)
        self.used = off
        self.align = max([t.align if isinstance(t, Struct) else _SIZE[t] for _, t, _ in self.fields])
        if off < size:
            fmt += "%dx" % (size - off)
        self.st = struct.Struct(fmt)
        assert self.st.size == size, (cname, self.st.size, size)

    def pack(self, **kw):
        vals = []
        for name, t, n in self.fields:
            if isinstance(t, Struct):
                vals.append(b"".join(kw.pop(name, [])))
                continue
            v = kw.pop(name, b"" if t == "char" else (0 if n == 1 else [0] * n))
            if t == "char" or n == 1:
                vals.append(v)
            else:
                v = list(v)
                assert len(v) == n, (self.cname, name, len(v), n)
                vals.extend(v)
        assert not kw, (self.cname, "unknown fields", kw)
        return self.st.pack(*vals)

    def unpack(self, buf, off=0):
        vals = list(self.st.unpack_from(buf, off))
        out = {}
        k = 0
        for name, t, n in self.fields:
            if isinstance(t, Struct):
                raw = vals[k]
                out[name] = [t.unpack(raw, t.size * i) for i in range(n)]
                k += 1
            elif t == "char" or n == 1:
                out[name] = vals[k]
                k += 1
            else:
                out[name] = vals[k:k + n]
                k += n
        return out

    def c_def(self):
        lines = ["typedef struct {"]
        for name, t, n in self.fields:
            ct = t.cname if isinstance(t, Struct) else _CTYPE[t]
            lines.append("    %s %s%s;" % (ct, name, "[%d]" % n if n > 1 or t == "char" else ""))
        if self.used < self.size:
            lines.append("    uint8_t pad_[%d];" % (self.size - self.used))
        lines.append("} %s;" % self.cname)
        lines.append("_Static_assert(sizeof(%s) == %d, \"%s size\");" % (self.cname, self.size, self.cname))
        for name, _, _ in self.fields:
            lines.append("_Static_assert(offsetof(%s, %s) == %d, \"%s.%s\");" % (
                self.cname, name, self.offsets[name], self.cname, name))
        return "\n".join(lines)


HEADER = Struct("dmc_header_t", 112, [
    ("magic", "char", 8), ("fmt_major", "u16"), ("fmt_minor", "u16"), ("image_size", "u32"),
    ("split", "u32"), ("image_uuid", "u8", 16), ("wad_sha256", "u8", 32), ("conv_version", "char", 16),
    ("pack_id", "char", 8), ("nsect", "u16"), ("nmaps", "u16"), ("flags", "u32"),
    ("header_crc", "u32"), ("skill_group", "u8", 5)],
    "Image header at logical offset 0. The header block is HEADER_SIZE bytes, CRC over all "
    "of it with header_crc = 0.")
assert HEADER.offsets["flags"] == 96 and HEADER.offsets["header_crc"] == 100
assert HEADER.offsets["skill_group"] == 104

SECTENT = Struct("dmc_sect_t", 16, [("id", "u16"), ("flags", "u16"), ("off", "u32"), ("size", "u32"),
                                    ("crc", "u32")])

MAPENT = Struct("dmc_mapent_t", 32, [
    ("name", "char", 8), ("off", "u32"), ("size", "u32"), ("crc", "u32"), ("episode", "u8"),
    ("map", "u8"), ("par_seconds", "u16"), ("name_string", "u16"), ("sky_texture", "u16"),
    ("next", "u8"), ("secret_next", "u8"), ("reserved", "u16")],
    "next/secret_next are map numbers in the same episode. DMC_NEXT_END_EPISODE ends the episode "
    "(finale), DMC_NEXT_NOT_IN_PACK means the vanilla next map is not in this pack (end of pack).")

DOOMX_HEADER = Struct("dmc_doomx_t", 64, [
    ("magic", "char", 8), ("fmt_major", "u16"), ("fmt_minor", "u16"), ("image_uuid", "u8", 16),
    ("logical_start", "u32"), ("length", "u32"), ("payload_crc", "u32")])

SAVE_HEADER = Struct("dmc_save_hdr_t", 64, [
    ("magic", "char", 8), ("map", "u8"), ("skill", "u8"), ("plan", "u8"), ("flags", "u8"),
    ("size", "u32"), ("crc", "u32"), ("leveltime", "u32"), ("desc", "char", 16),
    ("build_id", "u8", 8), ("image_uuid", "u8", 8), ("npages", "u16"), ("arena_tail", "u16"),
    ("store", "u32")],
    "Save slot header at the start of each slot of SAVE.DAT, programmed last. size and crc "
    "cover the bytes after the header. desc is made from map and leveltime (\"E1M3 12:34\"). "
    "build_id and image_uuid are the first 8 bytes of the firmware build ID and of the image "
    "uuid. An erased header (all 0xFF) is an empty slot.")
assert SAVE_HEADER.offsets["store"] == 60

TEXDIR_HDR = Struct("dmc_texdir_hdr_t", 16, [("count", "u16")])
TEXENT = Struct("dmc_texent_t", 16, [
    ("data", "u32"), ("width", "u16"), ("height", "u16"), ("flags", "u8"), ("log2_sh", "u8"),
    ("log2_w", "u8"), ("nlevels", "u8"), ("colinfo", "u32")],
    "data and colinfo are offsets into TEXDATA. Stored width is 1 << log2_w (vanilla "
    "texturewidthmask + 1). Stored height sh = 1 << log2_sh = min(pow2 >= height, 128), "
    "row r holds texture row r % height (the vanilla drawer masks rows with 127).")

PATCH_HDR = Struct("dmc_patch_t", 16, [
    ("width", "i16"), ("height", "i16"), ("leftoffset", "i16"), ("topoffset", "i16"),
    ("xshift", "u8"), ("flags", "u8"), ("ncols", "u16"), ("colinfo", "u32")],
    "colinfo is relative to the patch. Stored column k is original column k << xshift.")

SPRDEF_HDR = Struct("dmc_sprdef_hdr_t", 16, [("nsprites", "u16"), ("nframes", "u16"),
                                             ("frames_off", "u32")])
SPRITE = Struct("dmc_sprite_t", 4, [("first", "u16"), ("numframes", "u8")])
SPRFRAME = Struct("dmc_sprframe_t", 16, [("lump", "u16", 8)])

SOUNDS_HDR = Struct("dmc_sounds_hdr_t", 8, [("count", "u16")])
SOUND = Struct("dmc_sound_t", 8, [("off", "u32"), ("len", "u16")])

ANIM = Struct("dmc_anim_t", 8, [("kind", "u8"), ("speed", "u8"), ("first", "u16"), ("last", "u16"),
                                ("phase", "u16")],
              "Frame of global id g in [first, last]: first + (leveltime / speed + phase + "
              "(g - first)) % (last - first + 1). phase keeps vanilla's absolute-number cycle.")

LIST_HDR = Struct("dmc_list_hdr_t", 4, [("count", "u16")])

PLAN = Struct("dmc_plan_t", 24, [
    ("plan", "u8"), ("w", "u8"), ("k", "u8"), ("reserve", "u8"), ("cache_lines", "u16"),
    ("pages", "u16"), ("drop_slots", "u16"), ("mover_slots", "u16"), ("light_slots", "u16"),
    ("zclip_slots", "u16"), ("monsters", "u16"), ("need_play", "u16"), ("arena_bytes", "u16")])

MAPHDR = Struct("dmc_maphdr_t", 512, [
    ("magic", "char", 4), ("nlines", "u16"), ("nsides", "u16"), ("nsegs", "u16"),
    ("nssectors", "u16"), ("nnodes", "u16"), ("nsectors", "u16"), ("nthings", "u16"),
    ("nmonsters", "u16", 3), ("nspeclines", "u16"), ("nswitches", "u16"), ("nmutable", "u16"),
    ("ntags", "u16"), ("nzclip", "u16"), ("nlights", "u16"), ("nseclines", "u16"),
    ("nsndadj", "u16"), ("nteledest", "u16"), ("nscrollers", "u16"), ("ncellthings", "u16"),
    ("nsecthings", "u16"), ("bmap_ox", "i16"), ("bmap_oy", "i16"), ("bmap_cols", "u16"),
    ("bmap_rows", "u16"), ("root", "u16"), ("ntex", "u8"), ("nflat", "u8"), ("reserved0", "u32"),
    ("stream_off", "u32", 24), ("stream_size", "u32", 24), ("plans", PLAN, 3)],
    "Stream offsets are relative to the map block. plans[g] is the plan for rngk skill group g.")
assert MAPHDR.offsets["stream_off"] == 64 and MAPHDR.offsets["plans"] == 256

NODE = Struct("dmc_node_t", 16, [("x", "i16"), ("y", "i16"), ("dx", "i16"), ("dy", "i16"),
                                 ("child", "u16", 2)])
NODEBB = Struct("dmc_nodebb_t", 16, [("bbox", "i16", 8)],
                "bbox[child * 4 + BOXTOP/BOXBOTTOM/BOXLEFT/BOXRIGHT], map units.")
SSECTOR = Struct("dmc_ssector_t", 8, [("firstseg", "u16"), ("numsegs", "u16"), ("sector", "u16")])
SEG = Struct("dmc_seg_t", 16, [("v1x", "i16"), ("v1y", "i16"), ("v2x", "i16"), ("v2y", "i16"),
                               ("angle", "u16"), ("offset", "i16"), ("line", "u16"),
                               ("backsector", "u16")])
LINE = Struct("dmc_line_t", 16, [("x1", "i16"), ("y1", "i16"), ("dx", "i16"), ("dy", "i16"),
                                 ("front", "u16"), ("back", "u16"), ("flags", "u8"),
                                 ("special", "u8"), ("tag", "u8"), ("slot", "u8")],
                "tag is a TAGS index (0 none), slot the special-line slot (0xFF none).")
LINESIDES = Struct("dmc_linesides_t", 4, [("side", "u16", 2)])
SIDE = Struct("dmc_side_t", 8, [("textureoffset", "i16"), ("rowoffset", "i16"), ("top", "u8"),
                                ("bottom", "u8"), ("mid", "u8"), ("switch_slot", "u8")])
SECTOR = Struct("dmc_sector_t", 16, [("floor", "i16"), ("ceiling", "i16"), ("light", "i16"),
                                     ("floorpic", "u8"), ("ceilingpic", "u8"), ("special", "u8"),
                                     ("tag", "u8"), ("overlay", "u8"), ("pad", "u8"),
                                     ("linecount", "u16"), ("firstline", "u16")],
                "floorpic/ceilingpic are per-map flat indices, 0 = F_SKY1. light is i16 because "
                "vanilla maps use 256. tag is a TAGS index, overlay the mutable slot (0xFF none).")
SECEXTRA = Struct("dmc_secextra_t", 8, [("blockbox", "u8", 4), ("soundorg_x", "i16"),
                                        ("soundorg_y", "i16")],
                  "soundorg is the vanilla fixed point value >> FRACBITS (floor).")
THING = Struct("dmc_thing_t", 12, [("x", "i16"), ("y", "i16"), ("sector", "u16"), ("type", "u8"),
                                   ("angle45", "u8"), ("options", "u8"), ("rngk", "u8", 3)],
               "angle45 = (angle / 45) & 7 (C division). type is the dense mobjtype.")
RANGE = Struct("dmc_range_t", 4, [("first", "u16"), ("count", "u16")])
TELEDEST = Struct("dmc_teledest_t", 4, [("sector", "u16"), ("thing", "u16")])

ALL_STRUCTS = [HEADER, SECTENT, MAPENT, DOOMX_HEADER, SAVE_HEADER, TEXDIR_HDR, TEXENT, PATCH_HDR,
               SPRDEF_HDR, SPRITE, SPRFRAME, SOUNDS_HDR, SOUND, ANIM, LIST_HDR, PLAN, MAPHDR, NODE,
               NODEBB, SSECTOR, SEG, LINE, LINESIDES, SIDE, SECTOR, SECEXTRA, THING, RANGE,
               TELEDEST]

STREAM_DOC = {
    "NODES": "dmc_node_t[nnodes], child bit 15 = subsector",
    "NODEBB": "dmc_nodebb_t[nnodes]",
    "SSECTORS": "dmc_ssector_t[nssectors]",
    "SEGS": "dmc_seg_t[nsegs], line: bits 0-12 index, bit 13 side, bit 14 sidedef has midtexture",
    "LINES": "dmc_line_t[nlines]",
    "LINESIDES": "dmc_linesides_t[nlines], unique side index or 0xFFFF",
    "SIDES": "dmc_side_t[nsides], texture fields are per-map indices (0 none)",
    "SECTORS": "dmc_sector_t[nsectors]",
    "SECEXTRA": "dmc_secextra_t[nsectors]",
    "SECLINES": "u16[nseclines], vanilla P_GroupLines order",
    "TAGS": "dmc_range_t[ntags + 1] (entry 0 unused), u16 sectors[], u16 tagvalue[ntags + 1]",
    "BLOCKMAP": "vanilla lump from byte 8: u16 offsets[cols * rows] then lists. A list starts at "
                "stream + 2 * offset - 8 (offsets count lump words)",
    "GRID": "u16[cols * rows] start node (cell by * cols + bx like the blockmap), bit 15 = "
            "subsector. Points outside the blockmap start at the root",
    "REJECT": "vanilla bits, (nsectors^2 + 7) / 8 bytes",
    "SNDADJ": "dmc_range_t[nsectors] into u16 entries[nsndadj], entry = sector | 0x8000 if every "
              "joining two-sided line is ML_SOUNDBLOCK",
    "THINGS": "dmc_thing_t[nthings]",
    "CELLTHINGS": "u16 first[cols * rows + 1] then u16 things[ncellthings], reverse spawn order",
    "SECTHINGS": "u16 first[nsectors + 1] then u16 things[nsecthings], reverse spawn order",
    "ZCLIP": "u16 count, u16 things[count] (slot = position)",
    "TELEDEST": "dmc_teledest_t[nteledest] sorted by (sector, thing): every MT_TELEPORTMAN, the "
                "engine takes the first one spawned for the skill",
    "SPECIALS": "u16 lines[nscrollers] with special 48, ascending",
    "MONSTERS": "u16 thing index per monster ordinal (spawnstate action A_Look), all skills",
    "TEXXLAT": "u16 global texture id per per-map index (index 0 = 0xFFFF)",
    "FLATXLAT": "u16 global flat id per per-map index (index 0 = F_SKY1 = 0xFFFF)",
}

SECTION_DOC = {
    "PALETTE": "14 x 256 x u16 RGB565, colour c of palette p at (p * 256 + c) * 2",
    "TEXDIR": "dmc_texdir_hdr_t then dmc_texent_t[count]",
    "TEXDATA": "texture levels and masked posts, see dmc_texent_t",
    "FLATS": "flags = count. count x 1024 (texel [v * 32 + u]), u8 avgcolor[count], then the "
             "64x64 finale flat at DMC_FINALE_FLAT_OFF(count)",
    "PATCHDIR": "u32 count, u32 off[count] relative to PATCHDATA",
    "PATCHDATA": "dmc_patch_t, u32 colinfo[ncols] (offset 24 bits | length 8 bits), posts "
                 "{u8 topdelta, u8 length, u8 pixels[length]}..., 0xFF",
    "SPRDEF": "dmc_sprdef_hdr_t, dmc_sprite_t[nsprites] at 16, dmc_sprframe_t[nframes] at frames_off",
    "SOUNDS": "dmc_sounds_hdr_t, dmc_sound_t[count] at 8 indexed by vanilla sfx number, offsets "
              "relative to the section, data = DP sample bytes",
    "STRINGS": "dmc_list_hdr_t, u32 off[count] at 4 relative to the section, NUL-terminated Latin-1",
    "UIDIR": "flags = count. u16 patch id per enum dmc_ui, 0xFFFF absent",
    "ANIMS": "flags = count. dmc_anim_t[count]",
    "DEMOS": "dmc_list_hdr_t, u32 off[count] at 4 relative to the section, v1.9 lmp bytes",
}


def finale_flat_off(count):
    return (count * (FLAT_SIZE + 1) + ALIGN - 1) & ~(ALIGN - 1)


def colinfo(off, length):
    assert 0 <= off <= COLINFO_OFF_MASK and 0 < length <= 255, (off, length)
    return off | (length << COLINFO_LEN_SHIFT)


# ------------------------------------------------------------------ gen/dmcdat.h

def _enum(name, prefix, items, count_name):
    out = ["enum %s {" % name]
    for i, n in enumerate(items):
        out.append("    %s%s = %d," % (prefix, n, i))
    out.append("    %s = %d" % (count_name, len(items)))
    out.append("};")
    return "\n".join(out)


def emit_header():
    L = ["// SPDX-License-Identifier: GPL-2.0-or-later",
         "// Copyright (C) 2026 Artur Bieniek",
         "// Generated by tools/conv/format.py. Do not edit.",
         "",
         "#ifndef DMCDAT_H",
         "#define DMCDAT_H",
         "",
         "#include <stddef.h>",
         "#include <stdint.h>",
         "",
         "#define DMC_FMT_MAJOR %d" % FMT_MAJOR,
         "#define DMC_FMT_MINOR %d" % FMT_MINOR,
         "#define DMC_MAGIC \"DMCDOOM\"",
         "#define DMC_MAGIC_X \"DMCDOOMX\"",
         "#define DMC_MAGIC_SAVE \"DMCSAVE1\"",
         "#define DMC_MAGIC_MAP \"MAP1\"",
         "#define DMC_HEADER_SIZE %d" % HEADER_SIZE,
         "#define DMC_SECT_TABLE_OFF %d" % SECT_TABLE_OFF,
         "#define DMC_MAP_TABLE_OFF %d" % MAP_TABLE_OFF,
         "#define DMC_MAX_SECTS %d" % MAX_SECTS,
         "#define DMC_MAX_MAPS %d" % MAX_MAPS,
         "#define DMC_ALIGN %d" % ALIGN,
         "#define DMC_DOOMX_HDR %d" % DOOMX_HDR,
         "#define DMC_SAVE_SIZE %d" % SAVE_SIZE,
         "#define DMC_SAVE_SLOTS %d" % SAVE_SLOTS,
         "#define DMC_SAVE_SLOT_SECTORS %d" % SAVE_SLOT_SECTORS,
         "#define DMC_SAVE_GLOBALS_MAX %d" % SAVE_GLOBALS_MAX,
         "#define DMC_SF_ORDER 0x%xu" % SF_ORDER,
         "#define DMC_SF_DEMO 0x%xu" % SF_DEMO,
         "#define DMC_SF_EXACT 0x%xu" % SF_EXACT,
         "",
         "#define DMC_HF_WALL_MIPS 0x%xu" % HF_WALL_MIPS,
         "#define DMC_HF_HALF_SPRITES 0x%xu" % HF_HALF_SPRITES,
         "#define DMC_HF_HALF_WALLS 0x%xu" % HF_HALF_WALLS,
         "#define DMC_HF_DEMO1 0x%xu" % HF_DEMO1,
         "#define DMC_HF_HALF_MONSTERS 0x%xu" % HF_HALF_MONSTERS,
         "#define DMC_HF_HALF_UI 0x%xu" % HF_HALF_UI,
         "",
         "// CRC-32/IEEE: reflected, poly 0xEDB88320, init and xorout 0xFFFFFFFF",
         "",
         "enum dmc_section {"]
    for i, n in SECTIONS:
        L.append("    DMC_SECT_%s = %d,  // %s" % (n, i, SECTION_DOC[n]))
    L += ["    DMC_SECT_MAX = %d" % (max(i for i, _ in SECTIONS) + 1), "};", ""]
    L += ["#define DMC_PALETTE_COUNT %d" % PALETTE_COUNT,
          "#define DMC_FLAT_SIZE %d" % FLAT_SIZE,
          "#define DMC_FLAT_DIM %d" % FLAT_DIM,
          "#define DMC_FINALE_FLAT_SIZE %d" % FINALE_FLAT_SIZE,
          "#define DMC_FINALE_FLAT_OFF(count) ((((uint32_t)(count) * %du) + %du) & ~%du)" % (
              FLAT_SIZE + 1, ALIGN - 1, ALIGN - 1),
          "#define DMC_TF_MASKED 0x%xu" % TF_MASKED,
          "#define DMC_TF_MIPS 0x%xu" % TF_MIPS,
          "#define DMC_TF_BASE_SHIFT %d" % TF_BASE_SHIFT,
          "#define DMC_TF_BASE_MASK 0x%xu" % TF_BASE_MASK,
          "#define DMC_PF_OPAQUE 0x%xu" % PF_OPAQUE,
          "#define DMC_PF_HALF_ROWS 0x%xu" % PF_HALF_ROWS,
          "#define DMC_COLINFO_OFF(ci) ((ci) & 0x%06xu)" % COLINFO_OFF_MASK,
          "#define DMC_COLINFO_LEN(ci) ((ci) >> %d)" % COLINFO_LEN_SHIFT,
          "#define DMC_SPR_PATCH_MASK 0x%xu" % SPR_PATCH_MASK,
          "#define DMC_SPR_NO_PATCH 0x%xu" % SPR_NO_PATCH,
          "#define DMC_SPR_FLIP 0x%xu" % SPR_FLIP,
          "#define DMC_SPR_ROTATE 0x%xu" % SPR_ROTATE,
          "#define DMC_NEXT_END_EPISODE %d" % NEXT_END_EPISODE,
          "#define DMC_NEXT_NOT_IN_PACK 0x%x" % NEXT_NOT_IN_PACK,
          "#define DMC_ANIM_TEXTURE %d" % ANIM_TEXTURE,
          "#define DMC_ANIM_FLAT %d" % ANIM_FLAT,
          "#define DMC_ANIM_SWITCH %d" % ANIM_SWITCH,
          "",
          "#define DMC_NF_SUBSECTOR 0x%xu" % NF_SUBSECTOR,
          "#define DMC_NONE16 0x%xu" % NONE16,
          "#define DMC_NONE8 0x%xu" % NONE8,
          "#define DMC_SEG_LINE_MASK 0x%xu" % SEG_LINE_MASK,
          "#define DMC_SEG_SIDE 0x%xu" % SEG_SIDE,
          "#define DMC_SEG_HASMID 0x%xu" % SEG_HASMID,
          "#define DMC_SNDADJ_BLOCK 0x%xu" % SNDADJ_BLOCK,
          "#define DMC_GRID_MARGIN %d" % GRID_MARGIN,
          "#define DMC_OPT_SKILL_MASK %d" % OPT_SKILL_MASK,
          "#define DMC_OPT_AMBUSH %d" % OPT_AMBUSH,
          "#define DMC_OPT_NOT_SP %d" % OPT_NOT_SP,
          "",
          "// Skill (sk_baby..sk_nightmare) to rngk group and mapthing option bit",
          "#define DMC_SKILL_GROUP(sk) ((sk) <= 1 ? 0 : (sk) == 2 ? 1 : 2)",
          "#define DMC_GROUP_BIT(g) (1u << (g))",
          "",
          "enum dmc_plan { DMC_PLAN_A, DMC_PLAN_B, DMC_PLAN_C, DMC_PLAN_D };",
          ""]
    L.append("enum dmc_stream {")
    for i, n in enumerate(STREAMS):
        L.append("    DMC_ST_%s = %d,  // %s" % (n, i, STREAM_DOC[n]))
    L += ["    DMC_NSTREAMS = %d" % len(STREAMS), "};", ""]
    L.append(_enum("dmc_string", "DMC_STR_", STRING_IDS, "DMC_NSTRINGS"))
    L.append("")
    L.append(_enum("dmc_ui", "DMC_UI_", UI_NAMES, "DMC_NUI"))
    L.append("")
    for s in ALL_STRUCTS:
        if s.doc:
            L.append("// " + s.doc)
        L.append(s.c_def())
        L.append("")
    L += ["#endif", ""]
    return "\n".join(L)
