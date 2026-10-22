# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
#
# Build rules of the overlays: two-stage overlay link, DOOMCODE.DMC (tools/pack_overlays.py)
# and the overlay demo of tests/ovl.
#
#   make -j$(nproc) ovl               demo firmware, overlays, packs, NOR image, host tests
#   make -j$(nproc) ovl-test          packer tests and the host loader tests
#   make -j$(nproc) ovl-renode-test   demo firmware in Renode (RENODE from mk/platform.mk)
#
# Two-stage link, as the demo rules below do it:
# 1. The objects of each overlay are linked into one relocatable file (ld.lld -r).
# 2. Their undefined symbols become EXTERN() roots of the resident link (exports.ld), so
#    --gc-sections keeps the resident code and data the overlays use.
# 3. main.elf is linked with platform/link/app.ld and --emit-relocs (the packer follows its
#    calls) and stamped with its build ID.
# 4. Each overlay is linked at FRAME_BASE: ld.lld $(OVL_LDFLAGS) --just-symbols=main.elf.
# 5. "pack_overlays.py pack" checks windows, references and the build and writes the file.
#    Resident functions that write the frame region are passed with --forbid FUNC.
# Overlay code reaches resident functions through ld.lld range-extension thunks (plain BL,
# one 12 B thunk per resident callee) unless it is compiled with $(OVL_CALLS) = -mlong-calls,
# which made am_map, m_menu, wi_stuff and f_finale 2.5-10 % (48-534 B) larger. The demo
# builds and runs both variants.

OVL_CC      ?= clang
OVL_HOSTCC  ?= clang
OVL_LD      ?= ld.lld
OVL_NM      ?= llvm-nm
OVL_OBJCOPY ?= llvm-objcopy
OVL_PY      ?= python3
OVL_CALLS   ?=

OVL_B := $(BUILD)/ovl
OVL_D := $(OVL_B)/demo
OVL_N := $(OVL_B)/neg
OVL_ARCH := --target=thumbv7em-none-eabi -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
OVL_WARN := -Wall -Wextra -Wshadow -Wstrict-prototypes
OVL_INC := -Iplatform -Iport/ovl -Iport/data -Itests/ovl
# The demo builds set OVL_DEMO (ids of the test overlays), the prod objects do not.
OVL_PCFLAGS := $(OVL_ARCH) -std=c99 -Oz -g -ffreestanding -fno-builtin -ffunction-sections \
	-fdata-sections -fwrapv -fsigned-char $(OVL_WARN) $(OVL_INC) -MMD -MP
OVL_CFLAGS := $(OVL_PCFLAGS) -DOVL_DEMO
OVL_HCFLAGS := -std=c99 -O1 -g -DPLAT_HOST -DOVL_DEMO -fwrapv -fsigned-char $(OVL_WARN) \
	-fsanitize=address,undefined -fno-sanitize-recover=all $(OVL_INC) -MMD -MP
OVL_LDFLAGS := -T port/ovl/ovl.ld --gc-sections --emit-relocs
OVL_LDS := platform/link/app.ld platform/link/regions.ld
OVL_MAIN_LDFLAGS := $(OVL_ARCH) -nostdlib -fuse-ld=lld -Wl,-L,platform/link \
	-Wl,-T,platform/link/app.ld -Wl,--gc-sections -Wl,--emit-relocs
OVL_PACK := $(OVL_PY) -I tools/pack_overlays.py
OVL_NO_UNDEF = @u="$$($(OVL_NM) -u $@)"; if [ -n "$$u" ]; then echo "$@: undefined symbols:"; \
	echo "$$u"; rm -f $@; exit 1; fi

# ---- Objects: $(OVL_B)/obj/<variant>/<source>.o ----
# res: resident code, ovl: overlays with $(OVL_CALLS), lc: overlays with -mlong-calls,
# plat: platform sources built like the Renode objects of mk/platform.mk (polled NOR and LCD),
# prod: the loader as the firmware builds it (no demo ids).

