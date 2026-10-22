# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Artur Bieniek
#
# Build rules of the renderer: the strip renderer (engine/r_*, port/render) is built with
# the engine (mk/engine.mk). Run from the repository root with make -j$(nproc).
#
#   render-test     frame region per plan against its budget, W-invariance of
#                   the 3D view and the status band over DEMO1 and the E1M1
#                   script for W 1, 2, 4, 8, 16, 160 in both detail modes,
#                   the status band against the oracle's status bar (E1M1
#                   script and DEMO1) and the HUD line against its widget, renderer
#                   counters, pool high-water marks and the timing model
#                   (frame-test in mk/engine.mk is the oracle pixel test of the
#                   3D view)
#   render-size     ARM size of the renderer and of the drawer inner loops
#                   at -Oz and -O2, gate G2 (.text and static RAM outside the
#                   frame region), fails when a gate fails

.PHONY: render-test render-size render-region render-winvar render-stats render-band
test: render-test
gates: render-size

REND_PY ?= python3
REND_OUT := $(BUILD)/render

# The drawer inner loops are the only renderer code built -O2 on the target,
# with the hot loops unrolled by hand instead of by the compiler.
REND_O2 := -O2 -fno-unroll-loops
$(ENG_ARM_OUT)/r_inner.o: port/render/r_inner.c $(CONV_GEN_FILES) | $(ENG_ARM_OUT)
	$(ENG_CC) $(ENG_ARM_CFLAGS) $(REND_O2) -c $< -o $@

$(REND_OUT):
	mkdir -p $@

render-region: $(BUILD)/host/dmcdoom
	$(BUILD)/host/dmcdoom --frame-report

render-winvar: $(BUILD)/host/dmcdoom nor-images | $(REND_OUT)
	$(REND_PY) tests/render/winvar.py --engine $(BUILD)/host/dmcdoom --nor $(NOR_OUT)/E1.img \
		--nor-e1m1 $(NOR_OUT)/E1M1.img --out $(REND_OUT)

render-stats: $(BUILD)/host/dmcdoom nor-images | $(REND_OUT)
	$(REND_PY) tests/render/rstats.py --engine $(BUILD)/host/dmcdoom --nor $(NOR_OUT)/E1.img \
		--nor-e1m1 $(NOR_OUT)/E1M1.img --out $(REND_OUT)

render-band: oracle $(BUILD)/host/dmcdoom nor-images | $(REND_OUT)
	$(REND_PY) tests/render/bandtest.py --oracle $(ORA_OUT)/dg_oracle --engine $(BUILD)/host/dmcdoom \
		--nor $(NOR_OUT)/E1M1.img --nor-e1 $(NOR_OUT)/E1.img --out $(REND_OUT)

render-test: render-region render-winvar render-band render-stats

# -Oz and -O2 objects of the drawer loops for the report
$(REND_OUT)/r_inner_oz.o: port/render/r_inner.c $(CONV_GEN_FILES) | $(REND_OUT)
	$(ENG_CC) $(ENG_ARM_CFLAGS) -c $< -o $@
$(REND_OUT)/r_inner_o2.o: port/render/r_inner.c $(CONV_GEN_FILES) | $(REND_OUT)
	$(ENG_CC) $(ENG_ARM_CFLAGS) $(REND_O2) -c $< -o $@

render-size: $(ENG_ARM_OUT)/engine.elf $(REND_OUT)/r_inner_oz.o $(REND_OUT)/r_inner_o2.o mk/render.mk \
		| $(REND_OUT)
	$(ENG_PY) tools/gates/sizegate.py $(ENG_ARM_OUT)/engine.map --modules > $(REND_OUT)/sizegate.txt; \
		st=$$?; grep -E '^(group|renderer|r_|v_video|rview|trig|G2|size gates)' \
		$(REND_OUT)/sizegate.txt; exit $$st
	@for v in oz o2; do llvm-size -A $(REND_OUT)/r_inner_$$v.o | awk -v v=$$v \
		'$$1 ~ /^\.text/ { t += $$2 } END { printf "drawer inner loops (port/render/r_inner.c) -%s: .text %d B\n", \
		v == "oz" ? "Oz" : "O2 -fno-unroll-loops", t }'; done
