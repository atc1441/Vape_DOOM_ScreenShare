# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 VapeDoom port
#
# Firmware for the Aspire PIXO (PY32F403xC, 64 KB RAM, 254 KB flash).
#
#   make -j$(nproc) py32        DOOMVAPE.BIN and its size report
#
# Unlike the meter build this one has no overlays: the whole program is
# resident. The flash is two and a half times as large, and the overlay file
# would live on a NOR that shares its bus with the panel, so loading code
# would have to interrupt a frame. That also removes the two-stage link, the
# exports file and the packer, which is why this is a plain makefile and not
# another variant of mk/fw.mk.

# -O2, not the -Oz of the meter build: that one is squeezed into a 99 KB
# slot, this one has 254 KB and the engine is what the frame rate rides on.
P32_B := $(BUILD)/py32
P32_O := $(P32_B)/obj
P32_CC ?= $(FW_CC)
P32_NM ?= $(FW_NM)
P32_OBJCOPY ?= $(FW_OBJCOPY)
P32_SIZE ?= $(FW_SIZE)

# RC_SETS=256 makes the record cache 18.2 KiB instead of 2.3 KiB. On the
# DMC-100 that would not do: RAM is tight there. Here it is not, and every
# miss avoided is one serial NOR access less on the bus that the flash and
# the panel have to share.
P32_DEFS := -DDMC_FW -DDMC_HASHORDER -DDMC_SINE_L1 -DOVL_RESIDENT -DRC_SETS=256
P32_INC := -Iplatform/py32 -Iplatform -Iplatform/apps -Iport/game -Iport/data -Iport/render \
	-Iport/obj -Iport/math -Iport/ovl -Itests/lockstep -Icontroller/rc-n1 -I$(GEN)

P32_ECFLAGS := $(FW_ARCH) -O2 -ffunction-sections -fdata-sections -ffreestanding -fno-common \
	-fwrapv -fsigned-char -std=gnu11 -w $(P32_DEFS) -Iengine/stub/include -Iengine $(P32_INC) \
	-MMD -MP
P32_GCFLAGS := $(FW_ARCH) -O2 -g -std=gnu11 -ffunction-sections -fdata-sections -ffreestanding \
	-fno-common -fwrapv -fsigned-char -Wall -Wextra -Wshadow -Wstrict-prototypes $(P32_DEFS) \
	-isystem engine/stub/include -isystem engine $(P32_INC) -MMD -MP
P32_PCFLAGS := $(FW_ARCH) -std=c99 -O2 -g -ffreestanding -fno-builtin -ffunction-sections \
	-fdata-sections -fwrapv -fsigned-char -Wall -Wextra -Wshadow -Wstrict-prototypes \
	$(P32_DEFS) $(P32_INC) -MMD -MP

