/* Veil kernel entry. M1 poked VGA memory by hand; from M2 everything goes
 * through kprintf, which mirrors to the screen AND COM1 (serial.c). */

#include "vga.h"
#include "serial.h"
#include "kprintf.h"
#include "gdt.h"
#include "idt.h"

void kmain(void)
{
	serial_init();
	vga_clear();

	vga_set_color(VGA_WHITE, VGA_BLACK);
	kprintf(" Veil v0.1 booting\n");
	vga_set_color(VGA_LGREEN, VGA_BLACK);
	kprintf(" multiboot handoff OK\n");
	vga_set_color(VGA_GRAY, VGA_BLACK);
	kprintf(" console check: %s | int %d | hex 0x%x | ptr %p | char %c | %% done\n",
		"kprintf", -42, 0xDEADBEEF, (void *)0x101000, 'A');
	kprintf(" serial line visible ONLY on COM1? no — mirrored on both.\n");

	/* GDT after the banner on purpose: if the reload kills boot, the
	 * surviving banner lines tell us exactly where it died. */
	vga_set_color(VGA_LCYAN, VGA_BLACK);
	gdt_init();
	vga_set_color(VGA_GRAY, VGA_BLACK);

	idt_init();

	for (;;)
		__asm__ volatile("hlt"); /* interrupts ON since M4; nothing can wake us (PIC masked) — M5's PIT tick changes that */
}
