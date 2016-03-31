#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "nice test\n");

  int oval = nice(0);
  int nval;
  if (oval < 0) {
    nval = nice(10);
    if (nval == oval + 10) {
        printf(1, "successfully called nice(10)\n");
    } else {
        printf(1, "nice(10) failed\n");
    }
  } else {
    nval = nice(-10);
    if (nval == oval - 10) {
        printf(1, "successfully called nice(-10)\n");
    } else {
        printf(1, "nice(-10) failed\n");
    }
  }

  nval = nice(100);
  if (nval == 19) {
    printf(1, "successfully called nice(100)\n");
  } else {
    printf(1, "nice(100) failed\n");
  }

  nval = nice(-100);
  if (nval == -20) {
    printf(1, "successfully called nice(-100)\n");
  } else {
    printf(1, "nice(-100) failed\n");
  }

  exit();
}
