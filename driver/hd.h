#ifndef HDREG_H
#define HDREG_H

#include <sys/types.h>

/* hd controller port address */
#define HD_DATA		0x1F0
#define HD_ERROR	0x1F1
#define HD_PRECOMP HD_ERROR	// read=HD_ERROR, write=HD_PRECOMP
#define HD_NSECTOR	0x1F2
#define HD_SECTOR	0x1F3	// sector number note: starting from 1
#define HD_LCYL		0x1F4
#define HD_HCYL		0x1F5
#define HD_CURRENT	0x1F6
#define HD_STATUS	0x1F7
#define HD_COMMAND HD_STATUS// read=HD_STATUS, write=HD_COMMAND


/* bits of HD_STATUS*/
#define ERR_STAT	0x01
#define DRQ_STAT	0x08	// sector buffer requires servicing
#define SEEK_STAT	0x10
#define WR_ERR_STAT	0x20
#define READY_STAT	0x40
#define BUSY_STAT	0x80


/* commands to HD_COMMAND */
#define CMD_RESTORE		0x10	// did not find it in spec todo
#define CMD_READ		0x20	// read sectors with retry
#define CMD_WRITE		0x30	// write sectors with retry
#define CMD_INIT_DRIVE_PARAMETER		0x91

/*
 * 03F6	r/w	FIXED disk controller data register
		 bit 7-4    reserved
		 bit 3 = 0  reduce write current
			 1  head select 3 enable
		 bit 2 = 1  disk reset enable
			 0  disk reset disable
		 bit 1 = 0  disk initialization enable
			 1  disk initialization disable
		 bit 0	    reserved
 */
#define HD_CMD		0x3F6	// cmd register


struct hd_hw_info
{
	int32_t head;	// how many faces
	int32_t sect;	// how many sectors per track
	int32_t cyl;	// how many tracks per face
	int32_t wpcom;	// write compensation
	int32_t lzone;	// 磁头land 到哪个track
	int32_t ctl;	// 控制字节
};


struct hd_struct	// record partition start and end
{
	int32_t start_sect;
	int32_t nr_sects;
};

#endif //HDREG_H
