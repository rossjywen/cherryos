#include <sys/types.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <linux/kernel.h>
#include <asm/system.h>
#include <asm/io.h>


int32_t volatile jiffies = 0;

struct task_struct *current = NULL;


void TASK_0(void);
uint8_t task0_user_stack[PAGE_SIZE];
union task_union task0 = { 	// todo other fields initilization
	.task = 
	{ 
		.state = TASK_RUNNING,
		.exit_code = 0,

		.ts = 10,
		.priority = 10,

		/* signal related */
		.signal = 0,
		.sa = {{},},
		.blocked = 0,
		.alarm = 0,

		/* a.out related */
		//.start_code = ,
		//.end_code = ,
		//.end_data = ,
		//.brk = ,
		//.start_stack = ,

		/* pid related */
		.pid = 0,
		.father = -1,
		.pgrp = 0,

		/* uid related */
		.uid = 0,
		.euid = 0,
		.suid = 0,

		/* gid related */
		.gid = 0,
		.egid = 0,
		.sgid = 0,

		/* session related */
		.session = 0,
		.leader = 0,

		/* file system related */
		//.tty = ,
		//.umask = ,
		//.pwd = ,
		//.root = ,
		//.executable = ,
		//.close_on_exec = ,
		//filp = ,

		/* statistics related */
		.uts = 0,
		.kts = 0,
		.cuts = 0,
		.ckts = 0,

		.ldt = 
		{ 
			{.low_32 = 0, .high_32 = 0},			// null descriptor
			{.low_32 = 0xFFF, .high_32 = 0xC0FA00},	// code segment descriptor todo 16MB
			{.low_32 = 0xFFF, .high_32 = 0xC0F200},	// data segment descriptor
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
			
			.eip =  (uint32_t)TASK_0,		// 我打算给指向一个函数

			.eflags = 0x00003202,	// 这是根据手册中eflag的初值设定的

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

			.fs = 0x17,		// 指向ldt中的第2个seg_desc -> data segment	
			.fs_pad = 0,

			.gs = 0,		// 不使用gs 指向GDT的null descriptor
			.gs_pad = 0,

			.ldt = _LDT(0),
			.ldt_pad = 0,

			.debug_trap = 0,

			.io_map_base = 104,	// 大于等于segment.limit就说明不使用这个功能
		},
	},
};


struct task_struct *tasks_ptr[NUMBER_OF_TASKS] = {&task0.task,};


struct task_struct * find_pid_task(int32_t pid)
{
	uint32_t i;

	for(i = 1; i < NUMBER_OF_TASKS; i++)
	{
		if((tasks_ptr[i] != NULL) && (tasks_ptr[i]->pid == pid))
			break;
	}

	if(i == NUMBER_OF_TASKS)
		return NULL;
	else
		return tasks_ptr[i];
}


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

	//printk("in set_tss_desc() low : %#x\n", low);
	//printk("in set_tss_desc() high : %#x\n", high);

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


void timer_interrupt(void);

void sched_init(void)
{
	uint32_t i;

	set_tss_desc(&gdt, FIRST_TSS_DESC_INDEX, (uint32_t)(&task0.task.tss), DPL_0);
	set_ldt_desc(&gdt, FIRST_LDT_DESC_INDEX, (uint32_t)(task0.task.ldt), DPL_0);
	TASK_LOAD_TR(0);	// timer handler will do 'mov fs, 0x17'
	TASK_LOAD_LDTR(0);	// so LDTR must be initilized before timer interrupt

	for(i = 1; i < NUMBER_OF_TASKS; i++)
		tasks_ptr[i] = NULL;

	for(i = FIRST_TSS_DESC_INDEX + 2; i < (NUMBER_OF_TASKS - 1) * 2; i++)
		*((uint64_t*)(&gdt) + i) = 0;	// 其实已经在head.s中做过了 这里为了可读性再做一次

	// 开始对8254进行编程
	// 使用工作方式3 方波发生器 1秒产生100次中断
	out_b(0x36, CMD_PORT_8254);
	out_b((1193180 / NUM_INT_PS) & 0xFF, CR_PORT_8254);			// 先写低位
	out_b(((1193180 / NUM_INT_PS) >> 8) & 0xFF, CR_PORT_8254);	// 再写高位

	// program 8259 to enable timer0 interrupt
	set_gate(&idt, 0x20, INTERRUPT_GATE, KERNEL_CS, &timer_interrupt, DPL_0);
	out_b(in_b(MASTER_8259_OPERATE_OCW1) & ~0x01, MASTER_8259_OPERATE_OCW1);
}


