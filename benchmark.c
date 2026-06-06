/*
 * benchmark.c
 * CSC308 Spring 2026 — Member 3
 * Performance comparison across all four versions described in Member 2:
 *   1. Sequential
 *   2. Multithreaded WITHOUT synchronisation (unsafe — race condition)
 *   3. Multithreaded WITH mutex
 *   4. Multithreaded WITH semaphore
 *
 * Counter: 4 threads x 100,000 = 400,000 expected (matches Member 2 spec).
 * Also runs the matrix multiply for execution time comparison.
 *
 * Compile: gcc -O0 -pthread -o benchmark benchmark.c
 * Run:     ./benchmark
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

/* ── Matrix multiply setup (same seeds as other files) ── */
#define SIZE        512
#define NUM_THREADS   4
#define ITERS       100000

static double A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE];

/* ── Shared state for counter tests ── */
static volatile long unsafe_counter = 0;
static long          mutex_counter  = 0;
static long          sem_counter    = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static sem_t           sem;

/* ── Timer ── */
double elapsed_sec(struct timespec *s, struct timespec *e) {
    return (e->tv_sec - s->tv_sec) + (e->tv_nsec - s->tv_nsec) / 1e9;
}

/* ── Matrix helpers ── */
void init_matrix(double mat[SIZE][SIZE], int seed) {
    srand(seed);
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            mat[i][j] = (double)(rand() % 100) / 10.0;
}

/* ── Counter thread functions ── */
void *unsafe_inc(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERS; i++) { long t = unsafe_counter; t++; unsafe_counter = t; }
    return NULL;
}
void *mutex_inc(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERS; i++) {
        pthread_mutex_lock(&mtx); mutex_counter++; pthread_mutex_unlock(&mtx);
    }
    return NULL;
}
void *sem_inc(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERS; i++) {
        sem_wait(&sem); sem_counter++; sem_post(&sem);
    }
    return NULL;
}

/* ── Matrix worker ── */
typedef struct { int row_start, row_end; } marg_t;
void *mat_worker(void *arg) {
    marg_t *a = (marg_t *)arg;
    for (int i = a->row_start; i < a->row_end; i++)
        for (int k = 0; k < SIZE; k++)
            for (int j = 0; j < SIZE; j++)
                C[i][j] += A[i][k] * B[k][j];
    return NULL;
}

double run_counter(void *(*fn)(void*), long *result, long reset_val) {
    *(volatile long *)result = reset_val;
    pthread_t t[NUM_THREADS];
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < NUM_THREADS; i++) pthread_create(&t[i], NULL, fn, NULL);
    for (int i = 0; i < NUM_THREADS; i++) pthread_join(t[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    return elapsed_sec(&t0, &t1);
}

double run_sequential_matrix(void) {
    init_matrix(A, 42); init_matrix(B, 99); memset(C, 0, sizeof(C));
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < SIZE; i++)
        for (int k = 0; k < SIZE; k++)
            for (int j = 0; j < SIZE; j++)
                C[i][j] += A[i][k] * B[k][j];
    clock_gettime(CLOCK_MONOTONIC, &t1);
    return elapsed_sec(&t0, &t1);
}

double run_multithreaded_matrix(void) {
    init_matrix(A, 42); init_matrix(B, 99); memset(C, 0, sizeof(C));
    pthread_t t[NUM_THREADS]; marg_t args[NUM_THREADS];
    int rpt = SIZE / NUM_THREADS;
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].row_start = i * rpt;
        args[i].row_end   = (i == NUM_THREADS-1) ? SIZE : args[i].row_start + rpt;
        pthread_create(&t[i], NULL, mat_worker, &args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) pthread_join(t[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    return elapsed_sec(&t0, &t1);
}

int main(void) {
    long expected = (long)NUM_THREADS * ITERS;
    sem_init(&sem, 0, 1);

    printf("=== Performance Benchmark ===\n");
    printf("Counter: %d threads x %d increments = %ld expected\n\n",
           NUM_THREADS, ITERS, expected);

    /* ── Counter tests ── */
    double t_unsafe = run_counter(unsafe_inc,  (long*)&unsafe_counter, 0);
    double t_mutex  = run_counter(mutex_inc,   &mutex_counter, 0);
    double t_sem    = run_counter(sem_inc,      &sem_counter,   0);

    /* ── Matrix tests ── */
    double t_seq = run_sequential_matrix();
    double t_mt  = run_multithreaded_matrix();

    sem_destroy(&sem);
    pthread_mutex_destroy(&mtx);

    /* ── Counter results ── */
    printf("--- Counter Test (Expected: %ld) ---\n", expected);
    printf("%-38s %-12s %-12s %-10s\n", "Version", "Time (s)", "Counter", "Correct?");
    printf("%-38s %-12s %-12s %-10s\n", "-------", "--------", "-------", "--------");
    printf("%-38s %-12.4f %-12ld %-10s\n", "Sequential (single thread)",
           t_seq, expected, "Yes");
    printf("%-38s %-12.4f %-12ld %-10s\n", "Multithreaded NO sync (race)",
           t_unsafe, unsafe_counter, unsafe_counter == expected ? "Yes" : "NO - RACE");
    printf("%-38s %-12.4f %-12ld %-10s\n", "Multithreaded + Mutex",
           t_mutex, mutex_counter, mutex_counter == expected ? "Yes" : "NO");
    printf("%-38s %-12.4f %-12ld %-10s\n", "Multithreaded + Semaphore",
           t_sem, sem_counter, sem_counter == expected ? "Yes" : "NO");

    /* ── Matrix results ── */
    printf("\n--- Matrix Multiply Test (%dx%d, %d threads) ---\n", SIZE, SIZE, NUM_THREADS);
    printf("%-38s %-12s\n", "Version", "Time (s)");
    printf("%-38s %-12s\n", "-------", "--------");
    printf("%-38s %-12.4f\n", "Sequential", t_seq);
    printf("%-38s %-12.4f\n", "Multithreaded (4 threads)", t_mt);
    printf("%-38s %-12.2fx\n", "Speedup", t_seq / t_mt);

    return 0;
}
