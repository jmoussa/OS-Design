// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:

#include "my_pthread_t.h"
#include <errno.h>
/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	struct tcb *my_tcb=struct tcb * malloc(sizeof(struct tcb));
	thread=&tcb;
	//Thread status is decided by scheduler
	thread.thread_context.uc_link=;//initializes ucontext_t
	thread.thread_context.uc_sigmask=;
	thread.thread_context.uc_stack.ss_sp=sizeof(struct tcb);
	thread.thread_context.uc_stack.ss_flags=0;
	thread.thread_context.uc_stack.ss_size;
	thread.stack=&thread;//initializes other parameters
	thread.thread_params.run=function;
	thread.thread_params.arg=arg;
	thread.thread_params.joinable= true ; 
	//attaches function to context
	makecontext(&thread.thread_context,function,arg);
	add(&thread);//adds TCB to priority queue
	return 0;
}

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	//makes call to scheduler
	return 0;
}

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	//removes thread from queue
	free(value_ptr);//removes thread from memory
}

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
while(**value_ptr!=PTHREAD_EXITED){
	if(thread.status!=NULL){
		return ESRCH;
		//error means that thread is invalid and could not be found
	}
	else if(!thread.thread_params.joinable){
		return EINVAL;//not joinable
	}
	**value_ptr=thread.status;
	else if(**value_ptr==PTHREAD_WAITING && == my_pthread_t.status==PTHREAD_WAITING ){
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
    while(__sync_lock_test_and_set(&mutex->lock, 1) != 0){ //shared mutex was not 0->locked
        //spin_lock(lock);
        if(mutex->lock == 1){
            //put current thread in waitQueue
            //spin_unlock(lock);
            //put thread to sleep
            return 1; //thread is in waiting queue and blocked by 
        }else{
            //spin_unlock(lock);
        }
        my_pthread_yield();
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

