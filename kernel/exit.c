#include <sys/types.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <errno.h>



// release the task_union(task_struct) allocated in fork()
void release(struct task_struct *p)
{
	uint32_t i;
	

	// set corresponding entry in tasks_ptr to null
	// then invoke free_page()
	
	if(p == NULL)
		return;

	for(i = 0; i < NUMBER_OF_TASKS; i++)
	{
		if(tasks_ptr[i] == p)
		{
			tasks_ptr[i] = NULL;

			free_page((uint32_t)p);

			schedule();

			return;		// suppress warning from compiler
		}
	}

	panic("in release()\n");
}


// send a signal to the certain task_struct
static int32_t send_sig(int32_t sig, struct task_struct *p, int32_t priv)
{
	if((p == NULL) || (sig < 1) || (sig > 32))
		return -EINVAL;
	
	// 三种情况允许操作 1.priv置位 强制执行 2.root 3.待发送信号的进程的euid等于当前的euid
	if(priv || (current->euid == 0) || (current->euid == p->euid))
	{
		p->signal |= 1 << (sig - 1);
		return 0;
	}
	else
	{
		return -EPERM;
	}
}


int32_t sys_kill(int32_t pid, int32_t sig)
{
	struct task_struct *task;
	int32_t retval;
	int32_t i;

	if(pid > 0) // 1.如果pid>0 就会发送给这个进程
	{
		task = find_pid_task(pid);	// 不检查返回值 让send_sig()检查
		retval = send_sig(sig, task, 0);
	}
	else if(pid == 0) // 2.如果pid=0 就会把信号发送给current所在的进程组的所有进程
	{
		for(i = 1; i < NUMBER_OF_TASKS; i++)
		{
			if((tasks_ptr[i] != NULL) && (tasks_ptr[i]->pgrp == current->pid))
				send_sig(sig, tasks_ptr[i], 1);
		}
		retval = 0;
	}
	else if(pid == -1) // 3.如果pid=-1 就会发送给除了init(task1)之外的所有进程
	{
		for(i = 1; i < NUMBER_OF_TASKS; i++)
		{
			if(tasks_ptr[i] != NULL)
				send_sig(sig, tasks_ptr[i], 0);
		}
		retval = 0;
	}
	else if(pid < -1) // 4.如果pid<-1 就会发送给pid绝对值的进程组号的所有进程
	{
		for(i = 1; i < NUMBER_OF_TASKS; i++)
		{
			if((tasks_ptr[i] != NULL) && (tasks_ptr[i]->pgrp == -pid))
				send_sig(sig, tasks_ptr[i], 0);
		}
		retval = 0;
	}

	return retval;
}


// send SIGHUP to tasks in the session
static void kill_session(void)
{
	int32_t i;

	for(i = 1; i < NUMBER_OF_TASKS; i++)
	{
		if((tasks_ptr[i] != NULL) && (tasks_ptr[i]->session == current->session))
			tasks_ptr[i]->signal |= 1 << (SIGHUP - 1);
	}
}

// send SIGCHLD to father_pid to inform the exist of its child
static void tell_father(uint32_t father_pid)
{
	struct task_struct *p;

	if(father_pid == 0)	// 那说明父进程是TASK0 
	{
		return;
	}
	else
	{
		p = find_pid_task(father_pid);

		if(p != NULL)	// 说明找到了
		{
			p->signal |= (1 << (SIGCHLD - 1));
			return;
		}
		else
		{
			printk("no father found\n");
			release(current);		// 意思是如果没有父进程就自己给释放了
			return ;
		}
	}
}



int32_t do_exit(int32_t code)
{
	int32_t i;
	// todo do_exit

	// 1.把代码段和数据段分配的页表项free掉
	free_page_tables(get_segment_base(&(current->ldt[1])), get_segment_limit(0x000F));
	free_page_tables(get_segment_base(&(current->ldt[2])), get_segment_limit(0x0017));

	// 2.处理自己的子进程
	for(i = 1; i < NUMBER_OF_TASKS; i++)
	{
		if((tasks_ptr[i] != NULL) && (tasks_ptr[i]->father == current->pid))
		{
			tasks_ptr[i]->father = 1;	// adopted by TASK1
			if(tasks_ptr[i]->state == TASK_ZOMBIE)
				send_sig(SIGCHLD, tasks_ptr[1], 1);
		}
	}

	// 3.close the opened files
	//for (i=0 ; i<NR_OPEN ; i++)
	//{
	//	if (current->filp[i])
	//		sys_close(i);
	//}

	// 4.处理root和pwd
	//iput(current->pwd);
	//current->pwd=NULL;
	//iput(current->root);
	//current->root=NULL;
	//iput(current->executable);
	//current->executable=NULL;

	// 5.others
	//if (current->leader && current->tty >= 0)
	//	tty_table[current->tty].pgrp = 0;
	//if (last_task_used_math == current)
	//	last_task_used_math = NULL;
	
	if (current->leader)	// if is leader then send SIGHUP to all tasks in session
		kill_session();

	current->state = TASK_ZOMBIE;

	current->exit_code = code;

	tell_father(current->father);

	schedule();

	return 0;	// suppress warning from compiler
}


int32_t sys_exit(int32_t error_code)
{
	return do_exit((error_code & 0xFF) << 8);
}


