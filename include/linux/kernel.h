#ifndef KERNEL_H
#define KERNEL_H

#include <sys/types.h>

uint32_t printk(const char *fmt, ...);

void print_context_info(uint32_t *esp, uint32_t error_code, uint32_t fs, uint32_t es, \
						uint32_t ds, uint32_t ebp, uint32_t esi, uint32_t edi, \
						uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, \
						uint32_t eip, uint32_t cs, uint32_t eflags);

void panic(char *msg);

#endif //KERNEL_H

