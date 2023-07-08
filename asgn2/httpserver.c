/************************************************************************************************************************
 Raymond Huang
 asgn2 HTTPServer
 1/26/2023
 CSE130 Sec.2 Veenstra
************************************************************************************************************************/

#include "asgn2_helper_funcs.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_SIZE  2048
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// #define PARSE_REGEX "[a-z, A-Z]{1,8}[ ][/][a-zA-Z0-9._/]{1,19}[ ][H][T][T][P][/][0-9][.][0-9][\r][\n]"
typedef struct request {
    char *command;
    char *target_path; //file
    int content_length;
    char *version;
    char *content;
    char *pcontent;
} request;

typedef struct status {
    int code;
} status;

status statt;
request r;

void put(void) {
    char buf[MAX_SIZE];
    int file = open(r.target_path, O_WRONLY | O_CREAT | O_TRUNC, 0700);
    int counter = 0;
    while (counter < r.content_length) {
        int bytes_to_write = MIN(MAX_SIZE - 1, r.content_length - counter);
        strncpy(buf, r.content + counter, bytes_to_write);
        buf[bytes_to_write] = '\0';
        write_all(file, buf, bytes_to_write);
        counter += bytes_to_write;
    }
    close(file);
}
void get(int connfd) {
    char buf[MAX_SIZE];
    buf[MAX_SIZE - 1] = '\0';
    char holder[MAX_SIZE];
    holder[MAX_SIZE - 1] = '\0';
    int file = open(r.target_path, O_RDONLY, 0666);
    int readthings = 0;
    if (MAX_SIZE < r.content_length) {
        if (read_until(file, buf, r.content_length, "\0") == -1) {
            // fprintf(stderr, "ErroSr writing to stdout\n");
            // fprintf(stderr, "Operation Failed\n");
            statt.code = 403; //update for code
            close(file);
        }
        write_all(connfd, buf, r.content_length);
    } else {
        int sum = 0;
        while (sum < r.content_length) {
            readthings = read(file, buf, MAX_SIZE - 2);
            write_all(connfd, buf, readthings);
            sum += readthings;
        }
    }
    close(file);
}
void version(void) {
    char response[MAX_SIZE];
    response[MAX_SIZE - 1] = '\0';
    for (int i = 10; i < 100; i++) {
        sprintf(response, "HTTP/1.%d", i);
        if (strcmp(r.version, response) == 0) {
            statt.code = 400;
            break;
        }
    }
    for (int i = 2; i < 10; i++) {
        sprintf(response, "HTTP/1.%d", i);
        if (strcmp(r.version, response) == 0) {
            statt.code = 505;
            break;
        }
    }
    if (strcmp(r.version, "HTTP/1.0") == 0) {
        statt.code = 505;
    }
}
int parser(char *buffer) {
    char *rest = NULL;
    char *token = NULL;
    char c[MAX_SIZE];
    c[MAX_SIZE - 1] = '\0';
    r.command = strtok_r(buffer, "/", &rest);
    r.target_path = strtok_r(NULL, " ", &rest);
    r.version = strtok_r(NULL, "\r\n", &rest);
    if (strcmp(r.version, "HTTP/1.1") != 0) {
        version();
        return -1;
    } else if (strstr(r.version, ".txt") != NULL) {
        statt.code = 500;
        return -1;
    } else if (strcmp(r.command, "GET ") == 0) {
        int file = open(r.target_path, O_RDONLY, 0666);
        if (file <= 0) {
            // fprintf(stderr, "Error opening file: %s\n", filename);
            // fprintf(stderr, "Invalid Command\n");
            statt.code = 404; //update for code
            close(file);
            return -1;
        }
        struct stat f; //getting content length / counting bytes
        stat(r.target_path, &f);
        r.content_length = f.st_size;
        if (read_until(file, c, MAX_SIZE, "\0") == -1) {
            // fprintf(stderr, "ErroSr writing to stdout\n");
            // fprintf(stderr, "Operation Failed\n");
            statt.code = 403; //update for code
        }
        close(file);
        return 0;
    } else if (strcmp(r.command, "PUT ") == 0) {
        int f = open(r.target_path, O_WRONLY | O_TRUNC, 0700);
        if (f < 0) {
            statt.code = 201;
        }
        close(f);
        token = strtok_r(NULL, " ", &rest); //gets the word content length
        while (strstr(token, "Content-Length") == NULL) {
            token = strtok_r(NULL, " ", &rest);
        }
        r.content_length
            = atoi(strtok_r(NULL, "\r\n", &rest)); //gets the actual content length number
        r.content = strtok_r(NULL, "", &rest);
        r.content += 3;

    } else {
        statt.code = 501;
        return -1;
    }
    return 0;
}
void endgame(char *buffer, int connfd) {
    char response[MAX_SIZE];
    response[MAX_SIZE - 1] = 0;
    switch (statt.code) {
    case 200:
        if (strcmp(r.command, "GET ") == 0) {
            sprintf(response, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", r.content_length);
            write_all(connfd, response, strlen(response));
            get(connfd);
        } else {
            sprintf(response, "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK\n");
            write_all(connfd, response, strlen(response));
        }
        break;
    case 201:
        sprintf(response, "HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n");
        write_all(connfd, response, strlen(response));
        break;
    case 400:
        sprintf(response, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n");
        write_all(connfd, response, strlen(response));
        break;
    case 403:
        sprintf(response, "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n");
        write_all(connfd, response, strlen(response));
        break;
    case 404:
        sprintf(response, "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n");
        write_all(connfd, response, strlen(response));
        break;
    case 500:
        sprintf(response, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "
                          "22\r\n\r\nInternal Server Error\n");
        write_all(connfd, response, strlen(response));
        break;
    case 501:
        sprintf(response,
            "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n");
        write_all(connfd, response, strlen(response));
        break;
    case 505:
        sprintf(response, "HTTP/1.1 505 Version Not Supported\r\nContent-Length: 22\r\n\r\nVersion "
                          "Not Supported\n");
        write_all(connfd, response, strlen(response));
        break;
    default:
        sprintf(
            response, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s", r.content_length, buffer);
        write_all(connfd, response, strlen(response));
    }
}

int main(int argc, char *argv[]) {
    (void) argc;
    char *tmp = argv[1];
    int port = atoi(tmp);
    if (port < 1 || port > 65535) {
        fprintf(stderr, "Invalid Port\n");
        return 1;
    }
    Listener_Socket sock;
    int i = listener_init(&sock, port);
    if (i < 0) {
        fprintf(stderr, "Invalid Port\n");
        return 1;
    }
    while (1) { //server loop
        int connfd = listener_accept(&sock);
        // int bytesread = read_until(connfd,bufread,user,"\r\n\r\n");
        ssize_t bytesread = 0;
        char bufread[MAX_SIZE];
        bufread[MAX_SIZE - 1] = '\0';
        statt.code = 200;
        bytesread = read_until(connfd, bufread, MAX_SIZE - 1, "\0");
        if (bytesread == -1) {
            return -1;
        }
        int p = parser(bufread); //contains the content for put
        if (statt.code != 200 || p == -1) {
            endgame(r.content, connfd);
        } else if (strcmp(r.command, "GET ") == 0) {
            endgame(r.content, connfd);
        } else if (strcmp(r.command, "PUT ") == 0) {
            put();
            endgame(r.pcontent, connfd);
        }
        close(connfd);
    }
    return 0;
}
