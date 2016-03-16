// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

#define MAXPAGES (PHYSTOP / PGSIZE)

void _freerange(void *vstart, void *vend);
void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file

struct run {
  struct run *next;
  uint ref; // reference count
};


struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
  // DEP: For COW fork, we can't store the run in the 
  //      physical page, because we need space for the ref
  //      count.  Move to the kmem struct.
  struct run runs[MAXPAGES];
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  _freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  _freerange(vstart, vend);
  kmem.use_lock = 1;
}

// only called by kinit2 and kinit2.
void
_freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    _kfree(p);
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}

// Called by _freerange, which is only called by kinit1, kinit2,
// and also freevm and deallocvm in vm.c.
void
_kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || v2p(v) >= PHYSTOP)
    panic("_kfree");

  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = &kmem.runs[(V2P(v) / PGSIZE)];
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
}

//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || v2p(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = &kmem.runs[(V2P(v) / PGSIZE)];
  if (r->ref != 1) {
    cprintf("kfree: assert ref == 1 failed\n");
    cprintf("%d\n", r->ref);
    exit();
  }
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;
  char *rv;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    r->ref = 1;
    kmem.freelist = r->next;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  rv = P2V((r - kmem.runs) * PGSIZE);
  return rv;
}

/**
 * Increment the reference count of a page descriptor (struct run), and return
 * the new value.
 */
void
incref(char* v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || v2p(v) >= PHYSTOP)
    panic("incref");

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = &kmem.runs[(V2P(v) / PGSIZE)];
  r->ref++;
  cprintf("incref: address: 0x%p, ref: %d\n", r, r->ref);
  if(kmem.use_lock)
    release(&kmem.lock);
}

/**
 * Decrement the reference count of a page descriptor (struct run), and return
 * the new value.
 */
void
decref(char* v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || v2p(v) >= PHYSTOP)
    panic("decref");

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = &kmem.runs[(V2P(v) / PGSIZE)];
  r->ref--;
  cprintf("decref: address: 0x%p, ref: %d\n", r, r->ref);
  if(kmem.use_lock)
    release(&kmem.lock);
}

/**
 * Get reference count of a page descriptor (struct run).
 */
uint
getref(char *v)
{
  struct run *r = &kmem.runs[(V2P(v) / PGSIZE)];
  return r->ref;
}

/**
 * Print reference count of a page descriptor (struct run).
 */
void
printref(char *v)
{
  struct run *r = &kmem.runs[(V2P(v) / PGSIZE)];
  cprintf("printref: address: 0x%p, ref: %d\n", r, r->ref);
}
