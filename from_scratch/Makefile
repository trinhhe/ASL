DEBUG_FLAGS := -g
CFLAGS := -Wall -lm -Wno-sign-compare -Ofast -MMD
TEST_INPUT_easy := ../with_library/test/data/ratings_small.csv
TEST_INPUT_hard := ../measurement_utils/data_small/ratings_120user_norm.csv 
MAIN := build/main
TEST := build/test

variants =\
	-DOPTVARIANT__1\
	-DOPTVARIANT__2\
	-DOPTVARIANT__3\
	-DOPTVARIANT__3@-DGRAPH_PADDING\
	-DOPTVARIANT__7\
	-DOPTVARIANT__6@-DVEC2@-mavx2\
	-DOPTVARIANT__6@-DVEC2@-mavx2@-DGRAPH_PADDING\
	-DOPTVARIANT__6@-mavx2\
	-DOPTVARIANT__6@-mavx2@-DGRAPH_PADDING\
	-DOPTVARIANT__8@-mavx2\
	-DOPTVARIANT__8@-mavx2@-DNO_FABS\
	#-DOPTVARIANT__4\
	#-DOPTVARIANT__9\
	#-DOPTVARIANT__13@-mavx2

LOGS = $(foreach var,$(variants),test/easy@$(var).log test/hard@$(var).log)

# literal space:
s := $(subst ,, )

.PHONY: all run-debug run clean
all: run-debug

# TODO: don't loop through first variant
logs: $(LOGS)
	for var in $(variants); do \
    	diff "test/easy@-DOPTVARIANT__2.log" "test/easy@$$var.log" || echo "easy@-DOPTVARIANT__2.log and easy@$$var.log are different"; \
		diff "test/hard@-DOPTVARIANT__2.log" "test/hard@$$var.log" || echo "easy@-DOPTVARIANT__2.log and hard@$$var.log are different"; \
    done

run-debug: $(subst =,__,$(subst $(s),@,$(MAIN) $(DEBUG_FLAGS)))
	valgrind $^ $(TEST_INPUT_hard)
run: $(MAIN)
	$(MAIN) $(TEST_INPUT_hard)

run-%: build/%
	$< $(TEST_INPUT_hard)

run-debug-%: build/%
	valgrind $< $(TEST_INPUT_hard)

$(MAIN)%.d:
	@true

$(TEST)%.d:
	@true

$(MAIN)%: src/main_timer.cpp
	@mkdir -p build
	g++ $< $(CFLAGS) -o $@ $(subst __,=,$(subst @, ,$*))

$(MAIN)%.S: src/main_timer.cpp $(wildcard src/*.h)
	@mkdir -p build
	g++ -S -fverbose-asm $< $(CFLAGS) -o $@ $(subst __,=,$(subst @, ,$*))

$(TEST)%: src/test.cpp
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

-include $(wildcard build/*.d)
