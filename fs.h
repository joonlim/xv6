// On-disk file system format. 
// Both the kernel and user programs use this header file.


#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// Disk layout:
// [ boot block | super block | log | inode blocks | free bit map | data blocks ]
//
// mkfs computes the super block and builds an initial file system. The super describes
// the disk layout:
struct superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint nlog;         // Number of log blocks
  uint logstart;     // Block number of first log block
  // uint inodestart;   // Block number of first inode block
  // uint bmapstart;    // Block number of first free map block

  uint nblockgroups; // Number of block groups
  uint bgroupstart;  // Block number of the first block of the first block group
  uint bgroupsize;   // Size of a block group
  uint inodesperbgroup; // Inodes per block group
  uint inodeblocksperbgroup; // Inode blocks per block group

  uint bmapblocksperbgroup; // Bit map blocks per block group
  uint datablocksperbgroup; // Number of data blocks per block group

  uint bgroupmeta; // Number of blocks for metadata per block group = inodeblocksperbgroup + bmapblocksperbgroup
};

struct superblock sb;   // there should be one per dev, but we run with one dev

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses
};

// Used to hold information about a block group to be used with system call bgstat().
struct bgstat {
  uint bgnum;         // Block group number
  uint firstblocknum; // Number of first block in this block group
  uint allocatedinodes; // Number of allocated inodes in this block group
  uint allocateddatablocks; // Number of allocated data blocks in this block group
};

// Used to hold information about a file's blocks to be used with system call fbgstat().
struct fbgstat {
  uint inum;          // Inode number
  uint inodebgroup;   // Block group number that the inode belongs to
  uint ndatablocks;   // Number of data blocks allocated for this file
  uint datablockbgroups[NDIRECT + NINDIRECT]; // Block group number of each data block in this group.
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block group containing inode i
#define IBLOCKGROUP(i, sb)   ((i) / sb.inodesperbgroup)

// Starting block of block group containing inode i
#define IBLOCKGROUPSTART(i, sb)  (sb.bgroupstart + (IBLOCKGROUP((i), sb) * sb.bgroupsize))

// Starting block of block group given block group number
#define BBLOCKGROUPSTART(b, sb)  (sb.bgroupstart + ((b) * sb.bgroupsize))

// Block containing inode i
// #define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)
#define IBLOCK(i, sb)      (IBLOCKGROUPSTART((i), sb) + ((i) % sb.inodeblocksperbgroup))

// First inode of the given block group
#define FIRSTINODEOFBGROUP(b, sb)  ((b) * sb.inodesperbgroup)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Check if this block number is a valid data block
#define BISVALID(b, sb) (sb.bgroupstart < (b) && ((b) - sb.bgroupstart) % sb.bgroupsize >= (sb.bgroupmeta)) ? 1 : 0)

// Which group a block number is in
#define BGROUP(b, sb)  (((b) - sb.bgroupstart) / sb.bgroupstart)

// Block of free map containing bit for block b
// #define BBLOCK(b, sb) (b/BPB + sb.bmapstart)
#define BBLOCK(b, sb)  (BGROUP(b, sb) + sb.inodeblocksperbgroup + (((b) - sb.bgroupmeta) % sb.bgroupsize) / BPB)

// Offset bit inside bitmap block that contains bit for block b
#define BOFFSET(b, sb)  (((b) - sb.bgroupstart - sb.bgroupmeta) % BPB)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

