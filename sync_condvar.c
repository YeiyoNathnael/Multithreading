/*
 * sync_condvar.c
 * CSC308 Spring 2026 — Member 3
 * Condition Variable demonstration — Producer/Consumer pattern.
 * 4 threads x 100,000 items produced and consumed.
 *
 * A condition variable lets a thread sleep until a condition is true,
 * instead of wasting CPU in a busy-wait loop.
 *
 * Compile: gcc -O0 -pthread -o sync_condvar sync_condvar.c
 * Run:     ./sync_condvar
 */

#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS           4
#define ITEMS_PER_PRODUCER    100000

/* ── Shared state ── */
static long item_count   = 0;   /* items available in buffer     */
static long total_produced = 0;
static long total_consumed = 0;
static int  done           = 0; /* signals consumers to stop     */

static pthread_mutex_t lock  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  notempty = PTHREAD_COND_INITIALIZER; /* consumer waits on this */
static pthread_cond_t  notfull  = PTHREAD_COND_INITIALIZER; /* producer waits on this */

#define BUFFER_CAP 10  /* max items in buffer at once */

double elapsed_sec(struct timespec *s, struct timespec *e) {
    return (e->tv_sec - s->tv_sec) + (e->tv_nsec - s->tv_nsec) / 1e9;
}

void *producer(void *arg) {
    (void)arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        pthread_mutex_lock(&lock);

        /* Wait while buffer is full */
        while (item_count >= BUFFER_CAP)
            pthread_cond_wait(&notfull, &lock);

        item_count++;
        total_produced++;

        /* Signal a waiting consumer */
        pthread_cond_signal(&notempty);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void *consumer(void *arg) {
    (void)arg;
    while (1) {
        pthread_mutex_lock(&lock);

        /* Wait while buffer is empty and production not done */
        while (item_count == 0 && !done)
            pthread_cond_wait(&notempty, &lock);

        if (item_count == 0 && done) {
            pthread_mutex_unlock(&lock);
            break;
        }

        item_count--;
        total_consumed++;

        /* Signal a waiting producer */
        pthread_cond_signal(&notfull);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(void) {
    long expected = (long)NUM_THREADS * ITEMS_PER_PRODUCER;

    printf("=== Condition Variable Demo (Producer-Consumer) ===\n\n");
    printf("Producers            : %d\n", NUM_THREADS);
    printf("Items per producer   : %d\n", ITEMS_PER_PRODUCER);
    printf("Expected total items : %ld\n", expected);
    printf("Buffer capacity      : %d\n", BUFFER_CAP);
    printf("---------------------------------------------------\n");

    pthread_t prod[NUM_THREADS], cons[NUM_THREADS];

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int i = 0; i < NUM_THREADS; i++) pthread_create(&prod[i], NULL, producer, NULL);
    for (int i = 0; i < NUM_THREADS; i++) pthread_create(&cons[i], NULL, consumer, NULL);

    for (int i = 0; i < NUM_THREADS; i++) pthread_join(prod[i], NULL);

    /* Signal consumers that production is finished */
    pthread_mutex_lock(&lock);
    done = 1;
    pthread_cond_broadcast(&notempty);
    pthread_mutex_unlock(&lock);

    for (int i = 0; i < NUM_THREADS; i++) pthread_join(cons[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1);

    printf("Total produced       : %ld\n", total_produced);
    printf("Total consumed       : %ld\n", total_consumed);
    printf("Execution time       : %.4f seconds\n", elapsed_sec(&t0, &t1));

    if (total_produced == expected && total_consumed == expected)
        printf("\nResult: CORRECT — condition variable coordinated all threads.\n");
    else
        printf("\nResult: ERROR — mismatch detected.\n");

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&notempty);
    pthread_cond_destroy(&notfull);
    return 0;
}
