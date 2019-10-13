CFLAGS=-std=c11 -g -static

ccmsck: ccmsck.c

test: ccmsck
		./test.sh

clean:
		rm -f ccmsck *.o *~ tmp*

.PHONY: test clean