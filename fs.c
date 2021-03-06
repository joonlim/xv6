// File system implementation.  Five layers:
//   + Blocks: allocator for raw disk blocks.
//   + Log: crash recovery for multi-step updates.
//   + Files: inode allocator, reading, writing, metadata.
//   + Directories: inode with special contents (list of other inodes!)
//   + Names: paths like /usr/rtm/xv6/fs.c for convenient naming.
//
// This file contains the low-level file system manipulation 
// routines.  The (higher-level) system call implementations
// are in sysfile.c.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "fs.h"
#include "buf.h"
#include "file.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
static void itrunc(struct inode*);

// Fill a buffer fbgstat with block information about the given file.
void filebgstat(struct file *f, struct fbgstat *fbg) {
  struct inode *ip = f->ip;

  fbg->inum = ip->inum;
  fbg->inodebgroup = IBLOCKGROUP(ip->inum, sb);
  
  int ndatablocks = 0;
  int i;
  for (i = 0; i < NDIRECT; i++) {
    if (ip->addrs[i]) {
      ndatablocks++;
      // store which block group this data block is in
      fbg->datablockbgroups[i] = BGROUP(ip->addrs[i], sb) / 128;
    }
  }

  struct buf *bp;

  if (ip->addrs[NDIRECT]) {
    bp = bread(ip->dev, ip->addrs[NDIRECT]);
    uint *a = (uint*)bp->data;
    for (i = 0; i < NINDIRECT; i++) {
      if (a[i]) {
        ndatablocks++;
        // store which block group this data block is in
        fbg->datablockbgroups[i] = BGROUP(a[i], sb) / 128;
      }
    }
    brelse(bp);
  }

  fbg->ndatablocks = ndatablocks;
}

int numallocinodes(int bgnum) {
  int numalloc = 0;

  struct buf *bp;
  struct dinode *dip;
  int dev = 1; // ?
  int start = IBLOCKGROUP(bgnum, sb);
  int end = start + sb.inodesperbgroup;
  int i;
  for (i = start; i < end; i++) {
    bp = bread(dev, IBLOCK(i, sb));
    dip = (struct dinode*)bp->data + i%IPB;
    if (dip->type != 0)
      numalloc++;
    brelse(bp);
  }
  return numalloc;
}

int numallocdatablocks(int bgnum) {
  int numalloc = 0;

  struct buf *bp;
  int dev = 1; // ?
  int firstblock = BBLOCKGROUPSTART(bgnum, sb);
  int b = firstblock + sb.bgroupmeta;
  int bi, m;
  for (; b < firstblock + sb.bgroupsize; b += BPB) {
    bp = bread(dev, BBLOCK(b, sb));
    for (bi = 0; bi < BPB && b + bi < sb.size; bi++) {
      m = 1 << (bi % 8);
      if ((bp->data[bi/8] & m) == 1)
        numalloc++;
    }
    brelse(bp);
  }
  return numalloc;
}

// Fill a buffer bgstat with information about the block group with the given number.
void blockgroupstat(int bgnum, struct bgstat *bg) {
  bg->bgnum = bgnum;
  bg->firstblocknum = BBLOCKGROUPSTART(bgnum, sb);
  bg->allocatedinodes = numallocinodes(bgnum);
  bg->allocateddatablocks = numallocdatablocks(bgnum);
}

// Return the block number of the least utilized block group to be used whenever a new directory is created.
int least_utilized_bgroup() {
  int b, bi, m;
  struct buf *bp;
  int dev = 1; // ?

  int leastUtilizedBgroup = 0;
  int leastUtilizedNumBlocks = sb.bgroupsize;

  int i = 0; // block group
  for (; i < sb.nblockgroups; i++) {
    int firstblock = BBLOCKGROUPSTART(i, sb);
    b = firstblock + sb.bgroupmeta;

    int numAllocatedBlocks = 0;

    for (; b < firstblock + sb.bgroupsize; b += BPB) {
      bp = bread(dev, BBLOCK(b, sb));
      // look for free blocks here
      for(bi = 0; bi < BPB && b + bi < sb.size; bi++){
        m = 1 << (bi % 8);
        if((bp->data[bi/8] & m) == 1) {  // Is block allocated?
          numAllocatedBlocks++;
        }
      }
      brelse(bp);
    }

    if (numAllocatedBlocks < leastUtilizedNumBlocks) {
      leastUtilizedNumBlocks = numAllocatedBlocks;
      leastUtilizedBgroup = i;
    }
  }
  return leastUtilizedBgroup;
}

