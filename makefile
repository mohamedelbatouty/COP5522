CC=gcc
CFLAG= -Wall -I. -O0 -mavx2 -fopenmp

TARGETS=unoptimized generate openMP pthreads

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

openMP: openmp.o microtime.o
	$(CC) $(CFLAG) -o $@ $^

openMP.o: openmp.c microtime.h
	$(CC) $(CFLAG) -c $<
	
pthreads: pthreads_optimization.o microtime.o
	$(CC) $(CFLAG) -o $@ $^

pthreads_optimization.o: pthreads_optimization.c microtime.h
	$(CC) $(CFLAG) -c $<

clean:
	rm -f *.o *~ core $(TARGETS)
