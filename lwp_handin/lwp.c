#define _GNU_SOURCE
#ifndef lwp_h
#define lwp_h
#include "lwp.h"
#endif
#ifndef rr_h
#define rr_h
#include "rr.h"
#endif
#ifndef stddef_h
#define stddef_h
#include <stddef.h>
#endif
#ifndef sysmman_h
#define sysmman_h
#include <sys/mman.h>
#endif
#ifndef unistd_h
#define unistd_h
#include <unistd.h>
#endif
#ifndef stdio_h
#define stdio_h
#include <stdio.h>
#endif
#ifndef systime_h
#define systime_h
#include <sys/time.h>
#endif
#ifndef sysresource_h
#define sysresource_h
#include <sys/resource.h>
#endif
#ifndef stdlib_h
#define stdlib_h
#include <stdlib.h>
#endif

struct scheduler rr_publish = {NULL, NULL, rr_admit, rr_remove, rr_next, rr_qlen};
scheduler currScheduler = &rr_publish;
int NUM_THREADS = 0;
thread currThread = NULL;
thread terminatedQueue = NULL; 
thread waitingQueue = NULL;
void deallocateThread(thread t){
	// free stack
	if(t->stack)
	{
		free(t->stack);
	}
	// free thread
	if(t)
	{
		free(t);
	}
}

void lwp_wrap(lwpfun fun, void *arg){
	// Call the given lwpfunction with the given argument.
	// Calls lwp_exit() with its return value
	int rval;
	rval = fun(arg);
	lwp_exit(rval);
};

tid_t lwp_create(lwpfun function, void *argument){
	// Creates a new thread and admits it to the current scheduler. The thread’s resources will consist of a
	// context and stack, both initialized so that when the scheduler chooses this thread and its context is
	// loaded via swap_rfiles() it will run the given function. This may be called by any thread.
	long page_size = sysconf(_SC_PAGE_SIZE);
	struct rlimit rlim;
	size_t soft_limit = 0, default_size = 0, stack_size = 0;
	ssize_t howbig = 0;
	thread new_thread = NULL;
	unsigned long *SP;
	new_thread = (thread)malloc(sizeof(struct threadinfo_st));

	NUM_THREADS++;

	// set tid
	new_thread->tid = NUM_THREADS;

	if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
    	if (rlim.rlim_cur != RLIM_INFINITY) {
        	// Use soft limit for stack size
        	soft_limit = rlim.rlim_cur;
        	// Check if soft limit is a multiple of the page size
        	if (soft_limit % page_size != 0) {
            // Round up to the nearest multiple of the page size
            	soft_limit = ((soft_limit / page_size) + 1) * page_size;
        	}
			howbig = soft_limit;
    	} else{
			// RLIMIT_STACK is RLIM_INFINITY
			default_size = 8 * 1024 * 1024; // 8MB in bytes
			// Round up to the nearest multiple of the page size
			stack_size = ((default_size / page_size) + 1) * page_size;
			howbig = stack_size;
		}
	} else {
		// RLIMIT_STACK does not exist
		default_size = 8 * 1024 * 1024; // 8MB in bytes
		// Round up to the nearest multiple of the page size
		stack_size = ((default_size / page_size) + 1) * page_size;
		howbig = stack_size;
	}
	
	// allocate stack
	new_thread->stack = (unsigned long *)malloc(howbig * sizeof(unsigned long));
	new_thread->stacksize = howbig;

	// load registers
	new_thread->state.rdi = (unsigned long) function;
	new_thread->state.rsi = (unsigned long) argument;
	new_thread->state.fxsave = FPU_INIT;

	//go to top of stack
	SP = new_thread->stack + howbig;
	
	// add lwp_wrap to stack
	SP--;
    *SP = (unsigned long)lwp_wrap;
	SP--;
	
	// set base pointer to lwp_wrap where function is stored and stack pointer
	new_thread->state.rbp = (unsigned long)(SP);
	new_thread->state.rsp = (unsigned long)(SP);

	// set thread status to live
	new_thread->status = LWP_LIVE;

	// admit to scheduler
	currScheduler->admit(new_thread);
	return new_thread->tid;
};

void lwp_start(void){
	// Starts the threading system by converting the calling thread—the original system thread—into a LWP
	// by allocating a context for it and admitting it to the scheduler, and yields control to whichever thread the
	// scheduler indicates. It is not necessary to allocate a stack for this thread since it already has one.
	thread original = NULL;
	original = (thread)malloc(sizeof(struct threadinfo_st));

	// set tid
	NUM_THREADS++;
	original->tid = NUM_THREADS;
	original->stack = NULL;
	
	currScheduler->admit(original);
	// set current thread
	currThread = original;
	// yield to whatever scheduler decides
	lwp_yield();
};

void lwp_yield(void){
	// Yields control to the next thread as indicated by the scheduler. If there is no next thread, calls exit(3)
	// with the termination status of the calling thread (see below).
	// get next thread
	thread prevThread = currThread;
	unsigned int status;
	if((currThread = currScheduler->next()) == NULL){
		status = prevThread->status;
		deallocateThread(prevThread);
		exit(status);
	}
	if(prevThread){
		swap_rfiles(&(prevThread->state), &(currThread->state));	
	}else{
		swap_rfiles(NULL, &(currThread->state));
	}
};

