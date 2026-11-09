; Veil multiboot v1 header + entry point.
; GRUB (or QEMU -kernel) scans the first 8 KiB for this header, then loads
; the ELF at its linked VMA with protected mode + flat segments already on.
; So _start only needs a stack before C code is safe to call.

MBOOT_PAGE_ALIGN equ 1 << 0    ; align modules on page boundaries
MBOOT_MEM_INFO   equ 1 << 1    ; request mem_lower/mem_upper + memory map
MBOOT_FLAGS      equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_MAGIC      equ 0x1BADB002
MBOOT_CHECKSUM   equ -(MBOOT_MAGIC + MBOOT_FLAGS)

section .multiboot
align 4
    dd MBOOT_MAGIC
    dd MBOOT_FLAGS
    dd MBOOT_CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384                 ; 16 KiB is plenty for v1
stack_top:

section .text
global _start
extern kmain

_start:
    mov esp, stack_top
    ; multiboot leaves eax = 0x2BADB002 magic, ebx = info struct pointer.
    ; Only M9's memory-map parser needs them; retrieving them then means
    ; re-reading via a saved pointer, so we deliberately pass nothing now.
    call kmain
    ; kmain never returns; if it ever does, park instead of executing
    ; whatever garbage follows.
.hang:
    cli
    hlt
    jmp .hang
