// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// RCPROBE: what a DJI RC-N1 remote controller (RC231) says to a phone on its top USB port.
// The meter plays the phone: it enumerates as a phone-like device, answers the Android Open
// Accessory handshake (AOA: GET_PROTOCOL, SEND_STRING, START), comes back as an accessory
// (18D1:2D00, vendor interface with bulk IN and OUT) and logs what the controller sends.
// Once the accessory is up a guided timeline asks for the sticks, the dial and each button
// in turn and writes a marker record per step. After its first step a DUML v1 request 06/01
// (read the stick channels, sender 02) goes out every 100 ms, plain and wrapped in turn.
//
// On the accessory pipe the controller sends messages of 512 B: a wrapper (55 CC 49 57, u32
// LE length), one DUML v1 frame of that length, zero padding. Frames are logged without the
// wrapper: cmd set 06 except 06/1E and replies to sender 02 unless they only repeat the last
// frame of their kind apart from sequence number and CRC, other frames only the first time
// their kind (src, dst, type, cmd set, cmd id, wrapped) shows up. The header counts every
// kind. The screen shows the latest 06/05 payload in big hex (bytes that differ from the
// first one in yellow), its rate and the latest 06/01 reply.
//
// - Identities, tried in turn while the host does nothing useful (no SET_CONFIGURATION
//   within 10 s of connecting, no AOA request within 6 s of SET_CONFIGURATION or of the last
//   AOA request): 18D1:4EE1 (a phone) and 18D1:2D00 (already an accessory).
// - The log is a stream of records (type, aux, u16 length, u32 ms, data) in RCLOG.DAT on the
//   firmware drive, which must exist (made on the PC, 8 KB to 1 MB, any 4 KB clusters of the
//   FAT chain): its first 4 KB hold the header, the stream follows. A full timeline run logged
//   4.9 KB in 84 s (60 B/s on average, about 300 B/s while the sticks moved), so 16 KB holds
//   one and 64 KB leaves room for about 3 minutes of stick movement after it. RAM keeps the
//   part of the stream that is not final on the NOR yet. It is saved every 10 s while there
//   is new data, at the end of the timeline, when the file is full and on a key press.
//   rclog.py prints the log.
//
// Keys: any key but POWER saves, after the timeline or with the log full it also ends the
// log. POWER held 2 s ends the log, saves and powers off (a meter on USB power restarts).

#include <stddef.h>

#include "lcd.h"
#include "nor.h"
#include "platform.h"
#include "ui.h"
#include "usbd.h"

void *memcpy(void *d, const void *s, size_t n);
void *memmove(void *d, const void *s, size_t n);
int memcmp(const void *a, const void *b, size_t n);

usbd_state_t usbd_ram;

// ---- Log ----

#define SECT 4096u
#define LOG_RAM 20480u
#define MAX_UNITS 256u
#define LOG_MAGIC 0x474C4352u  // "RCLG"
#define LOG_VERSION 2u

enum { T_SETUP = 1, T_EP0, T_STATE, T_RX, T_TX, T_MARK };
enum {
    S_IDENT = 1, S_CONNECT, S_DISCONNECT, S_RESET, S_SUSPEND, S_RESUME, S_TIMEOUT, S_AOA_START,
    S_ACC_UP, S_TX_STUCK, S_TX_FLOW, S_USER_SAVE, S_END
};
// Header flags
#define F_FULL 1u
#define F_ACC 2u
#define F_TIMELINE 4u
#define F_END 8u
// Setup record aux: stalled, OUT data stage follows
#define A_STALL 1
#define A_DATA 2
// RX and TX record aux: wrapped, truncated to the message buffer, bad CRC, raw packet
#define A_WRAP 1
#define A_TRUNC 2
#define A_BAD 4
#define A_RAW 8

static uint8_t logb[LOG_RAM] PLAT_DYN;
static uint32_t unit_addr[MAX_UNITS] PLAT_DYN;  // NOR address of each 4 KB of RCLOG.DAT
static uint32_t nunits, log_cap, log_base, log_used, log_final, log_saved, dropped, flags;
static uint32_t saves, last_save;
// Counters for the screen and the header: frames not logged as repeats or as a known kind,
// requests sent and taken by the host, packets and bytes received, 06/01 replies to plain
// and wrapped requests
static uint32_t reps, known, tx_n, tx_done, rx_pkts, rx_bytes, ans[2];
// Header words, then the frame kinds: src | dst << 8 | type << 16 | set << 24, id | wrapped
// << 8, count
#define NK 32
static uint32_t hdr[17 + 3 * NK], nkind;
enum { SV_NONE, SV_OK, SV_FAIL, SV_NOFILE };
static int save_res;

