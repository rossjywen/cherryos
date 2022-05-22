#include <sys/types.h>
#include <asm/system.h>
#include <linux/head.h>
#include <linux/kernel.h>


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

void print_context_info(uint32_t *esp, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags)
{
	printk("eflags %#x\n", eflags);
	printk("cs %#x\n", cs);
	printk("eip %#x\n", eip);
	//printk("\n", );
	//printk("\n", );
	//printk("\n", );
	//printk("\n", );
	//printk("\n", );
	//printk("\n", );
	//printk("\n", );
	//printk("\n", );
	//printk("\n", );
}

void do_divide_error_0(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)
{
	printk("divide error\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}

void do_debug_1(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	printk("debug exception\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void do_nmi_2(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	printk("nmi interrupt\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void do_int3_3(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	// print some info and return
	printk("int3 exception\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void do_overflow_4(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	printk("overflow exception\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void do_bounds_5(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	printk("bounds exception\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void do_invalid_op_6(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	printk("invalid op exception\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void do_device_not_available_7(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	printk("devicr not available exception\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void do_double_fault_8(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	printk("double fault\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void do_invalid_TSS_10(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	printk("invalid TSS error\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void do_segment_not_present_11(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	printk("segment not present exception\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void do_stack_segment_error_12(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	printk("stack segment error\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void do_general_protection_13(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	printk("general protection\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void do_page_fault_14(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	printk("page fault\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void do_coprocessor_error_16(uint32_t *esp_ptr, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)

{
	printk("coprocessor error\n");
	print_context_info(esp_ptr, error_code, fs, es, ds, ebp, esi, edi, edx, ecx, ebx, eax, eip, cs, eflags);	
	//exit(11);
}


void set_gate(struct seg_desc *idt_table, uint8_t index, enum GATE_TYPE type, uint16_t code_seg_sel, void (*handler_addr)(void), uint8_t dpl)
{
	uint32_t low = 0;
	uint32_t high = 0;


	low = code_seg_sel << 16 /* Segment Selector */| \
				((uint32_t)handler_addr & 0x0000FFFF) /* Offset[15:0] */;
	
	high = ((uint32_t)handler_addr & 0xFFFF0000) /* Offset[31:16] */ \
				| (1 << 15) /* P=1 */ \
				| (dpl << 13) /* DPL */ \
				| (((uint8_t)type) << 8) /* bit11-bit8 int:0b1110 trap:0b1111 others are all 0 */;

	//printk("set_gate() low %#x\n", low);
	//printk("set_gate() high %#x\n", high);

	idt_table[index].low_32 = low;
	idt_table[index].high_32 = high;
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
	// 15 is reserved by Intel
	set_gate(&idt, 16, TRAP_GATE, KERNEL_CS, &coprocessor_error_16, DPL_0);

	// todo enable x87 with write OCW1 to 8259
	// todo enable coprocessor interrupt
	//set_trap_gate(45,&irq13);								// 调用了浮点错误的服务		判断是否是数学处理器造成的 是的话死机
	//outb_p(inb_p(0x21)&0xfb,0x21);
	//outb(inb_p(0xA1)&0xdf,0xA1);
	// todo enable parallel port interrupt
	//set_trap_gate(39,&parallel_interrupt);
}





