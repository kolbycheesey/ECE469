#ifndef __DFS_SHARED__
#define __DFS_SHARED__

typedef struct dfs_superblock {
  // STUDENT: put superblock internals here
  uint32 valid;
  uint32 dfs_blocksize; //filesys block size
  uint32 dfs_blocks; //number of filesys blocks
  uint32 inode_loc; //start of inode array
  uint32 no_inodes;
  uint32 freeblock_loc;
} dfs_superblock;

#define DFS_BLOCKSIZE 1024  // Must be an integer multiple of the disk blocksize
#define MAX_FILENAME_LENGTH 44

typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;

typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 128 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this, 
  // adjust the maximumm length of the filename until the size of the overall inode 
  // is 128 bytes.
  int inuse;
  char filename[MAX_FILENAME_LENGTH];
  uint32 translationTable[10];
  uint32 newBlockLoc;
  uint32 fileSize;
} dfs_inode;

#define DFS_MAX_FILESYSTEM_SIZE 0x1000000  // 16MB

#define DFS_FAIL -1
#define DFS_SUCCESS 1



#endif