static uint32_t le16(const uint8_t *p) { return p[0] | (uint32_t)p[1] << 8; }

// Maps RCLOG.DAT: the NOR address of each of its 4 KB units, by the FAT12 chain. Returns the
// number of units, 0 when the file is missing or under 8 KB.
static uint32_t file_map(void)
{
    uint8_t b[36];
    nor_read(0, b, 36);
    uint32_t spc = b[13], rsv = le16(b + 14), nfat = b[16], nroot = le16(b + 17), fatsz = le16(b + 22);
    uint32_t tot = le16(b + 19) ? le16(b + 19) : le16(b + 32) | le16(b + 34) << 16;
    if (le16(b + 11) != SECT || !spc || !rsv || !nfat || nfat > 2u || !nroot || nroot > 1024u)
        return 0;
    uint32_t root = (rsv + nfat * fatsz) * SECT, data = root + (nroot * 32u + SECT - 1u) / SECT * SECT;
    uint32_t end = tot * SECT < NOR_SIZE ? tot * SECT : NOR_SIZE;
    for (uint32_t i = 0; i < nroot; i++) {
        nor_read(root + i * 32u, b, 32);
        if (!b[0])
            break;
        if (b[0] == 0xE5u || (b[11] & 0x1Fu) || memcmp(b, "RCLOG   DAT", 11))
            continue;
        uint32_t clus = le16(b + 26), size = le16(b + 28) | le16(b + 30) << 16, n = 0;
        uint32_t want = size / SECT < MAX_UNITS ? size / SECT : MAX_UNITS;
        while (n < want && clus >= 2u && clus < 0xFF8u) {
            uint32_t a = data + (clus - 2u) * spc * SECT;
            for (uint32_t k = 0; k < spc && n < want; k++) {
                if (a + (k + 1u) * SECT > end)
                    return 0;
                unit_addr[n++] = a + k * SECT;
            }
            nor_read(rsv * SECT + clus + clus / 2u, b, 2);
            clus = clus & 1u ? le16(b) >> 4 : le16(b) & 0xFFFu;
        }
        return n >= 2u ? n : 0;
    }
    return 0;
}

// Writes the stream that is not final yet (whole 4 KB units become final), then the header.
// Units that became final leave the RAM.
static void save(void)
{
    uint32_t end = log_base + log_used;
    last_save = plat_ms();
    if (!nunits) {
        save_res = SV_NOFILE;
        return;
    }
    save_res = SV_FAIL;
    for (uint32_t s = log_final; s < end; s += SECT) {
        uint32_t k = end - s < SECT ? end - s : SECT, a = unit_addr[1u + s / SECT];
        if (nor_erase4k(a) || nor_program(a, logb + (s - log_base), k))
            return;
        if (k == SECT)
            log_final = s + SECT;
    }
    uint32_t h[17] = {LOG_MAGIC, LOG_VERSION, end, flags, ++saves, plat_ms(), dropped, log_cap,
                      reps, known, tx_n, tx_done, rx_pkts, rx_bytes, ans[0], ans[1], nkind};
    memcpy(hdr, h, sizeof(h));
    if (nor_erase4k(unit_addr[0]) || nor_program(unit_addr[0], hdr, (17u + 3u * nkind) * 4u))
        return;
    uint32_t done = log_final - log_base;
    memmove(logb, logb + done, log_used - done);
    log_base = log_final;
    log_used -= done;
    log_saved = end;
    save_res = SV_OK;
}

static void log_put(int type, int aux, const void *d, uint32_t n)
{
    uint32_t k = 8u + n, t = plat_ms();
    if (flags & F_END)
        return;
    if (log_used + k > LOG_RAM && nunits)
        save();
    if (log_base + log_used + k > log_cap || log_used + k > LOG_RAM) {
        dropped += k;
        flags |= F_FULL;
        return;
    }
    uint8_t *p = logb + log_used;
    p[0] = (uint8_t)type;
    p[1] = (uint8_t)aux;
    p[2] = (uint8_t)n;
    p[3] = (uint8_t)(n >> 8);
    for (int i = 0; i < 4; i++)
        p[4 + i] = (uint8_t)(t >> 8 * i);
    memcpy(p + 8, d, n);
    log_used += k;
}

static void log_state(int st, const void *d, uint32_t n) { log_put(T_STATE, st, d, n); }

// ---- DUML v1 ----

static uint32_t crc8(const uint8_t *p, uint32_t n)
{
    uint32_t c = 0x77;
    while (n--) {
        c ^= *p++;
        for (int i = 0; i < 8; i++)
            c = c & 1u ? c >> 1 ^ 0x8Cu : c >> 1;
    }
    return c;
}

