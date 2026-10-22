# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
#
# Build rules of the engine: lockstep oracle, engine host builds on the NOR image, the
# data layer (port/data) and its tests, lockstep, accessor parity, frame and
# the ARM size gate. Run from the repository root with make -j$(nproc).
#
#   host            build/host/dmcdoom and dmcdoom-asan (read the NOR image only)
#   lockstep        every workload against the oracle, also lockstep-skip
#   data-test       data layer on plain, fragmented and damaged NOR images
#   parity-test     every workload with the accessor-parity build
#   frame-test      3D view pixels against the oracle on the E1M1 pack, with
#                   async NOR reads completing at once and deferred
#   screen-test     the front-end screens (title, intermissions, finale,
#                   menu) against the oracle (tests/host/screentest.py)
#   frame-model-test
#                   the frame model of the meter (dmcdoom-frame): the exits,
#                   episode, menu and exitlist workloads, the screens and saves
#   save-test       the saves of SAVE.DAT: test saves and loads, a save and a
#                   load from the menu, the slot headers, the making of
#                   SAVE.DAT and power loss (tests/host/save*.py)
#   sizegate        ARM -Oz size report, gates G0, G1, G2 and the G3 report

.PHONY: oracle oracle-fetch host lockstep lockstep-skip engine-arm sizegate test gates
all: oracle host engine-arm
test: lockstep data-test parity-test frame-test screen-test frame-model-test save-test
gates: sizegate

ENG_CC ?= clang
ENG_PY ?= python3

# ---------------------------------------------------------------- oracle

ORA_SRC := oracle/src/doomgeneric-repo/doomgeneric
ORA_OUT := $(BUILD)/oracle
ORA_UP := dummy am_map doomdef doomstat dstrings d_event d_items d_iwad d_loop d_main \
	d_mode d_net f_finale f_wipe g_game hu_lib hu_stuff info i_cdmus i_endoom i_joystick \
	i_scale i_sound i_system i_timer memio m_argv m_bbox m_cheat m_config m_controls \
	m_fixed m_menu m_misc m_random p_ceilng p_doors p_enemy p_floor p_inter p_lights \
	p_map p_maputl p_mobj p_plats p_pspr p_saveg p_setup p_sight p_spec p_switch \
	p_telept p_tick p_user r_bsp r_data r_draw r_main r_plane r_segs r_sky r_things \
	sha1 sounds statdump st_lib st_stuff s_sound tables v_video wi_stuff w_checksum \
	w_file w_main w_wad z_zone w_file_stdc i_input i_video doomgeneric
ORA_OWN := oracle/dg_headless.c oracle/hash_hook.c oracle/png.c oracle/bot.c tests/lockstep/hash.c \
	tests/lockstep/play.c tests/lockstep/keys.c
ORA_CFLAGS := -O2 -g -std=gnu11 -w -fwrapv -fsigned-char -DNORMALUNIX -DLINUX \
	-D_DEFAULT_SOURCE -DCMAP256 -DDOOMGENERIC_RESX=320 -DDOOMGENERIC_RESY=200 \
	-I$(ORA_SRC) -Ioracle -Itests/lockstep -MMD -MP
ORA_WRAP := -Wl,--wrap=G_Ticker,--wrap=G_BuildTiccmd,--wrap=I_Quit,--wrap=I_Error \
	-Wl,--wrap=G_DeferedPlayDemo,--wrap=G_TimeDemo,--wrap=I_SetPalette \
	-Wl,--wrap=P_PlayerThink,--wrap=G_ExitLevel,--wrap=G_SecretExitLevel,--wrap=M_Drawer \
	-Wl,--wrap=F_StartFinale
ORA_OBJS := $(ORA_UP:%=$(ORA_OUT)/up/%.o) $(patsubst %.c,$(ORA_OUT)/own/%.o,$(notdir $(ORA_OWN)))

oracle: $(ORA_OUT)/dg_oracle

oracle-fetch:
	sh oracle/fetch.sh

# The checkout is fetched (or verified) once per build directory before the
# first oracle object is compiled. ORACLE_MIRROR is passed through.
ORA_STAMP := $(ORA_OUT)/fetched.stamp
$(ORA_STAMP): oracle/fetch.sh | $(ORA_OUT)/up
	sh oracle/fetch.sh
	touch $@
$(ORA_UP:%=$(ORA_SRC)/%.c): | $(ORA_STAMP)
	@test -f $@ || { echo "$@ missing, run make oracle-fetch"; exit 1; }

$(ORA_OUT)/dg_oracle: $(ORA_OBJS)
	$(ENG_CC) $(ORA_WRAP) -o $@ $^ -lm

$(ORA_OUT)/up/%.o: $(ORA_SRC)/%.c | $(ORA_OUT)/up $(ORA_STAMP)
	$(ENG_CC) $(ORA_CFLAGS) -c $< -o $@

$(ORA_OUT)/own/%.o: oracle/%.c | $(ORA_OUT)/own $(ORA_STAMP)
	$(ENG_CC) $(ORA_CFLAGS) -Wall -Wextra -c $< -o $@