# Engine and port: everything the meter splits into resident image and
# overlays, in one image. am_map is stubbed in fe_glue.c, z_zone is replaced
# by i_dmc.c and m_argv has no command line here.
P32_ENG_OUT := engine/am_map.c engine/z_zone.c engine/m_argv.c
P32_ENG_SRC := $(filter-out $(P32_ENG_OUT),$(ENG_SRC) $(wildcard port/data/*.c) \
	$(wildcard port/render/*.c) port/math/trig.c $(wildcard port/obj/*.c))
# dmc_hash and dmc_report belong to the lockstep harness and the benchmark,
# neither of which this build has. dmc_usb needs a USB device stack.
P32_GAME_OUT := port/game/dmc_hash.c port/game/dmc_usb.c port/game/dmc_report.c
P32_GAME_SRC := $(filter-out $(P32_GAME_OUT),$(wildcard port/game/*.c)) platform/apps/ui.c

P32_PLAT := startup plat_at32 systick buttons buzzer settings fault nor lcd pump text \
	font8x8 rt usbd dbg ovl_resident

P32_OBJS := $(patsubst %.c,$(P32_O)/%.o,$(P32_ENG_SRC) $(P32_GAME_SRC)) \
	$(patsubst $(GEN)/%.c,$(P32_O)/gen/%.o,$(ENG_GEN_SRC)) \
	$(P32_PLAT:%=$(P32_O)/plat/%.o)

P32_LDS := platform/link/py32_doom.ld platform/link/py32_regions.ld
# The only --wrap redirect this build needs. All the others in mk/fw.mk are
# overlay entry points or measuring points of the timed demo run; resident
# and unmeasured, they go straight to their target.
#
# D_PostEvent is different: under DMC_FW, engine/d_event.c keeps a ring of
# *two* events, because port/game/i_dmc.c handles each event the moment it
# is posted. Without the redirect nothing drains, and two events in a row
# make the ring's head and tail coincide - the queue then looks empty and
# both are gone.
#
# On the DMC-100 that never showed: five switches and one thumb do not
# produce two simultaneous presses, and two at once are a chord there, i.e.
# one key. A glass surface produces them constantly - a finger at the top
# left is exactly that. Measured: each alone worked, both together did
# nothing, one after the other worked again (tools/touch.py motion).
P32_LDFLAGS := -Wl,-L,platform/link -Wl,--defsym=__dmc_variant=3 	-Wl,--wrap=D_PostEvent

# Engine sources compile with upstream's own warning set (none); the port
# glue and the platform with ours. One pattern rule, the flags picked per
# object, because a static pattern rule per directory and a plain one cannot
# live side by side in make.
P32_CFLAGS_SEL = $(P32_GCFLAGS)
$(patsubst %.c,$(P32_O)/%.o,$(P32_ENG_SRC)): P32_CFLAGS_SEL = $(P32_ECFLAGS)
$(patsubst $(GEN)/%.c,$(P32_O)/gen/%.o,$(ENG_GEN_SRC)): P32_CFLAGS_SEL = $(P32_ECFLAGS)

$(P32_O)/%.o: %.c | $(GEN)/dmcdat.h
	@mkdir -p $(dir $@)
	$(P32_CC) $(P32_CFLAGS_SEL) -c $< -o $@

$(P32_O)/gen/%.o: $(GEN)/%.c
	@mkdir -p $(dir $@)
	$(P32_CC) $(P32_ECFLAGS) -c $< -o $@

$(P32_O)/plat/%.o: platform/py32/%.c
	@mkdir -p $(dir $@)
	$(P32_CC) $(P32_PCFLAGS) $(P32_OPT) -c $< -o $@
# The pixel loop carries the frame rate; size optimisation costs about a third
# of it.
$(P32_O)/plat/pump.o: P32_OPT := -O2
$(P32_O)/plat/nor.o: P32_OPT := -O2

$(P32_B)/DOOMVAPE.elf: $(P32_OBJS) $(P32_LDS)
	@mkdir -p $(dir $@)
	$(P32_CC) $(FW_ARCH) -nostdlib -fuse-ld=lld $(P32_LDFLAGS) \
		-Wl,-T,platform/link/py32_doom.ld -Wl,--gc-sections \
		-Wl,-Map,$(@:.elf=.map) $(filter %.o,$^) -o $@
# The overlay tables are deliberately weakly undefined: an overlay whose
# sources this build does not link has none. Anything else would be a bug.
	@u="$$($(P32_NM) -u $@ | grep -v " w ")"; if [ -n "$$u" ]; then \
		echo "$@: undefined symbols:"; echo "$$u"; rm -f $@; exit 1; fi
	$(P32_SIZE) $@

$(P32_B)/DOOMVAPE.BIN: $(P32_B)/DOOMVAPE.elf
	$(P32_OBJCOPY) -O binary -j .text -j .data $< $@

# Intel hex for the J-Link Commander. Its loadfile has proven more reliable
# on this part than the raw image through the DLL.
$(P32_B)/DOOMVAPE.hex: $(P32_B)/DOOMVAPE.elf
	$(P32_OBJCOPY) -O ihex -j .text -j .data $< $@

.PHONY: py32
py32: $(P32_B)/DOOMVAPE.BIN $(P32_B)/DOOMVAPE.hex
	@ls -l $(P32_B)/DOOMVAPE.BIN $(P32_B)/DOOMVAPE.hex

-include $(shell find $(P32_O) -name '*.d' 2>/dev/null)