static uint32_t crc16(const uint8_t *p, uint32_t n)
{
    uint32_t c = 0x3692;
    while (n--) {
        c ^= *p++;
        for (int i = 0; i < 8; i++)
            c = c & 1u ? c >> 1 ^ 0x8408u : c >> 1;
    }
    return c;
}

// Request 06/01 (read the stick channels, type 40, no payload) from src to the RC (06): 13 B
static void duml_req(uint8_t *f, uint8_t src, uint16_t seq)
{
    f[0] = 0x55;
    f[1] = 13;
    f[2] = 0x04;  // length bits 9:8, version 1
    f[3] = (uint8_t)crc8(f, 3);
    f[4] = src;
    f[5] = 0x06;
    f[6] = (uint8_t)seq;
    f[7] = (uint8_t)(seq >> 8);
    f[8] = 0x40;
    f[9] = 0x06;
    f[10] = 0x01;
    uint32_t c = crc16(f, 11);
    f[11] = (uint8_t)c;
    f[12] = (uint8_t)(c >> 8);
}

// ---- USB device ----

struct ident {
    uint16_t vid, pid;
};
static const struct ident ids[] = {
    {0x18D1, 0x4EE1},
    {0x18D1, 0x2D00},
};
#define NIDS 2
#define ID_ACC 1
#define ACC_VID 0x18D1u
#define ACC_PID 0x2D00u

// Manufacturer string 1, product string 2, no serial number
static const uint8_t dev_desc[18] = {18, 1, 0x00, 0x02, 0, 0, 0, 64, 0, 0, 0, 0, 0x00, 0x01, 1, 2, 0, 1};
// One configuration, self-powered, 100 mA: interface 0 of vendor class (FF/FF/00), no
// string, with bulk endpoints 0x81 and 0x01 of 64 B
static const uint8_t cfg_desc[32] = {
    9, 2, 32, 0, 1, 1, 0, 0xC0, 50,
    9, 4, 0, 0, 2, 0xFF, 0xFF, 0, 0,
    7, 5, 0x81, 2, 64, 0, 0,
    7, 5, 0x01, 2, 64, 0, 0,
};

static int cur, acc;            // identity, accessory mode (18D1:2D00)
static uint8_t cfg;             // configuration value
static uint8_t ep0[64];         // replies
static uint8_t ep0_data[512] PLAT_DYN;  // OUT data stages
static uint8_t ep0_setup[8];    // the request of the OUT data stage
static uint8_t rxbuf[64], txbuf[24];
static uint32_t deadline, switch_at, last_setup;
static int acc_up, susp;

// Counters for the screen
static uint32_t n_reset, n_setup, n_stall, n_conn, n_aoa, n_start, n_str;
static uint32_t n_msg, n_frames, n_bad, n_0605, rate_0605, pad_bytes, stray_bytes, n_raw;
static int tx_busy, tx_stuck;
static uint32_t tx_at;
static uint8_t p0605[13], base0605[13], r0601[25];
static int have0605, r0601_n, r0601_wrap;
// Message under way: its buffer, length, bytes so far (m_len 0: none), wrapped
static uint8_t mbuf[512] PLAT_DYN;
static uint32_t m_len, m_pos;
static int m_wrap;
static char aoa_str[6][24];

static uint32_t str_desc(const char *s)
{
    uint32_t n = 0;
    for (; s[n] && n < 31u; n++) {
        ep0[2 + 2 * n] = (uint8_t)s[n];
        ep0[3 + 2 * n] = 0;
    }
    ep0[0] = (uint8_t)(2 + 2 * n);
    ep0[1] = 3;
    return ep0[0];
}

// GET_DESCRIPTOR: the reply in ep0, its length, -1 to stall
static int32_t descriptor(uint32_t val, const uint8_t **d)
{
    const struct ident *id = &ids[cur];
    *d = ep0;
    switch (val) {
    case 0x100: {
        uint32_t vid = acc ? ACC_VID : id->vid, pid = acc ? ACC_PID : id->pid;
        memcpy(ep0, dev_desc, sizeof(dev_desc));
        ep0[8] = (uint8_t)vid;
        ep0[9] = (uint8_t)(vid >> 8);
        ep0[10] = (uint8_t)pid;
        ep0[11] = (uint8_t)(pid >> 8);
        return sizeof(dev_desc);
    }
    case 0x200:
        *d = cfg_desc;
        return sizeof(cfg_desc);
    case 0x300:
        ep0[0] = 4;
        ep0[1] = 3;
        ep0[2] = 0x09;  // English (United States)
        ep0[3] = 0x04;
        return 4;
    case 0x301: return (int32_t)str_desc("dmc100-doom");
    case 0x302: return (int32_t)str_desc("RCPROBE");
    }
    return -1;
}

