CC 	= gcc

CFLAGS  = -Wall -g -I .

LD 	= gcc

LDFLAGS  = -Wall -g 

PROGS	= nums

NUMOBJS    = numbersmain.o

OBJS	= $(NUMOBJS) 

SRCS	= numbersmain.c

HDRS	= 

EXTRACLEAN = core $(PROGS)

all: 	$(PROGS)

allclean: clean
	@rm -f $(EXTRACLEAN)

clean:	
	rm -f $(OBJS) *~ TAGS

nums: numbersmain.o libLWP.a 
	$(LD) $(LDFLAGS) -o nums numbersmain.o -L. -lLWP

numbermain.o: lwp.h

libLWP.a: lwp.c rr.c
	gcc -c rr.c lwp.c magic64.S 
	ar r libLWP.a lwp.o rr.o magic64.o
	rm lwp.o

submission: lwp.c rr.c Makefile README
	tar -cf project2_submission.tar lwp.c rr.c Makefile README
	gzip project2_submission.tar
