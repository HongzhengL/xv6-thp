#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int sys_fork(void) { return fork(); }

int sys_exit(void) {
    exit();
    return 0;  // not reached
}

int sys_wait(void) { return wait(); }

int sys_kill(void) {
    int pid;

    if (argint(0, &pid) < 0) return -1;
    return kill(pid);
}

int sys_getpid(void) { return myproc()->pid; }

int sys_sbrk(void) {
    int addr;
    int n;
    int baseHugeFlag = 0;

    if (argint(0, &n) < 0 || argint(1, &baseHugeFlag)) return -1;
    addr = myproc()->sz;
    if (baseHugeFlag == 1) addr = myproc()->hugesz;
    if (growproc(n, baseHugeFlag) < 0) return -1;
    return addr;
}

int sys_sleep(void) {
    int n;
    uint ticks0;

    if (argint(0, &n) < 0) return -1;
    acquire(&tickslock);
    ticks0 = ticks;
    while (ticks - ticks0 < n) {
        if (myproc()->killed) {
            release(&tickslock);
            return -1;
        }
        sleep(&ticks, &tickslock);
    }
    release(&tickslock);
    return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void) {
    uint xticks;

    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);
    return xticks;
}

// System calls for debugging huge page allocations/mappings
int sys_printhugepde() {
    pde_t *pgdir = myproc()->pgdir;
    int pid = myproc()->pid;
    int i = 0;
    for (i = 0; i < 1024; i++) {
        if ((pgdir[i] & PTE_PS) && (pgdir[i] & PTE_U) && (pgdir[i] & PTE_P))
            cprintf("PID %d: PDE[%d] is 0x%x\n", pid, i, pgdir[i]);
    }
    return 0;
}

int sys_procpgdirinfo() {
    int *buf;
    if (argptr(0, (void *)&buf, 2 * sizeof(buf[0])) < 0) return -1;
    pde_t *pgdir = myproc()->pgdir;
    int base_cnt = 0;  // base page count
    int huge_cnt = 0;  // huge page count
    int i = 0;
    int j = 0;
    for (i = 0; i < 1024; i++) {
        if ((pgdir[i] & PTE_PS) && (pgdir[i] & PTE_U) &&
            (pgdir[i] &
             PTE_P) /*PTE_P, PTE_U and PTE_PS should be set for huge pages*/)
            ++huge_cnt;
        if ((pgdir[i] & PTE_U) && (pgdir[i] & PTE_P) &&
            ((pgdir[i] & PTE_PS) ==
             0) /*Only PTE_P and PTE_U should be set for base pages*/) {
            uint *pgtab = (uint *)P2V(PTE_ADDR(pgdir[i]));
            for (j = 0; j < 1024; j++) {
                if ((pgtab[j] & PTE_U) && (pgtab[j] & PTE_P)) {
                    ++base_cnt;
                }
            }
        }
    }
    buf[0] = base_cnt;  // base page count
    buf[1] = huge_cnt;  // huge page count
    return 0;
}

int sys_setthp() {
    int val;
    if (argint(0, &val) < 0) {
        return -1;
    }
    if (val < 0 || val > 1) {
        return -1;
    }
    return setthp(val);
}

int sys_checkthp() { return checkthp(); }
