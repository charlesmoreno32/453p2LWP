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
	size_t soft_limit, default_size, stack_size;
	ssize_t howbig = 0;
	thread new_thread = (thread)malloc(sizeof(struct threadinfo_st));

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
	new_thread->stack = (unsigned long *)mmap(NULL, (howbig * sizeof(unsigned long)), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0 );
	
	new_thread->stacksize = howbig;
	unsigned long *stack_pointer;
	unsigned long *base_pointer;

	// load remaining registers
	new_thread->state.rdi = (unsigned long) function;
	new_thread->state.rsi = (unsigned long) argument;
	new_thread->state.fxsave = FPU_INIT;

	//go to top of stack
	stack_pointer = (new_thread->stack + new_thread->stacksize-1);
	stack_pointer = (unsigned long*)(((uintptr_t)stack_pointer + 15) & ~0xF);
	base_pointer = stack_pointer;

	//set the stack pointer
	
	stack_pointer--;
	*stack_pointer = (unsigned long)lwp_exit;
	// add lwp_wrap to stack
	stack_pointer--;
    *stack_pointer = (unsigned long)lwp_wrap;
	
	stack_pointer--;
    *stack_pointer = (unsigned long)base_pointer;
	
	// set base pointer to lwp_wrap where function is stored
	new_thread->state.rbp = (unsigned long)(stack_pointer);
	new_thread->state.rsp = (unsigned long)(stack_pointer);

	// set thread status to live
	new_thread->status = LWP_LIVE;

	// admit to scheduler
	lwp_get_scheduler()->admit(new_thread);
	return new_thread->tid;
};

void lwp_start(void){
	// Starts the threading system by converting the calling thread—the original system thread—into a LWP
	// by allocating a context for it and admitting it to the scheduler, and yields control to whichever thread the
	// scheduler indicates. It is not necessary to allocate a stack for this thread since it already has one.
	thread original = (thread)malloc(sizeof(struct threadinfo_st));
	//thread currThread  = currScheduler->next();

	// set tid
	NUM_THREADS++;
	original->tid = NUM_THREADS;
	original->stack = NULL;
	scheduler currentScheduler = lwp_get_scheduler();
	currentScheduler->admit(original);
	// set current thread
	//currThread = currentScheduler->next();
	
	// yield to whatever scheduler decides
	lwp_yield();
};

void lwp_yield(void){
	// Yields control to the next thread as indicated by the scheduler. If there is no next thread, calls exit(3)
	// with the termination status of the calling thread (see below).
	
	// save context of current thread
	//swap_rfiles(&(currThread->state), NULL);
	
	// get next thread
	
	thread next_thread = lwp_get_scheduler()->next();
	currThread = next_thread;
	if(next_thread){
		printf("%d\n", next_thread->tid);
	} else{
		printf("NULL");
	}
	
	
	// load context of next thread
	

	//if(next_thread == NULL){
	//	lwp_exit(currThread->status);
	//}

	if(currThread){
		swap_rfiles(&(currThread->state), &(next_thread->state));
	}else{
		swap_rfiles(NULL, &(next_thread->state));
	}
};

void lwp_exit(int exitval){
	// Terminates the calling thread. Its termination status becomes the low 8 bits of the passed integer. The
 	// thread’s resources will be deallocated once it is waited for in lwp_wait(). Yields control to the next
	// thread using lwp_yield()
	lwp_yield();
};

tid_t lwp_wait(int *status){
	// Deallocates the resources of a terminated LWP. If no LWPs have terminated and there still exist
	// runnable threads, blocks until one terminates. If status is non-NULL, *status is populated with its
	// termination status. Returns the tid of the terminated thread or NO_THREAD if it would block forever
	// because there are no more runnable threads that could terminate.
	// Be careful not to deallocate the stack of the thread that was the original system thread
	return 1;
};

tid_t lwp_gettid(void){
	return NO_THREAD;
}

thread tid2thread(tid_t tid){
	// returns the thread corresponding to the given thread ID
	// or NULL if the ID is invalid
	//thread head = 
	return NULL;
}

void lwp_set_scheduler(scheduler sched){
	currScheduler = sched;
};

scheduler lwp_get_scheduler(void){
	return currScheduler;
};

