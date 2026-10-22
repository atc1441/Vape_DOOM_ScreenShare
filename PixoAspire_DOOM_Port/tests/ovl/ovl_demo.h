// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Overlay demo: the two test overlays, the resident code they use and the checks that the
// Renode firmware (demo_main.c) and the host test (test_host.c) run. test_renode.py and
// norimg.py read the defines and the check list from this file.

#pragma once

#include <stdint.h>

#include "ovl.h"

// ---- Overlay A: calling convention, rodata, jump table, callback, state in resident data ----
enum { A_SUM5, A_MUL64, A_TRIO, A_NAME, A_CLASSIFY, A_APPLY, A_STATE, A_COUNT };
struct demo_trio {
    int32_t a, b, c;
};
typedef int32_t a_sum5_fn(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e);
typedef uint64_t a_mul64_fn(uint32_t a, uint32_t b);
typedef struct demo_trio a_trio_fn(int32_t x);
typedef const char *a_name_fn(void);
typedef int a_classify_fn(int k);
typedef int a_apply_fn(int (*fn)(int), int x);
typedef uint32_t a_state_fn(void);

// ---- Overlay B: calls into resident code ----
enum { B_MIX, B_DRAW, B_COUNT };
typedef uint32_t b_mix_fn(uint32_t x);
typedef int b_draw_fn(int y);

// ---- Resident side (demo_seq.c) ----
struct demo_state {
    uint32_t a_calls, b_seen, cb_calls;
};
extern struct demo_state demo_state;
extern const uint8_t demo_tab[16];
int demo_twice(int x);
// What the renderer does before it uses the frame region: ovl_invalidate() and a scribble.
// demo_drop_frame is an alias of it (the packer test reaches the loader through the alias).
void demo_take_frame(void);
void demo_drop_frame(void);
// demo_movt.c, built with -O2: stores the address of a function that loads an overlay,
// with MOVW/MOVT
void demo_defer(void);

// Runs the call checks on the pack at ovl_fixed_base, returns the failed CHK_ bits
uint32_t demo_calls(void);
// Runs the error checks, returns the failed ERR_ bits and stores each load result in codes
uint32_t demo_errors(int32_t codes[]);

// ---- NOR layout of the test image (norimg.py) ----
#define DEMO_NOR_GOOD     0x0F0000u  // packed by make, linker thunks
#define DEMO_NOR_GOOD_LC  0x0F4000u  // packed by make, -mlong-calls
#define DEMO_NOR_NOENT    0x0F8000u  // packed by make, overlay B only
#define DEMO_NOR_CRC      0x0FC000u  // GOOD with a code byte of A flipped
#define DEMO_NOR_HDR      0x100000u  // GOOD with a reserved header byte set
#define DEMO_NOR_BUILD    0x104000u  // GOOD with another build ID, CRCs fixed
#define DEMO_NOR_BLOB     0x108000u  // GOOD with the bss of A over its window, CRCs fixed
#define DEMO_NOR_BSS      0x10C000u  // GOOD with DEMO_BSS_SIZE B of bss for A, CRCs fixed
#define DEMO_NOR_EMPTY    0x110000u  // erased
// GOOD again as a fragmented file for test_ext.c: 128 B clusters, file cluster k at FAT
// cluster DEMO_FRAG_PCL0 - DEMO_FRAG_STEP * k, cluster 2 at DEMO_NOR_FRAG
#define DEMO_NOR_FRAG     0x114000u
#define DEMO_FRAG_SHIFT   7
#define DEMO_FRAG_PCL0    11
#define DEMO_FRAG_STEP    3
// GOOD as DOOMCODE.DMC on a small FAT12 volume (512 B clusters, fragmented) for test_ext.c
#define DEMO_NOR_FAT      0x118000u
#define DEMO_BSS_SIZE     68

// ---- Results (demo_res[] in the Renode firmware) ----
#define DEMO_MAGIC 0x0E1DE30u
#define R_MAGIC    0
#define R_STAGE    1   // 1 running, 2 checks done, 3 fatal call made, 4 fail hook ran
#define R_FAIL0    2   // failed CHK_ bits, GOOD pack
#define R_FAIL1    3   // failed CHK_ bits, GOOD_LC pack
#define R_FAILE    4   // failed ERR_ bits
#define R_LOADS    5
#define R_HOOK_ID  6
#define R_HOOK_ERR 7
#define R_CODES    8   // ERR_COUNT load results
#define R_COUNT    24

#define DEMO_CHECKS(X)                                                   \
    X(LOAD_A, "ovl_load(A) returns OK and A is resident")                \
    X(ARGS, "five arguments, the fifth on the stack")                    \
    X(RET64, "64-bit return value")                                      \
    X(SRET, "struct returned through a hidden pointer")                  \
    X(RODATA, "string from the overlay's rodata")                        \
    X(SWITCH, "switch with a jump table")                                \
    X(CALLBACK, "resident callback through a function pointer")          \
    X(STATE, "overlay state kept in a resident struct")                  \
    X(NO_RELOAD, "calls to the resident overlay do not reload it")       \
    X(LOAD_B, "OVL_CALL loads B over A")                                 \
    X(RESIDENT, "B calls resident code and uses resident data")          \
    X(PLATFORM, "B calls plat_fmt and lcd_text")                         \
    X(RELOAD, "A loaded again over B, its resident state intact")        \
    X(INVALIDATE, "ovl_invalidate and a scribbled window force a reload")

#define DEMO_ERRORS(X)                                                   \
    X(CRC, "blob CRC error gives OVL_ECRC, nothing resident")            \
    X(HDR, "header CRC error gives OVL_EHEADER")                         \
    X(BUILD, "other build ID gives OVL_EBUILD")                          \
    X(NOENT, "overlay missing from the file gives OVL_ENOENT")           \
    X(BLOB, "blob over its window gives OVL_EBLOB")                      \
    X(EMPTY, "erased NOR gives OVL_EHEADER")                             \
    X(UNKNOWN, "id unknown to the firmware gives OVL_ENOENT")            \
    X(BSS, "a blob with bss loads, the bss after it is cleared")         \
    X(OTHER, "B still loads from the file without A")                    \
    X(RECOVER, "the good file loads again after the errors")

enum {
#define X(n, s) CHK_##n,
    DEMO_CHECKS(X)
#undef X
    CHK_COUNT
};

enum {
#define X(n, s) ERR_##n,
    DEMO_ERRORS(X)
#undef X
    ERR_COUNT
};
