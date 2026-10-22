// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Host test of the USB drive (port/usb) on a 4 MB NOR image (host/nor_file.c), through a fake
// controller driver: the driver API of platform/usbd.h with the host's side of endpoint 0 and
// of the bulk pipes, packet by packet (64 B, a transfer ends at its packet count or at a short
// packet, a pipe NAKs while the device has no transfer for it).
//
// usage: usb_host suite NOR.img           protocol tests, the image is not written back
//        usb_host format NOR.img OUT.img   quick format of the data drive (vol_format), then a
//                                          block of its clusters written with WRITE(10), which
//                                          must need no erase (the image is saved before it)
//        usb_host replay NOR.img REC OUT.img
//                                          REC: records of u32 LBA (little-endian) and 4 KB
//                                          of data, written with WRITE(10), runs of
//                                          consecutive blocks in one command of at most 32
//        usb_host ui NOR.img OUT.img       the folder UI (vol_ui), prints its result
//        usb_host img NOR.img              img_init and img_verify of the data layer
// Exit status 0 when every check passes.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fat12.h"
#include "img.h"
#include "nor.h"
#include "nor_file.h"
#include "usb.h"

usb_ram_t usb_ram;

static int fails;

#define CHECK(c, ...)                                             \
    do {                                                          \
        if (!(c)) {                                               \
            fails++;                                              \
            printf("  FAIL %s:%d: ", __FILE__, __LINE__);          \
            printf(__VA_ARGS__);                                  \
            printf("\n");                                         \
        }                                                         \
    } while (0)

void dmc_fatal(const char *msg)
{
    printf("dmc_fatal: %s\n", msg);
    exit(1);
}

// ---- the fake controller

#define QN 16
static struct {
    int type;
    usbd_ev_t e;
} evq[QN];
static unsigned qh, qt;

static struct {
    uint8_t *buf;
    uint32_t len, pos, pk;
    int on;
} xin, xout;
static int stall_in, stall_out, open_in, open_out;
static uint8_t dev_addr, ctl[64];
static int ctl_n;  // reply length, -1 stall, -2 no answer
static const uint32_t uid[3] = {0x0102A3B4u, 0xC5D6E7F8u, 0x090A0B0Cu};

static void push(int type, const usbd_ev_t *e)
{
    if (qh - qt == QN) {
        printf("event queue full\n");
        exit(1);
    }
    evq[qh % QN].type = type;
    evq[qh % QN].e = *e;
    qh++;
}

void usbd_init(void) {}
void usbd_connect(void) {}
void usbd_off(void) {}
void usbd_uid(uint32_t id[3]) { memcpy(id, uid, sizeof(uid)); }
int usbd_suspended(void) { return 0; }

// The NOR programs fail in the sector nor_bad (linked with --wrap=nor_program)
int __real_nor_program(uint32_t phys, const void *src, uint32_t n);
static uint32_t nor_bad = ~0u;
int __wrap_nor_program(uint32_t phys, const void *src, uint32_t n)
{
    return (phys & ~(USB_BLOCK - 1)) == nor_bad ? -1 : __real_nor_program(phys, src, n);
}

// A bus reset closes the bulk endpoints, as the controller driver does
int usbd_poll(usbd_ev_t *e)
{
    if (qh == qt)
        return USBD_NONE;
    *e = evq[qt % QN].e;
    int t = evq[qt++ % QN].type;
    if (t == USBD_RESET) {
        usbd_close(USB_EP_IN);
        usbd_close(USB_EP_OUT);
    }
    return t;
}

void usbd_ctl_send(const void *data, uint32_t n)
{
    CHECK(n <= 64, "control reply of %u B", n);
    memcpy(ctl, data, n);
    ctl_n = (int)n;
}

void usbd_ctl_stall(void) { ctl_n = -1; }
void usbd_set_address(uint8_t addr) { dev_addr = addr; }

void usbd_open(uint8_t ep)
{
    if (ep & 0x80) {
        open_in = 1;
        stall_in = 0;
    } else {
        open_out = 1;
        stall_out = 0;
    }
}

void usbd_close(uint8_t ep)
{
    if (ep & 0x80) {
        open_in = xin.on = stall_in = 0;
    } else {
        open_out = xout.on = stall_out = 0;
    }
}

void usbd_xfer(uint8_t ep, void *buf, uint32_t n)
{
    CHECK(n > 0 && n <= USB_BLOCK, "transfer of %u B", n);
    if (ep & 0x80) {
        CHECK(open_in && !xin.on, "IN transfer on an endpoint that is closed or busy");
        xin.buf = buf;
        xin.len = n;
        xin.pos = 0;
        xin.on = 1;
        return;
    }
    CHECK(open_out, "OUT transfer on a closed endpoint");
    xout.buf = buf;
    xout.len = n;
    xout.pos = 0;
    if (!xout.on)  // the controller keeps the packet count of a transfer it still waits for
        xout.pk = (n + 63) / 64;
    xout.on = 1;
}

void usbd_abort_in(void) { xin.on = 0; }

void usbd_stall(uint8_t ep, int on)
{
    if (ep & 0x80)
        stall_in = on;
    else
        stall_out = on;
}

int usbd_stalled(uint8_t ep) { return ep & 0x80 ? stall_in : stall_out; }

// ---- the host's side

static void run(void)
{
    while (qh != qt)
        usb_task();
}

// A control transfer. Returns the reply length, -1 for a STALL.
static int control(uint8_t type, uint8_t req, uint16_t val, uint16_t idx, uint16_t len, uint8_t *reply)
{
    usbd_ev_t e = {{type, req, (uint8_t)val, (uint8_t)(val >> 8), (uint8_t)idx, (uint8_t)(idx >> 8),
                    (uint8_t)len, (uint8_t)(len >> 8)}, 0, 0};
    ctl_n = -2;
    push(USBD_SETUP, &e);
    run();
    if (ctl_n == -2) {
        printf("  FAIL request %02x %02x: no answer\n", type, req);
        exit(1);
    }
    if (ctl_n > 0 && reply)
        memcpy(reply, ctl, (size_t)ctl_n);
    return ctl_n;
}

static void clear_halt(uint8_t ep)
{
    CHECK(control(0x02, 1, 0, ep, 0, NULL) == 0, "ClearFeature(ENDPOINT_HALT) %02x refused", ep);
}

