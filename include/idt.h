#ifndef VEIL_IDT_H
#define VEIL_IDT_H

#include <stdint.h>

/* Register frame as laid down by boot/isr.s: pusha order (lowest address
 * first = edi), then our pushed vector/err, then the CPU's iretd frame.
 * No SS/ESP fields: ring-0 interrupts push neither (no privilege change). */
struct regs {
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags;
};

void idt_init(void);
/* Intentional divide-by-zero: the acceptance demo. Kept unused on purpose
 * (call it from kmain to watch the panic printer, or from the M8 shell's
 * `panic` builtin — which is its real home). */
void panic_test(void);

#endif
