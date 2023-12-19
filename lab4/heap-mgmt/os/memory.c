//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"

// num_pages = size_of_memory / size_of_one_page
static uint32 freemap[MEM_PT_SIZE / 32];
//static uint32 pagestart;
static int nfreepages = 0;
static int freemapmax = 0;
static AllocTree allocTree[255];

//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
}

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}


void setFreemap(int page_num) {
  int index = page_num / 32;
  int bit_num = page_num % 32;
  freemap[index] |= 1 << bit_num;
}
//----------------------------------------------------------------------
//
//	MemoryModuleInit
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------

void MemoryModuleInit() {
  int i;
  int first_page = (lastosaddress + MEM_PAGESIZE - 4) / MEM_PAGESIZE;
  int last_page = MemoryGetSize() / MEM_PAGESIZE;
  freemapmax = (last_page + 31) / 32;
  dbprintf('m',"Initializing memory module\n");
  for (i = 0; i < freemapmax; i++) {
    freemap[i] = 0;
  }

  //set page tables after os to be free 
  nfreepages = 0;
  for (i = first_page; i < last_page; i++) {
    setFreemap(i);
    nfreepages++;
  }
  for (i = 0; i < 255; i++) {
    allocTree[i].addr = 0;
    allocTree[i].inuse = 0;
    allocTree[i].chunkSize = 0;
  }
}


//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {
  int pageNumber;
  int physicalAddress;
  //int virtualPageAddress;
  int offset;

  offset = addr & MEM_ADDRESS_OFFSET_MASK;
  pageNumber = MemoryGetPageNumber(addr); 
  //printf("%x\n", pageNumber);
  //check valid bit
  /*  if (!(pcb->pagetable[pageNumber] & 0x1)) {
     pcb->currentSavedFrame[PROCESS_STACK_FAULT] = addr;
     MemoryPageFaultHandler(pcb);
    }*/
  physicalAddress = (pcb->pagetable[pageNumber] & invert(MEM_ADDRESS_OFFSET_MASK)) + offset;
  //printf("Memory address translated from %d to %d\n", addr, physicalAddress);
  return physicalAddress;
}


//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
  unsigned char *curUser;         // Holds current physical address representing user-space virtual address
  int		bytesCopied = 0;  // Running counter
  int		bytesToCopy;      // Used to compute number of bytes left in page to be copied

  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
    //printf("Translating %d\n", (int)user);
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);
    // If we could not translate address, exit now
    dbprintf('m',"Translating %p to %p Data: %s\n", user, curUser, (char *)curUser);
    if (curUser == (unsigned char *)0) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this page 
    // and the total number of bytes left to copy ("n").

    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
    bytesToCopy = MEM_PAGESIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);

    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    //printf("Copy\n");
    if (dir >= 0) {
      bcopy (system, curUser, bytesToCopy);
    } else {
      bcopy (curUser, system, bytesToCopy);
    }
   // printf("Copy complete\n");

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;           // Total number of bytes left to copy
    bytesCopied += bytesToCopy; // Total number of bytes copied thus far
    system += bytesToCopy;      // Current address in system space to copy next bytes from/into
    user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  //printf("%d\n", bytesCopied);
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault 
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page 
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that 
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference. 
// Feel free to edit.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) {
  uint32 faulting_address = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
  uint32 stack_pointer = pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER];
  uint32 page_index = MemoryGetPageNumber(faulting_address);
  uint32 page_number;

  stack_pointer &= 0x1FF000;		//clearing last 12 bits as the note from the TA

	//right shift and left shift
	//clear both fault address and stack pointer clear the last 12
  dbprintf('m',"Fault %x SP %x\n", faulting_address, stack_pointer);
  printf("we got here fault: %u, stack: %u\n",faulting_address, stack_pointer);
  if (faulting_address >= stack_pointer) {
    page_number = MemoryAllocPage();
    pcb->pagetable[page_index] = MemorySetupPte(page_number);
    return MEM_SUCCESS;
  }
  printf("Segmentation fault (core dumped)\n");
  printf("%p\n", pcb);
  ProcessKill();
  return MEM_FAIL;
}


//---------------------------------------------------------------------
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

//return allocated page number
int MemoryAllocPage(void) {
  int pageNumber;
  int i = 0;
  int j = 0;
  int free_page_found = 0;
  if (nfreepages == 0) return -1;
  while (!freemap[i] && i < freemapmax) {
    i++;
  }
  if (i >= freemapmax) {
    printf("no free page i %d freemapmax %d!\n", i, freemapmax);
    return -1;
  }
  while (!free_page_found) {
    free_page_found = (freemap[i] >> j) & 0x1;
    j++;
  }
  freemap[i] &= invert(1 << --j);
  pageNumber = i * 32 + j;
  nfreepages--;
  dbprintf('m',"successfully allocated page\n");
  return pageNumber;
}


