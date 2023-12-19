#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"
#include "mbox.h"

int closePid;
//-------------------------------------------------------
//
// void MboxModuleInit();
//
// Initialize all mailboxes.  This process does not need
// to worry about synchronization as it is called at boot
// time.  Only initialize necessary items here: you can
// initialize others in MboxCreate.  In other words, 
// don't waste system resources like locks and semaphores
// on unused mailboxes.
//
//-------------------------------------------------------

mbox mboxes[MBOX_NUM_MBOXES];
mbox_message msgs[MBOX_NUM_BUFFERS];
int msg_no_count;
void MboxModuleInit() {
  int i;
  msg_no_count = 0;
  for (i = 0; i < MBOX_NUM_MBOXES; i++) {
    mboxes[i].inuse = 0;
    mboxes[i].count = 0;
    mboxes[i].empty = -1;
    mboxes[i].full = -1;
  }
}

//-------------------------------------------------------
//
// mbox_t MboxCreate();
//
// Allocate an available mailbox structure for use. 
//
// Returns the mailbox handle on success
// Returns MBOX_FAIL on error.
//
//-------------------------------------------------------
mbox_t MboxCreate() {
  mbox_t m;
  uint32 intrval;
  int j;

  DisableIntrs(intrval);
  for (m = 0; m < MBOX_NUM_MBOXES; m++) {
    if (!mboxes[m].inuse) {
    mboxes[m].inuse = 1;
    mboxes[m].empty = SemCreate(MBOX_MAX_BUFFERS_PER_MBOX);
    mboxes[m].full = SemCreate(0);
    AQueueInit(&mboxes[m].msgs);
      for (j = 0; j < 32; j++) {
        mboxes[m].processes[j] = -1;
      }
      break;
    }
  }

  RestoreIntrs(intrval);
  if (m == MBOX_NUM_MBOXES) return MBOX_FAIL;

  dbprintf('b', "%d\n", m);
  return m;
}

//-------------------------------------------------------
// 
// void MboxOpen(mbox_t);
//
// Open the mailbox for use by the current process.  Note
// that it is assumed that the internal lock/mutex handle 
// of the mailbox and the inuse flag will not be changed 
// during execution.  This allows us to get the a valid 
// lock handle without a need for synchronization.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxOpen(mbox_t handle) {
  int i;

  if (handle >= MBOX_NUM_MBOXES) return MBOX_FAIL;

  for (i = 0; i < 32; i++) {
    if (mboxes[handle].processes[i] == -1) {
      mboxes[handle].processes[i] = GetCurrentPid();
      mboxes[handle].count++;
      break;
    }
  }

  if (mboxes[handle].count > 32) return MBOX_FAIL; //might change to 31

  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxClose(mbox_t);
//
// Close the mailbox for use to the current process.
// If the number of processes using the given mailbox
// is zero, then disable the mailbox structure and
// return it to the set of available mboxes.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxClose(mbox_t handle) {
  int i;

  if (handle >= MBOX_NUM_MBOXES) {
    dbprintf('b', "handle too big %d\n", handle);
    return MBOX_FAIL;
  }

  if (!mboxes[handle].count) {
    mboxes[handle].inuse = 0;
    return MBOX_SUCCESS;
  } 

  for (i = 0; i < 32; i++) {
    dbprintf('b', "Process pid %d\n", mboxes[handle].processes[i]);
    if (mboxes[handle].processes[i] == closePid) {
      mboxes[handle].processes[i] = -1;
      mboxes[handle].count--;
      break;
    }
  }
  dbprintf('b', "\n");
  if (i == 32) {
    dbprintf('b', "Iterated through all mail boxes\n");
    return MBOX_FAIL;
  }
  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxSend(mbox_t handle,int length, void* message);
//
// Send a message (pointed to by "message") of length
// "length" bytes to the specified mailbox.  Messages of
// length 0 are allowed.  The call 
// blocks when there is not enough space in the mailbox.
// Messages cannot be longer than MBOX_MAX_MESSAGE_LENGTH.
// Note that the calling process must have opened the 
// mailbox via MboxOpen.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxSend(mbox_t handle, int length, void* message) {
  int i;
  Link *l;
  Link *first;
  Link *it;

  if (handle >= MBOX_NUM_MBOXES) return MBOX_FAIL;

  //printf("Sending\n");
  printf("handle %d\n", handle);
  for (i = 0; i < 32; i++) {
    if (mboxes[handle].processes[i] == GetCurrentPid()) break;
  }

  if (i == 32) return MBOX_FAIL;

  if (length > MBOX_MAX_MESSAGE_LENGTH) return MBOX_FAIL;

  if (msg_no_count == MBOX_NUM_BUFFERS) return MBOX_FAIL;

