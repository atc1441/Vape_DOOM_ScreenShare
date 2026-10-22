# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
#
# Build rules of the firmware: DOOM.BIN, the firmware of the meter, its overlays
# in DOOMCODE.DMC, its Renode variant and the device package.
#
#   make -j$(nproc) fw              both variants with their DOOMCODE.DMC, flash and RAM report
#                                   of the meter image, DEMO2.LMP (E1M1 script)
#   make -j$(nproc) fw-size         the flash and RAM report alone, also when the meter image
#                                   does not link (over a budget)
#   make -j$(nproc) fw-test         host checks: tool self-tests, the firmware's hash against
#                                   lockstep-skip, DEMO2.LMP on the oracle
#   make -j$(nproc) fw-renode-test  the Renode variant in Renode (RENODE, mk/platform.mk),
#                                   FW_RENODE_ONLY=NAME one run of tests/renode/test_fw.py
#                                   (the multimap run only runs this way)
#   make -j$(nproc) fw-package      build/device/: DOOM.BIN, DOOMCODE.DMC, DOOMRES.DAT,
#                                   DEMO2.LMP, DOOM.DAT and DOOMX.DAT of the E1 pack
#
# Link variants, both from the same objects and link script, in the app slot (101,376 B) and
# 32 KB of SRAM, except the platform's NOR and LCD drivers:
#   meter   DMA NOR and LCD pump: DOOM.BIN. The link fails while the image or the fixed core
#           is over its budget. fw-size links the same image without stopping at those
#           limits and reports flash and RAM by module from the map (tools/fwsize.py).
#   renode  polled NOR and LCD (the Renode models drop SPI receive DMA inside a transmit DMA
#           and send 16-bit frames as one byte, tests/renode/dmc100.repl): DOOMREN.BIN
# Each variant has its own build ID, its DOOMCODE.DMC only loads into that variant.
#
# Two-stage link of the overlays (mk/ovl.mk), each linked at FRAME_BASE
# against the stamped resident image of its variant. An overlay may share objects with the
# resident image: tools/ovl_resident.py places every input section of them that main.elf has
# at its address there, so the overlay keeps only what the resident link dropped, the cold
# code reached from its entries alone and the entries themselves, which ld.lld --wrap turns
# into overlay calls in the resident image (port/game/i_dmc.c).
#   LEVEL   level setup (engine/p_setup.c, port/obj/level.c) with the status bar, HUD and
#           sound setup it runs, and the level transitions of g_game.c (port/game/l_level.c)
#   WI      the intermission (engine/wi_stuff.c, port/game/l_wi.c) and the end of a level
#           (G_DoCompleted, G_WorldDone of g_game.c)
#   FINALE  the finale and the pages of the title loop (engine/f_finale.c, port/game/l_finale.c)
#   BOOT    port/game/dmc_boot.c, port/data/img_check.c, the settings page of the platform,
#           the engine's start-up code (S_Init, HU_Init, ST_Init and what only they run) and
#           the FAT writes that make SAVE.DAT (port/data/fat_write.c)
#   REPORT  port/game/dmc_report.c
#   HASH    port/game/dmc_hash.c and tests/lockstep/hash.c (LS_SMALL)
#   LINES   the line specials of the engine (port/game/l_lines.c): P_CrossSpecialLine,
#           P_ShootSpecialLine, P_UseSpecialLine and the EV_* code only they run
#   MENU    the menu (engine/m_menu.c, port/game/l_menu.c) with the settings page writer of
#           the platform and the slot headers of SAVE.DAT (port/game/g_save.c)
#   USB     the 3 MB data volume as a USB drive: port/game/dmc_usb.c, port/usb, the FAT
#           writes and the controller driver of the platform (platform/usbd.c, resident for
#           the game pad driver controller/rc-n1/dmc_dji.c, so the overlay calls the resident
#           copy)
#   SAVEG   the saves of SAVE.DAT (port/game/g_save.c, port/game/l_save.c)
# MENU, WI and FINALE draw their screens with port/render/r_screen.c, which each of them links.
# Resident objects whose code only some overlays reach are linked into them too, so that code
# leaves the resident image (room for the USB controller driver of the game pad): fetch.c,
# i_dmc.c and dmc_demo.c into LEVEL, rec.c, fetch.c and dmc_demo.c into REPORT, things.c
# (O_ForEach) into HASH.
# The resident link keeps what the overlays' reachable code uses outside their objects
# (exports.ld, ovl_resident.py exports) and tools/pack_overlays.py checks each overlay.

