#ifndef MY_ALLOCATE_H
#define MY_ALLOCATE_H

#define PHYSICAL_MEM_SIZE 8388608 //8 MB
#define SWAP_FILE_SIZE 16777216 //16 MB
#define SYS_PAGE_SIZE sysconf(_SC_PAGE_SIZE)

#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ)
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ)

#endif