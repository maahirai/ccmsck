CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

ccmsck:	ccmsck.c

test: ccmsck
		./test.sh&&rm -f ccmsck *.o *~ tmp*

clean:
		rm -f ccmsck *.o *~ tmp*

.PHONY: test clean