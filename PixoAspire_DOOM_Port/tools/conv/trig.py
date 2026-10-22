# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Exact compressed trig, view and light tables, and COLORMAP rows 0-32.

Every encoding is decoded here in Python with the same integer arithmetic
as port/math/trig.c and checked against the upstream values before the C
tables are written. tests/conv/trig_test.c repeats the check in C.
"""

from .util import ConvError, check, fixed_div, s32, u32

FINEANGLES = 8192
SINE_FIX_BLOCK = 64
SINE_SHIFT = 4
SINE_STEP = 1 << SINE_SHIFT
TAN_FULL = 256
TAN_STEP = 8
ATAN_BLOCK = 32
ATAN_SHIFT = 12
LIGHTLEVELS = 16
MAXLIGHTSCALE = 48
MAXLIGHTZ = 128
NUMCOLORMAPS = 32
COLORMAP_ROWS = 33
ANG90 = 0x40000000


# ------------------------------------------------------------------ finesine

def sine_rule(q, i):
    """q is the quarter wave, a list or a function of the index."""
    if not callable(q):
        q = q.__getitem__
    a = i & (FINEANGLES - 1)
    j = a & 2047
    quad = a >> 11
    if quad == 0:
        return q(j)
    if quad == 1:
        return q(2047 - j)
    if quad == 2:
        return -q(j)
    return -q(2047 - j)


# Compact sine table (DMC_SINE_L1): the quarter wave interpolated between every
# 16th entry (mirrored past the quarter) plus a residual of 0..3 in 2 bits.
# The residual is 0..2 already with a straight line, so the fit has no
# quadratic term.

def enc_sine_l1(q):
    ncoarse = 2048 // SINE_STEP + 1
    coarse = [q[j] if j < 2048 else q[4095 - j] for j in range(0, SINE_STEP * ncoarse, SINE_STEP)]
    resid = []
    for j in range(2048):
        r = q[j] - sine_l1_pred(coarse, j)
        check(0 <= r <= 3, "finesine L1 residual %d at %d", r, j)
        resid.append(r)
    packed = bytes(resid[k] | resid[k + 1] << 2 | resid[k + 2] << 4 | resid[k + 3] << 6
                   for k in range(0, 2048, 4))
    return {"coarse": coarse, "resid": packed}


def sine_l1_pred(coarse, j):
    k = j // SINE_STEP
    c0, c1 = coarse[k], coarse[k + 1]
    return c0 + (((c1 - c0) * (j % SINE_STEP)) >> SINE_SHIFT)


def dec_sine_l1_quarter(e, j):
    return sine_l1_pred(e["coarse"], j) + (e["resid"][j >> 2] >> ((j & 3) * 2) & 3)


def enc_sine(fs):
    check(len(fs) == 10240, "finesine size")
    q = fs[:2048]
    check(all(0 <= v <= 0xFFFF for v in q), "finesine quarter does not fit u16")
    fix = [(i, fs[i]) for i in range(len(fs)) if sine_rule(q, i) != fs[i]]
    nmap = (len(fs) // SINE_FIX_BLOCK + 31) // 32
    fmap = [0] * nmap
    for i, _ in fix:
        b = i // SINE_FIX_BLOCK
        fmap[b >> 5] |= 1 << (b & 31)
    return {"q": q, "fix_idx": [i for i, _ in fix], "fix_val": [v for _, v in fix], "fix_map": fmap,
            "l1": enc_sine_l1(q)}


def dec_sine(e, i, l1=False):
    b = i // SINE_FIX_BLOCK
    if e["fix_map"][b >> 5] >> (b & 31) & 1:
        for k, idx in enumerate(e["fix_idx"]):
            if idx == i:
                return e["fix_val"][k]
    if l1:
        return sine_rule(lambda j: dec_sine_l1_quarter(e["l1"], j), i)
    return sine_rule(e["q"], i)


# ------------------------------------------------------------------ finetangent

def _tan_full(ft, i):
    return ft[i] if i < 2048 else -ft[4095 - i]


def enc_tan(ft):
    check(len(ft) == 4096, "finetangent size")
    check(all(ft[4095 - i] == -ft[i] for i in range(2048)), "finetangent not antisymmetric")
    ncoarse = (2048 - TAN_FULL) // TAN_STEP + 2
    coarse = [_tan_full(ft, TAN_FULL + TAN_STEP * m) for m in range(ncoarse)]
    resid = []
    for i in range(TAN_FULL, 2048):
        r = ft[i] - tan_pred(coarse, i)
        check(-8 <= r <= 7, "finetangent residual %d at %d", r, i)
        resid.append(r & 15)
    nib = bytes(resid[k] | (resid[k + 1] << 4) for k in range(0, len(resid), 2))
    return {"full": ft[:TAN_FULL], "coarse": coarse, "resid": nib}


def tan_pred(coarse, i):
    k = (i - TAN_FULL) >> 3
    f = (i - TAN_FULL) & 7
    c0, c1, c2 = coarse[k], coarse[k + 1], coarse[k + 2]
    d1 = c1 - c0
    d2 = c2 - 2 * c1 + c0
    t1 = d1 * f
    t2 = d2 * f * (f - 8)
    check(abs(t1) < 1 << 31 and abs(t2) < 1 << 31 and abs(d2) < 1 << 31, "finetangent overflow")
    return c0 + (t1 >> 3) + (t2 >> 7)


def dec_tan(e, i):
    neg = i >= 2048
    if neg:
        i = 4095 - i
    if i < TAN_FULL:
        v = e["full"][i]
    else:
        k = i - TAN_FULL
        r = (e["resid"][k >> 1] >> ((k & 1) * 4)) & 15
        if r & 8:
            r -= 16
        v = tan_pred(e["coarse"], i) + r
    return -v if neg else v


# ------------------------------------------------------------------ tantoangle

def enc_atan(tt):
    tt = [u32(v) for v in tt]
    check(len(tt) == 2049, "tantoangle size")
    nb = (len(tt) + ATAN_BLOCK - 1) // ATAN_BLOCK
    base, ca, cc, resid = [], [], [], []
    for b in range(nb):
        seg = tt[b * ATAN_BLOCK:(b + 1) * ATAN_BLOCK]
        y0 = seg[0]
        if len(seg) == ATAN_BLOCK:
            x1 = ATAN_BLOCK - 1
            m = ATAN_BLOCK // 2
            a = ((seg[x1] - y0) << ATAN_SHIFT) // x1
            c = (((seg[m] - y0) << ATAN_SHIFT) - a * m) // (m * (m - x1))
        else:
            a = c = 0
        check(-(1 << 31) <= a < 1 << 31 and -(1 << 31) <= c < 1 << 31, "tantoangle coefficient")
        base.append(y0)
        ca.append(a)
        cc.append(c)
        for j, v in enumerate(seg):
            r = v - atan_pred(y0, a, c, j)
            check(-128 <= r <= 127, "tantoangle residual %d", r)
            resid.append(r)
    return {"base": base, "a": ca, "c": cc, "resid": resid}


def atan_pred(y0, a, c, j):
    return u32(y0 + ((a * j + c * (j * (j - (ATAN_BLOCK - 1)))) >> ATAN_SHIFT))


def dec_atan(e, i):
    b = i // ATAN_BLOCK
    j = i % ATAN_BLOCK
    return u32(atan_pred(e["base"][b], e["a"][b], e["c"][b], j) + e["resid"][i])


# ------------------------------------------------------------------ view tables

def enc_vatx(v, width):
    check(len(v) == 4096, "viewangletox size")
    check(v[0] == width and v[-1] == 0, "viewangletox ends")
    steps = [v[i - 1] - v[i] for i in range(1, 4096)]
    check(all(s in (0, 1) for s in steps), "viewangletox steps are not 0/-1")
    lo = next(i for i in range(1, 4096) if v[i] != v[i - 1])
    hi = max(i for i in range(1, 4096) if v[i] != v[i - 1])
    n = hi - lo + 1
    words = [0] * ((n + 31) // 32)
    for k in range(n):
        if v[lo + k - 1] - v[lo + k]:
            words[k >> 5] |= 1 << (k & 31)
    prefix = []
    acc = 0
    for w in words:
        prefix.append(acc)
        acc += bin(w).count("1")
    return {"lo": lo, "hi": hi, "width": width, "words": words, "prefix": prefix}


def dec_vatx(e, i):
    if i < e["lo"]:
        return e["width"]
    if i > e["hi"]:
        return 0
    k = i - e["lo"]
    w = e["words"][k >> 5] & u32((2 << (k & 31)) - 1)
    return e["width"] - (e["prefix"][k >> 5] + bin(w).count("1"))


def enc_xtva(xta):
    out = []
    for v in xta:
        t = u32(u32(v) + ANG90)
        check(t & ((1 << 19) - 1) == 0, "xtoviewangle not a fine angle")
        out.append(t >> 19)
    return out


def dec_xtva(e, x):
    return u32((e[x] << 19) - ANG90)


# ------------------------------------------------------------------ light

def startmap(i):
    return ((LIGHTLEVELS - 1 - i) * 2) * NUMCOLORMAPS // LIGHTLEVELS


def enc_light():
    """R_InitLightTables distance term: level = startmap - dist[j]."""
    dist = []
    for j in range(MAXLIGHTZ):
        scale = fixed_div(160 << 16, (j + 1) << 20) >> 12
        dist.append(scale // 2)
    check(all(0 <= d < 256 for d in dist), "light distance range")
    return {"dist": dist, "startmap": [startmap(i) for i in range(LIGHTLEVELS)]}


def clamp_level(v):
    return 0 if v < 0 else NUMCOLORMAPS - 1 if v >= NUMCOLORMAPS else v


def dec_zlight(e, i, j):
    return clamp_level(e["startmap"][i] - e["dist"][j])


def dec_scalelight(e, i, j):
    return clamp_level(e["startmap"][i] - j // 2)


# ------------------------------------------------------------------ build + verify

def build(up, colormap):
    sine = enc_sine(up["finesine"])
    tan = enc_tan(up["finetangent"])
    atan = enc_atan(up["tantoangle"])
    view = {}
    for v in up["view"]:
        det = v["detail"]
        check(v["viewheight"] == 200, "viewheight")
        view[det] = {"vatx": enc_vatx(v["viewangletox"], v["viewwidth"]),
                     "xtva": enc_xtva(v["xtoviewangle"]), "width": v["viewwidth"]}
    check(up["view"][0]["yslope"] == up["view"][1]["yslope"], "yslope differs between detail modes")
    yslope = up["view"][0]["yslope"]
    light = enc_light()
    check(len(colormap) >= COLORMAP_ROWS * 256, "COLORMAP too short")
    t = {"sine": sine, "tan": tan, "atan": atan, "view": view, "yslope": yslope, "light": light,
         "colormap": bytes(colormap[:COLORMAP_ROWS * 256])}
    verify(t, up)
    return t


def verify(t, up):
    fs, ft, tt = up["finesine"], up["finetangent"], up["tantoangle"]
    for i in range(len(fs)):
        if dec_sine(t["sine"], i) != fs[i]:
            raise ConvError("finesine decode mismatch at %d" % i)
        if dec_sine(t["sine"], i, True) != fs[i]:
            raise ConvError("finesine L1 decode mismatch at %d" % i)
    for i in range(4096):
        if dec_tan(t["tan"], i) != ft[i]:
            raise ConvError("finetangent decode mismatch at %d" % i)
    for i in range(2049):
        if dec_atan(t["atan"], i) != u32(tt[i]):
            raise ConvError("tantoangle decode mismatch at %d" % i)
    for v in up["view"]:
        e = t["view"][v["detail"]]
        for i in range(4096):
            if dec_vatx(e["vatx"], i) != v["viewangletox"][i]:
                raise ConvError("viewangletox mismatch detail %d at %d" % (v["detail"], i))
        for x in range(v["viewwidth"] + 1):
            if dec_xtva(e["xtva"], x) != u32(v["xtoviewangle"][x]):
                raise ConvError("xtoviewangle mismatch")
        for i in range(LIGHTLEVELS):
            for j in range(MAXLIGHTSCALE):
                if dec_scalelight(t["light"], i, j) != v["scalelight"][i * MAXLIGHTSCALE + j]:
                    raise ConvError("scalelight mismatch")
    for i in range(LIGHTLEVELS):
        for j in range(MAXLIGHTZ):
            if dec_zlight(t["light"], i, j) != up["zlight"][i * MAXLIGHTZ + j]:
                raise ConvError("zlight mismatch")


def _arr(ctype, name, vals, per=12, fmt="%d"):
    out = ["const %s %s[%d] = {" % (ctype, name, len(vals))]
    for k in range(0, len(vals), per):
        out.append("    " + ", ".join(fmt % v for v in vals[k:k + per]) + ",")
    out.append("};")
    return out


def emit(t, source=None):
    """Return ({file name: text}, sizes). source names a local IWAD, None is Freedoom."""
    s, tn, at, li = t["sine"], t["tan"], t["atan"], t["light"]
    v0, v1 = t["view"][0], t["view"][1]
    hdr = ["// SPDX-License-Identifier: GPL-2.0-or-later",
           "// Copyright (C) 2026 Artur Bieniek",
           "// Generated by tools/conv/trig.py. Do not edit.",
           "", "#ifndef DMC_TRIG_TABLES_H", "#define DMC_TRIG_TABLES_H", "", "#include <stdint.h>", "",
           "#define DMC_SINE_FIX_BLOCK %d" % SINE_FIX_BLOCK,
           "#define DMC_SINE_NFIX %d" % len(s["fix_idx"]),
           "#define DMC_SINE_NMAP %d" % len(s["fix_map"]),
           "#define DMC_SINE_SHIFT %d" % SINE_SHIFT,
           "#define DMC_SINE_NCOARSE %d" % len(s["l1"]["coarse"]),
           "#define DMC_TAN_FULL %d" % TAN_FULL,
           "#define DMC_TAN_NCOARSE %d" % len(tn["coarse"]),
           "#define DMC_TAN_NRESID %d" % len(tn["resid"]),
           "#define DMC_ATAN_BLOCK %d" % ATAN_BLOCK,
           "#define DMC_ATAN_SHIFT %d" % ATAN_SHIFT,
           "#define DMC_ATAN_NBLOCKS %d" % len(at["base"]),
           "#define DMC_VATX_LO_HIGH %d" % v0["vatx"]["lo"],
           "#define DMC_VATX_HI_HIGH %d" % v0["vatx"]["hi"],
           "#define DMC_VATX_NW_HIGH %d" % len(v0["vatx"]["words"]),
           "#define DMC_VATX_LO_LOW %d" % v1["vatx"]["lo"],
           "#define DMC_VATX_HI_LOW %d" % v1["vatx"]["hi"],
           "#define DMC_VATX_NW_LOW %d" % len(v1["vatx"]["words"]),
           "#define DMC_COLORMAP_ROWS %d" % COLORMAP_ROWS,
           "",
           "extern const uint16_t dmc_sine_q[2048];",
           "extern const uint32_t dmc_sine_fix_map[DMC_SINE_NMAP];",
           "extern const uint16_t dmc_sine_fix_idx[DMC_SINE_NFIX];",
           "extern const int32_t dmc_sine_fix_val[DMC_SINE_NFIX];",
           "// compact sine table (DMC_SINE_L1): every 16th entry of dmc_sine_q and 2-bit residuals",
           "extern const uint16_t dmc_sine_coarse[DMC_SINE_NCOARSE];",
           "extern const uint8_t dmc_sine_resid[512];",
           "extern const int32_t dmc_tan_full[DMC_TAN_FULL];",
           "extern const int32_t dmc_tan_coarse[DMC_TAN_NCOARSE];",
           "extern const uint8_t dmc_tan_resid[DMC_TAN_NRESID];",
           "extern const uint32_t dmc_atan_base[DMC_ATAN_NBLOCKS];",
           "extern const int32_t dmc_atan_a[DMC_ATAN_NBLOCKS];",
           "extern const int32_t dmc_atan_c[DMC_ATAN_NBLOCKS];",
           "extern const int8_t dmc_atan_resid[2049];",
           "extern const uint32_t dmc_vatx_words_high[DMC_VATX_NW_HIGH];",
           "extern const uint16_t dmc_vatx_prefix_high[DMC_VATX_NW_HIGH];",
           "extern const uint32_t dmc_vatx_words_low[DMC_VATX_NW_LOW];",
           "extern const uint16_t dmc_vatx_prefix_low[DMC_VATX_NW_LOW];",
           "extern const uint16_t dmc_xtva_high[321];",
           "extern const uint16_t dmc_xtva_low[161];",
           "extern const int32_t dmc_yslope_tab[200];",
           "extern const uint8_t dmc_light_dist[128];",
           "extern const uint8_t dmc_light_startmap_tab[16];",
           "extern const uint8_t dmc_colormap[DMC_COLORMAP_ROWS * 256];",
           "", "#endif", ""]
    def head(what):
        return ["// SPDX-License-Identifier: GPL-2.0-or-later",
                "// Copyright (C) 1993-1996 Id Software, Inc.",
                "// Copyright (C) 2005-2014 Simon Howard",
                "// Copyright (C) 2026 Artur Bieniek",
                "// Generated by tools/conv/trig.py from %s. Do not edit." % what,
                "", "#include \"trig_tables.h\"", ""]
    c = head("the upstream tables.c values")
    c += _arr("uint16_t", "dmc_sine_q", s["q"], 16)
    c += _arr("uint32_t", "dmc_sine_fix_map", s["fix_map"], 8, "0x%08xu")
    c += _arr("uint16_t", "dmc_sine_fix_idx", s["fix_idx"], 12)
    c += _arr("int32_t", "dmc_sine_fix_val", s["fix_val"], 12)
    c += _arr("uint16_t", "dmc_sine_coarse", s["l1"]["coarse"], 16)
    c += _arr("uint8_t", "dmc_sine_resid", list(s["l1"]["resid"]), 16, "0x%02x")
    c += _arr("int32_t", "dmc_tan_full", [s32(v) for v in tn["full"]], 8)
    c += _arr("int32_t", "dmc_tan_coarse", tn["coarse"], 8)
    c += _arr("uint8_t", "dmc_tan_resid", list(tn["resid"]), 16, "0x%02x")
    c += _arr("uint32_t", "dmc_atan_base", at["base"], 8, "0x%08xu")
    c += _arr("int32_t", "dmc_atan_a", at["a"], 8)
    c += _arr("int32_t", "dmc_atan_c", at["c"], 8)
    c += _arr("int8_t", "dmc_atan_resid", at["resid"], 16)
    v = head("the upstream R_ExecuteSetViewSize and R_InitLightTables results")
    v += _arr("uint32_t", "dmc_vatx_words_high", v0["vatx"]["words"], 8, "0x%08xu")
    v += _arr("uint16_t", "dmc_vatx_prefix_high", v0["vatx"]["prefix"], 16)
    v += _arr("uint32_t", "dmc_vatx_words_low", v1["vatx"]["words"], 8, "0x%08xu")
    v += _arr("uint16_t", "dmc_vatx_prefix_low", v1["vatx"]["prefix"], 16)
    v += _arr("uint16_t", "dmc_xtva_high", v0["xtva"], 16)
    v += _arr("uint16_t", "dmc_xtva_low", v1["xtva"], 16)
    v += _arr("int32_t", "dmc_yslope_tab", t["yslope"], 8)
    v += _arr("uint8_t", "dmc_light_dist", li["dist"], 16)
    v += _arr("uint8_t", "dmc_light_startmap_tab", li["startmap"], 16)
    if source is None:
        m = ["// SPDX-License-Identifier: BSD-3-Clause",
             "// Copyright (c) 2001-2024 Contributors to the Freedoom project. All rights reserved.",
             "// Generated by tools/conv/trig.py from the Freedoom COLORMAP lump. Do not edit."]
    else:
        m = ["// Generated by tools/conv/trig.py from the COLORMAP lump of %s. Do not edit." % source]
    m += ["", "#include \"trig_tables.h\"", ""]
    m += _arr("uint8_t", "dmc_colormap", list(t["colormap"]), 16, "0x%02x")
    sizes = {
        "finesine": 2 * 2048 + 4 * len(s["fix_map"]) + 6 * len(s["fix_idx"]),
        "finesine_l1": 2 * len(s["l1"]["coarse"]) + 512 + 4 * len(s["fix_map"]) + 6 * len(s["fix_idx"]),
        "finetangent": 4 * TAN_FULL + 4 * len(tn["coarse"]) + len(tn["resid"]),
        "tantoangle": 12 * len(at["base"]) + 2049,
        "viewangletox": 6 * (len(v0["vatx"]["words"]) + len(v1["vatx"]["words"])),
        "xtoviewangle": 2 * (321 + 161),
        "yslope": 800,
        "light": 128 + 16,
        "colormap": len(t["colormap"]),
    }
    files = {"trig_tables.h": hdr, "trig_tables.c": c, "view.c": v, "colormap.c": m}
    return {k: "\n".join(x + [""]) for k, x in files.items()}, sizes
