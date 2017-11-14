#ifndef MY_ALLOCATE_H
#define MY_ALLOCATE_H

#define PHYSICAL_MEM_SIZE 8388608 //8 MB
#define SWAP_FILE_SIZE 16777216   //16 MB
#define SYS_PAGE_SIZE sysconf(_SC_PAGE_SIZE)

#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ)
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ)
#define LIBRARYREQ 0
#define THREADREQ 1
#define DISKREQ 2

typedef struct Page_t
{
    int isFree;
} Page;

typedef struct Segment_t
{
    int test;
    struct Segment_t *next;
} Segment;

#endif