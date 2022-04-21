DEBUG_FLAGS := 
CFLAGS := -g -Wall -lm $(DEBUG_FLAGS)
#TEST_INPUT := test/ratings1.csv
TEST_INPUT := ../libdai_impl/test_data/ratings_small.csv  
#TEST_INPUT := ../Data/ml-latest-small/ratings.csv
EXEC := build/main

all: run-debug

run-debug: $(EXEC)
	valgrind $(EXEC) $(TEST_INPUT)
run: $(EXEC)
	$(EXEC) $(TEST_INPUT)

$(EXEC): $(wildcard src/main.c src/*.c src/*.h)
	gcc $(wildcard src/*.c) -o $@ $(CFLAGS)

clean:
	rm -f build/*
