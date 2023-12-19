#ifndef	_memory_h_
#define	_memory_h_

// Put all your #define's in memory_constants.h
#include "memory_constants.h"


extern int lastosaddress; // Defined in an assembly file

typedef struct {
  uint32 addr;
  uint32 inuse;
  int chunkSize;
}AllocTree;
//--------------------------------------------------------
// Existing function prototypes:
//--------------------------------------------------------

int MemoryGetSize();
void MemoryModuleInit();
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr);
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir);
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryPageFaultHandler(PCB *pcb);

//---------------------------------------------------------
// Put your function prototypes here
//---------------------------------------------------------
uint32 MemorySetupPte(uint32 page);
int MemoryAllocPage(void);
uint32 MemoryGetPageNumber(uint32 addr);
void MemoryFreePage(uint32 page);
// All function prototypes including the malloc and mfree functions go here

void *malloc(int memsize);
int mallocHelper(int currChunkSize, int requestedSize, int addr, int currOrder, int arrIndex); //child = 0 if left child, child = 1 if right child
int mfree(void *ptr);
int freeHelper(int ptr, int addr, int index, int chunkSize, int order);


#endif	// _memory_h_