void lwp_exit(int exitval){
	// Terminates the calling thread. Its termination status becomes the low 8 bits of the passed integer. The
 	// thread’s resources will be deallocated once it is waited for in lwp_wait(). Yields control to the next
	// thread using lwp_yield()
	thread curr = NULL;
	// set last 8 bits to exit + term flag
	currThread->status = MKTERMSTAT(LWP_TERM, (exitval & 0xFF));
	// remove from scheduler
	currScheduler->remove(currThread);
	// add to termination queue
	if(terminatedQueue == NULL) {
		terminatedQueue = currThread;
	} else {
		curr = terminatedQueue;
		while(curr->lib_two != NULL)
		{
			curr = curr->lib_two;
		}
		if(curr->lib_one){
			curr->lib_one->lib_two = currThread;
		}
		currThread->lib_two = NULL;
	}

	// check if there are threads in waiting queue
	if(waitingQueue != NULL)
	{
		waitingQueue->exited = terminatedQueue;
		// readmit to scheduler
		currScheduler->admit(waitingQueue);
	}
	lwp_yield();
};

tid_t lwp_wait(int *status){
	// Deallocates the resources of a terminated LWP. If no LWPs have terminated and there still exist
	// runnable threads, blocks until one terminates. If status is non-NULL, *status is populated with its
	// termination status. Returns the tid of the terminated thread or NO_THREAD if it would block forever
	// because there are no more runnable threads that could terminate.
	// Be careful not to deallocate the stack of the thread that was the original system thread
	tid_t tid = 0;
	thread term = NULL, temp = NULL, terminated = NULL, temp2 = NULL;

	if (currScheduler->qlen() <= 1)
    {
        return NO_THREAD;
    }

	// check if there are terminated threads
	if(terminatedQueue != NULL)
	{
		// remove head (oldest) from terminated threads queue
		term = terminatedQueue;
		if(term != NULL)
		{
			terminatedQueue->lib_two->lib_one = NULL;
			terminatedQueue = terminatedQueue->lib_two;
		}
		
		if (status != NULL)
		{
			*status = term->status;
		}
		tid = term->tid;
		// deallocate the thread and return status
		deallocateThread(term);
		return tid;
	}

	// no terminated threads, and therefore calling thread should block
	// remove from scheduler
	currScheduler->remove(currThread);
	// add to waiting queue
	if(waitingQueue == NULL) {
		waitingQueue = currThread;
	} else {
		temp = waitingQueue;
		while(temp->lib_two != NULL){
			temp = temp->lib_two;
		}
		temp->lib_two = currThread;
		currThread->lib_one = temp;
		currThread->lib_two = NULL;
			
	}
	// move on to next thread
	lwp_yield();
	// retrieve exited
	terminated = currThread->exited;
	// remove terminated from terminated queue
	temp = terminatedQueue;
	if(temp != NULL){
		if(terminated == temp){
			terminatedQueue = temp->lib_two;
			if(terminatedQueue != NULL){
				terminatedQueue->lib_one = NULL;
			}
		}
		while(temp->lib_two != NULL) {
			temp = temp->lib_two;
			if(temp == terminated){
				temp->lib_one->lib_two = temp->lib_two;
				temp->lib_two->lib_one = temp->lib_one;
				break;
			}
		}
	}
	// remove current thread from waiting queue
	temp2 = waitingQueue;
	if(currThread == temp2){
		waitingQueue = temp2->lib_one;
		if(waitingQueue != NULL){
			waitingQueue->lib_one = NULL;
		}
	}
	while(temp2->lib_two != NULL) {
		temp2 = temp2->lib_two;
		if(temp2 == waitingQueue){
			temp2->lib_one->lib_two = temp2->lib_two;
			temp2->lib_two->lib_one = temp2->lib_one;
			break;
		}
	}
	if (terminated != NULL) {
        if (status != NULL) {
            *status = terminated->status;
        }
		tid = terminated->tid;
        deallocateThread(terminated);
        return tid;
    }
};

tid_t lwp_gettid(void){
	if(currThread){
		return currThread->tid;
	}
	return NO_THREAD;
}

thread tid2thread(tid_t tid){
	// returns the thread corresponding to the given thread ID
	// or NULL if the ID is invalid
	thread curr = terminatedQueue;
	if(curr){
		while(curr->sched_two != NULL){
			if(curr->tid == tid) {
				return curr;
			}
			curr = curr->sched_two;
		}
	}
	curr = waitingQueue;
	if(curr){
		while(curr->sched_two != NULL){
			if(curr->tid == tid) {
				return curr;
			}
			curr = curr->sched_two;
		}
	}
	curr = tidTothread(tid);
	return curr;
}

void lwp_set_scheduler(scheduler sched){
	thread head = NULL;
	if(!sched){
		sched = &rr_publish;
	}
	head = currThread;
    if (sched->init){
        sched->init();
    }
	// readmit all threads to new scheduler
    while ((currThread = currScheduler->next()))
    {
        currScheduler->remove(currThread);
        sched->admit(currThread);
    }
    if (currScheduler->shutdown){
        currScheduler->shutdown();
    }
    currScheduler = sched;

	currThread = head;
};

scheduler lwp_get_scheduler(void){
	return currScheduler;
};