static void configure(uint8_t c)
{
    cfg = c;
    tx_busy = tx_stuck = 0;
    usbd_close(0x81);
    usbd_close(0x01);
    if (!c)
        return;
    usbd_open(0x81);
    usbd_open(0x01);
    usbd_xfer(0x01, rxbuf, sizeof(rxbuf));
    if (acc) {
        deadline = 0;
        acc_up = 1;
        flags |= F_ACC;
        log_state(S_ACC_UP, 0, 0);
    } else {
        deadline = plat_ms() + 6000u;
    }
}

static int bulk_ep(uint32_t idx) { return cfg && (idx == 0x81u || idx == 0x01u); }

static void setup(const uint8_t *s)
{
    uint32_t val = s[2] | s[3] << 8, idx = s[4] | s[5] << 8, len = s[6] | s[7] << 8;
    const uint8_t *d = ep0;
    int32_t n = -1;
    int aux = 0;
    n_setup++;
    last_setup = plat_ms();
    ep0[0] = ep0[1] = 0;
    switch (s[0] << 8 | s[1]) {
    case 0x8006: n = descriptor(val, &d); break;  // GET_DESCRIPTOR
    case 0x0005:  // SET_ADDRESS
        usbd_set_address((uint8_t)(val & 0x7F));
        n = 0;
        break;
    case 0x0009:  // SET_CONFIGURATION
        if (val <= 1) {
            configure((uint8_t)val);
            n = 0;
        }
        break;
    case 0x8008: ep0[0] = cfg; n = 1; break;  // GET_CONFIGURATION
    case 0x8000: ep0[0] = 1; n = 2; break;    // GET_STATUS of the device: self-powered
    case 0x8100: n = 2; break;                // of the interface
    case 0x8200:                              // of an endpoint: its halt
        ep0[0] = (uint8_t)(bulk_ep(idx) && usbd_stalled((uint8_t)idx));
        n = 2;
        break;
    case 0x0001:  // CLEAR_FEATURE and SET_FEATURE of the device (remote wakeup)
    case 0x0003: n = 0; break;
    case 0x0201:  // CLEAR_FEATURE and SET_FEATURE(ENDPOINT_HALT)
    case 0x0203:
        if (bulk_ep(idx))
            usbd_stall((uint8_t)idx, s[1] == 3);
        n = 0;
        break;
    case 0x810A: n = 1; break;           // GET_INTERFACE: alternate setting 0
    case 0x010B: n = val ? -1 : 0; break;  // SET_INTERFACE
    case 0xC033:  // AOA GET_PROTOCOL: version 2
        ep0[0] = 2;
        n = 2;
        break;
    case 0x4035:  // AOA START: answered, then the switch to the accessory
        n_start++;
        switch_at = plat_ms() + 50u;
        n = 0;
        break;
    case 0x4036:  // AOA REGISTER_HID, UNREGISTER_HID, SET_AUDIO_MODE
    case 0x4037:
    case 0x403A: n = 0; break;
    }
    if (s[0] == 0xC0 || s[0] == 0x40) {
        n_aoa++;
        if (!acc && !switch_at)
            deadline = plat_ms() + 6000u;
    }
    if (n < 0 && !(s[0] & 0x80) && len && len <= sizeof(ep0_data)) {
        // an OUT data stage: received and logged, then answered (AOA SEND_STRING,
        // SET_HID_REPORT_DESC, SEND_HID_EVENT) or stalled
        memcpy(ep0_setup, s, 8);
        usbd_ctl_recv(ep0_data, len);
        aux = A_DATA;
    } else if (n < 0) {
        usbd_ctl_stall();
        n_stall++;
        aux = A_STALL;
    } else {
        usbd_ctl_send(d, (uint32_t)n < len ? (uint32_t)n : len);
    }
    log_put(T_SETUP, aux, s, 8);
}

static void ep0_done(uint32_t n)
{
    const uint8_t *s = ep0_setup;
    uint32_t idx = s[4] | s[5] << 8;
    log_put(T_EP0, s[1], ep0_data, n);
    if (s[0] == 0x40 && (s[1] == 52 || s[1] == 56 || s[1] == 57)) {
        usbd_ctl_send(0, 0);
        if (s[1] == 52 && idx < 6u) {
            uint32_t k = 0;
            for (; k < n && k < 23u && ep0_data[k]; k++)
                aoa_str[idx][k] = (char)ep0_data[k];
            aoa_str[idx][k] = 0;
            n_str++;
        }
    } else {
        usbd_ctl_stall();
        n_stall++;
    }
}

