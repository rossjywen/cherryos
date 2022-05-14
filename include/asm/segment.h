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
	_fs; \
})


#define GET_CPL() (GET_CS() & 0x3)


#define write_fs_dword(val, addr) asm("movl %0,%%fs:%1"::"r" (val),"m" (*(addr)))

#define write_fs_byte(val, addr) asm("movb %0,%%fs:%1"::"r"(val), "m"(*(addr)))

#define read_fs_dword(addr) \
({ \
	uint32_t __res; \
	asm("movl %%fs:%1, %0;":"=r"(__res):"m"(*(addr))); \
	__res; \
})

#define read_fs_byte(addr) \
({ \
	uint8_t __res; \
	asm("movb %%fs:%1, %0;":"=r" (__res):"m"(*(addr))); \
	__res; \
})


#define write_to_user(from, to, size) \
{ \
	uint8_t *tmp_from = from, *tmp_to = to;\
	uint32_t i; \
	/* verify_area(to, sizeof(struct sigaction)); todo */ \
	for (i = 0 ; i < size; i++) \
	{ \
		write_fs_byte(*tmp_from, tmp_to); \
		tmp_from++; \
		tmp_to++; \
	} \
}


#define read_from_user(from, to, size) \
{ \
	uint8_t *tmp_from = from, *tmp_to = to;\
	uint32_t i; \
	for (i = 0 ; i < size; i++) \
		*(tmp_to++) = read_fs_byte(tmp_from++); \
}

#define read_seg_byte(seg, addr) \
({ \
	uint8_t __res; \
	__asm__("push %%fs;mov %%ax,%%fs;movb %%fs:%2,%%al;pop %%fs" \
	:"=a" (__res):"0" (seg),"m" (*(addr))); \
	__res; \
})


#define read_seg_dword(seg, addr) \
({ \
	uint32_t __res; \
	__asm__("push %%fs;mov %%ax,%%fs;movl %%fs:%2,%%eax;pop %%fs" \
	:"=a" (__res):"0" (seg),"m" (*(addr))); \
	__res; \
})


#endif //SEGMENT_H

