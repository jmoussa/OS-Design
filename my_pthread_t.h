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

typedef uint my_pthread_t;
int tcb_num=0;

//Thread info to be passed in TCB struct
struct thread_info {
    void *(*run) (void *);//function
    void *arg;//arguments for function
    int joinable;
    int queue;
    int readyTime;
    int execTime;
    int deadline;
    struct thread_info *recov_info;
};


//Thread Control Block
typedef struct tcb {
	/* add something here */
    my_pthread_t tid;
    my_pthread_t ptid;
    void * result;
    void ** join_retval;
    int status;//0=joined 1=yielded 2=exited 3=running
    ucontext_t thread_context;//stores the context of the thread
    struct thread_info thread_params;//thread info
    uint executedTime;//tells how long program has been running
    struct tcb *childThread;
    uint priority;//used by scheduler object
    uint magic_key;//used for debugging
} ; 

// MUTEX Struct Definition
typedef struct my_pthread_mutex_t {
	int lock; //0 or 1, set to 0(unlocked) by default
    int flags; //TODO: Define further?
} my_pthread_mutex_t;

struct Node{
    struct tcb  head;
    struct Node * next;
};

struct Node Queue[7]; // all queues 4=running,5=waiting,6=completed
struct tcb tcbs[1000]; // all the tcbs that you could have are intialized.
struct tcb *tcbPtr;
//pointers to back of each queue
struct Node* back[7];
//pointers to the front of each queue
struct Node* front[7];
sigset_t sigProcMask;
// Feel free to add your own auxiliary data structures

struct itimerval it; 
struct sigaction act, oact;

//adds tcb to queue
int enqueue(struct tcb my_tcb,int a);
//removes tcb to from queue
int dequeue(int a);
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
