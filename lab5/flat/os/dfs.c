#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"
#include "files.h"

static dfs_inode inodes[192]; // all inodes
static dfs_superblock sb; // superblock
static uint32 fbv[512]; // Free block vector

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }

//locks needed defined static
static lock_t inode_lock;
static lock_t fbv_lock;   //should be how to declare these but getting an error on visual studio code
lock_t file_lock;
file_descriptor fds[FILE_MAX_OPEN_FILES];

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

// STUDENT: put your file system level functions below.
// Some skeletons are provided. You can implement additional functions.

///////////////////////////////////////////////////////////////////
// Non-inode functions first
///////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsModuleInit is called at boot time to initialize things and
// open the file system for use.
//-----------------------------------------------------------------

//Changes here Invalidate then aquire locks then open filesystem
void DfsModuleInit() {
  // You essentially set the file system as invalid and then open 
  // using DfsOpenFileSystem().
  int i;
  //printf("DfsModuleInit start.\n");
  DfsInvalidate();
  fbv_lock = LockCreate();
  inode_lock = LockCreate();
  //file.c stuff since theres no module init func in files.c
  file_lock = LockCreate();
  for (i = 0; i < FILE_MAX_OPEN_FILES; i++) {
    fds[i].filename = NULL;
    fds[i].inode_handle = -1;
    fds[i].pos = 0;
    fds[i].eof = 0;
    fds[i].mode = NULL;
    fds[i].pid = -1;
  }
  //printf("%d %d\n", fbv_lock, inode_lock);
  //  sb.valid = 1;         //do we want this since we are supposed to invalidate first?
  DfsOpenFileSystem();

  //printf("Finishing intialization in dfs.\n");
  //printf("%x %x %x %x %x %x\n", sb.valid, sb.dfs_blocksize, sb.dfs_blocks, sb.inode_loc, sb.no_inodes, sb.freeblock_loc);
}


//-----------------------------------------------------------------
// DfsInavlidate marks the current version of the filesystem in
// memory as invalid.  This is really only useful when formatting
// the disk, to prevent the current memory version from overwriting
// what you already have on the disk when the OS exits.
//-----------------------------------------------------------------

void DfsInvalidate() {
  // This is just a one-line function which sets the valid bit of the 
  // superblock to 0.
  sb.valid = 0;
}

//-------------------------------------------------------------------
// DfsOpenFileSystem loads the file system metadata from the disk
// into memory.  Returns DFS_SUCCESS on success, and DFS_FAIL on 
// failure.
//-------------------------------------------------------------------

uint32 stringToInt(char *ptr, int from, int to) {
  int i;
  uint32 ret;
  int bit_pos = 0;
  for (i = from; i <= to; i++) {
    ret |= ptr[from] << bit_pos;
    bit_pos += 8;
  }
  return ret;
}

