# because make doesn't like spaces and =, the cflags are encoded as follows:
# " " becomes @ and "=" becomes "__" Furthermore, we want an initial @.
# so, "-O3 -march=native" becomes "@-O3@-march__native"
interesting_cflags = @-O3 @-Ofast #@-Ofast@-march__native
# variants = from_scratch
# with_library currently not working
variants = from_scratch with_library
combinations = $(foreach cflags,$(interesting_cflags),$(foreach var,$(variants),measurements/$(var)$(cflags).csv))

all: $(combinations)

measurements/%.csv: build/%
	@mkdir -p measurements
	measurement_utils/measure_runtime.sh $< $@

build/from_scratch%:
	@mkdir -p build
	make -C from_scratch build/main$*
	ln from_scratch/build/main$* build/from_scratch$*

build/with_library%:
	@mkdir -p build
	make -C with_library build/main$*
	ln with_library/build/main$* build/with_library$*

clean:
	rm -rf build
	make -C from_scratch clean
	make -C with_library clean

distclean: clean
	rm measurements/*csv

.SECONDARY:
