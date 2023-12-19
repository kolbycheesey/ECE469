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
  dbprintf('m', "we got here fault: %u, stack: %u\n",faulting_address, stack_pointer);
  if (faulting_address >= stack_pointer) {
    if (pcb->npages >= 32) {
        printf("Cannot allocate anymore pages!\n");
        return MEM_FAIL;
    }
    page_number = MemoryAllocPage();
    pcb->pagetable[page_index] = MemorySetupPte(page_number);
    pcb->npages++;
    referenceCounter[page_number]++;
    return MEM_SUCCESS;
  }
  printf("Segmentation fault (core dumped)\n");
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

uint32 MemoryGetOffset(uint32 addr) {
  return addr & MEM_ADDRESS_OFFSET_MASK;
}

void MemoryFreePage(uint32 page) {
  dbprintf('m', "Freeing page %d\n", page);
  setFreemap(page);
  nfreepages++;
}

void MemoryROPAccessHandler(PCB *childPCB) {
  uint32 faulting_address;
  uint32 page_number;
  uint32 physical_addr;
  uint32 physical_page;
  uint32 new_page;
  int i;
  uint32 parent_pagetable[MEM_PT_SIZE];

  //obtain faulting address and pages
  faulting_address = childPCB->currentSavedFrame[PROCESS_STACK_FAULT];
  page_number = MemoryGetPageNumber(faulting_address);
  physical_addr = MemoryTranslateUserToSystem(childPCB, faulting_address);
  physical_page = MemoryGetPageNumber(physical_addr);
  bcopy((char *)childPCB->pagetable, (char *)parent_pagetable, sizeof(uint32) * MEM_PT_SIZE);
  //handle cases
  if (referenceCounter[physical_page] > 1) {
    //create a new page
    new_page = MemoryAllocPage();
    childPCB->pagetable[page_number] = MemorySetupPte(new_page);
    bcopy((char *)physical_addr, (char *)(childPCB->pagetable[page_number] &~0x7), MEM_PAGESIZE); //copy old page to new page, mask off the last 3 bits of the newly created page table entry
    referenceCounter[new_page] = 1; //counter for new page set to 1
    referenceCounter[physical_page]--; //decrement counter for old page
  }
  else {
    childPCB->pagetable[page_number] &= ~MEM_PTE_READONLY; //mark as read/write by clearing the readonly bit
  }
  printf("Valid entries for parent\n");
  for (i = 0; i < MEM_PT_SIZE; i++) {
    if ((parent_pagetable[i] & 0x1) == 1) {
      printf("%x\n", parent_pagetable[i]);
    }
  }
  printf("Valid entries for child\n");
  for (i = 0; i < MEM_PT_SIZE; i++) {
    if ((childPCB->pagetable[i] & 0x1) == 1) {
      printf("%x\n", childPCB->pagetable[i]);
    }
  }
}
