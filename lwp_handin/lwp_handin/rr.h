#include <stdio.h>

typedef struct threadinfo_st *thread;
typedef struct scheduler *scheduler;

void rr_admit(thread new_t);

void rr_remove(thread victim);

thread rr_next(void);

int rr_qlen(void);

scheduler rr_init();

extern struct scheduler rr_publish;