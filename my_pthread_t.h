// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:
#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE
//macro my_ to pthread functions
#define pthread_create(a, b, c, d) my_pthread_create(a, b, c, d)
#define pthread_yield() my_pthread_yield()
#define pthread_join(a, b) my_pthread_join(a, b)
#define my_pthread_exit(x) my_my_pthread_exit(x)
//macro my_ to mutex functions
#define pthread_mutex_init(a, b)  my_pthread_mutex_init(a, b)
#define pthread_mutex_lock(a) my_pthread_mutex_lock(a)
#define pthread_mutex_unlock(a) my_pthread_mutex_unlock(a)
#define pthread_mutex_destroy(a)	my_pthread_mutex_destroy(a)
/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <ucontext.h>
#include <errno.h>


//StackSize of each thread : 16 Kilobytes
#define MEM 16384
#define LEVELS 4
//Quantum duration : 25 ms
#define QUANTUM 25000
//Check for starvation every 50 quanta
#define MAINTAINENCE_CYCLE_THRESHOLD 50

int maintainence_cycle_counter = 0;

//Thread states
typedef enum state
{
    READY, RUNNING, WAITING, YIELDED, EXITED
} state;


typedef uint my_pthread_t;
int tcb_num=0;

//Thread info to be passed in TCB struct
struct thread_info {
    void *(*run) (void *);//function
    void *arg;//arguments for function
    int joinable;
    int queue;
    long int readyTime;
    int execTime;
    long int deadline;
    struct thread_info *recov_info;
};


//Thread Control Block
typedef struct threadControlBlock {
    /* add something here */
    my_pthread_t tid;
    my_pthread_t ptid;
    void * retval;
    void ** join_retval;
    state status; // Created enum state ; would not need this. 0=joined 1=yielded 2=exited 3=running
    ucontext_t thread_context;//stores the context of the thread
    struct thread_info thread_params;//thread info
    uint executedTime;//tells how long program has been running
    struct threadControlBlock * nextThread;
    uint priority;//used by scheduler object
    uint magic_key;//used for debugging
}tcb;

// MUTEX Struct Definition
typedef struct my_pthread_mutex_t {
    int lock; //0 or 1, set to 0(unlocked) by default
    int flags; //TODO: Define further?
} my_pthread_mutex_t;

struct queue{
    tcb * front;
    tcb * back;
    int counter;
};


struct queue Queue[7]; // all queues 5=running,6=waiting,7=completed


tcb *tcbs[1000]; // all the tcbs that you could have are intialized.

//pointers to back of each queue
struct Node* back[7];

//pointers to the front of each queue
struct Node* front[7];

sigset_t sigProcMask;
// Feel free to add your own auxiliary data structures

struct itimerval it;
struct sigaction act, oact;

//next thread to run for the scheduler
tcb * get_next_thread_to_run();
//get current time
long int get_current_time();
//adds tcb to queue
//int enqueue(tcb* my_tcb,int a);
int my_enqueue(struct queue * my_queue, tcb * my_tcb);
//removes tcb to from queue
//int dequeue(int a);
tcb * my_dequeue(struct queue * my_queue);
//looks at next tcb in priority queue
struct Node* peek();
/* Function Declarations: */
/*spin-locks*/
void spin_acquire(my_pthread_mutex_t *mutex);
void spin_release(my_pthread_mutex_t * mutex);

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#endif
