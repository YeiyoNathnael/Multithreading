# Makefile — CSC308 Spring 2026, Part 3
# Usage:
#   make            — build everything
#   make run_all    — build and run all programs in sequence
#   make clean      — remove binaries

CC      = gcc
CFLAGS  = -O0 -Wall -Wextra -g
LDFLAGS = -pthread

TARGETS = sequential multithreaded race_condition sync_mutex sync_semaphore \
					benchmark sync_condvar sync_rwlock sync_barrier

.PHONY: all run_all clean

all: $(TARGETS)

sequential: sequential.c
	$(CC) $(CFLAGS) -o $@ $<

multithreaded: multithreaded.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

race_condition: race_condition.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

sync_mutex: sync_mutex.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

sync_semaphore: sync_semaphore.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

benchmark: benchmark.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

sync_condvar: sync_condvar.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

sync_rwlock: sync_rwlock.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

sync_barrier: sync_barrier.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

# -------------------------------------------------------
# Run all demos in order (copy terminal output for report)
# -------------------------------------------------------
run_all: all
	@echo ""
	@echo "========== 1. SEQUENTIAL =========="
	./sequential
	@echo ""
	@echo "========== 2. MULTITHREADED (4 threads) =========="
	./multithreaded
	@echo ""
	@echo "========== 3. RACE CONDITION =========="
	./race_condition
	@echo ""
	@echo "========== 4. MUTEX =========="
	./sync_mutex
	@echo ""
	@echo "========== 5. SEMAPHORE =========="
	./sync_semaphore
	@echo ""
	@echo "========== 6. BENCHMARK =========="
	./benchmark
	@echo ""
	@echo "========== 7. CONDITION VARIABLE =========="
	./sync_condvar
	@echo ""
	@echo "========== 8. READ-WRITE LOCK =========="
	./sync_rwlock
	@echo ""
	@echo "========== 9. BARRIER =========="
	./sync_barrier

clean:
	rm -f $(TARGETS)
