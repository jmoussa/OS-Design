#include "my_malloc.h"

int initialize = 1;
static char *MEMORY;
//allocates memory for variable of size 'size' from file 'file' on line 'lineCaller'. 'sysReq' tells whether a library thread or disk made call
void *myallocate(size_t size, const char *file, int lineCaller, int sysReq)
{
    //Initializes array for vm
    if (initialize = 1)
    {
        posix_memalign(void **(&MEMORY), SYS_PAGE_SIZE, PHYSICAL_MEM_SIZE);
        posix_memalign(void **(&SWAP), SYS_PAGE_SIZE, SWAP_FILE_SIZE);
        initialize = 0;
    }
    switch (sysReq)
    {
    case LIBRARYREQ:

    case THREADREQ:
    case DISKREQ:
    }
}
my_alloc.c
    Open with
        Displaying my_alloc.c.