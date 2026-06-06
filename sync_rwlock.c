/*
 * sync_rwlock.c
 * CSC308 Spring 2026 — Member 3
 * Read-Write Lock demonstration.
 * 4 threads x 100,000 read operations + controlled write operations.
 *
 * Multiple readers can hold the lock simultaneously.
 * A writer gets exclusive access — all readers must finish first.
 *
 * Compile: gcc -O0 -pthread -o sync_rwlock sync_rwlock.c
 * Run:     ./sync_rwlock
 */

#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define NUM_READERS           4
#define READS_PER_THREAD      100000
#define NUM_WRITERS           1
#define WRITES_PER_THREAD     1000

static long shared_data    = 0;   /* protected shared resource    */
static long total_reads    = 0;
static long total_writes   = 0;
static pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
static pthread_mutex_t  count_lock = PTHREAD_MUTEX_INITIALIZER;

double elapsed_sec(struct timespec *s, struct timespec *e) {
    return (e->tv_sec - s->tv_sec) + (e->tv_nsec - s->tv_nsec) / 1e9;
}

void *reader(void *arg) {
    (void)arg;
    for (int i = 0; i < READS_PER_THREAD; i++) {
        pthread_rwlock_rdlock(&rwlock);   /* shared read lock — multiple allowed */
        volatile long val = shared_data;  /* read the shared value               */
        (void)val;
        pthread_rwlock_unlock(&rwlock);

        pthread_mutex_lock(&count_lock);
        total_reads++;
        pthread_mutex_unlock(&count_lock);
    }
    return NULL;
}

void *writer(void *arg) {
    (void)arg;
    for (int i = 0; i < WRITES_PER_THREAD; i++) {
        pthread_rwlock_wrlock(&rwlock);   /* exclusive write lock — one at a time */
        shared_data++;
        pthread_rwlock_unlock(&rwlock);

        pthread_mutex_lock(&count_lock);
        total_writes++;
        pthread_mutex_unlock(&count_lock);
    }
    return NULL;
}

int main(void) {
    printf("=== Read-Write Lock Demo ===\n\n");
    printf("Readers              : %d\n", NUM_READERS);
    printf("Reads per thread     : %d\n", READS_PER_THREAD);
    printf("Writers              : %d\n", NUM_WRITERS);
    printf("Writes per thread    : %d\n", WRITES_PER_THREAD);
    printf("Expected writes      : %d\n", NUM_WRITERS * WRITES_PER_THREAD);
    printf("---------------------------\n");

    pthread_t readers[NUM_READERS], writers[NUM_WRITERS];

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int i = 0; i < NUM_READERS; i++) pthread_create(&readers[i], NULL, reader, NULL);
    for (int i = 0; i < NUM_WRITERS; i++) pthread_create(&writers[i], NULL, writer, NULL);

    for (int i = 0; i < NUM_READERS; i++) pthread_join(readers[i], NULL);
    for (int i = 0; i < NUM_WRITERS; i++) pthread_join(writers[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1);

    printf("Total reads done     : %ld\n", total_reads);
    printf("Total writes done    : %ld\n", total_writes);
    printf("Final shared_data    : %ld  (must equal total writes)\n", shared_data);
    printf("Execution time       : %.4f seconds\n", elapsed_sec(&t0, &t1));

    if (shared_data == (long)(NUM_WRITERS * WRITES_PER_THREAD))
        printf("\nResult: CORRECT — read-write lock protected shared data.\n");
    else
        printf("\nResult: ERROR — shared_data mismatch.\n");

    pthread_rwlock_destroy(&rwlock);
    pthread_mutex_destroy(&count_lock);
    return 0;
}
