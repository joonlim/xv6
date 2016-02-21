#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  char buffer[3000];
  wolfie(buffer);
  printf(1, "Output of wolfie: \n%s\n", buffer);
  exit();
}