static int bot_reset(void) { return control(0x21, 0xFF, 0, 0, 0, NULL); }

// Bulk OUT: n bytes in packets of 64 B. 0, -1 when the pipe stalls, -2 when the device takes
// no more (NAK).
static int h_out(const uint8_t *d, uint32_t n)
{
    uint32_t off = 0;
    do {
        uint32_t k = n - off < 64 ? n - off : 64;
        run();
        if (stall_out)
            return -1;
        if (!xout.on)
            return -2;
        uint32_t room = xout.len - xout.pos;
        memcpy(xout.buf + xout.pos, d + off, k < room ? k : room);
        xout.pos += k < room ? k : room;
        off += k;
        if (--xout.pk == 0 || k < 64) {
            usbd_ev_t e = {{0}, USB_EP_OUT, xout.pos};
            xout.on = 0;
            push(USBD_DONE, &e);
        }
    } while (off < n);
    run();
    return 0;
}

// Bulk IN: packets until a short one or max bytes. The bytes, -1 when the pipe stalls first,
// -2 when the device sends nothing (NAK), -3 when it sends more than max.
static int stalled_after;  // bytes received before a STALL
static int h_in(uint8_t *d, uint32_t max)
{
    uint32_t got = 0;
    stalled_after = -1;
    for (;;) {
        run();
        if (stall_in) {
            stalled_after = (int)got;
            return got ? (int)got : -1;
        }
        if (!xin.on)
            return got ? (int)got : -2;
        uint32_t k = xin.len - xin.pos < 64 ? xin.len - xin.pos : 64;
        if (got + k > max)
            return -3;
        memcpy(d + got, xin.buf + xin.pos, k);
        xin.pos += k;
        got += k;
        if (xin.pos == xin.len) {
            usbd_ev_t e = {{0}, USB_EP_IN, 0};
            xin.on = 0;
            push(USBD_DONE, &e);
        }
        if (k < 64 || got == max) {
            run();
            return (int)got;
        }
    }
}

// ---- SCSI commands as Linux usb-storage sends them

typedef struct {
    int status;     // CSW status, -1 when there was no valid CSW
    uint32_t resid;
    int n;          // data bytes moved
    int stalled;    // the data phase ended in a STALL
} res_t;

static uint32_t tag = 0x1000;

static void cbw_make(uint8_t *c, const uint8_t *cdb, int len, int in, uint32_t h)
{
    memset(c, 0, 31);
    memcpy(c, "USBC", 4);
    tag++;
    memcpy(c + 4, &tag, 4);
    memcpy(c + 8, &h, 4);
    c[12] = in ? 0x80 : 0;
    c[14] = (uint8_t)len;
    memcpy(c + 15, cdb, (size_t)len);
}

// The CSW of the command with the tag of the last CBW. A STALL before it counts as a STALL
// of the data phase (after a short packet the host sees it only here).
static void csw_in(res_t *r)
{
    uint8_t s[13];
    int n = h_in(s, 13);
    if (n == -1) {
        r->stalled = 1;
        clear_halt(USB_EP_IN);
        n = h_in(s, 13);
    }
    uint32_t t;
    memcpy(&t, s + 4, 4);
    if (n != 13 || memcmp(s, "USBS", 4) || t != tag) {
        printf("  no valid CSW (%d)\n", n);
        return;
    }
    memcpy(&r->resid, s + 8, 4);
    r->status = s[12];
}

static res_t cmd(const uint8_t *cdb, int len, int in, uint32_t h, uint8_t *data)
{
    res_t r = {-1, 0, 0, 0};
    uint8_t c[31];
    cbw_make(c, cdb, len, in, h);
    if (h_out(c, 31)) {
        printf("  CBW not taken\n");
        return r;
    }
    if (h && in) {
        r.n = h_in(data, h);
        if (r.n == -1 || stalled_after >= 0) {
            r.stalled = 1;
            r.n = r.n < 0 ? 0 : r.n;
            clear_halt(USB_EP_IN);
        }
    } else if (h) {
        r.n = (int)h;
        if (h_out(data, h) == -1) {
            r.stalled = 1;
            clear_halt(USB_EP_OUT);
        }
    }
    csw_in(&r);
    return r;
}

static uint8_t sense[18];

// REQUEST SENSE: key << 16 | asc << 8 | ascq
static uint32_t req_sense(void)
{
    const uint8_t cdb[6] = {0x03, 0, 0, 0, 18, 0};
    res_t r = cmd(cdb, 6, 1, 18, sense);
    CHECK(r.status == 0 && r.n == 18 && sense[0] == 0x70, "REQUEST SENSE (%d, %d)", r.status, r.n);
    return (uint32_t)(sense[2] & 15) << 16 | sense[12] << 8 | sense[13];
}

static void rw10(uint8_t *cdb, uint8_t op, uint32_t lba, uint32_t n)
{
    memset(cdb, 0, 10);
    cdb[0] = op;
    cdb[2] = (uint8_t)(lba >> 24);
    cdb[3] = (uint8_t)(lba >> 16);
    cdb[4] = (uint8_t)(lba >> 8);
    cdb[5] = (uint8_t)lba;
    cdb[7] = (uint8_t)(n >> 8);
    cdb[8] = (uint8_t)n;
}

static res_t read10(uint32_t lba, uint32_t n, uint32_t h, uint8_t *d)
{
    uint8_t cdb[10];
    rw10(cdb, 0x28, lba, n);
    return cmd(cdb, 10, 1, h, d);
}

static res_t write10(uint32_t lba, uint32_t n, uint32_t h, uint8_t *d)
{
    uint8_t cdb[10];
    rw10(cdb, 0x2A, lba, n);
    return cmd(cdb, 10, 0, h, d);
}

static uint8_t *nor_copy(void)
{
    uint8_t *m = malloc(NOR_SIZE);
    nor_read(0, m, NOR_SIZE);
    return m;
}

// ---- protocol tests

