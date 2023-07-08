CC = clang -std=c99
CFLAGS = -g -Wall -Wextra -pedantic -Werror

all: queue.o

queue.o:	queue.c	queue.h
	$(CC) $(CFLAGS) -c queue.c

clean:
	rm -f queue queue.o
format:
	clang-format -i -style=file *.[c,h]