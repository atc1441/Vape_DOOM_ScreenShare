#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Import the engine's upstream files from the pinned doomgeneric checkout.

Copies a fixed list of files from oracle/src/doomgeneric-repo/doomgeneric
into engine/ and writes engine/UPSTREAM.txt with the commit and the SHA-256
of every file as imported. Files in engine/ that differ from upstream are
local modifications, and files listed in an existing UPSTREAM.txt but
missing from engine/ were deleted on purpose. Both are left alone unless
--force is given.

usage: import_upstream.py [--force] [--check] [--dest DIR]
  --check  only verify that UPSTREAM.txt matches the oracle checkout
  --dest   import into DIR instead of engine/ (for example an empty
           directory, to get the files as upstream has them)
"""
import hashlib
import os
import subprocess
import sys

COMMIT = 'dcb7a8dbc7a16ce3dda29382ac9aae9d77d21284'
URL = 'https://github.com/ozkl/doomgeneric.git'
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REPO = os.path.join(ROOT, 'oracle', 'src', 'doomgeneric-repo')
SRC = os.path.join(REPO, 'doomgeneric')
DST = os.path.join(ROOT, 'engine')

FILES = '''
am_map.c am_map.h d_englsh.h d_event.c d_event.h d_items.c d_items.h d_loop.c d_loop.h
d_main.c d_main.h d_mode.h d_player.h d_think.h d_ticcmd.h deh_main.h
deh_misc.h deh_str.h doomdata.h doomdef.c doomdef.h doomfeatures.h doomkeys.h doomstat.c
doomstat.h doomtype.h dstrings.c dstrings.h f_finale.c f_finale.h g_game.c g_game.h
hu_lib.c hu_lib.h hu_stuff.c hu_stuff.h i_sound.h i_swap.h i_system.h i_timer.h i_video.h
info.c info.h m_argv.c m_argv.h m_bbox.c m_bbox.h m_controls.c m_controls.h m_fixed.c
m_fixed.h m_menu.c m_menu.h m_misc.c m_misc.h m_random.c m_random.h p_ceilng.c p_doors.c
p_enemy.c p_floor.c p_inter.c p_inter.h p_lights.c p_local.h p_map.c p_maputl.c p_mobj.c
p_mobj.h p_plats.c p_pspr.c p_pspr.h p_setup.c p_setup.h p_sight.c p_spec.c p_spec.h
p_switch.c p_telept.c p_tick.c p_tick.h p_user.c r_bsp.c r_bsp.h r_data.c r_data.h r_defs.h
r_draw.c r_draw.h r_local.h r_main.c r_main.h r_plane.c r_plane.h r_segs.c r_segs.h r_sky.c
r_sky.h r_state.h r_things.c r_things.h s_sound.c s_sound.h sounds.c sounds.h st_lib.c
st_lib.h st_stuff.c st_stuff.h tables.c tables.h v_patch.h v_video.c v_video.h w_file.c
w_file.h w_wad.c w_wad.h wi_stuff.c wi_stuff.h z_zone.c z_zone.h
'''.split()


def sha256(path):
    with open(path, 'rb') as f:
        return hashlib.sha256(f.read()).hexdigest()


def head_commit():
    r = subprocess.run(['git', '-C', REPO, 'rev-parse', 'HEAD'], capture_output=True, text=True)
    return r.stdout.strip() if r.returncode == 0 else None


def check():
    path = os.path.join(DST, 'UPSTREAM.txt')
    bad = 0
    with open(path) as f:
        for line in f:
            if not line.strip() or line.startswith('#'):
                continue
            digest, name = line.split()
            if sha256(os.path.join(SRC, name)) != digest:
                print('mismatch', name)
                bad += 1
    print('UPSTREAM.txt: %s' % ('ok' if not bad else '%d mismatches' % bad))
    return 1 if bad else 0


def listed():
    path = os.path.join(DST, 'UPSTREAM.txt')
    if not os.path.exists(path):
        return set()
    with open(path) as f:
        return {line.split()[1] for line in f if line.strip() and not line.startswith('#')}


def main():
    global DST
    force = '--force' in sys.argv
    if '--dest' in sys.argv:
        DST = os.path.abspath(sys.argv[sys.argv.index('--dest') + 1])
    if head_commit() != COMMIT:
        sys.exit('oracle checkout is not at %s, run oracle/fetch.sh' % COMMIT)
    if '--check' in sys.argv:
        sys.exit(check())
    os.makedirs(DST, exist_ok=True)
    before = listed()
    copied = kept = same = deleted = 0
    lines = ['# doomgeneric files imported into engine/',
             '# repository %s' % URL, '# commit %s' % COMMIT,
             '# sha256 of each file as imported, then its path', '']
    for name in sorted(FILES):
        src = os.path.join(SRC, name)
        dst = os.path.join(DST, name)
        with open(src, 'rb') as f:
            data = f.read()
        lines.append('%s  %s' % (hashlib.sha256(data).hexdigest(), name))
        if os.path.exists(dst):
            with open(dst, 'rb') as f:
                if f.read() == data:
                    same += 1
                    continue
            if not force:
                kept += 1
                continue
        elif name in before and not force:
            print('deleted, kept deleted: %s' % name)
            deleted += 1
            continue
        with open(dst, 'wb') as f:
            f.write(data)
        copied += 1
    with open(os.path.join(DST, 'UPSTREAM.txt'), 'w') as f:
        f.write('\n'.join(lines) + '\n')
    print('imported %d, unchanged %d, locally modified and kept %d, deleted and kept deleted %d'
          % (copied, same, kept, deleted))


if __name__ == '__main__':
    main()