$(OVL_B)/obj/res/%.o: %.c mk/ovl.mk
	@mkdir -p $(dir $@)
	$(OVL_CC) $(OVL_CFLAGS) $(OVL_RES_OPT) -c $< -o $@
$(OVL_B)/obj/res/tests/ovl/demo_movt.o: OVL_RES_OPT := -O2
$(OVL_B)/obj/prod/%.o: %.c mk/ovl.mk
	@mkdir -p $(dir $@)
	$(OVL_CC) $(OVL_PCFLAGS) -c $< -o $@
$(OVL_B)/obj/ovl/%.o: %.c mk/ovl.mk
	@mkdir -p $(dir $@)
	$(OVL_CC) $(OVL_CFLAGS) $(OVL_CALLS) -c $< -o $@
$(OVL_B)/obj/lc/%.o: %.c mk/ovl.mk
	@mkdir -p $(dir $@)
	$(OVL_CC) $(OVL_CFLAGS) -mlong-calls -c $< -o $@
# The options stamp is PLAT_STAMP, mk/platform.mk is read after this file
$(OVL_B)/obj/plat/%.o: platform/%.c mk/ovl.mk $(BUILD)/plat/options.stamp
	@mkdir -p $(dir $@)
	$(OVL_CC) $(PLAT_CFLAGS) $(PLAT_DEFS_renode) $(OVL_PLAT_OPT) -c $< -o $@
$(OVL_B)/obj/plat/pump.o: OVL_PLAT_OPT := -O2

