// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
void freehugerange(void *vstart, void *vend);
extern char end[];  // first address after kernel loaded from ELF file
                    // defined by the kernel linker script in kernel.ld

struct run {
    struct run *next;
};

struct {
    struct spinlock lock;
    int use_lock;
    struct run *freelist;
} kmem, khugemem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void kinit1(void *vstart, void *vend) {
    initlock(&kmem.lock, "kmem");
    kmem.use_lock = 0;
    freerange(vstart, vend);
}

void kinit2(void *vstart, void *vend) {
    freerange(vstart, vend);
    kmem.use_lock = 1;
}

void khugeinit(void *vstart, void *vend) {
    initlock(&khugemem.lock, "khugemem");
    freehugerange(vstart, vend);
    khugemem.use_lock = 1;
}

void freerange(void *vstart, void *vend) {
    char *p;
    p = (char *)PGROUNDUP((uint)vstart);
    for (; p + PGSIZE <= (char *)vend; p += PGSIZE) kfree(p);
}

void freehugerange(void *vstart, void *vend) {
    char *p;
    p = (char *)HUGEPGROUNDUP((uint)vstart);
    for (; p + HUGE_PAGE_SIZE <= (char *)vend; p += HUGE_PAGE_SIZE)
        khugefree(p);
}

// PAGEBREAK: 21
//  Free the page of physical memory pointed at by v,
//  which normally should have been returned by a
//  call to kalloc().  (The exception is when
//  initializing the allocator; see kinit above.)
void kfree(char *v) {
    struct run *r;

    if ((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP) panic("kfree");

    // Fill with junk to catch dangling refs.
    memset(v, 1, PGSIZE);

    if (kmem.use_lock) acquire(&kmem.lock);
    r = (struct run *)v;
    r->next = kmem.freelist;
    kmem.freelist = r;
    if (kmem.use_lock) release(&kmem.lock);
}

//  Free the huge page of physical memory pointed at by v,
//  which normally should have been returned by a
//  call to khugealloc().  (The exception is when
//  initializing the huge page allocator; see khugeinit above.)
void khugefree(char *v) {
    struct run *r;

    if ((uint)(V2P(v) - HUGE_PAGE_START) % HUGE_PAGE_SIZE || v < end ||
        V2P(v) < HUGE_PAGE_START || V2P(v) > HUGE_PAGE_END)
        panic("khugefree");

    // Fill with junk to catch dangling refs.
    memset(v, 1, HUGE_PAGE_SIZE);

    if (khugemem.use_lock) acquire(&khugemem.lock);
    r = (struct run *)v;
    r->next = khugemem.freelist;
    khugemem.freelist = r;
    if (khugemem.use_lock) release(&khugemem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char *kalloc(void) {
    struct run *r;

    if (kmem.use_lock) acquire(&kmem.lock);
    r = kmem.freelist;
    if (r) kmem.freelist = r->next;
    if (kmem.use_lock) release(&kmem.lock);
    return (char *)r;
}

// Allocate one 4MB page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char *khugealloc(void) {
    struct run *r;

    if (khugemem.use_lock) acquire(&khugemem.lock);
    r = khugemem.freelist;
    if (r) khugemem.freelist = r->next;
    if (khugemem.use_lock) release(&khugemem.lock);
    return (char *)r;
}

int get_huge_page_count(void) {
    struct run *r;
    int count = 0;
    if (khugemem.use_lock) acquire(&khugemem.lock);
    r = khugemem.freelist;
    while (r) {
        count++;
        r = r->next;
    }
    if (khugemem.use_lock) release(&khugemem.lock);
    cprintf("XV6_TEST_INFO: Huge page count = %d\n", count);
    return count;
}
