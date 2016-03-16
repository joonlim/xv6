// Test to see that copy-on-write fork is working correctly.

#include "types.h"
#include "stat.h"
#include "user.h"

int main();

int staticaddr = 0;

void
forkunittest1(void)
{
  printf(1, "fork unit test 1\n");

  int i, pid;
  for (i = 0; i < 1000; i++) {
    pid = fork();
    if (pid == 0)
      staticaddr++; // write to address in data segment
    else {
      wait();
      break;
    }
  }

  if (staticaddr != i) {
    printf(1, "fork unit test 1 failed\n");
    exit();
  } else if (i == 0) {
    printf(1, "fork unit test 1 passed\n");
  }
}

int
main(void)
{
  forkunittest1();
  exit();
}
