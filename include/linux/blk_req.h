#ifndef BLK_REQ_H
#define BLK_REQ_H

#include <sys/types.h>
#include <linux/fs.h>


#define BLK_DEV_KINDS	7

#define NUMBER_OF_REQUESTS	32

#define BLK_DEV_NUM_HD	3

#define BLK_READ 0
#define BLK_WRITE 1


struct blk_request
{
	int32_t dev;
	int32_t cmd;	// BLK_READ or BLK_WRITE
	int32_t errors;

	uint32_t sector;	// starting sector number (relative)
	uint32_t nr_sector;	// number of sector to read or write
	int8_t *buffer;	// usually points to buffer_head->b_data

	struct task_struct *wait_q;
	struct buffer_head *bh;
	struct blk_request *next;
};


struct blk_request_dev
{
	void (*do_req_fn)(void);	// function to do requests
	struct blk_request *req_q;	// request queue
};


extern struct blk_request_dev blk_dev[BLK_DEV_KINDS];
extern struct blk_request request[NUMBER_OF_REQUESTS];
extern struct task_struct * wait_for_request;

#endif //BLK_REQ_H
