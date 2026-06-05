/*
 * multithreaded.c
 * CSC308 Spring 2026 — Member 3
 * Multithreaded matrix multiplication using POSIX threads.
 * Each thread computes a horizontal band of output rows.
 *
 * Compile: gcc -O0 -pthread -o multithreaded multithreaded.c
 * Run:     ./multithreaded [num_threads]   (default = 4)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define SIZE        512
#define MAX_THREADS  16

/* Shared matrices — read by all threads; C written with no overlap */
static double A[SIZE][SIZE];
static double B[SIZE][SIZE];
static double C[SIZE][SIZE];

/* Argument passed to each worker thread */
typedef struct {
    int thread_id;
    int row_start;   /* inclusive */
    int row_end;     /* exclusive */
    int num_threads;
} thread_arg_t;

/* ------------------------------------------------------------------ */
/*  Worker: computes rows [row_start, row_end)                         */
/* ------------------------------------------------------------------ */
void *worker(void *arg) {
    thread_arg_t *a = (thread_arg_t *)arg;
    for (int i = a->row_start; i < a->row_end; i++)
        for (int k = 0; k < SIZE; k++)
            for (int j = 0; j < SIZE; j++)
                C[i][j] += A[i][k] * B[k][j];
    pthread_exit(NULL);
}

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */
void init_matrix(double mat[SIZE][SIZE], int seed) {
    srand(seed);
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            mat[i][j] = (double)(rand() % 100) / 10.0;
}

double elapsed_sec(struct timespec *s, struct timespec *e) {
    return (e->tv_sec  - s->tv_sec) +
           (e->tv_nsec - s->tv_nsec) / 1e9;
}

/* ------------------------------------------------------------------ */
/*  main                                                               */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[]) {
    int num_threads = 4;
    if (argc > 1) {
        num_threads = atoi(argv[1]);
        if (num_threads < 1 || num_threads > MAX_THREADS) {
            fprintf(stderr, "num_threads must be 1-%d\n", MAX_THREADS);
            return 1;
        }
    }

    printf("=== Multithreaded Matrix Multiplication (%dx%d) | threads=%d ===\n",
           SIZE, SIZE, num_threads);

    init_matrix(A, 42);   /* same seeds as sequential → identical results */
    init_matrix(B, 99);
    memset(C, 0, sizeof(C));

    /* Create thread handles and argument structs */
    pthread_t       threads[MAX_THREADS];
    thread_arg_t    args[MAX_THREADS];

    int rows_per_thread = SIZE / num_threads;
    int leftover        = SIZE % num_threads;

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    int current_row = 0;
    for (int t = 0; t < num_threads; t++) {
        args[t].thread_id   = t;
        args[t].num_threads = num_threads;
        args[t].row_start   = current_row;
        args[t].row_end     = current_row + rows_per_thread + (t < leftover ? 1 : 0);
        current_row         = args[t].row_end;

        if (pthread_create(&threads[t], NULL, worker, &args[t]) != 0) {
            perror("pthread_create");
            return 1;
        }
        printf("  Thread %d launched → rows [%d, %d)\n",
               t, args[t].row_start, args[t].row_end);
    }

    /* Join all threads */
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
        printf("  Thread %d joined.\n", t);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);

    double elapsed = elapsed_sec(&t0, &t1);
    printf("Execution time  : %.4f seconds\n", elapsed);
    printf("C[0][0]         : %.4f  (must match sequential)\n", C[0][0]);
    printf("C[255][255]     : %.4f\n", C[255][255]);

    return 0;
}
