# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
#
# Platform layer (platform/): the objects of the meter's DMA build and of the polled build for
# Renode, the host model test of the DMA paths and the platform footprint report.
#
#   make -j$(nproc) platform               all of the above
#   make -j$(nproc) platform-test          host model test of pump.c and nor.c
#   make -j$(nproc) platform-size          the platform footprint against its ceiling
#   make platform-font                     regenerates platform/font8x8.c (FONT8X8_BASIC=path)
#
# Options (the objects are rebuilt when they change):
#   PLAT_LCD_INIT=MINIMAL  minimal LCD init as the boot default (PIMORONI otherwise)
#   PLAT_NOR_SLOW=0        SPI1 (NOR) at 72 MHz instead of 36 MHz. On the meter 72 MHz
#                          returned 2 bad 4 KB blocks in 1 GB read, 36 MHz none.
#   PLAT_LCD_SLOW=1        SPI2 (LCD) at 36 MHz instead of 72 MHz

PLAT_LCD_INIT ?= PIMORONI
PLAT_NOR_SLOW ?= 1
PLAT_LCD_SLOW ?= 0
PLAT_CC      ?= clang
PLAT_HOSTCC  ?= clang
PLAT_OBJCOPY ?= llvm-objcopy
PLAT_SIZE    ?= llvm-size
PLAT_NM      ?= llvm-nm
PLAT_PY      ?= python3
# The Renode tests of mk/fw.mk and mk/ovl.mk run this Renode (a portable 1.17 build works)
RENODE       ?= renode

PLAT_B  := $(BUILD)/plat
PLAT_ARCH := --target=thumbv7em-none-eabi -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
PLAT_WARN := -Wall -Wextra -Wshadow -Wstrict-prototypes
PLAT_CFLAGS := $(PLAT_ARCH) -std=c99 -Oz -g -ffreestanding -fno-builtin -ffunction-sections \
	-fdata-sections -fwrapv -fsigned-char $(PLAT_WARN) -Iplatform -Iplatform/apps -Iport/math -MMD -MP \
	-DLCD_INIT_DEFAULT=LCD_INIT_$(PLAT_LCD_INIT) -DNOR_SPI_SLOW=$(PLAT_NOR_SLOW) \
	-DLCD_SPI_SLOW=$(PLAT_LCD_SLOW)
PLAT_LDFLAGS := $(PLAT_ARCH) -nostdlib -fuse-ld=lld -Wl,-L,platform/link \
	-Wl,-T,platform/link/app.ld -Wl,--gc-sections

PLAT_CORE := startup plat_at32 systick buttons buzzer settings fault nor lcd pump text font8x8 rt
PLAT_LDS := platform/link/app.ld platform/link/regions.ld
PLAT_APP_SRC_footprint := tests/footprint

# Size check: the platform's .text in the footprint link (all of the engine-facing API, the
# vectors and the font). The build fails above PLAT_TEXT_MAX, set a little above the current
# size so that growth shows up.
PLAT_TEXT_MAX ?= 9020

PLAT_DEFS_dev :=
PLAT_DEFS_renode := -DNOR_POLLED=1 -DLCD_POLLED=1

# The options are not visible in file times: the target objects depend on this stamp, which
# is rewritten only when they change
PLAT_OPTS := $(PLAT_LCD_INIT) $(PLAT_NOR_SLOW) $(PLAT_LCD_SLOW)
PLAT_STAMP := $(PLAT_B)/options.stamp
$(PLAT_STAMP): plat-force
	@mkdir -p $(dir $@)
	@echo '$(PLAT_OPTS)' | cmp -s - $@ || echo '$(PLAT_OPTS)' > $@
.PHONY: plat-force

# Objects: $(PLAT_B)/<variant>/<name>.o, the pump interrupt is built with -O2. Variants: dev
# (the meter) and renode (polled NOR and LCD).
define PLAT_OBJ_RULES
$(PLAT_B)/$(1)/%.o: platform/%.c mk/platform.mk $(PLAT_STAMP)
	@mkdir -p $$(dir $$@)
	$(PLAT_CC) $(PLAT_CFLAGS) $(PLAT_DEFS_$(1)) $$(PLAT_OPT) -c $$< -o $$@
