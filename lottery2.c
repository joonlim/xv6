#include "types.h"
#include "user.h"

// test the lottery ticket scheduler
// This test makes sure that the process with the highest priority gets to run the most.
// This test also makes sure that processes with a priority of 0 (lowest possible) still gets to run.

#define X 10000

int
main(int argc, char *argv[])
{
  printf(1, "lottery test 2\n");

  // fork into 20 processes with different niceness values.
  int pid;

  int i;
  for (i = 20; i >= 0; i--) {
    pid = fork();
    if (pid == 0) {
      int count = 0;
      int laps = 0;
      while(1) {
        // since prints are expensive, only print after X iterations
        count++;
        if (count > X) {
          laps++;
          count = 0;
          printf(1, "%d\n", i);
        }

        // keep priority at i
        if (nice(0) < i)
          nice(1);
      }
    }   
  }

  while(1) {}

  exit();
}
