#ifndef rr_h
#define rr_h
thread tidTothread(tid_t tid);

void rr_admit(thread new_t);

void rr_remove(thread victim);

thread rr_next(void);

int rr_qlen(void);

extern struct scheduler rr_publish;

#endif