$(PLAT_B)/$(1)/pump.o: PLAT_OPT := -O2
endef
$(foreach v,dev renode,$(eval $(call PLAT_OBJ_RULES,$(v))))

# A stand-alone app $(1) of the variant $(2): the platform core and PLAT_APP_SRC_$(1)
define PLAT_APP_RULES
$(PLAT_B)/$(2)/$(1).elf: $(addprefix $(PLAT_B)/$(2)/,$(addsuffix .o,$(PLAT_CORE) $(PLAT_APP_SRC_$(1)))) $(PLAT_LDS)
	$(PLAT_CC) $(PLAT_LDFLAGS) -Wl,-Map,$(PLAT_B)/$(2)/$(1).map $$(filter %.o,$$^) -o $$@
	@undef=$$$$($(PLAT_NM) -u $$@); if [ -n "$$$$undef" ]; then echo "$$@: undefined symbols:"; echo "$$$$undef"; rm -f $$@; exit 1; fi

$(PLAT_B)/$(2)/$(1).bin: $(PLAT_B)/$(2)/$(1).elf
	$(PLAT_OBJCOPY) -O binary $$< $$@
endef
$(eval $(call PLAT_APP_RULES,footprint,dev))

$(PLAT_B)/dev/footprint.txt: $(PLAT_B)/dev/footprint.elf $(PLAT_B)/dev/tests/footprint.o
	@t=$$($(PLAT_SIZE) -A $< | awk '$$1 == ".text" { print $$2 }'); \
	a=$$($(PLAT_SIZE) $(PLAT_B)/dev/tests/footprint.o | awk 'NR == 2 { print $$1 }'); \
	echo "platform footprint $$((t - a)) B (.text $$t B minus the footprint app $$a B)," \
		"ceiling $(PLAT_TEXT_MAX) B" | tee $@; \
	if [ $$((t - a)) -gt $(PLAT_TEXT_MAX) ]; then \
		rm -f $@; echo "platform footprint over PLAT_TEXT_MAX"; exit 1; fi

# Host model test of the DMA paths of pump.c and nor.c. The stuck-DMA timeouts are longer
# than on the target, so a busy host does not trip them.
PLAT_MFLAGS := -std=gnu99 -O1 -g -DPLAT_MODEL -DNOR_MODEL -include platform/tests/model.h \
	-DPUMP_WAIT_MS=1000 -DNOR_WAIT_MS=1000 -fno-pie -pthread -fsanitize=address,undefined \
	$(PLAT_WARN) -Iplatform -MMD -MP
$(PLAT_B)/model/%.o: platform/%.c platform/tests/model.h mk/platform.mk
	@mkdir -p $(dir $@)
	$(PLAT_HOSTCC) $(PLAT_MFLAGS) -c $< -o $@
$(PLAT_B)/model/model: $(PLAT_B)/model/tests/model.o $(PLAT_B)/model/pump.o $(PLAT_B)/model/nor.o
	$(PLAT_HOSTCC) $(PLAT_MFLAGS) -no-pie $^ -o $@

# Bootloader stand-in for Renode
$(PLAT_B)/renode/blstub.bin: tests/renode/blstub.S
	@mkdir -p $(dir $@)
	$(PLAT_CC) $(PLAT_ARCH) -nostdlib -fuse-ld=lld -Wl,-Ttext=0x08000000 -Wl,-e,_start $< -o $(@:.bin=.elf)
	$(PLAT_OBJCOPY) -O binary $(@:.bin=.elf) $@

PLAT_OUT := $(PLAT_B)/model/model $(PLAT_B)/renode/blstub.bin $(PLAT_B)/dev/footprint.txt

.PHONY: platform platform-test platform-size platform-font
all: platform
test: platform-test
platform: $(PLAT_OUT)

platform-size: $(PLAT_B)/dev/footprint.txt
	@cat $(PLAT_B)/dev/footprint.txt

platform-test: $(PLAT_B)/model/model
	$(PLAT_B)/model/model

platform-font:
	$(PLAT_PY) -I tools/font2c.py

-include $(wildcard $(PLAT_B)/*/*.d $(PLAT_B)/*/tests/*.d)
