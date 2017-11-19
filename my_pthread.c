// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: Nandan Thakkar,Matthew Mann, Joseph Moussa
// username of iLab: nt284, mam1010, jam791
// iLab Server: ls

//#include <pthread.h>
#include "my_pthread_t.h"


#define THREAD_REQ 1
#define malloc(x) myallocate(x, __FILE__, __LINE__, THREAD_REQ)
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREAD_REQ)

#include <errno.h>

int current_page=0;
int mainPnum=0;

int start = 1;

my_pthread_mutex_t LOCK = {0,0,NULL};
int maintainence_cycle_counter = 0;
int tcb_num = 1;


tcb *current_thread, *main_thread;
static int Count_time_executed_for_maintainance_cycle = 0;

void spin_acquire(my_pthread_mutex_t mutex){
	while(1){
		while(mutex.lock==1);
		if(__sync_lock_test_and_set(&mutex.lock, 1)==0){
			break;		
		}	
	}
}

void spin_release(my_pthread_mutex_t mutex){
    mutex.lock = 0;
}

void scheduler(){
    struct itimerval my_timer;
    
    //clear the timer
    my_timer.it_value.tv_sec = 0;
    my_timer.it_value.tv_usec = 0;
    my_timer.it_interval.tv_sec = 0;
    my_timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &my_timer, NULL);
    tcb * temp = current_thread;
    if (current_thread != NULL)
    {
        int current_priority = temp->priority;
        temp->thread_params.execTime =  get_current_time() - temp->thread_params.readyTime;
        temp->executedTime += temp->thread_params.execTime;
        Count_time_executed_for_maintainance_cycle += (current_priority +1)*QUANTUM;
        if (temp->status == WAITING){
            current_thread = peek().front;
            if (current_thread != NULL)
                current_thread->status = RUNNING;
        }
        else if (temp->status == EXITED)
        {
            //pass
            int indicator = 0;
            tcb * ind = my_dequeue(&Queue[current_thread->priority]);
            indicator = my_enqueue(&Queue[5],current_thread);
            if(indicator != 1){perror("Enqueue did not work\n");}
            current_thread = peek().front;
            if (current_thread != NULL){
                current_thread->status = RUNNING;
            }
            
        }
        else if (temp->status == YIELDED)
        {
            //put the thread back into the original queue
            int add = 0;
            add = my_enqueue(&Queue[my_dequeue(&Queue[temp->priority])->priority], temp);
            if (add != 1) {perror("thread control block cannot be enqueued");}
            current_thread = peek().front;
            swapcontext(&temp->thread_context,&current_thread->thread_context);
            if (current_thread != NULL)
                current_thread->status = RUNNING;
        }
        else if (temp->thread_params.execTime >= (current_priority+1)*QUANTUM) //Thread ran through its time quantum slice
        {
            int new_priority;
            if (current_priority + 1 <= LEVELS){
            	new_priority = current_priority + 1;
            	 int add = 0;
            	 my_enqueue(&Queue[new_priority], my_dequeue(&Queue[new_priority-1]) );
            	 current_thread = peek().front;
            	 current_thread->priority=new_priority;
            	 if (current_thread != NULL){
                	 current_thread->status = RUNNING;
         		 }
            }
            else{
            	my_enqueue(&Queue[current_priority], my_dequeue(&Queue[current_priority]) );
            } //Priority cannot exceed anymore
               
          
        }
    }
    else //control is here because current thread is NULL
    {
        current_thread = peek().front;
        if (current_thread != NULL)
        {
            current_thread->status = RUNNING;
        }
    }
    //Update MLFQ on the basis of starvation
    if (Count_time_executed_for_maintainance_cycle > MAINTAINENCE_CYCLE_THRESHOLD * QUANTUM)
	{
        Count_time_executed_for_maintainance_cycle = 0; // did i increment  this var?
        int i;
        for(i = 1; i < LEVELS; i++)
        {
            while(Queue[i].counter > 0){
                tcb * my_tcb = my_dequeue(&Queue[i]);
                my_tcb->priority = 1;
                int was_successful;
                was_successful = my_enqueue(&Queue[0], my_tcb);
                if(was_successful != 1){
                    perror("my_enqueue was unsuccessful");
                }
            }
        }

    }
	 
    my_timer.it_value.tv_sec = 0;
    my_timer.it_value.tv_usec = (current_thread->priority +1)*QUANTUM;
    my_timer.it_interval.tv_sec = 0;
    //set timer
    setitimer(ITIMER_REAL, &my_timer, NULL);
    if (current_thread != NULL) //Context switch happens here
    {
        current_thread->thread_params.readyTime = get_current_time();
        current_thread->thread_params.deadline = current_thread->thread_params.readyTime + my_timer.it_value.tv_usec;
        setitimer(ITIMER_REAL, &my_timer, NULL);
        if (temp != NULL){
            swapcontext(&(temp->thread_context), &(current_thread->thread_context));
         }
        else{
            swapcontext(&main_thread->thread_context,&current_thread->thread_context);
        }
    }
    
    

}

tcb * get_next_thread_to_run(){
    int i = 0;
    tcb * my_next_thread_to_run = NULL;
    for(i =0; i < LEVELS; i ++){
        if(Queue[i].counter > 0){
            my_next_thread_to_run = my_dequeue(&Queue[i]);
            return my_next_thread_to_run;
        }
    }
    perror("no threads left to run");
    return NULL;
}

