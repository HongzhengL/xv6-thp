#include "types.h"
#include "user.h"

int main() {
  int original_page_cnt[2];
  int page_cnt[2];
  if(procpgdirinfo(original_page_cnt) == -1) {
    printf(1,"XV6_TEST_ERROR Error, procpgdirinfo returned -1\n");
  }
  printf(1, "Start of program - base pages:%d huge pages:%d\n", original_page_cnt[0], original_page_cnt[1]);
  char * a, *b;
  a = (char *)malloc(2097152);
  memset(a, 0, 2097152);
  if(procpgdirinfo(page_cnt) == -1) {
    printf(1,"XV6_TEST_ERROR Error, procpgdirinfo returned -1\n");
  }
  printf(1, "XV6_TEST_OUTPUT After malloc with THP disabled, 2MB - program base pages:%d huge pages:%d\n", page_cnt[0] - original_page_cnt[0], page_cnt[1] - original_page_cnt[1]);
  printf(1, "XV6_TEST_OUTPUT Now testing THP\n");
  if(setthp(1) == -1) {
    printf(1,"XV6_TEST_ERROR Error, setthp returned -1\n");
  }
  b = (char *)malloc(2097152);
  memset(b, 0, 2097152);
  if(procpgdirinfo(page_cnt) == -1) {
    printf(1,"XV6_TEST_ERROR Error, procpgdirinfo returned -1\n");
  }
  printf(1, "XV6_TEST_OUTPUT After malloc with THP enabled, 2MB - program base pages:%d huge pages:%d\n", page_cnt[0] - original_page_cnt[0], page_cnt[1] - original_page_cnt[1]);
  printf(1, "Now testing THP\n");
  if(setthp(0) == -1) {
    printf(1,"XV6_TEST_ERROR Error, setthp returned -1\n");
  }
  exit();
}
