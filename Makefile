CC = gcc
CFLAGS = -Wall -O3

all:
	$(CC) -o bin $(CFLAGS) trabalho.c -w
