extern struct schedular rr;

void rr_admit(thread new);

void rr_remove(thread victim);

thread rr_next(void);

int rr_qlen(void);

scheduler rr_init();
