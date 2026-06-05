/*
 * sequential.c
 * CSC308 Spring 2026 — Member 3
 * Sequential matrix multiplication for performance baseline.
 * Compile: gcc -O0 -o sequential sequential.c
 * Run:     ./sequential
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define SIZE 512   /* matrix dimension */

static double A[SIZE][SIZE];
static double B[SIZE][SIZE];
static double C[SIZE][SIZE];

/* Fill matrix with deterministic pseudo-random values */
void init_matrix(double mat[SIZE][SIZE], int seed) {
    srand(seed);
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            mat[i][j] = (double)(rand() % 100) / 10.0;
}

/* Sequential O(n^3) multiply */
void matrix_multiply_seq(void) {
    memset(C, 0, sizeof(C));
    for (int i = 0; i < SIZE; i++)
        for (int k = 0; k < SIZE; k++)
            for (int j = 0; j < SIZE; j++)
                C[i][j] += A[i][k] * B[k][j];
}

/* Wall-clock helper — returns seconds */
double elapsed_sec(struct timespec *start, struct timespec *end) {
    return (end->tv_sec  - start->tv_sec) +
           (end->tv_nsec - start->tv_nsec) / 1e9;
}

int main(void) {
    printf("=== Sequential Matrix Multiplication (%dx%d) ===\n", SIZE, SIZE);

    init_matrix(A, 42);
    init_matrix(B, 99);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    matrix_multiply_seq();

    clock_gettime(CLOCK_MONOTONIC, &t1);

    double elapsed = elapsed_sec(&t0, &t1);
    printf("Execution time : %.4f seconds\n", elapsed);
    printf("C[0][0]        : %.4f  (checksum — must match multithreaded)\n", C[0][0]);
    printf("C[255][255]    : %.4f\n", C[255][255]);

    return 0;
}
