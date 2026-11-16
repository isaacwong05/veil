/* Veil GDT — why own it when GRUB already set one up?
 * Multiboot only *guarantees* that some GDT exists; its size, layout and
 * entry order are unspecified, so every selector we'd hand to the IDT in M4
 * would be an undocumented assumption. Installing our own flat table makes
 * the segment layout a fact we wrote, not a favor we accepted. */

#include <stdint.h>
#include "gdt.h"
#include "kprintf.h"

/* One 8-byte segment descriptor. x86 splits both base and limit across
 * non-adjacent bytes, so the field order below is the CPU's memory order,
 * not a natural C layout — only ever write entries through set_gate(). */
struct gdt_entry {
	uint16_t limit_low;   /* limit bits 0-15 */
	uint16_t base_low;    /* base bits 0-15 */
	uint8_t  base_mid;    /* base bits 16-23 */
	uint8_t  access;      /* P | DPL | S | Type */
	uint8_t  limit_high;  /* flags<<4 | limit bits 16-19 */
	uint8_t  base_high;   /* base bits 24-31 */
};

/* 6 entries: null (required), kernel code/data (flat 4 GiB), user code/data
 * (flat, ring 3 — unused until post-v1 userland, but present so selector
 * numbers never get reshuffled later), and a TSS slot (zeroed; loaded into
 * TR only when rings other than 0 exist). */
static struct gdt_entry gdt[6];

/* base=0, limit=0xFFFFF with 4 KiB granularity = flat 4 GiB: segmentation
 * maps 1:1 so addresses stay physical until paging arrives (M9+). */
static void set_gate(int idx, uint8_t access)
{
	struct gdt_entry *e = &gdt[idx];
	e->limit_low  = 0xFFFF;
	e->limit_high = 0xCF;   /* 4 KiB units + 32-bit protected mode */
	e->access     = access;
	e->base_low = 0;
	e->base_mid = 0;
	e->base_high = 0;
}

void gdt_init(void)
{
	/* access bytes: P|DPL|S|Type.
	 * 0x9A kernel code (ring 0, executable, readable)
	 * 0x92 kernel data (ring 0, writable)
	 * 0xFA / 0xF2: same but DPL=3 for user mode. */
	set_gate(1, 0x9A);
	set_gate(2, 0x92);
	set_gate(3, 0xFA);
	set_gate(4, 0xF2);
	/* entry 5 (TSS) left zeroed: a null descriptor is harmless unless
	 * something loads TR, which v1 never does. */

	/* lgdt reads a 6-byte pseudo-descriptor: 16-bit limit + 32-bit base.
	 * packed(1) is load-bearing — natural alignment pads after the
	 * uint16_t and the CPU would read a garbage base. */
	struct __attribute__((packed)) {
		uint16_t limit;
		uint32_t base;
	} gdtr = { sizeof(gdt) - 1, (uint32_t)gdt };

	__asm__ volatile("lgdt %0" :: "m"(gdtr) : "memory");

	/* Reload CS: CS can't be written with mov — only a far jump loads
	 * it. $1f is a local assembler label (gcc doesn't see it). */
	__asm__ volatile(
		"ljmp $0x08, $1f\n"
		"1:\n"
		"movw $0x10, %%ax\n"
		"movw %%ax, %%ds\n"
		"movw %%ax, %%es\n"
		"movw %%ax, %%fs\n"
		"movw %%ax, %%gs\n"
		"movw %%ax, %%ss"
		:: "a"((uint16_t)0) : "memory");

	/* Reaching here means the CPU accepted the new descriptors and every
	 * segment register now points into our table. */
	kprintf(" GDT loaded: %u entries (flat 4 GiB, kcode/kdata + user + TSS slot)\n",
		(unsigned)(sizeof(gdt) / sizeof(gdt[0])));
}
