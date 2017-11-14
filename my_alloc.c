#include "my_malloc.h"

int initialize = 1;
Segment_t
static char *MEMORY;
static char *SWAP;
//signal handler
static void sigHandler(int sig.siginfo_t *si, void *unused)
{
}
//allocates memory for variable of size 'size' from file 'file' on line 'lineCaller'. 'sysReq' tells whether a library thread or disk made call
void *myallocate(size_t size, const char *file, int lineCaller, int sysReq)
{
	 //keeps track of current location when traversing memory within this function
	 Segment* current; 
    //Initializes array for vm
    if (initialize = 1)
    {
        posix_memalign(void **(&MEMORY), SYS_PAGE_SIZE, PHYSICAL_MEM_SIZE);
        posix_memalign(void **(&SWAP), SYS_PAGE_SIZE, SWAP_FILE_SIZE);
        //initializes signal handler in this function
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sigemptyset(&sa.sa_mask);
        sa.sa_sigaction = sighandler;
        if (sigaction(SISGEV, &sa, NULL) == -1)
        {
            printf("Fatal error setting up signal handler\n");
            exit(EXIT_FAILURE); //explode!!
        }
        //makes initialize happen once
        initialize = 0;
    }
    switch (sysReq)
    {
    case LIBRARYREQ:
    	  
    	  return;
        break;
    case THREADREQ:
    	  return;
        break;
    case DISKREQ:
    	  current=SWAP;
    	  while(current!=NULL){
    	  	 //If the amount of space available is equal to the size of the space needed then you return that
    	  	 if(current->test && current->segSpace==size){
    	  	 	current->test=0; //This segment is no longer available
    	  	 	return ((char*)current) + SEGSIZE;//allocated space starts after space meant to signify beginning of segment data
    	  	 }
    	  	 //If the segment is bigger than what is needed. A segment of the required size is allocated while the rest is set aside
    	  	 else if(current->test && current->segSpace>=size+SEGSIZE){
    	  	 	Segment* newSeg=(Segment*) ((char*)current + SEGSIZE + size);//Makes pointer to where free space will be once the new space has been allocated
    	  	 	newSeg->test=1;//This signifies the new area is free to use
    	  	 	newSeg->next=current->next;//links are next to the previous space with the new space
    	  	 	newSeg->segSpace=(current->segSpace) - size - SEGSIZE;//decreases size of free space by size of allocated data
    	  	 	
    	  	 	current->test=0;//allocated space is no longer free
    	  	 	current->segSpace=size;//size of allocated space
    	  	 	current->next=newSeg;//sets pointer to next segment equal to the free space
    	  	 	return ((char*)current) + SEGSIZE;
    	  	 }
    	  	 current=current->next;//increments pointer
    	  }
    	  printf("There is no memory left in the SWAP SPACE\N");
    	  return NULL;
        break;
    }
}

void* shalloc(size_t size){
	
}