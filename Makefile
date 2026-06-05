# Makefile — CSC308 Spring 2026, Part 3
# Usage:
#   make            — build everything
#   make run_all    — build and run all programs in sequence
#   make clean      — remove binaries

CC      = gcc
CFLAGS  = -O0 -Wall -Wextra -g
LDFLAGS = -pthread -lrt

TARGETS = sequential multithreaded race_condition sync_mutex sync_semaphore benchmark

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
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -lrt

benchmark: benchmark.c
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
	./multithreaded 4
	@echo ""
	@echo "========== 3. RACE CONDITION =========="
	./race_condition
	@echo ""
	@echo "========== 4. MUTEX (correct sync) =========="
	./sync_mutex
	@echo ""
	@echo "========== 5. SEMAPHORE (producer-consumer) =========="
	./sync_semaphore
	@echo ""
	@echo "========== 6. BENCHMARK (1/2/4/8 threads) =========="
	./benchmark

clean:
	rm -f $(TARGETS)
