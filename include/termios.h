#ifndef TERMIOS_H
#define TERMIOS_H

#define NCCS 17


// iflag
#define I_CRNL		0x400	// 回车符 转换成 换行符
#define I_IGNCR		0x200	// 忽略 回车符 如果先设置了I_CRNL 则忽略这个条件

#define I_NLCR		0x100	// 换行符 转换成 回车符

#define I_UCLC		0x1000	// 把大写字母转换成小写字母


// oflag


// lflag
#define L_ISIG		0x001
#define L_ICANON	0x002
#define L_ECHO		0x010


// control character index in c_cc[NCCS]
//#define 
//#define
#define V_ERASE		2	// 擦出一个字符 (包括非显示字符)
#define V_KILL		3	// 删除整行字符
#define	V_EOF		4	// end of file
//#define
//#define
//#define
#define V_START		8
#define V_STOP		9
//#define
//#define
//#define
//#define
//#define
//#define




struct termios
{
	uint32_t c_iflag;		// input flag
	uint32_t c_oflag;		// output flag
	uint32_t c_cflag;
	uint32_t c_lflag;
	//uint8_t c_line;
	uint8_t c_cc[NCCS];
};

#endif //TERMIOS_H

