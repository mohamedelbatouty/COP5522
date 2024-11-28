CC=mpicc
CFLAG= -Wall -I. -O0 -mavx2 -fopenmp

TARGETS=unoptimized generate openmp mpi

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

openmp: openmp.o microtime.o
	$(CC) $(CFLAG) -o $@ $^

openmp.o: openmp.c microtime.h
	$(CC) $(CFLAG) -c $<

mpi: mpi.o microtime.o
	$(CC) $(CFLAG) -o $@ $^

mpi.o: mpi.c microtime.h
	$(CC) $(CFLAG) -c $<

clean:
	rm -f *.o *~ core $(TARGETS)