uint32 MemorySetupPte (uint32 page) {
  dbprintf('m', "Setting up pte Address: %p\n", (uint32 *)(page * MEM_PAGESIZE));
  return page * MEM_PAGESIZE | MEM_PTE_VALID;
}

uint32 MemoryGetPageNumber(uint32 addr) {
  return addr >> MEM_L1FIELD_FIRST_BITNUM;
}


void MemoryFreePage(uint32 page) {
  dbprintf('m', "Freeing page %d\n", page);
  setFreemap(page);
  nfreepages++;
}

int mallocHelper(int currChunkSize, int requestedSize, int addr, int currOrder, int arrIndex) {
  int lChild = 0;
  int rChild = 0;
  int lChildIndex = 0;
  int rChildIndex = 0;
  lChildIndex = 2 * arrIndex + 1;
  rChildIndex = 2 * arrIndex + 2;;
  if (currChunkSize/ 2 < requestedSize || currChunkSize == 32) {
  //  printf("%d %d\n", arrIndex, currChunkSize);
    if (allocTree[arrIndex].inuse) {
      printf("Already used\n");
      return 0; //fail
    }
    else {
      allocTree[arrIndex].inuse = 1;
      allocTree[arrIndex].chunkSize = currChunkSize;
      printf("Allocated the block: order = %d, addr = %d, requested mem size %d, block size = %d\n", currOrder, addr, requestedSize, currChunkSize);
      return allocTree[arrIndex].addr;
    }
  }
  if (allocTree[arrIndex].inuse && !allocTree[lChildIndex].addr && !allocTree[rChildIndex].addr) { //leaf node that is already used
    printf("Leaf node\n");
    return 0;
  }
  //create nodes
  if (!allocTree[arrIndex].inuse) {
    printf("node created\n");
    allocTree[arrIndex].inuse = 1;
    allocTree[arrIndex].addr = addr;
    allocTree[lChildIndex].addr = addr;
    allocTree[rChildIndex].addr = addr ^ (currChunkSize >> 1);
  }
  lChild = mallocHelper(currChunkSize >> 1,  requestedSize, addr, currOrder - 1, lChildIndex);
  if (lChild) {
  printf("Created a left child node (order = %d, addr = %d, size = %d) of parent (order = %d, addr = %d, size %d\n", currOrder - 1, addr, currChunkSize >> 1, currOrder, addr, currChunkSize);
  }
  else {
    rChild = mallocHelper(currChunkSize >> 1, requestedSize, addr ^ (currChunkSize >> 1), currOrder - 1, rChildIndex);
    if (rChild) {
  printf("Created a right child node (order = %d, addr = %d, size = %d) of parent (order = %d, addr = %d, size %d\n", currOrder - 1, addr ^ (currChunkSize >> 1), currChunkSize >> 1, currOrder, addr, currChunkSize);
    }
  }
  return lChild | rChild;
}

void *malloc(int memsize) {
  printf("Entering malloc\n");
  return (void *)mallocHelper(0x1000, memsize, 0x4000, 7, 0);
}

int freeHelper(int ptr, int addr, int index, int currChunkSize, int order) {
  int leftChild = 0;
  int rightChild = 0;
  if (!ptr) return -1;
  if (currChunkSize == allocTree[index].chunkSize) {
    if (!allocTree[index].inuse) {
      printf("fail %d %d\n", addr, currChunkSize);
      return -1; //failure
    }
    allocTree[index].inuse = 0;
    printf("Freed the block: order = %d, addr = %d, size = %d\n", order, addr, currChunkSize);
    return 0; //success
}
  if (ptr <= addr || ptr < (addr ^ (currChunkSize >> 1))) {
    leftChild = freeHelper(ptr, addr, 2 * index + 1, currChunkSize >> 1, order - 1);
  }
  else {
    rightChild = freeHelper(ptr, addr ^ (currChunkSize >> 1), 2 * index + 2, currChunkSize >> 1, order - 1);
  }
  if (!allocTree[2 * index + 1].inuse && !allocTree[2 * index + 2].inuse) {
    allocTree[index].inuse = 0;
    //destroy child nodes
    allocTree[2 * index + 1].addr = 0;
    allocTree[2 * index + 2].addr = 0;
    allocTree[2 * index + 1].chunkSize = 0;
    allocTree[2 * index + 2].chunkSize = 0;
    printf("Coalesced buddy nodes (order = %d, addr = %d, size = %d) & (order = %d, addr = %d, size = %d) into the parent node (order = %d, addr = %d", order - 1, addr, (currChunkSize >> 1),  order - 1, addr ^ (currChunkSize >> 1), (currChunkSize >> 1), order, addr);
    printf(", size = %d)\n", currChunkSize);
  }
  return leftChild | rightChild;
}

int mfree(void *ptr) {
  return freeHelper((int)ptr, 0x4000, 0, 0x1000, 7);
}
