// Asgn 2: A simple HTTP server.
// By: Eugene Chou
//     Andrew Quinn
//     Brian Zhao

#include "asgn2_helper_funcs.h"
#include "connection.h"
#include "debug.h"
#include "response.h"
#include "request.h"
#include "queue.h"

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ctype.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/file.h>
#include <getopt.h>
#include <sys/types.h>

#define MAX_SIZE 1024

queue_t *job;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t auditlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t putlock = PTHREAD_MUTEX_INITIALIZER;

void handle_connection(int);
void handle_get(conn_t *);
void handle_put(conn_t *);
void handle_unsupported(conn_t *, const Request_t *);

void *thread() {
    // uintptr_t threadid = (uintptr_t)args;
    //fprintf(stderr, "I am thread %lu!\n", threadid);
    uintptr_t elem = 0;
    while (1) {
        queue_pop(job, (void **) &elem);
        handle_connection(elem);
        close(elem);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        warnx("wrong arguments: %s port_num", argv[0]);
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // default number of threads is 1
    int num_threads = 4;
    int c = 0;
    size_t port;
    char *endptr;
    size_t port_num;
    // if(strcmp(argv[1], "-t")== 0){
    //     port_pos = atoi(argv[2]);
    //     //changing number of threads to what the user wants if specified
    //     num_threads = atoi(argv[1]); //this changes the number of threads inputted from str to int
    // }
    while ((c = getopt(argc, argv, "t:")) != -1) {
        switch (c) {
        case 't':
            num_threads = atoi(optarg);
            port = (size_t) strtol(argv[3], NULL, 10);
            break;
        default:
            // Convert the port argument to an integer
            port_num = strtol(argv[optind], &endptr, 10);
            if (endptr && *endptr != '\0') {
                warnx("invalid port number: %s", argv[optind]);
                return EXIT_FAILURE;
            }
            port = (size_t) port_num;
            break;
        }
    }

    //creating the queue
    job = queue_new(num_threads);
    //init lock
    // pthread_mutex_init(&lock, NULL);
    //creating threadpool
    pthread_t thr[num_threads];
    for (int a = 0; a < num_threads; a++) {
        pthread_create(&thr[a], NULL, thread, NULL);
    }

    signal(SIGPIPE, SIG_IGN);
    Listener_Socket sock;
    listener_init(&sock, port);

    uintptr_t connfd = 0;
    while (1) {
        connfd = (uintptr_t) listener_accept(&sock);
        queue_push(job, (void *) connfd);
    }
    // pthread_mutex_destroy(&lock);
    return EXIT_SUCCESS;
}

void audit_log(const char *com, conn_t *conn, const Response_t *res) {
    pthread_mutex_lock(&auditlock);
    //getting uri and request id
    char *uri = conn_get_uri(conn);
    char *request_id = conn_get_header(conn, "Request-Id");

    //getting error code from response code
    uint16_t err = response_get_code(res);
    //if request id is not a number make it 0
    if (request_id == NULL) {
        request_id = "0";
    }
    int re = atoi(request_id);
    //printing audit log to stderr
    fprintf(stderr, "%s,%s,%d,%d\n", com, uri, err, re);
    pthread_mutex_unlock(&auditlock);
    return;
}

void handle_connection(int connfd) {
    conn_t *conn = conn_new(connfd);
    const Response_t *res = conn_parse(conn);
    if (res != NULL) {
        conn_send_response(conn, res);
    } else {
        // debug("%s", conn_str(conn));
        const Request_t *req = conn_get_request(conn);
        if (req == &REQUEST_GET) {
            handle_get(conn);
        } else if (req == &REQUEST_PUT) {
            handle_put(conn);
        } else {
            handle_unsupported(conn, req);
        }
    }
    conn_delete(&conn);
    // close(connfd);
}

void handle_get(conn_t *conn) {
    char *uri = conn_get_uri(conn);
    const char *com = "GET";
    // debug("GET request not implemented. But, we want to get %s", uri);
    const Response_t *res = NULL;
    // What are the steps in here?
    // 1. Open the file.
    // If  open it returns < 0, then use the result appropriately
    //   a. Cannot access -- use RESPONSE_FORBIDDEN
    //   b. Cannot find the file -- use RESPONSE_NOT_FOUND
    //   c. other error? -- use RESPONSE_INTERNAL_SERVER_ERROR
    // (hint: check errno for these cases)!
    pthread_mutex_lock(&lock);
    bool existed = access(uri, F_OK) == 0;
    if (!(existed)) {
        res = &RESPONSE_NOT_FOUND;
        pthread_mutex_unlock(&lock);
        audit_log(com, conn, &RESPONSE_OK);
        conn_send_response(conn, res);
        return;
    }
    int file = open(uri, O_RDONLY, 0666);
    if (file == -1) {
        if (errno == ENOENT) {
            res = &RESPONSE_NOT_FOUND;
            pthread_mutex_unlock(&lock);
            audit_log(com, conn, &RESPONSE_OK);
            conn_send_response(conn, res);
            close(file);
            return;
            //no such file or directory
            //NOT_FOUND
        } else if (errno == EPERM || errno == EACCES) {
            res = &RESPONSE_FORBIDDEN;
            pthread_mutex_unlock(&lock);
            audit_log(com, conn, &RESPONSE_OK);
            conn_send_response(conn, res);
            close(file);
            return;
            //Operation not permitted
            //FORBIDDEN
        } else {
            //internal server error
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            pthread_mutex_unlock(&lock);
            audit_log(com, conn, &RESPONSE_OK);
            conn_send_response(conn, res);
            close(file);
            return;
        }
    }
    flock(file, LOCK_SH);
    pthread_mutex_unlock(&lock);
    // 2. Get the size of the file.
    // (hint: checkout the function fstat)!

    struct stat filestat;
    if (fstat(file, &filestat) != 0) {
        // Handle the error
        res = &RESPONSE_INTERNAL_SERVER_ERROR;
        // pthread_mutex_unlock(&lock);
        audit_log(com, conn, &RESPONSE_OK);
        conn_send_response(conn, res);
        close(file);
        return;
    }
    uint64_t size = filestat.st_size;
    // Get the size of the file.

    // 3. Check if the file is a directory, because directories *will*
    // open, but are not valid.
    // (hint: checkout the macro "S_IFDIR", which you can use after you call fstat!)

    if (S_ISDIR(filestat.st_mode)) {
        // 4. Send the file
        // (hint: checkout the conn_send_file function!)
        res = &RESPONSE_FORBIDDEN;
        // pthread_mutex_unlock(&lock);
        audit_log(com, conn, &RESPONSE_OK);
        conn_send_response(conn, res);
        close(file);
        return;
    }

    res = conn_send_file(conn, file, size);
    if (res == NULL) {
        audit_log(com, conn, &RESPONSE_OK);
        //is a directory
    }
    close(file);
    return;
    // outt:
    //     // Log the outgoing error response
    //     audit_log(com, conn, &RESPONSE_OK);
    //     conn_send_response(conn, res);
    //     close(file);
}

void handle_unsupported(conn_t *conn, const Request_t *res) {
    // debug("handling unsupported request");

    // send responses
    conn_send_response(conn, &RESPONSE_NOT_IMPLEMENTED);
    const char *com = request_get_str(res);

    audit_log(com, conn, &RESPONSE_NOT_IMPLEMENTED);
}

void handle_put(conn_t *conn) {

    char *uri = conn_get_uri(conn);
    const char *com = "PUT";
    const Response_t *res = NULL;
    // debug("handling put request for %s", uri);

    // Check if file already exists before opening it.
    bool existed = access(uri, F_OK) == 0;
    // debug("%s existed? %d", uri, existed);

    // Open the file..
    pthread_mutex_lock(&putlock);
    int fd = open(uri, O_CREAT | O_WRONLY, 0600);

    if (fd < 0) {
        // debug("%s: %d", uri, errno);
        if (errno == EACCES || errno == EISDIR || errno == ENOENT) {
            res = &RESPONSE_FORBIDDEN;
            pthread_mutex_unlock(&putlock);
            goto out;
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            pthread_mutex_unlock(&putlock);
            goto out;
        }
    }

    flock(fd, LOCK_EX);
    ftruncate(fd, 0);
    pthread_mutex_unlock(&putlock);

    res = conn_recv_file(conn, fd);

    if (res == NULL && existed) {
        res = &RESPONSE_OK;
    } else if (res == NULL && !existed) {
        res = &RESPONSE_CREATED;
    }
    audit_log(com, conn, &RESPONSE_OK);
    close(fd);
    conn_send_response(conn, res);
    return;
out:
    audit_log(com, conn, &RESPONSE_OK);
    conn_send_response(conn, res);
    close(fd);
    // flock(fd,LOCK_UN);
}
