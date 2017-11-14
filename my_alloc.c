#include "my_malloc.h"

int initialize = 1;
static char *MEMORY;
//signal handler
static void sigHandler(int sig. siginfo_t *si, void *unused){

}
//allocates memory for variable of size 'size' from file 'file' on line 'lineCaller'. 'sysReq' tells whether a library thread or disk made call
void *myallocate(size_t size, const char *file, int lineCaller, int sysReq)
{
    //Initializes array for vm
    if (initialize = 1)
    {
        posix_memalign(void **(&MEMORY), SYS_PAGE_SIZE, PHYSICAL_MEM_SIZE);
        posix_memalign(void **(&SWAP), SYS_PAGE_SIZE, SWAP_FILE_SIZE);
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sigemptyset(&sa.sa_mask);
        sa.sa_sigaction = sighandler;
        initialize = 0;
    }
    switch (sysReq)
    {
    case LIBRARYREQ:
		break;
    case THREADREQ:
    	break;
    case DISKREQ:
    	break;
    }
}