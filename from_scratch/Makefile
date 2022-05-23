DEBUG_FLAGS := -g
CFLAGS := -Wall -lm -Wno-sign-compare
#TEST_INPUT := test/ratings1.csv
TEST_INPUT := ../with_library/test/data/ratings_small.csv  
#TEST_INPUT := ../Data/ml-latest-small/ratings.csv
EXEC := build/main
SRC := src/main_timer.c

# literal space:
s := $(subst ,, )

.PHONY: all run-debug run clean
all: run-debug

run-debug: $(subst =,__,$(subst $(s),@,$(EXEC) $(DEBUG_FLAGS)))
	valgrind $^ $(TEST_INPUT)
run: $(EXEC)
	$(EXEC) $(TEST_INPUT)

run-%: build/%
	$< $(TEST_INPUT)

run-debug-%: build/%
	valgrind $< $(TEST_INPUT)

$(EXEC)%: $(wildcard src/*.c src/*.h)
	@mkdir -p build
	g++ $(SRC) $(CFLAGS) -o $@ $(subst __,=,$(subst @, ,$*))

clean:
	rm -f build/*
