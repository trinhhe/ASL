CFLAGS := -g -Wall -lm
TEST_INPUT := ../libdai_impl/test_data/ratings_small.csv  
EXEC := build/main

all: run-debug

run-debug:
	valgrind $(EXEC) $(TEST_INPUT)
run:
	$(EXEC) $(TEST_INPUT)

$(EXEC): $(wildcard src/main.c src/*.c src/*.h)
	gcc $(wildcard src/*.c) -o $@ $(CFLAGS)
