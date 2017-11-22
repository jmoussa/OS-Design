#include "my_alloc.h"

int is_init = 0;
static char *MEMORY;
static char *SWAP;
static Page** Pages;
static Page** DiskPages;
static Segment* firstSeg;
static Segment* firstSwapSeg;
static char* UserMem=NULL;
static char* UserDisk=NULL;

//Initializes a page
void initializePage(int Pid){
	Page* page=Pages[Pid];//gets page from array
	

	page->tid=current_tid;
	page->pid=current_page++;//Sets the page number
	printf("Here12\n");	
	
	if(current_tid != 0){
		if(current_thread!=NULL){
			current_thread->pageNum=current_page;
		}
		else{
			current_page=current_page;
		}
	}
	else{
		mainPnum = current_page;
	}
   page->isFree = 0;
   printf("Here13\n");
	if(page->pid==0){
		firstSeg= (Segment*) UserMem;
      firstSeg->test = 1;
      firstSeg->segSpace = SYS_PAGE_SIZE - SEG_SIZE;
      firstSeg->next = NULL;
	}
}
//frees memory
int freeMem(Segment *block, Page *page, int sysReq){
	Segment *previous = NULL; //Both previous and current will be used to decide if the freed memory can be
	Segment *current = NULL;  //merged with other freed memory
	int i;
	//This switch is used to determine where in memory the function should start looking to find
	//the memory that needs to be freed
	switch (sysReq)
	{
	case LIB_REQ:
		//If the library called it then start at the beginning if the physical memory
		current = (Segment*) MEMORY;
		break;
	case THREAD_REQ:
		//Looks for page that deallocate wants
		for ( i = 0; i < MAX_NUM_PAGES ; i++){
			if (Pages[i]->tid == page->tid)
			{
				current = (Segment *)(SYS_PAGE_SIZE * i);
				break;
			}
		}
		break; //page could not be found
	}
	if (current == NULL)
	{
		return 0; //could not find memory meant to be freed
	}
	while (current != NULL)
	{
		if (current == block)
		{
			current->test = 1;				//frees memory
			Segment *after = current->next; //looks at next segment
			if (after != NULL && after->test)
			{ //Merges this segment with one before it if it is also free
				current->segSpace += after->segSpace + SEG_SIZE;
				current->next = after->next;
			}
			if (previous != NULL && previous->test)
			{ //Merges this segment with one after it if it is also free
				current->segSpace += previous->segSpace + SEG_SIZE;
				current->next = previous->next;
			}
			return 1; //successful
		}
		else
		{
			//increments values
			previous = current;
			current = current->next;
		}
		return 0;
	}
}
//swaps pages
void swapPage(int source, int dest){
	char temp[SYS_PAGE_SIZE];//temporary page for switching the position of two pages
	
	mprotect(UserMem+(dest*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_READ | PROT_WRITE);
	mprotect(UserMem+(source*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_READ | PROT_WRITE);//Protect both of the memory sections being swapped
	
	//change the location of the actual memory
	memcpy(temp , UserMem+(dest*SYS_PAGE_SIZE) , SYS_PAGE_SIZE);
	memcpy(UserMem+(dest*SYS_PAGE_SIZE) , UserMem+(source*SYS_PAGE_SIZE) , SYS_PAGE_SIZE);
	memcpy(UserMem+(source*SYS_PAGE_SIZE) , temp , SYS_PAGE_SIZE);
	
	Page* tempP;//switch them within their array
	tempP=Pages[dest];
	Pages[dest]=Pages[source];
	Pages[source]=tempP;
	
	//unprotect the memory since it is done being moved
	mprotect(UserMem+(dest*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_NONE);
	mprotect(UserMem+(source*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_NONE);//Protect both of the memory sections being swapped
	
}
//swaps pages from disk to physical memory
void diskToMem(int source,int dest){

	char temp[SYS_PAGE_SIZE];//temporary page for switching the position of two pages
	
	mprotect(UserMem+(dest*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_READ | PROT_WRITE);
	mprotect(UserDisk+(source*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_READ | PROT_WRITE);//Protect both of the memory sections being swapped

	//change the location of the actual memory
	memcpy(temp , UserDisk+(source*SYS_PAGE_SIZE) , SYS_PAGE_SIZE);
	memcpy(UserDisk+(source*SYS_PAGE_SIZE) , UserMem+(dest*SYS_PAGE_SIZE) , SYS_PAGE_SIZE);
	memcpy(UserMem+(dest*SYS_PAGE_SIZE) , temp , SYS_PAGE_SIZE);
	Page* tempP;//switch them within their array
	tempP=Pages[dest];
	DiskPages[dest]=Pages[source];
	Pages[source]=tempP;

	//unprotect the memory since it is done being moved
	mprotect(UserMem+(dest*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_NONE);
	mprotect(UserDisk+(source*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_NONE);//Protect both of the memory sections being swapped
}
//
int toFreeMem(int target){
	int i;
	for(i=0; i<MAX_NUM_PAGES ; i++){
			if( Pages[i]->isFree ){
				if( i==target ){
					return 1;
				}
				mprotect(UserMem+(target*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_READ | PROT_WRITE);
				
				memcpy(Pages[i] , Pages[target] , sizeof(Page) );
				memcpy(UserMem+(i*SYS_PAGE_SIZE) , UserMem+(target*SYS_PAGE_SIZE) , SYS_PAGE_SIZE);
				Pages[target]->isFree = 1;
				mprotect(UserMem+(target*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_NONE);
				
				return 1;
				
			}
	}

	return 0;
}
//places memory in the disk
int toDisk(int target){
	int i;
	for(i=0; i<MAX_SWAP ; i++){
		if( DiskPages[i] != NULL ){
			if( DiskPages[i]->isFree ){
				if( i==target ){
					return 1;
				}
				mprotect(UserMem+(target*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_READ | PROT_WRITE);
				
				memcpy(DiskPages[i] , Pages[target] , sizeof(Page) );
				memcpy(UserDisk+(i*SYS_PAGE_SIZE) , UserMem+(target*SYS_PAGE_SIZE) , SYS_PAGE_SIZE);
				Pages[target]->isFree = 1;
				
				mprotect(UserMem+(target*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_NONE);
				
				return 1;
			}
		}
	}
	return 0;
}
//signal handler
static void sigHandler(int sig, siginfo_t *si, void *unused){
	printf("Handler Called\n");
	
	int swapped=0;
	char *addr = si->si_addr;//The address of the memory trying to be accessed
	if((addr > MEMORY + PHYS_MEM_SIZE || addr < MEMORY) && (addr < UserDisk || addr > SWAP+SWAP_FILE_SIZE)){//checks to see if this is outside the physical and swap memory
		printf("SISGEV:SEGMENT FAULT Outside of Memory\n");
		exit(1);
	}
	else if((addr < UserMem && addr >= MEMORY) && (addr < UserDisk && addr >= SWAP)){//checks to see if it does to the user's physical and swap memory
		printf("SISGEV:SEGMENT FAULT Tried to Access Non user space\n");
		exit(1);
	}
	unsigned long long int loc=(char*)si->si_addr - UserMem;
	int tarPage=(int) loc/SYS_PAGE_SIZE;//target page number
	int i;
	for(i=0; i<MAX_NUM_PAGES ;i++){
		if(Pages[i]->tid == current_tid && Pages[i]->pid==tarPage){
			if(i!=tarPage){
				swapPage(tarPage,i);
			}
			mprotect(UserMem+(tarPage*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_READ | PROT_WRITE);
			swapped=1;
			break;
		}
	}
	if(swapped==1){
		return;
	}
	//Swaps the needed page from the memory to the disk
	for(i=0; i<MAX_SWAP ; i++){
		if(DiskPages[i]->tid == current_tid && Pages[i]->pid==tarPage){
			diskToMem(tarPage,i);
			mprotect(UserMem +(i*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_READ | PROT_WRITE);
			swapped=1;
			break;
		}
	}
	
	if(swapped==0){
		printf("Page was not swapped in to memory\n");
		exit(1);
	}
}
//allocates memory for variable of size 'size' from file 'file' on line 'lineCaller'. 'sysReq' tells whether a library thread or disk made call
void *myallocate(size_t size, const char *file, int lineCaller, int sysReq){
	printf("myallocate called\n");
	//keeps track of current location when traversing memory within this function
	Segment *current;
	int i;
	//is_inits array for vm
	if (is_init == 0)
	{
		//makes is_init happen once
		is_init = 1;
		int i;
		printf("Memory Initialized\n");
		//page aligns memory and makes the arrays for main memory and swap files
		posix_memalign((void **)&MEMORY, SYS_PAGE_SIZE, PHYS_MEM_SIZE);
		posix_memalign((void **)&SWAP, SYS_PAGE_SIZE, SWAP_FILE_SIZE);
		
		UserMem= &MEMORY[LIB_PAGES*SYS_PAGE_SIZE];
		UserDisk= &SWAP[100 *SYS_PAGE_SIZE];
		
		//initializes the segment within the swap memory
		firstSwapSeg=(Segment*)SWAP;
		firstSwapSeg->test=1;
		firstSwapSeg->segSpace=(4096*SYS_PAGE_SIZE)-SEG_SIZE;
		firstSwapSeg->next=NULL;
		
		//Makes a page array inside the virtual memory
		DiskPages = (Page **)myallocate(sizeof(Page *) * (MAX_SWAP), __FILE__, __LINE__, DISK_REQ);
		for (i= 0; i <  MAX_SWAP ; i++){
			DiskPages[i] = (Page*) myallocate(sizeof(Page), __FILE__, __LINE__, DISK_REQ);
			DiskPages[i]->isFree = 1;
			DiskPages[i]->tid=-1;
		}
		//initializes signal handler in this function
		struct sigaction sa;
		sa.sa_flags = SA_SIGINFO;
		sigemptyset(&sa.sa_mask);
		sa.sa_sigaction = sigHandler;
		if (sigaction(SIGSEGV, &sa, NULL) == -1)
		{
			printf("Fatal error setting up signal handler\n");
			exit(EXIT_FAILURE); //explode!!
		}

		//initializes the segment within the physical memory
		firstSeg=(Segment*)MEMORY;
		firstSeg->test=1;
		firstSeg->segSpace=(LIB_PAGES*SYS_PAGE_SIZE)-SEG_SIZE;
		firstSeg->next=NULL;
		//Makes a page array inside the physical memory
		Pages = (Page **)myallocate(sizeof(Page *) * ((SWAP_FILE_SIZE/SYS_PAGE_SIZE)-100), __FILE__, __LINE__, LIB_REQ);
		for (i= 0; i <  MAX_NUM_PAGES ; i++){
			Pages[i] = (Page*) myallocate(sizeof(Page), __FILE__, __LINE__, LIB_REQ);
			Pages[i]->isFree = 1;
			Pages[i]->tid=-1;
		}
		
		
		printf("Memory Initialization Finished\n");
	}
	switch (sysReq)
	{
	case LIB_REQ: //allocates space the same way you do in the disk
		current = firstSeg;
		while (current != NULL)
		{
			//If the amount of space available is equal to the size of the space needed then you return that
			if (current->test && current->segSpace == size)
			{
				current->test = 0;					 //This segment is no longer available
				return ((char *)current) + SEG_SIZE; //allocated space starts after the nspace meant to signify beginning of segment data
			}
			//If the segment is bigger than what is needed. A segment of the required size is allocated while the rest is set aside
			else if (current-> test && current->segSpace >= size + SEG_SIZE)
			{
				Segment *newSeg = (Segment *)((char *)current + SEG_SIZE + size); //Makes pointer to where free space will be once the new space has been allocated
				newSeg->test = 1;												  //This signifies the new area is free to use
				newSeg->next = current->next;									  //links are next to the previous space with the new space
				newSeg->segSpace = (current->segSpace) - size - SEG_SIZE;		  //decreases size of free space by size of allocated data

				current->test = 0;		  //allocated space is no longer free
				current->segSpace = size; //size of allocated space
				current->next = newSeg;   //sets pointer to next segment equal to the free space
				return ((char *)current) + SEG_SIZE;
			}
			current = current->next; //increments pointer
		}
		printf("There is no memory left in the SWAP SPACE. Error at line %d of file %s\n", lineCaller, file);
		return NULL;
		break;
	case THREAD_REQ:
		if(current_thread!=NULL){
			printf("Why though\n");
			current_tid=current_thread->tid;
		}
		else{
			current_tid=0;
		}
		//Search for page and swap into physical memory
		for (i= 0; i < MAX_NUM_PAGES; i++) {
			if(Pages[i]->tid == current_tid && i != Pages[i]->pid){
				swapPage(i, Pages[i]->pid);
			}
		}
		for(i=0; i<MAX_SWAP ; i++){
			if(DiskPages[i] != NULL){
				if(DiskPages[i]->tid == current_tid && i != DiskPages[i]->pid){
					diskToMem(i,DiskPages[i]->pid);
				}
			}	
		}
		Page* page = Pages[0];
		if(page->tid != current_tid){
			if(toFreeMem(0) != 1){
				if(toDisk(0) != 1){
					printf("Could not allocate space\n");
					return NULL;
				}
			}
			initializePage(0);
		}
      current = firstSeg;//starting point for memory location
		Segment* prev=NULL;

		while (current != NULL)
		{
			prev = current;
			//If the amount of space available is equal to the size of the space needed then you return that
			if (current->test && current->segSpace == size){
				current->test = 0;					 //This segment is no longer available
				return ((char *)current) + SEG_SIZE; //allocated space starts after the nspace meant to signify beginning of segment data
			}
			//If the segment is bigger than what is needed. A segment of the required size is allocated while the rest is set aside
			else if (current->test && current->segSpace >= (size + SEG_SIZE) ){
				printf("Here2\n");
				Segment *newSeg = (Segment *) ((char *)current + (SEG_SIZE + size) ); //Makes pointer to where free space will be once the new space has been allocated
				newSeg->test = 1;												  //This signifies the new area is free to use
				newSeg->segSpace = current->segSpace - (size + SEG_SIZE);		  //decreases size of free space by size of allocated data
				newSeg->next = current->next;									  //links are next to the previous space with the new space
				
				
				current->test = 0;		  //allocated space is no longer free
				current->segSpace = size; //size of allocated space
				current->next = newSeg;   //sets pointer to next segment equal to the free space
				return ((char *)current) + SEG_SIZE;
			}
			printf("Here3\n");
			current = current->next; //increments pointer
		}
		
		if( (SEG_SIZE + size) >((int)((char*) MEMORY + PHYS_MEM_SIZE + SEG_SIZE -(char*)prev ))){
			printf("Error at line %d of file %s\n Not enough memory\n",lineCaller,file);
			return NULL;
		}
		printf("Here4\n");
		//checks how many pages are free and how many we need
		int extraBlock = (prev->test == 0) ? SEG_SIZE : 0;
		int pagesRequired = ((SEG_SIZE + size+ extraBlock)/SYS_PAGE_SIZE) + 1;
		int amountFree=0;
		int i;
		for(i=0; i<MAX_NUM_PAGES ; i++){
			if(Pages[i]->isFree){
				amountFree++;
			}
		}
		for(i=0; i<MAX_SWAP ; i++){
			if(DiskPages[i]->isFree){
				amountFree++;
			}
		}
		pagesRequired = (amountFree >= pagesRequired) ? pagesRequired : 0;
		
		printf("Here5\n");
		if( pagesRequired > 0 ){
			int z, nextPage;
            if(extraBlock > 0){
            	 printf("Here7\n");
                int nextPage = current_page;

                if(toFreeMem(nextPage) != 1){
                	printf("Here8\n");
                    if(toDisk(nextPage) != 1){
                        puts("Disk & Memory FULL. Cannot allocate anymore.\n");
                        return NULL;
                    }
                }
					 if(nextPage>((PHYS_MEM_SIZE/SYS_PAGE_SIZE)-4)){
					 	printf("Here9\n");
					 	printf("This is the shared space\n");
					 	return NULL;
					 }
                initializePage(nextPage);
                Segment* block1 = (Segment*)((char*)UserMem + (nextPage * SYS_PAGE_SIZE));
                block1->test = 1;
                block1->segSpace = SYS_PAGE_SIZE - SEG_SIZE;
                prev->next = block1;
                prev = block1;
                z = pagesRequired - 1;
            }else{
                z = pagesRequired;
            }
            while(z > 0){
            	 printf("Here10\n");
                nextPage = current_tid;
                if(toFreeMem(nextPage)!=1){
                	  printf("Here12\n");
                    if(toDisk(nextPage)!=1){
                        puts("Out of Swap Space and Memory Space");
                        return NULL;
                    }
                }
                initializePage(nextPage);
                prev->segSpace += SYS_PAGE_SIZE;
                
                z--;
            }

            current = prev;

            Segment* newBlk = (Segment*) ((char*) current + (size+SEG_SIZE));
            
            newBlk->test = 1;
            newBlk->segSpace = current->segSpace - (size+SEG_SIZE);
            newBlk->next = current->next;

            current->test = 0;
            current->segSpace = size;
            current->next = newBlk;
            return ((char*) current) + SEG_SIZE;
		}
		printf("Here6\n");
		printf("There is no memory left in the SWAP SPACE\nError at line %d of file %s\n", lineCaller, file);
		return NULL;
		break;
	case DISK_REQ:
		current = firstSwapSeg;
		while (current != NULL)
		{
			//If the amount of space available is equal to the size of the space needed then you return that
			if (current->test && current->segSpace == size)
			{
				current->test = 0;					 //This segment is no longer available
				return ((char *)current) + SEG_SIZE; //allocated space starts after space meant to signify beginning of segment data
			}
			//If the segment is bigger than what is needed. A segment of the required size is allocated while the rest is set aside
			else if (current->test && current->segSpace >= size + SEG_SIZE)
			{
				Segment *newSeg = (Segment *)((char *)current + SEG_SIZE + size); //Makes pointer to where free space will be once the new space has been allocated
				newSeg->test = 1;												  //This signifies the new area is free to use
				newSeg->next = current->next;									  //links are next to the previous space with the new space
				newSeg->segSpace = (current->segSpace) - size - SEG_SIZE;		  //decreases size of free space by size of allocated data

				current->test = 0;		  //allocated space is no longer free
				current->segSpace = size; //size of allocated space
				current->next = newSeg;   //sets pointer to next segment equal to the free space
				return ((char *)current) + SEG_SIZE;
			}
			current = current->next; //increments pointer
		}
		printf("There is no memory left in the SWAP SPACE\nError at line %d of file %s\n", lineCaller, file);
		return NULL;
		break;
	}
}
//deallocates memory for variable with the pointer 'ptr' from file 'file' on line 'lineCaller'. 'sysReq' tells whether a library or thread made call
void mydeallocate(void *ptr, const char *file, int lineCaller, int sysReq){
	int i;
	Page* page;
	//Checks to see if pointer is out of bounds
	if ((char *)ptr < &MEMORY[0] || (char *)ptr < &MEMORY[PHYS_MEM_SIZE])
	{
		printf("Free Error at line %d of file %s\n", lineCaller, file);
		return;
	}
	//Gets segment pointer to the block that is to be freed
	Segment *sec = (Segment*) ((char *)ptr - SEG_SIZE); //SEG_SIZE is removed since the pointer points to the variable and not the segment data
	//Makes sure sections hasn't been freed already
	if (sec->test)
	{
		printf("Error at line %d in file %s. Block has already been freed.\n", lineCaller, file);
	}
	//how memory is freed depends on the sysReq that called it
	switch (sysReq)
	{
	case LIB_REQ:
		if (!freeMem(sec, NULL, LIB_REQ))
		{ //frees mem and checks to see it works
			printf("Error at lin %d in file %s. Unable to free memory for library request.\n", lineCaller, file);
		}
		break;
	case THREAD_REQ:
	
		//finds page to be deallocated
		for (i = 0; i < MAX_NUM_PAGES ; i++)
		{
			if (Pages[i]->tid == current_thread->tid)
			{
				page = Pages[i];
			}
		}
		//checks to see if page is null
		if (page == NULL)
		{
			mydeallocate(ptr, __FILE__, __LINE__, LIB_REQ); //deallocates pointer with library as the caller then
			return;											//this means the ptr points to not a page
		}
		if (!freeMem(sec, page, THREAD_REQ))
		{ //frees up memory and checks to make sure it worked at the same time
			printf("Error at lin %d in file %s. Unable to free memory for thread request\n", lineCaller, file);
		}
		break;
	default: //in case DISK_REQ gets passed
		printf("Error at lin %d in file %s\n", lineCaller, file);
		break;
	}
}

void *shalloc(size_t size){
	Segment *current;
	current_tid=current_thread->tid;//currently running thread
		
		int i;
        

		Page* page = Pages[0];
		if(page->tid != current_tid){
			initializePage(0);
		}
		
      current = (Segment*) UserMem;//starting point for memory location
		Segment* prev=NULL;

		while (current != NULL)
		{
			prev = current;
			//If the amount of space available is equal to the size of the space needed then you return that
			if (current->test && current->segSpace == size)
			{
				current->test = 0;					 //This segment is no longer available
				return ((char *)current) + SEG_SIZE; //allocated space starts after the nspace meant to signify beginning of segment data
			}
			//If the segment is bigger than what is needed. A segment of the required size is allocated while the rest is set aside
			else if (current->test && current->segSpace >= size + SEG_SIZE)
			{
				Segment *newSeg = (Segment *)((char *)current + SEG_SIZE + size); //Makes pointer to where free space will be once the new space has been allocated
				newSeg->test = 1;												  //This signifies the new area is free to use
				newSeg->next = current->next;									  //links are next to the previous space with the new space
				newSeg->segSpace = (current->segSpace) - size - SEG_SIZE;		  //decreases size of free space by size of allocated data

				current->test = 0;		  //allocated space is no longer free
				current->segSpace = size; //size of allocated space
				current->next = newSeg;   //sets pointer to next segment equal to the free space
				return ((char *)current) + SEG_SIZE;
			}
			current = current->next; //increments pointer
		}
		
		if(SEG_SIZE + size >((int)((char*) MEMORY + PHYS_MEM_SIZE + SEG_SIZE - (char*)prev ))){
			printf("Error at line %d of file %s\n Not enough memory\n",lineCaller,file);
			return NULL;
		}
		
		//checks how many pages are free and how many we need
		int extraBlock = (prev->test == 0) ? SEG_SIZE : 0;
		int pagesRequired = ((SEG_SIZE + size+ extraBlock)/SYS_PAGE_SIZE) + 1;
		int amountFree=0;
		
		for( i=0; i<MAX_NUM_PAGES ; i++){
			if(Pages[i]->isFree){
				amountFree++;
			}
		}
		for( i=0; i<MAX_NUM_PAGES ; i++){
			if(DiskPages[i]->isFree){
				amountFree++;
			}
		}
		pagesRequired = (amountFree >= pagesRequired) ? pagesRequired : 0;
		
		if( pagesRequired > 0 ){
			int z, nextPage;
            if(extraBlock > 0){
                int nextPage = current_page;
                initializePage(nextPage);
                Segment* block1 = (Segment*)((char*)UserMem + (nextPage * SYS_PAGE_SIZE));
                block1->test = 1;
                block1->segSpace = SYS_PAGE_SIZE - SEG_SIZE;
                prev->next = block1;
                prev = block1;
                z = pagesRequired - 1;
            }else{
                z = pagesRequired;
            }
            while(z > 0){
                nextPage = current_tid;
                initializePage(nextPage);
                prev->segSpace += SYS_PAGE_SIZE;
                z--;
            }
            current = prev;

            Segment* newBlk = (Segment*) ((char*) current + (size+SEG_SIZE));
            newBlk->test = 1;
            newBlk->segSpace = current->segSpace - (size+SEG_SIZE);
            newBlk->next = current->next;

            current->test = 0;
            current->segSpace = size;
            current->next = newBlk;
            return ((char*) current) + SEG_SIZE;
		}
		
		printf("There is no memory left in the SWAP SPACE\n");
		return NULL;
}
