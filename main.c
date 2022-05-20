#include <sys/types.h>
#include <asm/system.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <linux/kernel.h>
#include <string.h>


#define EXT_MEM_SIZE (*((uint16_t *)0x90002))


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
	uint32_t main_mem_end = EXT_MEM_SIZE;


	tty_init();
	
	trap_init();

	assemble_label_show();

	sched_init();
	
	system_call_init();

/*
	memory layout : 0 -> 1MB -> buf_mem_end -> main_mem_end

	0 ~ 1MB : low-end memory
	1MB ~ buf_mem_end : reserved for buffer
	buf_mem_end ~ main_mem_end : 
*/
	if(main_mem_end < 12 * 1024 * 1024)		// 小于12MB就直接报错死机
	{
		printk("memory size is too small\n");
		while(1);
	}
	else 
	{
		if(main_mem_end >= 16 * 1024 * 1024)
			main_mem_end = 16 * 1024 * 1024;
		buf_mem_end = 4 * 1024 * 1024;
	}
	mem_init(buf_mem_end, main_mem_end);

	switch_to_TASK0();
}



