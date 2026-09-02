# Embedded Operating Systems and Architecture course
# Licensed under the MIT License. See LICENSE in the repository root.
#
# Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>

CC := arm-none-eabi-gcc
BUILD_DIR := build
BIN := $(BUILD_DIR)/scratch-os.axf

SOURCES := boot/startup.c src/main.c
OBJECTS := $(SOURCES:%.c=$(BUILD_DIR)/%.o)

CFLAGS := -mthumb -mcpu=cortex-m3 -nostartfiles -std=gnu17
CFLAGS += -Wall -Wextra
CFLAGS += -O0 -g3
# Not -Werror: unlike freertos-labs/ (a pinned, unmodified vendor
# snapshot), this code is maintained across course years and should
# keep building as toolchains evolve, without silently breaking on a
# newly-strict warning turning into a hard error.

LDFLAGS := -T scripts/mps2_m3.ld
LDFLAGS += -Xlinker -Map=$(BUILD_DIR)/output.map

.PHONY: all clean run

all: $(BIN)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(BIN)

run: $(BIN)
	qemu-system-arm -machine mps2-an385 -monitor null -semihosting \
	    --semihosting-config enable=on,target=native \
	    -kernel $(BIN) -serial stdio -nographic

clean:
	rm -rf $(BUILD_DIR)
