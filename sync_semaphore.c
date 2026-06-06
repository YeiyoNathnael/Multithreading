/*
 * sync_semaphore.c
 * CSC308 Spring 2026 — Member 3
 * Same counter experiment using a POSIX binary semaphore instead of a mutex.
 *
 * 4 threads x 100,000 increments = 400,000 (matches Member 2 spec).
 * sem_wait() / sem_post() guard the critical section exactly like a mutex.
 *
 * Compile: gcc -O0 -pthread -o sync_semaphore sync_semaphore.c
 * Run:     ./sync_semaphore
 */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define NUM_THREADS           4
#define INCREMENTS_PER_THREAD 100000

static long shared_counter = 0;
static sem_t counter_sem;

double elapsed_sec(struct timespec *s, struct timespec *e) {
    return (e->tv_sec - s->tv_sec) + (e->tv_nsec - s->tv_nsec) / 1e9;
}

void *increment_sem(void *arg) {
    (void)arg;
    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        sem_wait(&counter_sem);    /* enter critical section */
        shared_counter++;
        sem_post(&counter_sem);    /* leave critical section */
    }
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    long expected = (long)NUM_THREADS * INCREMENTS_PER_THREAD;

    printf("=== Multithreaded WITH Semaphore Synchronisation ===\n\n");
    printf("Threads              : %d\n", NUM_THREADS);
    printf("Increments per thread: %d\n", INCREMENTS_PER_THREAD);
    printf("Expected counter     : %ld\n", expected);
    printf("----------------------------------------------------\n");

    sem_init(&counter_sem, 0, 1);  /* binary semaphore — max 1 thread inside */

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int t = 0; t < NUM_THREADS; t++)
        pthread_create(&threads[t], NULL, increment_sem, NULL);

    for (int t = 0; t < NUM_THREADS; t++)
        pthread_join(threads[t], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    sem_destroy(&counter_sem);

    printf("Actual counter       : %ld\n", shared_counter);
    printf("Execution time       : %.4f seconds\n", elapsed_sec(&t0, &t1));

    if (shared_counter == expected)
        printf("\nResult: CORRECT — semaphore prevented all race conditions.\n");
    else
        printf("\nResult: ERROR — unexpected mismatch.\n");

    return 0;
}