static void enumerate(void)
{
    uint8_t d[64];
    printf("enumeration as Linux does it\n");
    usb_start();
    usbd_ev_t e = {{0}, 0, 0};
    push(USBD_RESET, &e);
    run();
    CHECK(control(0x80, 6, 0x100, 0, 64, d) == 18 && d[0] == 18 && d[1] == 1 && d[7] == 64,
          "device descriptor");
    CHECK(d[8] == 0x3C && d[9] == 0x2E && d[10] == 0x20 && d[11] == 0x57, "VID 2E3C, PID 5720");
    CHECK(control(0x00, 5, 7, 0, 0, NULL) == 0 && dev_addr == 7, "SET_ADDRESS 7");
    CHECK(control(0x80, 6, 0x100, 0, 18, d) == 18, "device descriptor, 18 B");
    CHECK(control(0x80, 6, 0x200, 0, 9, d) == 9 && d[2] == 32 && d[7] == 0xC0 && d[8] == 50,
          "configuration descriptor, 9 B: self-powered, up to 100 mA from the bus");
    CHECK(control(0x80, 6, 0x200, 0, 255, d) == 32 && d[9 + 5] == 8 && d[9 + 6] == 6 && d[9 + 7] == 0x50
              && d[18 + 2] == 0x81 && d[18 + 3] == 2 && d[25 + 2] == 0x01 && d[25 + 3] == 2,
          "configuration: mass storage, SCSI, Bulk-Only, bulk 0x81 and 0x01");
    CHECK(control(0x80, 6, 0x300, 0, 255, d) == 4 && d[2] == 0x09 && d[3] == 0x04, "language English (US)");
    uint8_t dev[18];
    control(0x80, 6, 0x100, 0, 18, dev);
    char s[40] = {0};
    int n = control(0x80, 6, 0x300 | dev[15], 0x409, 255, d);
    for (int i = 0; 2 + 2 * i < n && i < 39; i++)
        s[i] = (char)d[2 + 2 * i];
    CHECK(!strcmp(s, "DMC-100 DOOM data"), "product string \"%s\"", s);
    memset(s, 0, sizeof(s));
    n = control(0x80, 6, 0x300 | dev[16], 0x409, 255, d);
    for (int i = 0; 2 + 2 * i < n && i < 39; i++)
        s[i] = (char)d[2 + 2 * i];
    CHECK(!strcmp(s, "0102A3B4C5D6E7F8090A0B0C"), "serial number from the chip ID \"%s\"", s);
    CHECK(!dev[14], "no manufacturer string");
    CHECK(control(0x80, 6, 0x600, 0, 10, d) == -1, "device qualifier stalls (full speed only)");
    CHECK(control(0x80, 6, 0x3EE, 0, 18, d) == -1, "string 0xEE stalls");
    CHECK(control(0xA1, 0xFE, 0, 0, 1, d) == -1, "Get Max LUN stalls before the configuration");
    CHECK(control(0x81, 0, 0, 0, 2, d) == -1, "GET_STATUS of the interface stalls before the configuration");
    CHECK(control(0x80, 8, 0, 0, 1, d) == 1 && d[0] == 0, "GET_CONFIGURATION 0");
    CHECK(control(0x00, 9, 2, 0, 0, NULL) == -1, "SET_CONFIGURATION 2 stalls");
    CHECK(control(0x00, 9, 1, 0, 0, NULL) == 0 && open_in && open_out && xout.on,
          "SET_CONFIGURATION 1: bulk endpoints open, the CBW awaited");
    CHECK(control(0x80, 8, 0, 0, 1, d) == 1 && d[0] == 1, "GET_CONFIGURATION 1");
    CHECK(control(0x80, 0, 0, 0, 2, d) == 2 && d[0] == 1 && !d[1], "GET_STATUS device: self-powered");
    CHECK(control(0x81, 0, 0, 0, 2, d) == 2 && !d[0] && !d[1], "GET_STATUS interface 0");
    CHECK(control(0x81, 0, 0, 1, 2, d) == -1, "GET_STATUS of interface 1 stalls");
    CHECK(control(0x82, 0, 0, 0x81, 2, d) == 2 && !d[0], "GET_STATUS 0x81 not halted");
    CHECK(control(0x82, 0, 0, 0x82, 2, d) == -1, "GET_STATUS of an endpoint that is not there stalls");
    CHECK(control(0x81, 10, 0, 0, 1, d) == 1 && !d[0], "GET_INTERFACE 0");
    CHECK(control(0x01, 11, 0, 0, 0, NULL) == 0, "SET_INTERFACE 0");
    CHECK(control(0x01, 11, 1, 0, 0, NULL) == -1, "SET_INTERFACE 1 stalls");
    CHECK(control(0xA1, 0xFE, 0, 0, 1, d) == 1 && d[0] == 0, "Get Max LUN: 0");
    CHECK(control(0x40, 0x55, 0, 0, 0, NULL) == -1, "vendor request stalls");
    CHECK(control(0x00, 3, 1, 0, 0, NULL) == -1, "SET_FEATURE remote wakeup stalls");
}

