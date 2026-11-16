#ifndef VEIL_GDT_H
#define VEIL_GDT_H

#include <stdint.h>

/* The GDT we install ourselves: GRUB's tables are fine for its own handoff
 * but their layout is unspecified — v1 reloads flat segments we control so
 * M4's interrupt gates and (later) user segments have a known home. */
void gdt_init(void);

/* selector values: index << 3 (low 3 bits are TI + RPL, both 0 in ring 0).
 * User selectors (ring 3) exist in the table for post-v1; nothing uses
 * them yet. */
#define SEL_KCODE 0x08
#define SEL_KDATA 0x10
#define SEL_UCODE 0x18
#define SEL_UDATA 0x20
#define SEL_TSS   0x28

#endif
