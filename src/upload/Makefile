CC=gcc
OPTS=-g -std=c99 -Werror

all: main.o cache.o
	$(CC) $(OPTS) -o cache main.o cache.o -lm

main.o: main.c cache.h
	$(CC) $(OPTS) -c main.c

cache.o: cache.h cache.c
	$(CC) $(OPTS) -c cache.c

clean:
	rm -f *.o cache;
