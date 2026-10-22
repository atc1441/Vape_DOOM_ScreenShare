// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Game pad driver for the DJI RC-N1 remote controller (RC231), play mode of the meter
// (part of DOOM.BIN). The meter is the USB device on the controller's top (phone) port and plays the
// phone of an Android Open Accessory host, as rcprobe.c found it on the meter:
//   1. it enumerates as 18D1:4EE1 with one vendor interface (bulk IN and OUT of 64 B)
//   2. the controller sends GET_PROTOCOL (answered 2), six strings (received and dropped)
//      and START
//   3. 50 ms after START the meter leaves the bus for 200 ms and comes back as the accessory
//      18D1:2D00, which the controller configures at once
//   4. the controller streams messages over bulk OUT: a header 55 CC 49 57 with a u32 length,
//      one DUML v1 frame, zero padding to 512 B. Every DUML frame that lies within one packet
//      is checked (CRC8 and CRC16), two of them go to pad_set(), everything else is dropped:
//      - the push 06/05 from the RC (06), about every 125 ms: five u16 at payload 0, 2, .. 8
//        (right X, right Y, left Y, left X, dial, raw 364..1684, centre 1024, X 1684 right,
//        Y 1684 up), then the buttons in bytes 10 to 12 (dji_buttons)
//      - with DJI_POLL, the reply to the request 06/01 (read the stick channels) the handler
//        sends about every 2 tics, wrapped in the same header (the controller does not answer
//        a bare frame): a status byte, then groups of a byte and a u16, the same axes in the
//        same order. The buttons stay those of the last push.
// pad_lost() on a bus reset, a suspend and 0.5 s without either (dji_tic).
//
// The OTGFS interrupt (IRQ 67, the priority of the DMA interrupts) runs the controller: every
// usbd call but usbd_init, usbd_connect and usbd_off (dji_tic and dji_stop, with the interrupt
// off) is made in the handler, so the controller answers the host while a frame renders or a
// level loads. The packet buffer is on the handler's stack: a waiting bulk OUT transfer is
// pointed at it on entry (one packet per transfer, usbd_ram.out is only written by usbd_poll).
// A packet whose transfer-complete entry is not in the receive FIFO yet is parsed at the end of
// the handler, its completion later only starts the next transfer. More than STORM_MAX entries
// of the handler in one millisecond switch the driver off (dji_tic finishes it).
//
// DJI_POLL=0 builds the driver without the 06/01 requests (the push alone, 8 Hz sticks).

#include <string.h>

#include "at32.h"
#include "dmc_dji.h"
#include "pad.h"
#include "platform.h"
#include "usbd.h"

#ifndef DJI_POLL
#define DJI_POLL 1
#endif

#define STORM_MAX 200u
#define SWITCH_MS 50u    // AOA START to leaving the bus, the status stage goes out first
#define GAP_MS 200u      // off the bus between the identities
#define ACC_MS 5000u     // a bus reset as the accessory without SET_CONFIGURATION: phone again
#define LOST_MS 500u
#define POLL_MS 57u      // 2 tics

// NVIC registers of IRQ 67: set enable, clear enable, clear pending
#define NVIC_SET 0xE000E100u
#define NVIC_CLR 0xE000E180u
#define NVIC_UNPEND 0xE000E280u

enum { P_OFF, P_RUN, P_SWITCH, P_GAP, P_DEAD };

usbd_state_t usbd_ram;

static struct {
    uint32_t t;        // P_SWITCH, P_GAP: since when, P_RUN as the accessory: the bus reset
    uint32_t t_rx;     // last push or reply
    uint16_t left;     // bytes of a vendor request's data stage still to come
    uint16_t storm_ms;
#if DJI_POLL
    uint32_t t_tx;     // last request
    uint8_t tx_busy, seq;
#endif
    uint8_t phase, acc, cfg, early, storm, scratch;
    uint8_t btn;       // PAD_* buttons of the last push
} dji;

// Buttons: payload byte of the push 06/05, its mask, the PAD_* bits. Byte 11 bits 0x30 are
// the flight mode switch (unused), the RC's power button is not reported.
static const struct {
    uint8_t byte, mask, pad;
} dji_buttons[] = {
    {12, 0x40, PAD_FIRE | PAD_ENTER},  // SHUTTER/RECORD: fire in a level, select in the menu
    {12, 0x01, PAD_USE},               // PHOTO/VIDEO
    {12, 0x10, PAD_ENTER},             // Fn
    {11, 0x08, PAD_MENU},              // PAUSE/RTH
};

// Raw axis of the push for each PAD_* axis: right X 0, right Y 1, left Y 2, left X 3, dial 4.
// Raw axis k is the u16 at payload byte 2 k of 06/05, at 2 + 3 k of the 06/01 reply.
static const uint8_t axis_src[PAD_NAXES] = {3, 2, 0, 1, 4};

// Device descriptor of the phone (18D1:4EE1), the accessory patches the product ID (2D00)
static const uint8_t dev_desc[18] = {18, 1, 0x00, 0x02, 0, 0, 0, 64, 0xD1, 0x18, 0xE1, 0x4E,
                                     0x00, 0x01, 1, 2, 3, 1};
