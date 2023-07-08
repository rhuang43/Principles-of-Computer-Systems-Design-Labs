CC = clang

CFLAGS = -gdwarf-4 -g -Wall -Wextra -pedantic -Werror

all: cacher

cacher: List.o cacher.o
	$(CC) $(CFLAGS) -o cacher List.o cacher.o

List.o: List.c List.h
	$(CC) $(CFLAGS) -c List.c

cacher.o: cacher.c List.h
	$(CC) $(CFLAGS) -c cacher.c

format:
	clang-format -i -style=file *.c *.h

clean:
	rm -f *.o cacher

