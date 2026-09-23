# Embedded Operating Systems and Architecture course
# Licensed under the MIT License. See LICENSE in the repository root.
#
# Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>

# ARM bare-metal cross-compiler: runs on the host, emits Cortex-M code.
CC := arm-none-eabi-gcc
# All generated files go here; `make clean` just deletes this directory.
BUILD_DIR := build
# Final linked image. .axf is ARM's name for an ELF carrying debug info.
BIN := $(BUILD_DIR)/scratch-os.axf

# Every translation unit that goes into the image.
SOURCES := boot/startup.c src/main.c
# Map each foo.c to its build/foo.o object file.
OBJECTS := $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# -mthumb / -mcpu: target the Cortex-M3's Thumb instruction set.
# -nostartfiles: no default C startup code - boot/startup.c is ours.
# -ffreestanding: no hosted C runtime/libc to rely on (also what makes
# the compiler's freestanding <stdint.h> resolve instead of looking for
# a hosted one that toolchain installs without a bundled newlib lack).
# -std=gnu17: C17 plus GNU extensions (used by the inline-asm blocks).
CFLAGS := -mthumb -mcpu=cortex-m3 -nostartfiles -ffreestanding -std=gnu17
# Enable the standard and extra warning sets.
CFLAGS += -Wall -Wextra
# -O0 + full debug info: keep generated code close to the source so it
# can be stepped through instruction by instruction in class.
CFLAGS += -O0 -g3
# Not -Werror: unlike freertos-labs/ (a pinned, unmodified vendor
# snapshot), this code is maintained across course years and should
# keep building as toolchains evolve, without silently breaking on a
# newly-strict warning turning into a hard error.

# Use our linker script instead of the toolchain's default.
LDFLAGS := -T scripts/mps2_m3.ld
# Also emit a map file showing which symbol landed at which address.
LDFLAGS += -Xlinker -Map=$(BUILD_DIR)/output.map
# -nostdlib: don't link against a hosted libc/startup - toolchain
# installs without a bundled newlib (e.g. plain Homebrew/apt packages)
# don't have one to link anyway. -lgcc still pulls in libgcc's software
# helpers (e.g. integer division) that the compiler itself may emit.
LDFLAGS += -nostdlib -lgcc

# These targets are command names, not files on disk.
.PHONY: all clean run

# Default goal: build the image.
all: $(BIN)

# Compile one .c to a .o, creating its build subdirectory first.
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link every object into the final image.
$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(BIN)

# Boot the image in QEMU's MPS2-AN385 model with semihosting enabled so
# the program's output reaches this terminal.
run: $(BIN)
	qemu-system-arm -machine mps2-an385 -monitor null -semihosting \
	    --semihosting-config enable=on,target=native \
	    -kernel $(BIN) -serial stdio -nographic

# Remove every generated file.
clean:
	rm -rf $(BUILD_DIR)
