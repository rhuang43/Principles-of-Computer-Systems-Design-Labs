#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER 4096

int get(const char *filename) {

    char buf[BUFFER];
    int file = open(filename, O_RDONLY, 0666);
    ssize_t readfile = read(file, buf, BUFFER);
    if (file == -1) {
        // fprintf(stderr, "Error opening file: %s\n", filename);
        fprintf(stderr, "Invalid Command\n");
        return 1;
    }
    if (readfile == -1) {
        // fprintf(stderr, "Error reading file: %s\n", filename);
        fprintf(stderr, "Operation Failed\n");
        return 1;
    }
    if (write(STDOUT_FILENO, buf, readfile) == -1) {
        // fprintf(stderr, "Error writing to stdout\n");
        fprintf(stderr, "Operation Failed\n");
        return 1;
    }
    close(file);
    return 0;
}
int set(char *filename) {
    char buf[BUFFER];
    int file = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
    while (1) {
        ssize_t userInput = read(STDIN_FILENO, buf, BUFFER);
        if (userInput == 0) {
            return 0;
        }
        write(file, buf, userInput);
    }
    close(file);
    return 0;
}
int main() {
    int checker = 0;
    char buf[BUFFER];
    ssize_t userInput = read(STDIN_FILENO, buf, BUFFER);
    buf[userInput - 1] = '\0'; // remove trailing newline character
    char *command = strtok(buf, " ");
    char *filename = strtok(NULL, "");
    if (strcmp(command, "get") == 0) {
        while (1) {
            ssize_t userInput = read(STDIN_FILENO, buf, BUFFER);
            if (userInput != 0) {
                checker += 1;
            }
            if (userInput == 0) {
                break;
            }
        }
        if (checker != 0) {
            fprintf(stderr, "Invalid Command\n");
            return 1;
        }
        get(filename);
    } else if (strcmp(command, "set") == 0) {
        set(filename);
        fprintf(stdout, "OK\n");
        return 0;
    } else {
        fprintf(stderr, "Invalid Command\n");
        return 1;
    }
    return 0;
}
