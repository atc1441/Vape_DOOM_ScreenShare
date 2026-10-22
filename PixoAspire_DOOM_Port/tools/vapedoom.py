#!/usr/bin/env python3
"""
vapedoom.py - flashing and watching the Doom firmware on the vape.

    python vapedoom.py flash [file.bin]    put firmware on and start it
    python vapedoom.py log                 read the debug ring live
    python vapedoom.py state               progress mark and sign of life
    python vapedoom.py shot [file.png]     photograph the display
    python vapedoom.py run                 flash, then follow the log

This board has neither a serial port nor USB for us - USB is the vape's own
connector. The firmware instead puts a ring in RAM that the debugger reads
while the CPU runs: the Cortex-M4 allows memory access without a halt, so
both watchdogs keep being fed while we read along.

For the firmware itself prefer tools/flash.py - the loader used here
reproducibly loses pages at the end of a 189 KB image.
"""

import os
import struct
import subprocess
import sys
import time

import pylink

# pylink's destructor still calls into the DLL at process exit and crashes
# there once the DLL has already been torn down. That happens only after
# getaner Arbeit, macht aber jede Ausgabe unlesbar.
pylink.JLink.__del__ = lambda self: None

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

# Everything that depends on this machine can be set from the environment;
# the defaults are the setup everything here was measured on.
#   VAPE_JLINK_DLL   JLink_x64.dll (version must match the Commander)
#   VAPE_SN          serial number of the probe
#   VAPE_DEVICE      device name for the J-Link
#   VAPE_ELF         ELF the symbols are read from (Windows path)
#   VAPE_ELF_LINUX   the same ELF as WSL sees it
#   VAPE_WSL         name of the WSL distribution
#   VAPE_NM          llvm-nm inside WSL
DLL = os.environ.get("VAPE_JLINK_DLL",
                     r"C:\Program Files\SEGGER\JLink_V870\JLink_x64.dll")
NM_WSL = os.environ.get("VAPE_NM", "/usr/lib/llvm-14/bin/llvm-nm")
WSL_DISTRO = os.environ.get("VAPE_WSL", "Ubuntu-22.04")
SN = int(os.environ.get("VAPE_SN", "694200801"))
DEVICE = os.environ.get("VAPE_DEVICE", "PY32F403xC")
SPEED = 4000

BIN = os.path.join(ROOT, "out", "DOOMVAPE.BIN")
# The tree built from your own IWAD (tools/build_ud.sh), not the Freedoom
# state in build/py32 - that one stood here and pointed at an image that had
# long since left the device.
ELF_WSL = os.environ.get("VAPE_ELF", r"\\wsl$\Ubuntu-22.04\home\pc\dmc\build\ud\py32\DOOMVAPE.elf")
ELF_LINUX = os.environ.get("VAPE_ELF_LINUX", "/home/pc/dmc/build/ud/py32/DOOMVAPE.elf")

DBG_MAGIC = 0x564C4F47  # "VLOG"
# Field offsets in struct dbg_block, platform/py32/dbg.c
O_MAGIC, O_SIZE, O_WR, O_STAGE, O_MS = 0x00, 0x04, 0x08, 0x0C, 0x10
O_FRAMES, O_BUTTONS, O_FPS = 0x14, 0x18, 0x1C
O_NOR_CYC, O_NOR_CALLS, O_NOR_BYTES = 0x20, 0x24, 0x28
O_PUMP_CYC, O_PUMP_PX = 0x2C, 0x30
O_TC_OK, O_TC_TOUCH = 0x34, 0x38
O_TOUCH_SX, O_TOUCH_SY, O_TOUCH_RAW = 0x3C, 0x40, 0x44
# The ring sits behind all the counters. That this was once wrong went
# unnoticed: a window shifted by 40 bytes onto a text ring still looks like
# text. The order is defined in dbg.c, not here.
O_BUF = 0x48

BTN = ["HOLD/rueck", "SAVE/rechts", "MODE/links", "VA/vor", "LIGHT/feuer", "POWER/umsch"]

STAGES = {
    0: "reset",
    1: "plat_init done",
    2: "NOR identified",
    3: "LCD ready",
    4: "data checked",
    5: "engine init",
    6: "game running",
}


