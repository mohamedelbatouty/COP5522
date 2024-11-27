CC=gcc
CFLAG= -Wall -I. -O0 -mavx2 #-fopenmp

TARGETS=unoptimized generate #optimized

all: $(TARGETS)

unoptimized: unoptimized.o microtime.o
	$(CC) $(CFLAG) -o $@ $^

unoptimized.o: unoptimized.c microtime.h
	$(CC) $(CFLAG) -c $<

microtime.o: microtime.c microtime.h
	$(CC) $(CFLAG) -c $<

generate: generate.o
	$(CC) $(CFLAG) -o $@ $^

generate.o: generate.c
	$(CC) $(CFLAG) -c $<

#optimized: optimized.o microtime.o
#	$(CC) $(CFLAG) -o $@ $^

#optimized.o: optimized.c microtime.h
#	$(CC) $(CFLAG) -c $<

clean:
	rm -f *.o *~ core $(TARGETS)
