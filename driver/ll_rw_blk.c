#include <sys/types.h>
#include <asm/system.h>
#include <linux/fs.h>
#include <linux/blk_req.h>
#include <linux/kernel.h>
#include <linux/sched.h>


struct blk_request request[NUMBER_OF_REQUESTS];

struct task_struct * wait_for_request = NULL;

struct blk_request_dev blk_dev[BLK_DEV_KINDS] =	// index is dev MAJOR number
{
	{ NULL, NULL },		/* no_dev */
	{ NULL, NULL },		/* dev mem */
	{ NULL, NULL },		/* dev fd */
	{ NULL, NULL },		/* dev hd */
	{ NULL, NULL },		/* dev ttyx */
	{ NULL, NULL },		/* dev tty */
	{ NULL, NULL }		/* dev lp */
};


static inline void lock_buffer(struct buffer_head * bh)
{
	disable_interrupt();

	while (bh->b_lock)
		sleep_on(&bh->b_wait);
	bh->b_lock=1;

	enable_interrupt();
}


static inline void unlock_buffer(struct buffer_head * bh)
{
	if (!bh->b_lock)
		printk("ll_rw_block.c: buffer not locked\n");
	bh->b_lock = 0;
	wake_up(&bh->b_wait);
}


static void add_request(struct blk_request_dev * dev, struct blk_request * req)
{
	struct blk_request *tmp;

	disable_interrupt();

	req->next = NULL;
	req->bh->b_dirt = 0;

	tmp = dev->req_q;
	if(tmp == NULL)	// means the queue is empty now
	{
		tmp = req;	// just assign to req
		enable_interrupt();
		dev->do_req_fn();
		return;
	}

	// reaching here, the queue is not empty, has to add req to queue
	// I do not employ elevator algorithm, I do it simply add it to tail
	while(1)
	{
		if(tmp->next == NULL)
			break;
		tmp = tmp->next;
	}
	tmp->next = req;

	enable_interrupt();
}


static void make_request(int32_t major, int32_t rw, struct buffer_head * bh)
{
	struct blk_request *req;


	if(rw != BLK_READ && rw != BLK_WRITE)
		panic("Bad block dev command, must be R/W/RA/WA");

	lock_buffer(bh);

	// reaching here, bh has been locked
	if((rw == BLK_WRITE && !bh->b_dirt) || (rw == BLK_READ && bh->b_uptodate))
	{
		unlock_buffer(bh);	// that means the buffer contains valid info
		return;
	}

// reaching here, need to add to request table
repeat:
	req = request + NUMBER_OF_REQUESTS;
	while (--req >= request)
	{
		if (req->dev < 0)
			break;
	}
	if (req < request)	// did not find idle entry
	{
		sleep_on(&wait_for_request);
		goto repeat;
	}

	// reaching here, idle entry has been found
	disable_interrupt();

	req->dev = bh->b_dev;
	req->cmd = rw;
	req->errors = 0;
	req->sector = bh->b_blocknr * 2;
	req->nr_sector = 2;
	req->bh = bh;
	req->buffer = bh->b_data;
	req->next = NULL;

	add_request(blk_dev + major, req);

	enable_interrupt();
}


void ll_rw_block(int32_t rw, struct buffer_head * bh)
{
	uint32_t major = MAJOR(bh->b_dev);

	if((major >= BLK_DEV_KINDS) || (blk_dev[major].do_req_fn == NULL))
	{
		printk("Trying to read nonexistent block-device\n\r");
		return;
	}

	make_request(major, rw, bh);
}


void blk_req_init(void)
{
	int32_t i;

	for (i = 0; i < NUMBER_OF_REQUESTS; i++)
	{
		request[i].dev = -1;	// -1 means idle
		request[i].next = NULL;
	}
}

