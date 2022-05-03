#include <sys/types.h>
#include <string.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <asm/system.h>

void trap_init(void);
void tty_init(void);
uint32_t printk(const char *fmt, ...);
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

	switch_to(0);
}



