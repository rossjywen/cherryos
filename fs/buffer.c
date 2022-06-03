#include <sys/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/blk_req.h>
#include <asm/system.h>


#define _hashfn(dev,block) (((unsigned)(dev^block))%NR_HASH)
#define hash(dev,block) hash_table[_hashfn(dev,block)]


int32_t number_of_buffers = 0;

struct buffer_head * buf_start_addr = (struct buffer_head*)(1 * 1024 * 1024); // buffer origins from 1MB
struct buffer_head * free_list;	// buffer free-list
struct buffer_head * hash_table[NR_HASH];	// 307 entries

static struct task_struct * buffer_wait = NULL;	// wait queue


void lock_buffer(struct buffer_head * bh)
{
	disable_interrupt();

	while (bh->b_lock)
		sleep_on(&bh->b_wait);
	bh->b_lock=1;

	enable_interrupt();
}


void unlock_buffer(struct buffer_head * bh)
{
	if (!bh->b_lock)
		printk("try to unlock a unlocked buffer\n");
	bh->b_lock = 0;
	wake_up(&bh->b_wait);
}


// wait b_lock to become 0
static inline void wait_on_buffer(struct buffer_head * bh)
{
	disable_interrupt();

	while (bh->b_lock)
		sleep_on(&bh->b_wait);

	enable_interrupt();
}


static struct buffer_head * find_buffer(int dev, int block)
{
	struct buffer_head * tmp;

	for (tmp = hash(dev,block); tmp != NULL; tmp = tmp->b_next_hash)
	{
		if (tmp->b_dev == dev && tmp->b_blocknr == block)
			return tmp;
	}
	return NULL;
}


static inline void remove_from_queues(struct buffer_head * bh)
{
	/* remove from hash-queue */
	if (bh->b_next_hash)
		bh->b_next_hash->b_prev_hash = bh->b_prev_hash;
	if (bh->b_prev_hash)
		bh->b_prev_hash->b_next_hash = bh->b_next_hash;
	if (hash(bh->b_dev,bh->b_blocknr) == bh)
		hash(bh->b_dev,bh->b_blocknr) = bh->b_next_hash;

	/* remove from free list */
	if (!(bh->b_prev_free) || !(bh->b_next_free))
		panic("Free block list corrupted");
	bh->b_prev_free->b_next_free = bh->b_next_free;
	bh->b_next_free->b_prev_free = bh->b_prev_free;
	if (free_list == bh)
		free_list = bh->b_next_free;
}


static inline void insert_into_queues(struct buffer_head * bh)
{
	/* post-insertion of free list */
	bh->b_next_free = free_list;
	bh->b_prev_free = free_list->b_prev_free;
	free_list->b_prev_free->b_next_free = bh;
	free_list->b_prev_free = bh;

	/* if it has no device just set to NULL and return */
	bh->b_prev_hash = NULL;
	bh->b_next_hash = NULL;
	if (!bh->b_dev)
		return;

	/* front insertion of list of hash table */
	bh->b_next_hash = hash(bh->b_dev,bh->b_blocknr);
	hash(bh->b_dev,bh->b_blocknr) = bh;
	bh->b_next_hash->b_prev_hash = bh;
}


int32_t sync_dev(int32_t dev)
{
	// todo sync_dev()
	return 0;
}



struct buffer_head * get_hash_table(int32_t dev, int32_t block)
{
	struct buffer_head *bh;

	while(1)
	{
		bh = find_buffer(dev, block);
		if(bh == NULL)
		{
			return NULL;
		}
		else	// found one
		{
			bh->b_count += 1;
			wait_on_buffer(bh);
			if(bh->b_dev == dev && bh->b_blocknr == block)	// make sure it is not modified during sleep
				return bh;
			bh->b_count -= 1;
		}
	}
}


/*
 * 这个宏用来计算当找不到“完美”的块的时候 哪个块更合适一些
 * 其中b_dirt的影响更大 所以左移1位
 * b_dirt影响大是因为如果等待的是dirty的 需要调用sync_dev() 成本很高
 * 这个宏计算出来的结果越大说明“越差” 所以尽量找到更小返回值的块
 */
