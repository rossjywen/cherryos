#include <sys/types.h>
#include <linux/mm.h>

#define LOW_MEM	0x100000

/* 这下面的宏都是按照ext mem来计算的 也就是高于1MB的内存 todo 16MB */
#define EXT_MEM_SIZE_LIMIT	(15 * 1024 * 1024)
#define PAGING_PAGES		(EXT_MEM_SIZE_LIMIT / PAGE_SIZE)
#define MAP_INDEX(addr)		((addr - LOW_MEM) / PAGE_SIZE)
#define USED	100

static uint8_t mem_map[PAGING_PAGES];	// 假设有16MB内存 (低端1MB + 高端15MB) todo 16MB
static uint32_t mem_end_addr;

void mem_init(uint32_t start_mem_addr, uint32_t end_mem_addr)
{
	int32_t i;

	mem_end_addr = end_mem_addr;

	for(i = 0; i < PAGING_PAGES; i++)
	{
		if(i >= MAP_INDEX(start_mem_addr) && i < MAP_INDEX(end_mem_addr))
			mem_map[i] = 0;
		else
			mem_map[i] = USED;
	}

}



void do_no_page(uint32_t error_code, uint32_t CR2)
{

}


void do_wp_page(uint32_t error_code, uint32_t CR2)
{

}


