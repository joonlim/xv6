/**
 * Test Fast File System 3
 *
 * Test heuristic 1 (chunking and spreading large files across multiple block groups).
 */

#include "types.h"
#include "user.h"
#include "fs.h"

#define TESTFILE "_testfile"

int
main(int argc, char *argv[])
{
  printf(1, "This tests the third heurstic of the Fast File System.\n");
  printf(1, "To test this heuristic, this test creates a file and writes a lot of repetitive\n"\
    "text to it. We can then call fbgstat() to see which block groups each of its\n"\
    "data blocks are in. This test should show chunking of size NDIRECT, when possible,\n"\
    "which is 12.\n\n");

  int fd;
  struct fbgstat fbg;

  printf(1, "Making large file (full of junk)...\n");
  if ((fd = open(TESTFILE, 0x001 | 0x002 | 0x200)) < 0) {
    printf(2, "failed call to open\n");
    exit();
  }

  char buf[512]; // random char buffer
  int i;
  for (i = 120; i >= 0; i--) {
    if (i % 10 == 0) printf(1, "...%d blocks left\n", i);
    write(fd, buf, sizeof(buf));
  }

  fbgstat(fd, &fbg);
  printf(1, "Inode block group: %d\n", fbg.inodebgroup);
  printf(1, "Number of data blocks: %d\n", fbg.ndatablocks);
  for (i = 0; i < fbg.ndatablocks; i++) {
    printf(1, "Data block %d is in block group %d\n", i, fbg.datablockbgroups[i]);
  }

  close(fd);

  exit();
}
