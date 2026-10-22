// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Host driver for the engine: the I_* system interface on stdio, headless.
// The clock only advances when the game sleeps and singletics is set, so
// every frame runs exactly one tic, as in the oracle runner.
//
// Options (removed before the rest reaches the engine):
//   --hash-out FILE         per-tic hash stream (tests/lockstep/hash.h)
//   --hash-dump TIC FILE    text dump of the hashed values of one tic
//   --cmds FILE             scripted ticcmd stream instead of the keyboard
//   --tics N                stop after N tics
//   --exit-at TIC           force a level exit (G_ExitLevel) before tic TIC
//   --exit-list L1,L2,L3s   forced level exits by level time (play.h)
//   --keys FILE             key events by tic (tests/lockstep/keys.h)
//   --pad FILE              game pad states by tic (port/game/pad.h), one per line:
//                           TIC LX LY RX RY DIAL BUTTONS, held from the I_StartTic
//                           of gametic TIC to the next line. BUTTONS are the PAD_*
//                           bits (C syntax, 0x.. too), pad_set adds PAD_PRESENT,
//                           # starts a comment. The chord resolver of the meter
//                           (port/game/i_input.c) runs on every tic then, with its
//                           defaults (always-run on), and posts the pad's keys
//   --hash-no-gametic       gametic hashed as 0 (hash.h)
//   --nor FILE              4 MB NOR image of the meter (tests/host/mknor.py)
//   --nor-out FILE          the NOR image as the run leaves it, written at exit
//   --nor-fail-after N      the power goes before the NOR erase or program after
//                           the N-th: the image is written as it is (--nor-out)
//                           and the run ends with status 3
//   --save-at TIC SLOT      the test save of the meter: after the display of tic
//                           TIC the game into SAVE.DAT slot SLOT (0..3), up to 4
//   --load SLOT             the test load: the game set up by the arguments (and
//                           a demo's first bytes), then the slot over it, gametic
//                           and a demo's cursor too
//   --stats-out FILE        NOR traffic per tic and frame (host/stats.h)
//   --rc2                   a second level of the record cache during the thinkers of
//                           each tic, off from the first LINES entry past the tests of
//                           other things (host/stats.h)
//   --rc2-all               the same, off from the first LINES entry
//   --nor-defer             async NOR reads complete only when waited for
//   --column-check          the renderer's column contexts against the data layer's
//                           own requests, its cached fragment setups against the
//                           setups computed again (I_Error on a difference)
//   --verify                check every section and map CRC at startup
//   --render-every N        draw the 3D view on every N-th tic only (statistics)
//   --low-detail            low detail (160 columns doubled), the device default
//   -playdemo NAME          DEMOn of the image, or a vanilla .lmp file when NAME
//                           has a dot
//   display and renderer options: host/i_video_host.c (R_HostOption)
//   --promote-all           every map thing a full object, corpses and drops too
//                           (object store debug mode, port/obj)
//   --keep-corpses          corpses at rest stay full objects, the rest as normal
//                           (the frame test of the static, sleeping and drop views)
//   --obj-stats FILE        object store events and peaks, one line per level
//   --obj-pages N           at most N pages in the level arena (stress runs)
//   --obj-movers N          at most N mover slots (stress runs)
//   --obj-reserve N         pages kept for missiles and damage (default 8)
//   --god                   god mode with every weapon, full ammo and the six
//                           keycards (tests/lockstep/play.h)
//   --refill                health and armor refilled, every weapon, full ammo
//                           and the six keycards
//   --no-exit               level exits do nothing
//   --tour FILE             teleport the player at the stops of FILE (play.h)
//   --route FILE            per-map ticcmds and stops by level time (play.h),
//                           repeatable
//   --verify-model          frame model build (dmcdoom-frame) only: nothing is
//                           drawn and the creation order of the object store
//                           lies under the tic scratch, as in the verification
//                           pass of the meter
//
// The game data comes from the NOR image only. The accessor-parity build
// (dmcdoom-parity, host/parity.c) also takes -iwad WAD -parnames names.json.
// SAVE.DAT is made at the start when the image has none, as the meter does
// (port/game/dmc_boot.c), the image file changes only with --nor-out.

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomstat.h"
#include "d_event.h"
#include "d_loop.h"
#include "d_main.h"
#include "g_game.h"
#include "i_sound.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "sounds.h"
#include "m_argv.h"
#include "z_zone.h"

