/*
 * sync_semaphore.c
 * CSC308 Spring 2026 — Member 3
 * Producer-Consumer pattern using POSIX semaphores.
 * One producer fills a bounded buffer; multiple consumers drain it.
 *
 * Compile: gcc -O0 -pthread -o sync_semaphore sync_semaphore.c -lrt
 * Run:     ./sync_semaphore
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE    8
#define NUM_CONSUMERS  3
#define TOTAL_ITEMS   30

/* ---- Bounded buffer ---- */
static int buffer[BUFFER_SIZE];
static int buf_in  = 0;   /* producer writes here  */
static int buf_out = 0;   /* consumer reads here   */

/* ---- Semaphores ---- */
static sem_t empty_slots;   /* counts free slots  (init = BUFFER_SIZE) */
static sem_t full_slots;    /* counts filled slots (init = 0)           */
static sem_t buf_mutex;     /* binary semaphore protecting buffer index */

/* ---- Shared state ---- */
static int produced_count  = 0;
static int consumed_count  = 0;
static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

/* ------------------------------------------------------------------ */
void *producer(void *arg) {
    (void)arg;
    for (int item = 1; item <= TOTAL_ITEMS; item++) {
        sem_wait(&empty_slots);   /* block if buffer full   */
        sem_wait(&buf_mutex);     /* enter critical section */

        buffer[buf_in] = item;
        buf_in = (buf_in + 1) % BUFFER_SIZE;
        produced_count++;

        pthread_mutex_lock(&print_lock);
        printf("[Producer]   produced item %2d  (total produced: %d)\n",
               item, produced_count);
        pthread_mutex_unlock(&print_lock);

        sem_post(&buf_mutex);     /* leave critical section */
        sem_post(&full_slots);    /* signal: one more item  */
        usleep(20000);            /* 20 ms — simulate work  */
    }
    return NULL;
}

void *consumer(void *arg) {
    int id = *(int *)arg;
    while (1) {
        sem_wait(&full_slots);    /* block if buffer empty  */
        sem_wait(&buf_mutex);

        /* Check stop condition inside lock */
        if (consumed_count >= TOTAL_ITEMS) {
            sem_post(&buf_mutex);
            sem_post(&full_slots);  /* unblock other waiting consumers */
            break;
        }

        int item = buffer[buf_out];
        buf_out = (buf_out + 1) % BUFFER_SIZE;
        consumed_count++;

        pthread_mutex_lock(&print_lock);
        printf("[Consumer %d] consumed item %2d  (total consumed: %d)\n",
               id, item, consumed_count);
        pthread_mutex_unlock(&print_lock);

        sem_post(&buf_mutex);
        sem_post(&empty_slots);   /* one slot freed         */
        usleep(50000);            /* 50 ms — simulate work  */
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
int main(void) {
    printf("=== Producer-Consumer with Semaphores ===\n");
    printf("Buffer size  : %d\n", BUFFER_SIZE);
    printf("Consumers    : %d\n", NUM_CONSUMERS);
    printf("Total items  : %d\n", TOTAL_ITEMS);
    printf("-----------------------------------------\n");

    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&full_slots,  0, 0);
    sem_init(&buf_mutex,   0, 1);

    pthread_t prod_thread;
    pthread_t cons_threads[NUM_CONSUMERS];
    int cons_ids[NUM_CONSUMERS];

    pthread_create(&prod_thread, NULL, producer, NULL);
    for (int c = 0; c < NUM_CONSUMERS; c++) {
        cons_ids[c] = c + 1;
        pthread_create(&cons_threads[c], NULL, consumer, &cons_ids[c]);
    }

    pthread_join(prod_thread, NULL);
    for (int c = 0; c < NUM_CONSUMERS; c++)
        pthread_join(cons_threads[c], NULL);

    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    sem_destroy(&buf_mutex);

    printf("-----------------------------------------\n");
    printf("Done. Produced: %d | Consumed: %d\n", produced_count, consumed_count);
    return 0;
}
