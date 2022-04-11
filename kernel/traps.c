#include <sys/types.h>
#include <asm/system.h>


extern void divide_error_0(void);
extern void debug_1(void);
extern void nmi_2(void);
extern void int3_3(void);
extern void overflow_4(void);
extern void bounds_5(void);
extern void invalid_op_6(void);
extern void device_not_available_7(void);
extern void double_fault_8(void);
extern void invalid_TSS_10(void);
extern void segment_not_present_11(void);
extern void stack_segment_error_12(void);
extern void general_protection_13(void);
extern void page_fault_14(void);
extern void coprocessor_error_16(void);

extern uint64_t idt;
extern uint64_t gdt;

void print_info_and_exit() 
{
	// exit(11);
}


void do_divide_error_0()
{
	print_info_and_exit();	
}

void do_debug_1()
{
	print_info_and_exit();
}

void do_nmi_2()
{
	print_info_and_exit();
}

void do_int3_3()
{
	// print some info and return
}

void do_overflow_4()
{
	print_info_and_exit();
}

void do_bounds_5()
{
	print_info_and_exit();
}

void do_invalid_op_6()
{
	print_info_and_exit();
}

void do_device_not_available_7()
{
	print_info_and_exit();
}

void do_double_fault_8()
{
	print_info_and_exit();
}

void do_invalid_TSS_10()
{
	print_info_and_exit();
}

void do_segment_not_present_11()
{
	print_info_and_exit();
}

void do_stack_segment_error_12()
{
	print_info_and_exit();
}

void do_general_protection_13()
{
	print_info_and_exit();
}

void do_page_fault_14()
{
	print_info_and_exit();
}

void do_coprocessor_error_16()
{
	print_info_and_exit();
}


void set_gate(uint64_t *idt_table, uint8_t index, enum GATE_TYPE type, uint16_t code_seg_sel, void (*handler_addr)(void), uint8_t dpl)
{
	volatile uint64_t tmp;
	uint32_t *p;
	p = (uint32_t*)(&tmp);

	
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
	set_gate(&idt, 0, TRAP_GATE, KERNEL_CS, &divide_error_0, DPL_0);
	set_gate(&idt, 1, TRAP_GATE, KERNEL_CS, &debug_1, DPL_0);
	set_gate(&idt, 2, TRAP_GATE, KERNEL_CS, &nmi_2, DPL_0);
	set_gate(&idt, 3, TRAP_GATE, KERNEL_CS, &int3_3, DPL_3);		// dpl=3
	set_gate(&idt, 4, TRAP_GATE, KERNEL_CS, &overflow_4, DPL_3);	// dpl=3
	set_gate(&idt, 5, TRAP_GATE, KERNEL_CS, &bounds_5, DPL_3);		// dpl=3 32bit only
	set_gate(&idt, 6, TRAP_GATE, KERNEL_CS, &invalid_op_6, DPL_0);
	set_gate(&idt, 7, TRAP_GATE, KERNEL_CS, &device_not_available_7, DPL_0);
	set_gate(&idt, 8, TRAP_GATE, KERNEL_CS, &double_fault_8, DPL_0);
	// 9 is not necessary for recent processor
	set_gate(&idt, 10, TRAP_GATE, KERNEL_CS, &invalid_TSS_10, DPL_0);
	set_gate(&idt, 11, TRAP_GATE, KERNEL_CS, &segment_not_present_11, DPL_0);
	set_gate(&idt, 12, TRAP_GATE, KERNEL_CS, &stack_segment_error_12, DPL_0);
	set_gate(&idt, 13, TRAP_GATE, KERNEL_CS, &general_protection_13, DPL_0);
	set_gate(&idt, 14, TRAP_GATE, KERNEL_CS, &page_fault_14, DPL_0);
	// 15 is reserved by Intel
	set_gate(&idt, 16, TRAP_GATE, KERNEL_CS, &coprocessor_error_16, DPL_0);

	// todo enable x87 with write OCW1 to 8259
}





