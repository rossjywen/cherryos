#include <sys/types.h>


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


static uint8_t cur_pos_x, cur_pos_y;
static uint8_t char_cnt_per_line;

static ptr_t video_mem_start, video_mem_end;		// it depends on MDA or CGA	
static uint16_t video_port_reg, video_port_val;		// it depends on MDA or CGA

static uint8_t video_type;

// step 1 detector display parameters according to data from BIOS
// step 2 display a brief information at top-right corner
void console_init(void)
{
	char *brief_info = "EGAC";
	uint8_t i;

	cur_pos_x = BIOS_CUR_POS_X;
	cur_pos_y = BIOS_CUR_POS_Y;
	char_cnt_per_line = BIOS_CHR_CNT_PER_LINE;

	video_mem_start = 0xB8000;		// due to book, 
	video_mem_end = 0xBC000;
	video_port_reg = 0x3D4;			// use CGA(Color Graphics Adapter) ports
	video_port_val = 0x3D5;
	video_type = VIDEO_TYPE_EGAC;
	
	for(i=0; i<4; i++)
	{
		// every 2-bytes represents a character
		((uint8_t*)(video_mem_start + char_cnt_per_line * 2 - 8))[i*2] = brief_info[i];
	}

	// todo set origin
	// todo keyboard interrupt
}