// Frames that repeat the last frame of their kind (direction and wrapping, length, src, dst,
// type, cmd set and id) apart from the sequence number and the CRC are counted, not logged.
#define NKIND 16
static uint32_t kind_key[NKIND], kind_sum[NKIND], kind_next;

static uint32_t fnv(uint32_t h, uint32_t b) { return (h ^ b) * 16777619u; }

static int repeated(int dir, const uint8_t *p, uint32_t n)
{
    uint32_t key = fnv(fnv(fnv(fnv(2166136261u, (uint32_t)dir), n), p[4]), p[5]), sum;
    for (uint32_t i = 8; i < 11u; i++)
        key = fnv(key, p[i]);
    sum = key;
    for (uint32_t i = 11; i < n - 2u; i++)
        sum = fnv(sum, p[i]);
    for (uint32_t i = 0; i < NKIND; i++) {
        if (kind_key[i] == key) {
            if (kind_sum[i] == sum)
                return 1;
            kind_sum[i] = sum;
            return 0;
        }
    }
    kind_key[kind_next] = key;
    kind_sum[kind_next] = sum;
    kind_next = (kind_next + 1u) % NKIND;
    return 0;
}

// Counts the frame by kind, returns 1 when the kind is new
static int count_kind(const uint8_t *f, int wrap)
{
    uint32_t a = f[4] | f[5] << 8 | (uint32_t)f[8] << 16 | (uint32_t)f[9] << 24, b = f[10] | (uint32_t)wrap << 8;
    uint32_t *k = hdr + 17;
    for (uint32_t i = 0; i < nkind; i++, k += 3) {
        if (k[0] == a && k[1] == b) {
            k[2]++;
            return 0;
        }
    }
    if (nkind == NK)
        return 0;
    k[0] = a;
    k[1] = b;
    k[2] = 1;
    nkind++;
    return 1;
}

// One DUML frame of n bytes (the first sizeof(mbuf) of them in f)
static void frame(const uint8_t *f, uint32_t n, int wrap)
{
    uint32_t len = n < sizeof(mbuf) ? n : sizeof(mbuf);
    int aux = (wrap ? A_WRAP : 0) | (n > len ? A_TRUNC : 0);
    n_frames++;
    if (n < 13u || f[0] != 0x55 || crc8(f, 3) != f[3] || (f[1] | (f[2] & 3u) << 8) != n ||
        (n == len && crc16(f, n - 2u) != le16(f + n - 2u))) {
        if (++n_bad <= 8u)
            log_put(T_RX, aux | A_BAD, f, len);
        return;
    }
    int first = count_kind(f, wrap);
    if (f[9] == 6 && f[10] == 5 && n == 26u) {
        n_0605++;
        memcpy(p0605, f + 11, 13);
        if (!have0605)
            memcpy(base0605, p0605, 13);
        have0605 = 1;
    }
    if (f[9] == 6 && f[10] == 1 && (f[8] & 0x80)) {
        ans[f[6] & 1u]++;  // plain requests have even sequence numbers
        r0601_n = (int)(n - 13u < sizeof(r0601) ? n - 13u : sizeof(r0601));
        memcpy(r0601, f + 11, (uint32_t)r0601_n);
        r0601_wrap = wrap;
    }
    if ((f[9] == 6 && f[10] != 0x1E) || ((f[8] & 0x80) && f[5] == 0x02)) {
        if (repeated(wrap, f, len))
            reps++;
        else
            log_put(T_RX, aux, f, len);
    } else if (first) {
        log_put(T_RX, aux, f, len);
    } else {
        known++;
    }
}

// One packet of the bulk OUT pipe. A packet that starts with the wrapper or with a DUML
// header starts a message, its frame is collected over the packets that follow, the zero
// padding after it is counted. Other bytes are counted as stray, the first 8 packets with
// them are logged raw.
static void rx_packet(const uint8_t *p, uint32_t n)
{
    uint32_t i = 0;
    if (n >= 8u && p[0] == 0x55 && p[1] == 0xCC && p[2] == 0x49 && p[3] == 0x57) {
        m_len = le16(p + 4) | le16(p + 6) << 16;
        m_pos = 0;
        m_wrap = 1;
        i = 8;
        n_msg++;
        if (m_len > 1024u)
            m_len = 0;
    } else if (!m_len && n >= 4u && p[0] == 0x55 && crc8(p, 3) == p[3]) {
        m_len = p[1] | (p[2] & 3u) << 8;
        m_pos = 0;
        m_wrap = 0;
    }
    if (m_len) {
        for (; i < n && m_pos < m_len; i++, m_pos++) {
            if (m_pos < sizeof(mbuf))
                mbuf[m_pos] = p[i];
        }
        if (m_pos == m_len) {
            frame(mbuf, m_len, m_wrap);
            m_len = 0;
        }
    }
    uint32_t nz = 0;
    for (; i < n; i++)
        nz += p[i] != 0;
    stray_bytes += nz;
    pad_bytes += n - nz;
    if (nz && n_raw < 8u) {
        n_raw++;
        log_put(T_RX, A_RAW, p, n);
    }
}

