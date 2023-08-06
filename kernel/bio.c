// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define BUCKETSIZE 13
#define BUFFERSIZE 6
extern uint ticks; 

struct {
  struct spinlock lock;
  struct buf buf[BUFFERSIZE];
} bucket[BUCKETSIZE];

int hash(uint blockno)
{
  return blockno%BUCKETSIZE;
}

void
binit(void)
{
  for (int i = 0; i < BUCKETSIZE; i++) {
    initlock(&bucket[i].lock, "bcachebucket");
    for (int j = 0; j < BUFFERSIZE; j++) {
      initsleeplock(&bucket[i].buf[j].lock, "buffer");
    }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int idx = hash(blockno);
  acquire(&bucket[idx].lock);

  // Is the block already cached?
  for(int i = 0;i < BUFFERSIZE;i++){
    b=&bucket[idx].buf[i];
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      b->lastuse=ticks;
      release(&bucket[idx].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  uint least=0xffffffff;
  int least_idx=-1;
  for(int i = 0; i < BUFFERSIZE; i++){
    b=&bucket[idx].buf[i];
    if(b->refcnt == 0 && b->lastuse < least) {
      least = b->lastuse;
      least_idx = i;
    }
  }

  if(least_idx==-1){
    panic("bget: no buffers");
  }

  b = &bucket[idx].buf[least_idx];
  b->dev = dev;
  b->blockno = blockno;
  b->lastuse = ticks;
  b->valid = 0;
  b->refcnt = 1;
  release(&bucket[idx].lock);
  acquiresleep(&b->lock);
  // --- end of critical session
  return b;
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");
  int idx = hash(b->blockno);
  acquire(&bucket[idx].lock);
  b->refcnt--;
  release(&bucket[idx].lock);
  releasesleep(&b->lock);
}

void
bpin(struct buf *b) {
  int idx=hash(b->blockno);
  acquire(&bucket[idx].lock);
  b->refcnt++;
  release(&bucket[idx].lock);
}

void
bunpin(struct buf *b) {
  int idx=hash(b->blockno);
  acquire(&bucket[idx].lock);
  b->refcnt--;
  release(&bucket[idx].lock);
}


