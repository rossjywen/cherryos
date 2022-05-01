#include <linux/sched.h>
#include <sys/types.h>
#include <linux/head.h>


uint32_t printk(const char *fmt, ...);

void test_task0(void);


uint8_t task0_user_stack[PAGE_SIZE];

union task_union task0 = { 
	.task = 
	{ 
		.ldt = 
		{ 
			{.low_32 = 0, .high_32 = 0},			// null descriptor
			{.low_32 = 0xFFF, .high_32 = 0xC0FA00},
			{.low_32 = 0xFFF, .high_32 = 0xC0F200},
		}, 

		.tss = 
		{ 
			.pre_task_link = 0,					// 不会设置eflags.nt 所以不会用到
			.pre_task_link_pad = 0, 
			
			.esp0 = (uint32_t)(task0.stack + PAGE_SIZE - 1),	// 内核栈 esp0指向task_union内存最高点
			.ss0 = 0x10,										// 内核栈 ss0指向内核数据段
			.ss0_pad = 0,

			.esp1 = 0,							// CPL=1 CPL=2 的栈都忽略不使用
			.ss1 = 0,
			.ss1_pad = 0,

			.esp2 = 0,
			.ss2 = 0,
			.ss2_pad = 0,

			.cr3 =  (uint32_t)(&page_dir),		// 所有的任务都和kernel一样 用page_dir作为cr3的值
			
			.eip =  (uint32_t)test_task0,		// 我打算给指向一个函数

			.eflags = 0x00000206,	// 这是根据手册中eflag的初值设定的

			.eax = 0,		// 所有通用寄存器初始化为0
			.ecx = 0,
			.edx = 0,
			.ebx = 0,
			.esp = (uint32_t)(task0_user_stack) + PAGE_SIZE - 1,
			.ebp = 0,
			.esi = 0,
			.edi = 0,

			.es = 0x17,		// 指向ldt中的第2个seg_desc -> data segment
			.es_pad = 0,

			.cs = 0x0F,		// 指向ldt中的第1个seg_desc -> code segment
			.cs_pad = 0,

			.ss = 0x17,		// 指向ldt中的第2个seg_desc -> data segment
			.ss_pad = 0,

			.ds = 0x17,		// 指向ldt中的第2个seg_desc -> data segment
			.ds_pad = 0,

			.fs = 0x17,		// 不使用fs 指向GDT的null descriptor
			.fs_pad = 0,

			.gs = 0x17,		// 不使用fs 指向GDT的null descriptor
			.gs_pad = 0,

			.ldt = _LDT(0),
			.ldt_pad = 0,

			.debug_trap = 0,

			.io_map_base = 0,
		},
	},
};


struct task_struct *tasks_ptr[NUMBER_OF_TASKS] = {&task0.task, };

void set_tss_desc(struct seg_desc *gdt_table, uint32_t index, uint32_t tss_data_addr, uint8_t dpl)
{
	uint32_t low = 0;
	uint32_t high = 0;

/*
	[31:16] -> tss data address 15~0
	[15:0] -> segment limit 15~0
*/
	low = ((tss_data_addr & 0xFFFF) << 16) + 104;	// 104 is segment limit 
													// I do not intend to use more than 104 in tss data
													// so it means no IO-map is included

/* 
	[31:24] -> tss data address 31~24
	[23] -> G (0)
	[22:21] -> 0
	[20] -> AVL (0)
	[19:16] -> segment limit 19~16
	[15] -> P (1)
	[14:13] -> DPL 
	[12] -> 0
	[11:8] -> 0b1001(type)
	[7:0] -> tss data address 23~16
*/
	high = (((tss_data_addr & 0xFF000000) >> 24) << 24) | \
				(dpl << 13) | \
				(1 << 15) | \
				(0x9 << 8) | \
				((tss_data_addr & 0xFF0000) >> 16);

	printk("in set_tss_desc() low : %#x\n", low);
	printk("in set_tss_desc() high : %#x\n", high);

	gdt_table[index].low_32 = low;
	gdt_table[index].high_32 = high;
}