static void ident_log(void)
{
    const struct ident *id = &ids[cur];
    uint32_t vid = acc ? ACC_VID : id->vid, pid = acc ? ACC_PID : id->pid;
    uint8_t d[6] = {(uint8_t)cur, (uint8_t)acc, (uint8_t)vid, (uint8_t)(vid >> 8), (uint8_t)pid,
                    (uint8_t)(pid >> 8)};
    log_state(S_IDENT, d, sizeof(d));
}

static void usb_up(void)
{
    cfg = 0;
    acc_up = tx_busy = tx_stuck = 0;
    switch_at = 0;
    usbd_init();
    usbd_connect();
    n_conn++;
    deadline = plat_ms() + 10000u;
    log_state(S_CONNECT, 0, 0);
}

// Disconnect for 200 ms, then connect as identity id (accessory mode a)
static void usb_switch(int id, int a)
{
    usbd_off();
    log_state(S_DISCONNECT, 0, 0);
    plat_delay_ms(200);
    cur = id;
    acc = a;
    ident_log();
    usb_up();
}

static void usb_event(int t, const usbd_ev_t *e)
{
    if (t == USBD_RESET) {
        n_reset++;
        cfg = 0;
        acc_up = tx_busy = tx_stuck = 0;
        if (!deadline && !switch_at)
            deadline = plat_ms() + 10000u;
        log_state(S_RESET, 0, 0);
    } else if (t == USBD_SETUP) {
        setup(e->setup);
    } else if (t == USBD_DONE && e->ep == 0) {
        ep0_done(e->n);
    } else if (t == USBD_DONE && e->ep == 0x01) {
        rx_bytes += e->n;
        rx_pkts++;
        rx_packet(rxbuf, e->n);
        usbd_xfer(0x01, rxbuf, sizeof(rxbuf));
    } else if (t == USBD_DONE && e->ep == 0x81) {
        tx_done++;
        tx_busy = 0;
        if (tx_stuck) {
            tx_stuck = 0;
            log_state(S_TX_FLOW, 0, 0);
        }
    }
}

// ---- Timeline ----

#define STEP_MS 6000u
#define NSTEPS 11
static const char *const step_text[NSTEPS + 1] = {
    "LISTENING\nDO NOT TOUCH",
    "DO NOT TOUCH\nANYTHING",
    "MOVE BOTH STICKS\nFULLY AROUND",
    "TURN THE LEFT DIAL\nBOTH WAYS",
    "PRESS SHUTTER/RECORD\nA FEW TIMES",
    "PRESS PHOTO/VIDEO\nA FEW TIMES",
    "PRESS PAUSE/RTH\nSHORT, A FEW TIMES",
    "FLIGHT MODE SWITCH\nC, N, S AND BACK",
    "PRESS FN/CUSTOM\nBUTTONS IF ANY",
    "PRESS RC POWER ONCE\nBRIEFLY, DO NOT HOLD",
    "DO NOT TOUCH\nANYTHING",
    "DONE. PRESS A METER\nKEY TO SAVE AND END",
};
static int step = -1;
static uint32_t tl_start, beep_end;

// ---- Screen ----

#define PROMPT_Y 154
#define HEX_Y 116
static char rows[11][44];
static uint16_t row_fg[11];
static const char *shown_text;
static char shown_head[24];

static void row(int r, uint16_t fg, const char *f, ...) __attribute__((format(printf, 3, 4)));
static void row(int r, uint16_t fg, const char *f, ...)
{
    char s[44] = {0};
    va_list ap;
    va_start(ap, f);
    plat_vfmt(s, sizeof(s), f, ap);
    va_end(ap);
    if (row_fg[r] == fg && !memcmp(rows[r], s, sizeof(s)))
        return;
    memcpy(rows[r], s, sizeof(s));
    row_fg[r] = fg;
    ui_text_row(UI_ROW_Y(r), fg, C_BLACK, s);
}

