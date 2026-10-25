# Veil

A small 32-bit x86 operating system kernel, written from scratch in C and NASM, one evening at a time.

Veil is a learning project: every line is hand-written and commented with the *why*, not just the *what*. The goal of v1 is a bootable kernel that drops you into an interactive shell — multiboot header, GDT, IDT, hardware interrupts, a PIT timer, a PS/2 keyboard driver, and `veilsh`.

## Status

Tracking in [milestone issues](../../issues) — boot → shell.

## Quick start (Arch Linux)

```bash
paru -S nasm qemu-full i686-elf-binutils i686-elf-gcc   # toolchain
make run          # boots Veil in QEMU (multiboot ELF via -kernel)
```

## Layout

```
boot/      NASM: multiboot header, _start, interrupt stubs
kernel/    C: kmain, VGA console, serial, kprintf, GDT, IDT
drivers/   C: PIC, PIT, PS/2 keyboard
shell/     C: readline, veilsh
mm/        C: physical frames + kernel heap
```

## How it runs

Veil is a multiboot 1 ELF. During development QEMU loads it directly:

```bash
qemu-system-i386 -kernel build/veil.elf -serial stdio -no-reboot
```

`kprintf` mirrors to COM1, so every run leaves a loggable transcript. For hard bugs: `-S -s` and `gdb build/veil.elf`.

## Notes

- Educational use only. Not secure, not fast, not complete — and deliberately so.
- Cross-compiler: `i686-elf-*` (AUR packages on Arch).
- Post-v1 ideas (paging, multitasking, userland) are parked; see the milestone list.
