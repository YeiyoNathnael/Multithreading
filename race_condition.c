/*
 * race_condition.c
 * CSC308 Spring 2026 — Member 3
 * Demonstrates a race condition on a shared counter.
 *
 * 4 threads x 100,000 increments = 400,000 expected (matches Member 2 spec).
 * Without synchronisation the actual value is almost always less due to
 * lost updates from the unsynchronised read-modify-write sequence.
 *
 * Compile: gcc -O0 -pthread -o race_condition race_condition.c
 * Run:     ./race_condition
 */

#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS           4
#define INCREMENTS_PER_THREAD 100000

volatile long shared_counter = 0;

void *increment_unsafe(void *arg) {
    (void)arg;
    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        /* NOT atomic — three separate machine instructions:
         * 1. LOAD  shared_counter into register
         * 2. ADD   1
         * 3. STORE back to memory
         * Two threads can interleave these steps, causing lost updates. */
        long tmp = shared_counter;
        tmp++;
        shared_counter = tmp;
    }
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    long expected = (long)NUM_THREADS * INCREMENTS_PER_THREAD;

    printf("=== Multithreaded WITHOUT Synchronisation (Race Condition) ===\n\n");
    printf("Threads              : %d\n", NUM_THREADS);
    printf("Increments per thread: %d\n", INCREMENTS_PER_THREAD);
    printf("Expected counter     : %ld\n", expected);
    printf("--------------------------------------------------------------\n");

    for (int t = 0; t < NUM_THREADS; t++)
        pthread_create(&threads[t], NULL, increment_unsafe, NULL);

    for (int t = 0; t < NUM_THREADS; t++)
        pthread_join(threads[t], NULL);

    printf("Actual counter       : %ld\n", shared_counter);
    printf("Lost increments      : %ld\n", expected - shared_counter);

    if (shared_counter == expected)
        printf("\nResult: No race this run — try again.\n");
    else
        printf("\nResult: RACE CONDITION CONFIRMED — data corrupted.\n");

    return 0;
}