// Read the super block.
void
readsb(int dev, struct superblock *sb)
{
  struct buf *bp;
  
  bp = bread(dev, 1);
  memmove(sb, bp->data, sizeof(*sb));
  brelse(bp);
}

// Zero a block.
static void
bzero(int dev, int bno)
{
  struct buf *bp;
  
  bp = bread(dev, bno);
  memset(bp->data, 0, BSIZE);
  log_write(bp);
  brelse(bp);
}

// Blocks. 

// // Allocate a zeroed disk block.
// static uint
// balloc(uint dev)
// {
//   int b, bi, m;
//   struct buf *bp;

//   bp = 0;
//   for(b = 0; b < sb.size; b += BPB){
//     bp = bread(dev, BBLOCK(b, sb));
//     for(bi = 0; bi < BPB && b + bi < sb.size; bi++){
//       m = 1 << (bi % 8);
//       if((bp->data[bi/8] & m) == 0){  // Is block free?
//         bp->data[bi/8] |= m;  // Mark block in use.
//         log_write(bp);
//         brelse(bp);
//         bzero(dev, b + bi);
//         return b + bi;
//       }
//     }
//     brelse(bp);
//   }
//   panic("balloc: out of blocks");
// }

// Allocate a zeroed disk block.
static uint
balloc(uint dev)
{
  int b, bi, m;
  struct buf *bp;

  // cprintf("balloc: dev: %d\n", dev);

  int i = 0;
  for (; i < sb.nblockgroups; i++) {
    int firstblock = BBLOCKGROUPSTART(i, sb);
    // cprintf("balloc: i: %d firstblock: %d\n", i, firstblock);
    b = firstblock + sb.bgroupmeta;
    for (; b < firstblock + sb.bgroupsize; b += BPB) {
      bp = bread(dev, BBLOCK(b, sb));
      // look for free blocks here
      for(bi = 0; bi < BPB && b + bi < sb.size; bi++){
        // cprintf("balloc: checking bmap for block %d...\n", b + bi);
        m = 1 << (bi % 8);
        if((bp->data[bi/8] & m) == 0){  // Is block free?
          bp->data[bi/8] |= m;  // Mark block in use.
          log_write(bp);
          brelse(bp);
          bzero(dev, b + bi);
          // cprintf("balloc: found free block: %d\n", b + bi);
          return b + bi;
        }
      }
      brelse(bp);
    }
  }
  panic("balloc: out of blocks");
}

// Allocate a zeroed disk block in the given block group, if possible.
// If not possible, return 0.
static uint
balloci(uint dev, uint bgroupnum)
{
  int b, bi, m;
  struct buf *bp;

  int firstblock = BBLOCKGROUPSTART(bgroupnum, sb);
  
  // cprintf("balloci: dev: %d bgroupnum: %d firstblock: %d\n", dev, bgroupnum, firstblock);

  // inode block number containing this inode
  b = firstblock + sb.bgroupmeta;
  // iterate through data blocks of this block group
  for(; b < firstblock + sb.bgroupsize; b += BPB) {
    bp = bread(dev, BBLOCK(b, sb));
    // look for free blocks here
    for(bi = 0; bi < BPB && b + bi < sb.size; bi++){
      // cprintf("balloci: checking bmap for block %d...\n", b + bi);
      m = 1 << (bi % 8);
      if((bp->data[bi/8] & m) == 0){  // Is block free?
        bp->data[bi/8] |= m;  // Mark block in use.
        log_write(bp);
        brelse(bp);
        bzero(dev, b + bi);
        // cprintf("balloci: found free block: %d\n", b + bi);
        return b + bi;
      }
    }
    brelse(bp);
  }
  return 0; // failed
}


// Free a disk block.
static void
bfree(int dev, uint b)
{
  struct buf *bp;
  int bi, m;

  // cprintf("bfree: dev: %d b: %d\n", dev, b);

  readsb(dev, &sb);
  bp = bread(dev, BBLOCK(b, sb));
  // bi = b % BPB;
  bi = BOFFSET(b, sb);
  m = 1 << (bi % 8);
  if((bp->data[bi/8] & m) == 0)
    panic("freeing free block");
  bp->data[bi/8] &= ~m;
  log_write(bp);
  brelse(bp);
}

