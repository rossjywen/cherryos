#include <sys/types.h>
#include <asm/system.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <linux/kernel.h>
#include <string.h>
#include <errno.h>

#define __LIBRARY__
#include <unistd.h>
static inline _syscall_0(int, fork)

#define EXT_MEM_K (*(unsigned short *)0x90002)

void trap_init(void);
void tty_init(void);
void sched_init(void);
void system_call_init(void);
void mem_init(uint32_t start_mem_addr, uint32_t end_mem_addrd);


void assemble_label_show()
{
	printk("from head.s\n");
	printk("_start addr %#x\n", &_start);
	printk("page_dir addr %#x\n", &page_dir);
	printk("kernel_stack addr %#x\n", &kernel_stack);
}


int main()
{
	uint32_t buf_mem_end;
	uint32_t main_mem_end;


	main_mem_end = EXT_MEM_K;	// note: at this point the unit is KB

	tty_init();
	
	trap_init();

	system_call_init();

	assemble_label_show();

/*
	memory layout : 0 -> 1MB -> buf_mem_end -> main_mem_end

	0 ~ 1MB : low-end memory
	1MB ~ buf_mem_end : reserved for buffer
	buf_mem_end ~ main_mem_end : 
*/
	if(main_mem_end < 12 * 1024)		// 小于12MB就直接报错死机
	{
		printk("memory size is too small\n");
		while(1);
	}
	else
	{
		if(main_mem_end >= 16 * 1024 * 1024)
			main_mem_end = 16 * 1024 * 1024;	// 这个时候单位就不是KB了而是B
		buf_mem_end = 4 * 1024 * 1024;
		printk("ext mem size : %#x\n", main_mem_end);
	}
	mem_init(buf_mem_end, main_mem_end);

	sched_init();	// 进入task0之前最后开启定时器中断

	switch_to_TASK0();
}

void TASK_0(void)
{
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t k = 0;

	if(fork() == 0)		// child
	{
		if(fork() == 0)
		{
			while(1)
			{
				k += 10;
				if(k >= 0x7A0000) // use vb to debug
				{
					//printk("in task2\n");	still does not work even I shared pagging in char graphic mem
					k = 0;
				}
			}
		}

		while(1)
		{
			j++;
			if(j == 0x4C0000)	// 5000000
			{
				printk("in task1\n");
				/*
				 * 下面这个操作会触发page fault
				 * 经过debug 发现是因为 通过fork()创建的任务的task_union是通过get_free_page()分配的
				 * 而从main()上面的代码可知 是从buf_mem_end开始分配
				 * 而在任务中读取这个current就会触发段级保护
				 * */
				//printk("current->ts %d\n", current->ts);
				j = 0;
			}
		}
	}

	while(1)
	{
		i++;
		if(i == 0x980000)	// 10000000
		{
			printk("in task0\n");
			//printk("current->ts%d\n", current->ts); 如果打开这条语句 printk就会出bug
			i = 0;
		}
	}
}

