#ifndef HEAD_H
#define HEAD_H

#include <sys/types.h>

struct seg_desc
{
	uint32_t low_32;
	uint32_t high_32;
};


extern uint32_t _start;

extern uint32_t page_dir;

extern struct seg_desc idt;
extern struct seg_desc gdt;

extern uint32_t kernel_stack;	// 跳转到main之前我把ESP设置成了这个地址


#endif //HEAD_H
