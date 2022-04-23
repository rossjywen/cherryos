#include <linux/tty.h>
#include <sys/types.h>

#define ASC_CR	0xC		// 回车符
#define ASC_NL	0xA		// 换行符

#define ASC_DEL	0x1F	// 删除


// todo
struct tty_struct tty_table[] = 
{
	{
		{},		// struct termios
	}
};



void copy_to_cooked(struct tty_struct *tty)
{
	char c;

	while(!EMPTY(tty->read_q) && !FULL(tty->secondary))
	{
		c = tty->read_q.buf[tty->read_q.tail];
		INC(tty->read_q.tail);

		if(c == ASC_CR)												// 处理 回车符
		{
			if(tty->termios.c_iflag & I_CRNL)
			{
				c = ASC_NL;		// convert CR to NL
			}
			else if(tty->termios.c_iflag & I_IGNCR)
			{
				continue;		// ignore CR
			}
		}
		else if((c == ASC_NL) && (tty->termios.c_iflag & I_NLCR))	// 处理 换行符
		{
			c = ASC_CR;
		}
		else
		{
			;
		}

		if(tty->termios.c_iflag & I_UCLC)
		{
			;	// todo convert upper case letters to lower case letters
		}

		// 规范模式 字符经过处理后给到secondary
		// 原始模式 字符不经过处理直接发送给secondary
		if(tty->termios.c_iflag & L_ICANON)	
		{
			// kill character 
			if(c == tty->termios.c_cc[V_KILL])
			{
				while(1)	
				{
					// 除非满足下面3个条件 否则就挪动secondary队列的head指针
					// 1.标准化buf的head和tail已经重合 (已经为空了)
					// 2.标准化buf上一个字符是换行符 (也就是说要保留换行符)
					// 3.当前这个字符是cc[V_EOF]
					if(EMPTY(tty->secondary) == false)
						c = LAST(tty->secondary);
					else
						break;	// 队列空 不需要处理

					if((c == ASC_NL) || (c == tty->termios.c_cc[V_EOF]))
					{
						break;	// 上一个字符是换行符 不需要处理
					}
					else
					{
						// 1.确定拿的字符不是特殊字符&&队列不空 才把secondary的head指针挪动
						DEC(tty->secondary.head);

						// 2.如果启用了回显 向console发送特定字符 
						// console得知这些之后会取消显示效果
						if(tty->termios.c_lflag & L_ECHO)
						{
							if(c < 0x20)	// 从ASCII码可知 小于0x20的都不是显示字符 额外再做一次
							{				// 因为发送的时候就发送了两个字符 见下面
								tty->write_q.buf[tty->write_q.head] = ASC_DEL;
								INC(tty->write_q.head);
							}
							tty->write_q.buf[tty->write_q.head] = ASC_DEL;
							INC(tty->write_q.head);

							tty->write(tty);
						}
					}
				}
				continue;
			}

			// erase character 
			if(c == tty->termios.c_cc[V_ERASE])
			{
				if(EMPTY(tty->secondary) == false)
					c = LAST(tty->secondary);
				else
					continue;

				if((c == ASC_NL) || (c == tty->termios.c_cc[V_EOF]))
				{
					continue;
				}
				else
				{
					DEC(tty->secondary.head);

					if(tty->termios.c_lflag & L_ECHO)
					{
						if(c < 0x20)
						{
							tty->write_q.buf[tty->write_q.head] = ASC_DEL;
							INC(tty->write_q.head);
						}
						tty->write_q.buf[tty->write_q.head] = ASC_DEL;
						INC(tty->write_q.head);

						tty->write(tty);
					}
				}
				continue;
			}
			// stop character 
			if(c == tty->termios.c_cc[V_STOP])
			{
				tty->stopped = 1;
				continue;
			}

			// start character
			if(c == tty->termios.c_cc[V_START])
			{
				tty->stopped = 0;
				continue;
			}
		}

		// 这个标志位表示可以通过键盘发送信号signal todo
		if(tty->termios.c_lflag & L_ISIG)
		{
			;
		}

		if((c == ASC_NL) || (c == tty->termios.c_cc[V_EOF]))
		{
			tty->secondary.data++;	// 记录换行符的数量
		}

		// 到这里开始处理和显示 能显示的 字符
		if(tty->termios.c_lflag & L_ECHO)
		{
			// 处理换行符的方式是发送一个换行和一个回车
			if(c == ASC_NL)
			{
				tty->write_q.buf[tty->write_q.head] = ASC_NL;
				INC(tty->write_q.head);

				tty->write_q.buf[tty->write_q.head] = ASC_CR;
				INC(tty->write_q.head);
			}
			// 处理非显示ASCII字符 发送一个'^'和一个代表其含义的字母
			else if(c < 0x20)
			{
				tty->write_q.buf[tty->write_q.head] = '^';
				INC(tty->write_q.head);

				tty->write_q.buf[tty->write_q.head] = c + 64;
				INC(tty->write_q.head);
			}
			// 处理显示ASCII字符
			else
			{
				tty->write_q.buf[tty->write_q.head] = c;
				INC(tty->write_q.head);
			}
		}

		// 最后 把c更新到secondary中
		tty->secondary.buf[tty->secondary.head] = c;
		INC(tty->write_q.head);
	}

	// todo wake up wait_q, 因为这些进程要读取secondary中的内容
	//wake_up()
}


//uint32_t tty_write(uint8_t channel, char *buf, uint32_t nr)
//{
//
//}


void do_tty_interrupt(uint32_t tty_index)
{
	copy_to_cooked(tty_table + tty_index);
}


