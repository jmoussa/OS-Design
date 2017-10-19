// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: nandan Thakkar
// username of iLab: nt284
// iLab Server:

#include "my_pthread_t.h"
#include <errno.h>

int start = 1;

const my_pthread_mutex_t *LOCK = {0};
int maintainence_cycle_counter = 0;
int tcb_num = 0;

//tcb * tcbPtr = tcbs[0];
tcb *current_thread, *main_thread;
static int Count_time_executed_for_maintainance_cycle = 0;

void spin_acquire(my_pthread_mutex_t *mutex){
    my_pthread_mutex_init(mutex, NULL);
    while(1){
        while(mutex->lock==1);
        if(__sync_lock_test_and_set(&mutex->lock, 1)==0){
            break;
        }
    }
}

void spin_release(my_pthread_mutex_t *mutex){
    if(!mutex)return;
    mutex->lock = 0;
}

//void init(){
//    int i = 0;
//    for(i = 0; i < 7 ; i++){
//        back[i] = &Queue[i];
//        front[i] = &Queue[i];
//    }
//}


void scheduler(){
    struct itimerval my_timer;
    //ucontext_t uc_temp; //To store the context of the main thread
    
    //clear the timer
    my_timer.it_value.tv_sec = 0;
    my_timer.it_value.tv_usec = 0;
    my_timer.it_interval.tv_sec = 0;
    my_timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &my_timer, NULL);
    
    tcb * temp = current_thread;
    if (temp != NULL)
    {
        int current_priority = temp->priority;
        
        temp->thread_params.execTime = (uint)(get_current_time() - temp->thread_params.readyTime);
        temp->executedTime += temp->thread_params.execTime;
        
        Count_time_executed_for_maintainance_cycle += (current_priority +1)*QUANTUM;
        if (temp->status == WAITING)//int my_pthread_init(){
        	//    sigemptyset(&sigProcMask);
        	//    sigaddset(&sigProcMask, SIGPROF);
        	//    return 0;
        	//}

        {
            //pass
            current_thread = get_next_thread_to_run();
            if (current_thread != NULL)
                current_thread->status = RUNNING;
        }
        else if (temp->status == EXITED)
        {
            //pass
            current_thread = get_next_thread_to_run();
            if (current_thread != NULL)
                current_thread->status = RUNNING;
        }
        else if (temp->status == YIELDED)
        {
            //put the thread back into the original queue
            int add = 0;
            add = my_enqueue(&Queue[temp->priority], temp);
            if (add != 1) {perror("thread control block cannot be enqueued");}
            current_thread = get_next_thread_to_run();
            if (current_thread != NULL)
                current_thread->status = RUNNING;
        }
        else if (temp->thread_params.execTime >= (current_priority+1)*QUANTUM) //Thread ran through its time quantum slice
        {
            int new_priority;
            if (current_priority + 1 < LEVELS)
                new_priority = current_priority + 1;
            else //Priority cannot exceed anymore
                new_priority = current_priority;
            //scheduler_add_thread(temp, new_priority);
            
            int add = 0;
            add = my_enqueue(&Queue[new_priority], temp);
            if (add != 1) {perror("thread control block cannot be enqueued");}
            
            current_thread = get_next_thread_to_run();
            if (current_thread != NULL)
                current_thread->status = RUNNING;
        }
    }
    else //control is here because current thread is NULL
    {
        current_thread = get_next_thread_to_run();
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
    //my_timer.it_value.tv_usec = 50000;
    my_timer.it_value.tv_usec = (current_thread->priority +1)*QUANTUM;
    my_timer.it_interval.tv_sec = 0;
    my_timer.it_interval.tv_usec = 0;
    
    //set timer
    setitimer(ITIMER_REAL, &my_timer, NULL);
    
    //if(temp != NULL){
    //getcontext(&uc_temp);
    //temp->uc = uc_temp;
    //}
    
    if (current_thread != NULL) //Context switch happens here
    {
        current_thread->thread_params.readyTime = get_current_time();
        current_thread->thread_params.deadline = current_thread->thread_params.readyTime + my_timer.it_value.tv_usec;
        setitimer(ITIMER_REAL, &my_timer, NULL);
        if (temp != NULL)
            swapcontext(&(temp->thread_context), &(current_thread->thread_context));
        else
        /*
         stores main context
         (enters else statement only in first call to scheduler)
         */
            swapcontext(&main_thread->thread_context,&current_thread->thread_context);
    }
    //return;

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


//int my_pthread_init(){
//    sigemptyset(&sigProcMask);
//    sigaddset(&sigProcMask, SIGPROF);
//    return 0;
//}


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


/*
int enqueue(struct tcb my_tcb,int a){
    struct Node *end= malloc(sizeof(struct Node));
    end->head=my_tcb;
    end->next=NULL;
    if(front[a]==NULL){
        front[a]=end;
    }
    else{
        back[a]->next=end;
    }
    back[a]=end;
    my_tcb.thread_params.queue=a;
    return 1;
}
*/
//removes tcb to from queue
/*
int dequeue(int a){
    struct Node temp=*front[a];
    front[a]=front[a]->next;
    if(front[a]==NULL){
        back[a]=NULL;
    }
    free(&temp);
    return 1;
}
*/
//looks at next value in queue
struct queue peek(){
    int i=0;
    while(Queue[i].counter==0){
        i++;
    }
	//printf("Priority: %d\n",i);
	return Queue[i];
}	
void wrapperfunction(void *(*function)(void*),void * arg,void * retval){
	retval=function(arg);
	return;
}

void my_pthread_init(){
	main_thread = malloc(sizeof(tcb));
	main_thread->tid = tcb_num;
	main_thread->priority = 0;
	my_enqueue(&Queue[0],main_thread);
	//scheduler();
}
/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	signal(SIGALRM, scheduler);
	sigemptyset(&sigProcMask);
	sigaddset(&sigProcMask, SIGALRM);
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
	if(start == 1){
    		start = 0;
    		my_pthread_init();
    }
    //printf("Thread Created\n");
    tcb* my_tcb= malloc(sizeof(tcb));
    //Thread status is decided by scheduler
    *thread = tcb_num;
    my_tcb->tid = *thread;
    tcbs[*thread]=my_tcb;
    tcb_num++;
    getcontext(&my_tcb->thread_context);
    my_tcb->thread_context.uc_link=NULL;//initializes ucontext_t
    my_tcb->priority = 0;
	my_tcb->joinid = -1;
    //sigfillset(&my_tcb->thread_context.uc_sigmask);
    my_tcb->thread_context.uc_stack.ss_sp = malloc(MEM);
    my_tcb->thread_context.uc_stack.ss_flags = 0;
    my_tcb->thread_context.uc_stack.ss_size = MEM;
    //attaches function to context
    makecontext(&my_tcb->thread_context,(void*)&wrapperfunction,3,function,arg,my_tcb->retval);
    //Thread status is decided by scheduler
    my_tcb->thread_params.run=function;
    my_tcb->thread_params.arg=arg;
    my_tcb->thread_params.joinable = 1;
    
    my_enqueue(&Queue[0],my_tcb);//adds TCB to priority queue
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    scheduler();
    return 0;
}
/*
int my_pthread_yield() {
	current_thread->status = YIELDED;
	scheduler();
	return 1;
}

int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
	if(tcbs[thread]->tid != thread){ return ESRCH;}
	if(tcbs[thread]->joinid != -1){return EINVAL;}
	if(tcbs[thread]->tid == current_thread->joinid){return EDEADLK;}
	tcbs[thread]->joinid = current_thread->tid;
	current_thread->status = WAITING;
	
	//my_enqueue(&tcbs[thread]->waiting, current_thread);
	tcbs[thread]->waiting = current_thread;
	*value_ptr = tcbs[thread]->retval;
	sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
	scheduler();
	return 1;
}

void my_pthread_exit(void *value_ptr) {
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
	value_ptr=current_thread->retval;
	current_thread->status = EXITED;
	if(current_thread->joinid != -1){
		int i = 0;
		i = my_enqueue(&Queue[current_thread->waiting->priority],current_thread->waiting);	
		if(i != 1){perror("TCB was not enqueued");}
	}
	current_thread->status = EXITED;
	free(&current_thread->thread_context.uc_stack);//clears stack in thread's context
   sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
   scheduler();
} 
 */


