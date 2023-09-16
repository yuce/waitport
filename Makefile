.PHONY: build clean

CC := cc

build: waitport

waitport: lib.h lib.c main.c
	$(CC) -o waitport -Os -s --std=c99 -Wall -Wpedantic -I. lib.c main.c

clean:
	rm -f waitport
