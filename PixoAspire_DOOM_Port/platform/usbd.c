// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// AT32F415 OTGFS in device mode, polled (usbd.h). The controller runs without DMA: received
// packets are popped from the receive FIFO by usbd_poll() into the buffer of their transfer,
// IN packets are pushed into the transmit FIFO of their endpoint whenever it has room.

#include "at32.h"
#include "platform.h"
#include "usbd.h"

#define OTG 0x50000000u
#define GAHBCFG     REG32(OTG + 0x008u)
#define GUSBCFG     REG32(OTG + 0x00Cu)
#define GRSTCTL     REG32(OTG + 0x010u)
#define GINTSTS     REG32(OTG + 0x014u)
#define GINTMSK     REG32(OTG + 0x018u)
#define GRXSTSP     REG32(OTG + 0x020u)
#define GRXFSIZ     REG32(OTG + 0x024u)
#define DIEPTXF0    REG32(OTG + 0x028u)
#define GCCFG       REG32(OTG + 0x038u)
#define DIEPTXF1    REG32(OTG + 0x104u)
#define DCFG        REG32(OTG + 0x800u)
#define DCTL        REG32(OTG + 0x804u)
#define DSTS        REG32(OTG + 0x808u)
#define DIEPMSK     REG32(OTG + 0x810u)
#define DOEPMSK     REG32(OTG + 0x814u)
#define DAINTMSK    REG32(OTG + 0x81Cu)
#define DIEPCTL(n)  REG32(OTG + 0x900u + 0x20u * (n))
#define DIEPINT(n)  REG32(OTG + 0x908u + 0x20u * (n))
#define DIEPTSIZ(n) REG32(OTG + 0x910u + 0x20u * (n))
#define DTXFSTS(n)  REG32(OTG + 0x918u + 0x20u * (n))
#define DOEPCTL(n)  REG32(OTG + 0xB00u + 0x20u * (n))
#define DOEPINT(n)  REG32(OTG + 0xB08u + 0x20u * (n))
#define DOEPTSIZ(n) REG32(OTG + 0xB10u + 0x20u * (n))
#define PCGCCTL     REG32(OTG + 0xE00u)
#define FIFO(n)     REG32(OTG + 0x1000u + 0x1000u * (n))

#define FHSTMODE  (1u << 29)  // GUSBCFG
#define FDEVMODE  (1u << 30)
#define CSFTRST   (1u << 0)   // GRSTCTL
#define RXFFLSH   (1u << 4)
#define TXFFLSH   (1u << 5)
#define TXFNUM(n) ((uint32_t)(n) << 6)
#define AHBIDLE   (1u << 31)
#define RXFLVL    (1u << 4)   // GINTSTS
#define GOUTNAKEFF (1u << 7)
#define USBSUSP   (1u << 11)
#define USBRST    (1u << 12)
#define ENUMDONE  (1u << 13)
#define IEPINT    (1u << 18)
#define OEPINT    (1u << 19)
#define PWRDOWN   (1u << 16)  // GCCFG: transceiver on
#define AVALIDSESEN (1u << 18)
#define BVALIDSESEN (1u << 19)
#define SFTDISCON (1u << 1)   // DCTL
#define SGOUTNAK  (1u << 9)
#define CGOUTNAK  (1u << 10)
#define EPENA     (1u << 31)  // DIEPCTL, DOEPCTL
#define EPDIS     (1u << 30)
#define SD0PID    (1u << 28)
#define SNAK      (1u << 27)
#define CNAK      (1u << 26)
#define STALL     (1u << 21)
#define BULK      (2u << 18)
#define USBACEPT  (1u << 15)
#define XFERC     (1u << 0)   // DIEPINT, DOEPINT
#define EPTDISD   (1u << 1)
#define TIMEOUT   (1u << 3)   // DIEPINT
#define SETUP     (1u << 3)   // DOEPINT
#define INEPTNAK  (1u << 6)
#define PKTCNT(n) ((uint32_t)(n) << 19)
// Endpoint 0 OUT: up to 3 back-to-back SETUP packets, one packet
#define EP0_OUT   (3u << 29 | PKTCNT(1) | 64u)

// FIFO RAM in words: receive FIFO, transmit FIFOs of endpoints 0 and 1 (of 1,280 B). The
// controller counts at most 8 packets in a transmit FIFO (RM 20.5.4.13): endpoint 1 gets room
// for 8 packets of 64 B, so fill() never queues more.
#define RX_WORDS  96u
#define TX0_WORDS 16u
#define TX1_WORDS 128u

