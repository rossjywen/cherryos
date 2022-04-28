#include <sys/types.h>

void trap_init(void);

void tty_init(void);

uint32_t printk(const char *fmt, ...);


/* _start, main, idt, gdt, idt_desc, gdt_desc, _pg_dir, after_page_table from head.s
 * pg_dir is from C
 * end is from linker
 * */
extern int _start;
int main();

void assemble_label_show()
{
	printk("from head.s\n");

	printk("&_start 0x%X\n", &_start);
	printk("_start 0x%X\n", _start);

	printk("&main 0x%X\n", &main);
	printk("main 0x%X\n", main);
}


int main()
{
	trap_init();

	tty_init();

	assemble_label_show();
}



