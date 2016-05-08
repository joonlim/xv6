/**
 * Test Fast File System 2
 *
 * Test heuristic 2 (placing files in the same directory in the same block group (when possible)).
 */

#include "types.h"
#include "user.h"
#include "fs.h"

#define TESTDIR1 "_testdir1"
#define TESTDIR2 "_testdir2"
#define TESTFILE1 "test1"
#define TESTFILE2 "test2"
#define TESTFILE3 "test3"
#define TESTFILE4 "test4"
#define TESTFILE5 "test5"

int
makeandopendir(char *name)
{
  printf(1, "Making new directory... ");
  if (mkdir(name) < 0) {
  	printf(2, "failed call to mkdir\n");
  	exit();
  }
  int fd;
  if((fd = open(name, 0)) < 0){
    printf(2, "failed call to open\n");
    exit();
  }
  return fd;
}

int
main(int argc, char *argv[])
{
  printf(1, "This tests the second heurstic of the Fast File System.\n");
  printf(1, "This test shows how files created in the same directory tend to be in the same\n"\
    "block group. To prove this point, this test creates 2 separate directories, and\n"\
    "creates 5 files in each one. Then we display the block group of all 10 files.\n\n");

  int fd;
  struct fbgstat fbg;

  // dir 1
  printf(1, "Making new directory %s...\n", TESTDIR1);
  if (mkdir(TESTDIR1) < 0) {
    printf(2, "failed call to mkdir\n");
    exit();
  }

  // dir 1 file 1
  printf(1, "Making file %s/%s...\n", TESTDIR1, TESTFILE1);
  if ((fd = open(TESTDIR1"/"TESTFILE1, 0x001 | 0x002 | 0x200)) < 0) {
    printf(2, "failed call to open\n");
    exit();
  }
  fbgstat(fd, &fbg);
  printf(1, "%s/%s block group: %d\n", TESTDIR1, TESTFILE1, fbg.inodebgroup);
  close(fd);

  // dir 1 file 2
  printf(1, "Making file %s/%s...\n", TESTDIR1, TESTFILE2);
  if ((fd = open(TESTDIR1"/"TESTFILE2, 0x001 | 0x002 | 0x200)) < 0) {
    printf(2, "failed call to open\n");
    exit();
  }
  fbgstat(fd, &fbg);
  printf(1, "%s/%s block group: %d\n", TESTDIR1, TESTFILE2, fbg.inodebgroup);
  close(fd);

  // dir 1 file 3
  printf(1, "Making file %s/%s...\n", TESTDIR1, TESTFILE3);
  if ((fd = open(TESTDIR1"/"TESTFILE3, 0x001 | 0x002 | 0x200)) < 0) {
    printf(2, "failed call to open\n");
    exit();
  }
  fbgstat(fd, &fbg);
  printf(1, "%s/%s block group: %d\n", TESTDIR1, TESTFILE3, fbg.inodebgroup);
  close(fd);

  // dir 1 file 4
  printf(1, "Making file %s/%s...\n", TESTDIR1, TESTFILE4);
  if ((fd = open(TESTDIR1"/"TESTFILE4, 0x001 | 0x002 | 0x200)) < 0) {
    printf(2, "failed call to open\n");
    exit();
  }
  fbgstat(fd, &fbg);
  printf(1, "%s/%s block group: %d\n", TESTDIR1, TESTFILE4, fbg.inodebgroup);
  close(fd);

  // dir 1 file 5
  printf(1, "Making file %s/%s...\n", TESTDIR1, TESTFILE5);
  if ((fd = open(TESTDIR1"/"TESTFILE5, 0x001 | 0x002 | 0x200)) < 0) {
    printf(2, "failed call to open\n");
    exit();
  }
  fbgstat(fd, &fbg);
  printf(1, "%s/%s block group: %d\n", TESTDIR1, TESTFILE5, fbg.inodebgroup);
  close(fd);

  // dir 2
  printf(1, "Making new directory %s...\n", TESTDIR2);
  if (mkdir(TESTDIR2) < 0) {
    printf(2, "failed call to mkdir\n");
    exit();
  }

  // dir 2 file 1
  printf(1, "Making file %s/%s...\n", TESTDIR2, TESTFILE1);
  if ((fd = open(TESTDIR2"/"TESTFILE1, 0x001 | 0x002 | 0x200)) < 0) {
    printf(2, "failed call to open\n");
    exit();
  }
  fbgstat(fd, &fbg);
  printf(1, "%s/%s block group: %d\n", TESTDIR2, TESTFILE1, fbg.inodebgroup);
  close(fd);

  // dir 2 file 2
  printf(1, "Making file %s/%s...\n", TESTDIR2, TESTFILE2);
  if ((fd = open(TESTDIR2"/"TESTFILE2, 0x001 | 0x002 | 0x200)) < 0) {
    printf(2, "failed call to open\n");
    exit();
  }
  fbgstat(fd, &fbg);
  printf(1, "%s/%s block group: %d\n", TESTDIR2, TESTFILE2, fbg.inodebgroup);
  close(fd);

  // dir 2 file 3
  printf(1, "Making file %s/%s...\n", TESTDIR2, TESTFILE3);
  if ((fd = open(TESTDIR2"/"TESTFILE3, 0x001 | 0x002 | 0x200)) < 0) {
    printf(2, "failed call to open\n");
    exit();
  }
  fbgstat(fd, &fbg);
  printf(1, "%s/%s block group: %d\n", TESTDIR2, TESTFILE3, fbg.inodebgroup);
  close(fd);

  // dir 2 file 4
  printf(1, "Making file %s/%s...\n", TESTDIR2, TESTFILE4);
  if ((fd = open(TESTDIR2"/"TESTFILE4, 0x001 | 0x002 | 0x200)) < 0) {
    printf(2, "failed call to open\n");
    exit();
  }
  fbgstat(fd, &fbg);
  printf(1, "%s/%s block group: %d\n", TESTDIR2, TESTFILE4, fbg.inodebgroup);
  close(fd);

  // dir 2 file 5
  printf(1, "Making file %s/%s...\n", TESTDIR2, TESTFILE5);
  if ((fd = open(TESTDIR2"/"TESTFILE5, 0x001 | 0x002 | 0x200)) < 0) {
    printf(2, "failed call to open\n");
    exit();
  }
  fbgstat(fd, &fbg);
  printf(1, "%s/%s block group: %d\n", TESTDIR2, TESTFILE5, fbg.inodebgroup);
  close(fd);


  // delete all created files and directories
  printf(1, "Cleaning up...\n");
  if (unlink(TESTDIR1"/"TESTFILE1) < 0 
    || unlink(TESTDIR1"/"TESTFILE2) < 0
    || unlink(TESTDIR1"/"TESTFILE3) < 0
    || unlink(TESTDIR1"/"TESTFILE4) < 0
    || unlink(TESTDIR1"/"TESTFILE5) < 0
    || unlink(TESTDIR2"/"TESTFILE1) < 0
    || unlink(TESTDIR2"/"TESTFILE2) < 0
    || unlink(TESTDIR2"/"TESTFILE3) < 0
    || unlink(TESTDIR2"/"TESTFILE4) < 0
    || unlink(TESTDIR2"/"TESTFILE5) < 0
    || unlink(TESTDIR1) < 0
    || unlink(TESTDIR2) < 0) {
  	printf(2, "failed to delete\n");
  	exit();
  }
  
  exit();
}
