DEBUG_FLAGS := -g
CFLAGS := -Wall -lm
#TEST_INPUT := test/ratings1.csv
TEST_INPUT := ../libdai_impl/test/data/ratings_small.csv  
#TEST_INPUT := ../Data/ml-latest-small/ratings.csv
EXEC := build/main
SRC := src/main_timer.c

# literal space:
s := $(subst ,, )

.PHONY: all run-debug run clean
all: run-debug

run-debug: $(subst =,::,$(subst $(s),@,$(EXEC) $(DEBUG_FLAGS)))
	valgrind $^ $(TEST_INPUT)
run: $(EXEC)
	$(EXEC) $(TEST_INPUT)

$(EXEC): $(wildcard src/*.c src/*.h)
	gcc $(SRC) $(CFLAGS) -o $@

$(EXEC)%: $(wildcard src/*.c src/*.h)
	gcc $(SRC) $(CFLAGS) -o $@ $(subst ::,=,$(subst @, ,$*))

clean:
	rm -f build/*