void switch_to_TASK0()
{
	uint32_t eip = tasks_ptr[0]->tss.eip;
	uint32_t eflags = tasks_ptr[0]->tss.eflags;

	uint32_t eax = tasks_ptr[0]->tss.eax;
	uint32_t ecx = tasks_ptr[0]->tss.ecx;
	uint32_t edx = tasks_ptr[0]->tss.edx;
	uint32_t ebx = tasks_ptr[0]->tss.ebx;
	uint32_t esp = tasks_ptr[0]->tss.esp;
	uint32_t ebp = tasks_ptr[0]->tss.ebp;
	uint32_t esi = tasks_ptr[0]->tss.esi;
	uint32_t edi = tasks_ptr[0]->tss.edi;

	uint16_t es = tasks_ptr[0]->tss.es;
	uint16_t cs = tasks_ptr[0]->tss.cs;
	uint16_t ss = tasks_ptr[0]->tss.ss;
	uint16_t ds = tasks_ptr[0]->tss.ds;
	uint16_t fs = tasks_ptr[0]->tss.fs;
	uint16_t gs = tasks_ptr[0]->tss.gs;


/*
	1.切换到新任务的ldt和tr (通过宏调用)
	2.把ss esp eflags cs eip 按顺序入栈 构造一个上一次带有CPL切换效果的调用栈 为iret准备环境
	3.切换到新任务的 通用寄存器 其中ebp和eax先不改变而是入栈中 因为需要ebp来访问函数局部变量 需要eax做一些数据挪动
	4.切换到新任务的ds es fs 最后改变ds 因为从ds改变的一刻起就不能访问C中的变量了 但可以使用之前压入栈中的数据 因为这时候ss依然是指向前一个任务的内核栈
	5.分别从栈上取出新任务eax和ebp的值并更新到eax和ebp中
	6.iret
*/

	current = tasks_ptr[0];
	
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
		 iret;" \
		: \
		:"m"(eax), "m"(ebx), "m"(ecx), "m"(edx), "m"(ebp), "m"(esi), "m"(edi), "m"(eflags), "m"(cs), "m"(ds), "m"(es), "m"(fs), "m"(gs), "m"(ss), "m"(eip), "m"(esp));
}


void schedule(void)
{
	int32_t i;
	int32_t next_task_nr = -1;
	int32_t next_task_ts = 0;

	// 1.check if there is signal pending, set task with signal pending to TASK_RUNNING
	for(i = NUMBER_OF_TASKS - 1; i >= 0; i--)
	{
		if(tasks_ptr[i] != NULL)
		{
			if((tasks_ptr[i]->alarm != 0) && (tasks_ptr[i]->alarm < jiffies))
			{
				tasks_ptr[i]->signal |= 1 << (SIGALRM - 1);
				tasks_ptr[i]->alarm = 0;
			}

			if(((tasks_ptr[i]->signal & ~(tasks_ptr[i]->blocked & SIG_BLOCKABLE)) != 0) && \
				(tasks_ptr[i]->state==TASK_INTERRUPTIBLE))
			{
				tasks_ptr[i]->state = TASK_RUNNING;
			}
		}
	}

	// 2.select the task with most time slices to switch to
	for(i = NUMBER_OF_TASKS - 1; i >= 0; i--)
	{
		if((tasks_ptr[i] != NULL) && \
				(tasks_ptr[i]->state == TASK_RUNNING) && \
				(tasks_ptr[i]->ts > next_task_ts))
		{
			next_task_nr = i;
			next_task_ts = tasks_ptr[i]->ts;
		}
	}

	if(next_task_nr == -1)	// 如果为-1说明已经没有任务有待运行的时间片了
	{
		for(i = NUMBER_OF_TASKS - 1; i >= 0; i--)
		{
			if((tasks_ptr[i] != NULL) && (tasks_ptr[i]->state == TASK_RUNNING))
			{
				tasks_ptr[i]->ts = tasks_ptr[i]->priority;
			}
		}
	}
	else	// 找到了待运行的任务 执行任务切换
	{
		if(current != tasks_ptr[next_task_nr])
		{
			switch_to(next_task_nr);
		}
	}
}



