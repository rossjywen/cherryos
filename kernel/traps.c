#include <sys/types.h>
#include <asm/system.h>


extern void divide_zero(void);

extern uint64_t idt;
extern uint64_t gdt;

void print_info_and_exit() 
{
	// exit(11);
}


void do_divide_zero()
{
	print_info_and_exit();
}


void set_gate(uint64_t *idt_table, uint8_t index, enum GATE_TYPE type, uint16_t code_seg_sel, void (*handler_addr)(void), uint8_t dpl)
{
	volatile uint64_t tmp;
	uint32_t *p;
	p = &tmp;

	
	*p = code_seg_sel << 16 /* Segment Selector */| \
				((uint32_t)handler_addr & 0x0000FFFF) /* Offset[15:0] */;
	
	*(p + 1) = ((uint32_t)handler_addr & 0xFFFF0000) /* Offset[31:16] */ \
				| (1 << 15) /* P=1 */ \
				| (dpl << 13) /* DPL */ \
				| (((uint8_t)type) << 8) /* bit11-bit8 int:0b1110 trap:0b1111 others are all 0 */;

	idt_table[index] = tmp;
}




void trap_init(void)
{
	(&idt)[0] = 0xffff;
	set_gate(&idt, 0, TRAP_GATE, KERNEL_CS, &divide_zero, 0);
}





