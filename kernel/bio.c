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

// Different disk blocks are placed in separate hash buckets, so cache hits
// for unrelated blocks do not contend on one global cache lock.
#define NBUCKET 13

struct bcache_bucket {
  struct spinlock lock;
  struct buf head;
};

static struct bcache_bucket bcache[NBUCKET];
static struct spinlock bcache_evict_lock;
static struct buf bbuf[NBUF];

static int
bhash(uint dev, uint blockno)
{
  return (dev + blockno) % NBUCKET;
}

static void
binsert(struct bcache_bucket *bucket, struct buf *b)
{
  b->next = bucket->head.next;
  b->prev = &bucket->head;
  bucket->head.next->prev = b;
  bucket->head.next = b;
}

static void
bremove(struct buf *b)
{
  b->next->prev = b->prev;
  b->prev->next = b->next;
}

void
binit(void)
{
  struct buf *b;

  initlock(&bcache_evict_lock, "bcache.evict");
  for(int i = 0; i < NBUCKET; i++){
    initlock(&bcache[i].lock, "bcache.bucket");
    bcache[i].head.prev = &bcache[i].head;
    bcache[i].head.next = &bcache[i].head;
  }
  for(b = bbuf; b < bbuf+NBUF; b++){
    initsleeplock(&b->lock, "buffer");
    b->dev = (uint)-1;
    b->blockno = (uint)-1;
    binsert(&bcache[(b - bbuf) % NBUCKET], b);
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int bucketno = bhash(dev, blockno);
  struct bcache_bucket *bucket = &bcache[bucketno];

  acquire(&bucket->lock);

  // Is the block already cached?
  for(b = bucket->head.next; b != &bucket->head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bucket->lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
  release(&bucket->lock);

  // Serialize cache misses. After obtaining this lock, check again because a
  // concurrent miss may have installed the block while we were waiting.
  acquire(&bcache_evict_lock);
  acquire(&bucket->lock);
  for(b = bucket->head.next; b != &bucket->head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bucket->lock);
      release(&bcache_evict_lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  release(&bucket->lock);
  // Recycle any unused buffer. It is removed from its old bucket before the
  // new identity is installed, so a block has at most one cache copy.
  for(int i = 0; i < NBUCKET; i++){
    acquire(&bcache[i].lock);
    for(b = bcache[i].head.next; b != &bcache[i].head; b = b->next){
      if(b->refcnt == 0){
        bremove(b);
        release(&bcache[i].lock);

        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;

        acquire(&bucket->lock);
        binsert(bucket, b);
        release(&bucket->lock);
        release(&bcache_evict_lock);
        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcache[i].lock);
  }
  release(&bcache_evict_lock);
  panic("bget: no buffers");
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

  releasesleep(&b->lock);

  struct bcache_bucket *bucket = &bcache[bhash(b->dev, b->blockno)];
  acquire(&bucket->lock);
  b->refcnt--;
  release(&bucket->lock);
}

void
bpin(struct buf *b) {
  struct bcache_bucket *bucket = &bcache[bhash(b->dev, b->blockno)];
  acquire(&bucket->lock);
  b->refcnt++;
  release(&bucket->lock);
}

void
bunpin(struct buf *b) {
  struct bcache_bucket *bucket = &bcache[bhash(b->dev, b->blockno)];
  acquire(&bucket->lock);
  b->refcnt--;
  release(&bucket->lock);
}

