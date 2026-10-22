# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
#
# DJI RC-N1 game pad (controller/rc-n1). The firmware links the driver dmc_dji.c (mk/fw.mk),
# the rules here build its host test and the RCPROBE app.
#
#   make -j$(nproc) dji-test   the driver against a fake usbd, as built and without the 06/01
#                              polling (DJI_POLL=0)
#   make -j$(nproc) rcprobe    build/rc-n1/RCPROBE.BIN, the probe app for the meter that logs
#                              what the controller sends (controller/rc-n1/README.md)

RCN1 := controller/rc-n1
RCN1_B := $(BUILD)/rc-n1

RCN1_DJI_SRC := $(RCN1)/dmc_dji.c $(RCN1)/dji_test.c
RCN1_DJI_CFLAGS := $(filter-out -MMD -MP,$(GAME_HCFLAGS)) -I$(RCN1) -DPLAT_MODEL \
	-include tests/game/regs.h
RCN1_DJI_DEPS := $(RCN1_DJI_SRC) $(RCN1)/dmc_dji.h port/game/pad.h platform/usbd.h tests/game/regs.h \
	mk/rc-n1.mk
$(RCN1_B)/dji_test: $(RCN1_DJI_DEPS)
	@mkdir -p $(dir $@)
	$(GAME_CC) $(RCN1_DJI_CFLAGS) -DDJI_POLL=0 $(RCN1_DJI_SRC) -o $@
$(RCN1_B)/dji_poll_test: $(RCN1_DJI_DEPS)
	@mkdir -p $(dir $@)
	$(GAME_CC) $(RCN1_DJI_CFLAGS) $(RCN1_DJI_SRC) -o $@

# RCPROBE, the meter's DMA build with the platform core, the text UI and the USB driver
PLAT_APP_SRC_rcprobe := rc-n1/rcprobe apps/ui usbd
$(PLAT_B)/dev/rc-n1/rcprobe.o: $(RCN1)/rcprobe.c mk/rc-n1.mk mk/platform.mk $(PLAT_STAMP)
	@mkdir -p $(dir $@)
	$(PLAT_CC) $(PLAT_CFLAGS) $(PLAT_DEFS_dev) -c $< -o $@
$(eval $(call PLAT_APP_RULES,rcprobe,dev))
$(RCN1_B)/RCPROBE.BIN: $(PLAT_B)/dev/rcprobe.bin
	@mkdir -p $(dir $@)
	cp $< $@

.PHONY: dji-test rcprobe
all: rcprobe
test: dji-test

rcprobe: $(RCN1_B)/RCPROBE.BIN

dji-test: $(RCN1_B)/dji_test $(RCN1_B)/dji_poll_test
	$(RCN1_B)/dji_test
	$(RCN1_B)/dji_poll_test

-include $(wildcard $(PLAT_B)/dev/rc-n1/*.d)