void do_timer_interrupt(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, \
						uint32_t edi, uint32_t esi, uint32_t ebp, uint32_t ds, uint32_t es, uint32_t fs, \
						uint32_t eip, uint32_t cs, uint32_t eflags)
{
	// 1.jiffies 自增
	jiffies++;

	// 2.ts自减
	current->ts -= 1;

	// todo timer

	// 3.检查到底是用户态还是内核态被中断的
	//   如果内核态被中断则不能调度
	//   如果用户态被中断则保存现场 然后选另外一个进程执行
	//   另外递增当前task的uts或kts
	if((cs & 0x3) == 0)	// 说明是内核态被中断
	{
		current->kts++;
	}
	else
	{
		current->uts++;
		
		if(current->ts <= 0)
		{
			schedule();
		}
	}

	return;
}


void sleep_on(struct task_struct **p)
{
	struct task_struct *tmp;

	if (!p)
		return;
	if (current == &(task0.task))
		panic("task[0] trying to sleep");
	tmp = *p;
	*p = current;
	current->state = TASK_UNINTERRUPTIBLE;
	schedule();
	if (tmp)
		tmp->state = TASK_RUNNING;
}


void interruptible_sleep_on(struct task_struct **p)
{
	struct task_struct *tmp;

	if (!p)
		return;
	if (current == &(task0.task))
		panic("task[0] trying to sleep");
	tmp = *p;
	*p = current;

repeat:
	current->state = TASK_INTERRUPTIBLE;
	schedule();
	if (*p && *p != current) {
		(**p).state = TASK_RUNNING;
		goto repeat;
	}
	*p=NULL;
	if (tmp)
		tmp->state = TASK_RUNNING;
}


void wake_up(struct task_struct **p)
{
	if (p && *p) {
		(**p).state = TASK_RUNNING;
		*p = NULL;
	}
}


uint32_t get_segment_base(struct seg_desc *seg)
{
	uint32_t base = 0;
	uint32_t base_15_0 = 0, base_23_16 = 0, base_31_24 = 0;

	base_15_0 = (seg->low_32 >> 16) & 0xFFFF;
	base_23_16 = seg->high_32 & 0xFF;
	base_31_24 = (seg->high_32 >> 24) & 0xFF;

	base = (base_31_24 << 24) | (base_23_16 << 16) | base_15_0;

	return base;
}


void set_segment_base(struct seg_desc *seg, uint32_t new_base)
{
	uint32_t base_15_0, base_23_16, base_31_24;

	base_15_0 = new_base & 0xFFFF;
	base_23_16 = (new_base >> 16) & 0xFF;
	base_31_24 = (new_base >> 24) & 0xFF;

	seg->low_32 &= 0x0000FFFF;	// 把base与成0
	seg->high_32 &= 0x00FFFF00;

	seg->low_32 |= base_15_0 << 8;
	seg->high_32 |= base_23_16;
	seg->high_32 |= base_31_24 << 24;
}


void set_segment_limit(struct seg_desc *seg, uint32_t new_limit)
{
	uint32_t limit_19_16, limit_15_0;

	limit_19_16 = (new_limit >> 16) & 0xF;
	limit_15_0 = new_limit & 0xFFFF;

	seg->low_32 &= 0xFFFF0000;	// 把limit与成0
	seg->high_32 &= 0xFFF0FFFF;

	seg->low_32 |= limit_15_0;
	seg->high_32 |= limit_19_16 << 16;
}


int sys_pause(void)
{
	current->state = TASK_INTERRUPTIBLE;
	schedule();
	return 0;
}

int sys_getpid(void)
{
	return current->pid;
}

int sys_getppid(void)
{
	return current->father;
}

int sys_getuid(void)
{
	return current->uid;
}

int sys_geteuid(void)
{
	return current->euid;
}

int sys_getgid(void)
{
	return current->gid;
}

int sys_getegid(void)
{
	return current->egid;
}

/*
	linux-0.11实现nice()和现在的nice不同
	这是因为在0.11中nice值越大就会分配到更多的执行时间
	这与现在linux的nice的效果相反 
*/
int sys_nice(int32_t increment)
{
	if (current->priority-increment>0)
	{
		current->priority -= increment;
	}
	else
	{
		if(current->euid == 0)	// root
			current->priority -= increment;

	}
	return 0;
}


