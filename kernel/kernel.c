/* Veil kernel entry. M1 prints a banner straight into VGA text memory;
 * the scrolling console (vga.c), serial mirror and kprintf arrive in M2. */

#include <stdint.h>

/* Hardware text mode: an 80x25 grid mapped at 0xB8000. Each cell is one
 * 16-bit word: low byte ASCII, high byte attribute (background<<4|foreground).
 * The buffer is memory-mapped RAM, not our RAM, hence volatile. */
#define VGA_BASE 0xB8000
#define COLS     80

static volatile uint16_t *const vga = (volatile uint16_t *)VGA_BASE;

static uint16_t cell(char c, uint8_t fg, uint8_t bg)
{
    return (uint16_t)(uint8_t)c | (uint16_t)((bg << 4) | (fg & 0x0F)) << 8;
}

static void putline(int row, const char *s, uint8_t fg, uint8_t bg)
{
    for (int col = 0; s[col] != '\0'; col++)
        vga[row * COLS + col] = cell(s[col], fg, bg);
}

void kmain(void)
{
    putline(0, " Veil v0.1 booting", 0x0F, 0x00); /* white on black */
    putline(1, " multiboot handoff OK", 0x0A, 0x00); /* light green   */
    for (;;)
        __asm__ volatile ("hlt"); /* no interrupts yet:hlt parks forever */
}
