#ifndef SYSTEM_H
#define SYSTEM_H

#include <sys/types.h>

#define KERNEL_CS	0x0008


#define enable_interrupt() asm("sti"::)
#define disable_interrupt() asm("cli"::)


enum GATE_TYPE {INTERRUPT_GATE=0xE, TRAP_GATE=0xF};

void set_gate(uint64_t *idt_table, uint8_t index, enum GATE_TYPE type, uint16_t code_seg_sel, void (*handler_addr)(void), uint8_t dpl);


//#define set_interrupt_gate(index, handler_addr) \
//			_set_gate(idt, index, INTERRUPT_GATE, KERNEL_CS, handler_addr, 0)
//
//#define set_trap_gate(index, handler_addr) \
//			_set_gate(idt, index, TRAP_GATE, KERNEL_CS, handler_addr, 0)


#endif //SYSTEM_H

