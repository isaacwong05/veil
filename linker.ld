/* Link at 1 MiB: above the real-mode IVT/BDA and the VGA hole, low enough
 * that post-v1 paging can cover the kernel with one 4 MiB page — and the
 * conventional first free address for a multiboot kernel. */

ENTRY(_start)

SECTIONS
{
    . = 1M;

    .text : ALIGN(4K)
    {
        /* the multiboot header must land in the first 8 KiB GRUB scans */
        KEEP(*(.multiboot))
        *(.text*)
    }

    .rodata : ALIGN(4K) { *(.rodata*) }
    .data   : ALIGN(4K) { *(.data*) }
    .bss    : ALIGN(4K) { *(COMMON) *(.bss*) }
}