#include "dmc_fw.h"
#include "extents.h"
#include "fat_write.h"
#include "fe.h"
#include "fetch.h"
#include "hash.h"
#include "img.h"
#include "keys.h"
#include "play.h"
#include "nor.h"
#include "nor_file.h"
#include "ovl.h"
#include "pad.h"
#include "i_input.h"
#include "platform.h"
#include "stats.h"
#include "things.h"

#ifdef DMC_PARITY
#include "p_mapdata.h"
#endif


static uint32_t clock_ms;
static int max_tics;
static int exit_at = -1;
static int verify_image;
static int render_every = 1;
static const char *nor_out;
static long nor_left = -1;
static int save_at[4][2], nsave;
static int load_slot = -1;
extern int detailLevel;

// host/i_video_host.c
int R_HostOption(int argc, char **argv, int *i);
void R_HostClose(void);

static void finish(void)
{
    LS_Close();
    HS_Close();
    R_HostClose();
    O_StatsWrite();
    if (o_statsfile)
        fclose(o_statsfile);
    o_statsfile = NULL;
    if (nor_out && nor_file_save(nor_out))
        fprintf(stderr, "cannot write %s\n", nor_out);
    nor_out = NULL;
    fflush(stdout);
}

void dmc_fatal(const char *msg)
{
    I_Error("%s", msg);
}

// -playdemo FILE.lmp: the demo file, read where G_DoPlayDemo asks.
static FILE *demo_f;
static uint32_t demo_size;

int dmc_demo_open(const char *name, uint32_t *size)
{
    if (demo_f)
        fclose(demo_f);
    demo_f = fopen(name, "rb");
    if (!demo_f || fseek(demo_f, 0, SEEK_END))
        return -1;
    demo_size = (uint32_t)ftell(demo_f);
    *size = demo_size;
    return 0;
}

void dmc_demo_read(uint32_t off, void *dst, uint32_t n)
{
    if (!demo_f || off > demo_size || n > demo_size - off || fseek(demo_f, (long)off, SEEK_SET)
        || fread(dst, 1, n, demo_f) != n)
        I_Error("demo file read at %u + %u failed", (unsigned)off, (unsigned)n);
}

// ---------------------------------------------------------------- system

