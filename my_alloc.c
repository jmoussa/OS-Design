#include "my_malloc.h"

int initialize = 1;
Segment_t
static char *MEMORY;
static char *SWAP;
//frees memory
int freeMem(Segment* block, Page* page, int sysReq){
	Segment* previous=NULL;//Both previous and current will be used to decide if the freed memory can be 
	Segment* current=NULL;//merged with other freed memory
	//This switch is used to determine where in memory the function should start looking to find 
	//the memory that needs to be freed
	switch(sysReq){
		case LIBRARYREQ:
			//If the library called it then start at the beginning if the physical memory
			current=Segment* MEMORY;
			break;
		case THREADREQ:
			//Looks for page that deallocate wants
			for(int i=0; i<((PHYSICAL_MEM_SIZE/SYS_PAGE_SIZE)-PAGEMAX) ; i++){
				if(Pages[i]->tid==page->tid){
					current=(Segment*) (SYS_PAGE_SIZE*i);
					break;
				}
			}
			break;//page could not be found
	}
	if(current==NULL){
		return 0;//could not find memory meant to be freed
	}
	while(current!=NULL){
		if(current==block){
			current->test=1;//frees memory
			Segment* after=current->next;//looks at next segment
			if(after != NULL && after->test){//Merges this segment with one before it if it is also free
				current->segSpace += after->segSpace +SEGSIZE;
				current->next = after->next;
			}
			if(previous != NULL && previous->test){//Merges this segment with one after it if it is also free
				current->segSpace += previous->segSpace +SEGSIZE;
				current->next = previous->next;
			}
			return 1;//successful
		}
		else{
			//increments values
			previous=current;
			current=current->next;
		}
		return 0;
	}
}
//signal handler
static void sigHandler(int sig, siginfo_t *si, void *unused)
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
    	  //page aligns memory and makes the arrays for main memory and swap files
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
        //Makes a page array inside the physical memory
        Pages = (Page**) myallocate(sizeof(Page*) * ((PHYSICAL_MEM_SIZE/SYS_PAGE_SIZE)-PAGEMAX), __FILE__, __LINE__, DISKREQ);
        for(int i=0; i<((PHYSICAL_MEM_SIZE/SYS_PAGE_SIZE)-PAGEMAX) ; i++){
        		Pages[i]=Page* myallocate(sizeof(Page),__FILE__,__LINE__,LIBRARYREQ);
        		Pages[i]->isFree=1;
        }
        //makes initialize happen once
        initialize = 0;
    }
    switch (sysReq)
    {
    case LIBRARYREQ://allocates space the same way you do in the disk
    	  current=MEMORY;
    	  while(current!=NULL){
    	  	 //If the amount of space available is equal to the size of the space needed then you return that
    	  	 if(current->test && current->segSpace==size){
    	  	 	current->test=0; //This segment is no longer available
    	  	 	return ((char*)current) + SEGSIZE;//allocated space starts after the nspace meant to signify beginning of segment data
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
    	  printf("There is no memory left in the SWAP SPACE\nError at line %d of file %s\n",line,file);
    	  return NULL;
        break;
    case THREADREQ://TODO: this case is not finished
    current=MEMORY;
    	  while(current!=NULL){
    	  	 //If the amount of space available is equal to the size of the space needed then you return that
    	  	 if(current->test && current->segSpace==size){
    	  	 	current->test=0; //This segment is no longer available
    	  	 	return ((char*)current) + SEGSIZE;//allocated space starts after the nspace meant to signify beginning of segment data
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
    	  printf("There is no memory left in the SWAP SPACE\nError at line %d of file %s\n",line,file);
    	  return NULL;
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
    	  printf("There is no memory left in the SWAP SPACE\nError at line %d of file %s\n",line,file);
    	  return NULL;
        break;
    }
}
//deallocates memory for variable with the pointer 'ptr' from file 'file' on line 'lineCaller'. 'sysReq' tells whether a library or thread made call
void deallocate(void *ptr, const char *file, int lineCaller, int sysReq){
	//Checks to see if pointer is out of bounds
	if( (char*) ptr < &Memory[0] || (char*) ptr < &Memory[PHYSICAL_MEM_SIZE] ){
		printf("Free Error at line %d of file %s\n",line,file);
		return;
	}
	//Gets segment pointer to the block that is to be freed
	Segment* sec = ((char*) ptr -SEGSIZE);//SEGSIZE is removed since the pointer points to the variable and not the segment data
	//Makes sure sections hasn't been freed already
	if( sec->test ){
		printf("Error at line %d in file %s. Block has already been freed.\n",lineCaller,file);
	}
	//how memory is freed depends on the sysReq that called it
	switch(sysReq){
		case LIBRARYREQ:
			if( !freeMem(sec,NULL,LIBRARYREQ) ){//frees mem and checks to see it works
				printf("Error at lin %d in file %s\n",lineCaller,file);
			}
			break;
		case THREADREQ:
			//finds page to be deallocated
			for(int i=0; i<((PHYSICAL_MEM_SIZE/SYS_PAGE_SIZE)-PAGEMAX) ; i++){
				if(Pages[i]->tid == current_thread->tid){
					Page* page=Pages[i];
				}
			}
			//checks to see if page is null
			if(page=NULL){
				mydeallocate(ptr,__FILE__,__LINE__,LIBRARYREQ);//deallocates pointer with library as the caller then
				return;//this means the ptr points to not a page 
			}
			if( !freeMem(sec,page,THREADREQ) ){//frees up memory and checks to make sure it worked at the same time
				printf("Error at lin %d in file %s\n",lineCaller,file);
			}
			break;
		default://in case diskreq gets passed
			printf("Error at lin %d in file %s\n",lineCaller,file);
			break;
	}
}


void* shalloc(size_t size){
	
}