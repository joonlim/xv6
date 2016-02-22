#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  uint size = 2736;
  char buffer[size];
  wolfie(buffer, size);
  printf(1, "Output of wolfie: \n%s\n", buffer);
  exit();
}
