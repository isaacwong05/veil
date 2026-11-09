#ifndef VEIL_IO_H
#define VEIL_IO_H

#include <stdint.h>

/* Port I/O inline in a header: every driver (serial now, PIC later) pokes
 * hardware ports, and static inline avoids per-call overhead and keeps the
 * compiler free to fold constants. */

static inline void outb(uint16_t port, uint8_t val)
{
	__asm__ volatile("outb %0, %1" :: "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t ret;
	__asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

/* wait ~1 microsecond: one dummy port write is enough for real hardware;
 * QEMU is far faster than the ISA bus this models, so it works there too. */
static inline void io_wait(void)
{
	outb(0x80, 0);
}

#endif
