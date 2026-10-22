// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Host test of the DJI RC-N1 game pad driver (dmc_dji.c) against a fake usbd: the enumeration
// as the phone, the AOA handshake and the switch to the accessory, the DUML frames from a log
// of RCPROBE on the meter and made-up stick positions and buttons, the pad timeout, the bus
// reset fallback and the interrupt storm guard. With DJI_POLL (the default) also the wrapped
// 06/01 requests against the frames rcprobe sent and a 06/01 reply from a second log of
// RCPROBE, built with DJI_POLL=0 that no request goes out. Prints "ok"/"FAIL" lines, exits 1
// on a failure.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dmc_dji.h"
#include "pad.h"
#include "platform.h"
#include "usbd.h"

#ifndef DJI_POLL
#define DJI_POLL 1
#endif

volatile pad_t dmc_pad;

static int failures;

static void check(int ok, const char *what)
{
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok)
        failures++;
}

// ---- platform model

static uint32_t now;
uint32_t plat_ms(void) { return now; }

static struct {
    uint32_t addr, val;
} regs[32];
static int nregs;

volatile uint32_t *reg_model(uint32_t addr)
{
    for (int i = 0; i < nregs; i++)
        if (regs[i].addr == addr)
            return &regs[i].val;
    if (nregs == 32) {
        fprintf(stderr, "register file full\n");
        exit(2);
    }
    regs[nregs].addr = addr;
    regs[nregs].val = 0;
    return &regs[nregs++].val;
}

static int masked;
uint32_t irq_save(void)
{
    masked++;
    return 0;
}
void irq_restore(uint32_t m)
{
    (void)m;
    masked--;
}

#define ISER2 0xE000E108u
#define ICER2 0xE000E188u
#define IPR67 0xE000E443u  // REG8 of the model: the byte lands in the word of its address

// ---- fake usbd: a queue of events, the calls recorded

enum { Q_EV, Q_DATA };  // Q_DATA: an OUT packet popped without its completion entry
typedef struct {
    int kind, t;
    uint8_t setup[8];
    uint8_t ep;
    uint8_t data[64];
    uint32_t n;
} qev_t;

static qev_t q[16];
static int qh, qt;
static int n_init, n_conn, n_off, n_irq_on, n_send, n_stall, n_recv, n_xfer_out, n_xfer_in, suspend;
static int addr = -1, open_in, open_out;
static uint8_t sent[64];
static uint32_t sent_n, recv_n, in_n;
static uint8_t in_data[64];

void usbd_init(void)
{
    n_init++;
    memset(&usbd_ram, 0, sizeof(usbd_ram));
    open_in = open_out = 0;
}
void usbd_connect(void) { n_conn++; }
void usbd_off(void) { n_off++; }
void usbd_irq_on(void) { n_irq_on++; }
int usbd_irq_done(void)
{
    int s = suspend;
    suspend = 0;
    return s;
}

int usbd_poll(usbd_ev_t *e)
{
    while (qh != qt) {
        qev_t *v = &q[qh++ & 15];
        if (v->t == USBD_DONE && v->ep == 0x01) {
            // what rx() does: the packet into the waiting transfer
            if (!usbd_ram.out)
                continue;
            if (v->kind == Q_DATA || !usbd_ram.out_pos) {
                memcpy(usbd_ram.out, v->data, v->n);
                usbd_ram.out_pos = v->n;
            }
            if (v->kind == Q_DATA)
                continue;
            e->n = usbd_ram.out_pos;
        }
        if (v->t == USBD_SETUP)
            memcpy(e->setup, v->setup, 8);
        e->ep = v->ep;
        if (v->t == USBD_DONE && v->ep == 0)
            e->n = 1;
        return v->t;
    }
    return USBD_NONE;
}

