#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Print the log that RCPROBE (controller/rc-n1/rcprobe.c) wrote to RCLOG.DAT on the firmware drive.

Usage: python3 -I controller/rc-n1/rclog.py [--summary] RCLOG.DAT

Create the file before running RCPROBE: copy a file of zeros named RCLOG.DAT of 8 KB to 1 MB
to the firmware drive. 16 KB holds one run of the timeline, 64 KB leaves room for about
3 minutes of stick movement after it. The timeline of the log is printed first: setup
requests, AOA strings, state changes, the step markers and the logged DUML v1 frames as hex
and parsed. RCPROBE logs the frames without the 55 CC 49 57 wrapper and flags those that came
in one as "wrapped". It does not log every frame: frames of command set 06 except 06/1E and
replies to 02 that repeat the last one of their kind apart from the sequence number and the
CRC are only counted. Of the other kinds only the first frame is logged (see rcprobe.c). The
summary follows: the header counters and the frame kinds the meter saw, identities and AOA
strings, the logged frames by kind, and per step the payload bytes that took values not seen
while the controller was left alone (steps 0 and 1), which is where the buttons show up.
--summary prints the summary only.

The log and the printout contain the controller's serial number (the AOA string "serial").
"""

import struct
import sys

MAGIC = 0x474C4352  # "RCLG"
SECT = 4096

T_SETUP, T_EP0, T_STATE, T_RX, T_TX, T_MARK = range(1, 7)
STATES = [None, "IDENT", "CONNECT", "DISCONNECT", "RESET", "SUSPEND", "RESUME", "TIMEOUT",
          "AOA_START", "ACC_UP", "TX_STUCK", "TX_FLOW", "USER_SAVE", "END"]
FLAGS = ["full", "accessory up", "timeline done", "ended"]
STEPS = ["listening, no requests sent", "do not touch", "sticks fully around", "left dial both ways",
         "SHUTTER/RECORD", "PHOTO/VIDEO", "PAUSE/RTH short presses", "flight mode C, N, S",
         "Fn/custom buttons", "RC POWER brief press", "do not touch", "done, free logging"]
AOA_STR = ["manufacturer", "model", "description", "version", "URI", "serial"]
AOA_REQ = {51: "AOA GET_PROTOCOL", 52: "AOA SEND_STRING", 53: "AOA START", 54: "AOA REGISTER_HID",
           55: "AOA UNREGISTER_HID", 56: "AOA SET_HID_REPORT_DESC", 57: "AOA SEND_HID_EVENT",
           58: "AOA SET_AUDIO_MODE"}
STD_REQ = {0: "GET_STATUS", 1: "CLEAR_FEATURE", 3: "SET_FEATURE", 5: "SET_ADDRESS", 6: "GET_DESCRIPTOR",
           7: "SET_DESCRIPTOR", 8: "GET_CONFIGURATION", 9: "SET_CONFIGURATION", 10: "GET_INTERFACE",
           11: "SET_INTERFACE", 12: "SYNCH_FRAME"}
DESC = {1: "device", 2: "configuration", 3: "string", 4: "interface", 5: "endpoint", 6: "device qualifier",
        7: "other speed configuration", 0xF: "BOS", 0x21: "HID", 0x22: "HID report"}
# Command sets with a name: 06 is the set of the controller's own commands (sticks, buttons,
# battery), the others print as "?"
CMD_SET = {0x06: "rc"}
CHANNELS = ["right H", "right V", "left V", "left H", "dial", "ch6", "ch7", "ch8"]


def crc8(data, c=0x77):
    for b in data:
        c ^= b
        for _ in range(8):
            c = (c >> 1) ^ 0x8C if c & 1 else c >> 1
    return c


def crc16(data, c=0x3692):
    for b in data:
        c ^= b
        for _ in range(8):
            c = (c >> 1) ^ 0x8408 if c & 1 else c >> 1
    return c


class Parser:
    """DUML v1 frames out of a byte stream, bytes outside frames are counted as skipped."""

    def __init__(self):
        self.buf = b""
        self.skipped = 0

    def feed(self, data):
        self.buf += data
        out = []
        while True:
            i = self.buf.find(b"\x55")
            if i < 0:
                self.skipped += len(self.buf)
                self.buf = b""
                return out
            self.skipped += i
            self.buf = self.buf[i:]
            if len(self.buf) < 4:
                return out
            n = self.buf[1] | (self.buf[2] & 3) << 8
            if crc8(self.buf[:3]) != self.buf[3] or n < 13:
                self.skipped += 1
                self.buf = self.buf[1:]
                continue
            if len(self.buf) < n:
                return out
            p, self.buf = self.buf[:n], self.buf[n:]
            out.append((crc16(p[:-2]) == p[-2] | p[-1] << 8, p))


def frame_str(ok, p):
    seq = p[6] | p[7] << 8
    name = CMD_SET.get(p[9], "?")
    s = (f"DUML {'ok ' if ok else 'BAD'} src {p[4]:02x} dst {p[5]:02x} seq {seq:5d} type {p[8]:02x} "
         f"set {p[9]:02x} ({name}) id {p[10]:02x} payload[{len(p) - 13}] {p[11:-2].hex(' ')}")
    if ok and p[9] == 0x06 and p[10] == 0x01 and p[8] & 0x80 and len(p) - 13 >= 25:
        pl = p[11:-2]
        s += "\n" + " " * 14 + "channels " + " ".join(
            f"{CHANNELS[k]} {pl[2 + 3 * k] | pl[3 + 3 * k] << 8}" for k in range(8))
    return s


def setup_name(s):
    rt, req, val, idx = s[0], s[1], s[2] | s[3] << 8, s[4] | s[5] << 8
    kind = (rt >> 5) & 3
    if kind == 0:
        name = STD_REQ.get(req, f"standard {req}")
        if req == 6:
            name += f" {DESC.get(val >> 8, hex(val >> 8))} {val & 0xFF}"
    elif kind == 2 and req in AOA_REQ:
        name = AOA_REQ[req]
        if req == 52:
            name += f" {AOA_STR[idx] if idx < 6 else idx}"
    else:
        name = f"{['standard', 'class', 'vendor', 'reserved'][kind]} request {req}"
    return name


def setup_str(s):
    val, idx, ln = s[2] | s[3] << 8, s[4] | s[5] << 8, s[6] | s[7] << 8
    return f"{s[0]:02x} {s[1]:02x} val {val:04x} idx {idx:04x} len {ln:4d}  {setup_name(s)}"


def ident_str(d):
    acc, vid, pid = d[1], d[2] | d[3] << 8, d[4] | d[5] << 8
    return f"{vid:04X}:{pid:04X} {'accessory' if acc else 'phone'}"


def records(data):
    hdr = struct.unpack_from("<17I", data) if len(data) >= 68 else (0,) * 17
    magic, version, nbytes = hdr[:3]
    if magic == MAGIC:
        end = min(SECT + nbytes, len(data))
    else:
        print(f"no RCPROBE header (magic {magic:08x}), parsing the records until the first invalid one")
        end = len(data)
    pos, out = SECT, []
    while pos + 8 <= end:
        t, aux, n, ms = struct.unpack_from("<BBHI", data, pos)
        if t < T_SETUP or t > T_MARK or pos + 8 + n > end:
            break
        out.append((t, aux, ms, data[pos + 8:pos + 8 + n]))
        pos += 8 + n
    return hdr, out


def main():
    if "-h" in sys.argv[1:] or "--help" in sys.argv[1:]:
        print(__doc__, end="")
        return
    args = [a for a in sys.argv[1:] if a != "--summary"]
    if len(args) != 1:
        sys.exit(__doc__)
    quiet = "--summary" in sys.argv
    data = open(args[0], "rb").read()
    hdr, recs = records(data)
    magic, version, nbytes, flags, saves, uptime, dropped, cap = hdr[:8]
    reps, known, tx_n, tx_done, rx_n, rx_b, ans_plain, ans_wrap, nkind = hdr[8:]
    kinds = [struct.unpack_from("<3I", data, 68 + 12 * i) for i in range(min(nkind, 32))] if magic == MAGIC else []

    def out(s):
        if not quiet:
            print(s)

    par = {T_RX: Parser(), T_TX: Parser()}
    step = -1
    ident = None
    last_setup = None
    aoa_strings = {}
    ident_log, aoa_ident, stalls = [], None, {}
    nrx = ntx = brx = btx = 0
    counts = {}
    frames = []  # (step, direction, frame)
    raw_rx = {}  # step: bytes outside frames
    for t, aux, ms, d in recs:
        ts = f"{ms / 1000:9.3f}"
        if t == T_SETUP:
            last_setup = d
            what = "STALLED" if aux & 1 else "data stage" if aux & 2 else ""
            out(f"{ts} SETUP  {setup_str(d)}  {what}")
            if aux & 1:
                k = f"{d[0]:02x} {d[1]:02x} {setup_name(d)}"
                stalls[k] = stalls.get(k, 0) + 1
            if d[1] == 53 and (d[0] >> 5) & 3 == 2:
                aoa_ident = ident
        elif t == T_EP0:
            txt = d.split(b"\0")[0].decode("utf-8", "replace")
            out(f"{ts} EP0 OUT request {aux} [{len(d)}] {d.hex(' ')}")
            out(f"{'':9}        text {txt!r}")
            if last_setup is not None and aux == 52:
                idx = last_setup[4] | last_setup[5] << 8
                aoa_strings[AOA_STR[idx] if idx < 6 else str(idx)] = txt
        elif t == T_STATE:
            name = STATES[aux] if aux < len(STATES) else f"state {aux}"
            extra = ""
            if name == "IDENT" and len(d) >= 6:
                ident = ident_str(d)
                ident_log.append(ident)
                extra = ident
            out(f"{ts} STATE  {name} {extra}")
        elif t == T_MARK:
            step = aux
            out(f"{ts} ==== step {aux}: {STEPS[aux] if aux < len(STEPS) else '?'} ====")
        elif t in (T_RX, T_TX):
            tag = "RX" if t == T_RX else "TX"
            if t == T_RX:
                nrx += 1
                brx += len(d)
            else:
                ntx += 1
                btx += len(d)
            fl = " ".join(n for i, n in enumerate(["wrapped", "TRUNCATED", "BAD", "raw packet"])
                          if aux >> i & 1)
            out(f"{ts} {tag} [{len(d):2d}] {fl} {d.hex(' ')}")
            if aux & 8:
                continue
            p = par[t] = Parser()
            sk = 0
            for ok, f in p.feed(d):
                out(f"{'':9}     {frame_str(ok, f)}")
                key = (tag, f[4], f[5], f[8], f[9], f[10], ok)
                counts[key] = counts.get(key, 0) + 1
                frames.append((step, tag, ok, f))
            if t == T_RX:
                raw_rx[step] = raw_rx.get(step, 0) + p.skipped - sk

    print()
    print("==== summary ====")
    if magic == MAGIC:
        fl = ", ".join(f for i, f in enumerate(FLAGS) if flags >> i & 1) or "none"
        print(f"header: version {version}, {nbytes} of {cap} B of records, flags {fl}, saved {saves}x, "
              f"uptime at save {uptime / 1000:.1f} s, {dropped} B dropped (log full)")
        print(f"meter counters: received {rx_n} packets {rx_b} B, requests sent {tx_n}, taken by the RC "
              f"{tx_done}, 06/01 replies to plain {ans_plain} and wrapped {ans_wrap} requests")
        print(f"frames not logged: {reps} repeats, {known} of a known kind")
        print("frame kinds seen by the meter (src->dst type set/id, count):")
        for a, b, n in kinds:
            print(f"  {a & 255:02x}->{a >> 8 & 255:02x} type {a >> 16 & 255:02x} {a >> 24:02x}/{b & 255:02x} "
                  f"{CMD_SET.get(a >> 24, '?'):12} {'wrapped' if b >> 8 else 'plain  '} {n}x")
    print(f"{len(recs)} records")
    print("identities tried: " + ("; ".join(ident_log) or "none"))
    print(f"AOA START while: {aoa_ident or 'never'}")
    for k in AOA_STR:
        if k in aoa_strings:
            print(f"  AOA {k:12} {aoa_strings[k]!r}")
    for k, v in aoa_strings.items():
        if k not in AOA_STR:
            print(f"  AOA string {k} {v!r}")
    if stalls:
        print("stalled requests:")
        for k, v in stalls.items():
            print(f"  {v:4d}x {k}")
    print(f"logged: RX (from the RC) {nrx} records {brx} B, TX (to the RC) {ntx} records {btx} B")
    print("DUML frames logged by direction, src, dst, type, cmd set, cmd id:")
    for key in sorted(counts):
        tag, src, dst, typ, cs, ci, ok = key
        print(f"  {tag} {src:02x}->{dst:02x} type {typ:02x} {cs:02x}/{ci:02x} {CMD_SET.get(cs, '?'):12} "
              f"{'' if ok else 'BAD CRC '}{counts[key]}x")

    # Payload bytes per RX frame kind: values seen while idle (steps 0 and 1), then per step
    # the bytes that took other values and the kinds first seen in that step
    def kind(f):
        return (f[4], f[5], f[8] & 0x80, f[9], f[10], len(f))

    idle = {}
    for st, tag, ok, f in frames:
        if tag == "RX" and ok and st in (0, 1):
            vals = idle.setdefault(kind(f), [set() for _ in range(len(f) - 13)])
            for i, b in enumerate(f[11:-2]):
                vals[i].add(b)
    seen = set(idle)
    print("per step, RX frames: payload bytes with values not seen in steps 0 and 1, new frame kinds")
    for st in range(-1, len(STEPS)):
        fs = [f for s, tag, ok, f in frames if s == st and tag == "RX" and ok]
        if not fs and not raw_rx.get(st):
            continue
        print(f"  step {st}: {STEPS[st] if st >= 0 else 'before the accessory came up'}, {len(fs)} frames, "
              f"{raw_rx.get(st, 0)} B outside frames")
        news, changed = {}, {}
        for f in fs:
            k = kind(f)
            if k not in seen:
                news[k] = news.get(k, 0) + 1
            base = idle.get(k)
            for i, b in enumerate(f[11:-2]):
                if base is None or b not in base[i]:
                    changed.setdefault(k, {}).setdefault(i, set()).add(b)
        for k, n in news.items():
            print(f"    new: {k[0]:02x}->{k[1]:02x} {'reply' if k[2] else 'cmd  '} {k[3]:02x}/{k[4]:02x} "
                  f"len {k[5]} {n}x")
        for k, pos in changed.items():
            if k in news:
                continue
            desc = []
            for i in sorted(pos):
                v = sorted(pos[i])
                if len(v) <= 6:
                    vs = " ".join(f"{x:02x}" for x in v)
                else:
                    vs = f"{v[0]:02x}..{v[-1]:02x} ({len(v)} values)"
                desc.append(f"[{i}] {vs}")
            print(f"    {k[0]:02x}->{k[1]:02x} {k[3]:02x}/{k[4]:02x} len {k[5]}: " + "; ".join(desc))
        seen.update(news)


if __name__ == "__main__":
    main()
