# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
#
# Build rules of the converter: generated firmware sources in gen/ and NOR packs in build/packs/.
# The Freedoom zip comes from $(FREEDOOM_ZIP), dl/ or a verified download,
# IWAD=path takes a local IWAD instead (see the Makefile).
#
#   make -j$(nproc) gen          gen/ sources, the other builds depend on $(CONV_GEN_FILES)
#   make -j$(nproc) packs        the packs in CONV_PACKS (default E1M1,E1)
#   make -j$(nproc) test-conv    converter tests on those packs

CONV := python3 -I tools/conv/__main__.py$(if $(IWAD), --wad $(IWAD)) --gen $(GEN)
CONV_PACKS ?= E1M1,E1
CONV_COMMA := ,
CONV_PACK_DIRS = $(addprefix $(BUILD)/packs/,$(subst $(CONV_COMMA), ,$(CONV_PACKS)))
CONV_GEN_FILES := $(addprefix $(GEN)/,dmcdat.h info.h info.c trig_tables.h trig_tables.c view.c colormap.c)
CONV_DEPS := $(wildcard tools/conv/*.py tools/conv/dumper/*.c) mk/conv.mk $(IWAD)
CONV_GEN_STAMP := $(BUILD)/conv/gen.stamp

.PHONY: conv gen packs test-conv

all: conv
test: test-conv

conv: packs

gen: $(CONV_GEN_FILES)

# The converter rewrites a gen/ file only when its content changes, so files
# that depend on gen/ rebuild only on real changes. The stamp marks the last
# run and the group recipe restores deleted files. The converter reads the
# upstream tables from the doomgeneric checkout that oracle/fetch.sh makes
# (the stamp of mk/engine.mk).
$(CONV_GEN_STAMP): $(CONV_DEPS) | $(BUILD)/oracle/fetched.stamp
	$(CONV) gen
	@mkdir -p $(@D) && touch $@

$(CONV_GEN_FILES) &: $(CONV_GEN_STAMP)
	@for f in $(CONV_GEN_FILES); do test -f $$f || { $(CONV) gen; break; }; done

# The pack step leaves gen/ alone. It runs after gen so the two never build
# the dumper at the same time.
packs: $(CONV_GEN_FILES)
	$(CONV) --packs $(CONV_PACKS) --out $(BUILD)/packs packs

# The checks against the WAD read $(IWAD) when given
CONV_TEST_WAD := $(if $(IWAD),--wad $(IWAD))
test-conv: packs
	python3 -I tests/conv/test_trig.py
	python3 -I tools/dmccheck.py $(CONV_PACK_DIRS)
	python3 -I tools/dmcview.py $(CONV_TEST_WAD) $(CONV_PACK_DIRS)
	@set -e; for p in $(CONV_PACK_DIRS); do \
		echo "python3 -I tests/conv/test_maps.py $(CONV_TEST_WAD) --gen $(GEN) $$p"; \
		python3 -I tests/conv/test_maps.py $(CONV_TEST_WAD) --gen $(GEN) $$p; done
	python3 -I tests/conv/test_determinism.py
