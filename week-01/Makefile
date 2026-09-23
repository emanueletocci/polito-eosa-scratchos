# Embedded Operating Systems and Architecture course
# Licensed under the MIT License. See LICENSE in the repository root.
#
# Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
#
# make reads this file top to bottom, but only *runs* a recipe when you
# ask for a target (`make`, `make run`, `make clean`, ...) that needs it.
# Everything before the first target (down to line ~46) just defines
# variables - plain text substitution, nothing executes yet.

# `:=` is a simple (immediate) variable assignment: the right-hand side
# is expanded once, right here, and the result is stored. (The other
# kind, `=`, is "recursive" and re-expands its value every time the
# variable is used - not needed in this file, but you'll see it elsewhere.)

# ARM bare-metal cross-compiler: runs on the host (x86/ARM Mac/PC), but
# emits machine code for the Cortex-M3 target, not for the host CPU.
CC := arm-none-eabi-gcc
# All generated files go here; `make clean` just deletes this directory.
BUILD_DIR := build
# Final linked image. .axf is ARM's name for an ELF carrying debug info.
BIN := $(BUILD_DIR)/scratch-os.axf

# Every translation unit (source file compiled independently) that goes
# into the image. Add a new .c file here when the project grows.
SOURCES := boot/startup.c src/main.c
# A "substitution reference": $(VAR:PATTERN=REPLACEMENT) rewrites every
# word of VAR that matches PATTERN. Here it turns each "dir/foo.c" into
# "build/dir/foo.o", producing the exact object-file names the rules
# below build and link. Try `make -p | grep OBJECTS` to see the result.
OBJECTS := $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# `+=` appends to a variable that was already defined with `:=` above.
# Splitting CFLAGS across several `+=` lines (rather than one long line)
# is a style choice: each line groups flags with a related purpose and
# can carry its own explanatory comment.

# -mthumb / -mcpu: target the Cortex-M3's Thumb instruction set.
# -nostartfiles: impacts the linker. It tells GCC not to automatically
#  include the default system startup code (the code that initializes
#  the environment before jumping to main). This is because we need
#  to use our own code - boot/startup.c.
# -ffreestanding: impacts the compiler. It tells GCC that standard
#  operating system features (like a full standard C library or a
#  guaranteed main entry point) are not available.
# -std=gnu17: C17 plus GNU extensions (used by the inline-asm blocks).
CFLAGS := -mthumb -mcpu=cortex-m3 -nostartfiles -ffreestanding -std=gnu17
# Enable the standard and extra warning sets.
CFLAGS += -Wall -Wextra
# -O0 + full debug info: keep generated code close to the source so it
# can be stepped through instruction by instruction in class.
CFLAGS += -O0 -g3

# Flags passed to the linking step only (not to plain compilation).
# -T: use our linker script instead of the toolchain's default, so code
# and data land at the addresses mps2_m3.ld describes for this board.
LDFLAGS := -T scripts/mps2_m3.ld
# -Xlinker passes the next word straight to the linker unchanged (GCC
# would otherwise try to interpret "-Map=..." itself). This makes the
# linker emit a map file showing which symbol landed at which address -
# useful for checking where _sdata, _estack, etc. actually ended up.
LDFLAGS += -Xlinker -Map=$(BUILD_DIR)/output.map
# -nostdlib: don't link against a hosted libc/startup - toolchain
# installs without a bundled newlib (e.g. plain Homebrew/apt packages)
# don't have one to link anyway. -lgcc still pulls in libgcc's software
# helpers (e.g. integer division) that the compiler itself may emit.
LDFLAGS += -nostdlib -lgcc

# .PHONY declares that "all", "clean" and "run" are commands to run, not
# files to produce. Without it, if a file literally named "clean" ever
# appeared in this directory, `make clean` would see it as already
# up to date and silently do nothing instead of running the recipe.
.PHONY: all clean run

# The first target in the file is the "default goal": plain `make` with
# no arguments is exactly equivalent to `make all`.
# `all: $(BIN)` means "before considering `all` done, make sure $(BIN)
# is up to date" - which recursively triggers the $(BIN) rule below.
all: $(BIN)

# A pattern rule: `%` matches any stem, so this one rule compiles *any*
# $(BUILD_DIR)/foo.o from the matching foo.c, instead of writing one
# rule per source file by hand.
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Inside a recipe, `$@` and `$<` are "automatic variables" make fills in
# per invocation of the rule above:
#   $@ - the target of this rule (e.g. build/src/main.o)
#   $< - the first prerequisite (e.g. src/main.c)
# `$(dir $@)` strips the filename, leaving just the directory part (e.g.
# build/src/), which `mkdir -p` then creates if it doesn't exist yet -
# necessary because $(BUILD_DIR) mirrors the source tree (build/boot/,
# build/src/) and neither subdirectory exists until this runs.
# A leading `@` on a recipe line suppresses echoing that line itself to
# the terminal (the compiler's own output, if any, still shows).
# `-c`: compile/assemble only, don't link - produces a .o, not a binary.

# Link every object into the final image. `$(BIN): $(OBJECTS)` means
# this recipe only re-runs when $(BIN) is missing or older than any
# file in $(OBJECTS) - make's incremental-build mechanism, comparing
# file timestamps so unchanged sources aren't needlessly recompiled.
$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(BIN)

# Boot the image in QEMU's MPS2-AN385 model (an emulated Cortex-M3 board
# matching scripts/mps2_m3.ld's memory map) with semihosting enabled so
# the program's output reaches this terminal.
#   -machine mps2-an385        - emulate this specific board
#   -monitor null              - disable QEMU's own interactive monitor,
#                                 so it doesn't compete for the terminal
#   -semihosting                - turn on the semihosting trap the bkpt
#                                 instruction in src/main.c relies on
#   --semihosting-config
#     enable=on,target=native  - "native" lets QEMU itself service the
#                                 semihosting calls (rather than passing
#                                 them to an attached GDB)
#   -kernel $(BIN)              - the ELF image to load and boot
#   -serial stdio               - route the emulated UART to this
#                                 terminal (unused in week 1, wired up
#                                 in week 3, but harmless to enable now)
#   -nographic                  - no graphical window; everything goes
#                                 to the terminal, which is all week 1
#                                 needs
run: $(BIN)
	qemu-system-arm -machine mps2-an385 -monitor null -semihosting \
	    --semihosting-config enable=on,target=native \
	    -kernel $(BIN) -serial stdio -nographic

# Remove every generated file. Safe to run any time - $(BUILD_DIR) holds
# nothing that isn't reproducible by `make all`.
clean:
	rm -rf $(BUILD_DIR)
