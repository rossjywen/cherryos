#ifndef TTY_H
#define TTY_H

#include <sys/types.h>
#include <termios.h>


#define TTY_BUF_SIZE	1024	// 注意 这个size不能随意修改 
								// 如果修改也必须是2的整数倍
								// 否则后果的宏就都错了 因为用-1的结果做&操作



// 参数是queue的index
#define INC(p)		(p = ((p) + 1)&(TTY_BUF_SIZE - 1))	// 桶形buff自增1 自动回滚
#define DEC(p)		(p = ((p) - 1)&(TTY_BUF_SIZE - 1))	// 桶形buff自减1 自动回滚

// 参数是queue
#define WRITE_SPACE(queue)		(((queue).tail - (queue).head -1) & (TTY_BUF_SIZE - 1))	// 还剩多少空间可以写入(head追上tail)
#define READ_SPACE(queue)		(((queue).head - (queue).tail) & (TTY_BUF_SIZE - 1))
#define EMPTY(queue)			(READ_SPACE(queue) == 0)
#define FULL(queue)				(WRITE_SPACE(queue) == 0)
#define LAST(queue)				((queue).buf[(queue.head - 1) & (TTY_BUF_SIZE - 1)])

//#define GET_CHAR(queue)


struct tty_queue
{
	uint32_t data;		// 换行符的数量
	uint32_t head;		// every write increases head
	uint32_t tail;		// every read decreases tail

	// task_struct * wait_q;	todo
	char buf[TTY_BUF_SIZE];
};



struct tty_struct
{
	struct termios termios;		

	uint32_t pgrp;
	uint32_t stopped;

	void (*write)(struct tty_struct *tty);

	struct tty_queue read_q;
	struct tty_queue write_q;
	struct tty_queue secondary;
};



#endif //TTY_H

