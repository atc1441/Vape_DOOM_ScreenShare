#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Size report and gates G0, G1 and G2 for the ARM -Oz build of the engine.

Reads the ld.lld map file of build/arm/engine/engine.elf, adds up the input
sections of every object file per output section (.text, .rodata, .data,
.bss) and groups the modules. Gate G0 limits the .text of three groups of
the trimmed upstream code (p_*, main loop, st/hu), gate G2 the renderer with
port/render (17.3 KB) and its static RAM (.data + .bss), which leaves out the
frame region at FRAME_BASE.
Exits with 1 when a gate fails.

The map data accessors (engine/p_mapdata.c) and the data layer (port/data)
are groups of their own. G0 measures the trimmed upstream game logic, so its
p_* group leaves the accessors out. G1 limits the .text of the game logic:
p_* (without p_setup), p_mapdata and the object store port/obj. The level
start code (p_setup, port/obj/level.c) is LEVEL overlay code.

G3 is an estimate of the resident flash of the firmware from this link:
everything but the overlay-destined code (LEVEL, MENU, WI, FINALE, AUTOMAP,
by module or by function) and the stub platform, with the alignment padding
in front of each resident input section, plus the overlay loader (--loader,
its objects as the firmware builds them), the resident objects outside this
link (--resident, the input and sound modules of port/game), the platform
footprint of mk/platform.mk (--platform, startup, vectors, fault screen, the
mem* functions it uses and font included) and the runtime line. The other
firmware code of port/game (boot, installer, demos, results) is not counted.
The merged strings count as resident unless only objects of overlay modules
have them, merged constants count as resident. G3 is reported against the
101,376 B code area of the app slot but does not fail here: the firmware
link fails above the code area, and tools/fwsize.py reports the linked
image. This estimate counts code that the firmware keeps in its other
overlays (for example the line specials of LINES and the start-up code of
BOOT) as resident, so it can be higher than the linked image
(build/fwdoom/meter/size.txt).

The runtime line holds the C library functions this link takes from the
stub libc and the functions the loader and resident objects call that
neither this link nor the footprint link (--footprint, its ELF) holds. Each
counts with its section in the platform objects (--runtime, platform/rt.c
and text.c, where vsnprintf is the formatter plat_vfmt) or else in the
objects of this link (code its --gc-sections dropped), with everything that
section refers to and the link does not keep, or else with the stub's plain
implementation in this link.

usage: sizegate.py MAP [--modules] [--symbols N] [--json FILE] [--platform FOOTPRINT]
                   [--loader OBJ]... [--resident OBJ]... [--footprint ELF] [--runtime OBJ]...