// Inodes.
//
// An inode describes a single unnamed file.
// The inode disk structure holds metadata: the file's type,
// its size, the number of links referring to it, and the
// list of blocks holding the file's content.
//
// The inodes are laid out sequentially on disk at
// sb.startinode. Each inode has a number, indicating its
// position on the disk.
//
// The kernel keeps a cache of in-use inodes in memory
// to provide a place for synchronizing access
// to inodes used by multiple processes. The cached
// inodes include book-keeping information that is
// not stored on disk: ip->ref and ip->flags.
//
// An inode and its in-memory represtative go through a
// sequence of states before they can be used by the
// rest of the file system code.
//
// * Allocation: an inode is allocated if its type (on disk)
//   is non-zero. ialloc() allocates, iput() frees if
//   the link count has fallen to zero.
//
// * Referencing in cache: an entry in the inode cache
//   is free if ip->ref is zero. Otherwise ip->ref tracks
//   the number of in-memory pointers to the entry (open
//   files and current directories). iget() to find or
//   create a cache entry and increment its ref, iput()
//   to decrement ref.
//
// * Valid: the information (type, size, &c) in an inode
//   cache entry is only correct when the I_VALID bit
//   is set in ip->flags. ilock() reads the inode from
//   the disk and sets I_VALID, while iput() clears
//   I_VALID if ip->ref has fallen to zero.
//
// * Locked: file system code may only examine and modify
//   the information in an inode and its content if it
//   has first locked the inode. The I_BUSY flag indicates
//   that the inode is locked. ilock() sets I_BUSY,
//   while iunlock clears it.
//
// Thus a typical sequence is:
//   ip = iget(dev, inum)
//   ilock(ip)
//   ... examine and modify ip->xxx ...
//   iunlock(ip)
//   iput(ip)
//
// ilock() is separate from iget() so that system calls can
// get a long-term reference to an inode (as for an open file)
// and only lock it for short periods (e.g., in read()).
// The separation also helps avoid deadlock and races during
// pathname lookup. iget() increments ip->ref so that the inode
// stays cached and pointers to it remain valid.
//
// Many internal file system functions expect the caller to
// have locked the inodes involved; this lets callers create
// multi-step atomic operations.

struct {
  struct spinlock lock;
  struct inode inode[NINODE];
} icache;

void
iinit(int dev)
{
  initlock(&icache.lock, "icache");
  readsb(dev, &sb);
  // cprintf("sb: size %d nblocks %d ninodes %d nlog %d logstart %d inodestart %d bmap start %d\n", sb.size,
  //         sb.nblocks, sb.ninodes, sb.nlog, sb.logstart, sb.inodestart, sb.bmapstart);

  cprintf("sb: size %d nblocks %d ninodes %d nlog %d logstart %d bgroupmeta %d nblockgroups %d bgroupstart %d\n", sb.size,
          sb.nblocks, sb.ninodes, sb.nlog, sb.logstart, sb.bgroupmeta, sb.nblockgroups, sb.bgroupstart);
  cprintf("bgroupsize %d inodesperbgroup %d inodeblocksperbgroup %d bmapblocksperbgroup %d datablocksperbgroup %d\n",
          sb.bgroupsize, sb.inodesperbgroup, sb.inodeblocksperbgroup, sb.bmapblocksperbgroup, sb.datablocksperbgroup);

}

static struct inode* iget(uint dev, uint inum);

//PAGEBREAK!
// Allocate a new inode with the given type on device dev.
// A free inode has a type of zero.
// If the given type is of T_DIR, then we look for the least-utilized block
// group, meaning the block group with the most free blocks.
// If the given type is of T_FILE, we use given pinum to try and allocate
// an inode int he same block group as the parent directory.
struct inode*
ialloc(uint dev, short type, uint pinum)
{
  int inum;
  struct buf *bp;
  struct dinode *dip;

  int until = sb.ninodes; // loop until heere
  inum = 1;

  if (type == T_DIR) {
    int blockgroup = least_utilized_bgroup(); // get an inode from this block group
    inum = FIRSTINODEOFBGROUP(blockgroup, sb);
    until = inum + sb.inodesperbgroup; // loop until the last inode in this block group
    if (inum == 0)
      inum = 1; // no inode 0
  } else if (type == T_FILE) {
    // try to find an inode in the same block group as pinum.
    int blockgroup = IBLOCKGROUP(pinum, sb);
    int start = FIRSTINODEOFBGROUP(blockgroup, sb);
    int end = start + sb.inodesperbgroup;
    if (start == 0)
      start = 1; // no inode 0
    int i;
    for (i = start; i < end; i++) {
      bp = bread(dev, IBLOCK(i, sb));
      dip = (struct dinode*)bp->data + i%IPB;
      if(dip->type == 0){  // a free inode
        memset(dip, 0, sizeof(*dip));
        dip->type = type;
        log_write(bp);   // mark it allocated on the disk
        brelse(bp);
        return iget(dev, i);
      }
      brelse(bp);
    }

    // if we reach here, fall down to search all block groups for a free inode.
  }

  for(; inum < until; inum++){
    bp = bread(dev, IBLOCK(inum, sb));
    dip = (struct dinode*)bp->data + inum%IPB;
    if(dip->type == 0){  // a free inode
      memset(dip, 0, sizeof(*dip));
      dip->type = type;
      log_write(bp);   // mark it allocated on the disk
      brelse(bp);
      return iget(dev, inum);
    }
    brelse(bp);
  }

  panic("ialloc: no inodes");
}

