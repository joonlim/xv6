#define NPROC        64  // maximum number of processes
#define KSTACKSIZE 4096  // size of per-process kernel stack
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       ((4096*64)+2+LOGSIZE)  // size of file system in blocks
#define NBLOCKGROUPS 64  // number of block groups in the fs.
// block group size = (FSSIZE - 2 - 30) / NBLOCKGROUPS
// nmeta 123 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 65) blocks 262053 total 262176