class Vape(object):
    def __init__(self):
        self.jl = None
        self.dbg = None
        self.cursor = 0
        self.tail = ""

    def open(self):
        self.jl = pylink.JLink(lib=pylink.library.Library(DLL))
        self.jl.open(serial_no=SN)
        self.jl.set_tif(pylink.enums.JLinkInterfaces.SWD)
        self.jl.set_speed(SPEED)
        self.jl.connect(DEVICE)
        return self

    def close(self):
        if self.jl:
            try:
                self.jl.close()
            except Exception:
                pass
            self.jl = None

    BLOCK = 4096
    PAGE = 256

    def reconnect(self):
        """Verbindung neu aufbauen.

        Without it you do not read flash after programming, you read the
        loader's buffer: it cheerfully reports what ought to be there. A
        half-written image then goes unnoticed - it starts, copies .data out
        of erased flash and fails later at some sanity check in the engine,
        far from the cause. A reset(halt=True) is not enough either; only
        close/open clears it away.
        """
        try:
            self.jl.close()
        except Exception:
            pass
        time.sleep(0.3)
        self.jl = pylink.JLink(lib=pylink.library.Library(DLL))
        self.jl.open(serial_no=SN)
        self.jl.set_tif(pylink.enums.JLinkInterfaces.SWD)
        self.jl.set_speed(SPEED)
        self.jl.connect(DEVICE)
        self.jl.halt()

    def flash(self, path=BIN, tries=6):
        """Loeschen, schreiben, neu verbinden, vergleichen, Luecken stopfen.

        The image is padded to a whole 256-byte page: otherwise the
        J-Link's loader reproducibly fails on the partial last page, and
        that page is exactly where the load image of .data ends.
        """
        if not os.path.exists(path):
            raise SystemExit("fehlt: %s" % path)
        data = open(path, "rb").read()
        if len(data) % self.PAGE:
            data += bytes([0xFF]) * (self.PAGE - len(data) % self.PAGE)
        # The extension has to stay .bin: that is how the loader picks the format.
        tmp = path[:-4] + "_padded.bin" if path.lower().endswith(".bin") else path + ".bin"
        open(tmp, "wb").write(data)

        self.jl.set_speed(1000)
        self.jl.halt()
        self.jl.erase()
        err = None
        try:
            self.jl.flash_file(tmp, 0x08000000)
        except Exception as e:
            err = e

        for attempt in range(1, tries + 1):
            self.reconnect()
            bad = self.bad_blocks(data)
            if not bad:
                extra = ""
                if attempt > 1:
                    extra = " (%d Nachbesserung%s)" % (attempt - 1, "" if attempt == 2 else "en")
                elif err is not None:
                    extra = " (Loader meldete: %s)" % err
                print("[flash] %s (%d B) written and verified after reconnecting%s" %
                      (os.path.basename(path), len(data), extra))
                break
            if attempt == tries:
                raise SystemExit("Flashen fehlgeschlagen, %d Bloecke bleiben falsch" % len(bad))
            # The loader refuses to rewrite single blocks ("Unspecified
            # error"), so the whole thing runs again.
            # Welche Seiten er verliert, wechselt von Lauf zu Lauf, darum
            # kommt ein sauberer Durchgang in aller Regel nach ein bis zwei
            # Versuchen zustande.
            print("[flash] Versuch %d: %d Block/Bloecke falsch, erster bei 0x%08X"
                  % (attempt, len(bad), 0x08000000 + bad[0]))
            self.jl.set_speed(1000)
            self.jl.halt()
            self.jl.erase()
            try:
                self.jl.flash_file(tmp, 0x08000000)
                err = None
            except Exception as e:
                err = e

        try:
            os.remove(tmp)
        except OSError:
            pass
        self.jl.set_speed(SPEED)
        self.jl.reset(halt=False)
        self.jl.restart()
        self.dbg = None
        self.cursor = 0
        time.sleep(0.8)

    def bad_blocks(self, data):
        """Offsets of all blocks that do not match the file."""
        out = []
        for off in range(0, len(data), self.BLOCK):
            n = min(self.BLOCK, len(data) - off)
            if bytes(self.jl.memory_read8(0x08000000 + off, n)) != data[off:off + n]:
                out.append(off)
        return out

    def verify(self, data):
        """Offset of the first difference, or None."""
        step = 4096
        for off in range(0, len(data), step):
            n = min(step, len(data) - off)
            back = bytes(self.jl.memory_read8(0x08000000 + off, n))
            if back != data[off:off + n]:
                for i in range(n):
                    if back[i] != data[off + i]:
                        return off + i
        return None

    # -- debug ring --
    def find(self):
        """Address of the ring: from the ELF first, searching RAM second.

        The order is not arbitrary. A search through RAM finds *a* magic,
        not necessarily the one belonging to the firmware that is running:
        a reset does not clear RAM, and an image with a different layout
        simply leaves the previous version's ring standing. Measured, that
        cost four bytes of offset and twenty minutes of debugging - the
        values looked plausible but were two firmwares old. A ring found by
        searching therefore says so now.

        The names "arm-none-eabi-nm" and "nm" never existed on this
        machine; the ELF lives in WSL, and so does llvm-nm.
        """
        env = dict(os.environ, MSYS2_ARG_CONV_EXCL="*")
        for cmd in (["wsl.exe", "-d", WSL_DISTRO, "--", NM_WSL, ELF_LINUX],
                    ["arm-none-eabi-nm", ELF_WSL], ["nm", ELF_WSL]):
            try:
                out = subprocess.run(cmd, capture_output=True, text=True, env=env).stdout
            except FileNotFoundError:
                continue
            for line in out.splitlines():
                p = line.split()
                if len(p) == 3 and p[2] == "dbg":
                    a = int(p[0], 16)
                    if self._ok(a):
                        self.dbg = a
                        return a
                    raise SystemExit(
                        "The ELF says dbg = 0x%08X, and there is no ring "
                        "there.\nIs this image really running? (compare the "
                        "flash against the BIN)" % a)
        for base in range(0x20000000, 0x20010000, 0x1000):
            blk = bytes(self.jl.memory_read8(base, 0x1000))
            for off in range(0, 0x1000, 4):
                if struct.unpack_from("<I", blk, off)[0] == DBG_MAGIC:
                    if self._ok(base + off):
                        self.dbg = base + off
                        print("[dbg] ring searched for in RAM and found at "
                              "0x%08X - without the ELF there is no telling which "
                              "firmware it belongs to." % self.dbg)
                        return self.dbg
        raise SystemExit("debug ring not found - is the firmware running?")

    def _ok(self, a):
        try:
            v = self.jl.memory_read32(a, 2)
            return v[0] == DBG_MAGIC and 0 < v[1] <= 65536
        except Exception:
            return False

    def _need(self):
        if self.dbg is None:
            self.find()
        return self.dbg

    def rd(self, off, n=1):
        v = self.jl.memory_read32(self._need() + off, n)
        return v[0] if n == 1 else list(v)

    def size(self):
        return self.rd(O_SIZE)

    def new_text(self):
        sz = self.size()
        wr = self.rd(O_WR)
        if self.cursor == 0 and wr > sz:
            self.cursor = wr - sz
        if wr == self.cursor:
            return ""
        lost = 0
        if wr - self.cursor > sz:
            lost = wr - self.cursor - sz
            self.cursor = wr - sz
        start = self.cursor % sz
        n = wr - self.cursor
        base = self._need() + O_BUF
        if start + n <= sz:
            raw = bytes(self.jl.memory_read8(base + start, n))
        else:
            k = sz - start
            raw = bytes(self.jl.memory_read8(base + start, k)) + \
                  bytes(self.jl.memory_read8(base, n - k))
        self.cursor = wr
        # The ring can hold leftovers after a firmware change, and the
        # console here is cp1252: anything outside printable ASCII becomes a
        # dot rather than making the print fail.
        t = "".join(chr(c) if 32 <= c < 127 or c in (10, 13) else "." for c in raw)
        if lost:
            t = "<<%d Bytes verloren>>\n" % lost + t
        return t

    def pump(self, prefix=""):
        t = self.new_text()
        if not t:
            return
        t = self.tail + t
        lines = t.split("\n")
        self.tail = lines.pop()
        for ln in lines:
            ln = ln.rstrip("\r")
            if ln:
                print(prefix + ln)

    def state(self):
        st = self.rd(O_STAGE)
        return dict(stage=st, stage_name=STAGES.get(st, "?"), ms=self.rd(O_MS),
                    wr=self.rd(O_WR), frames=self.rd(O_FRAMES),
                    buttons=self.rd(O_BUTTONS), fps=self.rd(O_FPS) / 100.0)


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 1
    what = argv[1]

    if what == "shot":
        sys.path.insert(0, os.path.join(
            r"C:\Users\pc\Desktop\VapeDoom\Vape_DOOM_ScreenShare\PixoAspire_Vape_FW", "tools"))
        import cam
        p = argv[2] if len(argv) > 2 else os.path.join(ROOT, "captures", "doom.png")
        os.makedirs(os.path.dirname(os.path.abspath(p)), exist_ok=True)
        cam.shot_crop(p, settle=0.5)
        cam.release()
        print("[cam] %s" % p)
        return 0

    v = Vape().open()
    try:
        if what in ("flash", "run"):
            # Optionaler Pfad: jeder WAD-Build hat sein eigenes Verzeichnis,
            # because firmware and data have to come from the same GEN tree.
            path = argv[2] if len(argv) > 2 and argv[2].lower().endswith(".bin") else BIN
            v.flash(path)
            v.find()
            print("[dbg] ring at 0x%08X" % v.dbg)
            time.sleep(0.5)
            v.pump("  | ")
            s = v.state()
            print("[sys] stage %d (%s), %d ms" % (s["stage"], s["stage_name"], s["ms"]))
        if what in ("log", "run"):
            v.find()
            print("[log] Ctrl-C ends it")
            last = None
            while True:
                v.pump()
                s = v.state()
                if last is not None and s["ms"] == last:
                    pass
                last = s["ms"]
                time.sleep(0.05)
        elif what == "state":
            v.find()
            s = v.state()
            print("ring 0x%08X  stage %d (%s)  uptime %.1f s" %
                  (v.dbg, s["stage"], s["stage_name"], s["ms"] / 1000.0))
            print("frames %d  frame rate %.2f fps" % (s["frames"], s["fps"]))
            b = s["buttons"]
            print("buttons 0x%02X %s" % (b, " ".join(n for i, n in enumerate(BTN) if b & (1 << i)) or "-"))
            a = v.rd(O_MS)
            time.sleep(0.3)
            b = v.rd(O_MS)
            print("sign of life: %+d ms in 300 ms %s" % (b - a, "(running)" if b > a else "(STOPPED)"))
        elif what not in ("flash", "run"):
            print(__doc__)
            return 1
    except KeyboardInterrupt:
        pass
    finally:
        v.close()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