// Copy a modified in-memory inode to disk.
void
iupdate(struct inode *ip)
{
  struct buf *bp;
  struct dinode *dip;

  bp = bread(ip->dev, IBLOCK(ip->inum, sb));
  dip = (struct dinode*)bp->data + ip->inum%IPB;
  dip->type = ip->type;
  dip->major = ip->major;
  dip->minor = ip->minor;
  dip->nlink = ip->nlink;
  dip->size = ip->size;
  memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
  log_write(bp);
  brelse(bp);
}

// Find the inode with number inum on device dev
// and return the in-memory copy. Does not lock
// the inode and does not read it from disk.
static struct inode*
iget(uint dev, uint inum)
{
  struct inode *ip, *empty;

  acquire(&icache.lock);

  // Is the inode already cached?
  empty = 0;
  for(ip = &icache.inode[0]; ip < &icache.inode[NINODE]; ip++){
    if(ip->ref > 0 && ip->dev == dev && ip->inum == inum){
      ip->ref++;
      release(&icache.lock);
      return ip;
    }
    if(empty == 0 && ip->ref == 0)    // Remember empty slot.
      empty = ip;
  }

  // Recycle an inode cache entry.
  if(empty == 0)
    panic("iget: no inodes");

  ip = empty;
  ip->dev = dev;
  ip->inum = inum;
  ip->ref = 1;
  ip->flags = 0;
  release(&icache.lock);

  return ip;
}

// Increment reference count for ip.
// Returns ip to enable ip = idup(ip1) idiom.
struct inode*
idup(struct inode *ip)
{
  acquire(&icache.lock);
  ip->ref++;
  release(&icache.lock);
  return ip;
}

// Lock the given inode.
// Reads the inode from disk if necessary.
void
ilock(struct inode *ip)
{
  struct buf *bp;
  struct dinode *dip;

  if(ip == 0 || ip->ref < 1)
    panic("ilock");

  acquire(&icache.lock);
  while(ip->flags & I_BUSY)
    sleep(ip, &icache.lock);
  ip->flags |= I_BUSY;
  release(&icache.lock);

  if(!(ip->flags & I_VALID)){
    bp = bread(ip->dev, IBLOCK(ip->inum, sb));
    dip = (struct dinode*)bp->data + ip->inum%IPB;
    ip->type = dip->type;
    ip->major = dip->major;
    ip->minor = dip->minor;
    ip->nlink = dip->nlink;
    ip->size = dip->size;
    memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
    brelse(bp);
    ip->flags |= I_VALID;
    if(ip->type == 0)
      panic("ilock: no type");
  }
}

// Unlock the given inode.
void
iunlock(struct inode *ip)
{
  if(ip == 0 || !(ip->flags & I_BUSY) || ip->ref < 1)
    panic("iunlock");

  acquire(&icache.lock);
  ip->flags &= ~I_BUSY;
  wakeup(ip);
  release(&icache.lock);
}

// Drop a reference to an in-memory inode.
// If that was the last reference, the inode cache entry can
// be recycled.
// If that was the last reference and the inode has no links
// to it, free the inode (and its content) on disk.
// All calls to iput() must be inside a transaction in
// case it has to free the inode.
void
iput(struct inode *ip)
{
  acquire(&icache.lock);
  if(ip->ref == 1 && (ip->flags & I_VALID) && ip->nlink == 0){
    // inode has no links and no other references: truncate and free.
    if(ip->flags & I_BUSY)
      panic("iput busy");
    ip->flags |= I_BUSY;
    release(&icache.lock);
    itrunc(ip);
    ip->type = 0;
    iupdate(ip);
    acquire(&icache.lock);
    ip->flags = 0;
    wakeup(ip);
  }
  ip->ref--;
  release(&icache.lock);
}