/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	current_thread->status=YIELDED;
	printf("----STATUS: YILEDED\n");
    my_dequeue(&Queue[current_thread->thread_params.queue]);//removes from its current queue
    my_enqueue(&Queue[5],current_thread);//places current tcb in waiting queue
    current_thread->status=WAITING;
	printf("----STATUS: WAITING\n");

	int oldID =  current_thread->tid;
	current_thread=peek().front;//changes tcb pointer to new current tcb
	int newID = current_thread->tid;
	printf("Old Current - %d\n", oldID);
	printf("New Current - %d\n", newID);
	if(oldID != newID){
		if(swapcontext(&current_thread->thread_context,current_thread->thread_context.uc_link)==-1){
			printf("swapcontext - CONTEXT ERRROR\n");
		}
	}	
	return 0;

}

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	value_ptr=current_thread->retval;
	current_thread->status=EXITED;//changes status to finished
    my_dequeue(&Queue[current_thread->thread_params.queue]);
    free(&current_thread->thread_context.uc_stack);//clears stack in thread's context
    my_enqueue(&Queue[6],current_thread);//places finished thread in completed queue
    current_thread=peek().front;
}

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
    tcb * t=tcbs[thread];
    t->joinid=current_thread->tid;
	printf("Waiter tid: %d\n", current_thread->tid);
	printf("Waitee tid: %d\n",  t->tid);
    if(t->tid==current_thread->tid){
        printf("Same Thread! DON'T JOIN\n");
		return 0;//same thread
    }else if(t->joinid==t->tid || current_thread->joinid==t->tid){
        printf("DEADLOCK\n");
		return EDEADLK;//1 means error EDEADLK
        //this means two threads joined with eachother or a thread joined with itself
    } 
    while(t->status!=EXITED){
        printf("t->status : %d\n", t->status);
		if(t->thread_params.joinable==0){
            printf("NOT JOINABLE\n");
			return EINVAL;//not joinable
        }
		printf("YIELDING\n");
		if(my_pthread_yield()==0){
			break;
		}
    }
	printf("THREAD FINISHED YIELD, NOW EXITING");
    
	*value_ptr=t->retval;
    my_pthread_exit(*value_ptr);
	return 0;
}

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
    mutex->lock = 0;
    mutex->flags = 0;
    printf("MUTEX INITIALIZED\n");
	return 0;
}

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	while(__sync_lock_test_and_set(&mutex->lock, 1)){ //shared mutex was locked
//        spin_acquire(LOCK);
//        if(mutex->lock == 1){ //value of mutex->lock is
           
			printf("--MUTEX Locked, calling yield\n");
			my_pthread_yield();
//            spin_release(LOCK);
            //thread is in waiting queue and blocked
//        }else{
//            spin_release(LOCK);
//        }
    }
	printf("--MUTEX GOT THE LOCK\n");
    return 0; //got the lock
}

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
//    spin_acquire(LOCK);
    __sync_lock_release(&mutex->lock);
//    spin_release(LOCK);
	printf("--MUTEX Unlocked, calling yield\n");
	my_pthread_yield();
		
//	current_thread->status = WAITING; 
    return 0;
}

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
    if(!mutex)return EINVAL;
    mutex->lock = 0;
    mutex->flags = 0;
    return 0;
}
