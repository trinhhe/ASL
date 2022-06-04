# because make doesn't like spaces and =, the cflags are encoded as follows:
# " " becomes @ and "=" becomes "__" Furthermore, we want an initial @.
# so, "-O3 -march=native" becomes "@-O3@-march__native"
interesting_cflags = @-Ofast @-Ofast@-march__native  @-Ofast@-frename-registers @-Ofast@-frename-registers@-march__native
#variants = from_scratch@-DOPTVARIANT__3 from_scratch@-DOPTVARIANT__3@-DGRAPH_PADDING from_scratch@-DOPTVARIANT__7 from_scratch@-DOPTVARIANT__6@-DVEC2@-mavx2 from_scratch@-DOPTVARIANT__6@-DVEC2@-mavx2@-DGRAPH_PADDING from_scratch@-DOPTVARIANT__8@-mavx2@-DOLD_FLOP_COUNT
variants = from_scratch@-DOPTVARIANT__3 from_scratch@-DOPTVARIANT__7 from_scratch@-DOPTVARIANT__6@-DVEC2@-mavx2 from_scratch@-DOPTVARIANT__6@-DVEC2@-mavx2@-DGRAPH_PADDING from_scratch@-DOPTVARIANT__8@-mavx2 from_scratch@-DOPTVARIANT__8@-mavx2@-DNO_FABS from_scratch@-DOPTVARIANT__11 with_library
# variants = from_scratch@-DOPTVARIANT__6@-mavx2 from_scratch@-DOPTVARIANT__6@-mavx2@-DVEC2 from_scratch@-DOPTVARIANT__6@-mavx2@-DVEC2@-DGRAPH_PADDING from_scratch@-DOPTVARIANT__6@-mavx2@-DGRAPH_PADDING
# variants = from_scratch@-DOPTVARIANT__8@-mavx2 from_scratch@-DOPTVARIANT__13@-mavx2
combinations = $(foreach cflags,$(interesting_cflags),$(foreach var,$(variants),measurements/$(var)$(cflags).csv))

all: plot

plot: measurement_utils/plot_perf.py $(combinations)
	$<

measurements/%.csv: build/% measurement_utils/measure_runtime.sh
	@mkdir -p measurements
	measurement_utils/measure_runtime.sh $< $@ $(EXTRA_FLAGS)

build/from_scratch%: .FORCE
	@mkdir -p build
	make -C from_scratch build/main$*
	ln -f from_scratch/build/main$* build/from_scratch$*

build/with_library%: .FORCE
	@mkdir -p build
	make -C with_library build/main$*
	ln -f with_library/build/main$* build/with_library$*

clean:
	rm -rf build
	make -C from_scratch clean
	make -C with_library clean

distclean: clean
	rm measurements/*csv

.SECONDARY:
.FORCE:
.PHONY: .FORCE
