#include "types.h"
#include "user.h"

/*freelist = 0xbdc0*/
/*free(0xa000)*/
/*0x9fff <- 0xa000 <- 0xa004*/
/*109: 0xbdc <- 0xa000 <- 0xa004*/
/*110: freelist = 0xa000*/

int main(int argc, char *argv[]) {
    // setthp(1);

    int size = 1024 * 1024 * (482 - 480);
    /*int size = 1024 * 8;*/

    // char *p = (char *)malloc(size);
    char *p = (char *)vmalloc((unsigned int)size, VMALLOC_SIZE_HUGE);

    // for (int i = 0; i < size; i++) {
    //     p[i] = 1;
    // }

    int page_cnt[2];
    if (procpgdirinfo(page_cnt) == -1) {
        printf(1, "XV6_TEST_ERROR Error, procpgdirinfo returned -1\n");
    }
    printf(1, "XV6_TEST_INFO: page_cnt[0] = %d, page_cnt[1] = %d\n",
           page_cnt[0], page_cnt[1]);
    // printhugepde();
    free(p);
    exit();
}
