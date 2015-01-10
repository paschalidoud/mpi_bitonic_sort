CC = gcc
FLG = -O3 -w
NAME = serial_bitonic

all: bitonic.o 
	$(CC) $(FLG) bitonic.o -o $(NAME)

bitonic.o: bitonic.c
	$(CC) $(FLG) bitonic.c -c

clean:
	rm -f *.o *.out *.exe
	rm -f serial_bitonic
