/*
 * multithreaded.c
 * CSC308 Spring 2026 — Member 3
 * Multithreaded matrix multiplication using POSIX pthreads.
 * Uses 4 threads (as per project spec), each handling a band of rows.
 * No synchronisation needed — threads write to non-overlapping rows.
 *
 * Compile: gcc -O0 -pthread -o multithreaded multithreaded.c
 * Run:     ./multithreaded
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define SIZE        512
#define NUM_THREADS   4   /* 4 threads as per project specification */

static double A[SIZE][SIZE];
static double B[SIZE][SIZE];
static double C[SIZE][SIZE];

typedef struct {
    int thread_id;
    int row_start;
    int row_end;
} thread_arg_t;

void *worker(void *arg) {
    thread_arg_t *a = (thread_arg_t *)arg;
    for (int i = a->row_start; i < a->row_end; i++)
        for (int k = 0; k < SIZE; k++)
            for (int j = 0; j < SIZE; j++)
                C[i][j] += A[i][k] * B[k][j];
    pthread_exit(NULL);
}

void init_matrix(double mat[SIZE][SIZE], int seed) {
    srand(seed);
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            mat[i][j] = (double)(rand() % 100) / 10.0;
}

double elapsed_sec(struct timespec *s, struct timespec *e) {
    return (e->tv_sec - s->tv_sec) + (e->tv_nsec - s->tv_nsec) / 1e9;
}

int main(void) {
    printf("=== Multithreaded Execution (%d threads) ===\n", NUM_THREADS);
    printf("Multiple tasks overlap, multiple cores engaged.\n\n");

    init_matrix(A, 42);
    init_matrix(B, 99);
    memset(C, 0, sizeof(C));

    pthread_t    threads[NUM_THREADS];
    thread_arg_t args[NUM_THREADS];
    int rows_per_thread = SIZE / NUM_THREADS;

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int t = 0; t < NUM_THREADS; t++) {
        args[t].thread_id = t;
        args[t].row_start = t * rows_per_thread;
        args[t].row_end   = (t == NUM_THREADS - 1) ? SIZE : args[t].row_start + rows_per_thread;
        pthread_create(&threads[t], NULL, worker, &args[t]);
        printf("  Thread %d launched -> rows [%d, %d)\n",
               t, args[t].row_start, args[t].row_end);
    }

    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
        printf("  Thread %d joined.\n", t);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);

    printf("\nMatrix size    : %dx%d\n", SIZE, SIZE);
    printf("Threads used   : %d\n", NUM_THREADS);
    printf("Execution time : %.4f seconds\n", elapsed_sec(&t0, &t1));
    printf("C[0][0]        : %.4f  (must match sequential checksum)\n", C[0][0]);

    return 0;
}
