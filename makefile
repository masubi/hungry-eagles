# CPSC545 Spring2011 Project 2
# login: masuij(login used to submit)
# Linux
# date: 03/28/11
# name: Justin Masui,
# emails: veks11@gmail.com*/

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

