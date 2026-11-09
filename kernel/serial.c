/* serial.c — 16550 UART on COM1 (M2).
 *
 * The plan's "every run leaves a transcript" rule: QEMU -serial stdio shows
 * exactly what the kernel writes here. COM1 lives at 0x3F8; registers are
 * banked behind a DLAB bit, so baud-rate writes need the bit set and the
 * data/FIFO writes need it clear. 115200 baud = divisor 1. */

#include "serial.h"
#include "io.h"

#define COM1 0x3F8

static int serial_selftest_ok = 1;

static void serial_putc_raw(char c)
{
	/* line status bit 5 = transmitter holding register empty */
	while (!(inb(COM1 + 5) & 0x20))
		;
	outb(COM1, (uint8_t)c);
}

void serial_init(void)
{
	outb(COM1 + 1, 0x00); /* disable interrupts while we reconfigure */
	outb(COM1 + 3, 0x80); /* DLAB on: now divisors are addressable */
	outb(COM1 + 0, 0x01); /* divisor 1 -> 115200 (baud clock / divisor) */
	outb(COM1 + 1, 0x00);
	outb(COM1 + 3, 0x03); /* DLAB off; 8 data bits, no parity, 1 stop */
	outb(COM1 + 2, 0xC7); /* enable FIFOs, clear them, 14-byte trigger */
	outb(COM1 + 4, 0x0B); /* DTR + RTS + out1: modem control for QEMU */
	/* self-test: loop the transmitter into the receiver (MCR bits 7+6)
	 * and see if a byte round-trips. Informational only — QEMU's 16550
	 * fails the wiki's MSR check, and returning early would strand the
	 * UART in loopback: TX vanishes internally, serial looks dead. */
	outb(COM1 + 4, 0x1E);
	outb(COM1, 0xAE);
	if (inb(COM1 + 6) != 0x01)
		serial_selftest_ok = 0;
	outb(COM1 + 4, 0x0F); /* ALWAYS leave loopback: the bug that ate M2 */
}

void serial_putc(char c)
{
	if (c == '\n')
		serial_putc_raw('\r'); /* real terminals want CRLF */
	serial_putc_raw(c);
}
