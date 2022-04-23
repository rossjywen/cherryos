#include <sys/types.h>
#include <linux/tty.h>
#include <asm/io.h>
#include <asm/system.h>


/* I watch the below values of bochs returns 
 * 0x90004 -> 0x00 (current page = 0)
 * 0x90006 -> 0x03 (type:A/N, max colors:16, alpha format:80x25, mem starts 0xB8000)
 * 0x90007 -> 0x50 (characters per-line = 80)
 * 0x9000A -> 0x03 (video memory size = 256K)
 * 0x9000B -> 0x00 (Color mode in effect, 30x address range. if 0x01, is Monochrome)
 * 0x9000C -> 0x09 
 * 0x9000B -> 0x00 
 */
#define BIOS_CUR_POS_X			(*((uint8_t*)0x90000))
#define BIOS_CUR_POS_Y			(*((uint8_t*)0x90001))
#define BIOS_PAGE_CNT			(*((uint16_t*)0x90004))
#define BIOS_DISP_MODE			(*((uint8_t*)0x90006))
#define BIOS_CHR_CNT_PER_LINE	(*((uint8_t*)0x90007))
//#define 		(*((uint8_t*)0x90008))
//#define 		(*((uint8_t*)0x9000A))
#define BIOS_COLOR_MODE			(*((uint8_t*)0x9000B))
//#define 		(*((uint8_t*)0x9000C))
//#define 		(*((uint8_t*)0x9000D))

/*
 * linux0.11 kernel supports 4 types as below
 * but I think for me it is not necessary, because MDA and CGA are too old
 * so I only support EGA which is the "newest"
 * the history of MDA CGA EGA is that
 * MDA -> CGA -> EGA(1984) -> VGA(1987)
 */
//#define VIDEO_TYPE_MDA		0x10	/* Monochrome Text Display	*/
//#define VIDEO_TYPE_CGA		0x11	/* CGA Display 			*/
//#define VIDEO_TYPE_EGAM		0x20	/* EGA/VGA in Monochrome Mode	*/
#define VIDEO_TYPE_EGAC		0x21	/* EGA/VGA in Color Mode	*/


static uint8_t display_char_per_line;		// 每行字符的数量 但是一个字符是由2个字节表示
static uint32_t size_per_line;				// 所以每行的数据宽度是 size_per_line = char_cnt_per_line * 2
static uint8_t display_line_nr = 25;


static uint32_t video_mem_start_addr;			// 显存起始地址 取决于BIOS检测出来的显卡类型
static uint32_t video_mem_end_addr;

static uint16_t video_reg_port;
static uint16_t video_val_port;

static uint8_t video_type;


static uint32_t state = 0;

#define BLANK	0x0720

// 下面的变量为了向上或向下移动屏幕 典型的就是ASCII中的LF和RI
static uint32_t scroll_origin_mem;			// 当前屏幕起始显存地址 (会写入mc6845的 r12 r13)
static uint32_t scroll_end_mem;			// 当前屏幕截止显存地址 (scroll_origin_mem+整个屏幕的字节数)

// 下面的变量为了定位光标
static uint32_t cur_pos_mem;		// 光标所在地址 (scroll_origin_mem+ pos_x pos_y偏移的地址)
static uint8_t cur_pos_x;			// 光标坐标 x y
static uint8_t cur_pos_y;			// (注意 坐标的单位是屏幕显示的字符 一个字符是两个字节)
//static uint8_t cur_pos_y_top;		// 光标坐标y的最小值 如果小于这个值就需要 scroll_down()
//static uint8_t cur_pos_y_bottom;	// 光标坐标y的最大值 如果大于这个值就需要 scroll_up()


static void update_cursor(void)
{
/*
	格式和R12 R13相同
*/
	disable_interrupt();

	out_b(14, video_reg_port);
	out_b(((cur_pos_mem - video_mem_start_addr) >> 9) & 0xFF, video_val_port);
	out_b(15, video_reg_port);
	out_b(((cur_pos_mem - video_mem_start_addr) >> 1) & 0xFF, video_val_port);

	enable_interrupt();
}

