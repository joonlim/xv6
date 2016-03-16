// Test that deliberately accesses an invalid address to trigger a page fault.

#include "types.h"
#include "stat.h"
#include "user.h"

void
pgfaulttest(void)
{
  printf(1, "page fault test\n");
  int x = 0;
  // deliberately access invalid address (segfault)
  printf(1, "%d\n", *(&x + 4096 * 2));
  
  printf(1, "process failed to terminate. page fault test failed\n");
}

int
main(void)
{
  pgfaulttest();
  exit();
}