void usbd_ctl_send(const void *data, uint32_t n)
{
    n_send++;
    sent_n = n;
    if (n)
        memcpy(sent, data, n);
}
void usbd_ctl_stall(void) { n_stall++; }
void usbd_ctl_recv(void *buf, uint32_t n)
{
    (void)buf;
    n_recv++;
    recv_n = n;
}
void usbd_set_address(uint8_t a) { addr = a; }
void usbd_open(uint8_t ep)
{
    if (ep & 0x80)
        open_in = 1;
    else
        open_out = 1;
}
void usbd_close(uint8_t ep)
{
    if (ep & 0x80) {
        open_in = 0;
    } else {
        open_out = 0;
        usbd_ram.out = 0;
        usbd_ram.out_pos = 0;
    }
}
void usbd_xfer(uint8_t ep, void *buf, uint32_t n)
{
    if (ep & 0x80) {
        n_xfer_in++;
        in_n = n;
        memcpy(in_data, buf, n);
        return;
    }
    n_xfer_out++;
    usbd_ram.out = buf;
    usbd_ram.out_len = n;
    usbd_ram.out_pos = 0;
}

static void push(int t, const uint8_t *setup, uint8_t ep, const uint8_t *data, uint32_t n, int kind)
{
    qev_t *v = &q[qt++ & 15];
    memset(v, 0, sizeof(*v));
    v->kind = kind;
    v->t = t;
    if (setup)
        memcpy(v->setup, setup, 8);
    v->ep = ep;
    if (data)
        memcpy(v->data, data, n);
    v->n = n;
}

static void isr(void) { OTGFS1_IRQHandler(); }

static void req(uint8_t type, uint8_t r, uint16_t val, uint16_t len)
{
    uint8_t s[8] = {type, r, (uint8_t)val, (uint8_t)(val >> 8),
                    0,    0, (uint8_t)len, (uint8_t)(len >> 8)};
    push(USBD_SETUP, s, 0, 0, 0, Q_EV);
    isr();
}

static void packet(const uint8_t *p, uint32_t n)
{
    push(USBD_DONE, 0, 0x01, p, n, Q_EV);
    isr();
}

// ---- DUML frames, built by the test's own CRCs (checked against the logged frames)

static uint32_t crc8(const uint8_t *p, uint32_t n)
{
    uint32_t c = 0x77;
    while (n--) {
        c ^= *p++;
        for (int i = 0; i < 8; i++)
            c = c & 1 ? (c >> 1) ^ 0x8C : c >> 1;
    }
    return c;
}

static uint32_t crc16(const uint8_t *p, uint32_t n)
{
    uint32_t c = 0x3692;
    while (n--) {
        c ^= *p++;
        for (int i = 0; i < 8; i++)
            c = c & 1 ? (c >> 1) ^ 0x8408 : c >> 1;
    }
    return c;
}

// DUML v1 frame into f, returns its length
static uint32_t frame(uint8_t *f, uint8_t src, uint8_t dst, uint8_t type, uint8_t set, uint8_t id,
                      const uint8_t *pl, uint32_t npl)
{
    uint32_t n = 13 + npl;
    f[0] = 0x55;
    f[1] = (uint8_t)n;
    f[2] = (uint8_t)(n >> 8 | 0x04);
    f[3] = (uint8_t)crc8(f, 3);
    f[4] = src;
    f[5] = dst;
    f[6] = f[7] = 0;
    f[8] = type;
    f[9] = set;
    f[10] = id;
    memcpy(f + 11, pl, npl);
    uint32_t c = crc16(f, n - 2);
    f[n - 2] = (uint8_t)c;
    f[n - 1] = (uint8_t)(c >> 8);
    return n;
}

// A packet as the RC sends it: 55 CC 49 57, u32 length, the frame, zeros
static void wrapped(uint8_t *p, const uint8_t *f, uint32_t n)
{
    static const uint8_t h[4] = {0x55, 0xCC, 0x49, 0x57};
    memset(p, 0, 64);
    memcpy(p, h, 4);
    p[4] = (uint8_t)n;
    memcpy(p + 8, f, n);
}

// The push 06/05 with raw values right X, right Y, left Y, left X, dial and button bytes 11, 12
static void push0605b(uint8_t *p, int rh, int rv, int lv, int lh, int dial, int b11, int b12)
{
    int v[5] = {rh, rv, lv, lh, dial};
    uint8_t pl[13] = {[11] = (uint8_t)b11, [12] = (uint8_t)b12}, f[64];
    for (int i = 0; i < 5; i++) {
        pl[2 * i] = (uint8_t)v[i];
        pl[2 * i + 1] = (uint8_t)(v[i] >> 8);
    }
    wrapped(p, f, frame(f, 0x06, 0x02, 0x00, 0x06, 0x05, pl, sizeof(pl)));
}