/*  if (mboxes[handle].msgs.nitems == MBOX_MAX_BUFFERS_PER_MBOX) {
    dbprintf('b', "Queue full! lock acquired, Sem: %d\n", mboxes[handle].sem);
    SemHandleWait(mboxes[handle].sem);
  }else{*/
    

  //printf("%s %s\n", m.buf, (char *)message);
 /* first = mboxes[handle].msgs.first;
  it = first;
  printf("Pre insert\n");
  while (it) {
    printf("%s\n", ((mbox_message *)AQueueObject(it))->buf);
    it = it->next;
  }*/
  SemHandleWait(mboxes[handle].empty);

  bcopy((char *)message, msgs[msg_no_count].buf, length);
  printf("Sending %s\n", (char *)message);
  l = AQueueAllocLink(&msgs[msg_no_count]);
 // dbprintf('b', "link inserted address %p\n", l);
  if (AQueueInsertLast(&(mboxes[handle].msgs), l) == QUEUE_FAIL) return MBOX_FAIL;
  
  first = mboxes[handle].msgs.first;
  it = first;
  while (it) {
    printf("%s\n", ((mbox_message *)AQueueObject(it))->buf);
    it = it->next;
  }
  SemHandleSignal(mboxes[handle].full);
  //printf("%s\n", ((mbox_message *)AQueueObject(AQueueLast(&mboxes[handle].msgs)))->buf);
//  if( mboxes[handle].msgs.nitems == 1) {
//    SemHandleSignal(mboxes[handle].sem);
    //dbprintf('b', "Sem released LOCK: %d\n", mboxes[handle].lock);
//  }



  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxRecv(mbox_t handle, int maxlength, void* message);
//
// Receive a message from the specified mailbox.  The call 
// blocks when there is no message in the buffer.  Maxlength
// should indicate the maximum number of bytes that can be
// copied from the buffer into the address of "message".  
// An error occurs if the message is larger than maxlength.
// Note that the calling process must have opened the mailbox 
// via MboxOpen.
//   
// Returns MBOX_FAIL on failure.
// Returns number of bytes written into message on success.
//
//-------------------------------------------------------
int MboxRecv(mbox_t handle, int maxlength, void* message) {
  int i;
  mbox_message *msg;
  Link *l;
  Link *first;
  Link *it;
  if (handle >= MBOX_NUM_MBOXES) return MBOX_FAIL;
  //dbprintf('b', "Receiving\n");
  printf("handle %d\n", handle);
  for (i = 0; i < 32; i++) {
    if (mboxes[handle].processes[i] == GetCurrentPid()) break;
  }

  if (i == 32) return MBOX_FAIL;
  if (maxlength > MBOX_MAX_MESSAGE_LENGTH) return MBOX_FAIL;

/*  if (AQueueEmpty(&mboxes[handle].msgs)) {
    //dbprintf('b', "Queue empty! lock acquired LOCK: %d\n", mboxes[handle].lock);
    printf("Waiting\n");
    SemHandleWait(mboxes[handle].sem);
  }else{*/
  
  first = mboxes[handle].msgs.first;
  it = first;
  while (it) {
    printf("%s\n", ((mbox_message *)AQueueObject(it))->buf);
    it = it->next;
  }
  SemHandleWait(mboxes[handle].full);
  l = AQueueFirst(&mboxes[handle].msgs);
  dbprintf('b', "Link removed address %p\n", l);
  msg = (mbox_message *)AQueueObject(l);
  bcopy(msg->buf, (char *)message, maxlength);

  printf("Receiving %s\n", (char *)message);
  if (AQueueRemove(&l) == QUEUE_FAIL) {
    dbprintf('b', "Queue empty\n");
    return MBOX_FAIL;
  }

  SemHandleSignal(mboxes[handle].empty);
  msg_no_count--;
 /* if (mboxes[handle].msgs.nitems ==  MBOX_MAX_BUFFERS_PER_MBOX - 1){
    SemHandleSignal(mboxes[handle].sem);
  //  dbprintf('b', "Lock released PID: %d, LOCK: %d\n", GetCurrentPid(), mboxes[handle].lock); 
  }  */

//  }

  return maxlength;
}

//--------------------------------------------------------------------------------
// 
// int MboxCloseAllByPid(int pid);
//
// Scans through all mailboxes and removes this pid from their "open procs" list.
// If this was the only open process, then it makes the mailbox available.  Call
// this function in ProcessFreeResources in process.c.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//--------------------------------------------------------------------------------
int MboxCloseAllByPid(int pid) {
  int i;
  int j;
  for (i = 0; i < MBOX_MAX_BUFFERS_PER_MBOX; i++) {
    for (j = 0; j < 32; j++) {
      if (mboxes[i].processes[j] == pid) {
        closePid = pid;
        dbprintf('b', "Attempting to close mailbox at pid %d\n", pid);
       if (MboxClose(i) == MBOX_FAIL) return MBOX_FAIL;
      }
    }
  }
  
  return MBOX_SUCCESS;
}
