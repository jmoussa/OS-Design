
Conversation opened. 1 unread message.

Skip to content
Using Gmail with screen readers
Search



Gmail
COMPOSE
Labels
Inbox (411)
Starred
Important
Chats
Sent Mail
Drafts (3)
All Mail
Spam (41)
Trash
Notes
Personal
Receipts
Travel
Work
More 
Hangouts

 
 
 
  More 
1 of 3,709  
 
Print all In new window
Fwd: 
Inbox
x 

MATTHEW MANN <mam1010@scarletmail.rutgers.edu>
Attachments6:54 PM (0 minutes ago)

to me 
---------- Forwarded message ----------
From: MATTHEW MANN <mam1010@scarletmail.rutgers.edu>
Date: Mon, Nov 13, 2017 at 6:53 PM
Subject: 
To: tilveail@gmail.com
2 Attachments 
 
	
Click here to Reply or Forward
1.21 GB (8%) of 15 GB used
Manage
Terms - Privacy
Last account activity: 50 minutes ago
Details


#include "my_malloc.h"

int initialize=1;
static char * MEMORY;
//allocates memory for variable of size 'size' from file 'file' on line 'lineCaller'. 'sysReq' tells whether a library thread or disk made call
void *myallocate(size_t size, const char* file, int lineCaller,int sysReq){
	//Initializes array for vm
	if(initialize=1){
		posix_memalign(void **(&MEMORY), SYS_PAGE_SIZE, PHYSICAL_MEM_SIZE);
		posix_memalign(void **(&SWAP), SYS_PAGE_SIZE, SWAP_FILE_SIZE);
		initialize=0;
	}
	switch(sysReq){
		case LIBRARYREQ:
			
		case THREADREQ:
		case DISKREQ:
	}
}
my_alloc.c
Open with
Displaying my_alloc.c.