void set_ldt_desc(struct seg_desc *gdt_table, uint32_t index, uint32_t ldt_data_addr, uint8_t dpl)
{
	uint32_t low = 0;
	uint32_t high = 0;

/*
	[31:16] -> ldt data address 15~0
	[15:0] -> segment limit 15~0
*/
	low = ((ldt_data_addr & 0xFFFF) << 16) + 32;	// 32 is segment limit accomodating 4 entries
/* 
	[31:24] -> ldt data address 31~24
	[23] -> G (0)
	[22:21] -> 0
	[20] -> AVL (0)
	[19:16] -> segment limit 19~16
	[15] -> P (1)
	[14:13] -> DPL 
	[12] -> 0
	[11:8] -> 0b0010(type)
	[7:0] -> ldt data address 23~16
*/
	high = (((ldt_data_addr & 0xFF000000) >> 24) << 24) | \
				(dpl << 13) | \
				(1 << 15) | \
				(0x2 << 8) | \
				((ldt_data_addr & 0xFF0000) >> 16);

	gdt_table[index].low_32 = low;
	gdt_table[index].high_32 = high;
}



void sched_init(void)
{
	TASK_LOAD_TR(0);
	TASK_LOAD_LDTR(0);
}



void switch_to(uint32_t task_number)
{
	uint32_t eip = tasks_ptr[task_number]->tss.eip;
	uint32_t eflags = tasks_ptr[task_number]->tss.eflags;

	uint32_t eax = tasks_ptr[task_number]->tss.eax;
	uint32_t ecx = tasks_ptr[task_number]->tss.ecx;
	uint32_t edx = tasks_ptr[task_number]->tss.edx;
	uint32_t ebx = tasks_ptr[task_number]->tss.ebx;
	uint32_t esp = tasks_ptr[task_number]->tss.esp;
	uint32_t ebp = tasks_ptr[task_number]->tss.ebp;
	uint32_t esi = tasks_ptr[task_number]->tss.esi;
	uint32_t edi = tasks_ptr[task_number]->tss.edi;

	uint16_t es = tasks_ptr[task_number]->tss.es;
	uint16_t cs = tasks_ptr[task_number]->tss.cs;
	uint16_t ss = tasks_ptr[task_number]->tss.ss;
	uint16_t ds = tasks_ptr[task_number]->tss.ds;
	uint16_t fs = tasks_ptr[task_number]->tss.fs;
	uint16_t gs = tasks_ptr[task_number]->tss.gs;

/*
	1.切换到新任务的ldt和tr (通过宏调用)
	2.把ss esp eflags cs eip 按顺序入栈 构造一个上一次带有CPL切换效果的调用栈 为iret准备环境
	3.切换到新任务的 通用寄存器 其中ebp和eax先不改变而是入栈中 因为需要ebp来访问函数局部变量 需要eax做一些数据挪动
	4.切换到新任务的ds es fs 最后改变ds 因为从ds改变的一刻起就不能访问C中的变量了 但可以使用之前压入栈中的数据 因为这时候ss依然是指向前一个任务的内核栈
	5.分别从栈上取出新任务eax和ebp的值并更新到eax和ebp中
	6.iret
*/

	TASK_LOAD_TR(task_number);
	TASK_LOAD_LDTR(task_number);

/*
	%0	eax
	%1	ebx
	%2	ecx
	%3	edx
	%4	ebp
	%5	esi
	%6	edi

	%7	eflags
	
	%8	cs
	%9	ds
	%10	es
	%11	fs
	%12	gs
	%13	ss
	%14	eip
	%15	esp
*/
	asm("mov %1, %%ebx; \
		 mov %2, %%ecx; \
		 mov %3, %%edx; \
		 mov %5, %%esi; \
		 mov %6, %%edi; \
		 mov %13, %%eax; \
		 push %%eax; \
		 mov %15, %%eax; \
		 push %%eax; \
		 mov %7, %%eax; \
		 push %%eax; \
		 mov %8, %%eax; \
		 push %%eax; \
		 mov %14, %%eax; \
		 push %%eax; \
		 mov %4, %%eax; \
		 push %%eax; \
		 mov %0, %%eax; \
		 push %%eax; \
		 mov %10, %%ax; \
		 mov %%ax, %%es; \
		 mov %11, %%ax; \
		 mov %%ax, %%fs; \
		 mov %12, %%ax; \
		 mov %%ax, %%gs; \
		 mov %9, %%ax; \
		 mov %%ax, %%ds; \
		 pop %%eax; \
		 pop %%ebp; \
		 iret;"\
		: \
		:"m"(eax), "m"(ebx), "m"(ecx), "m"(edx), "m"(ebp), "m"(esi), "m"(edi), "m"(eflags), "m"(cs), "m"(ds), "m"(es), "m"(fs), "m"(gs), "m"(ss), "m"(eip), "m"(esp));
}


void test_task0(void)
{
	printk("in test_task0\n");
	printk("in test_task0\n");

	while(1)
	{
		;
	}
}



