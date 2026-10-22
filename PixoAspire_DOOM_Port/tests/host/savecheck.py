#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""The writable globals of the host engine that a save leaves out: a review aid, not a gate.

A save (port/game/g_save.c) holds the level arena and the globals of SAVE_VARS. Every other
writable data symbol of the objects of the host link (llvm-nm) has to be set up again by the
level setup of a load, live within one tic or call, or be no game state. The objects of the
display, the front end, the platform and data layer, the sound, the statistics and the test
harness (OBJECTS) are left out whole, the symbols of the game's objects that are known to be
one of those (ALLOW) by name. The rest is printed: globals a save may miss, to look at when
globals change. Pass or fail rests on the round trips of savetest.py and savemenu.py, the
exit status is 0.

usage: savecheck.py [--objs DIR]
"""
import argparse
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
WRITABLE = 'DdBbC'

# Objects left out whole: the display, the front end, the platform and data layer, the sound,
# the statistics and the test harness
OBJECTS = re.compile(r'^(r_\w+|am_map|f_finale|hu_\w+|i_video_host|m_menu|png|rview|st_\w+|v_video'
                     r'|wi_stuff|fe_glue|l_\w+|cache|extents|fat12|fat_write|fetch|img|img_check'
                     r'|m_argv|main_host|nor_file|ovl|rec|z_zone|d_event|m_controls|dstrings'
                     r'|d_items|info|s_sound|sounds|attr|stats|trig|hash|hash_skip|play|keys)\.o$')

ALLOW = {
    # made by the level setup of the saved map and skill, which a load runs first, or pointers
    # into the level arena it places
    'level setup': '''M bmapwidth bmapheight bmaporgx bmaporgy numlines numnodes numsectors numsegs
        numsides numsubsectors scrollsides numscrollsides secstat secovl linevalid linemapped
        specialcleared switchflip buttonlist activeplats activeceilings lights movers o_pages
        o_tier o_sleep o_monbits o_zclip o_drops o_bhash o_shash o_arena_words totalkills
        totalitems totalsecret respawnmonsters gameepisode gamemap gameskill gamestate
        levelstarttic wminfo''',
    # the run: its options, the tic loop and the demo, set by the load itself (gametic and the
    # demo cursor of the test loads, wipegamestate and oldgamestate), the creation order block
    # and the store options of the header. No save is taken while a game action waits, so
    # secretexit is never read after a load.
    'the run': '''gametic maketic lasttime netcmds ticdata singletics TryRunTics.oldentertics
        consoleplayer displayplayer playeringame demobuf demofile demoend demobufoff demo_p
        defdemoname singledemo timingdemo starttime longtics precache d_episode d_map d_skill
        sendpause turbodetected G_Ticker.turbomessage turnheld gamekeydown viewactive paused
        oldgamestate wipegamestate gameaction secretexit devparm fastparm nomonsters
        respawnparm startepisode startmap startskill autostart advancedemo demosequence pagepic
        pagetic tempscreen D_Display.inhelpscreensstate D_Display.oldgamestate o_seq_page
        o_seq_drop o_seqnext o_arena_tail o_keep_corpses o_promote_all o_reserve o_mover_cap
        o_page_cap o_stats o_statsfile o_detached dmc_tic''',
    # within one tic or one call
    'transient': '''bottomslope sightzstart strace t2x t2y topslope earlyout intercepts_overrun
        numintercepts usething bulletslope sndcount sndqueued sndsweep P_String.buf
        nextsoundorg soundorgsec soundorgxy lightthink moverthink''',
}


def save_vars():
    """The names of SAVE_VARS in g_save.c."""
    with open(os.path.join(ROOT, 'port', 'game', 'g_save.c')) as f:
        src = f.read()
    m = re.search(r'#define SAVE_VARS\(X\)((?:.*\\\n)*.*)', src)
    return set(re.findall(r'X\((\w+)', m.group(1))) if m else set()


def symbols(objs):
    """(object, symbol) of every writable data symbol the objects define."""
    out = []
    for name in sorted(os.listdir(objs)):
        if not name.endswith('.o'):
            continue
        r = subprocess.run(['llvm-nm', '--defined-only', os.path.join(objs, name)],
                           capture_output=True, text=True)
        for line in r.stdout.splitlines():
            w = line.split()
            if len(w) == 3 and w[1] in WRITABLE:
                out.append((name, w[2]))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--objs', default=os.path.join(ROOT, 'build', 'host', 'opt'))
    a = ap.parse_args()
    saved = save_vars()
    allow = {s for v in ALLOW.values() for s in v.split()}
    try:
        syms = symbols(a.objs)
    except (OSError, subprocess.SubprocessError) as e:
        print('savecheck: no symbols (%s), nothing to report' % e)
        return
    rest = [(o, s) for o, s in syms if not OBJECTS.match(o) and s not in saved and s not in allow]
    nsaved = sum(s in saved for _, s in syms)
    print('savecheck: %d writable symbols, %d in SAVE_VARS, %d left out by the allowlist, %d to '
          'look at' % (len(syms), nsaved, len(syms) - nsaved - len(rest), len(rest)))
    for o, s in rest:
        print('  %-16s %s' % (o, s))
    missing = sorted(saved - {s for _, s in syms})
    if missing:
        print('  SAVE_VARS names no object defines: %s' % ' '.join(missing))


if __name__ == '__main__':
    sys.dont_write_bytecode = True
    main()
