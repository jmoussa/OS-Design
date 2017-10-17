// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: nandan Thakkar
// username of iLab: nt284
// iLab Server:

#include "my_pthread_t.h"
#include <errno.h>

my_pthread_mutex_t *LOCK;
tcb * tsbPtr = tcbs[0];
static tcb *current_thread, *main_thread;
static int Count_time_executed_for_maintainance_cycle =0;

void spin_aquire(my_pthread_mutex_t *mutex){
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

void init(){
    int i = 0;
    for(i = 0; i < 7 ; i++){
        back[i] = &Queue[i];
        front[i] = &Queue[i];
    }
}


void scheduler(int sig){
    struct itimerval my_timer;
    ucontext_t uc_temp; //To store the context of the main thread
    
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
        if (temp->status == WAITING)
        {
            //pass
            current_thread = scheduler_get_next_thread();
            if (current_thread != NULL)
                current_thread->status = RUNNING;
        }
        else if (temp->status == EXITED)
        {
            //pass
            current_thread = scheduler_get_next_thread();
            if (current_thread != NULL)
                current_thread->status = RUNNING;
        }
        else if (temp->status == YIELDED)
        {
            //put the thread back into the original queue
            scheduler_add_thread(temp, temp->priority);
            current_thread = scheduler_get_next_thread();
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
            scheduler_add_thread(temp, new_priority);
            
            current_thread = scheduler_get_next_thread();
            if (current_thread != NULL)
                current_thread->status = RUNNING;
        }
    }
    else //control is here because current thread is NULL
    {
        current_thread = scheduler_get_next_thread();
        if (current_thread != NULL)
        {
            current_thread->status = RUNNING;
        }
    }
    
    //Update MLFQ on the basis of starvation
    if (Count_time_executed_for_maintainance_cycle > MAINTAINENCE_CYCLE_THRESHOLD * QUANTUM)
    {
        Count_time_executed_for_maintainance_cycle = 0;
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
        if (temp != NULL)
            swapcontext(&(temp->thread_context), &(current_thread->thread_context));
        else
        /*
         stores main context
         (enters else statement only in first call to scheduler)
         */
            swapcontext(&uc_temp, &(current_thread->thread_context));
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


int my_pthread_init(){
    sigemptyset(&sigProcMask);
    sigaddset(&sigProcMask, SIGPROF);
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



//add tcb to queue
//int enqueue(struct tcb* my_tcb,int a){
//    struct Node * end= malloc(sizeof(Node));
//    end->head=my_tcb;
//    end->next=NULL;
//    if(*front[a]==NULL){
//        *front[a]=end;
//    }
//    else{
//        *back[a]->next=end;
//    }
//    *back[a]=end;
//    my_tcb->thread_params.queue=a;
//    return 1;
//}
//
////removes tcb to from queue
//tcb * dequeue(int a){
//    
//    struct Node * temp = front[a];
//    *front[a] = front[a]->next;
//    if(*front[a]==NULL)}{
//        *end[a]=NULL;
//    }
//    tcb * tempTCB = temp->head;
//    free(temp);
//    return temp;
//}

//looks at next value in queue
struct Node * peek(){
    int i=0;
    while(*front[i]==NULL){
        i++;
    }
    return *front[i];
}
/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
    
    struct tcb *my_tcb=(struct tcb *) malloc(sizeof(tcb));
    //Thread status is decided by scheduler
    my_tcb.tid=thread;
    my_tcb.thread_context.uc_link=;//initializes ucontext_t
    sigfillset(my_tcb.thread_context.uc_sigmask);
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
    *tcbPtr->status=1;
    dequeue((*tcbPtr)->thread_params->queue);//removes from its current queue
    enqueue(*tcbPtr,5);//places current tcb in waiting queue
    *tcbPtr=peek();//changes tcb pointer to new current tcb
    //gets current context from current tcb and swaps
    swapcontext(tcbPtr.thread_context,tcbPtr.thread_context.uclink);
    return 0;
}

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
    *tcbPtr->status=2;//changes status to finished
    dequeue(*tcbPtr->thread_params->queue);
    free(*tcbPtr->ucontext_t->uc_stack);//clears stack in thread's context
    enqueue(*tcbPtr,6);//places finished thread in completed queue
    *tcbPtr=peek()->head;
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
            //thread is in waiting queue and blocked
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
