#ifndef SIGNAL_H
#define SIGNAL_H

#include <sys/types.h>

typedef unsigned int sigset_t;		/* 32 bits */


struct sigaction 
{
	void (*sa_handler)(int);
	sigset_t sa_mask;
	uint32_t sa_flags;
	void (*sa_restorer)(void);
};


#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGIOT		 6
#define SIGUNUSED	 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22

#define SIG_BLOCKABLE (~(1 << (SIGKILL - 1) | 1 << (SIGSTOP - 1)))

#define SIG_DFL		((void (*)(int))0)	/* default signal handling */
#define SIG_IGN		((void (*)(int))1)	/* ignore signal */

#define SA_NOMASK	0x40000000		// 不用修改current->blocked 所以也就不用保存和恢复
#define SA_ONESHOT	0x80000000		// 实现传统的 signal()的效果 捕获信号的设置只生效一次

#endif //SIGNAL_H