static void push0605(uint8_t *p, int rh, int rv, int lv, int lh, int dial)
{
    push0605b(p, rh, rv, lv, lh, dial, 0, 0);
}

static int axes(int lx, int ly, int rx, int ry, int dial)
{
    return dmc_pad.axis[PAD_LX] == lx && dmc_pad.axis[PAD_LY] == ly && dmc_pad.axis[PAD_RX] == rx &&
           dmc_pad.axis[PAD_RY] == ry && dmc_pad.axis[PAD_DIAL] == dial;
}

// Packets of a probe log, first 36 B, zeros follow. The identifier in the payload of 09/84 is
// replaced by zeros.
static const uint8_t log_0605[36] = {0x55, 0xcc, 0x49, 0x57, 0x1a, 0x00, 0x00, 0x00, 0x55,
                                     0x1a, 0x04, 0xb1, 0x06, 0x02, 0x00, 0x00, 0x00, 0x06,
                                     0x05, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04,
                                     0x00, 0x04, 0x00, 0x00, 0x00, 0x67, 0x96, 0x00, 0x00};
static const uint8_t log_0943[36] = {0x55, 0xcc, 0x49, 0x57, 0x0e, 0x00, 0x00, 0x00,
                                     0x55, 0x0e, 0x04, 0x66, 0x0e, 0x02, 0xad, 0x00,
                                     0x00, 0x09, 0x43, 0x02, 0x2d, 0x30, 0x00, 0x00};
