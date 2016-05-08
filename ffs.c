#include "types.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  struct superblock sb;
  if(fsstat(&sb) < 0){
    printf(2, "ffs: failed to access superblock\n");
    exit();
  }

  printf(1, "\nFast File System - implemented by Joon Lim\n\n");

  printf(1, "General info\n");


  printf(1, "  total size: %d blocks\n", sb.size);
  printf(1, "  boot block size: 1\n");
  printf(1, "  super block size: 1\n");
  printf(1, "  total number of data blocks: %d\n", sb.nblocks);
  printf(1, "  total number of inodes: %d\n", sb.ninodes);
  printf(1, "  total number of log blocks: %d\n\n", sb.nlog);

  printf(1, "Block group info:\n\n");

  printf(1, "  number of block groups: %d\n", sb.nblockgroups);
  printf(1, "  size of each block group: %d blocks\n", sb.bgroupsize);
  printf(1, "  inodes per block group: %d\n", sb.inodesperbgroup);
  printf(1, "  inode blocks per block group: %d\n", sb.inodeblocksperbgroup);
  printf(1, "  bmap blocks per block group: %d\n", sb.bmapblocksperbgroup);
  printf(1, "  data blocks per block group: %d\n\n", sb.datablocksperbgroup);

  exit();
}
