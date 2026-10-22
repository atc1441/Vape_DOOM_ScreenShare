# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
#
# Build rules of the game I/O: button chord resolver and PC-speaker sound (port/game/), host tests.
#
#   make -j$(nproc) game-test   input and sound host tests, ARM -Oz size gate of both modules
#   make -j$(nproc) game-wav    every sound of the pack as a square-wave WAV in build/game/wav/
#   make -j$(nproc) pad-test    the game pad (port/game/pad.h) in the host engine, driven by --pad
#
# The RC-N1 pad driver and its test are in controller/rc-n1 (mk/rc-n1.mk).
#
# The sound test reads $(GAME_PACK)/DOOM.DAT (one of the packs of the packs target) and the
# Freedoom zip from $(FREEDOOM_ZIP) or dl/.

GAME_CC   ?= clang
GAME_PY   ?= python3
GAME_SIZE ?= llvm-size
GAME_NM   ?= llvm-nm
GAME_PACK ?= $(BUILD)/packs/E1M1

GAME_B := $(BUILD)/game
GAME_WARN := -Wall -Wextra -Wshadow -Wstrict-prototypes
# Upstream headers are not held to our warning set
GAME_INC := -Iport/game -Iplatform -Iport/data -isystem engine -I$(GEN)
GAME_SAN := -fsanitize=address,undefined -fno-sanitize-recover=all
GAME_HCFLAGS := -std=c99 -O1 -g -fwrapv -fsigned-char $(GAME_WARN) $(GAME_SAN) $(GAME_INC) -MMD -MP
# Upstream engine files as mk/engine.mk builds them
GAME_ECFLAGS := -std=gnu11 -O1 -g -fwrapv -fsigned-char -w $(GAME_SAN) -Iengine -Iport/data \
	-I$(GEN) -MMD -MP
GAME_ARMFLAGS := --target=thumbv7em-none-eabi -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -std=c99 \
	-Oz -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fno-common -fwrapv \
	-fsigned-char $(GAME_WARN) -Iengine/stub/include $(GAME_INC) -MMD -MP

GAME_MODS := i_input s_pcspk
# Flash gates (ARM -Oz .text + .rodata + .data) per module
GAME_GATE_i_input := 1184
GAME_GATE_s_pcspk := 380
GAME_ARM_OBJS := $(GAME_MODS:%=$(GAME_B)/arm/%.o)

$(GAME_B)/host/%.o: port/game/%.c mk/game.mk | $(CONV_GEN_FILES)
	@mkdir -p $(dir $@)
	$(GAME_CC) $(GAME_HCFLAGS) -c $< -o $@
$(GAME_B)/host/%.o: tests/game/%.c mk/game.mk | $(CONV_GEN_FILES)
	@mkdir -p $(dir $@)
	$(GAME_CC) $(GAME_HCFLAGS) -c $< -o $@
$(GAME_B)/host/eng/%.o: engine/%.c mk/game.mk | $(CONV_GEN_FILES)
	@mkdir -p $(dir $@)
	$(GAME_CC) $(GAME_ECFLAGS) -c $< -o $@
# The generated sound table (gen/info.c), with the sound names of DMC_HOST
$(GAME_B)/host/gen_info.o: $(GEN)/info.c mk/game.mk | $(CONV_GEN_FILES)
	@mkdir -p $(dir $@)
	$(GAME_CC) -std=c99 -O1 -g -fwrapv -fsigned-char $(GAME_SAN) -I$(GEN) -DDMC_HOST -MMD -MP -c $< -o $@
$(GAME_B)/host/pcspk_main.o: GAME_HCFLAGS += -DDMC_HOST
$(GAME_B)/host/buzzer.o: platform/buzzer.c tests/game/regs.h mk/game.mk
	@mkdir -p $(dir $@)
	$(GAME_CC) $(GAME_HCFLAGS) -DPLAT_MODEL -include tests/game/regs.h -c $< -o $@
$(GAME_B)/arm/%.o: port/game/%.c mk/game.mk | $(CONV_GEN_FILES)
	@mkdir -p $(dir $@)
	$(GAME_CC) $(GAME_ARMFLAGS) -c $< -o $@

$(GAME_B)/input_test: $(addprefix $(GAME_B)/host/,input_main.o i_input.o eng/m_controls.o)
	$(GAME_CC) $(GAME_SAN) $^ -o $@
$(GAME_B)/pcspk_test: $(addprefix $(GAME_B)/host/,pcspk_main.o s_pcspk.o buzzer.o eng/s_sound.o gen_info.o)
	$(GAME_CC) $(GAME_SAN) $^ -o $@

GAME_TEST := $(GAME_PY) -I tests/game/test_game.py --build $(GAME_B) --pack $(GAME_PACK)

.PHONY: game-test game-wav pad-test
test: game-test pad-test

game-test: $(GAME_B)/input_test $(GAME_B)/pcspk_test $(GAME_ARM_OBJS) packs
	@set -e; for m in $(foreach m,$(GAME_MODS),$(m):$(GAME_GATE_$(m))); do \
		o=$(GAME_B)/arm/$${m%:*}.o; g=$${m#*:}; \
		u=$$($(GAME_NM) -u $$o | awk '{print $$2}' | tr '\n' ' '); echo "$$o needs: $$u"; \
		$(GAME_SIZE) -A $$o | awk -v f=$$o -v g=$$g '$$1 ~ /^\.(text|rodata)/ { t += $$2 } \
			$$1 ~ /^\.data/ { d += $$2 } $$1 ~ /^\.bss/ { b += $$2 } \
			END { printf "%s: flash %d B (gate %d), RAM %d B (ARM -Oz)\n", f, t + d, g, d + b; \
			      if (t + d > g) { print "FAIL: " f " over its flash gate"; exit 1 } }'; done
	$(GAME_TEST) --wav-dir $(GAME_B)/wav

game-wav: $(GAME_B)/pcspk_test packs
	$(GAME_TEST) --sound-only --wav-dir $(GAME_B)/wav --wav-all

pad-test: $(BUILD)/host/dmcdoom nor-images
	$(GAME_PY) -I tests/game/test_pad.py --engine $(BUILD)/host/dmcdoom --nor $(NOR_OUT)/E1.img

-include $(wildcard $(GAME_B)/host/*.d $(GAME_B)/host/eng/*.d $(GAME_B)/arm/*.d)
