
CFLAGS+=-std=c11
CFLAGS+=-Wall -Wextra -Wpedantic
CFLAGS+=-Wwrite-strings -Wstack-usage=1024 -Wfloat-equal -Waggregate-return -Winline
CFLAGS+=-Idatastructures

CFLAGS+=-Idatastructures/llist -Idatastructures/hash -Idatastructures/queue -Idatastructures/heap
LDFLAGS+=-Ldatastructures/llist -Ldatastructures/hash -Ldatastructures/queue -Ldatastructures/heap

LDLIBS+=-lm

devmap: devmap.o decoder.o datastructures/llist/llist.o datastructures/hash/hash.o datastructures/queue/ll_queue.o datastructures/heap/heap.o datastructures/graph/graph.o

profile:CFLAGS+=-pg
profile:LDFLAGS+=-pg
profile:devmap

.PHONY: clean debug profile

clean:
	rm *.o

debug: CFLAGS+=-g
debug: devmap
