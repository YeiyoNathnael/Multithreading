/*
 * sync_mutex.c
 * CSC308 Spring 2026 — Member 3
 * Same counter experiment as race_condition.c, protected with a mutex.
 *
 * 4 threads x 100,000 increments = 400,000 (matches Member 2 spec).
 * Final value is always exactly 400,000 — mutex prevents all lost updates.
 *
 * Compile: gcc -O0 -pthread -o sync_mutex sync_mutex.c
 * Run:     ./sync_mutex
 */

#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS           4
#define INCREMENTS_PER_THREAD 100000

static long            shared_counter = 0;
static pthread_mutex_t counter_mutex  = PTHREAD_MUTEX_INITIALIZER;

double elapsed_sec(struct timespec *s, struct timespec *e) {
    return (e->tv_sec - s->tv_sec) + (e->tv_nsec - s->tv_nsec) / 1e9;
}

void *increment_safe(void *arg) {
    (void)arg;
    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        pthread_mutex_lock(&counter_mutex);
        shared_counter++;          /* critical section — one thread at a time */
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    long expected = (long)NUM_THREADS * INCREMENTS_PER_THREAD;

    printf("=== Multithreaded WITH Mutex Synchronisation ===\n\n");
    printf("Threads              : %d\n", NUM_THREADS);
    printf("Increments per thread: %d\n", INCREMENTS_PER_THREAD);
    printf("Expected counter     : %ld\n", expected);
    printf("------------------------------------------------\n");

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int t = 0; t < NUM_THREADS; t++)
        pthread_create(&threads[t], NULL, increment_safe, NULL);

    for (int t = 0; t < NUM_THREADS; t++)
        pthread_join(threads[t], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    pthread_mutex_destroy(&counter_mutex);

    printf("Actual counter       : %ld\n", shared_counter);
    printf("Execution time       : %.4f seconds\n", elapsed_sec(&t0, &t1));

    if (shared_counter == expected)
        printf("\nResult: CORRECT — mutex prevented all race conditions.\n");
    else
        printf("\nResult: ERROR — unexpected mismatch.\n");

    return 0;
}
