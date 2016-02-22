
CFLAGS+=-std=c11
CFLAGS+=-Wall -Wextra -Wpedantic
CFLAGS+=-Wwrite-strings -Wstack-usage=1024 -Wfloat-equal -Waggregate-return -Winline


devmap: devmap.o

profile:CFLAGS+=-pg
profile:LDFLAGS+=-pg
profile:devmap

.PHONY: clean debug profile

clean:
	rm devmap *.o

debug: CFLAGS+=-g
debug: devmap