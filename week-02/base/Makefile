#
# Embedded Operating Systems and Architecture course
# Licensed under the MIT License. See LICENSE in the repository root.
#
# Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
#
# Week 2 starting point. The fully commented version is in
# ../solution/Makefile.
#   make        build build/scratch-os.axf
#   make run    build and run in QEMU
#   make debug  run in QEMU halted at reset, waiting for gdb on port 1234

CC := arm-none-eabi-gcc
BUILD_DIR := build
BIN := $(BUILD_DIR)/scratch-os.axf

SOURCES := boot/startup.s src/main.c
OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(patsubst %.s,$(BUILD_DIR)/%.o,$(SOURCES)))

# Cortex-M3, Thumb, no hosted C library, keep code close to the source.
CFLAGS := -mthumb -mcpu=cortex-m3 -nostartfiles -ffreestanding -std=gnu17
CFLAGS += -Wall -Wextra
CFLAGS += -O0 -g3

# Our linker script, a map file, no C library (only libgcc helpers).
LDFLAGS := -T scripts/mps2_m3.ld
LDFLAGS += -Xlinker -Map=$(BUILD_DIR)/output.map
LDFLAGS += -nostdlib -lgcc

.PHONY: all clean run debug

all: $(BIN)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(BIN)

run: $(BIN)
	qemu-system-arm -machine mps2-an385 -monitor null -semihosting \
	    --semihosting-config enable=on,target=native \
	    -kernel $(BIN) -serial stdio -nographic

debug: $(BIN)
	qemu-system-arm -machine mps2-an385 -monitor null -semihosting \
	    --semihosting-config enable=on,target=native \
	    -kernel $(BIN) -serial stdio -nographic -S -gdb tcp::1234

clean:
	rm -rf $(BUILD_DIR)
