#include <sys/types.h>

/*
 * 向port指定的端口输出一个字节
 * */
#define out_b(value,port) \
__asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))

/*
 * 从port指定的端口读取一个字节
 * */
#define in_b(port) ({ \
	uint8_t _v; \
	__asm__ volatile ("inb %%dx,%%al":"=a" (_v):"d" (port)); \
	_v; \
})