// Big text: a head line and up to three lines of text (lines split at '\n')
static void prompt(const char *head, uint16_t hc, const char *text)
{
    char h[21];
    plat_fmt(h, sizeof(h), "%-20s", head);
    if (text != shown_text) {
        shown_text = text;
        shown_head[0] = 0;
        lcd_fill(0, PROMPT_Y - 2, LCD_W - 1, LCD_H - 12, C_BLACK);
        int y = PROMPT_Y + 18;
        for (const char *p = text; *p && y < LCD_H - 28; y += 18) {
            char l[21];
            int n = 0;
            while (*p && *p != '\n' && n < 20)
                l[n++] = *p++;
            l[n] = 0;
            if (*p == '\n')
                p++;
            lcd_text(0, y, l, C_WHITE, C_BLACK, 2);
        }
    }
    if (memcmp(h, shown_head, sizeof(h))) {
        memcpy(shown_head, h, sizeof(h));
        lcd_text(0, PROMPT_Y, h, hc, C_BLACK, 2);
    }
}

static void draw(void)
{
    const struct ident *id = &ids[cur];
    uint32_t now = plat_ms();
    row(0, acc_up ? C_GREEN : C_RED, "%s id %d/%d %04x:%04x", acc_up ? "ACC UP" : "acc down",
        cur + 1, NIDS, acc ? ACC_VID : id->vid, acc ? ACC_PID : id->pid);
    row(1, C_WHITE, "usb conn %u reset %u setup %u stall %u%s", n_conn, n_reset, n_setup, n_stall,
        susp ? " susp" : "");
    row(2, n_start ? C_GREEN : n_aoa ? C_YELLOW : C_GREY, "aoa %u str %u start %u %s", n_aoa, n_str, n_start,
        aoa_str[2]);
    row(3, C_WHITE, "rx %u KB msg %u frm %u bad %u", rx_bytes >> 10, n_msg, n_frames, n_bad);
    row(4, C_WHITE, "pad %u KB stray %u kinds %u rep %u", pad_bytes >> 10, stray_bytes, nkind, reps);
    row(5, C_YELLOW, "06/05 %u/s (%u)  req %u taken %u", rate_0605, n_0605, tx_n, tx_done);
    row(6, ans[0] | ans[1] ? C_GREEN : C_GREY, "06/01 replies: plain %u wrapped %u%s", ans[0], ans[1],
        r0601_n ? (r0601_wrap ? " (wr)" : " (pl)") : "");
    char x[44];
    for (int k = 0; k < 2; k++) {
        int i = 0;
        for (int b = 13 * k; b < 13 * k + 13 && b < r0601_n; b++)
            i += plat_fmt(x + i, (int)sizeof(x) - i, "%02x ", r0601[b]);
        x[i] = 0;
        row(7 + k, C_GREEN, "%s", x);
    }
    static const char *const sv[4] = {"not saved yet", "saved", "save FAILED", "NO RCLOG.DAT"};
    row(9, save_res == SV_OK ? C_GREEN : save_res >= SV_FAIL ? C_RED : C_GREY, "log %u/%u B%s %u %s",
        log_base + log_used, log_cap, flags & F_FULL ? " FULL" : "", saves, sv[save_res]);

    // 06/05 payload in big hex, 7 bytes on the first line, 6 on the second
    static uint8_t drawn_v[13];
    static uint16_t drawn_c[13];
    for (int b = 0; have0605 && b < 13; b++) {
        uint16_t c = p0605[b] != base0605[b] ? C_YELLOW : C_WHITE;
        if (drawn_c[b] == c && drawn_v[b] == p0605[b])
            continue;
        drawn_c[b] = c;
        drawn_v[b] = p0605[b];
        char t[3];
        plat_fmt(t, sizeof(t), "%02x", p0605[b]);
        lcd_text(48 * (b % 7), HEX_Y + 18 * (b / 7), t, c, C_BLACK, 2);
    }

    char h[24];
    if (flags & F_END) {
        prompt("LOG ENDED", C_GREEN,
               nunits ? "LOG SAVED. NOW PUT\nTHE METER IN UPGRADE\nMODE, COPY RCLOG.DAT"
                      : "NO RCLOG.DAT ON THE\nDRIVE, NOTHING SAVED");
    } else if (flags & F_FULL) {
        prompt("LOG FULL", C_ORANGE, "PRESS A METER KEY\nTO SAVE AND END");
    } else if (step >= 0 && step < NSTEPS) {
        plat_fmt(h, sizeof(h), "STEP %d/%d  %u s", step, NSTEPS - 1,
                 (tl_start + (uint32_t)(step + 1) * STEP_MS - now + 999u) / 1000u);
        prompt(h, C_YELLOW, step_text[step]);
    } else if (step >= NSTEPS) {
        prompt("DONE", C_GREEN, step_text[NSTEPS]);
    } else if (!nunits) {
        prompt("NO RCLOG.DAT", C_RED, "COPY RCLOG.DAT TO\nTHE FIRMWARE DRIVE\nFIRST, NOTHING SAVED");
    } else if (n_start) {
        prompt("AOA START", C_GREEN, "SWITCHING TO\nACCESSORY MODE");
    } else {
        prompt(n_reset ? "HOST SEEN" : "WAITING FOR HOST", C_YELLOW,
               "PLUG THE METER INTO\nRC TOP PORT, RC ON");
    }
}

