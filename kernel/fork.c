#include <sys/types.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/head.h>
#include <linux/kernel.h>
#include <asm/system.h>
#include <errno.h>


int32_t last_pid = 0;


int32_t copy_mem(int32_t task_nr, struct task_struct *p)
{
	uint32_t old_code_base, old_data_base;
	uint32_t old_code_limit, old_data_limit;
	uint32_t new_base = task_nr * 0x4000000;

	old_code_base = get_segment_base(&(current->ldt[1]));
	old_data_base = get_segment_base(&(current->ldt[2]));
	old_code_limit = get_segment_limit(0x000F);
	old_data_limit = get_segment_limit(0x0017);

	if(old_code_base != old_data_base)
		panic("segment base error\n");
	if(old_code_limit > old_data_limit)
		panic("segment limit error\n");

	set_segment_base(&(p->ldt[1]), new_base);	// code
	set_segment_base(&(p->ldt[2]), new_base);	// data

	printk("new_base %#x\n", new_base);
	printk("code hi32 %#x\n", p->ldt[1].high_32);
	printk("code low32 %#x\n", p->ldt[1].low_32);
	printk("old code limit %#x\n", old_code_limit);
	//while(1);

	if(task_nr == 1)	// because my kernel uses 4MB pages so at this point I have to
	{					// do some perticular stuff on task1(init task)
		init_task1_paging();
	}
	else
	{
		if (copy_page_tables(old_data_base, new_base, old_data_limit))
		{
			free_page_tables(new_base, old_data_limit);
			return -ENOMEM;
		}
	}

	return 0;
}



int find_empty_process(void)
{
	int i;

	// step 1 calculate pid to fork()
	while(1)
	{	
		last_pid += 1;
		if (last_pid < 0) 
			last_pid = 1;

		for(i = 0; i < NUMBER_OF_TASKS; i++)
		{
			if ((tasks_ptr[i] != NULL) && (tasks_ptr[i]->pid == last_pid)) 
				break;
		}
		
		if(i == NUMBER_OF_TASKS)	// 说明找到了
			break;
	}
	
	// step2 find a null entry in tasks_ptr[]
	for(i = 1 ; i < NUMBER_OF_TASKS; i++)
	{
		if (tasks_ptr[i] == NULL)
			return i;
	}

	return 0;
}

// parameter 'unused' is the eip pushed by hardware for sys_fork()
int copy_process(uint32_t unused, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t edi, uint32_t esi, \
				 uint32_t ebp, uint32_t ds, uint32_t es, uint32_t fs, uint32_t eip, \
				 uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss)
{
	int32_t nxt_task_index;
	struct task_struct *p = NULL;

	nxt_task_index = find_empty_process();

	if(nxt_task_index == 0)		// 说明tasks_ptr[]数组中没有null的entry了
		return -EAGAIN;

	printk("--- in fork() ---\n");
	printk("task nr %d\n", nxt_task_index);
	printk("last pid %d\n", last_pid);
	printk("ds 0x%lx\n", ds);
	printk("cs 0x%lx\n", cs);
	printk("eip 0x%lx\n", eip);
	printk("eflags 0x%lx\n", eflags);
	//printk("0x%lx\n", );
	printk("-----------------\n");

	// 到这里说明找到了null的entry

	// 分配1个4k页空间存放task_union
	p = (struct task_struct*)get_free_page();
	tasks_ptr[nxt_task_index] = p;

	*p = *current; 			// 先把父进程的信息copy过来 然后再把不一样的修改了

	p->state = TASK_UNINTERRUPTIBLE;	// 在状态设置好之前 千万不能被中断程序调度
	p->pid = last_pid;
	p->father = current->pid;
	p->ts = current->priority;	// 时间片和优先级继承了父进程
	p->signal = 0;			// pending的信号清零
	p->alarm = 0;			// alarm清零
	p->leader = 0;
	p->uts = 0;
	p->kts = 0;
	p->cuts = 0;
	p->ckts = 0;
	//p->start_time todo


	// tss field initilization
	p->tss.pre_task_link = 0;
	p->tss.esp0 = (uint32_t)p + PAGE_SIZE;
	p->tss.ss0 = KERNEL_SS;
	p->tss.eip = eip;		// switch_to()的时候 回到调用fork()的地方 也就是int 0x80的下一条
	p->tss.eflags = eflags;
	p->tss.eax = 0;			// 这个eax就是fork()子进程返回0的原因
	p->tss.ecx = ecx;
	p->tss.edx = edx;
	p->tss.ebx = ebx;
	p->tss.esp = esp;
	p->tss.ebp = ebp;
	p->tss.esi = esi;
	p->tss.edi = edi;
	p->tss.es = es & 0xFFFF;
	p->tss.cs = cs & 0xFFFF;
	p->tss.ss = ss & 0xFFFF;
	p->tss.ds = ds & 0xFFFF;
	p->tss.fs = fs & 0xFFFF;
	p->tss.gs = 0;
	p->tss.ldt = _LDT(nxt_task_index);
	p->tss.debug_trap = 0;
	p->tss.io_map_base = 104;	// do not use io-bit map

	// todo x87 save

//	if(copy_mem(nxt_task_index, p))
//	{
//		tasks_ptr[nxt_task_index] = NULL;
//		free_page((uint32_t)p);
//		return -EAGAIN;
//	}

	// todo 把父进程打开的文件的打开计数增加 因为子进程继承了这些文件

	// todo 设置pwd root

	set_tss_desc(&gdt, FIRST_TSS_DESC_INDEX + nxt_task_index * 2, (uint32_t)(&p->tss), DPL_0);
	set_ldt_desc(&gdt, FIRST_LDT_DESC_INDEX + nxt_task_index * 2, (uint32_t)(&p->ldt), DPL_0);

	p->state = TASK_RUNNING;

	return last_pid;	// 这就是父进程调用fork()返回子进程id的原因
}