// One configuration, self-powered, 100 mA: interface 0 of vendor class with bulk endpoints
// 0x81 and 0x01 of 64 B
static const uint8_t cfg_desc[32] = {
    9, 2, 32, 0, 1, 1, 0, 0xC0, 50,
    9, 4, 0, 0, 2, 0xFF, 0xFF, 0, 4,
    7, 5, 0x81, 2, 64, 0, 0,
    7, 5, 0x01, 2, 64, 0, 0,
};

static void nvic(uint32_t reg)
{
    REG32(reg + IRQ_OTGFS / 32u * 4u) = 1u << IRQ_OTGFS % 32u;
}

// Reflected CRC: DUML's CRC8 (init 0x77, poly 0x8C) and CRC16 (init 0x3692, poly 0x8408).
// Over a frame part with its CRC appended it is 0.
static uint32_t crc(const uint8_t *p, uint32_t n, uint32_t c, uint32_t poly)
{
    while (n--) {
        c ^= *p++;
        for (int i = 0; i < 8; i++)
            c = c & 1u ? c >> 1 ^ poly : c >> 1;
    }
    return c;
}

void dji_rx(const uint8_t *p, uint32_t n)
{
    for (uint32_t i = 0; i + 13u <= n; i++) {
        const uint8_t *f = p + i;
        uint32_t len = f[1] | (f[2] & 3u) << 8, base = 11, step = 2;
        if (f[0] != 0x55 || len < 13u || len > n - i || crc(f, 4, 0x77, 0x8C) ||
            crc(f, len, 0x3692, 0x8408))
            continue;
        i += len - 1u;
        if (f[9] != 6 || len < 26u)
            continue;
        if (f[10] == 5) {
            uint32_t b = 0;
            for (unsigned k = 0; k < sizeof(dji_buttons) / sizeof(dji_buttons[0]); k++)
                if (f[11 + dji_buttons[k].byte] & dji_buttons[k].mask)
                    b |= dji_buttons[k].pad;
            dji.btn = (uint8_t)b;
#if DJI_POLL
        } else if (f[10] == 1 && (f[8] & 0x80u) && len >= 29u) {
            base = 13;  // a status byte, then groups of a byte and a u16
            step = 3;
#endif
        } else {
            continue;
        }
        int16_t a[PAD_NAXES];
        for (unsigned k = 0; k < PAD_NAXES; k++) {
            const uint8_t *v = f + base + step * axis_src[k];
            a[k] = (int16_t)((v[0] | v[1] << 8) - 1024);
        }
        dji.t_rx = plat_ms();
        pad_set(a, dji.btn);
    }
}

#if DJI_POLL
// The request 06/01 (read the stick channels, type 40, no payload) from the app (02) to the RC
// (06) in the header of the controller's messages, with the sequence numbers 0x2000 and 0x2001
// in turn. rcprobe sent these frames, the first one bare and the second one in the header,
// and the controller answered only the requests in the header.
static const uint8_t poll_req[2][21] = {
    {0x55, 0xCC, 0x49, 0x57, 13, 0, 0, 0,
     0x55, 0x0D, 0x04, 0x33, 0x02, 0x06, 0x00, 0x20, 0x40, 0x06, 0x01, 0xDF, 0x27},
    {0x55, 0xCC, 0x49, 0x57, 13, 0, 0, 0,
     0x55, 0x0D, 0x04, 0x33, 0x02, 0x06, 0x01, 0x20, 0x40, 0x06, 0x01, 0x9B, 0x2C},
};

static void poll_send(void)
{
    dji.tx_busy = 1;
    // the packet goes into the transmit FIFO at once
    usbd_xfer(0x81, (void *)poll_req[dji.seq++ & 1u], sizeof(poll_req[0]));
}
#endif

// The requests the controller sends (rcprobe's log) and the usual standard ones. Other IN
// requests are stalled, other OUT requests acknowledged: SET_FEATURE, CLEAR_FEATURE,
// SET_INTERFACE and the vendor requests of AOA (SEND_STRING, START, HID, audio), whose data
// stage is received a packet at a time into one byte and dropped.
static void setup(const uint8_t *s, uint8_t *buf)
{
    uint32_t val = s[2] | s[3] << 8, len = s[6] | s[7] << 8, n = 2;
    const uint8_t *d = buf;
    buf[0] = buf[1] = 0;
    if (s[0] == 0x80 && s[1] == 6) {  // GET_DESCRIPTOR: device, configuration, no strings
        if (val == 0x200) {
            d = cfg_desc;
            n = sizeof(cfg_desc);
        } else if (val == 0x100) {
            memcpy(buf, dev_desc, sizeof(dev_desc));
            if (dji.acc) {
                buf[10] = 0x00;
                buf[11] = 0x2D;
            }
            n = sizeof(dev_desc);
        } else {
            n = 0;
        }
    } else if (s[0] == 0x80 || s[0] == 0x81 || s[0] == 0x82) {
        // GET_STATUS, GET_CONFIGURATION, GET_INTERFACE: 0, the configuration, 0
        buf[0] = s[1] == 8 ? dji.cfg : 0;
    } else if (s[0] == 0xC0 && s[1] == 51) {  // AOA GET_PROTOCOL: version 2
        buf[0] = 2;
    } else if (s[0] & 0x80) {
        n = 0;
    } else {
        n = 1;
        if (s[0] == 0 && s[1] == 5)  // SET_ADDRESS
            usbd_set_address((uint8_t)(val & 0x7F));
        if (s[0] == 0 && s[1] == 9) {  // SET_CONFIGURATION
            dji.cfg = (uint8_t)(val & 1);
            dji.early = 0;
            usbd_close(0x81);
            usbd_close(0x01);
            if (dji.cfg) {
                usbd_open(0x81);
                usbd_open(0x01);
                usbd_xfer(0x01, buf, 64);
#if DJI_POLL
                dji.tx_busy = 0;
#endif
            }
        }
        if (s[0] == 0x40 && s[1] == 53 && !dji.acc) {  // AOA START
            dji.phase = P_SWITCH;
            dji.t = plat_ms();
        }
        if (len) {
            dji.left = (uint16_t)len;
            usbd_ctl_recv(&dji.scratch, 1);
            return;
        }
        len = 0;
    }
    if (!n) {
        usbd_ctl_stall();
        return;
    }
    usbd_ctl_send(d, n < len ? n : len);
}

