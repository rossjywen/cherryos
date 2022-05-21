#ifndef MM_H
#define MM_H

#include <sys/types.h>

#define PAGE_SIZE 4096

uint32_t get_free_page(void);
void free_page(uint32_t addr);

void init_task1_paging(void);

int32_t copy_page_tables(uint32_t from, uint32_t to, uint32_t size);
int32_t free_page_tables(uint32_t from, uint32_t size);

#endif

