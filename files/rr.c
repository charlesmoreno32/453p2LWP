#define _GNU_SOURCE
#ifndef stddef_h
#define stddef_h
#include <stddef.h>
#endif
#ifndef sysmman_h
#define sysmman_h
#include <sys/mman.h>
#endif
#include <unistd.h>
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
#ifndef lwp_h
#define lwp_h
#include "lwp.h"
#endif

thread head = NULL;

void rr_admit(thread new_thread){
	if(head == NULL){
		head = &new_thread;
		new_thread->sched_one = NULL;
		new_thread->sched_two = NULL;
		return;
	}
	thread curr = head;
	while(curr->sched_two != NULL){
		curr = curr->sched_two;
	}
	curr->sched_two = &new_thread;
	new_thread->sched_one = curr;
	new_thread->sched_two = NULL;
};

void rr_remove(thread victim){
	if(head == NULL) {
		return;
	}
	if(victim == head){
		head = victim->sched_one;
	}
	if(victim->sched_one != NULL){
		victim->sched_one->sched_two = victim->sched_two;
	} 
	if(victim->sched_two != NULL){
		victim->sched_two->sched_one = victim->sched_one;
	}
};

thread rr_next(void){
	return head;
};

int rr_qlen(void){
	if(head == NULL){
		return 0;
	}
	int i = 1;
	thread curr = head;
	while(curr->sched_two != NULL){
		i++;
		curr = curr->sched_two;
	}
	return i;
};