#define WAIT_N 100000u

// GRSTCTL: v, then until the controller has cleared its reset and flush bits and its AHB
// master is idle. Bounded, a controller without its clock never clears them.
static void grst(uint32_t v)
{
    GRSTCTL = v;
    for (uint32_t n = WAIT_N; n && (GRSTCTL & (AHBIDLE | CSFTRST | RXFFLSH | TXFFLSH)) != AHBIDLE; n--) {
    }
}

// Until bit is set in the interrupt register r, bounded
static void wait_bit(volatile uint32_t *r, uint32_t bit)
{
    for (uint32_t n = WAIT_N; n && !(*r & bit); n--) {
    }
}

// n bytes of p as words, the last one padded: nothing is read past p + n
static void fifo_put(uint32_t f, const uint8_t *p, uint32_t n)
{
    for (uint32_t i = 0; i < n; i += 4) {
        uint32_t w = 0;
        for (uint32_t b = 0; b < 4 && i + b < n; b++)
            w |= (uint32_t)p[i + b] << 8 * b;
        FIFO(f) = w;
    }
}

// Stops IN endpoint n as the RM does it (NAK, then disable), flushes its transmit FIFO
static void in_stop(uint32_t n)
{
    if (DIEPCTL(n) & EPENA) {
        DIEPCTL(n) |= SNAK;
        wait_bit(&DIEPINT(n), INEPTNAK);
        DIEPCTL(n) |= EPDIS | SNAK;
        wait_bit(&DIEPINT(n), EPTDISD);
    }
    grst(TXFNUM(n) | TXFFLSH);
    DIEPINT(n) = 0xFFu;
}

void usbd_init(void)
{
    usbd_state_t *s = &usbd_ram;
    // USBDIV 101 (CRM_CFG bit 27 and bits 23:22 = 01): 144 MHz / 3 = 48 MHz
    CRM_CFG = (CRM_CFG & ~(1u << 27 | 3u << 22)) | 1u << 27 | 1u << 22;
    // VBUS (PA9) as an input with pull-up: the session comparators always see a valid
    // session. The controller only switches its D+ pull-up on with a valid session, and
    // VBUSIG alone does not give one on this board.
    pin_cfg(GPIOA, 9, PIN_IN_PU);
    pin_high(GPIOA, 9);
    CRM_AHBEN |= 1u << 12;  // OTGFS1
    for (uint8_t *p = (uint8_t *)s; p < (uint8_t *)(s + 1); p++)
        *p = 0;
    grst(0);
    grst(CSFTRST);
    GUSBCFG = (GUSBCFG & ~FHSTMODE) | FDEVMODE;
    plat_delay_ms(25);  // a forced mode takes effect after 25 ms
    DCTL = SFTDISCON;
    PCGCCTL = 0;
    DCFG = (DCFG & ~0x7F3u) | 3u;  // full speed, address 0
    GRXFSIZ = RX_WORDS;
    DIEPTXF0 = TX0_WORDS << 16 | RX_WORDS;
    DIEPTXF1 = TX1_WORDS << 16 | (RX_WORDS + TX0_WORDS);
    grst(TXFNUM(0x10) | TXFFLSH);
    grst(RXFFLSH);
    GINTSTS = 0xFFFFFFFFu;
    // the endpoint status bits that are polled, unmasked as the RM's reset handling does: it
    // leaves open whether masked ones are set (GINTSTS keeps its bits either way)
    DIEPMSK = XFERC | EPTDISD | TIMEOUT | INEPTNAK;
    DOEPMSK = XFERC | EPTDISD | SETUP;
    DAINTMSK = 0x00030003u;  // endpoints 0 and 1, IN and OUT
    GCCFG = PWRDOWN | AVALIDSESEN | BVALIDSESEN;
}

void usbd_connect(void) { DCTL &= ~SFTDISCON; }

void usbd_irq_on(void)
{
    GINTMSK = RXFLVL | USBSUSP | USBRST | ENUMDONE | IEPINT | OEPINT;
    GAHBCFG = 1u;  // GLBINTMSK: the interrupt reaches the NVIC
}

