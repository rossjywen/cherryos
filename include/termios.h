#ifndef TERMIOS_H
#define TERMIOS_H

#define NCCS 17


// iflag
#define I_CRNL		0x400	// 回车符 转换成 换行符
#define I_IGNCR		0x200	// 忽略 回车符 如果先设置了I_CRNL 则忽略这个条件

#define I_NLCR		0x100	// 换行符 转换成 回车符

#define I_UCLC		0x1000	// 把大写字母转换成小写字母


// oflag
#define O_POST		0x001	// 执行输出处理 这个flag置位下面的flag才生效 所以这个位一定置位
#define O_LCUC		0x002	// 把小写字母转换成大写字母然后输出
#define O_CRNL		0x010	// 把回车转换成换行
#define O_NLRET		0x040	// 把换行转换成回车
#define O_NLCR		0x004	// tty对外输出的时候(也就是外部读取tty内容的时候)
							// 把换行符实际实现为 换行 + 回车 的效果
							// 这个标志位配合 I_CRNL就把 回车符 和 换行符 都实现为 换行 + 回车 的效果

// lflag
#define L_ISIG		0x001	// 当收到特定字符的时候发送信号 
#define L_ICANON	0x002	// 规范模式 一个字符一个字符处理 (通常都使用这个flag)
#define L_ECHO		0x010	// 回显 键盘输入的信息也会显示在console上
#define L_ECHOCTL	0x1000	// 若设置了回显 除了TAB CR(NL) START STOP外
							// 其他控制字符都显示为 ^ + 字符+0x40对应的字符
#define L_ECHOKE	0x4000	// 若设置了回显 被删除的字符也会在屏幕上被删除

// control character index in c_cc[NCCS]
#define V_INTR		0	// 向进程发出中断信号
#define V_QUIT		1	// 向进程发出退出信号
#define V_ERASE		2	// 擦除一个字符 (包括非显示字符)
#define V_KILL		3	// 删除整行字符
#define	V_EOF		4	// end of file
#define V_TIME		5	// 非规范模式使用的超时 (未实现这个功能)
#define V_MIN		6	// 非规范模式使用的最小输入格式 (未实现这个功能)
#define V_SWTC		7	// 交换字符 未实现这个功能
#define V_START		8	// 终端开始工作
#define V_STOP		9	// 终端终止工作
//#define V_SUSP	10	// 向进程发出挂起信号
//#define V_EOL		11	// 行结束字符
//#define V_REPRINT	12	// 重显示字符
//#define V_DISCARD	13
//#define V_WERASE	14
//#define V_LNEXT	15
//#define V_EOL2	16




struct termios
{
	uint32_t c_iflag;		// input flag
	uint32_t c_oflag;		// output flag
	uint32_t c_cflag;		// control flag
	uint32_t c_lflag;		// local flag
	uint8_t c_line;	// todo	
	uint8_t c_cc[NCCS];		// control characters
};

#endif //TERMIOS_H