"""
import json
import os
import re
import struct
import sys
from collections import defaultdict

KB = 1024
SECTIONS = ('text', 'rodata', 'data', 'bss')

# (group, predicate on the module base name)
GROUPS = [
    ('game logic p_* (no p_setup, p_saveg, p_mapdata)',
     lambda m: m.startswith('p_') and m not in ('p_setup', 'p_saveg', 'p_mapdata')),
    ('level setup p_setup, port/obj/level', lambda m: m in ('p_setup', 'level')),
    ('renderer r_*, v_video, port/render', lambda m: m.startswith('r_') or m == 'v_video'),
    ('main loop d_*, g_game, s_sound',
     lambda m: (m.startswith('d_') and m != 'd_items') or m in ('g_game', 's_sound')),
    ('status bar and HUD st_*, hu_*', lambda m: m.startswith(('st_', 'hu_'))),
    ('front end m_menu, wi_stuff, f_finale, am_map',
     lambda m: m in ('m_menu', 'wi_stuff', 'f_finale', 'am_map')),
    ('tables info, tables, sounds, d_items',
     lambda m: m in ('info', 'tables', 'sounds', 'd_items')),
    ('wad and zone w_*, z_zone', lambda m: m.startswith('w_') or m == 'z_zone'),
    ('misc m_*, doomdef, doomstat, dstrings',
     lambda m: m.startswith('m_') or m in ('doomdef', 'doomstat', 'dstrings')),
    ('stub platform and libc', lambda m: m.startswith('stub_')),
    ('map accessors p_mapdata', lambda m: m == 'p_mapdata'),
    ('object store port/obj', lambda m: m in ('things', 'pages', 'chains', 'movers', 'lights', 'rview')),
    ('data layer port/data', lambda m: m in ('cache', 'crc32', 'extents', 'fat12', 'fetch',
                                             'img', 'rec')),
    ('generated gen/ (colormap, dense ids, tables)', lambda m: m.startswith('gen_')),
    ('table decoders port/math', lambda m: m == 'trig'),
    ('merged constants and strings', lambda m: m == '<internal>'),
]

# Gates G0 and G2: (name, group index, .text limit in bytes). The renderer
# group holds the strip renderer of port/render (r_frame, r_band, r_inner,
# the last built -O2) with the engine's r_* and v_video.
GATES = [
    ('G0 p_*', 0, int(27.0 * KB)),
    ('G0 main loop', 3, int(6.0 * KB)),
    ('G0 st/hu', 4, int(5.2 * KB)),
    ('G2 renderer', 2, int(17.3 * KB)),
]

# Static RAM of the renderer outside the frame region: the r_* globals, the
# NOR column buffers (two 256 B masked buffers that also hold the two 128 B
# wall buffers), the map record views, the pointers into the frame region
# and the status band compositor.
RAM_GATES = [
    ('G2 renderer RAM', 2, 1600),
]

# Gate G1: game logic with the accessors and the object store.
G1_LIMIT = int(33.5 * KB)
G1_GROUPS = ('game logic p_* (no p_setup, p_saveg, p_mapdata)', 'map accessors p_mapdata',
             'object store port/obj')

# G3: code area of the app slot
CODE_AREA = 101376

# Overlay-destined code. LEVEL takes the level start and
# end of g_game, the demo setup and the status bar and HUD widget init too.
LEVEL_FUNCS = {
    'G_InitNew', 'G_DeferedInitNew', 'G_DoNewGame', 'G_DoLoadLevel', 'G_DoCompleted',
    'G_WorldDone', 'G_DoWorldDone', 'G_DoPlayDemo', 'G_DeferedPlayDemo', 'G_TimeDemo',
    'ST_Start', 'ST_Stop', 'ST_initData', 'ST_createWidgets', 'ST_loadGraphics', 'STlib_init',
    'STlib_initNum', 'STlib_initPercent', 'STlib_initMultIcon', 'STlib_initBinIcon', 'HU_Start',
    'HUlib_initTextLine', 'HUlib_initSText', 'P_SpawnSpecials',
}
OVERLAYS = [
    ('LEVEL', lambda mod, sym: mod in ('p_setup', 'level') or sym in LEVEL_FUNCS),
    ('MENU', lambda mod, sym: mod == 'm_menu'),
    ('WI', lambda mod, sym: mod == 'wi_stuff'),
    ('FINALE', lambda mod, sym: mod == 'f_finale'),
    ('AUTOMAP', lambda mod, sym: mod == 'am_map'),
]
# Runtime functions that the platform provides under another name. The C
# function stays as a call to it, counted with the stub's size.
RUNTIME_ALIAS = {'vsnprintf': 'plat_vfmt'}
SHT_ARM_EXIDX = 0x70000001

LINE = re.compile(r'^\s*([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+(\d+) (.*)$')
ASSIGN = re.compile(r'^(?:PROVIDE\()?(\w+) = ')


def parse_map(path):
    """Sizes per module and output section and the input sections as [size,
    output section, module, symbol, alignment padding in front of it]."""
    per_mod = defaultdict(lambda: dict.fromkeys(SECTIONS, 0))
    syms = []
    out = None
    last = None
    with open(path) as f:
        for line in f:
            m = LINE.match(line.rstrip('\n'))
            if not m:
                continue
            addr = int(m.group(1), 16)
            size = int(m.group(3), 16)
            rest = m.group(5)
            indent = len(rest) - len(rest.lstrip(' '))
            rest = rest.strip()
            if indent == 0:
                if out and syms:
                    syms[-1][4] += out_end - end
                name = rest.lstrip('.')
                out = name if name in SECTIONS else None
                end = addr
                out_end = addr + size
                last = None
            elif indent == 8 and out:
                obj = rest.split(':(')[0]
                mod = os.path.basename(obj)
                mod = mod[:-2] if mod.endswith('.o') else mod
                per_mod[mod][out] += size
                last = [size, out, mod, rest.split(':(')[-1].rstrip(')'), addr - end]
                end = addr + size
                syms.append(last)
            elif indent == 16 and last and not rest.startswith('$'):
                last[3] = rest
    return per_mod, syms


def map_objects(path):
    """Object files of the link, the (object, section) pairs it keeps and the
    symbols defined outside the stub platform."""
    kept = set()
    defined = set()
    stub = False
    with open(path) as f:
        for line in f:
            m = LINE.match(line.rstrip('\n'))
            if not m:
                continue
            rest = m.group(5)
            indent = len(rest) - len(rest.lstrip(' '))
            rest = rest.strip()
            a = ASSIGN.match(rest)
            if indent == 0 and a:
                defined.add(a.group(1))
            elif indent == 8 and ':(' in rest:
                obj, sec = rest.split(':(')
                stub = os.path.basename(obj).startswith('stub_')
                if obj.endswith('.o'):
                    kept.add((obj, sec.rstrip(')')))
            elif indent == 16 and not stub and not rest.startswith('$'):
                defined.add(rest)
    return sorted({o for o, _ in kept}), kept, defined


def elf_sections(path):
    """Contents and (type, flags, offset, size, entsize) of the sections of an ELF32 file."""
    with open(path, 'rb') as f:
        b = f.read()
    shoff, = struct.unpack_from('<I', b, 0x20)
    shentsize, shnum = struct.unpack_from('<HH', b, 0x2e)
    return b, [struct.unpack_from('<4xII4xII12xI', b, shoff + i * shentsize) for i in range(shnum)]


def elf_strings(path):
    """Strings of the mergeable string sections of an object."""
    b, secs = elf_sections(path)
    out = set()
    for stype, flags, off, size, ent in secs:
        if flags & 0x30 == 0x30 and ent == 1:
            out.update(b[off:off + size].split(b'\0')[:-1])
    return out


def elf_sizes(path):
    """.text, .rodata, .data and .bss bytes of the allocated sections of an ELF32 file
    (the unwind index is discarded by the firmware link)."""
    out = dict.fromkeys(SECTIONS, 0)
    for stype, flags, _, size, _ in elf_sections(path)[1]:
        if not flags & 2 or stype == SHT_ARM_EXIDX:
            continue
        if flags & 4:
            out['text'] += size
        elif stype == 8:
            out['bss'] += size
        elif flags & 1:
            out['data'] += size
        else:
            out['rodata'] += size
    return out


def elf_symbols(path):
    """Section names, flash bytes per section (allocated, not .bss, not the
    unwind index), the global symbols {name: section index} the file defines,
    the names it leaves undefined and, per section, the symbols its relocations
    refer to as (name, binding, section index)."""
    with open(path, 'rb') as f:
        b = f.read()
    shoff, = struct.unpack_from('<I', b, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from('<HHH', b, 0x2e)
    sh = [struct.unpack_from('<10I', b, shoff + i * shentsize) for i in range(shnum)]

    def cstr(off):
        return b[off:b.index(b'\0', off)].decode()

    names = [cstr(sh[shstrndx][4] + s[0]) for s in sh]
    size = [s[5] if s[2] & 2 and s[1] not in (8, SHT_ARM_EXIDX) else 0 for s in sh]
    syms = []
    for s in sh:
        if s[1] == 2:
            for off in range(s[4] + 16, s[4] + s[5], 16):
                name, _, _, info, _, ndx = struct.unpack_from('<IIIBBH', b, off)
                syms.append((cstr(sh[s[6]][4] + name), info >> 4, ndx if ndx < 0xff00 else 0))
    defs = {n: ndx for n, bind, ndx in syms if bind and ndx}
    undef = {n for n, bind, ndx in syms if bind and not ndx}
    refs = defaultdict(set)
    for s in sh:
        if s[1] == 9:
            for off in range(s[4], s[4] + s[5], 8):
                i = struct.unpack_from('<I', b, off + 4)[0] >> 8
                if i:
                    refs[s[7]].add(syms[i - 1])
    return names, size, defs, undef, refs


def runtime_sizes(need, have, objs, kept, stub):
    """Bytes of each symbol in need that is not in have: its section in objs
    with all the sections it refers to that the link does not keep already
    (kept: (object, section) pairs), the global symbols those name taken the
    same way, or else the stub's size. Symbols nothing defines count as None."""
    info = [elf_symbols(o) for o in objs]
    taken = set(have)
    done = {(i, k) for i, o in enumerate(objs) for k, n in enumerate(info[i][0]) if (o, n) in kept}

    def take(sym):
        if sym in taken:
            return 0
        taken.add(sym)
        for i, (_, size, defs, _, refs) in enumerate(info):
            if sym not in defs:
                continue
            n = 0
            todo = [defs[sym]]
            while todo:
                sec = todo.pop()
                if (i, sec) in done:
                    continue
                done.add((i, sec))
                n += size[sec]
                for name, bind, ndx in refs[sec]:
                    if bind:
                        n += take(name) or 0
                    elif ndx:
                        todo.append(ndx)
            return n
        return stub.get(sym)

    rows = []
    for sym in sorted(need - set(have)):
        if sym in RUNTIME_ALIAS:
            n = take(RUNTIME_ALIAS[sym])
            rows.append(('%s (%s)' % (sym, RUNTIME_ALIAS[sym]),
                         None if n is None else n + stub.get(sym, 0)))
        else:
            rows.append((sym, take(sym)))
    return rows


