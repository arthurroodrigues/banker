CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -g

all: banker

banker: banker.c banker.h
	$(CC) $(CFLAGS) -o banker banker.c

clean:
	rm -f banker result.txt