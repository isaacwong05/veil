/* Veil console — vga.c, serial.c, kprintf.c (M2).
 *
 * Layout note: these headers live in include/ so kernel/, drivers/ and
 * shell/ all see the same declarations with -Iinclude. */

#ifndef VEIL_VGA_H
#define VEIL_VGA_H

#include <stdint.h>

/* VGA text attribute = background << 4 | foreground (low nibble may blink). */
enum vga_color {
	VGA_BLACK = 0, VGA_BLUE, VGA_GREEN, VGA_CYAN, VGA_RED,
	VGA_MAGENTA, VGA_BROWN, VGA_GRAY, VGA_DGRAY, VGA_LBLUE,
	VGA_LGREEN, VGA_LCYAN, VGA_LRED, VGA_LMAGENTA, VGA_YELLOW, VGA_WHITE,
};

void vga_clear(void);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_putc(char c);

#endif
