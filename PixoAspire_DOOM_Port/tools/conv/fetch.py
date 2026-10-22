# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
"""Freedoom 0.13.0 release: local zip or download, SHA-256 check, in-memory extract."""

import hashlib
import io
import os
import urllib.request
import zipfile

from .util import REPO, ConvError, mkdirs, tmp_name

ZIP_NAME = "freedoom-0.13.0.zip"
ZIP_URL = "https://github.com/freedoom/freedoom/releases/download/v0.13.0/" + ZIP_NAME
ZIP_SHA256 = "3f9b264f3e3ce503b4fb7f6bdcb1f419d93c7b546f4df3e874dd878db9688f59"
WAD_MEMBER = "freedoom-0.13.0/freedoom1.wad"
NOTICE_MEMBERS = ("freedoom-0.13.0/COPYING.txt", "freedoom-0.13.0/CREDITS.txt")
DL_DIR = os.path.join(REPO, "dl")


def _sha256(data):
    return hashlib.sha256(data).hexdigest()


def _verify(data, where):
    got = _sha256(data)
    if got != ZIP_SHA256:
        raise ConvError("%s: SHA-256 %s does not match %s" % (where, got, ZIP_SHA256))
    return data


def _read(path):
    try:
        with open(path, "rb") as f:
            return f.read()
    except OSError as e:
        raise ConvError("%s: %s" % (path, e.strerror or e))


def zip_bytes(zip_path=None):
    """Return the verified release zip. Order: argument, $FREEDOOM_ZIP, dl/ cache, download."""
    path = zip_path or os.environ.get("FREEDOOM_ZIP")
    if path:
        return _verify(_read(path), path)
    cached = os.path.join(DL_DIR, ZIP_NAME)
    if os.path.exists(cached):
        return _verify(_read(cached), cached)
    mkdirs(DL_DIR)
    try:
        with urllib.request.urlopen(ZIP_URL, timeout=120) as r:
            data = r.read()
    except OSError as e:
        raise ConvError("download of %s failed: %s" % (ZIP_URL, e))
    _verify(data, ZIP_URL)
    tmp = tmp_name(cached)
    with open(tmp, "wb") as f:
        f.write(data)
    os.replace(tmp, cached)
    return data


def read_wad(path):
    """Bytes of a local IWAD given instead of the release zip."""
    return _read(path)


def extract(zip_path=None):
    """Return (wad bytes, {notice name: bytes})."""
    z = zipfile.ZipFile(io.BytesIO(zip_bytes(zip_path)))
    wad = z.read(WAD_MEMBER)
    notices = {os.path.basename(n): z.read(n) for n in NOTICE_MEMBERS}
    return wad, notices
