// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: nandan Thakkar
// username of iLab: nt284
// iLab Server:

#include "my_pthread_t.h"
#include <errno.h>



void scheduler(int sig){
    
}


int my_pthread_init(){
    sigemptyset(&sigProcMask);
    sigaddset(&sigProcMask, SIGPROF);
}
//add tcb to queue
void enqueue(struct tcb my_tcb){
	struct Node* end= struct Node malloc(sizeof(Node));
	back0.next=end;
	end.head=my_tcb;
	end.next=NULL;
	back0=end;
}
//search for specified thread
int search(my_pthread_t thread,struct tcb* tcb_ptr){
	//return 0 means success
	for(int i=0; i<4 ;i++){
		
	for(struct Node*ptr=&Queue[i]; ptr->next=NULL ;ptr=ptr->next){
		if(thread==ptr->head->tid){
			return 0;
		}
	}
}
	for(struct Node*ptr=&RunningQueue; ptr->next=NULL ;ptr=ptr->next){
		if(thread==ptr->head->tid){
			return 0;
		}
	}
	for(struct Node*ptr=&WaitingQueue; ptr->next=NULL ;ptr=ptr->next){
		if(thread==ptr->head->tid){
			return 0; 
		}
	}
	for(struct Node*ptr=&CompletedQueue; ptr->next=NULL ;ptr=ptr->next){
		if(thread==ptr->head->tid){
			return 0;
		}
	}
	return 1;//error tcb not found
}
/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
   
	tcb *my_tcb=(tcb *) malloc(sizeof(tcb));
	//Thread status is decided by scheduler
	my_tcb.tid=thread;
	my_tcb.thread_context.uc_link=;//initializes ucontext_t
	my_tcb.thread_context.uc_sigmask=;
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
	tcb_num++;
	makecontext(&thread.thread_context,function,arg);
	add(&thread);//adds TCB to priority queue
	
	return 0;
}

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	//gets current context from current tcb
	swapcontext(tcbPtr.thread_context,tcbPtr.thread_context.);
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
    //get current thread
    while(__sync_lock_test_and_set(&mutex->lock, 1) != 0){ //shared mutex was locked
        //spin_lock(lock);
        if(mutex->lock == 1){
            //put current thread in waitQueue
            //spin_unlock(lock);
            //put thread to sleep
            my_pthread_yield();
            return 1; //thread is in waiting queue and blocked by 
        }else{
            //spin_unlock(lock);
        }
    }
    return 0; //got the lock
}

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
    //load next thread in the queue
    __sync_lock_release(&mutex);
    //if next!=null then wake up the next thread
    return 0;
}

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	if(!mutex)return EINVAL;
    mutex->lock = -1;
    mutex->flags = 0;
    return 0;
}