void I_Error(char *error, ...)
{
    static int nested;
    char msg[512];
    va_list ap;
    va_start(ap, error);
    vsnprintf(msg, sizeof(msg), error, ap);
    va_end(ap);
    if (nested++)
        exit(1);
    finish();
    if (!strncmp(msg, "timed ", 6)) {
        printf("%s\n", msg);
        exit(0);
    }
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

void I_Quit(void)
{
    finish();
    exit(0);
}

// ---------------------------------------------------------------- overlays

// The front end's overlays are linked in, port/ovl/ovl.c calls them through its native
// entry tables. The screens are drawn (the meter turns them off in its verification pass,
// --verify-model here).
int dmc_render = 1;

// A failed overlay call (port/ovl/ovl.c)
void plat_panic(const char *msg)
{
    I_Error("%s", msg);
    exit(1);
}

int plat_fmt(char *out, int cap, const char *f, ...)
{
    va_list ap;
    va_start(ap, f);
    int n = vsnprintf(out, (size_t)cap, f, ap);
    va_end(ap);
    return n;
}

// The settings the menu changes, as a meter without a settings page starts (dmc_boot.c). The
// host has no settings page and no sound.
dmc_set_t dmc_set = { .sound = SND_LOW };

int settings_save(const void *src, uint32_t size, uint16_t version)
{
    (void)src;
    (void)size;
    (void)version;
    return 0;
}

void snd_volume(int vol)
{
    (void)vol;
}

byte *I_ZoneBase(int *size)
{
    *size = 6 * 1024 * 1024;
    byte *p = malloc(*size);
    if (!p)
        I_Error("no memory for the zone");
    return p;
}

void I_Tactile(int on, int off, int total)
{
    (void)on;
    (void)off;
    (void)total;
}

// ---------------------------------------------------------------- timer

int I_GetTimeMS(void)
{
    return (int)clock_ms;
}

int I_GetTime(void)
{
    return (int)((clock_ms * TICRATE) / 1000);
}

void I_Sleep(int ms)
{
    clock_ms += ms > 0 ? ms : 1;
}

void I_InitTimer(void)
{
}

void I_WaitVBL(int count)
{
    (void)count;
}

// ---------------------------------------------------------------- input

void __real_I_StartTic(void);

// --pad
typedef struct {
    int tic;
    int16_t axis[PAD_NAXES];
    unsigned buttons;
} pad_line_t;
static pad_line_t *pad_lines;
static int pad_n, pad_at;

static int pad_load(const char *path)
{
    FILE *f = fopen(path, "r");
    char line[256];
    if (!f) {
        fprintf(stderr, "cannot read %s\n", path);
        return -1;
    }
    while (fgets(line, sizeof(line), f)) {
        char *c = strchr(line, '#');
        int a[PAD_NAXES], tic, b, n;
        if (c)
            *c = 0;
        n = sscanf(line, "%d %d %d %d %d %d %i", &tic, &a[0], &a[1], &a[2], &a[3], &a[4], &b);
        if (n <= 0)
            continue;
        if (n != 7 || (pad_n && tic < pad_lines[pad_n - 1].tic)) {
            fprintf(stderr, "%s: bad line or out of tic order: %s", path, line);
            fclose(f);
            return -1;
        }
        pad_lines = realloc(pad_lines, (pad_n + 1) * sizeof(*pad_lines));
        pad_line_t *p = &pad_lines[pad_n++];
        p->tic = tic;
        for (int i = 0; i < PAD_NAXES; i++)
            p->axis[i] = (int16_t)a[i];
        p->buttons = (unsigned)b;
    }
    fclose(f);
    return 0;
}

// The chord resolver's platform input: no buttons on the host
int plat_event(void)
{
    return -1;
}

uint32_t plat_buttons(void)
{
    return 0;
}

// The key events of this tic (--keys), as the meter posts them: data2 0. Then the pad
// state of this tic (--pad) and the keys the chord resolver posts for it.
void __wrap_I_StartTic(void)
{
    int down, key;
    while (LS_NextKey(gametic, &down, &key)) {
        event_t ev = { .type = down ? ev_keydown : ev_keyup, .data1 = key };
        D_PostEvent(&ev);
    }
    if (pad_lines) {
        for (; pad_at < pad_n && pad_lines[pad_at].tic <= gametic; pad_at++)
            pad_set(pad_lines[pad_at].axis, pad_lines[pad_at].buttons);
        I_InputTic();
    }
    __real_I_StartTic();
}

// ---------------------------------------------------------------- sound

void I_InitSound(boolean use_sfx_prefix)
{
    (void)use_sfx_prefix;
}

void I_UpdateSound(void)
{
}

// SOUNDS is indexed by the vanilla sfx number, a linked sound (chgun)
// plays the sample of its link.
int I_GetSfxLumpNum(sfxinfo_t *sfxinfo)
{
    if (sfxinfo->link)
        sfxinfo = &S_sfx[sfxinfo->link];
    return (int)(sfxinfo - S_sfx);
}

// No sound output on the host, the sound is looked up in the image as
// the PC speaker driver does. The parity build compares the samples
// with the DP lumps of the WAD.
int I_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep)
{
    uint32_t off = 0;
    uint16_t len = 0;
    int lump = I_GetSfxLumpNum(sfxinfo);
    int r = snd_get(lump, &off, &len);
    (void)vol;
    (void)sep;
#ifdef DMC_PARITY
    PAR_Sound(lump, r, off, len);
#else
    (void)r;
#endif
    return channel;
}

void I_StopSound(int handle)
{
    (void)handle;
}

boolean I_SoundIsPlaying(int handle)
{
    (void)handle;
    return false;
}