FW_CC      ?= clang
FW_LD      ?= ld.lld
FW_NM      ?= llvm-nm
FW_OBJCOPY ?= llvm-objcopy
FW_SIZE    ?= llvm-size
FW_PY      ?= python3

FW_B := $(BUILD)/fwdoom
FW_O := $(FW_B)/obj
FW_ARCH := --target=thumbv7em-none-eabi -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
# Compact sine table (port/math/trig.c): it saves 3.3 KB, without it the image does not fit
# the code area. It is exact like the default decoder, ENG_SINE_L1=1 runs the host tests with
# it.
FW_DEFS := -DDMC_FW -DDMC_HASHORDER -DDMC_SINE_L1
# Engine, data layer, renderer and object store as the ARM size build compiles them
FW_ECFLAGS := $(FW_ARCH) -Oz -ffunction-sections -fdata-sections -ffreestanding -fno-common \
	-fwrapv -fsigned-char -std=gnu11 -w $(FW_DEFS) -Iengine/stub/include -Iengine -Iport/data \
	-Iport/render -Iport/obj -Iport/math -Iplatform -Iport/game -Iport/ovl -I$(GEN) -MMD -MP
# Firmware glue: our warning set, upstream headers as system headers
FW_GCFLAGS := $(FW_ARCH) -Oz -g -std=gnu11 -ffunction-sections -fdata-sections -ffreestanding \
	-fno-common -fwrapv -fsigned-char -Wall -Wextra -Wshadow -Wstrict-prototypes $(FW_DEFS) \
	-isystem engine/stub/include -isystem engine -Iport/game -Iport/data -Iport/render \
	-Iport/obj -Iport/ovl -Iport/usb -Iplatform -Iplatform/apps -Itests/lockstep -I$(GEN) \
	-Icontroller/rc-n1 -MMD -MP
FW_HASH_DEFS := -DLS_SMALL -DLS_NOFILE -DLS_SKIP_RESTING_CORPSES=1

# Resident engine: the front end's glue is port/game/fe_glue.c (the automap is a stub there),
# port/game/i_dmc.c replaces the zone allocator. The sources only overlays link are left out.
FW_ENG_OUT := engine/am_map.c engine/z_zone.c engine/m_argv.c
FW_OVL_ONLY := engine/p_setup.c port/obj/level.c port/game/l_level.c port/game/dmc_boot.c \
	port/data/img_check.c port/game/dmc_report.c port/game/dmc_hash.c port/game/l_lines.c \
	engine/wi_stuff.c port/game/l_wi.c engine/f_finale.c port/game/l_finale.c \
	engine/m_menu.c port/game/l_menu.c port/render/r_screen.c port/data/fat_write.c \
	port/game/g_save.c port/game/l_save.c