int usbd_irq_done(void)
{
    // the unmasked endpoint status bits usbd_poll() does not clear (in_stop() waits for them)
    DIEPINT(0) = EPTDISD | TIMEOUT | INEPTNAK;
    DIEPINT(1) = EPTDISD | TIMEOUT | INEPTNAK;
    DOEPINT(1) = EPTDISD | SETUP;
    uint32_t g = GINTSTS & USBSUSP;
    GINTSTS = g;
    return g != 0;
}

void usbd_off(void)
{
    DCTL |= SFTDISCON;
    GCCFG = 0;
    CRM_AHBEN &= ~(1u << 12);
}

void usbd_reboot(void)
{
    usbd_off();
    __asm volatile("dsb");
    SCB_AIRCR = 0x05FA0004u;
    __asm volatile("dsb");
    for (;;) {
    }
}

void usbd_uid(uint32_t id[3])
{
    for (unsigned i = 0; i < 3; i++)
        id[i] = REG32(0x1FFFF7E8u + 4u * i);
}

// One entry of the receive FIFO: SETUP data to the setup buffer, OUT data of endpoint 1 and of
// an OUT data stage to its transfer (what does not fit is dropped), everything else popped and
// dropped.
static void rx(usbd_state_t *s)
{
    uint32_t st = GRXSTSP, n = st >> 4 & 0x7FFu, kind = st >> 17 & 15u, keep = 0;
    uint8_t *dst = s->setup;
    if (kind == 6) {
        keep = 8;
    } else if (kind == 2 && (st & 15u) == 1 && s->out) {
        dst = s->out + s->out_pos;
        keep = s->out_len - s->out_pos;
        s->out_pos += n < keep ? n : keep;
    } else if (kind == 2 && !(st & 15u) && s->ep0_out) {
        dst = s->ep0_out + s->ep0_pos;
        keep = s->ep0_len - s->ep0_pos;
        s->ep0_pos += n < keep ? n : keep;
    }
    for (uint32_t i = 0; i < n; i += 4) {
        uint32_t w = FIFO(0);
        for (uint32_t b = i; b < i + 4 && b < n && b < keep; b++, w >>= 8)
            dst[b] = (uint8_t)w;
    }
}

// Endpoint 1 IN: whole packets into its transmit FIFO while they fit
static void fill(usbd_state_t *s)
{
    while (s->in_pos < s->in_len) {
        uint32_t k = s->in_len - s->in_pos;
        if (k > 64)
            k = 64;
        if ((DTXFSTS(1) & 0xFFFFu) < (k + 3) / 4)
            return;
        fifo_put(1, s->in + s->in_pos, k);
        s->in_pos += k;
    }
}

int usbd_suspended(void) { return DSTS & 1u; }

// Endpoint 0 OUT takes one packet: a status stage or the next packet of a data stage
static void ep0_rx(void)
{
    DOEPTSIZ(0) = EP0_OUT;
    DOEPCTL(0) |= EPENA | CNAK;
}

int usbd_poll(usbd_ev_t *e)
{
    usbd_state_t *s = &usbd_ram;
    uint32_t g = GINTSTS;
    if (g & USBRST) {
        GINTSTS = USBRST;
        DCFG &= ~(0x7Fu << 4);
        usbd_close(0x81);
        usbd_close(0x01);
        grst(TXFNUM(0x10) | TXFFLSH);
        DIEPINT(0) = 0xFFu;
        DOEPINT(0) = 0xFFu;
        DOEPTSIZ(0) = EP0_OUT;
        s->ep0_in = 0;
        s->ep0_out = 0;
        return USBD_RESET;
    }
    if (g & ENUMDONE) {
        GINTSTS = ENUMDONE;
        DIEPCTL(0) &= ~3u;  // 64 B packets on endpoint 0
    }
    while (GINTSTS & RXFLVL)
        rx(s);
    fill(s);
    uint32_t v = DOEPINT(0);
    DOEPINT(0) = v;
    if (v & SETUP) {
        // a reply the host did not read (it gave up that request) leaves transmit FIFO 0
        if (DIEPCTL(0) & EPENA)
            in_stop(0);
        if (!(DOEPCTL(0) & EPENA))
            DOEPTSIZ(0) = EP0_OUT;
        for (unsigned i = 0; i < 8; i++)
            e->setup[i] = s->setup[i];
        s->ep0_in = 0;
        s->ep0_out = 0;
        return USBD_SETUP;
    }
    if ((v & XFERC) && s->ep0_out) {
        // a full packet and more to come: the next one, else the data stage is through
        if (s->ep0_pos < s->ep0_len && !(DOEPTSIZ(0) & 0x7Fu)) {
            ep0_rx();
        } else {
            e->ep = 0;
            e->n = s->ep0_pos;
            s->ep0_out = 0;
            return USBD_DONE;
        }
    }
    if (DIEPINT(0) & XFERC) {
        DIEPINT(0) = XFERC;
        if (s->ep0_in) {  // the data stage is through: the status stage
            s->ep0_in = 0;
            ep0_rx();
        }
    }
    if (DOEPINT(1) & XFERC) {
        DOEPINT(1) = XFERC;
        e->ep = 0x01;
        e->n = s->out_pos;
        return USBD_DONE;
    }
    if (DIEPINT(1) & XFERC) {
        DIEPINT(1) = XFERC;
        e->ep = 0x81;
        return USBD_DONE;
    }
    return USBD_NONE;
}

