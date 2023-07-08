/************************************************************************************************************************
 Raymond Huang
 asgn3 queue
 1/26/2023
 CSE130 Sec.2 Veenstra
************************************************************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <semaphore.h>

typedef struct queue {
    int size; //the maximum elements the queue can have
    int head;
    int tail;
    int amount; //the number of elements in the queue
    void **buffer;
    sem_t empty;
    sem_t full;
    pthread_mutex_t lock;
} queue_t;

queue_t *queue_new(int size) {
    queue_t *queue = (queue_t *) malloc(sizeof(queue_t));
    queue->buffer = malloc(size * sizeof(void *));

    queue->size = size;
    queue->amount = 0;
    queue->head = 0;
    queue->tail = 0;

    sem_init(&queue->empty, 0, 0);
    sem_init(&queue->full, 0, size);
    pthread_mutex_init(&queue->lock, NULL);
    return queue;
}

void queue_delete(queue_t **q) {
    free((*q)->buffer);
    sem_destroy(&((*q)->empty));
    sem_destroy(&((*q)->full));
    pthread_mutex_destroy(&((*q)->lock));
    free(*q);
    *q = NULL;
}

bool queue_push(queue_t *q, void *elem) {
    sem_wait(&q->empty);
    pthread_mutex_lock(&q->lock);
    q->buffer[q->tail] = elem;
    q->tail = (q->tail + 1) % q->size;
    q->amount++;
    pthread_mutex_unlock(&q->lock);
    sem_post(&q->full);
    return true;
}

bool queue_pop(queue_t *q, void **elem) {
    sem_wait(&q->full);
    pthread_mutex_lock(&q->lock);
    *elem = q->buffer[q->head];
    q->head = (q->head + 1) % q->size;
    q->amount--;
    pthread_mutex_unlock(&q->lock);
    sem_post(&q->empty);
    return true;
}
