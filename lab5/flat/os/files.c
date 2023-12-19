#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

// STUDENT: put your file-level functions here
//
extern file_descriptor fds[FILE_MAX_OPEN_FILES]; //array containing all the file descriptors
extern lock_t file_lock;
uint32 FileOpen(char *filename, char *mode) {
  int inode_handle;
  int i;
  for (i = 0; i < FILE_MAX_OPEN_FILES; i++) {
    if (!dstrncmp(fds[i].filename, filename, FILE_MAX_FILENAME_LENGTH) && fds[i].pid != GetCurrentPid()) return FILE_FAIL;
    if (fds[i].filename == NULL) break;
  }
  inode_handle = DfsInodeOpen(filename);
  if (inode_handle == DFS_FAIL) return FILE_FAIL;
  LockHandleAcquire(file_lock);
  dstrcpy(fds[i].filename, filename);
  fds[i].inode_handle = inode_handle;
  fds[i].pos = 0;
  fds[i].eof = 0;
  dstrcpy(fds[i].mode, mode);
  fds[i].pid = GetCurrentPid();
  LockHandleRelease(file_lock);
  return 0;
}

int FileClose(uint32 handle) {
  if (handle >= FILE_MAX_OPEN_FILES) return FILE_FAIL;
  // if (DfsInodeDelete(fds[handle].inode_handle) == DFS_FAIL) return FILE_FAIL;
  fds[handle].filename = NULL;
  fds[handle].inode_handle = -1;
  fds[handle].pos = 0;
  fds[handle].eof = 0;
  fds[handle].mode = NULL;
  fds[handle].pid = -1;
  return FILE_SUCCESS;
}

int FileDelete(char *filename) {
  uint32 start;
  if ((start = DfsInodeFilenameExists(filename)) == DFS_FAIL) return FILE_FAIL;

  if ((DfsInodeDelete(start)) == DFS_FAIL) return FILE_FAIL;
  return FILE_SUCCESS;
}

int FileRead(uint32 handle, char *mem, int num_bytes) {
  int bytes_read;
  int filesize;
  if (handle >= FILE_MAX_OPEN_FILES) return FILE_FAIL;
  if ((filesize = DfsInodeFilesize(fds[handle].inode_handle)) == DFS_FAIL) return FILE_FAIL;
  if (num_bytes > 4096 || fds[handle].eof) return FILE_FAIL;
  if (DfsInodeOpen(fds[handle].filename) == FILE_FAIL) return FILE_FAIL;
  if (!(!dstrncmp("r", fds[handle].mode, 1) || !dstrncmp("rw", fds[handle].mode, 2))) return FILE_FAIL;
  if ((bytes_read = DfsInodeReadBytes(fds[handle].inode_handle, (void *)mem, fds[handle].pos, num_bytes)) == FILE_FAIL) return FILE_FAIL;
  if (fds[handle].pos + num_bytes >= filesize) {
    fds[handle].eof = 1;
  }
  return bytes_read;
}

int FileWrite(uint32 handle, char *mem, int num_bytes) {
  int bytes_written;
  if (handle >= FILE_MAX_OPEN_FILES) return FILE_FAIL;
  if (num_bytes > 4096 || fds[handle].eof) return FILE_FAIL;
  if (!(!dstrncmp("w", fds[handle].mode, 1) || !dstrncmp("rw", fds[handle].mode, 2))) return FILE_FAIL;
  if (DfsInodeOpen(fds[handle].filename) == FILE_FAIL) return FILE_FAIL;
  if ((bytes_written = DfsInodeWriteBytes(fds[handle].inode_handle, (void *)mem, fds[handle].pos, num_bytes)) == FILE_FAIL) return FILE_FAIL;
  return bytes_written;
}

int FileSeek(uint32 handle, int num_bytes, int from_where) {
  int filesize;
  int seek_loc;

  if (!fds[handle].mode) return FILE_FAIL;

  fds[handle].eof = 0;
  if ((filesize = DfsInodeFilesize(fds[handle].inode_handle)) == DFS_FAIL) return FILE_FAIL;

  if (from_where == FILE_SEEK_SET) {
      if ((num_bytes < 0) || (num_bytes > filesize)) return FILE_FAIL;
      fds[handle].pos = num_bytes;
  }

  seek_loc = fds[handle].pos + num_bytes;

  if (from_where == FILE_SEEK_CUR) {
    if ((seek_loc < 0) || (seek_loc > filesize)) return FILE_FAIL;
        fds[handle].pos += num_bytes;
  }

  if (from_where == FILE_SEEK_END) {
    if ((filesize + num_bytes < 0) || (num_bytes > 0)) return FILE_FAIL;
    fds[handle].pos = filesize + num_bytes;
  }
  return FILE_SUCCESS;
}
