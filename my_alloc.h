#ifndef MY_ALLOC_H
#define MY_ALLOC_H


#include "my_pthread_t.h"



#include <malloc.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <ucontext.h>
#include <string.h>
#include <errno.h>


#define PHYS_MEM_SIZE 8388608 //8 MB
#define SWAP_FILE_SIZE 16777216   //16 MB
#define SYS_PAGE_SIZE sysconf(_SC_PAGE_SIZE)



#define LIB_REQ 0
#define THREAD_REQ 1
#define DISK_REQ 2
#define SEG_SIZE sizeof(Segment)
#define LIB_PAGES 200
#define MAX_NUM_PAGES (PHYS_MEM_SIZE/SYS_PAGE_SIZE) -  LIB_PAGES//sets a max for number of pages (This prevents pages from taking up all the memory)
#define MAX_SWAP (SWAP_FILE_SIZE/SYS_PAGE_SIZE) - 100



typedef struct Page_t
{
    int tid; //thread id
    int pid; //page id
    int isFree;
} Page;

typedef struct Segment_t
{
    int test;
    unsigned int segSpace;
    struct Segment_t *next;
} Segment;

void *myallocate(size_t size, const char *file, int lineCaller, int sysReq);
void mydeallocate(void *ptr, const char *file, int lineCaller, int sysReq);
void initializePage(int pid);
int toFreeMem(int target);//gets passed a page id
int toDisk(int target);//gets passed a page id
int freeMem(Segment *block, Page *page, int sysReq);
void swapPage(int source, int dest);//gets passed page ids
void diskToMem(int source,int dest);//gets passed page ids
void *shalloc(size_t size);

#endif