static void mount_sequence(uint8_t *ref)
{
    uint8_t d[USB_BLOCK * 2];
    res_t r;
    printf("Linux, Windows and macOS attach and mount\n");
    const uint8_t inq[6] = {0x12, 0, 0, 0, 36, 0};
    r = cmd(inq, 6, 1, 36, d);
    CHECK(r.status == 0 && r.n == 36 && !r.resid && d[0] == 0 && d[1] == 0x80, "INQUIRY: direct access, removable");
    CHECK(!memcmp(d + 8, "DMC-100 DOOM data       1.0 ", 28), "INQUIRY vendor, product, revision");
    const uint8_t tur[6] = {0};
    r = cmd(tur, 6, 0, 0, NULL);
    CHECK(r.status == 0, "TEST UNIT READY");
    const uint8_t rc[10] = {0x25};
    r = cmd(rc, 10, 1, 8, d);
    CHECK(r.status == 0 && r.n == 8 && d[2] == 2 && d[3] == 0xFF && d[6] == 0x10 && d[7] == 0,
          "READ CAPACITY(10): last LBA 767, 4096 B blocks");
    const uint8_t rc16[16] = {0x9E, 0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0};
    r = cmd(rc16, 16, 1, 32, d);
    CHECK(r.status == 1 && r.stalled && r.resid == 32, "READ CAPACITY(16) is not supported");
    CHECK(req_sense() == 0x052000, "its sense: ILLEGAL REQUEST, INVALID COMMAND OPERATION CODE");
    const uint8_t ms6[6] = {0x1A, 0, 0x3F, 0, 192, 0};
    r = cmd(ms6, 6, 1, 192, d);
    CHECK(r.status == 0 && r.n == 4 && r.resid == 188 && r.stalled && d[0] == 3 && !(d[2] & 0x80),
          "MODE SENSE(6) all pages: header, not write protected, short packet and STALL");
    const uint8_t ms6c[6] = {0x1A, 0, 0x08, 0, 4, 0};
    r = cmd(ms6c, 6, 1, 4, d);
    CHECK(r.status == 0 && r.n == 4 && !r.resid, "MODE SENSE(6) caching page, 4 B");
    const uint8_t ms10[10] = {0x5A, 0, 0x1C, 0, 0, 0, 0, 0, 0xFF, 0};
    r = cmd(ms10, 10, 1, 255, d);
    CHECK(r.status == 0 && r.n == 8 && r.resid == 247 && d[1] == 6, "MODE SENSE(10)");
    const uint8_t rfc[10] = {0x23, 0, 0, 0, 0, 0, 0, 0, 0xFC, 0};
    r = cmd(rfc, 10, 1, 0xFC, d);
    CHECK(r.status == 0 && r.n == 12 && d[3] == 8 && d[6] == 3 && d[7] == 0 && d[8] == 2 && d[10] == 0x10,
          "READ FORMAT CAPACITIES: 768 formatted blocks of 4096 B");
    r = read10(0, 1, USB_BLOCK, d);
    CHECK(r.status == 0 && r.n == USB_BLOCK && !memcmp(d, ref + USB_VOL, USB_BLOCK), "READ(10) LBA 0");
    r = read10(1, 2, 2 * USB_BLOCK, d);
    CHECK(r.status == 0 && r.n == 2 * USB_BLOCK && !memcmp(d, ref + USB_VOL + USB_BLOCK, 2 * USB_BLOCK),
          "READ(10) LBA 1, 2 blocks");
    const uint8_t pa[6] = {0x1E, 0, 0, 0, 1, 0};
    r = cmd(pa, 6, 0, 0, NULL);
    CHECK(r.status == 0, "PREVENT ALLOW MEDIUM REMOVAL");
    const uint8_t sc[10] = {0x35};
    r = cmd(sc, 10, 0, 0, NULL);
    CHECK(r.status == 0, "SYNCHRONIZE CACHE(10)");
    uint8_t vf[10];
    rw10(vf, 0x2F, 5, 10);
    r = cmd(vf, 10, 0, 0, NULL);
    CHECK(r.status == 0, "VERIFY(10)");
    CHECK(req_sense() == 0, "REQUEST SENSE: no sense");
}

static void read_write(uint8_t *ref)
{
    uint8_t *d = malloc(32 * USB_BLOCK), *e = malloc(32 * USB_BLOCK);
    res_t r;
    printf("multi-block reads and writes over the whole volume\n");
    int ok = 1;
    for (uint32_t lba = 0; lba < USB_BLOCKS; lba += 32) {
        r = read10(lba, 32, 32 * USB_BLOCK, d);
        ok &= r.status == 0 && r.n == 32 * USB_BLOCK && !memcmp(d, ref + USB_VOL + lba * USB_BLOCK, 32 * USB_BLOCK);
    }
    CHECK(ok, "READ(10) of the volume in 24 commands of 32 blocks");
    // the whole volume written, in commands of 1 to 32 blocks, then read back
    ok = 1;
    for (uint32_t lba = 0, n = 1; lba < USB_BLOCKS; lba += n, n = n % 32 + 1) {
        if (n > USB_BLOCKS - lba)
            n = USB_BLOCKS - lba;
        for (uint32_t i = 0; i < n * USB_BLOCK; i++)
            d[i] = (uint8_t)((lba * USB_BLOCK + i) * 2654435761u >> 24);
        r = write10(lba, n, n * USB_BLOCK, d);
        ok &= r.status == 0 && !r.resid && !r.stalled;
    }
    CHECK(ok, "WRITE(10) of every block of the volume");
    uint8_t *m = nor_copy();
    ok = !memcmp(m, ref, USB_VOL);
    CHECK(ok, "the NOR below the volume is unchanged");
    ok = 1;
    for (uint32_t i = 0; i < USB_BLOCKS * USB_BLOCK; i++)
        ok &= m[USB_VOL + i] == (uint8_t)(i * 2654435761u >> 24);
    CHECK(ok, "the NOR holds what was written");
    for (uint32_t lba = 0; lba < USB_BLOCKS; lba += 32) {
        r = read10(lba, 32, 32 * USB_BLOCK, d);
        ok &= r.status == 0 && !memcmp(d, m + USB_VOL + lba * USB_BLOCK, 32 * USB_BLOCK);
    }
    CHECK(ok, "READ(10) gives it back");
    // a rewrite of the same data, of erased and of mixed blocks
    memcpy(d, m + USB_VOL + 100 * USB_BLOCK, 3 * USB_BLOCK);
    memset(d + 3 * USB_BLOCK, 0xFF, USB_BLOCK);
    memset(d + 4 * USB_BLOCK, 0x00, USB_BLOCK);
    r = write10(100, 5, 5 * USB_BLOCK, d);
    CHECK(r.status == 0, "WRITE(10) of unchanged, erased and zero blocks");
    memset(e, 0xFF, USB_BLOCK);
    r = write10(104, 1, USB_BLOCK, e);
    CHECK(r.status == 0, "WRITE(10) of an erased block over zeros");
    memset(e, 0x5A, USB_BLOCK);
    r = write10(103, 1, USB_BLOCK, e);
    CHECK(r.status == 0, "WRITE(10) over an erased block");
    nor_read(USB_VOL + 100 * USB_BLOCK, e, 5 * USB_BLOCK);
    memset(d + 4 * USB_BLOCK, 0xFF, USB_BLOCK);
    memset(d + 3 * USB_BLOCK, 0x5A, USB_BLOCK);
    CHECK(!memcmp(d, e, 5 * USB_BLOCK), "those blocks hold what was written last");
    free(m);
    m = nor_copy();
    CHECK(!memcmp(m, ref, USB_VOL), "the NOR below the volume is still unchanged");
    memcpy(ref, m, NOR_SIZE);
    free(m);
    free(d);
    free(e);
}