// Common idiom: unlock, then put.
void
iunlockput(struct inode *ip)
{
  iunlock(ip);
  iput(ip);
}

//PAGEBREAK!
// Inode content
//
// The content (data) associated with each inode is stored
// in blocks on the disk. The first NDIRECT block numbers
// are listed in ip->addrs[].  The next NINDIRECT blocks are 
// listed in block ip->addrs[NDIRECT].

// Return the disk block address of the nth block in inode ip.
// If there is no such block, bmap allocates one.
static uint
bmap(struct inode *ip, uint bn)
{
  uint addr, *a;
  struct buf *bp;

  uint bgroupnum = IBLOCKGROUP(ip->inum, sb);
  // chunking. increment bgroupnum depending on what bn is. For large files
  // with more than NDIRECT data blocks, the blocks will be chunked and spread
  // across multiple block groups.
  bgroupnum += (bn / NDIRECT); // chunk size is NDIRECT blocks
  bgroupnum = bgroupnum % sb.nblockgroups; // loop back to front block group if necessary

  if(bn < NDIRECT){
    if((addr = ip->addrs[bn]) == 0) {
      uint blocknum = balloci(ip->dev, bgroupnum);
      if (blocknum != 0)
        ip->addrs[bn] = addr = blocknum;
      else
        ip->addrs[bn] = addr = balloc(ip->dev);
    }
    return addr;
  }
  bn -= NDIRECT;

  if(bn < NINDIRECT){
    // Load indirect block, allocating if necessary.
    if((addr = ip->addrs[NDIRECT]) == 0) {
      uint blocknum = balloci(ip->dev, bgroupnum);
      if (blocknum != 0)
        ip->addrs[NDIRECT] = addr = blocknum;
      else
        ip->addrs[NDIRECT] = addr = balloc(ip->dev);
    }
    bp = bread(ip->dev, addr);
    a = (uint*)bp->data;
    if((addr = a[bn]) == 0){
      uint blocknum = balloci(ip->dev, bgroupnum);
      if (blocknum != 0)
        a[bn] = addr = blocknum;
      else
        a[bn] = addr = balloc(ip->dev);
      log_write(bp);
    }
    brelse(bp);
    return addr;
  }

  panic("bmap: out of range");
}

// Truncate inode (discard contents).
// Only called when the inode has no links
// to it (no directory entries referring to it)
// and has no in-memory reference to it (is
// not an open file or current directory).
static void
itrunc(struct inode *ip)
{
  int i, j;
  struct buf *bp;
  uint *a;

  for(i = 0; i < NDIRECT; i++){
    if(ip->addrs[i]){
      bfree(ip->dev, ip->addrs[i]);
      ip->addrs[i] = 0;
    }
  }
  
  if(ip->addrs[NDIRECT]){
    bp = bread(ip->dev, ip->addrs[NDIRECT]);
    a = (uint*)bp->data;
    for(j = 0; j < NINDIRECT; j++){
      if(a[j])
        bfree(ip->dev, a[j]);
    }
    brelse(bp);
    bfree(ip->dev, ip->addrs[NDIRECT]);
    ip->addrs[NDIRECT] = 0;
  }

  ip->size = 0;
  iupdate(ip);
}

// Copy stat information from inode.
void
stati(struct inode *ip, struct stat *st)
{
  st->dev = ip->dev;
  st->ino = ip->inum;
  st->type = ip->type;
  st->nlink = ip->nlink;
  st->size = ip->size;
}

//PAGEBREAK!
// Read data from inode.
int
readi(struct inode *ip, char *dst, uint off, uint n)
{
  uint tot, m;
  struct buf *bp;

  if(ip->type == T_DEV){
    if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
      return -1;
    return devsw[ip->major].read(ip, dst, n);
  }

  if(off > ip->size || off + n < off)
    return -1;
  if(off + n > ip->size)
    n = ip->size - off;

  for(tot=0; tot<n; tot+=m, off+=m, dst+=m){
    bp = bread(ip->dev, bmap(ip, off/BSIZE));
    m = min(n - tot, BSIZE - off%BSIZE);
    memmove(dst, bp->data + off%BSIZE, m);
    brelse(bp);
  }
  return n;
}

