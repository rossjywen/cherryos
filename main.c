#include <sys/types.h>
#include <asm/system.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <linux/kernel.h>
#include <string.h>

void trap_init(void);
void tty_init(void);
void sched_init(void);


void assemble_label_show()
{
	printk("from head.s\n");
	printk("_start addr %#x\n", &_start);
	printk("page_dir addr %#x\n", &page_dir);
	printk("kernel_stack addr %#x\n", &kernel_stack);
}


int main()
{
	tty_init();
	
	trap_init();

	assemble_label_show();

	sched_init();

	switch_to_TASK0();
}