def flash(d):
    return d['text'] + d['rodata'] + d['data']


def overlay_of(mod, sym):
    for name, pred in OVERLAYS:
        if pred(mod, sym):
            return name
    return None


def g3_report(syms, path, platform, loaders, opts):
    """Print the resident flash of the firmware (G3) and the overlay code."""
    res = dict.fromkeys(SECTIONS, 0)
    ovl = defaultdict(lambda: dict.fromkeys(SECTIONS, 0))
    stub = 0
    pad = defaultdict(int)
    for size, out, mod, sym, gap in syms:
        if mod.startswith('stub_'):
            stub += size if out != 'bss' else 0
            continue
        o = overlay_of(mod, sym)
        (ovl[o] if o else res)[out] += size
        if out != 'bss':
            pad[o] += gap
    # merged strings that only objects of overlay modules have
    objs, kept, defined = map_objects(path)
    seen, only = set(), {}
    for obj in objs:
        mod = os.path.basename(obj)[:-2]
        o = None if mod.startswith('stub_') else overlay_of(mod, '')
        for st in elf_strings(obj):
            if o:
                only.setdefault(st, o)
            else:
                seen.add(st)
    for st, o in only.items():
        if st not in seen:
            res['rodata'] -= len(st) + 1
            ovl[o]['rodata'] += len(st) + 1
    print()
    print('G3 resident flash (text + rodata + data) and static RAM (data + bss)')
    print('  %-44s %7s %7s' % ('', 'flash', 'RAM'))
    print('  %-44s %7d %7d' % ('engine, resident part of this link', flash(res),
                                 res['data'] + res['bss']))
    for name, _ in OVERLAYS:
        d = ovl[name]
        print('  %-44s %7d %7d  (overlay, not counted)' % (name, flash(d), d['data'] + d['bss']))
    print('  %-44s %7d %7s  (replaced by the platform)' % ('stub platform and libc', stub, ''))
    if platform is None or not loaders or opts['footprint'] is None:
        print('G3: give --platform, --loader and --footprint for the total')
        return None
    total = flash(res)
    load = dict.fromkeys(SECTIONS, 0)
    for obj in loaders:
        for k, v in elf_sizes(obj).items():
            load[k] += v
    rows = [('alignment padding, resident share (of %d B)' % sum(pad.values()), pad[None]),
            ('overlay loader port/ovl (%s)' % ', '.join(os.path.basename(o) for o in loaders),
             flash(load))]
    rows += [('resident %s' % os.path.basename(o), flash(elf_sizes(o))) for o in opts['resident']]

    # runtime: what the stub libc gives this link and the outside objects call
    stub_fn = {}
    for size, out, mod, sym, _ in syms:
        if mod == 'stub_libc' and out == 'text':
            stub_fn[sym] = size
    need = set(stub_fn)
    have = defined | set(elf_symbols(opts['footprint'])[2])
    for o in loaders + opts['resident']:
        need |= elf_symbols(o)[3]
        have |= set(elf_symbols(o)[2])
    # the platform's functions first, then what this link left out of its objects
    run = runtime_sizes(need, have, opts['runtime'] + [
        o for o in objs if not os.path.basename(o).startswith('stub_')], kept, stub_fn)
    rows.append(('runtime and functions this link dropped', sum(n for _, n in run if n)))
    rows.append(('platform footprint (mk/platform.mk)', platform))
    for name, n in rows:
        print('  %-44s %7d' % (name, n))
        total += n
    print('  runtime: %s' % ', '.join('%s %d' % (s, n) for s, n in run if n is not None))
    missing = [s for s, n in run if n is None]
    if missing:
        print('  not counted, no object of this tree defines %s' % ', '.join(missing))
    print('G3 resident estimate %d B, code area %d B: %s %d B (the linked image: tools/fwsize.py)' % (
        total, CODE_AREA, 'free' if total <= CODE_AREA else 'over by', abs(CODE_AREA - total)))
    return total


