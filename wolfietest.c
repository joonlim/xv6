#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  const uint size = 2604;
  char buffer[size];
  if (wolfie(buffer, size) != -1)
    printf(1, "%s", buffer);
  exit();
}
