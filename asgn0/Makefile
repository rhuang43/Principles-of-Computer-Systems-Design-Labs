CC = clang
CFLAGS = -g -Wall -Wextra -pedantic -Werror
TARGET = hello

all:$(TARGET)

$(TARGET):	$(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET)