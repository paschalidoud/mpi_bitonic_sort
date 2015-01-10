CC = mpicc.mpich2 
FLG = -O3 -w -std=c99 -lm
NAME = mpi_bitonic_sort

all: bitonic_sorter.o utils.o defines.h

	$(CC) $(FLG) bitonic_sorter.o utils.o -o $(NAME) -lm

utils.o: utils.c utils.h defines.h

	$(CC) $(FLG) utils.c -c 

bitonic_sorter.o: bitonic_sorter.c bitonic_sorter.h utils.h defines.h

	$(CC) $(FLG) bitonic_sorter.c -c 

clean:
	rm -f *.o *.out *.exe
	rm -f mpi_bitonic_sort