static uint32_t cur_goto_pos(uint8_t new_x, uint8_t new_y)
{
	if((new_x > display_char_per_line) || (new_y > display_line_nr))
		return 0;

	cur_pos_x = new_x;
	cur_pos_y = new_y;
	cur_pos_mem = scroll_origin_mem + size_per_line * cur_pos_y + cur_pos_x * 2;

	return 1;
}


static void update_scroll(void)
{
/*
	MC6845 programing set R12 R13
	(R12 R13) is for fast-scroll
	R12 is high 6-bit
	R13 is low 8-bit
	注意 实际上0xb8000 偏移一共是15-bit
	但每一个字符是2个字节 第0位一定是0
	所以直接把第0位省略
*/

	disable_interrupt();

	out_b(12, video_reg_port);		// specify R12
	out_b(((scroll_origin_mem - video_mem_start_addr) >> 9) & 0xFF, video_val_port);
	out_b(13, video_reg_port);		// specify R13
	out_b(((scroll_origin_mem - video_mem_start_addr) >> 1) & 0xFF, video_val_port);

	enable_interrupt();
}


static void scroll_up(void)
{
/* 	imagin like using a phone to scroll up with finger
	if scroll up means will show the content downward

	1. update scroll_origin_mem scroll_end_mem
	2. pad the new line with blank 0x0720
	3. update cur_pos_mem
*/

	uint32_t i;

	// need to move whole screen data to video_start_mem
	if(scroll_end_mem + size_per_line > video_mem_end_addr)
	{
		// 1.从过滤掉开始 移动到整屏末尾
		scroll_origin_mem += size_per_line;	// 先执行这条 过滤掉滚动掉的首行
		for(i = 0; i < display_char_per_line * (display_line_nr - 1); i++)
		{
			*((uint16_t*)(video_mem_start_addr) + i) = *((uint16_t*)(scroll_origin_mem) + i);
		}

		// 2.填充BLANK
		scroll_origin_mem = video_mem_start_addr;
		scroll_end_mem = scroll_origin_mem + size_per_line * display_line_nr;
		for(i = 0; i < display_char_per_line; i++)
		{
			*((uint16_t*)(scroll_end_mem - size_per_line) + i) = BLANK;
		}

		cur_pos_mem = scroll_origin_mem + cur_pos_y * size_per_line + cur_pos_x * 2; // scroll不会造成x y变化 只会造成cur_pos_mem变化
	}
	// do not need to move data
	else
	{
		for(i = 0; i < display_char_per_line; i++)
		{
			*((uint16_t*)(scroll_end_mem) + i) = BLANK;
		}

		scroll_origin_mem += size_per_line;
		scroll_end_mem += size_per_line;

		cur_pos_mem += size_per_line;
	}

	update_scroll();
}


static void cr(void)
{
/*
	回车符 移动到行的最左边
	这也是为什么要把回车变成换行+回车的原因
	单独一个回车是没有意义的
*/
	cur_pos_mem -= cur_pos_x * 2;

	cur_pos_x = 0;
}

// 换行符
// 这个函数只更新cur_pos_y以及对cur_pos_mem的影响
// 所以cur_pos_x的更新以及对cur_pos_mem的影响需要程序外额外计算
static void lf(void)
{
	if(cur_pos_y + 1 <= display_line_nr)
	{
		cur_pos_y += 1;
		cur_pos_mem += size_per_line;
	}
	else
	{
		scroll_up();
	}
}

// 删除符
static void del(void)
{
	if(cur_pos_x == 0)
	{
		return;		// 如果已经到了1行的最左端就忽略delete
	}
	else
	{
		cur_pos_x--;
		cur_pos_mem -= 2;

		*((uint16_t*)cur_pos_mem) = BLANK;	// 把之前写入的字符变成BLANK
	}
}