OVL_R := $(OVL_B)/obj/res
OVL_O := $(OVL_B)/obj/ovl
OVL_PLAT_OBJS := $(patsubst platform/%.c,$(OVL_B)/obj/plat/%.o,$(wildcard platform/*.c))
OVL_RES_OBJS := $(addprefix $(OVL_R)/,port/ovl/ovl.o port/ovl/ovl_fixed.o port/data/crc32.o \
	tests/ovl/demo_main.o tests/ovl/demo_seq.o tests/ovl/demo_movt.o)

# ---- Demo: resident image, overlays A and B in both call variants, packs ----

$(OVL_D)/a.r.o: $(OVL_O)/tests/ovl/ovl_a.o
$(OVL_D)/b.r.o: $(OVL_O)/tests/ovl/ovl_b.o
$(OVL_D)/a_lc.r.o: $(OVL_B)/obj/lc/tests/ovl/ovl_a.o
$(OVL_D)/b_lc.r.o: $(OVL_B)/obj/lc/tests/ovl/ovl_b.o
$(OVL_N)/neg_big.r.o: $(OVL_O)/tests/ovl/neg_big.o
$(OVL_N)/neg_huge.r.o: $(OVL_O)/tests/ovl/neg_huge.o
$(OVL_N)/neg_loader.r.o: $(OVL_O)/tests/ovl/neg_loader.o
$(OVL_N)/neg_cross.r.o: $(OVL_O)/tests/ovl/neg_cross.o
$(OVL_N)/neg_indirect.r.o: $(OVL_O)/tests/ovl/neg_indirect.o
$(OVL_N)/neg_state.r.o: $(OVL_O)/tests/ovl/neg_state.o
$(OVL_N)/neg_frame.r.o: $(OVL_O)/tests/ovl/neg_frame.o
$(OVL_N)/neg_movt.r.o: $(OVL_O)/tests/ovl/neg_movt.o
$(OVL_N)/ok_frame.r.o: $(OVL_O)/tests/ovl/ok_frame.o
$(OVL_B)/%.r.o:
	@mkdir -p $(dir $@)
	$(OVL_LD) -r -o $@ $^

OVL_PARTS := $(OVL_D)/a.r.o $(OVL_D)/b.r.o $(OVL_D)/a_lc.r.o $(OVL_D)/b_lc.r.o
$(OVL_D)/exports.ld: $(OVL_PARTS)
	@{ echo '/* Resident symbols used by overlays (generated) */'; echo 'EXTERN('; \
	  for f in $^; do $(OVL_NM) -u -j $$f; done | sort -u; echo ')'; } > $@

$(OVL_D)/main.unstamped.elf: $(OVL_RES_OBJS) $(OVL_PLAT_OBJS) $(OVL_D)/exports.ld $(OVL_LDS)
	$(OVL_CC) $(OVL_MAIN_LDFLAGS) -Wl,-Map,$(@:.elf=.map) $(filter %.o,$^) \
		-Wl,$(OVL_D)/exports.ld -o $@
	$(OVL_NO_UNDEF)
$(OVL_D)/main.elf: $(OVL_D)/main.unstamped.elf tools/pack_overlays.py
	$(OVL_PACK) stamp $< $@
$(OVL_D)/OVLDEMO.BIN: $(OVL_D)/main.elf
	$(OVL_OBJCOPY) -O binary $< $@

$(OVL_D)/%.elf: $(OVL_D)/%.r.o $(OVL_D)/main.elf port/ovl/ovl.ld
	$(OVL_LD) $(OVL_LDFLAGS) --just-symbols=$(OVL_D)/main.elf -Map $(@:.elf=.map) $< -o $@
	$(OVL_NO_UNDEF)

$(OVL_D)/DOOMCODE.DMC: $(OVL_D)/main.elf $(OVL_D)/a.elf $(OVL_D)/b.elf tools/pack_overlays.py
	$(OVL_PACK) pack --report --main $< -o $@ $(filter-out $<,$(filter %.elf,$^))
$(OVL_D)/DOOMCODE_LC.DMC: $(OVL_D)/main.elf $(OVL_D)/a_lc.elf $(OVL_D)/b_lc.elf tools/pack_overlays.py
	$(OVL_PACK) pack --report --main $< -o $@ $(filter-out $<,$(filter %.elf,$^))
$(OVL_D)/NOENT.DMC: $(OVL_D)/main.elf $(OVL_D)/b.elf tools/pack_overlays.py
	$(OVL_PACK) pack --main $< -o $@ $(filter-out $<,$(filter %.elf,$^))

$(OVL_D)/nor.img $(OVL_D)/nor.args &: $(OVL_D)/DOOMCODE.DMC $(OVL_D)/DOOMCODE_LC.DMC \
		$(OVL_D)/NOENT.DMC tests/ovl/norimg.py tests/ovl/ovl_demo.h port/ovl/ovl.h
	$(OVL_PY) -I tests/ovl/norimg.py --good $(OVL_D)/DOOMCODE.DMC --lc $(OVL_D)/DOOMCODE_LC.DMC \
		--noent $(OVL_D)/NOENT.DMC -o $(OVL_D)/nor.img --args $(OVL_D)/nor.args

# ---- Packer test fixtures: overlays the packer or the link must refuse, and a firmware with
# an object at FRAME_BASE whose other addresses are all moved ----

$(OVL_N)/%.elf: $(OVL_N)/%.r.o $(OVL_D)/main.elf port/ovl/ovl.ld
	$(OVL_LD) $(OVL_LDFLAGS) --just-symbols=$(OVL_D)/main.elf $< -o $@
$(OVL_N)/main_alt.unstamped.elf: $(OVL_R)/tests/ovl/neg_dyn.o $(OVL_RES_OBJS) $(OVL_PLAT_OBJS) \
		$(OVL_D)/exports.ld $(OVL_LDS)
	@mkdir -p $(dir $@)
	$(OVL_CC) $(OVL_MAIN_LDFLAGS) $(filter %.o,$^) -Wl,$(OVL_D)/exports.ld \
		-Wl,-u,neg_dyn_fn -Wl,-u,neg_dyn_buf -o $@
$(OVL_N)/main_alt.elf: $(OVL_N)/main_alt.unstamped.elf tools/pack_overlays.py
	$(OVL_PACK) stamp $< $@
$(OVL_N)/b_alt.elf: $(OVL_D)/b.r.o $(OVL_N)/main_alt.elf port/ovl/ovl.ld
	$(OVL_LD) $(OVL_LDFLAGS) --just-symbols=$(OVL_N)/main_alt.elf $< -o $@

OVL_FIXTURES := $(OVL_N)/neg_big.elf $(OVL_N)/neg_loader.elf $(OVL_N)/neg_indirect.elf \
	$(OVL_N)/neg_state.elf $(OVL_N)/neg_frame.elf $(OVL_N)/neg_movt.elf $(OVL_N)/ok_frame.elf \
	$(OVL_N)/neg_huge.r.o $(OVL_N)/neg_cross.r.o $(OVL_N)/main_alt.elf $(OVL_N)/b_alt.elf

# ---- Host loader tests (ASan and UBSan): fixed NOR offset, and the data layer's extents ----

OVL_HOST_COMMON := port/ovl/ovl.c port/data/crc32.c tests/ovl/demo_seq.c tests/ovl/ovl_a.c \
	tests/ovl/ovl_b.c tests/ovl/host_stubs.c
OVL_HOST_SRC_test_host := port/ovl/ovl_fixed.c tests/ovl/test_host.c
OVL_HOST_SRC_test_ext := port/ovl/ovl_file.c port/data/extents.c port/data/fat12.c \
	tests/ovl/test_ext.c
$(OVL_B)/host/%.o: %.c mk/ovl.mk
	@mkdir -p $(dir $@)
	$(OVL_HOSTCC) $(OVL_HCFLAGS) -c $< -o $@
$(OVL_B)/host/test_host: $(patsubst %.c,$(OVL_B)/host/%.o,$(OVL_HOST_COMMON) $(OVL_HOST_SRC_test_host))
	$(OVL_HOSTCC) $(OVL_HCFLAGS) $^ -o $@
$(OVL_B)/host/test_ext: $(patsubst %.c,$(OVL_B)/host/%.o,$(OVL_HOST_COMMON) $(OVL_HOST_SRC_test_ext))
	$(OVL_HOSTCC) $(OVL_HCFLAGS) $^ -o $@

# Bootloader stand-in for Renode
$(OVL_B)/blstub.bin: tests/renode/blstub.S
	@mkdir -p $(dir $@)
	$(OVL_CC) $(OVL_ARCH) -nostdlib -fuse-ld=lld -Wl,-Ttext=0x08000000 -Wl,-e,_start $< \
		-o $(@:.bin=.elf)
	$(OVL_OBJCOPY) -O binary $(@:.bin=.elf) $@

OVL_OUT := $(OVL_D)/OVLDEMO.BIN $(OVL_D)/nor.img $(OVL_B)/host/test_host $(OVL_B)/host/test_ext \
	$(OVL_B)/blstub.bin $(OVL_B)/obj/prod/port/ovl/ovl.o $(OVL_B)/obj/prod/port/ovl/ovl_file.o

.PHONY: ovl ovl-test ovl-renode-test
all: ovl
test: ovl-test
ovl: $(OVL_OUT)

ovl-test: $(OVL_OUT) $(OVL_FIXTURES)
	$(OVL_PY) -I tests/ovl/test_pack.py --build $(OVL_B) --ld "$(OVL_LD) $(OVL_LDFLAGS)"
	$(OVL_B)/host/test_host --nor $(OVL_D)/nor.img $$(cat $(OVL_D)/nor.args)
	$(OVL_B)/host/test_ext --nor $(OVL_D)/nor.img $$(cat $(OVL_D)/nor.args)

ovl-renode-test: $(OVL_D)/OVLDEMO.BIN $(OVL_D)/nor.img $(OVL_B)/blstub.bin
	RENODE=$(RENODE) $(OVL_PY) -I tests/ovl/test_renode.py --build $(OVL_B)

-include $(wildcard $(OVL_B)/obj/*/*/*/*.d $(OVL_B)/obj/plat/*.d $(OVL_B)/host/*/*/*.d)