void usbd_ctl_send(const void *data, uint32_t n)
{
    if (n > 64)
        n = 64;
    usbd_ram.ep0_in = usbd_ram.setup[0] & 0x80u;
    DIEPTSIZ(0) = PKTCNT(1) | n;
    DIEPCTL(0) |= EPENA | CNAK;
    fifo_put(0, data, n);
}

void usbd_ctl_recv(void *buf, uint32_t n)
{
    usbd_ram.ep0_out = buf;
    usbd_ram.ep0_len = n;
    usbd_ram.ep0_pos = 0;
    ep0_rx();
}

void usbd_ctl_stall(void)
{
    DIEPCTL(0) |= STALL;
    DOEPCTL(0) |= STALL;
}

void usbd_set_address(uint8_t addr) { DCFG = (DCFG & ~(0x7Fu << 4)) | (uint32_t)addr << 4; }

void usbd_open(uint8_t ep)
{
    if (ep & 0x80) {
        DIEPCTL(1) = USBACEPT | BULK | 1u << 22 | SD0PID | SNAK | 64u;  // transmit FIFO 1
        grst(TXFNUM(1) | TXFFLSH);
    } else {
        DOEPCTL(1) = USBACEPT | BULK | SD0PID | SNAK | 64u;
    }
}

void usbd_close(uint8_t ep)
{
    usbd_state_t *s = &usbd_ram;
    if (ep & 0x80) {
        usbd_abort_in();
        DIEPCTL(1) = SNAK;
        return;
    }
    s->out = 0;
    s->out_pos = 0;
    s->out_new = 1;
    if (DOEPCTL(1) & EPENA) {
        // the OUT endpoint disable of the RM: global OUT NAK, effective once its entry is
        // popped from the receive FIFO, then the endpoint disabled, then global OUT NAK off
        DCTL |= SGOUTNAK;
        for (uint32_t n = WAIT_N; n && !(GINTSTS & GOUTNAKEFF); n--) {
            if (GINTSTS & RXFLVL)
                rx(s);
        }
        DOEPCTL(1) |= EPDIS | SNAK;
        wait_bit(&DOEPINT(1), EPTDISD);
        DCTL |= CGOUTNAK;
    }
    DOEPCTL(1) = SNAK;
    DOEPINT(1) = 0xFFu;
}

void usbd_xfer(uint8_t ep, void *buf, uint32_t n)
{
    usbd_state_t *s = &usbd_ram;
    uint32_t pk = (n + 63) / 64;
    if (ep & 0x80) {
        s->in = buf;
        s->in_len = n;
        s->in_pos = 0;
        DIEPTSIZ(1) = PKTCNT(pk) | n;
        DIEPCTL(1) |= EPENA | CNAK;
        fill(s);
        return;
    }
    s->out = buf;
    s->out_len = n;
    s->out_pos = 0;
    if (s->out_new || !(DOEPCTL(1) & EPENA)) {
        s->out_new = 0;
        DOEPTSIZ(1) = PKTCNT(pk) | pk * 64u;
        DOEPCTL(1) |= EPENA | CNAK;
    }
}

void usbd_abort_in(void)
{
    usbd_ram.in_len = usbd_ram.in_pos = 0;
    in_stop(1);
}

void usbd_stall(uint8_t ep, int on)
{
    volatile uint32_t *c = ep & 0x80 ? &DIEPCTL(1) : &DOEPCTL(1);
    *c = on ? *c | STALL : (*c & ~STALL) | SD0PID;
}

int usbd_stalled(uint8_t ep) { return ((ep & 0x80 ? DIEPCTL(1) : DOEPCTL(1)) & STALL) != 0; }
