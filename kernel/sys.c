#include <linux/head.h>
#include <asm/system.h>
#include <errno.h>


#define NR_OF_SYSCALLS	72

//int sys_setup();
//int sys_exit();
int sys_fork();
//int sys_read();
//int sys_write();
//int sys_open();
//int sys_close();
//int sys_waitpid();
//int sys_creat();
//int sys_link();
//int sys_unlink();
//int sys_execve();
//int sys_chdir();
//int sys_time();
//int sys_mknod();
//int sys_chmod();
//int sys_chown();
int sys_break();
//int sys_stat();
//int sys_lseek();
int sys_getpid();
//int sys_mount();
//int sys_umount();
//int sys_setuid();
int sys_getuid();
//int sys_stime();
int sys_ptrace();
int sys_alarm();
//int sys_fstat();
//int sys_pause();
//int sys_utime();
int sys_stty();
int sys_gtty();
//int sys_access();
int sys_nice();
int sys_ftime();
//int sys_sync();
//int sys_kill();
int sys_rename();
//int sys_mkdir();
//int sys_rmdir();
//int sys_dup();
//int sys_pipe();
//int sys_times();
int sys_prof();
//int sys_brk();
//int sys_setgid();
int sys_getgid();
int sys_signal();
int sys_geteuid();
int sys_getegid();
int sys_acct();
int sys_phys();
int sys_lock();
//int sys_ioctl();
//int sys_fcntl();
int sys_mpx();
//int sys_setpgid();
int sys_ulimit();
//int sys_uname();
//int sys_umask();
//int sys_chroot();
//int sys_ustat();
//int sys_dup2();
int sys_getppid();
//int sys_getpgrp();
//int sys_setsid();
int sys_sigaction();
int sys_sgetmask();
int sys_ssetmask();
//int sys_setreuid();
//int sys_setregid();

typedef int (*fn_ptr)();


fn_ptr system_call_table[NR_OF_SYSCALLS];	// 注意 这个数组中元素的下标一定要和unistd.h中的__NR##func_name 相对应

void system_call(void);

void system_call_init(void)
{
	//system_call_table[0] = sys_setup;
	//system_call_table[1] = sys_exit;
	system_call_table[2] = sys_fork;
	//system_call_table[3] = sys_read;
	//system_call_table[4] = sys_write;
	//system_call_table[5] = sys_open;
	//system_call_table[6] = sys_close;
	//system_call_table[7] = sys_waitpid;
	//system_call_table[8] = sys_creat, 
	//system_call_table[9] = sys_link,
	//system_call_table[10] = sys_unlink, 
	//system_call_table[11] = sys_execve, 
	//system_call_table[12] = sys_chdir, 
	//system_call_table[13] = sys_time, 
	//system_call_table[14] = sys_mknod, 
	//system_call_table[15] = sys_chmod,
	//system_call_table[16] = sys_chown, 
	system_call_table[17] = sys_break;
	//system_call_table[18] = sys_stat, 
	//system_call_table[19] = sys_lseek, 
	system_call_table[20] = sys_getpid;
	//system_call_table[21] = sys_mount,
	//system_call_table[22] = sys_umount, 
	//system_call_table[23] = sys_setuid, 
	system_call_table[24] = sys_getuid;
	//system_call_table[25] = sys_stime, 
	system_call_table[26] = sys_ptrace;
	system_call_table[27] = sys_alarm;
	//system_call_table[28] = sys_fstat, 
	//system_call_table[29] = sys_pause, 
	//system_call_table[30] = sys_utime, 
	system_call_table[31] = sys_stty;
	system_call_table[32] = sys_gtty;
	//system_call_table[33] = sys_access,
	system_call_table[34] = sys_nice;
	system_call_table[35] = sys_ftime;
	//system_call_table[36] = sys_sync,
	//system_call_table[37] = sys_kill, 
	system_call_table[38] = sys_rename;
	//system_call_table[39] = sys_mkdir,
	//system_call_table[40] = sys_rmdir,
	//system_call_table[41] = sys_dup, 
	//system_call_table[42] = sys_pipe, 
	//system_call_table[43] = sys_times, 
	system_call_table[44] = sys_prof;
	//system_call_table[45] = sys_brk, 
	//system_call_table[46] = sys_setgid,
	system_call_table[47] = sys_getgid;
	system_call_table[48] = sys_signal;
	system_call_table[49] = sys_geteuid;
	system_call_table[50] = sys_getegid;
	system_call_table[51] = sys_acct;
	system_call_table[52] = sys_phys;
	system_call_table[53] = sys_lock;
	//system_call_table[54] = sys_ioctl, 
	//system_call_table[55] = sys_fcntl, 
	system_call_table[56] = sys_mpx;
	//system_call_table[57] = sys_setpgid, 
	system_call_table[58] = sys_ulimit;
	//system_call_table[59] = sys_uname, 
	//system_call_table[60] = sys_umask, 
	//system_call_table[61] = sys_chroot, 
	//system_call_table[62] = sys_ustat, 
	//system_call_table[63] = sys_dup2, 
	system_call_table[64] = sys_getppid;
	//system_call_table[65] = sys_getpgrp, 
	//system_call_table[66] = sys_setsid, 
	system_call_table[67] = sys_sigaction;
	system_call_table[68] = sys_sgetmask;
	system_call_table[69] = sys_ssetmask,
	//system_call_table[70] = sys_setreuid,
	//system_call_table[71] = sys_setregid 

	set_gate(&idt, 0x80, INTERRUPT_GATE, KERNEL_CS, &system_call, DPL_3);
}

int sys_ftime()
{
	return -ENOSYS;
}

int sys_break()
{
	return -ENOSYS;
}

int sys_ptrace()
{
	return -ENOSYS;
}

int sys_stty()
{
	return -ENOSYS;
}

int sys_gtty()
{
	return -ENOSYS;
}

int sys_rename()
{
	return -ENOSYS;
}

int sys_prof()
{
	return -ENOSYS;
}

int sys_acct()
{
	return -ENOSYS;
}

int sys_phys()
{
	return -ENOSYS;
}

int sys_lock()
{
	return -ENOSYS;
}

int sys_mpx()
{
	return -ENOSYS;
}

int sys_ulimit()
{
	return -ENOSYS;
}

