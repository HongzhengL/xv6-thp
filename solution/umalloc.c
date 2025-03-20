#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"
#include "memlayout.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;

union header {
    struct {
        union header *ptr;
        uint size;
    } s;
    Align x;
};

typedef union header Header;

static Header base;
static Header hugebase;
static Header *freep;
static Header *hugefreep;

const uint THP_CUTOFF = 1 * 1024 * 1024;  // 1MB

void vfree(void *ap) {
    if (ap > (void *)KERNBASE) {
        const char *msg =
            "Error - trying to free a kernel pointer in userspace\n";
        printf(2, "%s", msg);
        exit();
    }
    if (ap < (void *)HUGE_PAGE_START || ap > (void *)HUGE_PAGE_END) return;
    Header *bp, *p;

    bp = (Header *)ap - 1;
    for (p = hugefreep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) break;
    if (bp + bp->s.size == p->s.ptr) {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else
        bp->s.ptr = p->s.ptr;
    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else
        p->s.ptr = bp;
    hugefreep = p;
}

void free(void *ap) {
    if (ap > (void *)KERNBASE) {
        const char *msg =
            "Error - trying to free a kernel pointer in userspace\n";
        printf(2, "%s", msg);
        exit();
    }
    if (ap > (void *)HUGE_PAGE_START) {
        vfree(ap);
        return;
    }
    Header *bp, *p;

    bp = (Header *)ap - 1;
    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) break;
    if (bp + bp->s.size == p->s.ptr) {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else
        bp->s.ptr = p->s.ptr;
    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else
        p->s.ptr = bp;
    freep = p;
}

static Header *morecore(uint nu) {
    char *p;
    Header *hp;

    if (nu < 4096) nu = 4096;
    p = sbrk(nu * sizeof(Header), VMALLOC_SIZE_BASE);
    if (p == (char *)-1) return 0;
    hp = (Header *)p;
    hp->s.size = nu;
    free((void *)(hp + 1));
    return freep;
}

static Header *morehugecore(uint nu) {
    char *p;
    Header *hp;

    if (nu < HUGE_PAGE_SIZE) nu = HUGE_PAGE_SIZE / sizeof(Header);
    p = sbrk(nu * sizeof(Header), VMALLOC_SIZE_HUGE);
    if (p == (char *)-1) return 0;
    hp = (Header *)p;
    hp->s.size = nu;
    vfree((void *)(hp + 1));
    return hugefreep;
}

void *malloc(uint nbytes) {
    Header *p, *prevp;
    uint nunits;

    int thp_enable = checkthp();

    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
    if (nbytes < THP_CUTOFF || !thp_enable) {
        if ((prevp = freep) == 0) {
            base.s.ptr = freep = prevp = &base;
            base.s.size = 0;
        }
        for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
            if (p->s.size >= nunits) {
                if (p->s.size == nunits)
                    prevp->s.ptr = p->s.ptr;
                else {
                    p->s.size -= nunits;
                    p += p->s.size;
                    p->s.size = nunits;
                }
                freep = prevp;
                return (void *)(p + 1);
            }
            if (p == freep)
                if ((p = morecore(nunits)) == 0) return 0;
        }
    } else {
        if ((prevp = hugefreep) == 0) {
            hugebase.s.ptr = hugefreep = prevp = &hugebase;
            hugebase.s.size = 0;
        }
        for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
            if (p->s.size >= nunits) {
                if (p->s.size == nunits)
                    prevp->s.ptr = p->s.ptr;
                else {
                    p->s.size -= nunits;
                    p += p->s.size;
                    p->s.size = nunits;
                }
                hugefreep = prevp;
                return (void *)(p + 1);
            }
            if (p == hugefreep)
                if ((p = morehugecore(nunits)) == 0) return 0;
        }
    }
}

void *vmalloc(uint nbytes, int baseHugeFlag) {
    Header *p, *prevp;
    uint nunits;

    if (baseHugeFlag != VMALLOC_SIZE_BASE &&
        baseHugeFlag != VMALLOC_SIZE_HUGE) {
        const char *msg =
            "Please pass VMALLOC_SIZE_BASE or VMALLOC_SIZE_HUGE as flag.\n";
        printf(2, "%s", msg);
        exit();
    }
    if (baseHugeFlag == VMALLOC_SIZE_BASE) return malloc(nbytes);

    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
    if ((prevp = hugefreep) == 0) {
        hugebase.s.ptr = hugefreep = prevp = &hugebase;
        hugebase.s.size = 0;
    }
    for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
        if (p->s.size >= nunits) {
            if (p->s.size == nunits)
                prevp->s.ptr = p->s.ptr;
            else {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            hugefreep = prevp;
            return (void *)(p + 1);
        }
        if (p == hugefreep)
            if ((p = morehugecore(nunits)) == 0) return 0;
    }
}