int DfsOpenFileSystem() {
  //Basic steps:
  // Check that filesystem is not already open

  // Read superblock from disk.  Note this is using the disk read rather 
  // than the DFS read function because the DFS read requires a valid 
  // filesystem in memory already, and the filesystem cannot be valid 
  // until we read the superblock. Also, we don't know the block size 
  // until we read the superblock, either.

  // Copy the data from the block we just read into the superblock in memory
  int i;
  disk_block superblock;
  dfs_block inode_block;
  dfs_block fbv_block;
  int loc_within_block = 0;
  //printf("Starting DfsOpenFileSystem.\n");

  if (sb.valid) {
    printf("Error FileSystem is open.\n");
    return DFS_FAIL;
  }

  if (DiskReadBlock(1, &superblock) == DISK_FAIL) {
    printf("Failed to read the superblock from the disk\n");
    return DFS_FAIL;
  }

/*  for (i = 0; i < sizeof(superblock.data); i++) {
    printf("%x", superblock.data[i]);
  }
  printf("\n");*/
  /*sb.valid = stringToInt(superblock.data, 0, 3);
  sb.dfs_blocksize = stringToInt(superblock.data, 4, 7);
  sb.dfs_blocks= stringToInt(superblock.data, 8, 11);
  sb.inode_loc= stringToInt(superblock.data, 12, 15);
  sb.no_inodes = stringToInt(superblock.data, 16, 19);
  sb.freeblock_loc = stringToInt(superblock.data, 20, 23);*/
  //bcopy((char *)&superblock, (char *)&sb, sizeof(sb));
  bcopy((char *)&superblock, (char *)&sb, sizeof(sb));
  // All other blocks are sized by virtual block size:
  // Read inodes
  // Read free block vector
  // Change superblock to be invalid, write back to disk, then change 
  // it back to be valid in memory

  //read inodes

  //printf("%x %x %x %x %x %x\n", sb.valid, sb.dfs_blocksize, sb.dfs_blocks, sb.inode_loc, sb.no_inodes, sb.freeblock_loc);
  for (i = sb.inode_loc; i < sb.freeblock_loc; i++) {
    if (DfsReadBlock(i, &inode_block) == DFS_FAIL) {
      printf("Failed to read inode %d from the disk\n", i);
      return DFS_FAIL;
    }
    bcopy((char *)&inode_block, (char *)&inodes[(i - sb.inode_loc) * sb.dfs_blocksize], sb.dfs_blocksize);
  }

  //_ead fbv
  for (i = 0; i < 2; i++) {
    if (DfsReadBlock(sb.freeblock_loc + i, &fbv_block) == DFS_FAIL) {
      printf("Failed to read fbv from the disk\n");
      return DFS_FAIL;
    }
    bcopy((char *)&fbv_block, (char *)(fbv + loc_within_block), sb.dfs_blocksize);
    loc_within_block += sb.dfs_blocksize;
  }
  //DfsInvalidate();
  //sb.valid = 0;		//for some fucking reason sb.valid is already invalid
  //printf("\nsb.valid: %d\n",sb.valid);

  //write back invalid superblock
  //bzero(superblock.data, DISK_BLOCKSIZE);
  bcopy((char *)&sb, (char *)&superblock, sizeof(sb));
  //bcopy((char *)&sb, superblock.data, sizeof(sb));
  /*for (i = 0; i < sizeof(superblock.data); i++) {
    printf("%x", superblock.data[i]);
  }*/
  printf("\n");
  if (DiskWriteBlock(1, &superblock) == DISK_FAIL) {
     printf("Failed to write superblock back to disk\n");
     return DFS_FAIL;
   }
  sb.valid = 1;
  dbprintf('d', "Finishing DfsOpenFileSystem.\n");
  return DFS_SUCCESS;
}


//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
//-------------------------------------------------------------------

