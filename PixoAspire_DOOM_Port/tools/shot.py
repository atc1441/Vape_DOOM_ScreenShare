#!/usr/bin/env python3
"""
shot.py - photographs of the vape's screen, brightened.

    python shot.py file.png [count]

In daylight the camera looks at a black glossy pane. What it delivers is the
display *plus* the window behind it, and the window is brighter. Two steps
help, and neither changes the content:

  averaging  Lay several frames on top of each other. The sensor's noise is
             random and disappears, the picture stays. At 15 fps on the
             device that is no smearing either, as long as nothing moves.
  stretching The content occupies only a narrow part of the brightness
             range, because the reflection adds a pedestal. Subtracting the
             lower percentile and stretching to the full range brings it
             back.

What such a picture is not: a measurement. It shows that something is
visible, and what. Whether the right thing is being rendered is in the
engine's state (tools/touch.py state), not in a photo through a mirroring
pane.
"""

import os
import sys
import time

import numpy as np

# cam.py lives in the screen-share firmware's tools.
sys.path.insert(0, os.environ.get(
    "VAPE_CAM_TOOLS",
    r"C:\Users\pc\Desktop\VapeDoom\Vape_DOOM_ScreenShare"
    r"\PixoAspire_Vape_FW\tools"))
import cam  # noqa: E402
import cv2  # noqa: E402

LO, HI = 2.0, 99.5  # percentiles the image is stretched to

# The crop in cam.py covers more than the display. Cropped tighter the
# picture carries more content per pixel; VAPE_ROI="x0,y0,x1,y1" changes it.
if os.environ.get("VAPE_ROI"):
    cam.ROI = tuple(int(v) for v in os.environ["VAPE_ROI"].split(","))


def grab(n=6, settle=0.5):
    """Average n frames and crop to the ROI."""
    acc = None
    for i in range(n):
        f = cam.shot(None, settle=settle if i == 0 else 0.0)
        c = cam.crop(f, None, 1).astype(np.float32)
        acc = c if acc is None else acc + c
    return acc / float(n)


def reference(n, change, settle=0.25):
    """Estimate the reflection.

    The room lies on the pane: a window on the left, the flasher's blue
    case on the right. What the camera sees is display *plus* that
    reflection, and the reflection is brighter than the game. It stands
    still, though, while the picture changes.

    So: many frames, turning the view in the game in between, and take the
    minimum per pixel. Every point of the display is dark at some moment,
    the reflection never is - what is left is the reflection alone.
    Subtracted from a frame, the picture stays.

    `change` is called between frames and is meant to change the picture
    (in game: turn).

    Measured, this did not do enough here to leave it on by default: a Doom
    scene changes less while turning than the arithmetic needs. Many points
    stay bright across all frames - walls stand in similar places - so the
    minimum still contains picture, and subtracting it leaves noise. The
    factor in shot(..., ref_gain=) can be turned up when the scene changes
    more strongly; without it the picture is currently better.""" 
    acc = None
    for i in range(n):
        change(i)
        time.sleep(settle)
        c = cam.crop(cam.shot(None), None, 1).astype(np.float32)
        acc = c if acc is None else np.minimum(acc, c)
    return acc


def stretch(img):
    out = np.empty_like(img)
    for ch in range(img.shape[2]):
        lo = np.percentile(img[:, :, ch], LO)
        hi = np.percentile(img[:, :, ch], HI)
        if hi - lo < 1.0:
            hi = lo + 1.0
        out[:, :, ch] = np.clip((img[:, :, ch] - lo) * 255.0 / (hi - lo), 0, 255)
    return out.astype(np.uint8)


def shot(path, n=6, scale=2, ref=None, ref_gain=0.0):
    img = grab(n)
    if ref is not None and ref_gain > 0.0:
        img = np.clip(img - ref_gain * ref, 0, 255)
    img = stretch(img)
    if scale != 1:
        img = cv2.resize(img, None, fx=scale, fy=scale, interpolation=cv2.INTER_CUBIC)
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    cv2.imwrite(path, img)
    return img


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 1
    n = int(argv[2]) if len(argv) > 2 else 6
    img = shot(argv[1], n)
    cam.release()
    print("[cam] %s  %dx%d  from %d frames" % (argv[1], img.shape[1], img.shape[0], n))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
