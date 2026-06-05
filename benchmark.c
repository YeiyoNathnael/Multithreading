/*
 * benchmark.c
 * CSC308 Spring 2026 — Member 3
 * Runs the matrix multiply with 1,2,4,8 threads and prints a
 * comparison table showing speedup and CPU utilisation estimate.
 *
 * Compile: gcc -O0 -pthread -o benchmark benchmark.c
 * Run:     ./benchmark
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define SIZE        512
#define MAX_THREADS  8

static double A[SIZE][SIZE];
static double B[SIZE][SIZE];
static double C[SIZE][SIZE];

typedef struct {
    int row_start;
    int row_end;
} arg_t;

void init_matrix(double mat[SIZE][SIZE], int seed) {
    srand(seed);
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            mat[i][j] = (double)(rand() % 100) / 10.0;
}

double elapsed_sec(struct timespec *s, struct timespec *e) {
    return (e->tv_sec - s->tv_sec) + (e->tv_nsec - s->tv_nsec) / 1e9;
}

void *worker(void *raw) {
    arg_t *a = (arg_t *)raw;
    for (int i = a->row_start; i < a->row_end; i++)
        for (int k = 0; k < SIZE; k++)
            for (int j = 0; j < SIZE; j++)
                C[i][j] += A[i][k] * B[k][j];
    return NULL;
}

double run(int num_threads) {
    memset(C, 0, sizeof(C));
    pthread_t threads[MAX_THREADS];
    arg_t     args[MAX_THREADS];

    int rpt      = SIZE / num_threads;
    int leftover = SIZE % num_threads;
    int cur      = 0;

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int t = 0; t < num_threads; t++) {
        args[t].row_start = cur;
        args[t].row_end   = cur + rpt + (t < leftover ? 1 : 0);
        cur               = args[t].row_end;
        pthread_create(&threads[t], NULL, worker, &args[t]);
    }
    for (int t = 0; t < num_threads; t++)
        pthread_join(threads[t], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    return elapsed_sec(&t0, &t1);
}

int main(void) {
    init_matrix(A, 42);
    init_matrix(B, 99);

    int configs[] = {1, 2, 4, 8};
    int n         = sizeof(configs) / sizeof(configs[0]);
    double times[8];

    printf("=== Matrix Multiplication Benchmark (%dx%d) ===\n\n", SIZE, SIZE);
    printf("%-10s %-14s %-10s %-14s\n",
           "Threads", "Time (s)", "Speedup", "Efficiency");
    printf("%-10s %-14s %-10s %-14s\n",
           "-------", "--------", "-------", "----------");

    double base = 0.0;
    for (int i = 0; i < n; i++) {
        times[i] = run(configs[i]);
        if (i == 0) base = times[i];
        double speedup    = base / times[i];
        double efficiency = speedup / configs[i] * 100.0;
        printf("%-10d %-14.4f %-10.2f %-14.1f%%\n",
               configs[i], times[i], speedup, efficiency);
    }

    printf("\nC[0][0] = %.4f  (sanity check — must be non-zero)\n", C[0][0]);
    return 0;
}
