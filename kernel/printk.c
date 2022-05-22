#include <stdarg.h>
#include <sys/types.h>
#include <linux/kernel.h>
#include <asm/system.h>

static char buf[1024];

int vsprintf(char * buf, const char * fmt, va_list args);

uint32_t kernel_tty_write(char *buf, uint32_t nr);

uint32_t printk(const char *fmt, ...)
{
	va_list args;
	int i;

	disable_interrupt();

	va_start(args, fmt);
	i=vsprintf(buf,fmt,args);
	va_end(args);

	kernel_tty_write(buf, i);

	enable_interrupt();

	return i;
}


// todo sys_sync()
//void sys_sync(void);

void panic(char *msg)
{
	printk("Kernel panic: %s\n", msg);

	while(1) ;
	//sys_sync();
}



