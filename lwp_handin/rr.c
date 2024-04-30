#define _GNU_SOURCE
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
#ifndef lwp_h
#define lwp_h
#include "lwp.h"
#endif

thread head = NULL;
thread curr = NULL;

thread tidTothread(tid_t tid){
	thread temp = head;
	if(!head){
		return NULL;
	}
	while(temp->sched_two){
		if(temp->tid == tid){
			return temp;
		}
		temp = temp->sched_two;
	}
	return NULL;
}

void rr_admit(thread new_thread){
	thread temp;
	if(!head){
		head = new_thread;
		new_thread->sched_one = NULL;
		new_thread->sched_two = NULL;
		return;
	}
	temp = head;
	while(temp->sched_two){
		temp = temp->sched_two;
	}
	temp->sched_two = new_thread;
	new_thread->sched_one = temp;
	new_thread->sched_two = NULL;
};

void rr_remove(thread victim){
	if(!head) {
		return;
	}
	if(victim == head){
		head = victim->sched_two;
	}
	if(victim->sched_one != NULL){
		victim->sched_one->sched_two = victim->sched_two;
	} 
	if(victim->sched_two != NULL){
		victim->sched_two->sched_one = victim->sched_one;
	}
};

thread rr_next(void){
	if(curr){
		curr = curr->sched_two;
	}
	if(!curr){
		curr = head;
	}
	return curr;
};

int rr_qlen(void){
	int i;
	if(!head){
		return 0;
	}
	i = 1;
	thread temp = head;
	while(temp->sched_two){
		i++;
		temp = temp->sched_two;
	}
	return i;
};