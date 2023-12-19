#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"

dfs_superblock sb;
dfs_inode inodes[DFS_INODE_MAX_NUM];
uint32 fbv[DFS_FBV_MAX_NUM_WORDS];

int diskblocksize = 0; // These are global in order to speed things up
int disksize = 0;      // (i.e. fewer traps to OS to get the same number)
int numfilesystemblocks = 0;

int FdiskWriteBlock(uint32 blocknum, dfs_block *b); //You can use your own function. This function 
//calls disk_write_block() to write physical blocks to disk

static inline uint32 invert(uint32 n) {
  return n ^ 0xffffffff;
}
void setFreeBlock(int block_num) {
  int index = block_num / 32;
  int bit_num = block_num % 32;
  fbv[index] |= (1 << bit_num);
}

void clearFreeBlock(int block_num) {
  int index = block_num / 32;
  int bit_num = block_num % 32;
  //Printf("%x\n", (1 << bit_num) ^ 0xffffffff);
 // Printf("%d\n", (1 << bit_num) - 2);
  if (bit_num < 31) {
  fbv[index] &= invert(1 << bit_num);
  }
  else {
  fbv[index] &= invert(1 << 31);
  }
}

void main (int argc, char *argv[])
{
  // STUDENT: put your code here. Follow the guidelines below. They are just the main steps. 
  // You need to think of the finer details. You can use bzero() to zero out bytes in memory

  //Initializations and argc check
  int i;
  dfs_block block;
  int disk_block_loc = 0;

  // Need to invalidate filesystem before writing to it to make sure that the OS
  // doesn't wipe out what we do here with the old version in memory
  // You can use dfs_invalidate(); but it will be implemented in Problem 2. You can just do 
  sb.valid = 0;

  disksize = DFS_MAX_FILESYSTEM_SIZE;
  diskblocksize = DFS_BLOCKSIZE / 2;
  numfilesystemblocks = disksize / DFS_BLOCKSIZE;

  // Make sure the disk exists before doing anything else
  if (disk_create() == DISK_FAIL) {
    Printf("Failed to create disk!\n");
    Exit();
  }

  // Write all inodes as not in use and empty (all zeros)

  bzero((char *)inodes, sizeof(inodes));
  // Next, setup free block vector (fbv) and write free block vector to the disk

  bzero((char *)fbv, sizeof(fbv));
  //disk blocks 0-1 are now occupied due to superblock and boot record
  for (i = 0; i < numfilesystemblocks; i++) {
    setFreeBlock(i);
  }
  for (i = 0; i < 2; i++) {
    clearFreeBlock(i);
  }
  //fbv
  Printf("Created disk\n");
  for (i = 2; i < 30; i++) {
    clearFreeBlock(i);
  }
  for (i = 30; i < 38; i++) {
    clearFreeBlock(i);
  }
  for (i = 38; i < 41; i++) {
    clearFreeBlock(i);
  }
  // Finally, setup superblock as valid filesystem and write superblock and boot record to disk: 
  sb.valid = 1;
  sb.dfs_blocksize = DFS_BLOCKSIZE;
  sb.dfs_blocks = numfilesystemblocks;
  sb.inode_loc = FDISK_INODE_BLOCK_START;
  sb.no_inodes = FDISK_NUM_INODES;
  sb.freeblock_loc = FDISK_FBV_BLOCK_START;
  // boot record is all zeros in the first physical block, and superblock structure goes into the second physical block
  bzero(block.data, sizeof(block.data));
  FdiskWriteBlock(0, &block);
  bcopy((char *)&sb, block.data, sizeof(sb));
  bzero(block.data + sizeof(sb), sizeof(block.data) - sizeof(sb));
  FdiskWriteBlock(1, &block);

  //write fbv
  for (i = 0; i < 4; i++) {
    bcopy((char *)(fbv + disk_block_loc), (char *)&block, diskblocksize);
    FdiskWriteBlock(19 * 2 + i, &block);
    disk_block_loc += diskblocksize;
  }

  /*for(i = 0; i<FDISK_INODE_NUM_BLOCKS; i++){		//error thrown: cant write block larger than filesystem size
    bcopy((char *)(inodes) + disk_block_loc, (char *)&block, diskblocksize);
    if(FdiskWriteBlock(FDISK_INODE_BLOCK_START + i, &block) == DISK_FAIL){
      Printf("inode writing failed in main of fdisk.c\n");
    }
    disk_block_loc += diskblocksize;
  }*/


  Printf("fdisk (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);
}

int FdiskWriteBlock(uint32 blocknum, dfs_block *b) {
  // STUDENT: put your code here
  if (disk_write_block(blocknum, b->data) == DISK_FAIL) {
    //Printf("Failed to write to disk!\n");
    return DISK_FAIL; //failure
  }
  return DISK_SUCCESS; //success
}
