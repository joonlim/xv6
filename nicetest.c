#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "nice test\n");

  int oval = nice(0); // get nice val
  int nval;
  if (oval < 10) {
    nval = nice(5); // inc by 5
    if (nval == oval + 5) {
      printf(1, "successfully called nice(5)\n");
    } else {
      printf(1, "nice(5) failed. oval: %d, nval: %d\n", oval, nval);
    }
  } else {
    nval = nice(-5); // dec by 5
    if (nval == oval - 5) {
      printf(1, "successfully called nice(-5)\n");
    } else {
      printf(1, "nice(-5) failed. oval: %d, nval: %d\n", oval, nval);
    }
  }

  nval = nice(100);
  if (nval == 20) {
    printf(1, "successfully called nice(100)\n");
  } else {
    printf(1, "nice(100) failed\n");
  }

  nval = nice(-100);
  if (nval == 0) {
    printf(1, "successfully called nice(-100)\n");
  } else {
    printf(1, "nice(-100) failed\n");
  }

  exit();
}
