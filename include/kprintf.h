#ifndef VEIL_KPRINTF_H
#define VEIL_KPRINTF_H

#include <stdarg.h>

/* Formats every call through both vga_putc and serial_putc — the serial
 * mirror is what makes every QEMU run a loggable transcript (plan rule). */
void kprintf(const char *fmt, ...);
void kvprintf(const char *fmt, va_list ap);

#endif
