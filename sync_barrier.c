/*
 * sync_barrier.c
 * CSC308 Spring 2026 — Member 3
 * Barrier synchronisation demonstration.
 * 4 threads work in phases — no thread moves to the next phase
 * until ALL threads have finished the current one.
 *
 * Compile: gcc -O0 -pthread -o sync_barrier sync_barrier.c
 * Run:     ./sync_barrier
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS  4
#define NUM_PHASES   3
#define WORK_UNITS   100000   /* per thread per phase — keeps totals at 400,000 */

static pthread_barrier_t barrier;
static pthread_mutex_t   print_lock = PTHREAD_MUTEX_INITIALIZER;

/* Per-thread partial results accumulated across phases */
static long partial[NUM_THREADS];

double elapsed_sec(struct timespec *s, struct timespec *e) {
    return (e->tv_sec - s->tv_sec) + (e->tv_nsec - s->tv_nsec) / 1e9;
}

void *worker(void *arg) {
    int id = *(int *)arg;

    for (int phase = 0; phase < NUM_PHASES; phase++) {

        /* ── Phase work: each thread counts WORK_UNITS increments ── */
        for (int i = 0; i < WORK_UNITS; i++)
            partial[id]++;

        pthread_mutex_lock(&print_lock);
        printf("  Thread %d finished phase %d  (partial = %ld)\n",
               id, phase + 1, partial[id]);
        pthread_mutex_unlock(&print_lock);

        /* ── Barrier: wait for ALL threads before next phase ── */
        int rc = pthread_barrier_wait(&barrier);
        if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            fprintf(stderr, "barrier_wait error\n");
            pthread_exit(NULL);
        }

        /* Only print the phase-complete message once (the serial thread) */
        if (rc == PTHREAD_BARRIER_SERIAL_THREAD) {
            long phase_total = 0;
            for (int t = 0; t < NUM_THREADS; t++) phase_total += partial[t];
            printf("  >>> All threads cleared phase %d. Combined total so far: %ld\n\n",
                   phase + 1, phase_total);
        }

        /* Second barrier so the serial-thread print finishes before next phase */
        pthread_barrier_wait(&barrier);
    }
    return NULL;
}

int main(void) {
    long expected = (long)NUM_THREADS * NUM_PHASES * WORK_UNITS;

    printf("=== Barrier Synchronisation Demo ===\n\n");
    printf("Threads    : %d\n", NUM_THREADS);
    printf("Phases     : %d\n", NUM_PHASES);
    printf("Work/thread/phase : %d\n", WORK_UNITS);
    printf("Expected total    : %ld\n", expected);
    printf("-------------------------------------\n\n");

    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1);

    long final_total = 0;
    for (int i = 0; i < NUM_THREADS; i++) final_total += partial[i];

    printf("Final combined total : %ld\n", final_total);
    printf("Execution time       : %.4f seconds\n", elapsed_sec(&t0, &t1));

    if (final_total == expected)
        printf("\nResult: CORRECT — barrier kept all phases synchronised.\n");
    else
        printf("\nResult: ERROR — total mismatch.\n");

    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&print_lock);
    return 0;
}
