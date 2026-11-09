/* Veil kernel entry. M1 poked VGA memory by hand; from M2 everything goes
 * through kprintf, which mirrors to the screen AND COM1 (serial.c). */

#include "vga.h"
#include "serial.h"
#include "kprintf.h"

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

	for (;;)
		__asm__ volatile("hlt"); /* no interrupts yet: hlt parks forever */
}
