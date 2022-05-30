#ifndef FS_H
#define FS_H

#include <sys/types.h>
#include <linux/sched.h>

#define BLK_READ 0
#define BLK_WRITE 1

#define BLOCK_SIZE 1024
#define NR_HASH 307		// number of hash-table entries

#define MAJOR(a) (((unsigned)(a))>>8)	// major device number
#define MINOR(a) ((a)&0xff)	// minor device number


struct buffer_head
{
	int8_t *b_data;		// data pointer 1024B
	uint32_t b_blocknr;	// block-number in device
	uint16_t b_dev;		// dev-number (high 8 bit is MAJOR low 8 bit is MINOR)
	uint8_t b_uptodate;	// if this READ/WRITE is succeeded
	uint8_t b_dirt;		// whether was modified and has not been synchronized
	uint8_t b_count;	// how many tasks are using this buffer
	uint8_t b_lock;		// when expecting data in-flight from device set this flag

	struct task_struct * b_wait;		// wait queue

	struct buffer_head * b_prev_hash;	// previous pointer in hash-table
	struct buffer_head * b_next_hash;	// next pointer in hash-table

	struct buffer_head * b_prev_free;	// previous pointer in free-list
	struct buffer_head * b_next_free;	// next pointer in free-list
};

/* buffer related */
void buffer_init(uint32_t buffer_end);
struct buffer_head * bread(int32_t dev,int32_t block);
void brelse(struct buffer_head * buf);
struct buffer_head * get_hash_table(int32_t dev, int32_t block);
struct buffer_head * getblk(int32_t dev, int32_t block);
int32_t sync_dev(int32_t dev);

/* block driver related */
void ll_rw_block(int32_t rw, struct buffer_head * bh);


#endif //FS_H

