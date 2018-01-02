#compiler
CC = g++

CFLAGS = -g -Wall -c

compile : causualMessageOrdering

causualMessageOrdering: causualMessageOrdering.o
	g++ causualMessageOrdering.o -o causualMessageOrdering -lpthread

causualMessageOrdering.o: causualMessageOrdering.cpp
	g++ -c -lpthread causualMessageOrdering.cpp 

clean:
	rm -rf *.o compile
