#include <sys/types.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/head.h>
#include <asm/system.h>

#define LOW_MEM	0x100000

/* 这下面的宏都是按照ext mem来计算的 也就是高于1MB的内存 todo 16MB */
#define EXT_MEM_SIZE_LIMIT	(15 * 1024 * 1024)
#define PAGING_PAGES		(EXT_MEM_SIZE_LIMIT / PAGE_SIZE)
#define MAP_INDEX(addr)		((addr - LOW_MEM) / PAGE_SIZE)
#define NOT_EXIST	100

#define flush_TLB() asm("movl %%eax,%%cr3"::"a" (0))

static uint8_t mem_map[PAGING_PAGES];	// 假设有16MB内存 (低端1MB + 高端15MB) todo 16MB
static uint32_t mem_end_addr;

// note: returned value is physical address
uint32_t get_free_page(void)
{
	int32_t i, j;
	uint8_t *p;


	// look up mem_map[] find a valid entry of 4k page
	for(i = 0; i < PAGING_PAGES; i++)
	{
		if(mem_map[i] == 0)
			break;
	}

	if(i == PAGING_PAGES)	// 说明没有空余页了
	{
		return 0;
	}
	else
	{
		mem_map[i] += 1;	// 增加一次分配

		p = (uint8_t*)(0x1000 * i + LOW_MEM);
		for(j = 0; j < PAGE_SIZE; j++)	// 把这个page都刷成0
			p[j] = 0;

		//printk("allocate %dth page\n", i);

		return 0x1000 * i + LOW_MEM;
	}
}

// note: parameter is physical address
void free_page(uint32_t addr)
{
	if(addr < LOW_MEM)
		panic("free addr < 1MB\n");

	if(addr >= LOW_MEM + EXT_MEM_SIZE_LIMIT)
		panic("free addr > 16MB\n");	// todo 16MB

	if(mem_map[MAP_INDEX(addr)] >= 1)
	{
		mem_map[MAP_INDEX(addr)] -= 1;
		return;
	}
	else
	{
		panic("free addr bad map\n");
	}
}


/*
 * CR3页目录中每一个entry对应4MB
 * task1从64MB开始
 * 所以task1在CR3中就是第16-32个entry 每一个entry对应一个page
 * 但实际上不需要那么多 因为只初始化1个page即可 (事实上1MB以内即可)
 * */
void init_task1_paging(void)
{
	int32_t i;
	uint32_t page;
	uint32_t *p = (uint32_t *)(16 * 4);

	page = get_free_page();
	if(page == 0)
		panic("init task1 failed\n");

	for(i = 0; i < (1024 / 4); i++)	// 把1MB的entry都填满
	{
		*((uint32_t *)page + i) = (i * 0x1000) | 7;
	}
	*p = page | 7;	// 页目录

	return;
}


int32_t free_page_tables(uint32_t from, uint32_t size)
{
	uint32_t *page_dir_addr, *page_tab_addr;
	uint32_t nr_of_dir_ent;
	int32_t i, j;

	if(from & 0x3FFFFF)
		panic("free: page dir alignment error");
	if(from == 0)
		panic("free addr = 0");

	page_dir_addr = (uint32_t *)((from >> 22) * 4);
	nr_of_dir_ent = (size + 0x3FFFFF) >> 22;	// 得出有几个4MB的页目录

	for(i = 0; i < nr_of_dir_ent; i++)
	{
		if((page_dir_addr[i] & 1) == 0)
			continue;
		page_tab_addr = (uint32_t *)((uint32_t)(page_dir_addr[i]) & 0xFFFFF000);
		for(j = 0; j < 1024; j++)
		{
			if((page_tab_addr[j] & 1) == 1)
				free_page(page_tab_addr[j] & 0xFFFFF000);
			page_tab_addr[j] = 0;
		}
		free_page(page_dir_addr[i] & 0xFFFFF000);
		page_dir_addr[i] = 0;
	}

	flush_TLB();

	return 0;
}


/*
 * 这个函数只是fork()的时候调用
 * 所以from和to一定是64MB对齐的
 * 但是考虑到语意copy的是page table 所以检查4MB边界
 * */
int32_t copy_page_tables(uint32_t from, uint32_t to, uint32_t size)
{
	uint32_t *from_page_dir_addr, *to_page_dir_addr;
	uint32_t nr_of_dir_ent;
	uint32_t *from_page_tab_addr, *to_page_tab_addr;
	uint32_t curr_page;
	int32_t i, j;

	if((from & 0x3FFFFF) || (to & 0x3FFFFF))
		panic("copy: page dir alignment error");

	from_page_dir_addr = (uint32_t *)((from >> 22) * 4);
	to_page_dir_addr = (uint32_t *)((to >> 22) * 4);
	nr_of_dir_ent = (size + 0x3FFFFF) >> 22;	// 得出有几个4MB的页目录

	for(i = 0; i < nr_of_dir_ent; i++)
	{
		if((to_page_dir_addr[i] & 1) != 0)
			panic("destination page dir is present");
		if((from_page_dir_addr[i] & 1) == 0)
			panic("source page dir is not present");

		from_page_tab_addr = (uint32_t *)((uint32_t)(from_page_dir_addr[i]) & 0xFFFFF000);
		to_page_tab_addr = (uint32_t *)get_free_page();
		if(to_page_tab_addr == 0)
			return -1;
		to_page_dir_addr[i] = (uint32_t)to_page_tab_addr | 7;

		for(j = 0; j < 1024; j++)
		{
			curr_page = from_page_tab_addr[j];
			if((curr_page & 1) == 0)
				continue;
			curr_page &= ~2;	// R/W位置0
			to_page_tab_addr[j] = curr_page;

			if(curr_page > LOW_MEM)	// 说明是1MB以上的
			{
				from_page_tab_addr[j] = curr_page;	// 把父进程的页表R/W位也置0
				mem_map[MAP_INDEX(curr_page)] += 1;	// 先写的会触发exception
			}
		}
	}

	flush_TLB();

	return 0;
}

void page_fault(void);
void mem_init(uint32_t start_mem_addr, uint32_t end_mem_addr)
{
	int32_t i;

	mem_end_addr = end_mem_addr;

	for(i = 0; i < PAGING_PAGES; i++)
	{
		if(i >= MAP_INDEX(start_mem_addr) && i < MAP_INDEX(end_mem_addr))
			mem_map[i] = 0;
		else
			mem_map[i] = NOT_EXIST;
	}

	set_gate(&idt, 14, TRAP_GATE, KERNEL_CS, &page_fault, DPL_0);
}



void do_no_page(uint32_t error_code, uint32_t CR2, uint32_t fs, uint32_t es,
		uint32_t ds, uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax,
		uint32_t eip, uint32_t cs, uint32_t eflags)
{
	printk("do_no_page : error code %#x\n", error_code);
	printk("CR2 %#x\n", CR2);
	printk("eip %#x\n", eip);
	while(1);
}


void do_wp_page(uint32_t error_code, uint32_t CR2, uint32_t fs, uint32_t es,
		uint32_t ds, uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax,
		uint32_t eip, uint32_t cs, uint32_t eflags)
{
	printk("do_wp_page : error code %#x\n", error_code);
	printk("CR2 %#x\n", CR2);
	printk("eip %#x\n", eip);
	while(1);
}


