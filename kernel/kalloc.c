// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  // Physical pages returned by kinit() are all in [KERNBASE, PHYSTOP).
  // The count records the number of user page-table mappings that share a
  // page, plus the single ownership held by a freshly allocated page.
  int refcnt[(PHYSTOP - KERNBASE) / PGSIZE];
} kmem;

static int
paindex(void *pa)
{
  return ((uint64)pa - KERNBASE) / PGSIZE;
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    // kfree() drops a reference, so seed every page with one reference
    // while constructing the initial free list.
    acquire(&kmem.lock);
    kmem.refcnt[paindex(p)] = 1;
    release(&kmem.lock);
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&kmem.lock);
  if(kmem.refcnt[paindex(pa)] < 1)
    panic("kfree ref");
  kmem.refcnt[paindex(pa)]--;
  if(kmem.refcnt[paindex(pa)] > 0){
    release(&kmem.lock);
    return;
  }

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    kmem.refcnt[paindex(r)] = 1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

// Add one mapping reference to a physical page shared by COW fork.
void
krefinc(void *pa)
{
  acquire(&kmem.lock);
  if(kmem.refcnt[paindex(pa)] < 1)
    panic("krefinc");
  kmem.refcnt[paindex(pa)]++;
  release(&kmem.lock);
}

int
krefcount(void *pa)
{
  int count;

  acquire(&kmem.lock);
  count = kmem.refcnt[paindex(pa)];
  release(&kmem.lock);
  return count;
}
