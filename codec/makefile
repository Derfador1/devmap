
CFLAGS+=-std=c11 -Wall -Wextra -Wpedantic -Wwrite-strings -Wstack-usage=1024 -Wfloat-equal -Waggregate-return -Winline

all: decoder encoder

decoder: decoder.c
	gcc $(CFLAGS) -o decoder decoder.c -lm

encoder: encoder.c
	gcc $(CFLAGS) -o encoder encoder.c -lm

debug: CFLAGS+=-g
debug: all

clean:
	-rm decoder
	-rm encoder

