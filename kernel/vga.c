/* vga.c — text-mode console driver (M2).
 *
 * Text mode is a 80x25 grid of 16-bit cells at 0xB8000: low byte ASCII,
 * high byte attribute (bg<<4|fg). We keep our own cursor + default color
 * and scroll by copying rows up when the cursor runs past the last line —
 * hardware text mode has no scroll register, the buffer *is* the display. */

#include "vga.h"
#include "io.h"

#define VGA_COLS 80
#define VGA_ROWS 25
#define VGA_BASE 0xB8000

static volatile uint16_t *const vga = (volatile uint16_t *)VGA_BASE;

/* CRT controller regs 0x0F/0x10 (ports 0x3D4/0x3D5) set the hardware cursor
 * to a linear cell offset; without this the caret sits frozen at 0. */
#define CRT_ADDR  0x3D4
#define CRT_DATA  0x3D5
#define CURSOR_HI 0x0F
#define CURSOR_LO 0x0E

static int cursor;
static uint8_t attr = 0x0F; /* default light grey on black */

static void update_cursor(void)
{
	outb(CRT_ADDR, CURSOR_HI);
	outb(CRT_DATA, (uint8_t)(cursor >> 8));
	outb(CRT_ADDR, CURSOR_LO);
	outb(CRT_DATA, (uint8_t)(cursor & 0xFF));
}

static void scroll(void)
{
	for (int i = 0; i < VGA_COLS * (VGA_ROWS - 1); i++)
		vga[i] = vga[i + VGA_COLS];
	for (int i = VGA_COLS * (VGA_ROWS - 1); i < VGA_COLS * VGA_ROWS; i++)
		vga[i] = ' ' | (attr << 8);
	cursor = VGA_COLS * (VGA_ROWS - 1);
}

void vga_clear(void)
{
	for (int i = 0; i < VGA_COLS * VGA_ROWS; i++)
		vga[i] = ' ' | (attr << 8);
	cursor = 0;
	update_cursor();
}

void vga_set_color(uint8_t fg, uint8_t bg)
{
	attr = (uint8_t)((bg << 4) | (fg & 0x0F));
}

void vga_putc(char c)
{
	if (c == '\n') {
		/* newline = next row at column 0 (like a real terminal),
		 * not cursor+80 which kept the old column */
		cursor = (cursor / VGA_COLS + 1) * VGA_COLS;
		if (cursor >= VGA_COLS * VGA_ROWS)
			scroll();
	} else if (c == '\b') {
		if (cursor > 0) {
			cursor--;
			vga[cursor] = ' ' | (attr << 8);
		}
	} else {
		vga[cursor++] = (uint8_t)c | (attr << 8);
		if (cursor >= VGA_COLS * VGA_ROWS)
			scroll();
	}
	update_cursor();
}
