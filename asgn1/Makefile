CC = clang
CFLAGS = -g -Wall -Wextra -pedantic -Werror
TARGET = memory

all:$(TARGET)

$(TARGET):	$(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET)