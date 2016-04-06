#include "types.h"
#include "user.h"

#define NUM_ITEMS 200

// Return a random integer between a given range.
int
randomrange(int lo, int hi)
{
  if (hi < lo) {
    int tmp = lo;
    lo = hi;
    hi = tmp;
  }
  int range = hi - lo + 1;
  return random() % (range) + lo;
}

int
main(int argc, char *argv[])
{
  printf(1, "random test\n");

  // random numbers between 0 and ((2^32 - 1) / 2), which is 2147483647.
  int i;

  printf(1, "random numbers between 0 and 2147483647:\n");
  for (i = 0; i < NUM_ITEMS; i++) {
    printf(1, "%d ", random());
  }

  printf(1, "\n");
  printf(1, "random numbers between -99 and 100:\n");

  // random numbers between [
  for (i = 0; i < NUM_ITEMS; i++) {
    // int d = random() % 200 - 99;
    int d = randomrange(-99, 100);
    printf(1, "%d ", d);
  }

  printf(1, "\n");
  printf(1, "random numbers between 0 and 10:\n");

  // random numbers between [
  for (i = 0; i < NUM_ITEMS; i++) {
    // int d = random() % 11;
    int d = randomrange(0, 10);
    printf(1, "%d ", d);
  }

  printf(1, "\n");
  printf(1, "random numbers between 1 and 1:\n");

  // random numbers between [
  for (i = 0; i < NUM_ITEMS; i++) {
    // int d = random() % 11;
    int d = randomrange(1, 1);
    printf(1, "%d ", d);
  }

  printf(1, "\n");

  exit();
}
