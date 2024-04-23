#define _GNU_SOURCE
#include "lwp.h"
#include "rr.h"

#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

struct scheduler rr_publish = {NULL, NULL, rr_admit, rr_remove, rr_next, rr_qlen};
scheduler currScheduler = &rr_publish;
int NUM_THREADS = 0;

tid_t lwp_create(lwpfun function, void *argument){
	// Creates a new thread and admits it to the current scheduler. The thread’s resources will consist of a
	// context and stack, both initialized so that when the scheduler chooses this thread and its context is
	// loaded via swap_rfiles() it will run the given function. This may be called by any thread.
	NUM_THREADS++;
	long page_size = sysconf(_SC_PAGE_SIZE);
	struct rlimit rlim;
	size_t howbig;

	thread new_thread = (thread)malloc(sizeof(struct threadinfo_st));

	// set tid
	new_thread->tid = NUM_THREADS;

	if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
    	if (rlim.rlim_cur != RLIM_INFINITY) {
        	// Use soft limit for stack size
        	size_t soft_limit = rlim.rlim_cur;
        	// Check if soft limit is a multiple of the page size
        	if (soft_limit % page_size != 0) {
            // Round up to the nearest multiple of the page size
            	soft_limit = ((soft_limit / page_size) + 1) * page_size;
        	}
			howbig = soft_limit;
    	} else{
			// RLIMIT_STACK is RLIM_INFINITY
			size_t default_size = 8 * 1024 * 1024; // 8MB in bytes
			// Round up to the nearest multiple of the page size
			size_t stack_size = ((default_size / page_size) + 1) * page_size;
			howbig = stack_size;
		}
	} else {
		// RLIMIT_STACK does not exist
		size_t default_size = 8 * 1024 * 1024; // 8MB in bytes
		// Round up to the nearest multiple of the page size
		size_t stack_size = ((default_size / page_size) + 1) * page_size;
		howbig = stack_size;
	}
	new_thread->stacksize = howbig;
	
	// allocate stack
	unsigned long *stack = mmap(NULL, howbig, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0 );
	new_thread->stack = stack;

	// load registers
	new_thread->state.rdi = (unsigned long)argument;
	new_thread->state.rsi = (unsigned long)function;
	new_thread->state.rsp = stack;


	// admit to scheduler
	//lwp_get_scheduler()->admit(new_thread);
	lwp_get_scheduler()->admit(new_thread);
	return new_thread;
};

void lwp_start(void){
	// Starts the threading system by converting the calling thread—the original system thread—into a LWP
	// by allocating a context for it and admitting it to the scheduler, and yields control to whichever thread the
	// scheduler indicates. It is not necessary to allocate a stack for this thread since it already has one.

};

void lwp_yield(void){
	// Yields control to the next thread as indicated by the scheduler. If there is no next thread, calls exit(3)
	// with the termination status of the calling thread (see below).
};

void lwp_exit(int exitval){
	// Terminates the calling thread. Its termination status becomes the low 8 bits of the passed integer. The
 	// thread’s resources will be deallocated once it is waited for in lwp_wait(). Yields control to the next
	// thread using lwp_yield()
};

tid_t lwp_wait(int *status){
	// Deallocates the resources of a terminated LWP. If no LWPs have terminated and there still exist
	// runnable threads, blocks until one terminates. If status is non-NULL, *status is populated with its
	// termination status. Returns the tid of the terminated thread or NO_THREAD if it would block forever
	// because there are no more runnable threads that could terminate.
	// Be careful not to deallocate the stack of the thread that was the original system thread

	return NO_THREAD;
};

tid_t lwp_gettid(void){
	return NO_THREAD;
}

thread tid2thread(tid_t tid){
	return NULL;
}

void lwp_set_scheduler(scheduler sched){
	currScheduler = sched;
};

scheduler lwp_get_scheduler(void){
	return currScheduler;
};

int main(){
	return 0;
}


