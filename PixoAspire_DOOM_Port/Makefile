# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
#
# Top-level build. The rules of each part of the tree are in mk/*.mk.
# Run with: make -j$(nproc) <target>
#   fw-package   the files for the meter in build/device/ (README.md)
#   all          everything, the host engine and the test tools included
#   test         the host test suite

BUILD := build
GEN := gen

# IWAD=path converts a local IWAD instead of the Freedoom zip (mk/conv.mk).
# Such a build needs its own BUILD directory, its generated sources go to
# $(BUILD)/gen unless GEN is given.
ifneq ($(IWAD),)
ifeq ($(abspath $(BUILD)),$(abspath build))
$(error IWAD needs its own BUILD directory, for example BUILD=build/sw)
endif
ifneq ($(origin GEN),command line)
GEN := $(BUILD)/gen
endif
ifeq ($(abspath $(GEN)),$(abspath gen))
$(error IWAD needs its own GEN directory)
endif
endif

.PHONY: all clean
all:

-include mk/*.mk

clean:
	rm -rf $(BUILD) $(GEN)
