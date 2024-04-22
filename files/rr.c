#include "lwp.h"
#include "rr.h"
#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

void rr_admit(thread new){};

void rr_remove(thread victim){};

thread rr_next(void){
	return NULL;
};

int rr_qlen(void){
	return 0;
};

scheduler rr_init(){
	struct scheduler rr_publish = {NULL, NULL, rr_admit, rr_remove, rr_next, rr_qlen};
	scheduler RoundRobin = &rr_publish;

	return RoundRobin;
};

int main(){
	return 0;
};
