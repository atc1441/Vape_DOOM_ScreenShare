// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Host test of the overlay loader. The overlays are linked in natively. In copy mode
// (ovl_host_copy = 1) every load reads DOOMCODE.DMC from the NOR image that norimg.py wrote,
// checks it like the target and copies the blob into ovl_host_frame. The test runs the demo
// checks in copy and direct mode, compares the copied blob and its cleared bss with the file
// and follows each fatal path into plat_panic.
//
// Usage: test_host --nor FILE --build-id HEX40 --vma HEX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "host_stubs.h"
#include "ovl_demo.h"

static int hook_id, hook_err, hook_calls;

static const char *const chk_name[] = {
#define X(n, s) s,
    DEMO_CHECKS(X)
#undef X
};
static const char *const err_name[] = {
#define X(n, s) s,
    DEMO_ERRORS(X)
#undef X
};

static void report_calls(const char *pass, uint32_t fail)
{
    char s[160];
    for (int k = 0; k < CHK_COUNT; k++) {
        snprintf(s, sizeof(s), "%s: %s", pass, chk_name[k]);
        check(!(fail & 1u << k), s);
    }
}

// The blob in ovl_host_frame equals the file's, its bss is cleared and nothing after it
// changed. The demo overlays have no bss, the pack at DEMO_NOR_BSS gives A some.
static void check_copy(void)
{
    const uint8_t *f = nor_mem + DEMO_NOR_BSS;
    uint32_t off = 0, size = 0, count = f[10] | f[11] << 8;
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t *e = f + 64 + 16 * i;
        if (e[0] == OVL_TEST_A) {
            off = rd32(e + 4);
            size = rd32(e + 8);
        }
    }
    ovl_invalidate();
    memset(ovl_host_frame, 0xA5, sizeof(ovl_host_frame));
    ovl_fixed_base = DEMO_NOR_BSS;
    int err = ovl_load(OVL_TEST_A);
    uint32_t bss = rd32(ovl_host_frame + 12);
    int ok = err == OVL_OK && size && !memcmp(ovl_host_frame, f + off, size) &&
             bss == DEMO_BSS_SIZE && size + bss <= OVL_WIN_FRAME;
    for (uint32_t i = 0; ok && i < OVL_WIN_FRAME - size; i++)
        ok = ovl_host_frame[size + i] == (i < bss ? 0 : 0xA5);
    char s[96];
    snprintf(s, sizeof(s), "copy mode: blob A (%u B) copied, its bss (%u B) cleared, rest untouched", size, bss);
    check(ok, s);
}

static void hook(int id, int err)
{
    hook_id = id;
    hook_err = err;
    hook_calls++;
}

// OVL_CALL with the pack at base must end in plat_panic after the hook got (id, want)
static void fatal(const char *what, uint32_t base, int id, int idx, int want, const char *msg, int with_hook)
{
    char s[160];
    hook_id = hook_err = hook_calls = 0;
    panic_msg[0] = 0;
    ovl_set_fail_hook(with_hook ? hook : NULL);
    ovl_fixed_base = base;
    ovl_invalidate();
    volatile int reached = 0;
    if (!setjmp(panic_jmp)) {
        panic_armed = 1;
        OVL_CALL(id, idx, a_sum5_fn, 1, 2, 3, 4, 5);
        reached = 1;
    }
    panic_armed = 0;
    char tail[16];
    snprintf(tail, sizeof(tail), "overlay %d", id);
    int ok = !reached && !strncmp(panic_msg, msg, strlen(msg)) && strstr(panic_msg, tail) &&
             (with_hook ? hook_calls == 1 && hook_id == id && hook_err == want : hook_calls == 0);
    snprintf(s, sizeof(s), "fatal %s: %s\"%s\"", what, with_hook ? "hook, then " : "", panic_msg);
    check(ok, s);
    ovl_set_fail_hook(NULL);
}

int main(int argc, char **argv)
{
    stub_setup(argc, argv);
    printf("copy mode (DOOMCODE.DMC from the NOR image)\n");
    ovl_host_copy = 1;
    ovl_fixed_base = DEMO_NOR_GOOD;
    report_calls("linker thunks", demo_calls());
    ovl_fixed_base = DEMO_NOR_GOOD_LC;
    report_calls("-mlong-calls", demo_calls());
    int32_t codes[ERR_COUNT];
    uint32_t fail = demo_errors(codes);
    for (int k = 0; k < ERR_COUNT; k++) {
        char s[160];
        snprintf(s, sizeof(s), "%s (%d)", err_name[k], (int)codes[k]);
        check(!(fail & 1u << k), s);
    }
    check_copy();
    check(ovl_win[OVL_AUTOMAP] == 5312 && ovl_win[OVL_MENU] == 5696 && ovl_win[OVL_WI] == 5696 &&
          ovl_win[OVL_FINALE] == 5696 && ovl_win[OVL_SAVEG] == 5696 && ovl_win[OVL_LEVEL] == 8368 &&
          ovl_win[OVL_TEST_A] == 8368 && !ovl_win[0] && !ovl_win[OVL_SAVEG - 1],
          "windows: automap 5312, front end 5696, others 8368, unknown ids 0");

    const char *damaged = "DOOMCODE.DMC is damaged, copy it again";
    fatal("build mismatch", DEMO_NOR_BUILD, OVL_TEST_A, A_SUM5, OVL_EBUILD,
          "Copy DOOMCODE.DMC from the same release", 1);
    fatal("build mismatch without hook", DEMO_NOR_BUILD, OVL_TEST_A, A_SUM5, OVL_EBUILD,
          "Copy DOOMCODE.DMC from the same release", 0);
    fatal("blob CRC", DEMO_NOR_CRC, OVL_TEST_A, A_SUM5, OVL_ECRC, "DOOMCODE.DMC CRC error", 1);
    fatal("header CRC", DEMO_NOR_HDR, OVL_TEST_A, A_SUM5, OVL_EHEADER, damaged, 1);
    fatal("erased NOR", DEMO_NOR_EMPTY, OVL_TEST_A, A_SUM5, OVL_EHEADER, damaged, 1);
    fatal("overlay missing", DEMO_NOR_NOENT, OVL_TEST_A, A_SUM5, OVL_ENOENT, "DOOMCODE.DMC lacks", 1);
    fatal("blob over its window", DEMO_NOR_BLOB, OVL_TEST_A, A_SUM5, OVL_EBLOB, "DOOMCODE.DMC has a bad", 1);
    fatal("entry index", DEMO_NOR_GOOD, OVL_TEST_A, A_COUNT, OVL_EENTRY, "Overlay entry out of range", 1);
    fatal("unknown id", DEMO_NOR_GOOD, 0, 0, OVL_ENOENT, "DOOMCODE.DMC lacks", 1);

    printf("direct mode (native calls, no NOR access)\n");
    ovl_host_copy = 0;
    ovl_fixed_base = DEMO_NOR_EMPTY;
    nor_reads = 0;
    report_calls("direct", demo_calls());
    check(nor_reads == 0, "direct mode reads nothing from the NOR");
    check(ovl_load(OVL_LEVEL) == OVL_ENOENT, "direct mode: an overlay that is not linked in gives OVL_ENOENT");

    free(nor_mem);
    printf(failures ? "%u FAILED\n" : "ALL PASSED\n", failures);
    return failures ? 1 : 0;
}
