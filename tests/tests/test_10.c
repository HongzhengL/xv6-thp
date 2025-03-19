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
  // 100000 bytes in base page heap
  a = (char *)malloc(1048576);
  memset(a, 0, 1048576);
  if(procpgdirinfo(page_cnt) == -1) {
    printf(1,"XV6_TEST_ERROR Error, procpgdirinfo returned -1\n");
  }
  printf(1, "XV6_TEST_OUTPUT After malloc with THP disabled, 1 MB - program base pages:%d huge pages:%d\n", page_cnt[0] - original_page_cnt[0], page_cnt[1] - original_page_cnt[1]);
  printf(1, "XV6_TEST_OUTPUT Now enabling THP\n");
  if(setthp(1) == -1) {
    printf(1,"XV6_TEST_ERROR Error, setthp returned -1\n");
  }
  // 2 MB in huge page heap
  b = (char *)malloc(2097152);
  memset(b, 0, 2097152);
  if(procpgdirinfo(page_cnt) == -1) {
    printf(1,"XV6_TEST_ERROR Error, procpgdirinfo returned -1\n");
  }
  printf(1, "XV6_TEST_OUTPUT After malloc with THP, 2MB - program base pages:%d huge pages:%d\n", page_cnt[0] - original_page_cnt[0], page_cnt[1] - original_page_cnt[1]);
  printf(1, "Now testing THP\n");
  if(setthp(0) == -1) {
    printf(1,"XV6_TEST_ERROR Error, setthp returned -1\n");
  }

  // Test fork
  int pid = fork();
  if(pid < 0) {
      printf(2, "Fork failed\n");
      exit();
  }
  if(pid == 0) {
      printf(1, "XV6_TEST_OUTPUT Child stats:\n");
      if(procpgdirinfo(page_cnt) == -1) {
          printf(1,"XV6_TEST_ERROR Error, procpgdirinfo returned -1\n");
      }
      printf(1, "XV6_TEST_OUTPUT Child program base pages:%d huge pages:%d\n", page_cnt[0] - original_page_cnt[0], page_cnt[1] - original_page_cnt[1]);
  } else {
      wait();
      printf(1, "XV6_TEST_OUTPUT Parent stats:\n");
      if(procpgdirinfo(page_cnt) == -1) {
          printf(1,"XV6_TEST_ERROR Error, procpgdirinfo returned -1\n");
      }
      printf(1, "XV6_TEST_OUTPUT Parent program base pages:%d huge pages:%d\n", page_cnt[0] - original_page_cnt[0], page_cnt[1] - original_page_cnt[1]);
  }
  exit();
}