// ---- Main ----

void app_main(void)
{
    plat_init();
    snd_volume(SND_HIGH);
    ui_clear();
    ui_text_row(0, C_BLACK, C_YELLOW, "RCPROBE  DJI RC-N1 phone port probe");
    ui_footer("any key: save   POWER 2 s: save + off");
    nunits = file_map();
    log_cap = nunits ? (nunits - 1u) * SECT : LOG_RAM;
    if (!nunits)
        save_res = SV_NOFILE;
    ident_log();
    usb_up();
    uint32_t tx_last = 0, drawn = 0, rate_at = 0, rate_n = 0;
    int full_saved = 0;
    for (;;) {
        usbd_ev_t e;
        int t;
        while ((t = usbd_poll(&e)) != USBD_NONE)
            usb_event(t, &e);
        uint32_t now = plat_ms();

        if (switch_at && (int32_t)(now - switch_at) >= 0) {
            log_state(S_AOA_START, 0, 0);
            usb_switch(cur, 1);
        } else if (deadline && (int32_t)(now - deadline) >= 0) {
            log_state(S_TIMEOUT, 0, 0);
            int next = (cur + 1) % NIDS;
            usb_switch(next, next == ID_ACC);
        }
        if (usbd_suspended() != susp) {
            susp = !susp;
            log_state(susp ? S_SUSPEND : S_RESUME, 0, 0);
        }

        // Timeline
        if (acc_up && step < 0) {
            step = 0;
            tl_start = now;
            log_put(T_MARK, 0, 0, 0);
        }
        if (step >= 0 && step < NSTEPS && now - tl_start >= (uint32_t)(step + 1) * STEP_MS) {
            step++;
            log_put(T_MARK, step, 0, 0);
            snd_tone(step < NSTEPS ? 1500 : 3000);
            beep_end = now + 120u;
            if (step == NSTEPS) {
                flags |= F_TIMELINE;
                save();
            }
        }
        if (beep_end && (int32_t)(now - beep_end) >= 0) {
            snd_tone(0);
            beep_end = 0;
        }

        // 06/01 requests every 100 ms after the first step
        if (cfg && acc && step >= 1 && now - tx_last >= 100u && !(flags & F_END)) {
            tx_last = now;
            if (!tx_busy) {
                // even sequence numbers plain, odd ones wrapped
                static const uint8_t wrap_hdr[8] = {0x55, 0xCC, 0x49, 0x57, 13, 0, 0, 0};
                int w = (int)(tx_n & 1u);
                memcpy(txbuf, wrap_hdr, 8);
                duml_req(txbuf + 8, 0x02, (uint16_t)(0x2000u + tx_n));
                tx_n++;
                tx_busy = 1;
                tx_at = now;
                usbd_xfer(0x81, w ? txbuf : txbuf + 8, w ? 21 : 13);
                if (repeated(2 + w, txbuf + 8, 13))
                    reps++;
                else
                    log_put(T_TX, w ? A_WRAP : 0, txbuf + 8, 13);
            } else if (!tx_stuck && now - tx_at >= 1000u) {
                tx_stuck = 1;
                log_state(S_TX_STUCK, 0, 0);
            }
        }

        // Keys and saves
        int ev = plat_event();
        if (ev >= 0 && (ev & EV_PRESS) && (ev & 7) != BTN_POWER) {
            if (step >= NSTEPS || (flags & F_FULL)) {
                log_state(S_END, 0, 0);
                flags |= F_END;
            } else {
                log_state(S_USER_SAVE, 0, 0);
            }
            save();
        }
        if (ui_power_held(2000)) {
            log_state(S_END, 0, 0);
            flags |= F_END;
            save();
            ui_clear();
            plat_power_off();
        }
        if ((flags & F_FULL) && !full_saved) {
            full_saved = 1;
            save();
        }
        if (log_saved != log_base + log_used && now - last_save >= 10000u && now - last_setup >= 1000u)
            save();

        if (now - rate_at >= 1000u) {
            rate_at = now;
            rate_0605 = n_0605 - rate_n;
            rate_n = n_0605;
        }
        if (now - drawn >= 250u) {
            drawn = now;
            draw();
        }
    }
}
