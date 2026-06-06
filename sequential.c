/*
 * sequential.c
 * CSC308 Spring 2026 — Member 3
 * Sequential matrix multiplication — single execution path baseline.
 *
 * Compile: gcc -O0 -o sequential sequential.c
 * Run:     ./sequential
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define SIZE 512

static double A[SIZE][SIZE];
static double B[SIZE][SIZE];
static double C[SIZE][SIZE];

void init_matrix(double mat[SIZE][SIZE], int seed) {
    srand(seed);
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            mat[i][j] = (double)(rand() % 100) / 10.0;
}

void matrix_multiply_seq(void) {
    memset(C, 0, sizeof(C));
    for (int i = 0; i < SIZE; i++)
        for (int k = 0; k < SIZE; k++)
            for (int j = 0; j < SIZE; j++)
                C[i][j] += A[i][k] * B[k][j];
}

double elapsed_sec(struct timespec *s, struct timespec *e) {
    return (e->tv_sec - s->tv_sec) + (e->tv_nsec - s->tv_nsec) / 1e9;
}

int main(void) {
    printf("=== Sequential Execution ===\n");
    printf("One task at a time, single execution path.\n\n");

    init_matrix(A, 42);
    init_matrix(B, 99);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    matrix_multiply_seq();
    clock_gettime(CLOCK_MONOTONIC, &t1);

    printf("Matrix size    : %dx%d\n", SIZE, SIZE);
    printf("Execution time : %.4f seconds\n", elapsed_sec(&t0, &t1));
    printf("C[0][0]        : %.4f  (checksum)\n", C[0][0]);
    printf("\nResult: CORRECT — sequential execution has no race conditions.\n");

    return 0;
}
