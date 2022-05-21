#ifndef SCHED_H
#define SCHED_H

#include <sys/types.h>
#include <linux/mm.h>
#include <linux/head.h>
#include <linux/signal.h>


#define NUMBER_OF_TASKS	64


#define FIRST_TSS_DESC_INDEX	4
#define FIRST_LDT_DESC_INDEX	(FIRST_TSS_DESC_INDEX + 1)

// 下面这两个宏来根据任务号来计算相应的16-bit selector, RPL=0, TI=0 (GDT)
#define _TSS(task_nr)	(((task_nr * 2 + FIRST_TSS_DESC_INDEX) << 3))
#define _LDT(task_nr)	(((task_nr * 2 + FIRST_LDT_DESC_INDEX) << 3))

#define TASK_LOAD_TR(task_nr)	asm("ltr %%ax"::"a"(_TSS(task_nr)))
#define TASK_LOAD_LDTR(task_nr)	asm("lldt %%ax"::"a"(_LDT(task_nr)))

#define get_segment_limit(selector)	\
({ \
	uint32_t __res; \
	asm("lsll %1, %0;":"=r"(__res):"r"(selector)); \
	__res; \
})


#define switch_to(task_nr) \
({ \
	uint32_t tmp[2] = {0,0}; \
	current = tasks_ptr[task_nr]; \
	asm("movw %%ax, %1; \
		 ljmp *%0;" \
		 ::"m"(tmp[0]), "m"(tmp[1]), "a"(_TSS(task_nr))); \
})


extern int32_t volatile jiffies;
#define NUM_INT_PS	100


struct tss_data
{
	uint16_t pre_task_link;
	uint16_t pre_task_link_pad;

	uint32_t esp0;

	uint16_t ss0;
	uint16_t ss0_pad;

	uint32_t esp1;

	uint16_t ss1;
	uint16_t ss1_pad;
	
	uint32_t esp2;

	uint16_t ss2;
	uint16_t ss2_pad;

	uint32_t cr3;

	uint32_t eip;

	uint32_t eflags;

	uint32_t eax;

	uint32_t ecx;

	uint32_t edx;

	uint32_t ebx;

	uint32_t esp;

	uint32_t ebp;

	uint32_t esi;

	uint32_t edi;

	uint16_t es;
	uint16_t es_pad;

	uint16_t cs;
	uint16_t cs_pad;

	uint16_t ss;
	uint16_t ss_pad;

	uint16_t ds;
	uint16_t ds_pad;

	uint16_t fs;
	uint16_t fs_pad;

	uint16_t gs;
	uint16_t gs_pad;

	uint16_t ldt;
	uint16_t ldt_pad;

	uint16_t debug_trap;		// 是个bool型变量 只有bit1有效 决定每次task switch的时候是否会触发异常

	uint16_t io_map_base;		// 和eflag中的iopl一起控制任务能够使用的io地址空间

	//uint32_t ssp; // this is a new feature Shadow Stack Pointer
};


enum task_state 
{
	TASK_RUNNING = 0, 
	TASK_INTERRUPTIBLE, 
	TASK_UNINTERRUPTIBLE, 
	TASK_ZOMBIE, 
	TASK_STOPPED
};


struct task_struct
{
	enum task_state state;
	
	int32_t exit_code;
	int32_t start_time;

	int32_t ts;			// time slice
	int32_t priority;	// nice value, also distributed time slice in schedule()
	
	/* signal related */
	int32_t signal;
	struct sigaction sa[32];
	int32_t blocked;
	int32_t alarm;

	/* a.out related */
	//uint32_t start_code;			//代码段地址
	//uint32_t end_code;			//代码长度 字节
	//uint32_t end_data;			//代码长度+数据长度
	//uint32_t brk;					//总长度
	//uint32_t start_stack;			//堆栈段地址


	/* pid related */
	int32_t pid;		// process id
	int32_t father;		// father task pid
	int32_t pgrp;		// process group id

	/* uid related */
	uint16_t uid;		// real uid (the uid which created the task)
	uint16_t euid;		// the effective uid which may grant more permission than uid like 'sudo'
	uint16_t suid;		// saved uid

	/* gid related */
	uint16_t gid;		// real gid
	uint16_t egid;		// effective gid
	uint16_t sgid;		// saved gid

	/* session related */
	int32_t session;	// session id
	int32_t leader;		// whether is session leader

	/* file system related */
	//int32_t tty;						//进程使用tty的子设备号 -1表示没有使用
	//uint16_t umask;						//文件创建属性屏蔽位
	//struct m_inode * pwd;				//当前工作目录的i-node
	//struct m_inode * root;				//根目录的i-node
	//struct m_inode * executable;		//执行文件i-node
	//uint32_t close_on_exec;				//执行时关闭文件句柄位图标志
	//struct file * filp[NR_OPEN];		//使用的文件结构

	/* statistics related */
	uint32_t uts;		// user time slice
	uint32_t kts;		// kernel time slice
	uint32_t cuts;		// child user time slice
	uint32_t ckts;		// child kernel time slice

	/* x86 related */
	struct seg_desc ldt[3];
	struct tss_data tss;
};


union task_union
{
	struct task_struct task;

	uint8_t stack[PAGE_SIZE];	// use as kernel stack when the task invokes system calls
};



struct task_struct * find_pid_task(int32_t pid);
void switch_to_TASK0(void);
void schedule(void);

uint32_t get_segment_base(struct seg_desc *seg);
void set_segment_base(struct seg_desc *seg, uint32_t new_base);
void set_segment_limit(struct seg_desc *seg, uint32_t new_limit);

void init_task1_paging(void);

extern struct task_struct *tasks_ptr[NUMBER_OF_TASKS];
extern union task_union task0;
extern struct task_struct *current;

#endif //SCHED_H