static void errors(uint8_t *ref)
{
    uint8_t d[3 * USB_BLOCK];
    res_t r;
    printf("out of range LBAs, unsupported commands, sense data\n");
    r = read10(USB_BLOCKS, 1, USB_BLOCK, d);
    CHECK(r.status == 1 && r.stalled && r.resid == USB_BLOCK && r.n == 0, "READ(10) at LBA 768 fails, IN stalled");
    CHECK(req_sense() == 0x052100, "sense: ILLEGAL REQUEST, LBA OUT OF RANGE");
    r = read10(USB_BLOCKS - 1, 2, 2 * USB_BLOCK, d);
    CHECK(r.status == 1 && r.stalled, "READ(10) of LBA 767 and 768 fails");
    CHECK(req_sense() == 0x052100, "sense: LBA OUT OF RANGE");
    r = read10(0xFFFFFFFFu, 1, USB_BLOCK, d);
    CHECK(r.status == 1 && req_sense() == 0x052100, "READ(10) at LBA 0xFFFFFFFF fails");
    r = read10(USB_BLOCKS - 1, 1, USB_BLOCK, d);
    CHECK(r.status == 0 && !memcmp(d, ref + NOR_SIZE - USB_BLOCK, USB_BLOCK), "READ(10) of LBA 767");
    r = read10(10, 0, 0, d);
    CHECK(r.status == 0, "READ(10) of 0 blocks");
    memset(d, 0x11, sizeof(d));
    r = write10(USB_BLOCKS, 1, USB_BLOCK, d);
    CHECK(r.status == 1 && r.resid == USB_BLOCK && !r.stalled, "WRITE(10) at LBA 768 fails, its data dropped");
    CHECK(req_sense() == 0x052100, "sense: LBA OUT OF RANGE");
    r = write10(USB_BLOCKS - 1, 2, 2 * USB_BLOCK, d);
    CHECK(r.status == 1 && req_sense() == 0x052100, "WRITE(10) of LBA 767 and 768 fails");
    uint8_t *m = nor_copy();
    CHECK(!memcmp(m, ref, NOR_SIZE), "the NOR is unchanged");
    free(m);
    uint8_t vf[10];
    rw10(vf, 0x2F, 0, 1);
    vf[1] = 2;
    r = cmd(vf, 10, 0, 0, NULL);
    CHECK(r.status == 1 && req_sense() == 0x052400, "VERIFY(10) with BYTCHK: INVALID FIELD IN CDB");
    rw10(vf, 0x2F, 767, 2);
    r = cmd(vf, 10, 0, 0, NULL);
    CHECK(r.status == 1 && req_sense() == 0x052100, "VERIFY(10) out of range");
    const uint8_t evpd[6] = {0x12, 1, 0x80, 0, 255, 0};
    r = cmd(evpd, 6, 1, 255, d);
    CHECK(r.status == 1 && r.stalled && req_sense() == 0x052400, "INQUIRY of a VPD page: INVALID FIELD IN CDB");
    static const uint8_t ops[] = {0x08, 0x0A, 0x15, 0x55, 0x4D, 0xA0, 0x88, 0x85, 0x04, 0x01, 0xFF};
    int ok = 1;
    for (unsigned i = 0; i < sizeof(ops); i++) {
        uint8_t c[16] = {ops[i]};
        r = cmd(c, ops[i] >= 0x80 ? 16 : ops[i] >= 0x20 ? 10 : 6, 1, 64, d);
        ok &= r.status == 1 && r.stalled && r.resid == 64 && req_sense() == 0x052000;
        r = cmd(c, ops[i] >= 0x80 ? 16 : ops[i] >= 0x20 ? 10 : 6, 0, 0, NULL);
        ok &= r.status == 1 && req_sense() == 0x052000;
    }
    CHECK(ok, "READ(6), WRITE(6), MODE SELECT, LOG SENSE, REPORT LUNS, ATA PASS-THROUGH, 0xFF and "
              "others: INVALID COMMAND OPERATION CODE");
    CHECK(req_sense() == 0, "the sense is cleared once read");
    // the NOR fails in the second of three blocks: the residue counts the two not written
    nor_bad = USB_VOL + 41 * USB_BLOCK;
    memset(d, 0x66, sizeof(d));
    r = write10(40, 3, 3 * USB_BLOCK, d);
    nor_bad = ~0u;
    CHECK(r.status == 1 && r.resid == 2 * USB_BLOCK && !r.stalled,
          "WRITE(10) of 3 blocks, the NOR fails in the second: residue of 2 blocks (%d, %u)", r.status, r.resid);
    CHECK(req_sense() == 0x030C00, "sense: MEDIUM ERROR, WRITE ERROR");
    m = nor_copy();
    CHECK(!memcmp(m + USB_VOL + 40 * USB_BLOCK, d, USB_BLOCK)
              && !memcmp(m + USB_VOL + 42 * USB_BLOCK, ref + USB_VOL + 42 * USB_BLOCK, USB_BLOCK),
          "the first block is written, the third is not");
    memcpy(ref, m, NOR_SIZE);
    free(m);
}

