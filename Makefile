CC := gcc
CFLAGS := -std=c11 -O2 -Wall -Wextra -Iinclude

SRC := src/deck.c src/evaluator.c src/ranges.c src/equity.c
OBJ := $(SRC:.c=.o)

.PHONY: all test clean

all: poker_equity

poker_equity: $(OBJ) src/main.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

run_tests: $(OBJ) tests/test_evaluator.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

test: run_tests
	./run_tests

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) src/main.o tests/test_evaluator.o poker_equity run_tests
