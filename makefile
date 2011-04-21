#Makefile: if you use gcc, then change g++ to gcc
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

default: clean main

main : $(OBJS)
	$(CC) $(LFLAGS) -pthread eaglefeed.cpp -o eaglefeed
	chmod 700 eaglefeed

clean:
	rm -f eaglefeed

