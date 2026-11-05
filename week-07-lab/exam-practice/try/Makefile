# Embedded Operating Systems and Architecture course
# Licensed under the MIT License. See LICENSE in the repository root.
#
# Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>

# A convenience to try a practice question's code. It is not part of any
# question and nothing here checks an answer.
#
#   make run Q=q02         questions with an answer.c (Q1, Q2, Q5, Q6):
#                          builds your answer.c with play/q02.c and runs
#                          it in QEMU; stop it with Ctrl-C
#   make run Q=q03         questions on the kernel (Q3, Q4, Q7): builds
#                          the kernel of week 5 with the files of the
#                          question's kernel/ and src/ folders laid over
#                          it, and runs it in QEMU, as in week 5
#   make clean             removes build/

# ARM bare-metal cross-compiler: runs on the host, emits Cortex-M code.
CC := arm-none-eabi-gcc

# The folder of the question: ../q02-rta for Q=q02.
QDIR := $(firstword $(wildcard ../$(Q)-*))

.PHONY: all run clean

ifeq ($(Q),)
all run:
	@echo "usage: make run Q=q02 (one of q01 to q07)"; false
else ifeq ($(QDIR),)
all run:
	@echo "no question folder ../$(Q)-*"; false
else ifneq ($(wildcard $(QDIR)/answer.c),)
# --- A question with an answer.c: your file, a play/ main, the runtime. ---

# Same flags as the kernel's Makefile (weeks 1 to 5): Thumb code for the
# Cortex-M3, no C library, no start files (runtime/startup.c is ours),
# warnings on, no optimisation and debug information so that gdb follows
# the C. play/qNN.c includes your answer.c, found through -I.
CFLAGS := -mthumb -mcpu=cortex-m3 -nostartfiles -ffreestanding -fno-builtin
CFLAGS += -std=gnu17 -Wall -Wextra -O0 -g3 -Iruntime -I$(QDIR)
LDFLAGS := -T runtime/common.ld -nostdlib -lgcc

SOURCES := play/$(Q).c runtime/startup.c runtime/console.c runtime/format.c
BIN := build/$(Q).axf

all: $(BIN)

$(BIN): $(SOURCES) $(QDIR)/answer.c $(wildcard runtime/*.h) runtime/common.ld
	@mkdir -p build
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $(BIN)

# The program never ends by itself: quit with Ctrl-C. The same QEMU line
# as `make run` in the kernel: UART0 on this terminal.
run: $(BIN)
	qemu-system-arm -machine mps2-an385 -monitor null -nographic -serial stdio -kernel $(BIN)
else
# --- A kernel question: week 5's kernel with the question's files. ---

# The kernel you start from, and the folder where it is built.
BASE := ../../../week-05/solution
WORK := build/$(Q)

# Every file of the question's kernel/ and src/ folders, as paths
# relative to the question folder: kernel/task.c, src/task_b.c, ...
FILES := $(patsubst $(QDIR)/%,%,$(wildcard $(QDIR)/kernel/* $(QDIR)/src/*))

# Copy the kernel, then the question's files over its own (a file the
# kernel does not have is added), and build there. The copy is made again
# whenever one of the files changes.
all: $(WORK)/.synced
	$(MAKE) -C $(WORK)

$(WORK)/.synced: $(addprefix $(QDIR)/,$(FILES))
	mkdir -p $(WORK)
	cd $(BASE) && tar cf - --exclude=build . | tar xf - -C $(CURDIR)/$(WORK)
	cd $(QDIR) && tar cf - $(FILES) | tar xf - -C $(CURDIR)/$(WORK)
	touch $@

# `make run Q=q03 QUANTUM=2` passes the setting to the kernel's Makefile.
run: all
	$(MAKE) -C $(WORK) run
endif

clean:
	rm -rf build