FW_ENG_SRC := $(filter-out $(FW_ENG_OUT) $(FW_OVL_ONLY),$(ENG_SRC) $(wildcard port/data/*.c) \
	$(wildcard port/render/*.c) port/math/trig.c $(wildcard port/obj/*.c))
FW_GAME_SRC := port/game/dmc_main.c port/game/dmc_demo.c port/game/dmc_res.c port/game/i_dmc.c \
	port/game/libc_dmc.c port/game/fe_glue.c port/game/fe_play.c port/game/i_input.c \
	port/game/s_pcspk.c controller/rc-n1/dmc_dji.c port/ovl/ovl.c port/ovl/ovl_file.c platform/apps/ui.c
FW_PLAT := startup plat_at32 systick buttons buzzer fault nor lcd pump text font8x8 rt usbd

FW_RES_OBJS := $(patsubst %.c,$(FW_O)/%.o,$(FW_ENG_SRC) $(FW_GAME_SRC)) \
	$(patsubst $(GEN)/%.c,$(FW_O)/gen/%.o,$(ENG_GEN_SRC))
# Platform objects of mk/platform.mk: dev is the meter's DMA build, renode the polled one
FW_VARIANTS := meter renode
FW_PLAT_meter := $(FW_PLAT:%=$(BUILD)/plat/dev/%.o)
FW_PLAT_renode := $(FW_PLAT:%=$(BUILD)/plat/renode/%.o)
FW_PDIR_meter := dev
FW_PDIR_renode := renode
FW_BIN_meter := DOOM.BIN
FW_BIN_renode := DOOMREN.BIN
# Overlays: their sources, the objects the resident image links too included, then platform
# objects of the variant
FW_OVLS := LEVEL BOOT REPORT HASH LINES USB WI FINALE MENU SAVEG
FW_SRC_LEVEL := port/game/l_level.c engine/p_setup.c port/obj/level.c \
	$(addprefix engine/,g_game.c st_stuff.c st_lib.c hu_stuff.c hu_lib.c s_sound.c d_main.c) \
	port/data/rec.c port/game/libc_dmc.c port/data/fetch.c port/game/i_dmc.c port/game/dmc_demo.c
FW_SRC_BOOT := port/game/dmc_boot.c port/data/img_check.c engine/s_sound.c engine/hu_stuff.c \
	engine/st_stuff.c engine/d_loop.c engine/v_video.c port/game/i_input.c port/game/s_pcspk.c \
	port/game/i_dmc.c port/data/fat_write.c
FW_SRC_REPORT := port/game/dmc_report.c port/data/rec.c port/data/fetch.c port/game/dmc_demo.c
FW_SRC_HASH := port/game/dmc_hash.c tests/lockstep/hash.c port/obj/things.c
FW_SRC_LINES := port/game/l_lines.c $(addprefix engine/,p_spec.c p_switch.c p_doors.c p_plats.c \
	p_ceilng.c p_lights.c p_telept.c p_floor.c)
FW_SRC_USB := port/game/dmc_usb.c port/usb/usb_ctrl.c port/usb/usb_msc.c port/usb/usb_vol.c \
	port/data/fat_write.c
FW_SRC_WI := port/game/l_wi.c engine/wi_stuff.c port/render/r_screen.c engine/g_game.c
FW_SRC_FINALE := port/game/l_finale.c engine/f_finale.c port/render/r_screen.c port/data/fetch.c
FW_SRC_MENU := port/game/l_menu.c engine/m_menu.c port/render/r_screen.c port/game/i_dmc.c \
	port/data/fetch.c port/game/g_save.c engine/g_game.c
FW_SRC_SAVEG := port/game/l_save.c port/game/g_save.c
FW_PLAT_BOOT := settings
FW_PLAT_MENU := settings
FW_PLAT_USB := usbd
FW_OVL_OBJS = $(patsubst %.c,$(FW_O)/%.o,$(FW_SRC_$(2))) $(FW_PLAT_$(2):%=$(BUILD)/plat/$(FW_PDIR_$(1))/%.o)

# Overlay entries, the timed demo's probes and the forced exits (port/game/dmc_demo.c)
FW_WRAP := -Wl,--wrap=G_DoLoadLevel,--wrap=G_DoNewGame,--wrap=G_DoPlayDemo,--wrap=G_DoCompleted \
	-Wl,--wrap=G_DoWorldDone,--wrap=P_Init,--wrap=D_PostEvent,--wrap=G_Ticker \
	-Wl,--wrap=P_CrossSpecialLine,--wrap=P_ShootSpecialLine,--wrap=P_UseSpecialLine \
	-Wl,--wrap=R_RenderPlayerView,--wrap=R_RenderBSPPass,--wrap=R_BandFlush \
	-Wl,--wrap=pump_sync,--wrap=pump_submit,--wrap=pump_end
FW_LDS := platform/link/doom.ld platform/link/regions.ld
# A 1,536 B stack instead of the default 2 KB, the other 512 B go to the fixed core
# (platform/link/regions.ld). The deepest stack use measured in Renode is 1,120 B, also over
# 400 drawn frames per demo (without the DMA interrupts of the meter), and an overflow faults
# below SRAM.
FW_STACK := -Wl,--defsym=STACK_SIZE_OVERRIDE=0x600
FW_LDFLAGS_meter := -Wl,-L,platform/link $(FW_STACK) -Wl,--defsym=__dmc_variant=1
FW_LDFLAGS_renode := -Wl,-L,platform/link $(FW_STACK) -Wl,--defsym=__dmc_variant=2
FW_PACK := $(FW_PY) -I tools/pack_overlays.py
FW_NO_UNDEF = @u="$$($(FW_NM) -u $@)"; if [ -n "$$u" ]; then echo "$@: undefined symbols:"; \
	echo "$$u"; rm -f $@; exit 1; fi

# ---- objects

$(FW_O)/%.o: %.c mk/fw.mk $(CONV_GEN_FILES)
	@mkdir -p $(dir $@)
	$(FW_CC) $(FW_ECFLAGS) $(FW_OPT) -c $< -o $@
$(FW_O)/port/game/%.o $(FW_O)/port/ovl/%.o $(FW_O)/port/usb/%.o $(FW_O)/platform/apps/%.o \
	$(FW_O)/controller/%.o: FW_ECFLAGS := $(FW_GCFLAGS)
# The drawer inner loops are built -O2 (mk/render.mk)
$(FW_O)/port/render/r_inner.o: FW_OPT := -O2 -fno-unroll-loops
$(FW_O)/gen/%.o: $(GEN)/%.c mk/fw.mk $(CONV_GEN_FILES)
	@mkdir -p $(dir $@)
	$(FW_CC) $(filter-out -I%,$(FW_ECFLAGS)) -I$(GEN) -c $< -o $@
$(FW_O)/tests/lockstep/hash.o: tests/lockstep/hash.c mk/fw.mk $(CONV_GEN_FILES)
	@mkdir -p $(dir $@)
	$(FW_CC) $(FW_ECFLAGS) -Itests/lockstep $(FW_HASH_DEFS) -c $< -o $@

# ---- the resident roots of the overlays: $(1) variant

define FW_EXPORTS
$(FW_B)/$(1)/exports.ld: $(foreach o,$(FW_OVLS),$(call FW_OVL_OBJS,$(1),$(o))) tools/ovl_resident.py \
		tools/pack_overlays.py
	@mkdir -p $$(dir $$@)
	@echo "ovl_resident.py exports $$@"
	@$(FW_PY) -I tools/ovl_resident.py exports -o $$@ \
		$(foreach o,$(FW_OVLS),--ovl "$(call FW_OVL_OBJS,$(1),$(o))")
endef
$(foreach v,$(FW_VARIANTS),$(eval $(call FW_EXPORTS,$(v))))

# ---- overlay links: $(1) variant, $(2) overlay

define FW_OVL
$(FW_B)/$(1)/$(2).ld: $(FW_B)/$(1)/main.unstamped.elf tools/ovl_resident.py port/ovl/ovl.ld
	@echo "ovl_resident.py script $$@"
	@$(FW_PY) -I tools/ovl_resident.py script --map $(FW_B)/$(1)/main.unstamped.map \
		--script port/ovl/ovl.ld -o $$@ $(call FW_OVL_OBJS,$(1),$(2))
$(FW_B)/$(1)/$(2).elf: $(FW_B)/$(1)/$(2).ld $(call FW_OVL_OBJS,$(1),$(2)) $(FW_B)/$(1)/main.elf
	$(FW_LD) -T $(FW_B)/$(1)/$(2).ld --gc-sections --emit-relocs --just-symbols=$(FW_B)/$(1)/main.elf \
		--allow-multiple-definition -Map $$(@:.elf=.map) $(call FW_OVL_OBJS,$(1),$(2)) -o $$@
	$$(FW_NO_UNDEF)
endef
$(foreach v,$(FW_VARIANTS),$(foreach o,$(FW_OVLS),$(eval $(call FW_OVL,$(v),$(o)))))

# ---- links: $(1) variant

define FW_LINK
$(FW_B)/$(1)/main.unstamped.elf: $(FW_RES_OBJS) $(FW_PLAT_$(1)) $(FW_B)/$(1)/exports.ld $(FW_LDS)
	@mkdir -p $$(dir $$@)
	$(FW_CC) $(FW_ARCH) -nostdlib -fuse-ld=lld $(FW_LDFLAGS_$(1)) -Wl,-T,platform/link/doom.ld \
		-Wl,--gc-sections -Wl,--emit-relocs -Wl,-Map,$$(@:.elf=.map) $(FW_WRAP) \
		$$(filter %.o,$$^) -Wl,$(FW_B)/$(1)/exports.ld -o $$@
	$$(FW_NO_UNDEF)
$(FW_B)/$(1)/main.elf: $(FW_B)/$(1)/main.unstamped.elf tools/pack_overlays.py
	$(FW_PACK) stamp $$< $$@
$(FW_B)/$(1)/DOOMCODE.DMC: $(FW_B)/$(1)/main.elf $(FW_OVLS:%=$(FW_B)/$(1)/%.elf) tools/pack_overlays.py
	$(FW_PACK) pack --report --main $$< -o $$@ --forbid R_DrawFrame --forbid R_RenderPlayerView \
		$(FW_OVLS:%=$(FW_B)/$(1)/%.elf)
# The app slot only (the settings page and anything after it are not part of DOOM.BIN)
$(FW_B)/$(1)/$(FW_BIN_$(1)): $(FW_B)/$(1)/main.elf
	$(FW_OBJCOPY) -O binary -j .text -j .data $$< $$@
endef
$(foreach v,$(FW_VARIANTS),$(eval $(call FW_LINK,$(v))))

# Flash and RAM report of the meter's image: the meter link without
# stopping at the budgets, its map read by tools/fwsize.py. size.txt adds the overlays of the
# meter build, fw-size reports without them.
$(FW_B)/meter/probe.map: $(FW_RES_OBJS) $(FW_PLAT_meter) $(FW_B)/meter/exports.ld $(FW_LDS)
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(FW_CC) $(FW_ARCH) -nostdlib -fuse-ld=lld $(FW_LDFLAGS_meter) -Wl,-T,platform/link/doom.ld \
		-Wl,--gc-sections -Wl,--noinhibit-exec -Wl,-Map,$@ $(FW_WRAP) \
		$(filter %.o,$^) -Wl,$(FW_B)/meter/exports.ld -o $(FW_B)/meter/probe.elf > /dev/null 2>&1 || true
	@rm -f $(FW_B)/meter/probe.elf
$(FW_B)/meter/size.txt: $(FW_B)/meter/probe.map $(FW_B)/meter/DOOMCODE.DMC tools/fwsize.py
	$(FW_PY) -I tools/fwsize.py --map $< --code $(FW_B)/meter/DOOMCODE.DMC \
		$(FW_OVLS:%=--ovl $(FW_B)/meter/%.elf) | tee $@

# ---- the E1M1 script as a vanilla demo (DEMO2.LMP)

FW_LMP := $(FW_B)/DEMO2.LMP
$(FW_LMP): tools/mklmp.py tests/lockstep/run_lockstep.py
	@mkdir -p $(dir $@)
	$(FW_PY) -I tools/mklmp.py -o $@

.PHONY: fw fw-size fw-test fw-renode-test fw-package
all: fw
test: fw-test

FW_OUT := $(foreach v,$(FW_VARIANTS),$(FW_B)/$(v)/$(FW_BIN_$(v)) $(FW_B)/$(v)/DOOMCODE.DMC) \
	$(FW_B)/meter/size.txt $(FW_LMP)
fw: $(FW_OUT)

fw-size: $(FW_B)/meter/probe.map tools/fwsize.py
	$(FW_PY) -I tools/fwsize.py --map $<

# The hash of the firmware (tests/lockstep/hash.c with LS_SMALL: no list, a scratch of 2 B
# per handle slot) in the host engine: its stream must equal the masked hash of lockstep-skip
# on DEMO1 and on the E1M1 script demo.
$(FW_B)/host/hash_small.o: tests/lockstep/hash.c $(CONV_GEN_FILES) mk/fw.mk
	@mkdir -p $(dir $@)
	$(ENG_CC) $(ENG_HOST_CFLAGS) $(ENG_SKIP) -DLS_SMALL -O2 -c $< -o $@
$(FW_B)/host/dmcdoom_small: $(filter-out %/hash.o,$(call ENG_HOST_OBJS,$(BUILD)/host/opt)) \
		$(FW_B)/host/hash_small.o
	$(ENG_CC) $(ENG_HOST_WRAP) -o $@ $^ -lm

# The oracle plays DEMO2.LMP as a vanilla demo too (it wants the extension in lower case): its
# unmasked hash stream must equal the engine's with every thing a full object (--promote-all,
# corpses at rest are masked only in lockstep-skip). It reads $(IWAD) when given.
FW_HB := $(abspath $(FW_B)/host)
FW_NOR := $(abspath $(NOR_OUT)/E1.img)
fw-test: $(FW_OUT) $(FW_B)/host/dmcdoom_small $(BUILD)/host/dmcdoom_skip $(BUILD)/host/dmcdoom oracle \
		nor-images
	$(FW_PY) -I tests/renode/fwnor.py --self-test --build $(FW_B)
	$(FW_PY) -I tests/device/doomres.py --self-test
	@set -e; cd '$(FW_HB)'; for d in demo1 '$(abspath $(FW_LMP))'; do \
		'$(abspath $(BUILD)/host/dmcdoom_skip)' --nor '$(FW_NOR)' -playdemo "$$d" --hash-out skip.txt > /dev/null; \
		./dmcdoom_small --nor '$(FW_NOR)' -playdemo "$$d" --hash-out small.txt > /dev/null; \
		cmp skip.txt small.txt; echo "$$(basename "$$d"): firmware hash equal to lockstep-skip, $$(wc -l < small.txt) tics"; \
		done; rm -f skip.txt small.txt
	@set -e; wad=$(if $(IWAD),'$(abspath $(IWAD))',$$($(FW_PY) -c "import sys; \
		sys.path.insert(0, 'tests/lockstep'); sys.dont_write_bytecode = True; \
		import run_lockstep as L; print(L.get_wad(None))")); \
		cd '$(FW_HB)'; cp '$(abspath $(FW_LMP))' e1m1script.lmp; \
		'$(abspath $(ORA_OUT)/dg_oracle)' -iwad "$$wad" -playdemo e1m1script.lmp --hash-out oracle.txt > /dev/null; \
		'$(abspath $(BUILD)/host/dmcdoom)' --nor '$(FW_NOR)' --promote-all -playdemo e1m1script.lmp \
			--hash-out engine.txt > /dev/null; \
		cmp oracle.txt engine.txt; echo "DEMO2.LMP: engine and oracle in lockstep, $$(wc -l < engine.txt) tics"; \
		rm -f e1m1script.lmp oracle.txt engine.txt

fw-renode-test: $(FW_OUT) $(BUILD)/plat/renode/blstub.bin $(BUILD)/host/dmcdoom \
		$(BUILD)/host/dmcdoom_skip nor-images oracle
	RENODE=$(RENODE) $(FW_PY) -I tests/renode/test_fw.py --build $(BUILD) \
		$(if $(FW_RENODE_ONLY),--only $(FW_RENODE_ONLY))

# Device package: the meter build (needs the compact tables and a fixed core within budget).
# A pack that fits in DOOM.DAT has no DOOMX.DAT.
FW_DEV_PKG := $(BUILD)/device
fw-package: $(FW_B)/meter/DOOM.BIN $(FW_B)/meter/DOOMCODE.DMC $(FW_LMP) packs
	@mkdir -p $(FW_DEV_PKG)
	cp $(FW_B)/meter/DOOM.BIN $(FW_B)/meter/DOOMCODE.DMC $(FW_LMP) $(FW_DEV_PKG)/
	$(FW_PY) -I tests/device/doomres.py --blank $(FW_DEV_PKG)/DOOMRES.DAT
	ln -sf ../packs/E1/DOOM.DAT $(FW_DEV_PKG)/DOOM.DAT
	if [ -f $(BUILD)/packs/E1/DOOMX.DAT ]; then \
		ln -sf ../packs/E1/DOOMX.DAT $(FW_DEV_PKG)/DOOMX.DAT; \
	else rm -f $(FW_DEV_PKG)/DOOMX.DAT; fi
	$(FW_PY) -I tests/renode/fwnor.py --budget $(FW_DEV_PKG)

-include $(wildcard $(FW_O)/*/*.d $(FW_O)/*/*/*.d $(FW_O)/gen/*.d)