// step 1 detector display parameters according to data from BIOS
// step 2 display a brief information at top-right corner
void console_init(void)
{
	char *brief_info = "EGAC";
	uint8_t i;

	display_char_per_line = BIOS_CHR_CNT_PER_LINE;

	size_per_line = display_char_per_line * 2;

	video_mem_start_addr = 0xB8000;		// BIOS book information
	video_mem_end_addr = 0xBC000;

	video_reg_port = 0x3D4;			// use CGA(Color Graphics Adapter) ports
	video_val_port = 0x3D5;

	video_type = VIDEO_TYPE_EGAC;
	
	for(i=0; i<4; i++)
	{
		// every 2-bytes represents a character
		((uint8_t*)(video_mem_start_addr + display_char_per_line * 2 - 4 * 2))[i * 2] = brief_info[i];
	}

	// set scroll related variables
	scroll_origin_mem = video_mem_start_addr;
	scroll_end_mem = scroll_origin_mem + size_per_line * display_line_nr;

	// set cursor related variables
	cur_goto_pos(BIOS_CUR_POS_X, BIOS_CUR_POS_Y);

	// todo keyboard interrupt
}


//void console_write(struct tty_struct *tty)
void console_write(char* string, uint32_t nr)
{
	//uint32_t nr;
	char c;
	uint32_t i = 0;
	uint16_t wr_c = 0x0700;
	uint8_t ht_offset;

	//nr = READ_SPACE(tty->write_q);

	while(nr != 0)
	{
		nr--;

		//c = tty->write_q.buf[tty->write_q.tail];	// read a charactor from write_q
		//INC(tty->write_q.tail);
		c = string[i];
		i++;

		switch(state)
		{
			// 处理正常字符
			case 0:
				if(c > 0x1F && c < 0x7F)	// this means the displayable ASCII character
				{
					// detect whether at boundary of a line
					if(cur_pos_x >= display_char_per_line)
					{
						cur_pos_x -= display_char_per_line;
						cur_pos_mem -= size_per_line;
						lf();	// equivalent to issue a LF
					}
					// write to corresponding video-memory address
					*((uint16_t*)cur_pos_mem) = wr_c + (uint8_t)c;

					// update cursor
					cur_pos_x++;
					cur_pos_mem += 2;
				}
				else if(c == 0x1B)			// 0x1B is ESC
				{
					// escape byte indicates a sequence
					state = 1;
				}
				else if(c == 0x0A || c == 0x0B || c == 0x0C)	// 1.0xA 换行符 2.0xB 垂直制表符 3.0xC 换页符
				{												// 都当换行符来处理
					lf();
				}
				else if(c == 0x0D)			// 0x0D 回车符
				{
					cr();
				}
				//else if(c == tty->termios.c_cc[V_ERASE])	// 删除符
				//{
				//	del();
				//}
				else if(c == 0x8)	// 0x08 backspace
				{
					// backspace的效果就是光标往左移动一个
					if(cur_pos_x != 0)
					{
						cur_pos_x--;
						cur_pos_mem -= 2;
					}
				}
				else if(c == 0x9)	// 0x09 horizontal tabs 水平制表符
				{
					// 光标移动到8的倍数上的横坐标
					ht_offset = 8 - (cur_pos_x & 7);

					if(cur_pos_x + ht_offset > display_char_per_line)
					{
						cur_pos_x -= display_char_per_line - ht_offset;
						cur_pos_mem -= (display_char_per_line - ht_offset) * 2;
						lf();
					}
					else
					{
						cur_pos_x += ht_offset;
						cur_pos_mem += ht_offset * 2;
					}
				}
				else if(c == 7)		// 0x07 蜂鸣器
				{
					// todo beep
					;
				}

				break;
			// 处理
			case 1:
				break;
			// 处理
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
			default:
				break;
		}
	}

	update_cursor();
}


