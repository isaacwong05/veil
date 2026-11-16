/* Veil IDT + panic printer (M4).
 * 32 exception gates point at boot/isr.s stubs; any fault reaching C ends
 * in panic(): white-on-red dump of the whole register frame, then cli+hlt.
 * Until M5's PIC remap, hardware IRQs share vectors 0-15 with the CPU
 * exceptions (default PIC mapping) — so both PICs are fully MASKED before
 * we enable interrupts. A masked PIC can't fire; IF=1 only arms CPU
 * exceptions, which is exactly what this milestone can prove. */

#include <stdint.h>
#include "idt.h"
#include "io.h"
#include "kprintf.h"
#include "vga.h"
#include "gdt.h"

/* 256 gates, but v1 fills 0-31. Unfilled gates are absent (P=0): invoking
 * them raises #GP instead of jumping into garbage — a good default anyway. */
struct idt_entry {
	uint16_t base_low;
	uint16_t selector;
	uint8_t  zero;
	uint8_t  type_attr;   /* 0x8E = present, DPL0, 32-bit interrupt gate */
	uint16_t base_high;
} __attribute__((packed));

static struct idt_entry idt[256];

static const char *exc_name[32] = {
	"Division by zero (#DE)", "Debug (#DB)", "Non-maskable interrupt",
	"Breakpoint (#BP)", "Overflow (#OF)", "Bound range exceeded (#BR)",
	"Invalid opcode (#UD)", "Device not available (#NM)",
	"Double fault (#DF)", "Coprocessor segment overrun (reserved)",
	"Invalid TSS (#TS)", "Segment not present (#NP)",
	"Stack-segment fault (#SS)", "General protection fault (#GP)",
	"Page fault (#PF)", "x87 FP error (#MF)",
	"Alignment check (#AC)", "Machine check (#MC)",
	"SIMD FP exception (#XM)", "Virtualization exception (#VE)",
	"Control protection exception (#CP)", "reserved", "reserved",
	"reserved", "reserved", "reserved", "reserved", "reserved",
	"reserved", "Reserved", "Security exception (#SX)",
	"VMware VMCALL hypervisor",
};

/* Address table emitted by boot/isr.s: isr_table[i] is the stub for
 * vector i. Arrays of function pointers and asm labels are the same
 * thing to the linker — this is the whole glue. */
extern uint32_t isr_table[32];

void idt_init(void)
{
	for (int i = 0; i < 32; i++) {
		uint32_t base = isr_table[i];
		idt[i].base_low  = base & 0xFFFF;
		idt[i].base_high = base >> 16;
		idt[i].selector  = SEL_KCODE;  /* our GDT from M3 — the payoff */
		idt[i].zero      = 0;
		idt[i].type_attr = 0x8E;
	}
	/* selector for unused gates: 0 (null) with P=0 — never loaded. */

	struct __attribute__((packed)) {
		uint16_t limit;
		uint32_t base;
	} idtr = { sizeof(idt) - 1, (uint32_t)idt };

	/* PIC mask BEFORE sti — see file header for why unmapped IRQs would
	 * corrupt exception vectors. Full remap + unmasking is M5's job. */
	outb(0x21, 0xFF);   /* master: IRQ0-7 all masked */
	outb(0xA1, 0xFF);   /* slave:  IRQ8-15 all masked */

	__asm__ volatile("lidt %0" :: "m"(idtr));
	__asm__ volatile("sti");

	kprintf(" IDT loaded: 32 exception gates, PIC masked, interrupts ON\n");
}

void isr_handler(struct regs *r)
{
	vga_set_color(VGA_WHITE, VGA_RED);
	kprintf("\n PANIC: ");
	if (r->int_no < 32)
		kprintf("%s", exc_name[r->int_no]);
	else
		kprintf("interrupt %u", r->int_no);
	kprintf("\n    vector=0x%02x err=0x%08x eip=0x%08x cs=0x%04x eflags=0x%08x\n",
		r->int_no, r->err_code, r->eip, r->cs, r->eflags);
	kprintf("    eax=0x%08x ebx=0x%08x ecx=0x%08x edx=0x%08x\n",
		r->eax, r->ebx, r->ecx, r->edx);
	kprintf("    esi=0x%08x edi=0x%08x ebp=0x%08x esp=0x%08x\n",
		r->esi, r->edi, r->ebp, r->esp);
	kprintf("    (halted; no panic-recovery until reboot)\n");

	for (;;) {
		__asm__ volatile("cli; hlt");
	}
}

void panic_test(void)
{
	/* volatile so -O2 can't fold the divide away — the point is a REAL
	 * idiv by zero at runtime, i.e. a genuine #DE from the CPU. */
	volatile int zero = 0;
	volatile int out;
	out = 42 / zero;
	(void)out;
}
