#ifndef SYSTEM_H
#define SYSTEM_H

#include <sys/types.h>

#define KERNEL_CS	0x0008
#define KERNEL_DS	0x0010
#define KERNEL_SS	0x0010

#define DPL_0	0
#define DPL_3	3


#define enable_interrupt() asm("sti"::)
#define disable_interrupt() asm("cli"::)


enum GATE_TYPE {INTERRUPT_GATE=0xE, TRAP_GATE=0xF};

void set_gate(uint64_t *idt_table, uint8_t index, enum GATE_TYPE type, uint16_t code_seg_sel, void (*handler_addr)(void), uint8_t dpl);


#endif //SYSTEM_H

