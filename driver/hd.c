#include <sys/types.h>
#include <asm/io.h>
#include <asm/system.h>
#include <linux/blk_req.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include "hd.h"

#define MAX_ERRORS	7
#define MAX_HD	2

#define DEVICE_NAME "harddisk"

#define port_read(port,buf,nr) asm("cld;rep;insw"::"d" (port),"D" (buf),"c" (nr))
#define port_write(port,buf,nr) asm("cld;rep;outsw"::"d" (port),"S" (buf),"c" (nr))

#define CURR_REQ	(blk_dev[BLK_DEV_NUM_HD].req_q)
#define CURR_DEV	(blk_dev[BLK_DEV_NUM_HD].req_q->dev & 0x3)

static int32_t reset = 1;
static int32_t recalibrate = 1;

static int32_t number_of_hd = 0;	// will be initilized by sys_setup()

struct hd_hw_info hd_hw[MAX_HD] = {{0,0,0,0,0,0}, {0,0,0,0,0,0}};

/*
 * hd[0] is first whole hard disk sect range -> /dev/hd0
 * hd[1:4] are partitions sect ranges of first hard disk -> /dev/hd{1-4}
 * same is to hd[5] and hd[6:0]
 */
struct hd_struct hd[MAX_HD * 5] = {{0,0},};

void (*do_hd)(void) = NULL;


void hd_interrupt(void);	// in kernel/system_call.s
void do_hd_request(void);


/*
 * 1.update buffer head 'uptodate' field
 * 2.recyle request entry and wake up waiting tasks
 * 3.grab next request
 */
static inline void end_request(int uptodate)
{
	if (CURR_REQ->bh)
	{
		CURR_REQ->bh->b_uptodate = uptodate;
		unlock_buffer(CURR_REQ->bh);
	}
	if (!uptodate)
	{
		printk(DEVICE_NAME " I/O error\n\r");
		printk("dev %04x, block %d\n\r",CURR_REQ->dev,CURR_REQ->bh->b_blocknr);
	}
	wake_up(&CURR_REQ->wait_q);	// 好像代码中没有sleep_on这个q的地方 todo
	wake_up(&wait_for_request);
	CURR_REQ->dev = -1;	// set this request to idle
	CURR_REQ = CURR_REQ->next;	// grab next request
}


static int controller_ready(void)
{
	int retries=10000;	//尝试次数

	while (--retries && (in_b(HD_STATUS) & 0xc0) != 0x40) ;

	return (retries);
}

/*
 * p1 which drive 0/1
 * p2 amount of sects
 * p3 starting sect
 * p4 starting head
 * p5 starting cyclinder
 * p6 command to HD_COMMAND
 * p7 function that interrupt handler will invoke
 */
static void hd_out(uint32_t drive, uint32_t nsect, uint32_t sect,
		uint32_t head, uint32_t cyl, uint32_t cmd, void (*intr_addr)(void))
{
//	printk("hd_out drive: %d\n", drive);
//	printk("hd_out nsect: %d\n", nsect);
//	printk("hd_out sect: %d\n", sect);
//	printk("hd_out head: %d\n", head);
//	printk("hd_out cyl: %d\n", cyl);
//	printk("hd_out cmd: 0x%X\n", cmd);

	if(drive >= 2 || head > 15)
		panic("Trying to write bad sector");
	if(!controller_ready())
		panic("HD controller not ready");

	do_hd = intr_addr;

	out_b(hd_hw[drive].ctl, HD_CMD);
	out_b(hd_hw[drive].wpcom / 4, HD_PRECOMP);	// early time HD consider this field
	out_b(nsect, HD_NSECTOR);
	out_b(sect, HD_SECTOR);
	out_b(cyl, HD_LCYL);
	out_b(cyl >> 8, HD_HCYL);
	out_b(0xA0|(drive<<4)|head, HD_CURRENT);
	out_b(cmd, HD_COMMAND);
}

static void bad_rw_intr(void)
{
	CURR_REQ->errors++;
	if(CURR_REQ->errors >= MAX_ERRORS)
		end_request(0);	// 没必要抢救了
	if(CURR_REQ->errors > MAX_ERRORS / 2)
		reset = 1;	// 还要抢救一下
}


static int32_t drive_busy(void)
{
	int32_t i;

	for (i = 0; i < 10000; i++)	//有一个小的等待时间 查询10000次
		if (READY_STAT == (in_b(HD_STATUS) & (BUSY_STAT | READY_STAT)))	//ready位置位了 且 busy位复位了
			break;
	i = in_b(HD_STATUS);	//读取主控制寄存器
	i &= BUSY_STAT | READY_STAT | SEEK_STAT; //分别读取 忙 就绪 寻道结束 3个标志位
	if (i == (READY_STAT | SEEK_STAT))	//如果ready和寻道结束同时置位 busy复位 说明不忙 返回0
		return(0);
	printk("HD controller times out\n\r");
	return(1);	//否则打印信息后返回1
}