void I_UpdateSoundParams(int handle, int vol, int sep)
{
    (void)handle;
    (void)vol;
    (void)sep;
}

// ---------------------------------------------------------------- per-tic hooks

void __real_G_Ticker(void);
void __real_G_BuildTiccmd(ticcmd_t *cmd, int maketic);
void __real_R_RenderPlayerView(player_t *player);

static const char *const save_errors[] = {
    "", "SAVE.DAT is missing or not usable", "the slot is empty", "the save is damaged (CRC)",
    "not in a level, or a save of another map, skill or object store",
    "the save does not fit its slot", "a NOR write failed",
};

void __wrap_G_Ticker(void)
{
    // --save-at: after the display of the tic before, where a save from the menu is taken
    for (int k = 0; k < nsave; k++) {
        if (gametic == save_at[k][0] + 1) {
            int e = OVL_CALL(OVL_SAVEG, SAVE_E_WRITE, save_fn, save_at[k][1], DMC_SF_EXACT);
            if (e)
                I_Error("--save-at %d %d: %s", save_at[k][0], save_at[k][1], save_errors[e]);
        }
    }
    if (gametic == exit_at)
        G_ExitLevel();
    LS_ExitTic();
    HS_Begin(HS_TIC);
    __real_G_Ticker();
    // level tics only, flag 1 for the tic that loaded the level
    if (gamestate == GS_LEVEL)
        HS_End(HS_TIC, gametic, leveltime == 1);
    else
        HS_End(HS_TIC, gametic, 2);
    // the display of this tic follows (singletics)
    HS_Begin(HS_UI);
    LS_Tic();
    if (max_tics && gametic + 1 >= max_tics) {
        finish();
        exit(0);
    }
}

// The 3D view of a frame, apart from the rest of the display (status
// bar, HUD, pages), which ends in I_FinishUpdate.
void __wrap_R_RenderPlayerView(player_t *player)
{
    if (!dmc_render || gametic % render_every)
        return;
    HS_Pause(HS_UI);
    HS_Begin(HS_FRAME);
    __real_R_RenderPlayerView(player);
    HS_End(HS_FRAME, gametic, 0);
    HS_Resume(HS_UI);
}

#ifdef DMC_HOST_FRAME
void __real_R_BandFlush(int hud, int bar);

// --verify-model: no status band either
void __wrap_R_BandFlush(int hud, int bar)
{
    if (dmc_render)
        __real_R_BandFlush(hud, bar);
}
#endif

void __wrap_G_BuildTiccmd(ticcmd_t *cmd, int maketic)
{
    if (LS_HaveRoute() && gamestate != GS_LEVEL)
        LS_RouteIdle(cmd, maketic);
    else if (LS_HaveCmds()) {
        __real_G_BuildTiccmd(cmd, maketic);
        LS_FillCmd(cmd, maketic);
    } else
        __real_G_BuildTiccmd(cmd, maketic);
}

// ---------------------------------------------------------------- saves

// SAVE.DAT as the BOOT overlay of the meter makes it, when the image has none
static void save_dat(void)
{
    static uint8_t sector[FAT_SECTOR];
    fat_create(DMC_VOL_MAIN, "SAVE    DAT", DMC_SAVE_SIZE / FAT_SECTOR, 8, SAVE_ERASE, sector,
               NULL);
}

void G_DoPlayDemo(void);
void __real_TryRunTics(void);

// --load: after D_DoomMain, before the first ticcmd is built (the first TryRunTics of
// D_DoomLoop), the demo opened first (G_DoPlayDemo). That call runs no tic then: the tic after
// the save runs in the loop of D_DoomLoop, followed by its display as in the run that saved.
void __wrap_TryRunTics(void)
{
    static int started;
    if (!started) {
        started = 1;
        if (G_SaveUsable())
            fprintf(stderr, "SAVE.DAT is missing or not usable: no saves\n");
        if (load_slot >= 0) {
            if (gameaction == ga_playdemo)
                G_DoPlayDemo();
            int e = fe_load(load_slot, DMC_SF_EXACT | (demoplayback ? DMC_SF_DEMO : 0));
            if (e)
                I_Error("--load %d: %s", load_slot, save_errors[e]);
            return;
        }
    }
    __real_TryRunTics();
}