static void phases(uint8_t *ref)
{
    uint8_t d[3 * USB_BLOCK], z[3 * USB_BLOCK];
    res_t r;
    const uint8_t tur[6] = {0}, inq[6] = {0x12, 0, 0, 0, 36, 0};
    printf("the thirteen cases of the Bulk-Only Transport\n");
    r = read10(3, 1, 0, d);
    CHECK(r.status == 2, "case 2 (Hn < Di): READ(10) without data, phase error");
    r = write10(3, 1, 0, d);
    CHECK(r.status == 2, "case 3 (Hn < Do): WRITE(10) without data, phase error");
    r = cmd(tur, 6, 1, 64, d);
    CHECK(r.status == 0 && r.stalled && r.resid == 64, "case 4 (Hi > Dn): STALL, good status, residue 64");
    r = cmd(inq, 6, 1, 100, d);
    CHECK(r.status == 0 && r.stalled && r.n == 36 && r.resid == 64,
          "case 5 (Hi > Di): short packet, STALL, residue 64");
    const uint8_t rs64[6] = {0x03, 0, 0, 0, 64, 0};
    r = cmd(rs64, 6, 1, 128, d);
    CHECK(r.status == 0 && r.stalled && r.n == 18 && r.resid == 110, "case 5: REQUEST SENSE of 18 B for 128, STALL");
    r = cmd(inq, 6, 1, 36, d);
    CHECK(r.status == 0 && !r.stalled && r.n == 36 && !r.resid, "case 6 (Hi = Di): no STALL");
    r = read10(3, 1, 2 * USB_BLOCK, d);
    CHECK(r.status == 0 && r.stalled && r.n == USB_BLOCK && r.resid == USB_BLOCK
              && !memcmp(d, ref + USB_VOL + 3 * USB_BLOCK, USB_BLOCK),
          "case 5: READ(10) of 1 block for 2, whole packets: data, STALL, residue");
    r = read10(3, 2, USB_BLOCK, d);
    CHECK(r.status == 2 && r.n == USB_BLOCK && !memcmp(d, ref + USB_VOL + 3 * USB_BLOCK, USB_BLOCK),
          "case 7 (Hi < Di): READ(10) of 2 blocks for 1, phase error");
    r = read10(3, 1, 1000, d);
    CHECK(r.status == 2 && r.n == 1000 && !memcmp(d, ref + USB_VOL + 3 * USB_BLOCK, 1000),
          "case 7: READ(10) for 1000 B");
    uint8_t wr[10];
    rw10(wr, 0x2A, 3, 1);
    r = cmd(wr, 10, 1, USB_BLOCK, d);
    CHECK(r.status == 2 && r.stalled && r.resid == USB_BLOCK, "case 8 (Hi <> Do): WRITE(10) with IN, phase error");
    r = cmd(inq, 6, 0, 36, z);
    CHECK(r.status == 2 && !r.stalled, "case 10 (Ho <> Di): INQUIRY with OUT, its data dropped, phase error");
    r = cmd(tur, 6, 0, 100, z);
    CHECK(r.status == 0 && r.resid == 100, "case 9 (Ho > Dn): data dropped, good status, residue 100");
    memset(z, 0x33, sizeof(z));
    r = write10(3, 1, 2 * USB_BLOCK, z);
    nor_read(USB_VOL + 3 * USB_BLOCK, d, 2 * USB_BLOCK);
    CHECK(r.status == 0 && r.resid == USB_BLOCK && !memcmp(d, z, USB_BLOCK)
              && !memcmp(d + USB_BLOCK, ref + USB_VOL + 4 * USB_BLOCK, USB_BLOCK),
          "case 11 (Ho > Do): WRITE(10) of 1 block for 2, one block written, residue");
    memset(z, 0x44, sizeof(z));
    r = write10(3, 2, USB_BLOCK, z);
    nor_read(USB_VOL + 3 * USB_BLOCK, d, USB_BLOCK);
    CHECK(r.status == 2 && d[0] == 0x33, "case 13 (Ho < Do): WRITE(10) of 2 blocks for 1, phase error, nothing written");
    r = write10(3, 1, USB_BLOCK, z);
    nor_read(USB_VOL + 3 * USB_BLOCK, d, USB_BLOCK);
    CHECK(r.status == 0 && !r.resid && !memcmp(d, z, USB_BLOCK), "case 12 (Ho = Do)");
    // the host ends the data of a WRITE(10) of 2 blocks with a short packet in the second
    uint8_t c[31];
    memset(z, 0x55, sizeof(z));
    rw10(wr, 0x2A, 3, 2);
    cbw_make(c, wr, 10, 0, 2 * USB_BLOCK);
    r = (res_t){-1, 0, 0, 0};
    CHECK(h_out(c, 31) == 0 && h_out(z, USB_BLOCK + 100) == 0, "WRITE(10) of 2 blocks, the host sends 4196 B");
    csw_in(&r);
    nor_read(USB_VOL + 3 * USB_BLOCK, d, 2 * USB_BLOCK);
    CHECK(r.status == 2 && r.resid == USB_BLOCK && !memcmp(d, z, USB_BLOCK)
              && !memcmp(d + USB_BLOCK, ref + USB_VOL + 4 * USB_BLOCK, USB_BLOCK),
          "data ended early: phase error, the first block written, residue of the second (%d, %u)", r.status, r.resid);
    r = cmd(tur, 6, 0, 0, NULL);
    CHECK(r.status == 0, "case 1 (Hn = Dn)");
    nor_read(0, ref, NOR_SIZE);
}

// A CBW the device must refuse: both pipes stall until a reset recovery
static void bad_cbw(const uint8_t *c, uint32_t n, const char *what)
{
    uint8_t d[64];
    const uint8_t tur[6] = {0};
    uint8_t ok[31];
    int r = h_out(c, n);
    CHECK(r == 0 && stall_in && stall_out, "%s: both pipes stall", what);
    clear_halt(USB_EP_IN);
    clear_halt(USB_EP_OUT);
    CHECK(stall_in && stall_out, "%s: ClearFeature does not end the stall", what);
    cbw_make(ok, tur, 6, 0, 0);
    CHECK(h_out(ok, 31) == -1, "%s: the next CBW is stalled", what);
    CHECK(control(0x82, 0, 0, 0x81, 2, d) == 2 && d[0] == 1, "%s: GET_STATUS shows the halt", what);
    CHECK(bot_reset() == 0, "%s: Bulk-Only Mass Storage Reset", what);
    clear_halt(USB_EP_IN);
    clear_halt(USB_EP_OUT);
    CHECK(!stall_in && !stall_out, "%s: the halts clear after the reset", what);
    res_t s = cmd(tur, 6, 0, 0, NULL);
    CHECK(s.status == 0, "%s: TEST UNIT READY works again", what);
}

