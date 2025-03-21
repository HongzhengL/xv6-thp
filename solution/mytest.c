#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
    setthp(1);

    int size = 1024 * 1024 * (1);

    [[maybe_unused]] char *p = (char *)malloc(size);

    int page_cnt[2];
    if (procpgdirinfo(page_cnt) == -1) {
        printf(1, "XV6_TEST_ERROR Error, procpgdirinfo returned -1\n");
    }
    printf(1, "XV6_TEST_INFO: page_cnt[0] = %d, page_cnt[1] = %d\n",
           page_cnt[0], page_cnt[1]);

    setthp(0);
    p = (char *)malloc(size * 8);
    if (procpgdirinfo(page_cnt) == -1) {
        printf(1, "XV6_TEST_ERROR Error, procpgdirinfo returned -1\n");
    }
    printf(1, "XV6_TEST_INFO: page_cnt[0] = %d, page_cnt[1] = %d\n",
           page_cnt[0], page_cnt[1]);

    setthp(1);
    size = 1024 * 1024 * (992 - 480);
    p = vmalloc(size, VMALLOC_SIZE_HUGE);
    if (procpgdirinfo(page_cnt) == -1) {
        printf(1, "XV6_TEST_ERROR Error, procpgdirinfo returned -1\n");
    }
    printf(1, "XV6_TEST_INFO: page_cnt[0] = %d, page_cnt[1] = %d\n",
           page_cnt[0], page_cnt[1]);
    exit();
}
