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

# Every source file that goes into the image: the hand-written assembly
# boot code (vector table + Reset_Handler) and the C program.
SOURCES := boot/startup.s src/main.c
# Map each foo.c or foo.s to its build/foo.o object file. Two nested
# patsubst calls: the inner one rewrites any ".s" source to a ".o" path,
# and the outer one does the same for ".c" sources (a patsubst that
# doesn't match a word just leaves it alone, so each source is only
# touched by the substitution that actually applies to it).
OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(patsubst %.s,$(BUILD_DIR)/%.o,$(SOURCES)))

# -mthumb / -mcpu: target the Cortex-M3's Thumb instruction set.
# -nostartfiles: no default C startup code - boot/startup.s is ours.
# -ffreestanding: no hosted environment - no full standard library, no
#  distinguished main. With -nostdlib below, this is what makes "no C
#  library" true of the image rather than merely intended.
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
# -nostdlib: don't link a hosted libc or its startup code. Toolchain
# installs without a bundled newlib have none to link anyway, and without
# this the linker still searches newlib, so a stray stdio call fails on
# its missing syscall stubs (_write, _sbrk) instead of on the call itself.
# -lgcc still supplies libgcc's software helpers, such as integer
# division, that the compiler may emit on its own.
LDFLAGS += -nostdlib -lgcc

# These targets are command names, not files on disk.
.PHONY: all clean run debug

# Default goal: build the image.
all: $(BIN)

# Compile one .c to a .o, creating its build subdirectory first.
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble one .s (hand-written assembly) to a .o, same idea as the .c
# rule above. We still call $(CC), not a separate assembler: GCC knows
# to hand a ".s" file straight to the assembler, and passing it the same
# CPU flags (-mthumb -mcpu=cortex-m3) makes sure the assembled code
# matches what the C files were compiled for.
$(BUILD_DIR)/%.o: %.s
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

# Same as `run`, but paused for a debugger instead of running freely:
#   -S             - freeze the emulated CPU at reset instead of letting
#                    it start executing immediately
#   -gdb tcp::1234 - open a GDB remote-debug server on TCP port 1234 and
#                    wait for a debugger to attach there
# From a second terminal, connect with:
#   arm-none-eabi-gdb build/scratch-os.axf   (on Linux: gdb-multiarch)
# and inside gdb:
#   target remote :1234
# The CPU stays halted (no program output yet) until gdb tells it to run.
debug: $(BIN)
	qemu-system-arm -machine mps2-an385 -monitor null -semihosting \
	    --semihosting-config enable=on,target=native \
	    -kernel $(BIN) -serial stdio -nographic -S -gdb tcp::1234

# Remove every generated file.
clean:
	rm -rf $(BUILD_DIR)
