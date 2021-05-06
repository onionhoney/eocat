CC = gcc
CFLAGS = -Wall -O3 
all: solver.c solver.h
	$(CC) $(CFLAGS) -o solver solver.c
