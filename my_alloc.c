#include "my_malloc.h"

int is_init = 0;
static char *MEMORY;
static char *SWAP;
static Page** Pages;
static Page** DiskPages;
char* UserMem=NULL;
char* UserDisk=NULL;

//Initializes a page
void initializePage(int Pid){
	Page* page=Pages[pid];
	
	page->tid=current_thread->tid;
	page->pid=current_page++;

	
	if(current_thread->tid != 1){
		current_thread->pageNum=current_page;
	}
	else{
		mainPnum = current_thread_page;
	}
	/*
	if(page->pid==0){
		Segment* 
	}
	*/
}
//frees memory
int freeMem(Segment *block, Page *page, int sysReq)
{
	Segment *previous = NULL; //Both previous and current will be used to decide if the freed memory can be
	Segment *current = NULL;  //merged with other freed memory
	//This switch is used to determine where in memory the function should start looking to find
	//the memory that needs to be freed
	switch (sysReq)
	{
	case LIB_REQ:
		//If the library called it then start at the beginning if the physical memory
		current = Segment * MEMORY;
		break;
	case THREAD_REQ:
		//Looks for page that deallocate wants
		for (int i = 0; i < ((PHYS_MEM_SIZE / SYS_PAGE_SIZE) - MAX_NUM_PAGES); i++)
		{
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
	memcpy(temp , UserDisk+(dest*SYS_PAGE_SIZE) , SYS_PAGE_SIZE);
	memcpy(UserDisk+(dest*SYS_PAGE_SIZE) , UserMem+(source*SYS_PAGE_SIZE) , SYS_PAGE_SIZE);
	memcpy(UserMem+(source*SYS_PAGE_SIZE) , temp , SYS_PAGE_SIZE);
	
	Page* tempP;//switch them within their array
	tempP=Pages[dest];
	DiskPages[dest]=Pages[source];
	Pages[source]=tempP;
	
	//unprotect the memory since it is done being moved
	mprotect(UserMem+(dest*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_NONE);
	mprotect(UserDisk+(source*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_NONE);//Protect both of the memory sections being swapped
}
int toFreeMem(int target){
	for(int i=0; i<MAX_NUM_PAGES ; i++){
			if( Pages[i]->isFree ){
				if( i==target ){
					return 1;
				}
				mprotect(UserMem+(target*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_READ | PROT_WRITE);
				
				memcpy(Pages[i] , Pages[target] , sizeof(Page);
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
	for(int i=0; i<MAX_NUM_PAGES ; i++){
		if( DiskPages[i] != NULL ){
			if( DiskPages[i]->isFree ){
				if( i==target ){
					return 1;
				}
				mprotect(UserMem+(target*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_READ | PROT_WRITE);
				
				memcpy(DiskPages[i] , Pages[target] , sizeof(Page);
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
static void sigHandler(int sig, siginfo_t *si, void *unused)
{
	int swapped=0;
	char *addr = si->si_addr;//The address of the memory trying to be accessed
	if((addr < MEMORY + PHYS_MEM_SIZE || addr > MEMORY) && (addr > SWAP + SWAP_FILE_SIZE || addr < SWAP)){//checks to see if this is outside the physical and swap memory
		printf("SISGEV:SEGMENT FAULT Outside of Memory\n");
		exit(1);
	}
	if((addr > MEMORY + (MAX_NUM_PAGES*SYS_PAGE_SIZE) ||addr < MEMORY) && (addr > SWAP + (MAX_NUM_PAGES*SYS_PAGE_SIZE) || addr < SWAP)){//checks to see if it does to the user's physical and swap memory
		printf("SISGEV:SEGMENT FAULT Tried to Access Non user space\n");
		exit(1);
	}
	unsigned long long int loc=(char*) addr-(MAX_NUM_PAGES*SYS_PAGE_SIZE);
	int tarPage=(int) loc/SYS_PAGE_SIZE;//target page number 
	for(int i=0; i<MAX_NUM_PAGES ;i++){
		if(Pages[i]->tid == current_thread->tid && Pages[i]->pid==tarPage){
			if(i!=tarPage){
				swapPage(tarPage,i);
			}
			mprotect();
			swapped=1;
			break;
		}
	}
	if(swapped==1){
		return;
	}
	//Swaps the needed page from the memory to the disk
	for(int i=0; i< ; i++){
		if(Pages[i]->tid == current_thread->tid && Pages[i]->pid==tarPage){
			diskToMem();
			mprotect((MAX_NUM_PAGES*SYS_PAGE_SIZE)+(i*SYS_PAGE_SIZE),SYS_PAGE_SIZE,PROT_READ | PROT_WRITE);
			swapped=1;
			break;
		}
	}
	
	if(swapped==0){
		printf("Page was not swapped in to memory\n");
		exit(1)
	}
}
//allocates memory for variable of size 'size' from file 'file' on line 'lineCaller'. 'sysReq' tells whether a library thread or disk made call
void *myallocate(size_t size, const char *file, int lineCaller, int sysReq)
{
	//keeps track of current location when traversing memory within this function
	Segment *current;
	//is_inits array for vm
	if (is_init = 0)
	{
		//page aligns memory and makes the arrays for main memory and swap files
		posix_memalign(void **(&MEMORY), SYS_PAGE_SIZE, PHYS_MEM_SIZE);
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
		Pages = (Page **)myallocate(sizeof(Page *) * (MAX_NUM_PAGES), __FILE__, __LINE__, LIB_REQ);
		for (int i = 0; i <  MAX_NUM_PAGES ; i++)
		{
			Pages[i] = Page * myallocate(sizeof(Page), __FILE__, __LINE__, LIB_REQ);
			Pages[i]->isFree = 1;
		}
		UserMem= &MEMORY[MAX_NUM_PAGES*SYS_PAGE_SIZE];
		UserDisk= &SWAP[MAX_NUM_PAGES*SYS_PAGE_SIZE];
		//Makes a page array inside the virtual memory
		DiskPages = (Page **)myallocate(sizeof(Page *) * (MAX_NUM_PAGES), __FILE__, __LINE__, DISK_REQ);
		{
			DiskPages[i] = Page * myallocate(sizeof(Page), __FILE__, __LINE__, DISK_REQ);
			DiskPages[i]->isFree = 1;
		}
		//makes is_init happen once
		is_init = 1;
	}
	switch (sysReq)
	{
	case LIB_REQ: //allocates space the same way you do in the disk
		current = MEMORY;
		while (current != NULL)
		{
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
		printf("There is no memory left in the SWAP SPACE\nError at line %d of file %s\n", line, file);
		return NULL;
		break;
	case THREAD_REQ: //TODO: this case is not finished
		int current_tid=current_thread->tid;//currently running thread
		current = (Segment*) UserMem;//starting point for memory location
		Segment* prev=NULL;
		//searches for page in swap file and puts it into the physical memory
		for(int i=0; i<MAX_NUM_PAGES ; i++){
			if(DiskPages[i]->tid == thread && i != DiskPages[i]->pid){
				diskToMem(i,DiskPages[i]->pid);
			}
		}
		////////////////////////////////////////
		//I'm not sure what this section of code is for
		Page* page = Pages[0];
		if(page->tid != thread){
			if(toFreeMem(0) != 1){
				printf("Could not allocate space\n");
				return NULL;
			}
			initializePage(0);
		}
		////////////////////////////////////////
		
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
		
		if(SEG_SIZE + size >((int)((char*) Memory + PHYS_MEM_SIZE + SEG_SIZE))){
			printf("Error at line %d of file %s\n Not enough memory\n");
			return NULL;
		}
		
		//checks how many pages are free and how many we need
		int extraBlock = (prev->test == 0) ? SEG_SIZE : 0;
		int pagesRequired = ((SEG_SIZE + size+ extraBlock)/SYS_PAGE_SIZE) + 1;
		int amountFree=0;
		
		for(int i=0; i<MAX_NUM_PAGES ; i++){
			if(Pages[i]->isFree){
				amountFree++;
			}
		}
		for(int i=0; i<MAX_NUM_PAGES ; i++){
			if(DiskPages[i]->isFree){
				amountFree++;
			}
		}
		pagesRequired = (amountFree >= pagesRequired) ? pagesRequired : 0;
		
		if( pagesRequired > 0 ){
			
		}
		
		printf("There is no memory left in the SWAP SPACE\nError at line %d of file %s\n", lineCaller, file);
		return NULL;
		break;
	case DISK_REQ:
		current = SWAP;
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
void deallocate(void *ptr, const char *file, int lineCaller, int sysReq)
{
	//Checks to see if pointer is out of bounds
	if ((char *)ptr < &Memory[0] || (char *)ptr < &Memory[PHYS_MEM_SIZE])
	{
		printf("Free Error at line %d of file %s\n", line, file);
		return;
	}
	//Gets segment pointer to the block that is to be freed
	Segment *sec = ((char *)ptr - SEG_SIZE); //SEG_SIZE is removed since the pointer points to the variable and not the segment data
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
		for (int i = 0; i < ((PHYS_MEM_SIZE / SYS_PAGE_SIZE) - MAX_NUM_PAGES); i++)
		{
			if (Pages[i]->tid == current_thread->tid)
			{
				Page *page = Pages[i];
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

void *shalloc(size_t size)
{
}