int DfsCloseFileSystem() {
  //basically just some pseudocode for now
  dfs_block inode_block;
  dfs_block fbv_block;
  // char * info;
  int i;
  int loc_within_block = 0;

  dbprintf('d', "Starting DfsCloseFileSystem.\n");
  //make sure file system is open
  /*if (!sb.valid) {
    printf("Error FileSystem is closed.\n");
    return DFS_FAIL;
  }*/

  //need the information to write
  //  info = (char *) inodes;

/*  for (i = 0; i < 16; i++) {
    if (DfsWriteBlock(1 + i, &inode_block) == DFS_FAIL) {
      printf("Failed to write inodes from the disk\n");
      return DFS_FAIL;
    }
  }*/
  for (i = sb.inode_loc; i < sb.freeblock_loc; i++) {
    bcopy((char *)&inodes[(i - sb.inode_loc) * sb.dfs_blocksize], (char *)&inode_block, sb.dfs_blocksize);
    if (DfsWriteBlock(i, &inode_block) == DFS_FAIL) {
      printf("Failed to write inode %d from the disk\n", i);
      return DFS_FAIL;
    }
  }

  for (i = 0; i < 2; i++) {
    bcopy((char *)fbv, (char *)&fbv_block, sb.dfs_blocksize);
    if (DfsWriteBlock(sb.freeblock_loc + i, &fbv_block) == DFS_FAIL) {
      printf("Failed to write fbv_block from the disk\n");
      return DFS_FAIL;
    }
    loc_within_block += sb.dfs_blocksize;
  }

  sb.valid = 0;   //invalidate after finishing
  dbprintf('d', "Finishing Filesys Close.\n");
  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use 
// locks where necessary.
//-----------------------------------------------------------------

uint32 DfsAllocateBlock() {
  // Check that file system has been validly loaded into memory
  // Find the first free block using the free block vector (FBV), mark it in use
  // Return handle to block
  int i = 0;
  int j = 0;
  int bit;
  int handle;

  dbprintf('d', "DfsAllocateBlock starting.\n");

  /*if(!sb.valid) {
    printf("DfsAllocateBlock cancled file system is not open.\n");
    //return DFS_FAIL;
    return 0x4000; //invalid handle lol
  }*/

  if (sb.dfs_blocksize != DFS_BLOCKSIZE) {
    return 0x4000;
  }

  LockHandleAcquire(fbv_lock);

  //fbv is the critical selection since it is a shared resource among readers/writers
  while (fbv[i]) {
    i++;
  }
  while (!bit) {
    bit = (fbv[i] >> j) & 0x1;
    j++;
  }
  fbv[i] &= invert(0x1 << j);
  handle = i * 32 + j;
  LockHandleRelease(fbv_lock);

  dbprintf('d', "DfsAllocateBlock finishing.\n");
  return handle; //dfs block handle
}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------
//added a lock aquire here and to make sure the filesystem is open if not we should return
int DfsFreeBlock(uint32 blocknum) {
  int index = blocknum / 32;
  int bit_pos = blocknum % 32;

  dbprintf('d', "Start DfsFreeBlock.\n");

  /*if(!sb.valid) {
    printf("Error can't allocate, file system is invalid.\n");
    return DFS_FAIL;
  }*/

  LockHandleAcquire(fbv_lock);
  

  //fbv is the critical selection since it is a shared resource among readers/writers
  fbv[index] |= 1 << bit_pos;

  LockHandleRelease(fbv_lock);

  dbprintf('d', "Finishing DfsFreeBlock.\n");

  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------

int DfsReadBlock(uint32 blocknum, dfs_block *b) {
  int i;
  disk_block physical_block;
  int bytesPerBlock = DiskBytesPerBlock();
  int diskblocknum = sb.dfs_blocksize / bytesPerBlock * blocknum;
  int hold = sb.dfs_blocksize / bytesPerBlock;
  int written = 0;
  if(fbv[blocknum/32] & (1<<(blocknum % 32))){
    printf("block isnt allocated: %d\n", blocknum);
    return DFS_FAIL;
  }
  for (i = 0; i < hold; i++) {
    if (DiskReadBlock(diskblocknum + i, &physical_block) == DISK_FAIL) {
    dbprintf('d', "Failed to read disk block %d\n", diskblocknum);
    return DFS_FAIL;
    }
    bcopy((char *)physical_block.data, (char *)b->data + written, DISK_BLOCKSIZE);
    written += bytesPerBlock;
  }
  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------

int DfsWriteBlock(uint32 blocknum, dfs_block *b){
  int i;
  //int diskblocknum = 2 * blocknum;
  disk_block physical_block;
  int bytesPerBlock = DiskBytesPerBlock();
  int hold = sb.dfs_blocksize / bytesPerBlock;
  int written = 0;

//  printf("hold: %d, bytesPerBlock: %d blocksize: %d\n", hold, bytesPerBlock,sb.dfs_blocksize);
  if(fbv[blocknum/32] & (1<<(blocknum % 32))){
    printf("block isnt allocated: %d\n", blocknum);
    return DFS_FAIL;
  }
  //printf("Writing DFS blocknum %d...\n", blocknum);
  for (i = 0; i < hold; i++) {
    //might want a bzero
    bzero(physical_block.data, bytesPerBlock);
    bcopy(&(b->data[i * bytesPerBlock]), physical_block.data, bytesPerBlock);
   // printf("dfs blocknum i: %d\n", i);
    if (DiskWriteBlock(blocknum * hold  + i, &physical_block) == DISK_FAIL) {		//why diskblocknum?
      //dbprintf('d', "Failed to write disk blocknum: %d, hold: %d, i: %d\n", blocknum, hold, i);
    //  printf("Failed to write disk blocknum: %d, hold: %d, i: %d\n", blocknum, hold, i);
      return DFS_FAIL;
    }
    written += bytesPerBlock;
  }
  return written;//hold * bytesPerBlock;
}


////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for 
// the given filename. If the filename is found, return the handle 
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------
int readInodes() {
  int i = 0;
  int counter = 0;
  dfs_block inode_block;
  while (i < sizeof(inodes) / sizeof(inodes[0])) {
    //printf("%d\n", sb.inode_loc + counter);
    if (DfsReadBlock(sb.inode_loc + counter, &inode_block) == DFS_FAIL) {
      return DFS_FAIL;
    }
    bcopy((char *)&inode_block, (char *)&inodes[i], sb.dfs_blocksize);
    i += sizeof(dfs_block) / sizeof(inodes[0]);
    counter++;
  }
  return DFS_SUCCESS;
}

int DfsInodeFilenameExists(char *filename) {

  int i;
  if (!sb.valid) {
    return DFS_FAIL;
  }

  readInodes();
  for(i = 0; i < sb.no_inodes; i++){ 

    //file name match return handle 
    printf("%s %s\n", filename, inodes[i].filename);
    if(inodes[i].inuse){
      printf("%s %s\n", filename, inodes[i].filename);
      if(!dstrncmp(filename, inodes[i].filename, MAX_FILENAME_LENGTH)){ 
        return i; //handle
      }
    } 
  }

  return DFS_FAIL; //not found so fail

}


int writeInodes() {
  int i = 0;
  int counter = 0;
  dfs_block inode_block;
  while (i < sizeof(inodes) / sizeof(inodes[0])) {
    bcopy((char *)&inodes[i], (char *)&inode_block, sb.dfs_blocksize);
    //printf("%d\n", sb.inode_loc + counter);
    if (DfsWriteBlock(sb.inode_loc + counter, &inode_block) == DFS_FAIL) {
      return DFS_FAIL;
    }
    i += sizeof(dfs_block) / sizeof(inodes[0]);
    counter++;
  }
  return DFS_SUCCESS;
}
//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------

uint32 DfsInodeOpen(char *filename) {
  int i;
  for (i = 0; i < sizeof(inodes) / sizeof(inodes[0]); i++) {
    if (inodes[i].inuse) {
      if (!dstrncmp(filename, inodes[i].filename, MAX_FILENAME_LENGTH)) {
        break;
      }
    }
  }
  LockHandleAcquire(inode_lock);

  for (i = 0; i < sizeof(inodes) / sizeof(inodes[0]); i++) {
    if (!inodes[i].inuse) {
      printf("%s created with handle %d\n", filename, i);
      inodes[i].inuse = 1;
      inodes[i].fileSize = 0;
      dstrcpy(filename, inodes[i].filename);
      break;
    }
  }

  printf("Opening inode %d and writing inodes to disk\n", i);
  writeInodes();
  LockHandleRelease(inode_lock);
  return i;
}


//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode.Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------

int DfsInodeDelete(uint32 handle) {
  int i;
  dfs_block zeroes;
  dfs_block table;
  if (handle >= sizeof(inodes) / sizeof(inodes[0])) { //handle larger than bounds of array
    return DFS_FAIL;
  }

  LockHandleAcquire(inode_lock);

  if (!inodes[handle].inuse) { //invalid node
    return DFS_FAIL;
  }

  bzero((char *)&zeroes, sizeof(zeroes));
  //handle direct addressing

  //loop through translation table and write a bunch of 0s
  for (i = 0; i < 10; i++) {
    /*if (DfsWriteBlock(inodes[handle].translationTable[i], &zeroes) == DFS_FAIL) {
      return DFS_FAIL;
    }*/
    if (DfsFreeBlock(inodes[handle].translationTable[i]) == DFS_FAIL) {
      return DFS_FAIL;
    }
  }

  //handle indirect addressing later
  if (inodes[handle].newBlockLoc) {
    if (DfsReadBlock(inodes[handle].newBlockLoc, &table) == DFS_FAIL) {
      return DFS_FAIL;
    }
    //fetch table entries and zero them out
    for (i = 0; i < sizeof(table); i+=4) {
    /*  if (DfsWriteBlock(stringToInt(table.data, i, i + 4), &zeroes) == DFS_FAIL) {
        return DFS_FAIL;
      }*/
      if (DfsFreeBlock(stringToInt(table.data, i, i + 4)) == DFS_FAIL) {
        return DFS_FAIL;
      }
    }
  }
 // bzero((char *)&inodes[handle], sizeof(dfs_inode));
  inodes[handle].inuse = 0;
  writeInodes();
  LockHandleRelease(inode_lock);
  return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
  int physical_block_no;
  int offset;
  dfs_block read_block;
  int currentByte;
  //translate start byte to virtual block no.
  //calculate offset to figure out where in the disk block to copy to mem
  //translate virtual block to physical block
  //read physical block to memory

  currentByte = start_byte;
  while (currentByte < start_byte + num_bytes) {
    physical_block_no = DfsInodeTranslateVirtualToFilesys(handle, currentByte);
    offset = start_byte % sb.dfs_blocksize;
    if (DfsReadBlock(physical_block_no, &read_block) == DFS_FAIL) {
        dbprintf('d', "Failed to read block from file system\n");
        return DFS_FAIL;
    }
    bcopy((char *)(&read_block + offset), (char *)mem + currentByte - start_byte, num_bytes);
    currentByte += sb.dfs_blocksize;
  }
  return currentByte - start_byte;
}


//-----------------------------------------------------------------
// DfsInodeWriteBytes writes num_bytes from the memory pointed to 
// by mem to the file represented by the inode handle, starting at 
// virtual byte start_byte. Note that if you are only writing part 
// of a given file system block, you'll need to read that block 
// from the disk first. Return DFS_FAIL on failure and the number 
// of bytes written on success.
//-----------------------------------------------------------------

int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
  int physical_block_no;
  int offset;
  int currentByte;
  dfs_block write_block;
  //translate start byte to virtual block no.
  //calculate offset to figure out where the destination address is within the block
  //translate virtual block to physical block
  //write memory to filesys block

  currentByte = start_byte ;
  offset = start_byte % sb.dfs_blocksize;
  while (currentByte < start_byte + num_bytes) {
    physical_block_no = DfsInodeTranslateVirtualToFilesys(handle, currentByte);
    if (DfsReadBlock(physical_block_no, &write_block) == DFS_FAIL) {
        dbprintf('d', "Failed to read block from file system\n");
        return DFS_FAIL;
    }
    bcopy((char *)mem + currentByte, (char *)(&write_block + offset), num_bytes);
    if (DfsWriteBlock(physical_block_no, &write_block + offset) == DFS_FAIL) {
        dbprintf('d', "Failed to read block from file system\n");
        return DFS_FAIL;
    }
    currentByte += sb.dfs_blocksize;
  }
  if (inodes[handle].fileSize <  currentByte) {
    inodes[handle].fileSize = currentByte;
  }
  inodes[handle].fileSize += num_bytes;
  return currentByte - start_byte;
}


//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

int DfsInodeFilesize(uint32 handle) {
  if (handle >= sizeof(inodes) / sizeof(inodes[0])) {
    dbprintf('d', "handle out of bounds!\n");
    return DFS_FAIL;
  }
  if (!inodes[handle].inuse) {
    dbprintf('d', "invalid inode!\n");
    return DFS_FAIL;
  }
  return inodes[handle].fileSize;
}


//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block 
// for the given inode, storing its blocknumber at index 
// virtual_blocknumber in the translation table. If the 
// virtual_blocknumber resides in the indirect address space, and 
// there is not an allocated indirect addressing table, allocate it. 
// Return DFS_FAIL on failure, and the newly allocated file system 
// block number on success.
//-----------------------------------------------------------------

int DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) {
  int fbv_index;
  int fbv_bitnum;
  int new_block;
  dfs_block new_dfs_block;
  if (handle >= sizeof(inodes) / sizeof(inodes[0])) {
    printf("Out of bounds\n");
    return DFS_FAIL;
  }
  LockHandleAcquire(inode_lock);
  if (!inodes[handle].inuse) {
    printf("invalid inode\n");
    return DFS_FAIL;
  }

  //if larger than 10, then it must reside in the indirect address space
  if (virtual_blocknum >= 10) {
    fbv_index = virtual_blocknum / 32;
    fbv_bitnum = virtual_blocknum % 32;
    if (!inodes[handle].newBlockLoc) { //check if dfs block is free if so allocate a block
      inodes[handle].newBlockLoc = DfsAllocateBlock();
    }
    new_block = DfsAllocateBlock(); //new physical block this will be the value in the translation lookup table
    if (DfsReadBlock(inodes[handle].newBlockLoc, &new_dfs_block) == DFS_FAIL) {
      return DFS_FAIL;
    }
    bcopy((char *)&new_block, new_dfs_block.data + virtual_blocknum - 10, sizeof(new_block));
    if (DfsWriteBlock(inodes[handle].newBlockLoc, &new_dfs_block) == DFS_FAIL) {
      return DFS_FAIL;
    }
  }
  else {
    inodes[handle].translationTable[virtual_blocknum] = DfsAllocateBlock();
  }
  LockHandleRelease(inode_lock);
  return DFS_SUCCESS; 
}



//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum) {
  dfs_block block;
  uint32 fs_block = 0;
  if (handle >= sizeof(inodes) / sizeof(inodes[0])) {
    printf("Out of bounds\n");
    GracefulExit();
  }
  LockHandleAcquire(inode_lock);
  if (!inodes[handle].inuse) {
    printf("invalid inode\n");
    GracefulExit();
  }

  if (virtual_blocknum < 10) {
    return inodes[handle].translationTable[virtual_blocknum];
  }
  else {
    if (DfsReadBlock(inodes[handle].newBlockLoc, &block) == DFS_FAIL) {
      printf("Failed to read block\n");
      GracefulExit();
    }
    fs_block = dstrtol(&block.data[virtual_blocknum - 10], NULL, 10);
  }
  return fs_block;
}
