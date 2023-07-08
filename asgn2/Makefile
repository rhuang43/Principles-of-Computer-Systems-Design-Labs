#The tutor, LEO worked with me to lead me to this version of the makefile after the one I had failed to make
EXEC = httpserver
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:%.c=%.o)

CC = clang
CFLAGS = -Wall -Wextra -Werror -pedantic
LDFLAGS = 
PROGRAM = httpserver
FILES = httpserver.o

all: $(PROGRAM)

$(PROGRAM): $(FILES) asgn2_helper_funcs.a
	$(CC) $(CFLAGS) -o $(PROGRAM) $(FILES) asgn2_helper_funcs.a $(LDFLAGS)
$(FILES): httpserver.c asgn2_helper_funcs.h
	$(CC) $(CFLAGS) -c httpserver.c

clean: 
	rm -f $(PROGRAM) $(FILES)

format: 
	clang-format -i -style=file *.[ch]

scan-build: clean
	scan-build make