#define BADNESS(bh) (((bh)->b_dirt<<1)+(bh)->b_lock)
struct buffer_head * getblk(int32_t dev, int32_t block)
{
	struct buffer_head *tmp, *bh;

repeat:
	bh = get_hash_table(dev, block);
	if(bh != NULL)	// returen non-null in hash table
		return bh;


	// search in free list

	tmp = free_list;
	do
	{
		if(tmp->b_count != 0)	// someone possess this
		{
			continue;
		}
		else
		{
			if(!bh || BADNESS(tmp)<BADNESS(bh))
			{
				bh = tmp;
				if (!BADNESS(tmp))	// if BADNESS returns 0 means good enough
					break;
			}
		}
	}while((tmp = tmp->b_next_free) != free_list);	// fully searched

	if(bh == NULL)	// if all block's b_count are not 0
	{
		sleep_on(&buffer_wait);
		goto repeat;	// search again
	}
	else	// found one
	{
		wait_on_buffer(bh);	// because not perfect, have to wait to be perfect
		if(bh->b_count != 0)	// if possessed by another waked-up task
			goto repeat;	// search again

		while(bh->b_dirt == 1)
		{
			sync_dev(bh->b_dev);	// todo sync_dev()
			wait_on_buffer(bh);
			if(bh->b_count)
				goto repeat;	// search again
		}

		if(find_buffer(dev, block))	// check if someone has already added this block into hash
			goto repeat;	// search again, but this time hash table will return corresponding block

		// this place should avoid race-condition maybe spinlock
		disable_interrupt();

		bh->b_count = 1;
		bh->b_dirt = 0;	// has not write yet
		bh->b_uptodate = 0;	// has not read yet
		remove_from_queues(bh);
		bh->b_dev = dev;
		bh->b_blocknr = block;
		insert_into_queues(bh);

		enable_interrupt();

		return bh;
	}

}


struct buffer_head * bread(int dev,int block)
{
	struct buffer_head *bh;

	if (!(bh=getblk(dev,block)))
		panic("bread: getblk returned NULL\n");
	if (bh->b_uptodate)
		return bh;

	ll_rw_block(BLK_READ, bh);
	wait_on_buffer(bh);
	if (bh->b_uptodate)
	{
		return bh;
	}
	else
	{
		brelse(bh);
		return NULL;
	}
}


void brelse(struct buffer_head * buf)
{
	if(!buf)
		return;
	wait_on_buffer(buf);
	if(buf->b_count == 0)
		panic("Trying to free free buffer");

	buf->b_count -= 1;
	wake_up(&buffer_wait);	// todo 为什么不确保b_count是0之后再wake_up?
}


void buffer_init(uint32_t buffer_end)
{
	struct buffer_head *heads = buf_start_addr;
	void *end;
	int32_t i;


	end = (void*)(buffer_end);

	while(1) // 从end(高地址)开始 从上往下初始化尽可能多的buffer
	{
		end -= BLOCK_SIZE;
		if(end < (void *)(heads + 1))
			break;

		heads->b_dev = 0;
		heads->b_dirt = 0;
		heads->b_count = 0;
		heads->b_lock = 0;
		heads->b_uptodate = 0;
		heads->b_wait = NULL;
		heads->b_prev_hash = NULL;
		heads->b_next_hash = NULL;
		heads->b_data = (int8_t *)end;
		heads->b_prev_free = heads - 1;		// 空闲链表指针设置 注意这里第0项这个域的初始化是错的 下面会修正
		heads->b_next_free = heads + 1;		// 空闲链表指针设置 注意这里最后项的这个域的初始化是错的 下面会修正
	
		heads++;
		
		number_of_buffers++;
	}

	// 从while出来 heads和end都指向了无效的地址 即最后一个有效地址的++和--

	heads -= 1;

	free_list = (struct buffer_head*)(1 * 1024 * 1024);	
	free_list->b_prev_free = heads;		// 修正第0项的b_prev_free
	heads->b_next_free = free_list;		// 修正最后项的b_next_free

	for(i = 0; i < NR_HASH; i++)	// 现在开始初始化hash表 先都设置成null、
		hash_table[i] = NULL;

	printk("init %d buffers\n", number_of_buffers);
}

