# Veil — cross-compiled, freestanding: no host libc anywhere.
CC      := i686-elf-gcc
NASM    := nasm
CFLAGS  := -ffreestanding -O2 -Wall -Wextra
LDFLAGS := -nostdlib
BUILD   := build

OBJS := $(BUILD)/boot.o $(BUILD)/kernel.o

.PHONY: all run debug clean

all: $(BUILD)/veil.elf

$(BUILD):
	mkdir -p $@

$(BUILD)/boot.o: boot/boot.s | $(BUILD)
	$(NASM) -f elf32 -o $@ $<

$(BUILD)/kernel.o: kernel/kernel.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/veil.elf: $(OBJS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -T linker.ld -o $@ $(OBJS)

# -no-reboot: a triple fault freezes QEMU instead of reboot-looping, so a
# boot failure stays observable (add -d int to see why it faulted).
run: all
	qemu-system-i386 -kernel $(BUILD)/veil.elf -serial stdio -no-reboot

debug: all
	qemu-system-i386 -kernel $(BUILD)/veil.elf -S -s -no-reboot

clean:
	rm -f $(BUILD)/*.o $(BUILD)/veil.elf
