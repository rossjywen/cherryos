#include <sys/types.h>

// 8254 port address
#define CMD_PORT_8254	0x43
#define CR_PORT_8254	0x40

// 8259 master
#define MASTER_8259_INIT_ICW1		0x20
#define MASTER_8259_INIT_ICW2		0x21
#define MASTER_8259_INIT_ICW3		0x21
#define MASTER_8259_INIT_ICW4		0x21
#define MASTER_8259_OPERATE_OCW1	0x21
// 8259 slave
#define SLAVE_8259_INIT_ICW1		0xA0
#define SLAVE_8259_INIT_ICW2		0xA1
#define SLAVE_8259_INIT_ICW3		0xA1
#define SLAVE_8259_INIT_ICW4		0xA1
#define SLAVE_8259_OPERATE_OCW1		0xA1




#define out_b(value,port) \
__asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))

#define in_b(port) ({ \
	uint8_t _v; \
	__asm__ volatile ("inb %%dx,%%al":"=a" (_v):"d" (port)); \
	_v; \
})


