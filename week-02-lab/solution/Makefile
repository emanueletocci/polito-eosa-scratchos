# Embedded Operating Systems and Architecture course
# Licensed under the MIT License. See LICENSE in the repository root.
#
# Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
#
# WEEK 2 LAB SOLUTION - not a checkpoint. Nothing later in the course
# builds on this folder.
#
# lab A2: this whole file is the A2 deliverable, written from
# lecture-notes/week-01.md section 6 (rules, variables, automatic
# variables, pattern rules, phony targets). lab B1 adds `size` and
# `disasm` at the bottom.

# --- Variables (notes section 6, "Variables") ----------------------------

# The cross-compiler: runs on the host, emits Cortex-M code (section 2).
CC := arm-none-eabi-gcc
# Every generated file goes here, so `clean` is one rm -rf.
BUILD_DIR := build
# The final linked image. .axf is ARM's name for an ELF with debug info.
BIN := $(BUILD_DIR)/scratch-os.axf

# The translation units that make up the image. host/ is deliberately
# NOT here: that program is built for the host, not the target.
SOURCES := boot/startup.c src/main.c
# Substitution reference: map each foo.c to build/foo.o.
OBJECTS := $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# Target policy (notes section 2).
CFLAGS := -mthumb -mcpu=cortex-m3 -nostartfiles -std=gnu17
CFLAGS += -Wall -Wextra
CFLAGS += -O0 -g3

# Our linker script, plus a map file (notes section 3). The map is what
# exercise B4 reads - without this second line, B4 has nothing to open.
LDFLAGS := -T scripts/mps2_m3.ld
LDFLAGS += -Xlinker -Map=$(BUILD_DIR)/output.map

# --- Phony targets (notes section 6, "Phony targets") --------------------

# These name actions, not files. `all` is first, so it is the default
# goal: plain `make` builds the image.
.PHONY: all clean run size disasm

all: $(BIN)

# --- Rules (notes section 6, "Automatic variables" / "Pattern rules") ----

# One pattern rule covers every source file. $(dir $@) is the target's
# directory, created before the compiler writes into it; $< is the .c
# being compiled; $@ is the .o being produced.
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# The link rule: the image depends on every object, so make builds each
# of them (recursively applying the rule above) before linking.
$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(BIN)

# Boot the image under QEMU's MPS2-AN385 model with semihosting on, so
# the target's output reaches this terminal (notes section 7).
run: $(BIN)
	qemu-system-arm -machine mps2-an385 -monitor null -semihosting \
	    --semihosting-config enable=on,target=native \
	    -kernel $(BIN) -serial stdio -nographic

clean:
	rm -rf $(BUILD_DIR)

# --- lab B1: two inspection targets --------------------------------------

# Section sizes: how much FLASH and RAM the image actually claims.
size: $(BIN)
	arm-none-eabi-size $(BIN)

# Disassembly: the instructions really in the file, which B2 compares
# against the compiler's own output for the C version of swap().
disasm: $(BIN)
	arm-none-eabi-objdump -d $(BIN)

# NOTE (lab B1, step 4): scripts/mps2_m3.ld is an input to the link but
# is NOT a prerequisite of $(BIN) above, so editing it does not trigger
# a rebuild - use `make clean && make`. This is the defect the notes
# flag in section 6, left in place deliberately so B1 can find it.
