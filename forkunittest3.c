// Test to see that copy-on-write fork is working correctly.

#include "types.h"
#include "stat.h"
#include "user.h"

int main();

void
forkunittest3(void)
{
  printf(1, "fork unit test 3\n");

  if (fork() == 0) {
    if (fork() == 0) {
      exit();
    } else {
      wait();
    }
    exit();
  } else {
    wait();
  }

  printf(1, "fork unit test 3 passed\n");
}

int
main(void)
{
  forkunittest3();
  exit();
}
