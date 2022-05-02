DEBUG_FLAGS := -g
CFLAGS := -I libdai/include -L libdai/lib -ldai -lgmpxx -lgmp -Wall -lm 
TEST_INPUT := test/data/ratings_small.csv  
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

$(EXEC): $(wildcard src/*.c src/*.h libdai/lib/libdai.a)
	@mkdir -p build
	g++ $(SRC) $(CFLAGS) -o $@

$(EXEC)%: $(wildcard src/*.c src/*.h libdai/lib/libdai.a)
	@mkdir -p build
	g++ $(SRC) $(CFLAGS) -o $@ $(subst __,=,$(subst @, ,$*))

clean:
	rm -f build/*
