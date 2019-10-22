CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

ccmsck: $(OBJS)
			$(CC) -o ccmsck $(OBJS) $(LDFLAGS)

$(OBJS): ccmsck.h

test: ccmsck
		./test.sh

clean:
		rm -f ccmsck *.o *~ tmp*

.PHONY: test clean