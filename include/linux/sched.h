#ifndef SCHED_H
#define SCHED_H

#include <sys/types.h>
#include <linux/mm.h>
#include <linux/head.h>


#define NUMBER_OF_TASKS	64


#define FIRST_TSS_DESC_INDEX	4
#define FIRST_LDT_DESC_INDEX	(FIRST_TSS_DESC_INDEX + 1)

// 下面这两个宏来根据任务号来计算相应的16-bit selector, RPL=3, TI=0 (GDT)
#define _TSS(task_nr)	(((task_nr * 2 + FIRST_TSS_DESC_INDEX) << 3) + 3)
#define _LDT(task_nr)	(((task_nr * 2 + FIRST_LDT_DESC_INDEX) << 3) + 3)

#define TASK_LOAD_TR(task_nr)	asm("ltr %%ax"::"a"(_TSS(task_nr)))
#define TASK_LOAD_LDTR(task_nr)	asm("lldt %%ax"::"a"(_LDT(task_nr)))


struct tss_data
{
	uint16_t pre_task_link;
	uint16_t pre_task_link_pad;

	uint32_t esp0;

	uint16_t ss0;
	uint16_t ss0_pad;

	uint32_t esp1;

	uint16_t ss1;
	uint16_t ss1_pad;
	
	uint32_t esp2;

	uint16_t ss2;
	uint16_t ss2_pad;

	uint32_t cr3;

	uint32_t eip;

	uint32_t eflags;

	uint32_t eax;

	uint32_t ecx;

	uint32_t edx;

	uint32_t ebx;

	uint32_t esp;

	uint32_t ebp;

	uint32_t esi;

	uint32_t edi;

	uint16_t es;
	uint16_t es_pad;

	uint16_t cs;
	uint16_t cs_pad;

	uint16_t ss;
	uint16_t ss_pad;

	uint16_t ds;
	uint16_t ds_pad;

	uint16_t fs;
	uint16_t fs_pad;

	uint16_t gs;
	uint16_t gs_pad;

	uint16_t ldt;
	uint16_t ldt_pad;

	uint16_t debug_trap;		// 是个bool型变量 只有bit1有效 决定每次task switch的时候是否会触发异常

	uint16_t io_map_base;		// 和eflag中的iopl一起控制任务能够使用的io地址空间

	//uint32_t ssp; // this is a new feature Shadow Stack Pointer
};



struct task_struct
{
	struct seg_desc ldt[3];

	struct tss_data tss;
};


union task_union
{
	struct task_struct task;

	uint8_t stack[PAGE_SIZE];	// use as kernel stack when the task invokes system calls
};



extern struct task_struct *tasks_ptr[NUMBER_OF_TASKS];

extern union task_union task0;


void switch_to(uint32_t task_number);


#endif //SCHED_H
