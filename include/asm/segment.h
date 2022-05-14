#ifndef SEGMENT_H
#define SEGMENT_H

#include <sys/types.h>

#define GET_CS() \
({ \
	uint16_t _cs = 0; \
	asm("mov %%cs, %%ax;":"=a"(_cs):); \
	_cs; \
})


#define GET_FS() \
({ \
	uint16_t _fs = 0; \
	asm("mov %%cs, %%ax;":"=a"(_fs):); \
	_cs; \
})


#define GET_CPL() (GET_CS() & 0x3)


#define write_fs_dword(val, addr)	asm("movl %%eax, %%fs:%1;"::"a"(val), "m"(*(addr)))


#define read_seg_byte(seg,addr) \
({ \
	uint8_t __res; \
	__asm__("push %%fs;mov %%ax,%%fs;movb %%fs:%2,%%al;pop %%fs" \
	:"=a" (__res):"0" (seg),"m" (*(addr))); \
	__res; \
})


#define read_seg_dword(seg,addr) \
({ \
	uint32_t __res; \
	__asm__("push %%fs;mov %%ax,%%fs;movl %%fs:%2,%%eax;pop %%fs" \
	:"=a" (__res):"0" (seg),"m" (*(addr))); \
	__res; \
})


#endif //SEGMENT_H

