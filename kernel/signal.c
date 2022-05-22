#include <sys/types.h>
#include <linux/signal.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/segment.h>


int32_t sys_alarm(uint32_t seconds)
{
	int32_t old = current->alarm;

	if (old)
		old = (old - jiffies) / NUM_INT_PS;

	current->alarm = (seconds > 0) ? (jiffies + NUM_INT_PS * seconds) : 0;

	return (old);
}


int32_t sys_sgetmask()
{
	return current->blocked;
}


int32_t sys_ssetmask(int32_t newmask)
{
	int32_t old_mask = current->blocked;

	current->blocked = newmask & ~(1<<(SIGKILL-1));

	return old_mask;
}


int32_t sys_signal(uint32_t signum, uint32_t handler, uint32_t restorer)
{
	struct sigaction tmp;	
	int32_t res;

	if(signum < 1 || signum > 32 || signum == SIGKILL)
		return -1;
	
	tmp.sa_handler = (void (*)(int)) handler;
	tmp.sa_mask = 0;
	tmp.sa_flags = SA_ONESHOT | SA_NOMASK;
	tmp.sa_restorer = (void (*)(void)) restorer;

	current->sa[signum - 1] = tmp;

	res = (int32_t) (current->sa[signum-1].sa_handler);

	return res;
}


// 注意 这里的 action 和 oldaction 都是指向用户空间的地址
int32_t sys_sigaction(uint32_t signum, struct sigaction * action, struct sigaction * oldaction)
{
	struct sigaction tmp;	

	if(signum < 1 || signum > 32 || signum == SIGKILL)
		return -1;
	
	tmp = current->sa[signum - 1];

	read_from_user((uint8_t *)action, (uint8_t *)(current->sa + signum - 1), sizeof(struct sigaction));

	if (oldaction)
		write_to_user((uint8_t *)&tmp, (uint8_t *)oldaction, sizeof(struct sigaction));
	
	if (current->sa[signum-1].sa_flags & SA_NOMASK)
		current->sa[signum-1].sa_mask = 0;
	else
		current->sa[signum-1].sa_mask |= (1<<(signum-1));
	
	return 0;
}

int32_t do_exit(int32_t exit_code);

void do_signal(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, \
			uint32_t edi, uint32_t esi, uint32_t ebp, uint32_t ds, uint32_t es, uint32_t fs, \
			uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t *esp, uint32_t ss)
{
	uint32_t sig_bitmap;
	int32_t sig_index;
	int32_t sig_number;
	struct sigaction *sig_act;
	uint32_t old_eip = eip;

	if(current == tasks_ptr[0])		// task0 cann't have signal
		return;

	if((cs & 0x3) == 0)		// if CPL = 0 cann't response to signal pending
		return;				// that is because no USER SS and ESP is pushed onto the stack

	// start to detect pending signal number
	sig_bitmap = current->signal & current->blocked;
	//sig_bitmap = 0x80;

	// use instruction 'bsf' to get index of first bit '1'
	// if operand is 0 the zero-flag is raised and result is undefined
	asm("bsfl %%ecx, %%eax; \
		 jnz 1f; \
		 movl $-1, %0; \
		 1:":"=a"(sig_index):"c"(sig_bitmap));

	//sig_bitmap &= ~(1 << sig_index);
	//printk("signal %#x\n", sig_bitmap);
	//printk("bsr result %d\n", sig_index);
	//return;
	
	if(sig_index == -1)		// no signal pending
		return;

	current->signal &= ~(1 << sig_index);	// 把对应信号的位reset
	
	sig_number = sig_index + 1;				// 信号号 比检测bit 1的index大1
	sig_act = &(current->sa[sig_index]);	// 而index正好就是sa数组的下标

	// 到这里根据用户是否设置了回掉函数来进行
	if(sig_act->sa_handler == SIG_IGN)		// 如果是ignore 直接return
		return;

	if(sig_act->sa_handler == SIG_DFL)		// 如果是default 则继续判断
	{
		if(sig_number == SIGCHLD)	// todo 确认子进程退出状态 改变TASK_ZOMBIE态
			return; // todo 应该调用wait() 但是linux-0.11直接return
		else
			do_exit(1 << sig_number);
	}

	// 如果走到了这里就说明需要直接用户设置的handler了

	*(&eip) = (uint32_t)(sig_act->sa_handler);	// ret_from_syscall中执行iret的地址被修改为用户设置的handler地址

	//verify_area(); todo
	write_fs_dword(old_eip, esp - 1);	// 执行完sa_restorer()之后返回的地址
	write_fs_dword(eflags, esp - 2);	// sa_restorer()会恢复eflags
	write_fs_dword(edx, esp - 3);		// sa_restorer()会恢复edx (实现系统调用的第二个参数)
	write_fs_dword(ecx, esp - 4);		// 同上 注意 没有ebx 因为gcc默认不使用ebx 如果使用会保存和恢复
	write_fs_dword(eax, esp - 5);		// eax是系统调用的返回值
	if(sig_act->sa_flags & SA_NOMASK)	// 不需要保存current->blocked
	{
		write_fs_dword(sig_number, esp - 6);
		write_fs_dword(sig_act->sa_restorer, esp - 7);
		*(&esp) -= 7;
	}
	else
	{
		write_fs_dword(current->blocked, esp - 6);
		write_fs_dword(sig_number, esp - 7);			// sa_restorer()实际上没有使用这个值
		write_fs_dword(sig_act->sa_restorer, esp - 8);	// 执行完sa_handler()之后的返回地址
		*(&esp) -= 8;
	}

	current->blocked |= sig_act->sa_mask;

	// 最后根据sa_flags的值决定是否需要恢复成SIG_DFL
	if(sig_act->sa_flags & SA_ONESHOT)	// 说明捕获信号只生效一次 需恢复成SIG_DFL
		sig_act->sa_handler = SIG_DFL;		// 否则就不用改变 一直保持
}


