#!/usr/bin/env python3
"""Shows where a frame loses its time. Values out of the debug block."""
import os, sys, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from vapedoom import Vape
O = dict(ms=0x10, frames=0x14, fps=0x1C, nor_cyc=0x20, nor_calls=0x24,
         nor_bytes=0x28, pump_cyc=0x2C, pump_px=0x30)

def main(wait=12, span=8):
    v = Vape().open(); v.find()
    try:
        time.sleep(wait)
        a = {k: v.rd(o) for k, o in O.items()}
        time.sleep(span)
        b = {k: v.rd(o) for k, o in O.items()}
    finally:
        v.close()
    d = {k: b[k] - a[k] for k in O}
    f = max(d["frames"], 1)
    ms = max(d["ms"], 1)
    print("%d Bilder in %d ms = %.2f fps   (%.1f ms je Bild)" %
          (d["frames"], ms, d["frames"] * 1000.0 / ms, ms / float(f)))
    nor = d["nor_cyc"] / 144000.0 / f
    pump = d["pump_cyc"] / 144000.0 / f
    rest = (ms * 144000.0 - d["nor_cyc"] - d["pump_cyc"]) / 144000.0 / f
    print("  NOR    %6.1f ms  %5d Zugriffe  %7d B  (%.1f us je Zugriff, %d B)" %
          (nor, d["nor_calls"] // f, d["nor_bytes"] // f,
           d["nor_cyc"] / 144.0 / max(d["nor_calls"], 1),
           d["nor_bytes"] // max(d["nor_calls"], 1)))
    print("  Panel  %6.1f ms  %5d Pixel" % (pump, d["pump_px"] // f))
    print("  Engine %6.1f ms" % rest)
    return 0

if __name__ == "__main__":
    sys.exit(main(*(int(x) for x in sys.argv[1:])))