// PAGEBREAK!
// Write data to inode.
int
writei(struct inode *ip, char *src, uint off, uint n)
{
  uint tot, m;
  struct buf *bp;

  if(ip->type == T_DEV){
    if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
      return -1;
    return devsw[ip->major].write(ip, src, n);
  }

  if(off > ip->size || off + n < off)
    return -1;
  if(off + n > MAXFILE*BSIZE)
    return -1;

  for(tot=0; tot<n; tot+=m, off+=m, src+=m){

    bp = bread(ip->dev, bmap(ip, off/BSIZE));
    m = min(n - tot, BSIZE - off%BSIZE);
    memmove(bp->data + off%BSIZE, src, m);
    log_write(bp);
    brelse(bp);
  }

  if(n > 0 && off > ip->size){
    ip->size = off;
    iupdate(ip);
  }
  return n;
}

//PAGEBREAK!
// Directories

int
namecmp(const char *s, const char *t)
{
  return strncmp(s, t, DIRSIZ);
}

// Look for a directory entry in a directory.
// If found, set *poff to byte offset of entry.
struct inode*
dirlookup(struct inode *dp, char *name, uint *poff)
{
  uint off, inum;
  struct dirent de;

  if(dp->type != T_DIR)
    panic("dirlookup not DIR");

  for(off = 0; off < dp->size; off += sizeof(de)){
    if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("dirlink read");
    if(de.inum == 0)
      continue;
    if(namecmp(name, de.name) == 0){
      // entry matches path element
      if(poff)
        *poff = off;
      inum = de.inum;
      return iget(dp->dev, inum);
    }
  }

  return 0;
}

// Write a new directory entry (name, inum) into the directory dp.
int
dirlink(struct inode *dp, char *name, uint inum)
{
  int off;
  struct dirent de;
  struct inode *ip;

  // Check that name is not present.
  if((ip = dirlookup(dp, name, 0)) != 0){
    iput(ip);
    return -1;
  }

  // Look for an empty dirent.
  for(off = 0; off < dp->size; off += sizeof(de)){
    if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("dirlink read");
    if(de.inum == 0)
      break;
  }

  strncpy(de.name, name, DIRSIZ);
  de.inum = inum;
  if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
    panic("dirlink");
  
  return 0;
}

//PAGEBREAK!
// Paths

// Copy the next path element from path into name.
// Return a pointer to the element following the copied one.
// The returned path has no leading slashes,
// so the caller can check *path=='\0' to see if the name is the last one.
// If no name to remove, return 0.
//
// Examples:
//   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
//   skipelem("///a//bb", name) = "bb", setting name = "a"
//   skipelem("a", name) = "", setting name = "a"
//   skipelem("", name) = skipelem("////", name) = 0
//
static char*
skipelem(char *path, char *name)
{
  char *s;
  int len;

  while(*path == '/')
    path++;
  if(*path == 0)
    return 0;
  s = path;
  while(*path != '/' && *path != 0)
    path++;
  len = path - s;
  if(len >= DIRSIZ)
    memmove(name, s, DIRSIZ);
  else {
    memmove(name, s, len);
    name[len] = 0;
  }
  while(*path == '/')
    path++;
  return path;
}

// Look up and return the inode for a path name.
// If parent != 0, return the inode for the parent and copy the final
// path element into name, which must have room for DIRSIZ bytes.
// Must be called inside a transaction since it calls iput().
static struct inode*
namex(char *path, int nameiparent, char *name)
{
  struct inode *ip, *next;

  if(*path == '/')
    ip = iget(ROOTDEV, ROOTINO);
  else
    ip = idup(proc->cwd);

  while((path = skipelem(path, name)) != 0){
    ilock(ip);
    if(ip->type != T_DIR){
      iunlockput(ip);
      return 0;
    }
    if(nameiparent && *path == '\0'){
      // Stop one level early.
      iunlock(ip);
      return ip;
    }
    if((next = dirlookup(ip, name, 0)) == 0){
      iunlockput(ip);
      return 0;
    }
    iunlockput(ip);
    ip = next;
  }
  if(nameiparent){
    iput(ip);
    return 0;
  }
  return ip;
}

struct inode*
namei(char *path)
{
  char name[DIRSIZ];
  return namex(path, 0, name);
}

struct inode*
nameiparent(char *path, char *name)
{
  return namex(path, 1, name);
}
