DEBUG_FLAGS := -g
CFLAGS := -Wall -lm -Wno-sign-compare -Ofast
TEST_INPUT_easy := ../with_library/test/data/ratings_small.csv
TEST_INPUT_hard := ../measurement_utils/data/ratings_120user_norm.csv 
MAIN := build/main
TEST := build/test

variants = -DOPTVARIANT__2 -DOPTVARIANT__3 -DOPTVARIANT__3@-DGRAPH_PADDING -DOPTVARIANT__7 -DOPTVARIANT__6@-DVEC2@-mavx2 -DOPTVARIANT__6@-DVEC2@-mavx2@-DGRAPH_PADDING -DOPTVARIANT__6@-mavx2 -DOPTVARIANT__6@-mavx2@-DGRAPH_PADDING

LOGS = $(foreach var,$(variants),test/easy@$(var).log test/hard@$(var).log)

# literal space:
s := $(subst ,, )

.PHONY: all run-debug run clean
all: run-debug

logs: $(LOGS)

run-debug: $(subst =,__,$(subst $(s),@,$(MAIN) $(DEBUG_FLAGS)))
	valgrind $^ $(TEST_INPUT_hard)
run: $(MAIN)
	$(MAIN) $(TEST_INPUT_hard)

run-%: build/%
	$< $(TEST_INPUT_hard)

run-debug-%: build/%
	valgrind $< $(TEST_INPUT_hard)

$(MAIN)%: src/main_timer.cpp $(wildcard src/*.h)
	@mkdir -p build
	g++ $< $(CFLAGS) -o $@ $(subst __,=,$(subst @, ,$*))

$(MAIN)%.S: src/main_timer.cpp $(wildcard src/*.h)
	@mkdir -p build
	g++ -S -fverbose-asm $< $(CFLAGS) -o $@ $(subst __,=,$(subst @, ,$*))

$(TEST)%: src/test.cpp $(wildcard src/*.h)
	@mkdir -p build
	g++ $< $(CFLAGS) -o $@ $(subst __,=,$(subst @, ,$*))

test/easy%.log: $(TEST)%
	@mkdir -p test
	$< $(TEST_INPUT_easy) > $@

test/hard%.log: $(TEST)%
	@mkdir -p test
	$< $(TEST_INPUT_hard) > $@

clean:
	rm -f build/*