static void recovery(uint8_t *ref)
{
    uint8_t c[64], d[2 * USB_BLOCK];
    const uint8_t tur[6] = {0};
    printf("invalid CBWs, Bulk-Only Mass Storage Reset\n");
    cbw_make(c, tur, 6, 0, 0);
    c[0] = 'X';
    bad_cbw(c, 31, "wrong signature");
    cbw_make(c, tur, 6, 0, 0);
    bad_cbw(c, 32, "CBW of 32 B");
    bad_cbw(c, 30, "CBW of 30 B");
    cbw_make(c, tur, 6, 0, 0);
    c[13] = 1;
    bad_cbw(c, 31, "LUN 1");
    cbw_make(c, tur, 6, 0, 0);
    c[14] = 17;
    bad_cbw(c, 31, "command block of 17 B");
    cbw_make(c, tur, 6, 0, 0);
    c[12] = 0x40;
    bad_cbw(c, 31, "reserved flag bit");
    // a reset in the middle of a READ: no stale data afterwards
    uint8_t rd[10];
    rw10(rd, 0x28, 20, 2);
    cbw_make(c, rd, 10, 1, 2 * USB_BLOCK);
    CHECK(h_out(c, 31) == 0 && h_in(d, 640) == 640, "READ(10) of 2 blocks, the host reads 640 B");
    CHECK(bot_reset() == 0, "Bulk-Only Mass Storage Reset during the data phase");
    clear_halt(USB_EP_IN);
    clear_halt(USB_EP_OUT);
    res_t r = read10(30, 1, USB_BLOCK, d);
    CHECK(r.status == 0 && !memcmp(d, ref + USB_VOL + 30 * USB_BLOCK, USB_BLOCK), "the next READ(10) is right");
    // a reset in the middle of a WRITE: the next CBW is taken
    uint8_t wr[10];
    rw10(wr, 0x2A, 20, 2);
    cbw_make(c, wr, 10, 0, 2 * USB_BLOCK);
    memset(d, 0x99, sizeof(d));
    CHECK(h_out(c, 31) == 0 && h_out(d, 1024) == 0, "WRITE(10) of 2 blocks, the host sends 1 KB");
    CHECK(bot_reset() == 0, "Bulk-Only Mass Storage Reset during the data phase");
    clear_halt(USB_EP_IN);
    clear_halt(USB_EP_OUT);
    r = cmd(tur, 6, 0, 0, NULL);
    CHECK(r.status == 0, "TEST UNIT READY after it");
    nor_read(USB_VOL + 20 * USB_BLOCK, d, USB_BLOCK);
    CHECK(!memcmp(d, ref + USB_VOL + 20 * USB_BLOCK, USB_BLOCK), "the block of the broken WRITE is unchanged");
    // a USB reset and a new configuration in the middle of a command
    cbw_make(c, rd, 10, 1, 2 * USB_BLOCK);
    CHECK(h_out(c, 31) == 0, "READ(10), then a USB reset");
    usbd_ev_t e = {{0}, 0, 0};
    push(USBD_RESET, &e);
    run();
    CHECK(!usb_ram.config && !open_in && !open_out, "USB reset: not configured");
    CHECK(control(0x00, 9, 1, 0, 0, NULL) == 0, "SET_CONFIGURATION 1");
    r = read10(30, 1, USB_BLOCK, d);
    CHECK(r.status == 0 && !memcmp(d, ref + USB_VOL + 30 * USB_BLOCK, USB_BLOCK), "READ(10) after the new configuration");
    CHECK(control(0x00, 9, 0, 0, 0, NULL) == 0 && !open_in && !open_out, "SET_CONFIGURATION 0 closes the endpoints");
    CHECK(control(0x00, 9, 1, 0, 0, NULL) == 0, "SET_CONFIGURATION 1 again");
    // SET_FEATURE(ENDPOINT_HALT) by the host, cleared again
    CHECK(control(0x02, 3, 0, 0x81, 0, NULL) == 0 && stall_in, "SET_FEATURE(ENDPOINT_HALT) of 0x81");
    clear_halt(USB_EP_IN);
    r = cmd(tur, 6, 0, 0, NULL);
    CHECK(!stall_in && r.status == 0, "cleared, TEST UNIT READY");
}

static void eject(void)
{
    const uint8_t tur[6] = {0}, inq[6] = {0x12, 0, 0, 0, 36, 0};
    const uint8_t pa[6] = {0x1E, 0, 0, 0, 0, 0}, stop[6] = {0x1B, 0, 0, 0, 2, 0};
    const uint8_t halt[6] = {0x1B, 0, 0, 0, 0, 0}, start[6] = {0x1B, 0, 0, 0, 1, 0};
    const uint8_t idle[6] = {0x1B, 0, 0, 0, 0x20, 0};
    uint8_t d[64];
    res_t r;
    printf("the host lets go: stop, eject, configuration 0\n");
    CHECK(!usb_released(), "not released yet");
    r = cmd(idle, 6, 0, 0, NULL);
    CHECK(r.status == 0 && !usb_released(), "START STOP UNIT with a power condition is taken, nothing else");
    r = cmd(halt, 6, 0, 0, NULL);
    CHECK(r.status == 0 && usb_released(), "START STOP UNIT stop without LoEj releases the drive");
    r = cmd(tur, 6, 0, 0, NULL);
    CHECK(r.status == 1 && req_sense() == 0x023A00, "then TEST UNIT READY: NOT READY, MEDIUM NOT PRESENT");
    r = cmd(start, 6, 0, 0, NULL);
    CHECK(r.status == 0 && !usb_released(), "START STOP UNIT start undoes it");
    r = cmd(tur, 6, 0, 0, NULL);
    CHECK(r.status == 0, "TEST UNIT READY again");
    CHECK(control(0x00, 9, 0, 0, 0, NULL) == 0 && usb_released(), "SET_CONFIGURATION 0 after 1 releases the drive");
    CHECK(control(0x00, 9, 0, 0, 0, NULL) == 0 && !usb_released(), "SET_CONFIGURATION 0 when not configured does not");
    CHECK(control(0x00, 9, 1, 0, 0, NULL) == 0 && !usb_released(), "SET_CONFIGURATION 1 again");
    CHECK(control(0x00, 9, 0, 0, 0, NULL) == 0 && usb_released(), "SET_CONFIGURATION 0 again");
    usbd_ev_t e = {{0}, 0, 0};
    push(USBD_RESET, &e);
    run();
    CHECK(!usb_released(), "a USB reset takes it back");
    CHECK(control(0x00, 9, 1, 0, 0, NULL) == 0, "SET_CONFIGURATION 1");
    r = cmd(pa, 6, 0, 0, NULL);
    CHECK(r.status == 0, "ALLOW MEDIUM REMOVAL");
    r = cmd(stop, 6, 0, 0, NULL);
    CHECK(r.status == 0 && usb_released(), "START STOP UNIT with LoEj: ejected once its CSW is out");
    r = cmd(tur, 6, 0, 0, NULL);
    CHECK(r.status == 1 && req_sense() == 0x023A00, "TEST UNIT READY: NOT READY, MEDIUM NOT PRESENT");
    r = read10(0, 1, USB_BLOCK, d);
    CHECK(r.status == 1 && r.stalled && req_sense() == 0x023A00, "READ(10): NOT READY");
    r = cmd(inq, 6, 1, 36, d);
    CHECK(r.status == 0, "INQUIRY still answers");
}