int __real_nor_erase(uint32_t phys, unsigned cmd);
int __real_nor_program(uint32_t phys, const void *src, uint32_t n);

// --nor-fail-after: no erase or program after the last one allowed
static void nor_power(void)
{
    if (nor_left >= 0 && !nor_left--) {
        fprintf(stderr, "nor: the power goes before an erase or program\n");
        finish();
        exit(3);
    }
}

int __wrap_nor_erase(uint32_t phys, unsigned cmd)
{
    nor_power();
    return __real_nor_erase(phys, cmd);
}

int __wrap_nor_program(uint32_t phys, const void *src, uint32_t n)
{
    nor_power();
    return __real_nor_program(phys, src, n);
}

// ---------------------------------------------------------------- main

static void usage(void)
{
    fprintf(stderr, "usage: dmcdoom --nor FILE [--hash-out FILE] [--hash-dump TIC FILE] [--frame-out DIR]\n"
                    "  [--frame-stride N] [--cmds FILE] [--tics N] [--exit-at TIC] [--exit-list LIST]\n"
                    "  [--keys FILE] [--pad FILE] [--hash-no-gametic] [--nor-out FILE] [--nor-fail-after N]\n"
                    "  [--save-at TIC SLOT] [--load SLOT] [--stats-out FILE]\n"
                    "  [--rc2] [--rc2-all] [--flat-slots K] [--nor-defer] [--nor-col-reject K]\n"
                    "  [--column-check] [--verify]\n"
                    "  [--render-every N] [--low-detail] [--view-hash FILE] [--view-cols FILE] [--strip-w N]\n"
                    "  [--solid-floors 0|1] [--no-lod] [--strip-clear]\n"
                    "  [--rstats-out FILE] [--frame-report] [--promote-all] [--keep-corpses]\n"
                    "  [--obj-stats FILE] [--obj-pages N] [--obj-movers N] [--obj-reserve N] [--god]\n"
                    "  [--refill] [--no-exit] [--tour FILE] [--route FILE] [--verify-model]\n"
                    "  [--screen-hash FILE] [--screen-full] [--panel-out FILE] game-args...\n");
    exit(2);
}

