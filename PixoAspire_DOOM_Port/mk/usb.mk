# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
#
# Build rules of the USB drive: the 3 MB data volume as a USB drive (port/usb, the USB overlay
# of the firmware), host tests.
#
#   make -j$(nproc) usb-test   the transport and the SCSI commands through a fake controller
#                              driver, the quick format, the folder UI and a DOOM.DAT copied
#                              the way mtools writes it, checked with fsck.fat, mtools and the
#                              data layer (tests/usb/test_usb.py)

USB_CC ?= clang
USB_PY ?= python3

USB_B := $(BUILD)/usb
USB_CFLAGS := -std=gnu11 -O1 -g -Wall -Wextra -Wshadow -fwrapv -fsigned-char -fno-omit-frame-pointer \
	-fsanitize=address,undefined -fno-sanitize-recover=all -Iport/usb -Iport/data -Iplatform -I$(GEN) -Ihost \
	-MMD -MP
USB_SRC := port/usb/usb_ctrl.c port/usb/usb_msc.c port/usb/usb_vol.c port/data/fat12.c \
	port/data/fat_write.c port/data/extents.c port/data/img.c port/data/img_check.c \
	port/data/crc32.c host/nor_file.c tests/usb/usb_host.c

$(USB_B)/%.o: %.c mk/usb.mk $(CONV_GEN_FILES)
	@mkdir -p $(dir $@)
	$(USB_CC) $(USB_CFLAGS) -c $< -o $@
# nor_program wrapped: the suite makes it fail in one sector
$(USB_B)/usb_host: $(patsubst %.c,$(USB_B)/%.o,$(USB_SRC))
	$(USB_CC) -fsanitize=address,undefined -Wl,--wrap=nor_program -o $@ $^

.PHONY: usb-test
test: usb-test

usb-test: $(USB_B)/usb_host packs
	$(USB_PY) -I tests/usb/test_usb.py --build $(BUILD)

-include $(wildcard $(USB_B)/*/*/*.d $(USB_B)/*/*.d)
