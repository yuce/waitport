.PHONY: build clean

CC = gcc
CFLAGS = -ansi -Wall -Wpedantic -Wshadow -std=gnu99

build: waitport

waitport: lib.h lib.c main.c
	$(CC) -o waitport lib.c main.c $(CFLAGS)

clean:
	rm -f waitport