int main(int argc, char **argv)
{
    char **av = calloc(argc + 1, sizeof(char *));
    int ac = 0;
    av[ac++] = argv[0];
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        int more = argc - i - 1;
        if (!strcmp(a, "--hash-out") && more >= 1) {
            if (LS_Open(argv[++i])) {
                fprintf(stderr, "cannot open %s\n", argv[i]);
                return 2;
            }
        } else if (!strcmp(a, "--hash-dump") && more >= 2) {
            LS_DumpAt(atoi(argv[i + 1]), argv[i + 2]);
            i += 2;
        } else if (R_HostOption(argc, argv, &i)) {
            continue;
        } else if (!strcmp(a, "--cmds") && more >= 1) {
            if (LS_LoadCmds(argv[++i])) {
                fprintf(stderr, "cannot read %s\n", argv[i]);
                return 2;
            }
        } else if (!strcmp(a, "--tics") && more >= 1) {
            max_tics = atoi(argv[++i]);
        } else if (!strcmp(a, "--exit-at") && more >= 1) {
            exit_at = atoi(argv[++i]);
        } else if (!strcmp(a, "--exit-list") && more >= 1) {
            if (LS_ExitList(argv[++i]))
                usage();
        } else if (!strcmp(a, "--keys") && more >= 1) {
            if (LS_LoadKeys(argv[++i]))
                return 2;
        } else if (!strcmp(a, "--pad") && more >= 1) {
            if (pad_load(argv[++i]))
                return 2;
        } else if (!strcmp(a, "--hash-no-gametic")) {
            LS_NoGametic = 1;
        } else if (!strcmp(a, "--nor") && more >= 1) {
            if (nor_file_open(argv[++i])) {
                fprintf(stderr, "cannot read %s\n", argv[i]);
                return 2;
            }
        } else if (!strcmp(a, "--nor-out") && more >= 1) {
            nor_out = argv[++i];
        } else if (!strcmp(a, "--nor-fail-after") && more >= 1) {
            nor_left = atol(argv[++i]);
        } else if (!strcmp(a, "--save-at") && more >= 2 && nsave < 4) {
            save_at[nsave][0] = atoi(argv[i + 1]);
            save_at[nsave][1] = atoi(argv[i + 2]);
            if (save_at[nsave][0] < 0 || (unsigned)save_at[nsave][1] >= DMC_SAVE_SLOTS)
                usage();
            nsave++;
            i += 2;
        } else if (!strcmp(a, "--load") && more >= 1) {
            load_slot = atoi(argv[++i]);
            if ((unsigned)load_slot >= DMC_SAVE_SLOTS)
                usage();
        } else if (!strcmp(a, "--stats-out") && more >= 1) {
            if (HS_Open(argv[++i])) {
                fprintf(stderr, "cannot open %s\n", argv[i]);
                return 2;
            }
        } else if (!strcmp(a, "--rc2")) {
            HS_Rc2(1);
        } else if (!strcmp(a, "--rc2-all")) {
            HS_Rc2(2);
        } else if (!strcmp(a, "--nor-defer")) {
            nor_file_defer(1);
        } else if (!strcmp(a, "--nor-col-reject") && more >= 1) {
            nor_file_col_reject(atoi(argv[++i]));
        } else if (!strcmp(a, "--column-check")) {
            r_columncheck = 1;
        } else if (!strcmp(a, "--verify")) {
            verify_image = 1;
        } else if (!strcmp(a, "--render-every") && more >= 1) {
            render_every = atoi(argv[++i]);
            if (render_every < 1)
                usage();
        } else if (!strcmp(a, "--low-detail")) {
            detailLevel = 1;
        } else if (!strcmp(a, "--promote-all")) {
            o_promote_all = true;
        } else if (!strcmp(a, "--keep-corpses")) {
            o_keep_corpses = true;
        } else if (!strcmp(a, "--obj-pages") && more >= 1) {
            o_page_cap = atoi(argv[++i]);
        } else if (!strcmp(a, "--obj-movers") && more >= 1) {
            o_mover_cap = atoi(argv[++i]);
        } else if (!strcmp(a, "--obj-reserve") && more >= 1) {
            o_reserve = atoi(argv[++i]);
        } else if (!strcmp(a, "--god")) {
            LS_God = 1;
        } else if (!strcmp(a, "--refill")) {
            LS_Refill = 1;
        } else if (!strcmp(a, "--no-exit")) {
            LS_NoExit = 1;
        } else if (!strcmp(a, "--tour") && more >= 1) {
            if (LS_LoadTour(argv[++i])) {
                fprintf(stderr, "cannot read %s\n", argv[i]);
                return 2;
            }
        } else if (!strcmp(a, "--route") && more >= 1) {
            if (LS_LoadRoute(argv[++i])) {
                fprintf(stderr, "cannot read %s\n", argv[i]);
                return 2;
            }
#ifdef DMC_HOST_FRAME
        } else if (!strcmp(a, "--verify-model")) {
            dmc_render = 0;
#endif
        } else if (!strcmp(a, "--obj-stats") && more >= 1) {
            o_statsfile = fopen(argv[++i], "w");
            if (!o_statsfile) {
                fprintf(stderr, "cannot open %s\n", argv[i]);
                return 2;
            }
        } else if (!strncmp(a, "--", 2)) {
            usage();
        } else {
            av[ac++] = argv[i];
        }
    }
    if (nor_init()) {
        fprintf(stderr, "no NOR image, give one with --nor\n");
        return 2;
    }
    save_dat();
    if (verify_image) {
        int r = img_init();
        if (r == IMG_OK)
            r = img_verify();
        printf("image check: %s (%d)\n", r == IMG_OK ? "ok" : "FAILED", r);
        if (r != IMG_OK)
            return 1;
    }
    myargc = ac;
    myargv = av;
    singletics = true;
    D_DoomMain();
    return 0;
}