static int suite(void)
{
    uint8_t *ref = nor_copy();
    enumerate();
    mount_sequence(ref);
    read_write(ref);
    errors(ref);
    phases(ref);
    recovery(ref);
    eject();
    free(ref);
    printf(fails ? "%d FAILED\n" : "all passed\n", fails);
    return fails != 0;
}

// ---- file level steps

static int save(const char *path)
{
    uint8_t *m = nor_copy();
    FILE *f = fopen(path, "wb");
    int ok = f && fwrite(m, 1, NOR_SIZE, f) == NOR_SIZE;
    if (f)
        fclose(f);
    free(m);
    if (!ok)
        printf("cannot write %s\n", path);
    return !ok;
}

// The drive started, reset, addressed and configured, ready for commands
static void attach(void)
{
    usb_start();
    usbd_ev_t e = {{0}, 0, 0};
    push(USBD_RESET, &e);
    run();
    control(0x00, 5, 3, 0, 0, NULL);
    control(0x00, 9, 1, 0, 0, NULL);
}

static uint32_t prog_n, prog_done, prog_total, prog_bad;

static void progress(uint32_t done, uint32_t total)
{
    prog_bad += done < prog_done || done > total || (prog_n && total != prog_total);
    prog_n++;
    prog_done = done;
    prog_total = total;
}

static int format(const char *out)
{
    static uint8_t buf[USB_BLOCK], d[USB_BLOCK], b[USB_BLOCK];
    fat_vol_t v;
    nor_count_t n0 = nor_count;
    int e = vol_format(buf, progress);
    printf("vol_format: %d, %llu sector and %llu block erases\n", e,
           (unsigned long long)(nor_count.erase4k - n0.erase4k),
           (unsigned long long)(nor_count.erase64k - n0.erase64k));
    if (e || save(out))
        return 1;
    if (fat_mount(&v, USB_VOL)) {
        printf("no FAT12 volume after the format\n");
        return 1;
    }
    // the whole 4 KB sectors of the clusters, the last of them written
    uint32_t d0 = (v.data + USB_BLOCK - 1) & ~(USB_BLOCK - 1);
    uint32_t end = (v.data + ((uint32_t)v.nclus << v.cshift)) & ~(USB_BLOCK - 1);
    uint32_t lba = (end - USB_VOL) / USB_BLOCK - 1;
    CHECK(prog_n && !prog_bad && prog_done == prog_total && prog_total >= end - d0,
          "the progress rises to its total %u, which covers the %u B of the clusters", prog_total, end - d0);
    for (uint32_t i = 0; i < USB_BLOCK; i++)
        d[i] = (uint8_t)(i * 7 + 3);
    attach();
    n0 = nor_count;
    res_t r = write10(lba, 1, USB_BLOCK, d);
    nor_file_peek(USB_VOL + lba * USB_BLOCK, b, USB_BLOCK);
    CHECK(r.status == 0 && !r.resid && !memcmp(b, d, USB_BLOCK), "WRITE(10) of block %u after the format", lba);
    CHECK(nor_count.erase4k == n0.erase4k && nor_count.erase64k == n0.erase64k, "block %u needs no erase", lba);
    printf("WRITE(10) of block %u: %llu erases\n", lba,
           (unsigned long long)(nor_count.erase4k - n0.erase4k + nor_count.erase64k - n0.erase64k));
    return fails != 0;
}

static int replay(const char *rec)
{
    FILE *f = fopen(rec, "rb");
    static uint8_t d[32 * USB_BLOCK];
    uint32_t lba, first = 0, n = 0, cmds = 0, blocks = 0;
    if (!f) {
        printf("cannot read %s\n", rec);
        return 1;
    }
    attach();
    for (int more = 1; more;) {
        more = fread(&lba, 4, 1, f) == 1;
        if (n && (!more || lba != first + n || n == 32)) {
            res_t r = write10(first, n, n * USB_BLOCK, d);
            CHECK(r.status == 0 && !r.resid, "WRITE(10) of %u blocks at %u", n, first);
            cmds++;
            blocks += n;
            n = 0;
        }
        if (!more)
            break;
        if (!n)
            first = lba;
        if (fread(d + n * USB_BLOCK, USB_BLOCK, 1, f) != 1) {
            printf("%s: short record\n", rec);
            fclose(f);
            return 1;
        }
        n++;
    }
    fclose(f);
    printf("replayed %u blocks in %u WRITE(10) commands, %llu sector and %llu block erases\n", blocks, cmds,
           (unsigned long long)nor_count.erase4k, (unsigned long long)nor_count.erase64k);
    return fails != 0;
}

int main(int argc, char **argv)
{
    static uint8_t buf[USB_BLOCK];
    if (argc < 3 || nor_file_open(argv[2])) {
        fprintf(stderr, "usage: usb_host suite|format|replay|ui|img NOR.img [args]\n");
        return 2;
    }
    const char *c = argv[1];
    if (!strcmp(c, "suite"))
        return suite();
    if (!strcmp(c, "format") && argc == 4)
        return format(argv[3]);
    if (!strcmp(c, "replay") && argc == 5)
        return replay(argv[3]) || save(argv[4]);
    if (!strcmp(c, "ui") && argc == 4) {
        int e = vol_ui(buf);
        printf("vol_ui: %d\n", e);
        return save(argv[3]) || e;
    }
    if (!strcmp(c, "img")) {
        int e = img_init(), v = e ? e : img_verify();
        printf("img_init %d, img_verify %d\n", e, v);
        return e || v;
    }
    fprintf(stderr, "usb_host: unknown command %s\n", c);
    return 2;
}
