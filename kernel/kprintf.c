/* kprintf.c — minimal formatted print (M2).
 *
 * No libc in a freestanding kernel, so this is a hand-rolled subset:
 * %c %s %d %u %x %p plus %% (the plan asked for %c %s %d %x %p; %u is the
 * same path as %d minus one test, and %% is mandatory once % exists).
 * Widths + zero-padding arrived with M4 (register dumps are unreadable
 * unaligned); right-justify and '+' stay out until something needs them.
 * Every character goes to both consoles — that mirroring is the point. */

#include "kprintf.h"
#include "vga.h"
#include "serial.h"
#include <stddef.h>

static void emit(char c)
{
	vga_putc(c);
	serial_putc(c);
}

static void emit_str(const char *s)
{
	if (!s)
		s = "(null)"; /* kernel pointers lie all the time; crash-safe wins */
	while (*s)
		emit(*s++);
}

/* 64-bit unsigned into `base`, right-aligned in a 24-byte scratch buffer.
 * long long because %p hands us addresses and %d callers may want 64-bit. */
static void emit_num(unsigned long long n, int base, int width, int zeropad)
{
	char buf[24];
	int i = (int)sizeof(buf);
	const char *digits = "0123456789abcdef";

	do {
		buf[--i] = digits[n % (unsigned)base];
		n /= (unsigned)base;
	} while (n);

	/* pad before emitting: '0' flag packs zeros left of the digits,
	 * plain width right-justifies with spaces */
	for (int pad = width - ((int)sizeof(buf) - i); pad > 0; pad--)
		emit(zeropad ? '0' : ' ');

	while (i < (int)sizeof(buf))
		emit(buf[i++]);
}

void kvprintf(const char *fmt, va_list ap)
{
	for (const char *f = fmt; *f; f++) {
		if (*f != '%') {
			emit(*f);
			continue;
		}
		f++; /* past '%': consume optional 0-flag + decimal width */
		int zeropad = 0, width = 0;
		if (*f == '0') {
			zeropad = 1;
			f++;
		}
		while (*f >= '1' && *f <= '9') {
			width = width * 10 + (*f - '0');
			f++;
		}
		switch (*f) {
		case 'c': emit((char)va_arg(ap, int)); break;
		case 's': emit_str(va_arg(ap, const char *)); break;
		case 'd': {
			/* negate into unsigned first: -INT_MIN stays correct here,
			 * and flipping a flag inside emit_num wrapped negatives
			 * to giant decimals — caught by the -42 self-check line. */
			int v = va_arg(ap, int);
			if (v < 0) {
				emit('-');
				emit_num((unsigned long long)-(long long)v, 10, width, zeropad);
			} else {
				emit_num((unsigned long long)v, 10, width, zeropad);
			}
			break;
		}
		case 'u': emit_num(va_arg(ap, unsigned), 10, width, zeropad); break;
		case 'x': emit_num(va_arg(ap, unsigned), 16, width, zeropad); break;
		case 'p': emit_str("0x");
			  emit_num((unsigned long)(uintptr_t)va_arg(ap, void *), 16, width, zeropad);
			  break;
		case '%': emit('%'); break;
		default:  emit('%'); emit(*f); break; /* unknown spec: print it literally */
		}
	}
}

void kprintf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	kvprintf(fmt, ap);
	va_end(ap);
}
