#ifndef SYSTEM_H
#define SYSTEM_H

#include <sys/types.h>
#include <linux/head.h>

#define KERNEL_CS	0x0008
#define KERNEL_DS	0x0010
#define KERNEL_SS	0x0010

#define DPL_0	0
#define DPL_3	3


#define enable_interrupt() asm("sti"::)
#define disable_interrupt() asm("cli"::)

#define GET_CS() \
({ \
	uint16_t _cs = 0; \
	asm("mov %%cs, %%ax;":"=a"(_cs):); \
	_cs; \
})


#define GET_FS() \
({ \
	uint16_t _fs = 0; \
	asm("mov %%cs, %%ax;":"=a"(_fs):); \
	_cs; \
})


#define GET_CPL() (GET_CS() & 0x3)

#define get_seg_byte(seg,addr) \
({ \
	uint8_t __res; \
	__asm__("push %%fs;mov %%ax,%%fs;movb %%fs:%2,%%al;pop %%fs" \
	:"=a" (__res):"0" (seg),"m" (*(addr))); \
	__res; \
})

#define get_seg_dword(seg,addr) \
({ \
	uint32_t __res; \
	__asm__("push %%fs;mov %%ax,%%fs;movl %%fs:%2,%%eax;pop %%fs" \
	:"=a" (__res):"0" (seg),"m" (*(addr))); \
	__res; \
})

enum GATE_TYPE {INTERRUPT_GATE=0xE, TRAP_GATE=0xF};

void set_gate(struct seg_desc *idt_table, uint8_t index, enum GATE_TYPE type, uint16_t code_seg_sel, void (*handler_addr)(void), uint8_t dpl);

void set_tss_desc(struct seg_desc *gdt_table, uint32_t index, uint32_t tss_data_addr, uint8_t dpl);
void set_ldt_desc(struct seg_desc *gdt_table, uint32_t index, uint32_t ldt_data_addr, uint8_t dpl);

#endif //SYSTEM_H

