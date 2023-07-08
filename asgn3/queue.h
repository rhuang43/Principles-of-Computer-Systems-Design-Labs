#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <stdint.h>

typedef struct queue queue_t;

queue_t *queue_new(int size);

void queue_delete(queue_t **q);

bool queue_push(queue_t *q, void *elem);

bool queue_pop(queue_t *q, void **elem);
