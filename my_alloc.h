#ifndef MY_ALLOCATE_H
#define MY_ALLOCATE_H

#define PHYS_MEM_SIZE 8388608 //8 MB
#define SWAP_FILE_SIZE 16777216   //16 MB
#define SYS_PAGE_SIZE sysconf(_SC_PAGE_SIZE)

#define malloc(x) myallocate(x, __FILE__, __LINE__, THREAD_REQ)
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREAD_REQ)
#define LIB_REQ 0
#define THREAD_REQ 1
#define DISK_REQ 2
#define SEG_SIZE sizeof(Segment_t)
#define MAX_NUM_PAGES 50 //sets a max for number of pages (This prevents pages from taking up all the memory)

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
int freeMem(Segment *block, Page *page, int sysReq);
void *shalloc(size_t size);
void *myallocate(size_t size, const char *file, int lineCaller, int sysReq);
void deallocate(void *ptr, const char *file, int lineCaller, int sysReq);
#endif