long int get_current_time(){
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return (tval.tv_sec * 1000000) + tval.tv_usec;
}

void queue_init(struct queue * my_queue){
    my_queue->back = NULL;
    my_queue->front = NULL;
    my_queue->counter = 0;
}

int my_enqueue(struct queue * my_queue, tcb * my_tcb){
    if(my_queue->counter == 0){
    	  
        my_queue->back = my_tcb;
        my_queue->front = my_tcb;
        my_queue->counter++;
        return 1;
    }
    else{
        my_queue->back->nextThread = my_tcb;
        my_queue->back = my_tcb;
        my_tcb->nextThread=NULL;
        my_queue->counter++;
        return 1;
    }
    return 0;
}

tcb * my_dequeue(struct queue * my_queue){
    tcb * my_tcb = my_queue->front;
    if(my_queue->front == NULL){
        return NULL;
    }
    else{
        if(my_queue->front == my_queue->back){
            my_queue->back = NULL;
        }
        my_queue->front = my_queue->front->nextThread;
        my_queue->counter--;
        return my_tcb;
    }
}

//looks at next value in queue
struct queue peek(){
    int i=0;
    while(Queue[i].counter==0){
      i++;
    }
    return Queue[i];
}
void wrapperfunction(void *(*function)(void*),void * arg,void ** retval){
	signal(SIGALRM, scheduler);
	*retval=function(arg);
	printf("in wraper function\n");
	current_thread->status = EXITED;
	free(current_thread->thread_context.uc_stack.ss_sp);
	printf("leaving wraper function\n");
	scheduler();
	return;
}

void my_pthread_init(){
	main_thread = malloc(sizeof(tcb));
	main_thread->tid = 0;
	main_thread->priority = 0;
	my_enqueue(&Queue[0],main_thread);
	getcontext(&main_thread->thread_context);
}

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	signal(SIGALRM, scheduler);
	sigemptyset(&sigProcMask);
	sigaddset(&sigProcMask, SIGALRM);
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    tcb* my_tcb= malloc(sizeof(tcb));
    //Thread status is decided by scheduler
    *thread = tcb_num;
    my_tcb->tid = *thread;
    tcbs[*thread]=my_tcb;
    tcb_num++;
    if(start == 1){
    		start = 0;
    		my_pthread_init();
    }
    getcontext(&my_tcb->thread_context);
    my_tcb->thread_context.uc_link=NULL;//initializes ucontext_t
    my_tcb->priority = 0;
    my_tcb->joinid = -1;
    my_tcb->thread_context.uc_stack.ss_sp = malloc(MEMY);
    my_tcb->thread_context.uc_stack.ss_flags = 0;
    my_tcb->thread_context.uc_stack.ss_size = MEMY;
    //attaches function to context
    makecontext(&my_tcb->thread_context,(void*)&wrapperfunction,3,function,arg,&my_tcb->retval);
    //Thread status is decided by scheduler
    my_tcb->thread_params.run=function;
    my_tcb->thread_params.arg=arg;
    my_tcb->thread_params.joinable = 1;
     my_tcb->thread_params.readyTime = get_current_time();
    printf("Thread created ID=%d\n",my_tcb->tid);
    my_enqueue(&Queue[0],my_tcb);//adds TCB to priority queue
   
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    scheduler();
    return 0;
}

int my_pthread_yield() {
	printf("YILEDED\n");
	current_thread->status = YIELDED;
	return 1;
}

int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	if(tcbs[thread]->tid != thread){
		perror("ESRCH"); return ESRCH;}
	if(tcbs[thread]->joinid != -1){
		perror("EINVAL"); return EINVAL;}
	if(tcbs[thread]->tid == current_thread->joinid && current_thread->joinid != -1){
		perror("EDEADLK"); return EDEADLK;}
	if(tcbs[thread]->status == EXITED){
		//*value_ptr = tcbs[thread]->retval;
		return 1;
	}
	tcbs[thread]->joinid = current_thread->tid;
	my_pthread_yield();
	printf("Joined\n");
	
	scheduler();
	return 1;
}

void my_pthread_exit(void *value_ptr) {
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
	printf("Thread Exited\n");
	value_ptr=current_thread->retval;
	current_thread->status = EXITED;
	if(current_thread->joinid != -1){
		int i = 0;
		i = my_enqueue(&Queue[current_thread->waiting->priority],current_thread->waiting);	
		if(i != 1){perror("TCB was not enqueued");}
	}
	current_thread->status = EXITED;
	free(current_thread->thread_context.uc_stack.ss_sp);//clears stack in thread's context
   sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
   scheduler();
}

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	 printf("Mutex initialized!\n");
    mutex->lock = 0;
    mutex->flags = 0;
    return 0;
}

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	while(__sync_lock_test_and_set(&mutex->lock, 1)==0){ //shared mutex was unlocked
	printf("Locked!\n");
        spin_acquire(LOCK);
        if(mutex->lock == 1){ 
            my_pthread_yield();
            spin_release(LOCK);
        }else{
        		spin_release(LOCK);
        }
        
    }
    return 0; //got the lock
}

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	spin_acquire(LOCK);	
	printf("Unlocked!\n");
   __sync_lock_release(&mutex->lock);
   spin_release(LOCK);
   return 0;
}

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	printf("Mutex Destroyed!\n");
    if(!mutex)return EINVAL;
    mutex->lock = 0;
    mutex->flags = 0;
    return 0;
}