$(ORA_OUT)/own/hash.o: tests/lockstep/hash.c | $(ORA_OUT)/own $(ORA_STAMP)
	$(ENG_CC) $(ORA_CFLAGS) -c $< -o $@
$(ORA_OUT)/own/play.o: tests/lockstep/play.c | $(ORA_OUT)/own $(ORA_STAMP)
	$(ENG_CC) $(ORA_CFLAGS) -c $< -o $@
$(ORA_OUT)/own/keys.o: tests/lockstep/keys.c | $(ORA_OUT)/own $(ORA_STAMP)
	$(ENG_CC) $(ORA_CFLAGS) -c $< -o $@

$(ORA_OUT)/up $(ORA_OUT)/own:
	mkdir -p $@

-include $(wildcard $(ORA_OUT)/up/*.d $(ORA_OUT)/own/*.d)

# ---------------------------------------------------------------- engine, host

# The engine reads its game data from the NOR image through port/data. The
# WAD code (w_wad, w_file and the host shim) is linked only into the
# accessor-parity build, which loads the WAD to compare every record.
ENG_WAD_SRC := engine/w_wad.c engine/w_file.c
ENG_SRC := $(filter-out $(ENG_WAD_SRC),$(wildcard engine/*.c))
ENG_DATA_SRC := $(wildcard port/data/*.c)
# Strip renderer (mk/render.mk) and the table decoders, the object store
ENG_REND_SRC := $(wildcard port/render/*.c) port/math/trig.c
ENG_OBJ_SRC := $(wildcard port/obj/*.c)
ENG_GEN_SRC := $(GEN)/info.c $(GEN)/colormap.c $(GEN)/trig_tables.c $(GEN)/view.c
# The front end's glue and its overlays as the meter has them, called through the overlay
# loader in its host mode (native entry tables, port/ovl/ovl.h)
ENG_FE_SRC := port/game/fe_glue.c port/game/l_menu.c port/game/l_wi.c port/game/l_finale.c \
	port/game/l_save.c port/game/g_save.c port/ovl/ovl.c port/ovl/ovl_file.c
# The meter's chord resolver turns the game pad states of --pad into keys (host/main_host.c)
ENG_HOST_SRC := host/main_host.c host/nor_file.c host/stats.c host/i_video_host.c oracle/png.c \
	tests/lockstep/hash.c tests/lockstep/play.c tests/lockstep/keys.c port/game/i_input.c \
	$(ENG_FE_SRC)
ENG_DEFS := -fwrapv -fsigned-char -std=gnu11
# Compact sine table: ENG_SINE_L1=1 interpolates the finesine
# quarter wave between every 16th entry and adds 2-bit residuals
# (port/math/trig.c), 3.3 KB less flash. The decoder objects depend on the
# stamp of the option.
ENG_SINE_L1 ?= 0
ifeq ($(ENG_SINE_L1),1)
ENG_DEFS += -DDMC_SINE_L1
endif
ENG_OPT_STAMP := $(BUILD)/engine-options.stamp
$(ENG_OPT_STAMP): eng-force
	@mkdir -p $(dir $@)
	@echo '$(ENG_SINE_L1)' | cmp -s - $@ || echo '$(ENG_SINE_L1)' > $@
.PHONY: eng-force
ENG_INC := -Iengine -Iport/data -Iport/render -Iport/obj -Iport/math -Iplatform -Iport/game \
	-Iport/ovl -I$(GEN)
ENG_HOST_CFLAGS := $(ENG_DEFS) -g -Wall -Wno-unused-but-set-variable -Wno-unused-variable \
	-Wno-unused-function -Wno-misleading-indentation -Wno-parentheses \
	-Wno-dangling-else -Wno-pointer-sign -Wno-unused-const-variable \
	-Wno-shift-negative-value -Wno-missing-braces -Wno-unused-value \
	-Wno-pointer-to-int-cast -Wno-absolute-value \
	$(ENG_INC) -Ihost -Ioracle -Itests/lockstep -DDMC_STATS -DDMC_RSTATS -DDMC_HOST \
	-DR_FRAME_HOST=196608 -MMD -MP
# The engine uses the dense ids of gen/info.h. DMC_HOST adds the tables of
# the host tools: the vanilla numbers of the dense ids for the lockstep hash
# and the lump names for the parity build.
ENG_GEN_CFLAGS := $(ENG_DEFS) -g -Wall -I$(GEN) -DDMC_HOST -MMD -MP
ENG_HOST_WRAP := -Wl,--wrap=G_Ticker,--wrap=G_BuildTiccmd,--wrap=R_RenderPlayerView \
	-Wl,--wrap=P_PlayerThink,--wrap=G_ExitLevel,--wrap=G_SecretExitLevel,--wrap=I_StartTic \
	-Wl,--wrap=TryRunTics,--wrap=nor_erase,--wrap=nor_program
ENG_HOST_OBJS = $(patsubst %.c,$(1)/%.o,$(notdir $(ENG_SRC) $(ENG_DATA_SRC) $(ENG_REND_SRC) \
	$(ENG_OBJ_SRC) $(ENG_HOST_SRC))) \
	$(patsubst %.c,$(1)/gen_%.o,$(notdir $(ENG_GEN_SRC)))

host: $(BUILD)/host/dmcdoom $(BUILD)/host/dmcdoom-asan

$(BUILD)/host/dmcdoom: $(call ENG_HOST_OBJS,$(BUILD)/host/opt)
	$(ENG_CC) $(ENG_HOST_WRAP) -o $@ $^ -lm

$(BUILD)/host/dmcdoom-asan: $(call ENG_HOST_OBJS,$(BUILD)/host/asan)
	$(ENG_CC) -fsanitize=address,undefined $(ENG_HOST_WRAP) -o $@ $^ -lm

# Objects of a host variant: $(1) directory, $(2) extra flags
define ENG_HOST_RULES
$(1)/%.o: engine/%.c $(CONV_GEN_FILES) | $(1)
	$(ENG_CC) $(ENG_HOST_CFLAGS) $(2) -c $$< -o $$@
$(1)/%.o: port/data/%.c $(CONV_GEN_FILES) | $(1)
	$(ENG_CC) $(ENG_HOST_CFLAGS) $(2) -Wextra -c $$< -o $$@
$(1)/%.o: port/render/%.c $(CONV_GEN_FILES) | $(1)
	$(ENG_CC) $(ENG_HOST_CFLAGS) $(2) -Wextra -c $$< -o $$@
$(1)/%.o: port/obj/%.c $(CONV_GEN_FILES) | $(1)
	$(ENG_CC) $(ENG_HOST_CFLAGS) $(2) -Wextra -c $$< -o $$@
$(1)/%.o: port/math/%.c $(CONV_GEN_FILES) $(ENG_OPT_STAMP) | $(1)
	$(ENG_CC) $(ENG_HOST_CFLAGS) $(2) -Wextra -c $$< -o $$@
$(1)/%.o: host/%.c $(CONV_GEN_FILES) | $(1)
	$(ENG_CC) $(ENG_HOST_CFLAGS) $(2) -Wextra -c $$< -o $$@
$(1)/%.o: port/game/%.c $(CONV_GEN_FILES) | $(1)
	$(ENG_CC) $(ENG_HOST_CFLAGS) $(2) -DPLAT_HOST -Wextra -c $$< -o $$@
$(1)/%.o: port/ovl/%.c $(CONV_GEN_FILES) | $(1)
	$(ENG_CC) $(ENG_HOST_CFLAGS) $(2) -DPLAT_HOST -Wextra -c $$< -o $$@
$(1)/%.o: oracle/%.c | $(1)
	$(ENG_CC) $(ENG_HOST_CFLAGS) $(2) -Wextra -c $$< -o $$@
$(1)/%.o: tests/lockstep/%.c $(CONV_GEN_FILES) | $(1)
	$(ENG_CC) $(ENG_HOST_CFLAGS) $(2) -c $$< -o $$@
$(1)/gen_%.o: $(GEN)/%.c $(CONV_GEN_FILES) | $(1)
	$(ENG_CC) $(ENG_GEN_CFLAGS) $(2) -c $$< -o $$@
$(1):
	mkdir -p $$@
-include $(wildcard $(1)/*.d)
endef

# Known upstream patterns are not reported: think functions are called
# through a generic pointer type, and the out-of-bounds accesses listed in
# host/sanitize_ignore.txt.
ENG_SAN_IGNORE := host/sanitize_ignore.txt
ENG_ASAN := -O1 -fno-omit-frame-pointer -fsanitize=address,undefined \
	-fno-sanitize=function -fno-sanitize-recover=all -fsanitize-ignorelist=$(ENG_SAN_IGNORE)

$(eval $(call ENG_HOST_RULES,$(BUILD)/host/opt,-O2))
$(eval $(call ENG_HOST_RULES,$(BUILD)/host/asan,$(ENG_ASAN)))
$(BUILD)/host/asan/%.o: $(ENG_SAN_IGNORE)

# Accessor-parity build: every record read from the image is compared with
# the vanilla structures loaded from the WAD of -iwad (host/parity.c).
ENG_PARITY_DIR := $(BUILD)/host/parity
ENG_PARITY_OBJS := $(call ENG_HOST_OBJS,$(ENG_PARITY_DIR)) \
	$(patsubst %.c,$(ENG_PARITY_DIR)/%.o,$(notdir $(ENG_WAD_SRC) host/w_file_host.c host/parity.c))
$(eval $(call ENG_HOST_RULES,$(ENG_PARITY_DIR),-O1 -DDMC_PARITY))

$(BUILD)/host/dmcdoom-parity: $(ENG_PARITY_OBJS)
	$(ENG_CC) $(ENG_HOST_WRAP) -o $@ $^ -lm

# Frame test binaries: the oracle with the pixel attribution of
# tests/lockstep/attr.c, the engine with its renderer built with DMC_ATTR
# (drawers record a class per pixel, host/rattr_host.c writes the same
# files). The engine's generated tables are built with DMC_ORACLE_FRAMES,
# the frames of the oracle.
ENG_ATTR_WRAP := -Wl,--wrap=R_DrawColumn,--wrap=R_DrawColumnLow,--wrap=R_DrawTranslatedColumn \
	-Wl,--wrap=R_DrawTranslatedColumnLow,--wrap=R_DrawFuzzColumn,--wrap=R_DrawFuzzColumnLow \
	-Wl,--wrap=R_DrawSpan,--wrap=R_DrawSpanLow,--wrap=R_ClearPlanes,--wrap=R_DrawMasked
ENG_ATTR_DIR := $(BUILD)/host/attr
ENG_ATTR_REND := $(notdir $(basename $(wildcard engine/r_*.c) $(wildcard port/render/*.c)))
$(ORA_OUT)/own/attr.o: tests/lockstep/attr.c | $(ORA_OUT)/own $(ORA_STAMP)
	$(ENG_CC) $(ORA_CFLAGS) -c $< -o $@
$(ORA_OUT)/dg_oracle_attr: $(ORA_OBJS) $(ORA_OUT)/own/attr.o
	$(ENG_CC) $(ORA_WRAP) $(ENG_ATTR_WRAP) -o $@ $^ -lm
$(ENG_ATTR_DIR)/gen_info.o: $(GEN)/info.c $(CONV_GEN_FILES) | $(ENG_ATTR_DIR)
	$(ENG_CC) $(ENG_GEN_CFLAGS) -O2 -DDMC_ORACLE_FRAMES -c $< -o $@
$(ENG_ATTR_DIR)/%.o: engine/%.c $(CONV_GEN_FILES) | $(ENG_ATTR_DIR)
	$(ENG_CC) $(ENG_HOST_CFLAGS) -O2 -DDMC_ATTR -c $< -o $@
$(ENG_ATTR_DIR)/%.o: port/render/%.c $(CONV_GEN_FILES) | $(ENG_ATTR_DIR)
	$(ENG_CC) $(ENG_HOST_CFLAGS) -O2 -DDMC_ATTR -Wextra -c $< -o $@
$(ENG_ATTR_DIR)/%.o: host/%.c $(CONV_GEN_FILES) | $(ENG_ATTR_DIR)
	$(ENG_CC) $(ENG_HOST_CFLAGS) -O2 -DDMC_ATTR -Wextra -c $< -o $@
$(ENG_ATTR_DIR):
	mkdir -p $@
$(BUILD)/host/dmcdoom-attr: $(filter-out %/gen_info.o $(addprefix %/,$(ENG_ATTR_REND:=.o)), \
		$(call ENG_HOST_OBJS,$(BUILD)/host/opt)) \
		$(ENG_ATTR_DIR)/gen_info.o $(ENG_ATTR_REND:%=$(ENG_ATTR_DIR)/%.o) $(ENG_ATTR_DIR)/rattr_host.o
	$(ENG_CC) $(ENG_HOST_WRAP) -o $@ $^ -lm
-include $(wildcard $(ENG_ATTR_DIR)/*.d)

# Corpses at rest are cosmetic records of the object store (drawn at
# integer x, y on their sector's floor), so the exact comparison runs with
# every thing a full object (--promote-all): high detail at W8, low detail
# at W8 and W1, then low detail W16 and high detail W3 with async NOR reads
# deferred (column reads are sent one column ahead of the drawer). The last
# run is the normal mode with only the corpses kept as full objects
# (--keep-corpses): the views of static items, sleeping monsters and drops
# must draw the same pixels.
.PHONY: frame-test
frame-test: $(ORA_OUT)/dg_oracle_attr $(BUILD)/host/dmcdoom-attr nor-images
	$(ENG_PY) tests/host/frametest.py --oracle $(ORA_OUT)/dg_oracle_attr \
		--engine $(BUILD)/host/dmcdoom-attr --nor $(NOR_OUT)/E1M1.img --strip-w 8 \
		--engine-arg=--promote-all
	$(ENG_PY) tests/host/frametest.py --oracle $(ORA_OUT)/dg_oracle_attr \
		--engine $(BUILD)/host/dmcdoom-attr --nor $(NOR_OUT)/E1M1.img --low-detail --strip-w 8 \
		--engine-arg=--promote-all
	$(ENG_PY) tests/host/frametest.py --oracle $(ORA_OUT)/dg_oracle_attr \
		--engine $(BUILD)/host/dmcdoom-attr --nor $(NOR_OUT)/E1M1.img --low-detail --strip-w 1 \
		--engine-arg=--promote-all
	$(ENG_PY) tests/host/frametest.py --oracle $(ORA_OUT)/dg_oracle_attr \
		--engine $(BUILD)/host/dmcdoom-attr --nor $(NOR_OUT)/E1M1.img --low-detail --strip-w 16 \
		--engine-arg=--nor-defer --engine-arg=--promote-all
	$(ENG_PY) tests/host/frametest.py --oracle $(ORA_OUT)/dg_oracle_attr \
		--engine $(BUILD)/host/dmcdoom-attr --nor $(NOR_OUT)/E1M1.img --strip-w 3 \
		--engine-arg=--nor-defer --engine-arg=--promote-all
	$(ENG_PY) tests/host/frametest.py --oracle $(ORA_OUT)/dg_oracle_attr \
		--engine $(BUILD)/host/dmcdoom-attr --nor $(NOR_OUT)/E1M1.img --strip-w 8 \
		--engine-arg=--keep-corpses

# The front-end screens of the engine against the oracle's frames, with the half-width
# pictures of the E1 pack given to the oracle as build/packs/E1/halfui.wad, the menu against
# the oracle's menu, and against the same screens composed whole (--screen-full).
.PHONY: screen-test
screen-test: oracle host nor-images
	$(ENG_PY) tests/host/screentest.py --oracle $(ORA_OUT)/dg_oracle --engine $(BUILD)/host/dmcdoom

# Test saves (--save-at) against the runs without them and their loads (--load), a save and a
# load from the menu, then SAVE.DAT on images made for it: slot headers of another build or
# image, a damaged save, a SAVE.DAT of the wrong size, fragmented and full volumes and power
# loss. Last the globals a save leaves out, a list to look at that never fails.
.PHONY: save-test
save-test: $(BUILD)/host/dmcdoom_skip nor-images $(BUILD)/fwdoom/DEMO2.LMP
	$(ENG_PY) tests/host/savetest.py
	$(ENG_PY) tests/host/savemenu.py
	$(ENG_PY) tests/host/savefs.py
	$(ENG_PY) tests/host/savecheck.py

.PHONY: parity parity-test
parity: $(BUILD)/host/dmcdoom-parity

# Every workload with the parity build: it must stay in lockstep and find no
# record that differs from the WAD-loaded one.
parity-test: oracle $(BUILD)/host/dmcdoom-parity nor-images
	$(ENG_PY) tests/lockstep/run_lockstep.py --oracle $(ORA_OUT)/dg_oracle \
		--engine $(BUILD)/host/dmcdoom-parity --parity --tag parity --engine-arg=--promote-all

# ---------------------------------------------------------------- engine, ARM size gate

ENG_ARM_OUT := $(BUILD)/arm/engine
ENG_ARM_CFLAGS := --target=thumbv7em-none-eabi -mcpu=cortex-m4 -mthumb -mfloat-abi=soft \
	-Oz -ffunction-sections -fdata-sections -ffreestanding -fno-common $(ENG_DEFS) -w \
	-Iengine/stub/include $(ENG_INC) -MMD -MP
ENG_ARM_SRC := $(ENG_SRC) $(ENG_DATA_SRC) $(ENG_REND_SRC) $(ENG_OBJ_SRC) engine/stub/stub_libc.c \
	engine/stub/stub_platform.c engine/stub/stub_pump.c
ENG_ARM_OBJS := $(patsubst %.c,$(ENG_ARM_OUT)/%.o,$(notdir $(ENG_ARM_SRC))) \
	$(patsubst %.c,$(ENG_ARM_OUT)/gen_%.o,$(notdir $(ENG_GEN_SRC)))

engine-arm: $(ENG_ARM_OUT)/engine.elf

$(ENG_ARM_OUT)/engine.elf: $(ENG_ARM_OBJS) engine/stub/size.ld
	ld.lld --gc-sections -T engine/stub/size.ld -Map $(ENG_ARM_OUT)/engine.map -o $@ $(ENG_ARM_OBJS)
	@u="$$(llvm-nm -u $@)"; if [ -n "$$u" ]; then echo "$$u"; echo "undefined symbols in $@"; exit 1; fi
	llvm-size $@

$(ENG_ARM_OUT)/%.o: engine/%.c $(CONV_GEN_FILES) | $(ENG_ARM_OUT)
	$(ENG_CC) $(ENG_ARM_CFLAGS) -c $< -o $@
$(ENG_ARM_OUT)/%.o: engine/stub/%.c $(CONV_GEN_FILES) | $(ENG_ARM_OUT)
	$(ENG_CC) $(ENG_ARM_CFLAGS) -c $< -o $@
$(ENG_ARM_OUT)/%.o: port/data/%.c $(CONV_GEN_FILES) | $(ENG_ARM_OUT)
	$(ENG_CC) $(ENG_ARM_CFLAGS) -c $< -o $@
$(ENG_ARM_OUT)/%.o: port/render/%.c $(CONV_GEN_FILES) | $(ENG_ARM_OUT)
	$(ENG_CC) $(ENG_ARM_CFLAGS) -c $< -o $@
$(ENG_ARM_OUT)/%.o: port/obj/%.c $(CONV_GEN_FILES) | $(ENG_ARM_OUT)
	$(ENG_CC) $(ENG_ARM_CFLAGS) -c $< -o $@
$(ENG_ARM_OUT)/%.o: port/math/%.c $(CONV_GEN_FILES) $(ENG_OPT_STAMP) | $(ENG_ARM_OUT)
	$(ENG_CC) $(ENG_ARM_CFLAGS) -c $< -o $@
$(ENG_ARM_OUT)/gen_%.o: $(GEN)/%.c $(CONV_GEN_FILES) | $(ENG_ARM_OUT)
	$(ENG_CC) $(filter-out -I%,$(ENG_ARM_CFLAGS)) -I$(GEN) -c $< -o $@

$(ENG_ARM_OUT):
	mkdir -p $@

# G3 adds the overlay loader as the firmware builds it, the input and sound
# modules of port/game, the platform footprint and the runtime functions the
# footprint link leaves out (mk/ovl.mk, mk/game.mk, mk/platform.mk).
ENG_G3_LOADER := $(BUILD)/ovl/obj/prod/port/ovl/ovl.o $(BUILD)/ovl/obj/prod/port/ovl/ovl_file.o
ENG_G3_RESIDENT := $(BUILD)/game/arm/i_input.o $(BUILD)/game/arm/s_pcspk.o
ENG_G3_PLATFORM := $(BUILD)/plat/dev/footprint.txt
ENG_G3_FOOTPRINT := $(BUILD)/plat/dev/footprint.elf
ENG_G3_RUNTIME := $(BUILD)/plat/dev/rt.o $(BUILD)/plat/dev/text.o

sizegate: $(ENG_ARM_OUT)/engine.elf $(ENG_G3_LOADER) $(ENG_G3_RESIDENT) $(ENG_G3_PLATFORM) \
		$(ENG_G3_FOOTPRINT) $(ENG_G3_RUNTIME)
	$(ENG_PY) tools/gates/sizegate.py $(ENG_ARM_OUT)/engine.map --platform $(ENG_G3_PLATFORM) \
		$(addprefix --loader ,$(ENG_G3_LOADER)) $(addprefix --resident ,$(ENG_G3_RESIDENT)) \
		--footprint $(ENG_G3_FOOTPRINT) $(addprefix --runtime ,$(ENG_G3_RUNTIME))

-include $(wildcard $(ENG_ARM_OUT)/*.d)

# ---------------------------------------------------------------- lockstep

# Binaries with LS_SKIP_RESTING_CORPSES=1 in the hash (hash.h), same objects
# otherwise.
ENG_SKIP := -DLS_SKIP_RESTING_CORPSES=1
$(ORA_OUT)/dg_oracle_skip: $(filter-out %/own/hash.o,$(ORA_OBJS)) $(ORA_OUT)/own/hash_skip.o
	$(ENG_CC) $(ORA_WRAP) -o $@ $^ -lm
$(ORA_OUT)/own/hash_skip.o: tests/lockstep/hash.c | $(ORA_OUT)/own $(ORA_STAMP)
	$(ENG_CC) $(ORA_CFLAGS) $(ENG_SKIP) -c $< -o $@
$(BUILD)/host/dmcdoom_skip: $(filter-out %/hash.o,$(call ENG_HOST_OBJS,$(BUILD)/host/opt)) \
		$(BUILD)/host/opt/hash_skip.o
	$(ENG_CC) $(ENG_HOST_WRAP) -o $@ $^ -lm
$(BUILD)/host/opt/hash_skip.o: tests/lockstep/hash.c $(CONV_GEN_FILES) | $(BUILD)/host/opt
	$(ENG_CC) $(ENG_HOST_CFLAGS) $(ENG_SKIP) -O2 -c $< -o $@

# FREEDOOM_ZIP may point at a local copy of the Freedoom 0.13.0 zip. The
# unmasked hash holds with every thing a full object (--promote-all), the
# corpses at rest of the normal mode are masked in lockstep-skip.
lockstep: oracle host nor-images
	$(ENG_PY) tests/lockstep/run_lockstep.py --oracle $(ORA_OUT)/dg_oracle \
		--engine $(BUILD)/host/dmcdoom --asan --engine-arg=--promote-all

lockstep-skip: $(ORA_OUT)/dg_oracle_skip $(BUILD)/host/dmcdoom_skip nor-images
	$(ENG_PY) tests/lockstep/run_lockstep.py --oracle $(ORA_OUT)/dg_oracle_skip \
		--engine $(BUILD)/host/dmcdoom_skip --tag skip

# Frame model of the meter: one array is the frame region of the renderer, the tic scratch at
# its offset on the meter (6,912) and the window the overlays load into, every overlay load
# overwrites that window as the copy of the meter does (port/ovl/ovl.c). The object store runs
# as on the meter, with the masked hash of lockstep-skip, at the strip widths of the plans
# (wider host strips reach the tic scratch). --verify-model draws nothing and keeps the
# creation order under the tic scratch, as the verification pass of the meter. Linked at a
# fixed address: the linker defines the two symbols as absolute addresses.
ENG_FRAME_DIR := $(BUILD)/host/frame
ENG_FRAME_WRAP := $(ENG_HOST_WRAP) -Wl,--wrap=R_BandFlush -no-pie \
	-Wl,--defsym=dmc_tic=frame_mem+6912,--defsym=ovl_host_frame=frame_mem
$(eval $(call ENG_HOST_RULES,$(ENG_FRAME_DIR),-O2 -DDMC_HOST_FRAME $(ENG_SKIP)))
$(BUILD)/host/dmcdoom-frame: $(call ENG_HOST_OBJS,$(ENG_FRAME_DIR))
	$(ENG_CC) $(ENG_FRAME_WRAP) -o $@ $^ -lm

.PHONY: frame-model-test
frame-model-test: $(ORA_OUT)/dg_oracle_skip oracle $(BUILD)/host/dmcdoom-frame nor-images \
		$(BUILD)/host/dmcdoom_skip
	$(ENG_PY) tests/lockstep/run_lockstep.py --oracle $(ORA_OUT)/dg_oracle_skip \
		--engine $(BUILD)/host/dmcdoom-frame --tag frame --workload exits
	$(ENG_PY) tests/lockstep/run_lockstep.py --oracle $(ORA_OUT)/dg_oracle_skip \
		--engine $(BUILD)/host/dmcdoom-frame --tag frame --workload episode
	$(ENG_PY) tests/lockstep/run_lockstep.py --oracle $(ORA_OUT)/dg_oracle_skip \
		--engine $(BUILD)/host/dmcdoom-frame --tag frame --workload menu
	$(ENG_PY) tests/lockstep/run_lockstep.py --oracle $(ORA_OUT)/dg_oracle_skip \
		--engine $(BUILD)/host/dmcdoom-frame --tag frame-verify --workload exitlist \
		--engine-arg=--verify-model
	$(ENG_PY) tests/host/screentest.py --oracle $(ORA_OUT)/dg_oracle \
		--engine $(BUILD)/host/dmcdoom-frame --tag frame
	$(ENG_PY) tests/host/savetest.py --engine $(BUILD)/host/dmcdoom-frame \
		--ref $(BUILD)/host/dmcdoom_skip --case e1m1-s4 --case e1m6-s3 --tag frame
	$(ENG_PY) tests/host/savetest.py --engine $(BUILD)/host/dmcdoom-frame \
		--ref $(BUILD)/host/dmcdoom_skip --case e1m1-s4 --case e1m6-s3 --tag frame-verify \
		--engine-arg=--verify-model

# ---------------------------------------------------------------- data layer

# NOR images of the meter built from the packs (tests/host/mknor.py): plain
# images for the engine and fragmented ones for the data layer test.
DATA_SRC := $(wildcard port/data/*.c)
DATA_TEST_CFLAGS := -std=c99 -O1 -g -Wall -Wextra -fwrapv -fsigned-char -fno-omit-frame-pointer \
	-fsanitize=address,undefined -fno-sanitize-recover=all \
	-Iport/data -Iplatform -I$(GEN) -Ihost -MMD -MP
DATA_TEST_OUT := $(BUILD)/host/datatest
NOR_OUT := $(BUILD)/nor
MKNOR := $(ENG_PY) -I tests/host/mknor.py

$(DATA_TEST_OUT)/%.o: port/data/%.c $(CONV_GEN_FILES) | $(DATA_TEST_OUT)
	$(ENG_CC) $(DATA_TEST_CFLAGS) -c $< -o $@
$(DATA_TEST_OUT)/%.o: host/%.c | $(DATA_TEST_OUT)
	$(ENG_CC) $(DATA_TEST_CFLAGS) -c $< -o $@
$(DATA_TEST_OUT)/%.o: tests/host/%.c $(CONV_GEN_FILES) | $(DATA_TEST_OUT)
	$(ENG_CC) $(DATA_TEST_CFLAGS) -c $< -o $@
$(DATA_TEST_OUT):
	mkdir -p $@

$(BUILD)/host/test_data: $(patsubst port/data/%.c,$(DATA_TEST_OUT)/%.o,$(DATA_SRC)) \
		$(DATA_TEST_OUT)/nor_file.o $(DATA_TEST_OUT)/test_data.o
	$(ENG_CC) -fsanitize=address,undefined -o $@ $^

# The packs target is phony and cheap when nothing changed, so the images
# are rebuilt every time (mknor takes a few milliseconds).
.PHONY: nor-images data-test
NOR_IMAGES := $(NOR_OUT)/E1M1.img $(NOR_OUT)/E1.img
nor-images: packs | $(NOR_OUT)
	$(MKNOR) $(BUILD)/packs/E1M1 $(NOR_OUT)/E1M1.img
	$(MKNOR) $(BUILD)/packs/E1 $(NOR_OUT)/E1.img
$(NOR_OUT):
	mkdir -p $@

# Image layouts: plain, LFN/deleted/directory entries, 16 extents (the
# DOOM.DAT limit), 17 extents (rejected), DOOMX.DAT in 8 runs (its limit) and
# 9 runs (rejected), E1 without its DOOMX.DAT, with a DOOMX.DAT of another
# image or of the wrong length (rejected), a DOOM.DAT chain that ends early
# or loops (rejected), 512 B sectors with 2 per cluster (multi-sector FATs),
# DOOMCODE.DMC on the firmware drive, on the data drive and on both (the
# firmware drive wins). Then DEMO1 on the fragmented E1 image, with async
# reads completing at once and deferred, must give the hash stream of the
# plain image.
TEST_DATA := $(BUILD)/host/test_data
data-test: $(TEST_DATA) $(BUILD)/host/dmcdoom nor-images
	$(MKNOR) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_e1m1_frag16.img --junk --frag 16
	$(MKNOR) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_e1m1_frag17.img --frag 17
	$(MKNOR) $(BUILD)/packs/E1 $(NOR_OUT)/t_e1_frag.img --junk --frag 8 --xfrag 8
	$(MKNOR) $(BUILD)/packs/E1 $(NOR_OUT)/t_e1_xfrag9.img --xfrag 9
	mkdir -p $(NOR_OUT)/nodoomx && cp $(BUILD)/packs/E1/DOOM.DAT $(NOR_OUT)/nodoomx/
	$(MKNOR) $(NOR_OUT)/nodoomx $(NOR_OUT)/t_e1_nodoomx.img
	$(MKNOR) $(BUILD)/packs/E1 $(NOR_OUT)/t_e1_xuuid.img --doomx-bad uuid
	$(MKNOR) $(BUILD)/packs/E1 $(NOR_OUT)/t_e1_xlen.img --doomx-bad length
	$(MKNOR) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_e1m1_chain0.img --chain-bad zero
	$(MKNOR) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_e1m1_chainloop.img --chain-bad loop
	$(MKNOR) $(BUILD)/packs/E1 $(NOR_OUT)/t_e1_s512.img --sector 512 --spc 2 --frag 6 --xfrag 3
	$(ENG_PY) -c "import sys; open(sys.argv[1], 'wb').write(bytes(range(256)) * 36)" $(NOR_OUT)/t_code_upd.bin
	$(ENG_PY) -c "import sys; open(sys.argv[1], 'wb').write(bytes(range(255, -1, -1)) * 21)" $(NOR_OUT)/t_code_main.bin
	$(MKNOR) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_code_upd.img --code $(NOR_OUT)/t_code_upd.bin
	$(MKNOR) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_code_main.img --code-main $(NOR_OUT)/t_code_main.bin
	$(MKNOR) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_code_both.img --code $(NOR_OUT)/t_code_upd.bin \
		--code-main $(NOR_OUT)/t_code_main.bin
	$(TEST_DATA) $(BUILD)/packs/E1M1 $(NOR_OUT)/E1M1.img
	$(TEST_DATA) $(BUILD)/packs/E1 $(NOR_OUT)/E1.img
	$(TEST_DATA) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_e1m1_frag16.img
	$(TEST_DATA) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_e1m1_frag17.img -3
	$(TEST_DATA) $(BUILD)/packs/E1 $(NOR_OUT)/t_e1_frag.img
	$(TEST_DATA) $(BUILD)/packs/E1 $(NOR_OUT)/t_e1_xfrag9.img -3
	$(TEST_DATA) $(BUILD)/packs/E1 $(NOR_OUT)/t_e1_nodoomx.img -6
	$(TEST_DATA) $(BUILD)/packs/E1 $(NOR_OUT)/t_e1_xuuid.img -6
	$(TEST_DATA) $(BUILD)/packs/E1 $(NOR_OUT)/t_e1_xlen.img -6
	$(TEST_DATA) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_e1m1_chain0.img -1
	$(TEST_DATA) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_e1m1_chainloop.img -1
	$(TEST_DATA) $(BUILD)/packs/E1 $(NOR_OUT)/t_e1_s512.img
	$(TEST_DATA) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_code_upd.img 0 $(NOR_OUT)/t_code_upd.bin
	$(TEST_DATA) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_code_main.img 0 $(NOR_OUT)/t_code_main.bin
	$(TEST_DATA) $(BUILD)/packs/E1M1 $(NOR_OUT)/t_code_both.img 0 $(NOR_OUT)/t_code_upd.bin
	$(BUILD)/host/dmcdoom --nor $(NOR_OUT)/E1.img -playdemo demo1 --hash-out $(NOR_OUT)/t_plain.txt > /dev/null
	$(BUILD)/host/dmcdoom --nor $(NOR_OUT)/t_e1_frag.img -playdemo demo1 --hash-out $(NOR_OUT)/t_frag.txt > /dev/null
	$(BUILD)/host/dmcdoom --nor $(NOR_OUT)/t_e1_frag.img --nor-defer -playdemo demo1 \
		--hash-out $(NOR_OUT)/t_defer.txt > /dev/null
	cmp $(NOR_OUT)/t_plain.txt $(NOR_OUT)/t_frag.txt
	cmp $(NOR_OUT)/t_plain.txt $(NOR_OUT)/t_defer.txt
	@echo "DEMO1 on the fragmented E1 image, async reads at once and deferred: same hash stream"
	rm -rf $(NOR_OUT)/t_*.img $(NOR_OUT)/t_*.txt $(NOR_OUT)/t_*.bin $(NOR_OUT)/nodoomx

-include $(wildcard $(DATA_TEST_OUT)/*.d)