static void reset_controller(void)
{
	int32_t i;

	out_b(1 << 2, HD_CMD);	// bit 2 = 1  disk reset enable
	for(i = 0; i < 100; i++)
		asm("nop"::);
	out_b(hd_hw[0].ctl & 0xF, HD_CMD);	// write control byte to HD_CMD

	if(drive_busy())
		printk("HD-controller still busy\n");
	if ((i = in_b(HD_ERROR)) != 1)	// diagnostic mode 001->no error detected
		printk("HD-controller reset failed: %02x\n",i);
}


static int cmd_result(void)
{
	int i=in_b(HD_STATUS);

	if ((i & (BUSY_STAT | READY_STAT | WR_ERR_STAT | SEEK_STAT | ERR_STAT))
		== (READY_STAT | SEEK_STAT))
		return(0); /* ok */

	if (i&1)
		i=in_b(HD_ERROR);

	return (1);
}


static void recal_intr(void)
{
	if (cmd_result())
		bad_rw_intr();
	do_hd_request();
}


static void read_intr(void)
{
	if (cmd_result())
	{
		bad_rw_intr();
		do_hd_request();
		return;
	}
	port_read(HD_DATA,CURR_REQ->buffer,256);
	CURR_REQ->errors = 0;
	CURR_REQ->buffer += 512;
	CURR_REQ->sector += 1;
	if (--CURR_REQ->nr_sector)	// 检测是否已经读完
	{
		do_hd = read_intr;	// 如果没读完 就不会调用end_request()
		return;	// 直接return 等待下次被中断调用
	}
	end_request(1);	// grab next request
	do_hd_request();
}


static void write_intr(void)
{
	if (cmd_result())
	{
		bad_rw_intr();
		do_hd_request();
		return;
	}
	if (--CURR_REQ->nr_sector)	// 因为第一次写是在do_hd_request()中完成的 所以这里直接前--
	{
		CURR_REQ->sector += 1;
		CURR_REQ->buffer += 512;
		do_hd = write_intr;
		port_write(HD_DATA,CURR_REQ->buffer,256);
		return;
	}
	end_request(1);	// grab next request
	do_hd_request();
}


static void reset_hd(int32_t nr)
{
	reset_controller();
	hd_out(nr, hd_hw[nr].sect, hd_hw[nr].sect, hd_hw[nr].head-1, \
			hd_hw[nr].cyl, CMD_INIT_DRIVE_PARAMETER, &recal_intr);
}


void unexpected_hd_interrupt(void)
{
	printk("Unexpected HD interrupt\n\r");
}


void do_hd_request(void)
{
	uint8_t dev_minor, dev;
	uint32_t rel_block, abs_block;
	uint32_t sec, head, cyl, nsect;
	int32_t i, r;

repeat:
	if(CURR_REQ == NULL)	// means queue is empty
		return;

	if (MAJOR(CURR_REQ->dev) != BLK_DEV_NUM_HD)
		panic(DEVICE_NAME ": request list destroyed");

	if (blk_dev[BLK_DEV_NUM_HD].req_q->bh && \
			!blk_dev[BLK_DEV_NUM_HD].req_q->bh->b_lock)
		panic(DEVICE_NAME ": block not locked");

	dev_minor = MINOR(CURR_REQ->dev);
	rel_block = CURR_REQ->sector;
	if(dev_minor >= number_of_hd*5 || rel_block+2 > hd[dev_minor].nr_sects)
	{
		// reaching here means error
		end_request(0);
		goto repeat;
	}

	// start to calculate CHS
	abs_block = rel_block + hd[dev_minor].start_sect;
	dev = dev_minor / 5;

	sec = abs_block % hd_hw[dev].sect;
	sec += 1;	// sect number starts from 1, same as floppy, see bootsect.s
	cyl = (abs_block / hd_hw[dev].sect) / hd_hw[dev].head;
	head = (abs_block / hd_hw[dev].sect) % hd_hw[dev].head;
	nsect = CURR_REQ->nr_sector;

	if(reset)
	{
		reset = 0;
		recalibrate = 1;
		reset_hd(dev);
		return;
	}
	if(recalibrate)
	{
		recalibrate = 0;
		hd_out(dev, hd_hw[dev].sect, 0, 0, 0, CMD_RESTORE, recal_intr);
		return;
	}
	if(CURR_REQ->cmd == BLK_WRITE)
	{
		hd_out(dev, nsect, sec, head, cyl, CMD_WRITE, write_intr);
		for(i=0 ; i<3000; i++)
			if((r=in_b(HD_STATUS) & DRQ_STAT))	// wait controller set DRQ_STAT to inform ready to receive data
				break;
		if(!r)
		{
			bad_rw_intr();
			goto repeat;
		}
		port_write(HD_DATA,CURR_REQ->buffer,256);	// first time write, remainings done in write_intr()
	}
	else if(CURR_REQ->cmd == BLK_READ)
	{
		hd_out(dev, nsect, sec, head, cyl, CMD_READ, read_intr);
	}
	else
	{
		panic("unknown hd-command");
	}
}


void hd_init(void)
{
	blk_dev[BLK_DEV_NUM_HD].do_req_fn = do_hd_request;
	set_gate(&idt, 0x2E, INTERRUPT_GATE, KERNEL_CS, &hd_interrupt, DPL_0);
	out_b(in_b(0x21)&0xfb,0x21);
	out_b(in_b(0xA1)&0xbf,0xA1);
}