static const uint8_t log_0984[36] = {
    0x55, 0xcc, 0x49, 0x57, 0xb1, 0x00, 0x00, 0x00, 0x55, 0xb1, 0x04, 0x7c, 0x0e, 0xce, 0xaa, 0x00,
    0x00, 0x09, 0x84, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t log_061e[36] = {0x55, 0xcc, 0x49, 0x57, 0x13, 0x00, 0x00, 0x00, 0x55,
                                     0x13, 0x04, 0x03, 0x06, 0x02, 0x01, 0x00, 0x20, 0x06,
                                     0x1e, 0x84, 0x07, 0x00, 0x00, 0x4a, 0x00, 0x35, 0x5f};

static void enumerate(uint16_t pid)
{
    char m[80];
    int s0 = n_send;
    push(USBD_RESET, 0, 0, 0, 0, Q_EV);
    isr();
    req(0x00, 5, 1, 0);
    check(addr == 1 && n_send == s0 + 1 && sent_n == 0, "SET_ADDRESS: address and status stage");
    req(0x80, 6, 0x100, 8);
    check(sent_n == 8 && sent[0] == 18 && sent[1] == 1 && sent[7] == 64,
          "GET_DESCRIPTOR device, 8 B");
    req(0x80, 6, 0x100, 18);
    snprintf(m, sizeof(m), "GET_DESCRIPTOR device, 18 B, 18D1:%04X", pid);
    check(sent_n == 18 && sent[8] == 0xD1 && sent[9] == 0x18 && sent[10] == (pid & 0xFF) &&
              sent[11] == pid >> 8,
          m);
    req(0x80, 6, 0x200, 9);
    check(sent_n == 9 && sent[2] == 32 && sent[4] == 1, "GET_DESCRIPTOR configuration, 9 B");
    req(0x80, 6, 0x200, 32);
    check(sent_n == 32 && sent[9 + 5] == 0xFF && sent[18 + 2] == 0x81 && sent[25 + 2] == 0x01,
          "GET_DESCRIPTOR configuration, 32 B, vendor interface, bulk 0x81 and 0x01");
    int st = n_stall;
    req(0x80, 6, 0x301, 255);
    check(n_stall == st + 1, "GET_DESCRIPTOR string: stalled");
    int x = n_xfer_out;
    req(0x00, 9, 1, 0);
    check(open_in && open_out && n_xfer_out == x + 1 && usbd_ram.out && usbd_ram.out_len == 64,
          "SET_CONFIGURATION 1: endpoints open, bulk OUT waits for 64 B");
    req(0x80, 8, 0, 1);
    check(sent_n == 1 && sent[0] == 1, "GET_CONFIGURATION: 1");
}

int main(void)
{
    // ---- start: phone identity
    dji_start();
    check(n_init == 1 && n_irq_on == 1 && n_conn == 1,
          "dji_start: usbd_init, interrupt mode, connect");
    check((*reg_model(ISER2) & 8u) && (*reg_model(IPR67) & 0xFFu) == 0x80u,
          "dji_start: IRQ 67 enabled with the shared priority");
    enumerate(0x4EE1);

    // ---- AOA
    req(0xC0, 51, 0, 2);
    check(sent_n == 2 && sent[0] == 2 && sent[1] == 0, "AOA GET_PROTOCOL: 2");
    int s0 = n_send, r0 = n_recv;
    req(0x40, 52, 0, 4);
    check(n_recv == r0 + 1 && recv_n == 1 && n_send == s0,
          "AOA SEND_STRING: data stage received a packet at a time, no status yet");
    push(USBD_DONE, 0, 0, 0, 0, Q_EV);
    isr();
    check(n_send == s0 + 1 && sent_n == 0, "AOA SEND_STRING: status stage after its packet");
    s0 = n_send;
    req(0x40, 52, 0, 100);
    push(USBD_DONE, 0, 0, 0, 0, Q_EV);
    isr();
    check(n_recv == r0 + 3 && n_send == s0, "AOA SEND_STRING of 100 B: a second packet asked for");
    push(USBD_DONE, 0, 0, 0, 0, Q_EV);
    isr();
    check(n_send == s0 + 1 && sent_n == 0,
          "AOA SEND_STRING of 100 B: status stage after two packets");
    req(0x40, 53, 0, 0);
    check(sent_n == 0 && n_off == 0, "AOA START: status stage, still on the bus");
    now += 49;
    dji_tic();
    check(n_off == 0, "49 ms after START: still on the bus");
    now += 1;
    dji_tic();
    check(n_off == 1 && (*reg_model(ICER2) & 8u) && !masked,
          "50 ms after START: off the bus, IRQ off");
    now += 199;
    dji_tic();
    check(n_init == 1, "199 ms later: still off");
    now += 1;
    dji_tic();
    check(n_init == 2 && n_conn == 2 && n_irq_on == 2, "200 ms later: on the bus again");
    enumerate(0x2D00);

    // ---- DUML
    uint8_t p[64] = {0};
    memcpy(p, log_0605, sizeof(log_0605));
    check(crc8(p + 8, 3) == p[11] && crc16(p + 8, 24) == (uint32_t)(p[32] | p[33] << 8),
          "test CRCs agree with the logged 06/05 frame");
    packet(p, 64);
    check(dmc_pad.buttons == PAD_PRESENT && axes(0, 0, 0, 0, 0),
          "logged 06/05 at rest: pad present, centred");
    check(n_xfer_out >= 1 && usbd_ram.out && usbd_ram.out_pos == 0,
          "bulk OUT waits for the next packet");
    push0605(p, 1684, 1500, 364, 1124, 900);
    packet(p, 64);
    check(axes(100, -660, 660, 476, -124) && dmc_pad.buttons == PAD_PRESENT,
          "06/05: axes in pad order, right stick up is no button");
    // SHUTTER pressed, as RCPROBE logged it
    static const uint8_t shutter[26] = {0x55, 0x1a, 0x04, 0xb1, 0x06, 0x02, 0x00, 0x00, 0x00,
                                        0x06, 0x05, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00,
                                        0x04, 0x00, 0x04, 0x00, 0x00, 0x40, 0x63, 0xd4};
    wrapped(p, shutter, 26);
    packet(p, 64);
    check(axes(0, 0, 0, 0, 0) && dmc_pad.buttons == (PAD_PRESENT | PAD_FIRE | PAD_ENTER),
          "logged SHUTTER: fire and enter");
    push0605b(p, 1024, 1024, 1024, 1024, 1024, 0, 0x01);
    packet(p, 64);
    check(dmc_pad.buttons == (PAD_PRESENT | PAD_USE), "PHOTO/VIDEO: use");
    push0605b(p, 1024, 1024, 1024, 1024, 1024, 0, 0x10);
    packet(p, 64);
    check(dmc_pad.buttons == (PAD_PRESENT | PAD_ENTER), "Fn: enter");
    push0605b(p, 1024, 1024, 1024, 1024, 1024, 0x08, 0);
    packet(p, 64);
    check(dmc_pad.buttons == (PAD_PRESENT | PAD_MENU), "PAUSE/RTH: menu");
    push0605b(p, 1024, 1024, 1024, 1024, 1024, 0x30, 0);
    packet(p, 64);
    check(dmc_pad.buttons == PAD_PRESENT, "flight mode switch: ignored");
    push0605b(p, 1024, 1024, 1024, 1024, 1024, 0x08, 0x51);
    packet(p, 64);
    check(dmc_pad.buttons == (PAD_PRESENT | PAD_FIRE | PAD_ENTER | PAD_USE | PAD_MENU),
          "SHUTTER, PHOTO/VIDEO, Fn and PAUSE/RTH together");
    dmc_pad.axis[PAD_LX] = 77;
    push0605(p, 1684, 1684, 1684, 1684, 1684);
    p[30] ^= 1;
    packet(p, 64);
    check(dmc_pad.axis[PAD_LX] == 77, "06/05 with a bad CRC16: dropped");
    uint8_t f[64], pl[13] = {0};
    pl[7] = 0x06;  // left X 1536
    for (int i = 0; i < 5; i++)
        if (i != 3)
            pl[2 * i + 1] = 4;
    uint32_t n = frame(f, 0x06, 0x02, 0x00, 0x06, 0x05, pl, 13);
    memset(p, 0, 64);
    memcpy(p, f, n);
    packet(p, 64);
    check(dmc_pad.axis[PAD_LX] == 512, "06/05 without the wrapper at offset 0: taken");
    memset(p, 0, 64);
    memcpy(p, log_061e + 8, 19);
    pl[7] = 0x02;  // left X 512
    n = frame(f, 0x06, 0x02, 0x00, 0x06, 0x05, pl, 13);
    memcpy(p + 19, f, n);
    packet(p, 64);
    check(dmc_pad.axis[PAD_LX] == -512, "06/1E then 06/05 in one packet: the push taken");
    dmc_pad.axis[PAD_LX] = 77;
    memset(p, 0, 64);
    memcpy(p, log_0943, sizeof(log_0943));
    packet(p, 64);
    memcpy(p, log_0984, sizeof(log_0984));
    packet(p, 64);
    memcpy(p, log_061e, sizeof(log_061e));
    packet(p, 64);
    packet(p, 0);
    check(dmc_pad.axis[PAD_LX] == 77 && dmc_pad.buttons == PAD_PRESENT,
          "logged 09/43, 09/84 start, 06/1E and a zero-length packet: no pad change");
    // a packet whose completion comes in the next interrupt
    push0605(p, 1024, 1024, 1024, 1324, 1024);
    push(USBD_DONE, 0, 0x01, p, 64, Q_DATA);
    isr();
    check(dmc_pad.axis[PAD_LX] == 300,
          "packet before its completion entry: parsed at the end of the handler");
    dmc_pad.axis[PAD_LX] = 77;
    int x = n_xfer_out;
    push(USBD_DONE, 0, 0x01, 0, 0, Q_EV);
    isr();
    check(dmc_pad.axis[PAD_LX] == 77 && n_xfer_out == x + 1 && usbd_ram.out_pos == 0,
          "its completion: not parsed again, the next transfer started");
    push0605(p, 1024, 1024, 1024, 1424, 1024);
    packet(p, 64);
    check(dmc_pad.axis[PAD_LX] == 400, "the next packet: parsed");

#if !DJI_POLL
    check(n_xfer_in == 0 && in_n == 0, "no request sent without DJI_POLL");
#else
    // ---- 06/01 polling, wrapped (frames rcprobe sent, from its logs)
    static const uint8_t req0[21] = {0x55, 0xcc, 0x49, 0x57, 0x0d, 0x00, 0x00,
                                     0x00, 0x55, 0x0d, 0x04, 0x33, 0x02, 0x06,
                                     0x00, 0x20, 0x40, 0x06, 0x01, 0xdf, 0x27};
    static const uint8_t req1[13] = {0x55, 0x0d, 0x04, 0x33, 0x02, 0x06, 0x01,
                                     0x20, 0x40, 0x06, 0x01, 0x9b, 0x2c};
    check(crc8(req0 + 8, 4) == 0 && crc16(req0 + 8, 13) == 0 && crc16(req1, 13) == 0,
          "the logged requests carry valid CRCs");
    // the handler sends whenever 2 tics passed and the last request went out
    push(USBD_DONE, 0, 0x81, 0, 0, Q_EV);
    isr();
    now += 57;
    int in0 = n_xfer_in;
    packet(p, 0);
    int first = !memcmp(in_data, req0, 21) ? 0 : 1;
    check(n_xfer_in == in0 + 1 && in_n == 21 && !memcmp(in_data, req0, 8) &&
              !memcmp(in_data + 8, first ? req1 : req0 + 8, 13),
          "after 2 tics the handler sends a request in the wrapper, a frame rcprobe sent (0x2000 "
          "bare, 0x2001 wrapped)");
    now += 57;
    packet(p, 0);
    check(n_xfer_in == in0 + 1, "no new request while the last one is not taken");
    push(USBD_DONE, 0, 0x81, 0, 0, Q_EV);
    isr();
    check(n_xfer_in == in0 + 2 && !memcmp(in_data + 8, first ? req0 + 8 : req1, 13),
          "after it went out: the next one, the other sequence number");
    push(USBD_DONE, 0, 0x81, 0, 0, Q_EV);
    now += 56;
    isr();
    check(n_xfer_in == in0 + 2, "not within 2 tics");
    // a push with SHUTTER held, then a reply from a log of RCPROBE: right X 1684, left X 364
    push0605b(p, 1024, 1024, 1024, 1024, 1024, 0, 0x40);
    packet(p, 64);
    static const uint8_t rpl[38] = {0x55, 0x26, 0x04, 0xd1, 0x0e, 0x02, 0x55, 0x20, 0x80, 0x06,
                                    0x01, 0x00, 0x00, 0x94, 0x06, 0x00, 0x00, 0x04, 0x00, 0x00,
                                    0x04, 0x00, 0x6c, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00, 0x04,
                                    0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x5f, 0x77};
    wrapped(p, rpl, 38);
    packet(p, 64);
    check(axes(-660, 0, 660, 0, 0) && dmc_pad.buttons == (PAD_PRESENT | PAD_FIRE | PAD_ENTER),
          "logged 06/01 reply: its axes, the buttons of the last push");
#endif

    // ---- timeout, reset, suspend
    push0605(p, 1024, 1024, 1024, 1024, 1024);
    packet(p, 64);
    now += 500;
    dji_tic();
    check(dmc_pad.buttons == PAD_PRESENT, "500 ms without a push: still present");
    now += 1;
    dji_tic();
    check(dmc_pad.buttons == 0, "501 ms without a push: lost");
    packet(p, 64);
    suspend = 1;
    isr();
    check(dmc_pad.buttons == 0, "suspend: lost");
    packet(p, 64);
    int off = n_off;
    push(USBD_RESET, 0, 0, 0, 0, Q_EV);
    isr();
    check(dmc_pad.buttons == 0, "bus reset: lost");
    now += 4990;
    dji_tic();
    check(n_off == off, "accessory, 4990 ms after a reset without configuration: waits");
    now += 20;
    dji_tic();
    check(n_off == off + 1, "5010 ms: off the bus");
    now += 200;
    dji_tic();
    check(n_init == 3, "then on the bus again");
    enumerate(0x4EE1);

    // ---- storm guard, stop
    now++;
    for (int i = 0; i < 200; i++)
        isr();
    *reg_model(ICER2) = 0;
    check(n_off == off + 1, "200 interrupts in one millisecond: running");
    isr();
    check((*reg_model(ICER2) & 8u) != 0, "201: IRQ off");
    dji_tic();
    check(n_off == off + 2 && dmc_pad.buttons == 0, "dji_tic after the storm: controller off");
    dji_stop();
    check(n_off == off + 2, "dji_stop when off: nothing");
    dji_start();
    dji_stop();
    check(n_off == off + 3 && !masked, "dji_stop: controller off, interrupts unmasked again");

    printf("%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures != 0;
}
