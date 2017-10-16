// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: nandan Thakkar
// username of iLab: nt284
// iLab Server:

#include "my_pthread_t.h"
#include <errno.h>

my_pthread_mutex_t *LOCK;

void spin_aquire(my_pthread_mutex_t *mutex){
    my_pthread_mutex_init(mutex, NULL);
    while(1){
        while(lock==1);
        if(__sync_lock_test_and_set(&mutex->lock, 1)==0){
            break;
        }
    }
}

void spin_release(my_pthread_mutex_t *mutex){
    if(!mutex)return;
    mutex->lock = 0;
}


void scheduler(int sig){
    
}


int my_pthread_init(){
    sigemptyset(&sigProcMask);
    sigaddset(&sigProcMask, SIGPROF);
}
//add tcb to queue
bool enqueue(struct tcb* my_tcb,int a){
	struct Node end= malloc(sizeof(Node));
	end->head=my_tcb;
	end->next=NULL;
	if(*front[a]==NULL){
		*front[a]=end;
	}
	else{
		*back[a]->next=end;
	}
	*back[a]=end;
	my_tcb.thread_params.queue=a;
	return true;
}

//removes tcb to from queue
bool dequeue(int a){
	struct Node temp=*front[a];
	*front[a]=*front[a].next;
	if(*front[a]==NULL)}{
		*end[a]=NULL;
	}
	free(temp);
	return true;
}

//looks at next value in queue
struct tcb* peek(){
	return *front[0];
}
/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
   
	struct tcb *my_tcb=(struct tcb *) malloc(sizeof(tcb));
	//Thread status is decided by scheduler
	my_tcb.tid=thread;
	my_tcb.thread_context.uc_link=;//initializes ucontext_t
	my_tcb.thread_context.uc_sigmask=0;
	my_tcb.thread_context.uc_stack.ss_sp=sizeof(thread_info);
	my_tcb.thread_context.uc_stack.ss_flags=0;
	my_tcb.thread_context.uc_stack.ss_size;
	my_tcb.stack=&thread;//initializes other parameters
	my_tcb.thread_params.run=function;
	my_tcb.thread_params.arg=arg;
	my_tcb.thread_params.joinable= true ; 
	//attaches function to context
	thread=tcb_num;
	my_tcb.tid=thread;
	tcbs[thread]=&my_tcb;
	tcb_num++;
	makecontext(&my_tcb.thread_context,function,arg);
	enqueue(&my_tcb);//adds TCB to priority queue
	
	return 0;
}

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	dequeue((*tcbPtr)->thread_params->queue);//removes from its current queue
	enqueue(*tcbPtr,6);//places current tcb in waiting queue
	*tcbPtr=peek();//changes tcb pointer to new current tcb
	//gets current context from current tcb and swaps
	swapcontext(tcbPtr.thread_context,tcbPtr.thread_context.uclink);
	return 0;
}

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	//removes thread from queue
	free(value_ptr);//removes stack in uncontext
}

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
struct tcb t;
while(**value_ptr!=PTHREAD_EXITED){
	if(search(thread,&t)==1){
		return ESRCH;
	}
	else if(!t.thread_params.joinable){
		return EINVAL;//not joinable
	}
	**value_ptr=t.status;
	else if(**value_ptr==PTHREAD_WAITING && == t.status==PTHREAD_WAITING ){
		return EDEADLK;//1 means error EDEADLK
		//this means two threads joined with eachother or a thread joined with itself
	}
	my_pthread_yield();
}
return 0;
}

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	mutex->lock = 0;
    mutex->flags = 0;
    return 0;
}

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
    while(__sync_lock_test_and_set(mutex->lock, 1) != 0){ //shared mutex was locked
        spin_aquire(LOCK);
        if(mutex->lock == 1){ //value of mutex->lock is 
            my_pthread_yield();
            spin_release(LOCK);
            return 1; //thread is in waiting queue and blocked
        }else{
            spin_release(LOCK);
        }
    }
    return 0; //got the lock
}

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
    spin_aquire(LOCK);
    //TODO: load next thread in the queue
    __sync_lock_release(mutex);
    spin_release(LOCK);
    //if nextThread!=null then wake up the next thread
    return 0;
}

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	if(!mutex)return EINVAL;
    mutex->lock = 0;
    mutex->flags = 0;
    return 0;
}
