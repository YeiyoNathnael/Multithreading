/*
 * race_condition.c
 * CSC308 Spring 2026 — Member 3
 * Demonstrates a classic race condition: N threads each increment
 * a shared counter INCREMENTS_PER_THREAD times WITHOUT synchronisation.
 *
 * Expected final value : NUM_THREADS * INCREMENTS_PER_THREAD
 * Actual final value   : LESS (lost updates due to race)
 *
 * Compile: gcc -O0 -pthread -o race_condition race_condition.c
 * Run:     ./race_condition
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS           8
#define INCREMENTS_PER_THREAD 1000000

/* ---- Shared counter — accessed without any lock ---- */
static long shared_counter = 0;

void *increment_unsafe(void *arg) {
    (void)arg;
    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        /*
         * This is NOT atomic:
         *   1. Load  shared_counter into register
         *   2. Add 1
         *   3. Store back
         * Two threads can read the same value and both write the same
         * incremented value → one increment is silently lost.
         */
        shared_counter++;
    }
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    long expected = (long)NUM_THREADS * INCREMENTS_PER_THREAD;

    printf("=== Race Condition Demonstration ===\n");
    printf("Threads              : %d\n", NUM_THREADS);
    printf("Increments per thread: %d\n", INCREMENTS_PER_THREAD);
    printf("Expected final value : %ld\n", expected);
    printf("------------------------------------\n");

    for (int t = 0; t < NUM_THREADS; t++)
        pthread_create(&threads[t], NULL, increment_unsafe, NULL);

    for (int t = 0; t < NUM_THREADS; t++)
        pthread_join(threads[t], NULL);

    printf("Actual final value   : %ld\n", shared_counter);
    printf("Lost increments      : %ld\n", expected - shared_counter);

    if (shared_counter == expected)
        printf("Result: CORRECT (race did not manifest this run — try again)\n");
    else
        printf("Result: RACE CONDITION CONFIRMED — data was corrupted.\n");

    return 0;
}
