# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
#
# Build rules of the object store (port/obj): lockstep of the
# normal mode with corpses at rest masked, of the promote everything mode
# against both oracles, the counted events, the corpus runs and the RAM
# layout check. Run from the repository root with make -j$(nproc).
#
#   obj-test     every lockstep workload: normal mode against the masked
#                oracle (opt and sanitizer builds), the counted events must be 0,
#                promote everything against the masked and the unmasked
#                oracle
#   obj-corpus   the corpus bot of the oracle on every E1 map at skills 4,
#                3 and 2 with two seeds, 8000-tic lives, replayed by the
#                engine in both modes (normal also under the sanitizer):
#                lockstep up to the first counted event, lives as long as
#                the run, zombies and A_BossDeath covered, peaks against
#                the plans of the image
#   ramcheck     RAM layout per map and skill group within 32,768 B
#   obj-stress   the sanitizer build with few pages and mover slots: wake-ups
#                wait, transients are skipped, activations are deferred, the
#                game must go on (no lockstep, the events change the game)
#
# The size gates G0 and G1 (game logic with port/obj) are the sizegate
# target of mk/engine.mk.

.PHONY: obj-test obj-corpus ramcheck obj-stress
test: obj-test ramcheck obj-stress

OBJ_PY ?= python3
OBJ_LS := $(OBJ_PY) tests/lockstep/run_lockstep.py
OBJ_ASAN_SKIP := $(BUILD)/host/dmcdoom-asan_skip

# The sanitizer build with the masked hash.
$(OBJ_ASAN_SKIP): $(filter-out %/hash.o,$(call ENG_HOST_OBJS,$(BUILD)/host/asan)) \
		$(BUILD)/host/asan/hash_skip.o
	$(ENG_CC) -fsanitize=address,undefined $(ENG_HOST_WRAP) -o $@ $^ -lm
$(BUILD)/host/asan/hash_skip.o: tests/lockstep/hash.c $(CONV_GEN_FILES) $(ENG_SAN_IGNORE) \
		| $(BUILD)/host/asan
	$(ENG_CC) $(ENG_HOST_CFLAGS) $(ENG_ASAN) $(ENG_SKIP) -c $< -o $@

OBJ_BINS := oracle $(ORA_OUT)/dg_oracle_skip $(BUILD)/host/dmcdoom $(BUILD)/host/dmcdoom_skip \
	$(OBJ_ASAN_SKIP) nor-images

obj-test: $(OBJ_BINS)
	$(OBJ_LS) --oracle $(ORA_OUT)/dg_oracle_skip --engine $(BUILD)/host/dmcdoom_skip \
		--engine $(OBJ_ASAN_SKIP) --obj-stats --tag obj-normal
	$(OBJ_LS) --oracle $(ORA_OUT)/dg_oracle_skip --engine $(BUILD)/host/dmcdoom_skip \
		--engine-arg=--promote-all --obj-stats --tag obj-promote-skip
	$(OBJ_LS) --oracle $(ORA_OUT)/dg_oracle --engine $(BUILD)/host/dmcdoom \
		--engine-arg=--promote-all --obj-stats --tag obj-promote

obj-corpus: $(OBJ_BINS)
	$(OBJ_LS) --oracle $(ORA_OUT)/dg_oracle_skip --engine $(BUILD)/host/dmcdoom_skip \
		--engine $(OBJ_ASAN_SKIP) --workload corpus --obj-stats --tag obj-corpus
	$(OBJ_LS) --oracle $(ORA_OUT)/dg_oracle --engine $(BUILD)/host/dmcdoom \
		--engine-arg=--promote-all --workload corpus --obj-stats --tag obj-corpus-promote

ramcheck: $(BUILD)/host/dmcdoom nor-images $(ENG_ARM_OUT)/engine.elf
	$(OBJ_PY) tests/host/ramcheck.py --engine $(BUILD)/host/dmcdoom --nor $(NOR_OUT)/E1.img \
		--map $(ENG_ARM_OUT)/engine.map

# DEMO1 and the script on every E1 map with the pages capped at 12-40 and
# one or two mover slots, then with no reserve pages, where corpse pages
# and transients are evicted and missiles are lost: each run must end
# normally under the sanitizer.
OBJ_STRESS_DIR := $(BUILD)/lockstep/obj-stress
OBJ_STRESS := ../../host/dmcdoom-asan_skip --nor ../../nor/E1.img
obj-stress: $(OBJ_ASAN_SKIP) nor-images
	mkdir -p $(OBJ_STRESS_DIR)
	$(OBJ_PY) -c "import sys; sys.path.insert(0, 'tests/lockstep'); import run_lockstep as L; \
		L.make_cmds(sys.argv[1], 2500)" $(OBJ_STRESS_DIR)/stress.cmds
	@set -e; cd $(OBJ_STRESS_DIR); rm -f *.txt; \
	for p in 40 25 12; do \
		$(OBJ_STRESS) -playdemo demo1 --obj-pages $$p --obj-movers 2 --obj-stats demo1-$$p.txt \
			> /dev/null; done; \
	for m in 1 2 3 4 5 6 7 8 9; do \
		$(OBJ_STRESS) -warp 1 $$m -skill 4 --cmds stress.cmds --tics 2500 --obj-pages 20 \
			--obj-movers 1 --obj-stats e1m$$m.txt > /dev/null; done; \
	$(OBJ_STRESS) -playdemo demo1 --obj-pages 10 --obj-reserve 2 --obj-stats r-demo1.txt > /dev/null; \
	for m in 4 5; do for p in 9 12; do \
		$(OBJ_STRESS) -warp 1 $$m -skill 4 --cmds stress.cmds --tics 2500 --obj-pages $$p \
			--obj-reserve 0 --obj-stats r-e1m$$m-$$p.txt > /dev/null; done; done; \
	cat *.txt | awk '{ for (i = 1; i < NF; i++) if ($$i ~ /^ev_|evicted/) n[$$i] += $$(i + 1) } \
		END { for (k in n) printf "%s %d ", k, n[k]; print "(all stress runs)" }'
	@echo "obj-stress: PASS"