void OTGFS1_IRQHandler(void)
{
    uint8_t buf[64];
    usbd_ev_t e;
    uint32_t now = plat_ms();
    if ((uint16_t)now != dji.storm_ms) {
        dji.storm_ms = (uint16_t)now;
        dji.storm = 0;
    }
    if (++dji.storm > STORM_MAX) {
        nvic(NVIC_CLR);
        dji.phase = P_DEAD;
        irq_end();
        return;
    }
    if (usbd_ram.out && !usbd_ram.out_pos)
        usbd_ram.out = buf;
    for (int t; (t = usbd_poll(&e)) != USBD_NONE;) {
        if (t == USBD_RESET) {
            dji.cfg = 0;
            pad_lost();
            if (dji.acc && dji.phase == P_RUN)
                dji.t = now | 1u;
        } else if (t == USBD_SETUP) {
            setup(e.setup, buf);
        } else if (e.ep == 0) {
            // a packet of a vendor request's data stage, then the status stage
            dji.left = dji.left > 64u ? (uint16_t)(dji.left - 64u) : 0;
            if (dji.left)
                usbd_ctl_recv(&dji.scratch, 1);
            else
                usbd_ctl_send(0, 0);
        } else if (e.ep == 0x01) {
            if (!dji.early)
                dji_rx(buf, e.n);
            dji.early = 0;
            usbd_xfer(0x01, buf, 64);
#if DJI_POLL
        } else {
            dji.tx_busy = 0;
#endif
        }
    }
    if (usbd_ram.out_pos && !dji.early) {
        dji_rx(buf, usbd_ram.out_pos);
        dji.early = 1;
    }
#if DJI_POLL
    // the controller's own traffic (about 140 packets/s) brings the handler often enough
    if (dji.cfg && dji.acc && !dji.tx_busy && now - dji.t_tx >= POLL_MS) {
        dji.t_tx = now;
        poll_send();
    }
#endif
    if (usbd_irq_done())
        pad_lost();
    irq_end();
}

// Connects as the phone (acc 0) or the accessory
static void up(int acc)
{
    usbd_init();
    dji.acc = (uint8_t)acc;
    dji.cfg = 0;
    dji.t = 0;
    dji.phase = P_RUN;
    usbd_irq_on();
    NVIC_IPR(IRQ_OTGFS) = IRQ_PRIO;
    nvic(NVIC_UNPEND);
    nvic(NVIC_SET);
    usbd_connect();
}

void dji_start(void)
{
    up(0);
}

void dji_stop(void)
{
    if (dji.phase == P_OFF)
        return;
    uint32_t m = irq_save();
    nvic(NVIC_CLR);
    usbd_off();
    irq_restore(m);
    dji.phase = P_OFF;
    pad_lost();
}

// Off the bus for GAP_MS, then up() as the accessory (acc 1) or the phone
static void leave(int acc)
{
    dji_stop();
    dji.acc = (uint8_t)acc;
    dji.phase = P_GAP;
    dji.t = plat_ms();
}

void dji_tic(void)
{
    int p = dji.phase;
    __asm volatile("" ::: "memory");  // dji.t after dji.phase: the handler sets both
    uint32_t now = plat_ms(), dt = now - dji.t;
    if (p == P_OFF)
        return;
    if (p == P_DEAD)
        dji_stop();
    else if (p == P_SWITCH && dt >= SWITCH_MS)
        leave(1);
    else if (p == P_GAP && dt >= GAP_MS)
        up(dji.acc);
    else if (p == P_RUN && dji.acc && !dji.cfg && dji.t && dt >= ACC_MS)
        leave(0);
    // signed: the handler may store a t_rx newer than now
    if ((dmc_pad.buttons & PAD_PRESENT) && (int32_t)(now - dji.t_rx) > (int32_t)LOST_MS)
        pad_lost();
}