def group_of(mod):
    for i, (_, pred) in enumerate(GROUPS):
        if pred(mod):
            return i
    return len(GROUPS)


def main():
    args = sys.argv[1:]
    if not args:
        sys.exit(__doc__)
    path = args[0]
    per_mod, syms = parse_map(path)
    totals = [dict.fromkeys(SECTIONS, 0) for _ in range(len(GROUPS) + 1)]
    members = defaultdict(list)
    for mod, d in per_mod.items():
        g = group_of(mod)
        members[g].append(mod)
        for k in SECTIONS:
            totals[g][k] += d[k]
    names = [g for g, _ in GROUPS] + ['other']

    print('ARM -Oz size by module group (bytes, from %s)' % path)
    print('%-46s %8s %8s %8s %8s %8s' % ('group', '.text', '.rodata', '.data', '.bss', 'flash'))
    for i, name in enumerate(names):
        t = totals[i]
        if any(t.values()):
            print('%-46s %8d %8d %8d %8d %8d' % (name, t['text'], t['rodata'], t['data'], t['bss'],
                                                 flash(t)))
    s = {k: sum(t[k] for t in totals) for k in SECTIONS}
    print('%-46s %8d %8d %8d %8d %8d' % ('total', s['text'], s['rodata'], s['data'], s['bss'],
                                         flash(s)))
    print('flash (text + rodata + data) %d, RAM (data + bss) %d' % (
        s['text'] + s['rodata'] + s['data'], s['data'] + s['bss']))

    if '--modules' in args:
        print()
        print('%-16s %8s %8s %8s %8s' % ('module', '.text', '.rodata', '.data', '.bss'))
        for mod in sorted(per_mod, key=lambda m: (group_of(m), m)):
            d = per_mod[mod]
            print('%-16s %8d %8d %8d %8d' % (mod, d['text'], d['rodata'], d['data'], d['bss']))

    if '--symbols' in args:
        n = int(args[args.index('--symbols') + 1])
        print()
        for size, out, mod, sym, _ in sorted(syms, reverse=True)[:n]:
            print('%7d %-7s %-12s %s' % (size, out, mod, sym))

    print()
    ok = True
    gates = []
    for name, g, limit in GATES:
        text = totals[g]['text']
        passed = text <= limit
        ok &= passed
        gates.append({'gate': name, 'text': text, 'limit': limit, 'pass': passed})
        print('%-18s .text %6d / %6d (%5.2f / %5.2f KB)  %s %d B' % (
            name, text, limit, text / KB, limit / KB,
            'PASS, margin' if passed else 'FAIL, over by', abs(limit - text)))

    for name, g, limit in RAM_GATES:
        ram = totals[g]['data'] + totals[g]['bss']
        passed = ram <= limit
        ok &= passed
        gates.append({'gate': name, 'ram': ram, 'limit': limit, 'pass': passed})
        print('%-18s .data + .bss %5d / %5d  %s %d B' % (
            name, ram, limit, 'PASS, margin' if passed else 'FAIL, over by', abs(limit - ram)))

    # G1: p_* + p_mapdata + port/obj
    g1 = sum(totals[names.index(n)]['text'] for n in G1_GROUPS)
    passed = g1 <= G1_LIMIT
    ok &= passed
    gates.append({'gate': 'G1 game logic', 'text': g1, 'limit': G1_LIMIT, 'pass': passed})
    print('%-18s .text %6d / %6d (%5.2f / %5.2f KB)  %s %d B  (p_* %d, p_mapdata %d, port/obj %d)' % (
        'G1 game logic', g1, G1_LIMIT, g1 / KB, G1_LIMIT / KB,
        'PASS, margin' if passed else 'FAIL, over by', abs(G1_LIMIT - g1),
        *(totals[names.index(n)]['text'] for n in G1_GROUPS)))

    platform = None
    if '--platform' in args:
        with open(args[args.index('--platform') + 1]) as f:
            platform = int(re.search(r'platform footprint (\d+) B', f.read()).group(1))
    loaders = [args[i + 1] for i, a in enumerate(args) if a == '--loader']
    opts = {'resident': [args[i + 1] for i, a in enumerate(args) if a == '--resident'],
            'runtime': [args[i + 1] for i, a in enumerate(args) if a == '--runtime'],
            'footprint': args[args.index('--footprint') + 1] if '--footprint' in args else None}
    g3 = g3_report(syms, path, platform, loaders, opts)

    if '--json' in args:
        with open(args[args.index('--json') + 1], 'w') as f:
            json.dump({'groups': {names[i]: totals[i] for i in range(len(names))},
                       'modules': per_mod, 'gates': gates, 'g3': g3}, f, indent=1)
    print('size gates G0, G1, G2: %s' % ('PASS' if ok else 'FAIL'))
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
