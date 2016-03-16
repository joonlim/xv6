// Test to see that copy-on-write fork is working correctly.

#include "types.h"
#include "stat.h"
#include "user.h"

int main();

void
forkunittest2(void)
{
  printf(1, "fork unit test 2\n");

  if (fork() != 0) {
    if (fork() != 0) {
      wait();
    } else {
      exit();
    }
    wait();
  } else {
    exit();
  }

  printf(1, "fork unit test 2 passed\n");
}

int
main(void)
{
  forkunittest2();
  exit();
}
