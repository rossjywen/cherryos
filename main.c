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
	char *str = "hello Ross";

	trap_init();

	tty_init();

	assemble_label_show();

	printk("string %s : len %d \n", str, strlen(str));

	printk("struct tss_data size: %d\n", sizeof(struct tss_data));

	set_tss_desc(&gdt, 4, (uint32_t)(&task0.task.tss), DPL_0);
	set_ldt_desc(&gdt, 5, (uint32_t)(task0.task.ldt), DPL_0);

	switch_to(0);
}



