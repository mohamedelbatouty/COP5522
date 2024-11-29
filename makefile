CC = gcc
MPI = mpicc
CFLAG = -Wall -I. -O3 -march=native -fopenmp
TARGETS = unoptimized generate openmp optimized_merge_sort

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

optimized_merge_sort: optimized_merge_sort.o microtime.o
	$(CC) $(CFLAG) -o $@ $^

optimized_merge_sort.o: optimized_merge_sort.c microtime.h
	$(CC) $(CFLAG) -c $<

mpi: mpi.o microtime.o
	$(MPI) $(CFLAG) -o $@ $^

mpi.o: mpi.c microtime.h
	$(MPI) $(CFLAG) -c $<

clean:
	rm -f *.o *~ core $(TARGETS)
