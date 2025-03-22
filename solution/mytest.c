#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
    uint size = 1024 * 1024 * (500);
    char *p = (char *)vmalloc(size, VMALLOC_SIZE_HUGE);
    int page_cnt[2];
    if (procpgdirinfo(page_cnt) == -1) {
        printf(1, "XV6_TEST_ERROR Error, procpgdirinfo returned -1\n");
    }
    printf(1, "XV6_TEST_INFO: page_cnt[0] = %d, page_cnt[1] = %d\n",
           page_cnt[0], page_cnt[1]);
    vfree(p);
    size = 1024 * 1024 * (499);
    p = (char *)vmalloc(size, VMALLOC_SIZE_HUGE);
    if (procpgdirinfo(page_cnt) == -1) {
        printf(1, "XV6_TEST_ERROR Error, procpgdirinfo returned -1\n");
    }
    printf(1, "XV6_TEST_INFO: page_cnt[0] = %d, page_cnt[1] = %d\n",
           page_cnt[0], page_cnt[1]);
    wait();
    exit();
}
