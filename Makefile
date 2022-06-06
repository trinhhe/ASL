# because make doesn't like spaces and =, the cflags are encoded as follows:
# " " becomes @ and "=" becomes "__" Furthermore, we want an initial @.
# so, "-O3 -march=native" becomes "@-O3@-march__native"
interesting_cflags = @-Ofast@-march__native
variants = \
	from_scratch@-DOPTVARIANT__3\
	from_scratch@-DOPTVARIANT__7\
	from_scratch@-DOPTVARIANT__6\
	from_scratch@-DOPTVARIANT__6@-DVEC2\
	from_scratch@-DOPTVARIANT__8\
	from_scratch@-DOPTVARIANT__8@-DNO_FABS\
	from_scratch@-DOPTVARIANT__11@-frename-registers\
	from_scratch@-DOPTVARIANT__9\
	from_scratch@-DOPTVARIANT__10\
	from_scratch@-DOPTVARIANT__13
variants_small_only = \
	from_scratch@-DOPTVARIANT__1\
	from_scratch@-DOPTVARIANT__2\
	from_scratch@-DOPTVARIANT__4\
	with_library
combinations_small = $(foreach cflags,$(interesting_cflags),$(foreach var,$(variants) $(variants_small_only),measurements/small/$(var)$(cflags).csv))
combinations_big = $(foreach cflags,$(interesting_cflags),$(foreach var,$(variants),measurements/big/$(var)$(cflags).csv))
combinations_compl_bipartite = $(foreach cflags,$(interesting_cflags),$(foreach var,$(variants),measurements/compl_bipartite/$(var)$(cflags).csv))

PLOT_OPTS=-n
PLOT=measurement_utils/plot_perf.py
MEASURE=true
#MEASURE=measurement_utils/measure_runtime.sh

all: plots

plots: $(combinations_small) plots/slow-comparison plots/slow-vs-fast plots/vectorisation plots/compaction-and-vectorisation plots/compaction-and-vectorisation plots/end-to-end plots/compaction-and-vectorisation-big

plots/small: $(PLOT) $(combinations_small)
	$^ $(PLOT_OPTS) -o $@

plots/big: $(PLOT) $(combinations_big)
	$^ $(PLOT_OPTS) -o $@

plots/compl_bipartite: $(PLOT) $(combinations_compl_bipartite)
	$^ $(PLOT_OPTS) -o $@

plots/slow-comparison: $(PLOT) $(combinations_small)
	$< $(PLOT_OPTS) -o $@ -r measurements/small/*_1@* measurements/small/*{__1,__2,with*}@* -p "baseline [1]" "scalar_replacement+collapse_2x2_loop [2]" library

plots/slow-vs-fast: $(PLOT) $(combinations_small)
	$< $(PLOT_OPTS) -o $@ -r measurements/small/*_2@* measurements/small/*__{2,3}@* -p "scalar_replacement+collapse_2x2_loop [2]" "precompute_products [3]"

plots/vectorisation: $(PLOT) $(combinations_small)
	$< $(PLOT_OPTS) -o $@ -r measurements/small/*_3@* measurements/small/*__{3,6@-O*,6@-DVEC2}@* -p "precompute [3]" "precompute+vectorise [6]" "precompute+vectorise2 [6 -DVEC2]"

plots/compaction-and-vectorisation: $(PLOT) $(combinations_small)
	$< $(PLOT_OPTS) -o $@ -r measurements/small/*_3@* measurements/small/*__{3,6@-DVEC2,7,8@-O*}@* -p "precompute [3]" "precompute+vectorise2 [6 -DVEC2]" "precompute+save_memory [7]" "precompute+vectorise2+save_memory [8]"

plots/compaction-and-vectorisation2: $(PLOT) $(combinations_small)
	$< $(PLOT_OPTS) -o $@ -r measurements/small/*_8@-O*@* measurements/small/*__{3,6@-DVEC2,7,8@-O*}@*

plots/compaction-and-vectorisation-big: $(PLOT) $(combinations_big)
	$< $(PLOT_OPTS) -o $@ -r measurements/small_and_big/*_3@* measurements/small_and_big/*__{3,6@-DVEC2,7,8@-O*}@* -p "precompute [3]" "precompute+vectorise2 [6 -DVEC2]" "precompute+save_memory [7]" "precompute+vectorise2+save_memory [8]"

plots/end-to-end: $(PLOT) $(combinations_small)
	$< $(PLOT_OPTS) -o $@ -r measurements/small/*_1@* measurements/small/*__{1,3,8@-O*}@* -p "baseline [1]" "precompute products [3]" "precompute+vectorise2+save_memory [8]"

measurements/small/%.csv: build/% measurement_utils/measure_runtime.sh
	@mkdir -p measurements/small
	$(MEASURE) $< $@ small

measurements/big/%.csv: build/% measurement_utils/measure_runtime.sh
	@mkdir -p measurements/big
	$(MEASURE) $< $@ big

measurements/compl_bipartite/%.csv: build/% measurement_utils/measure_runtime.sh
	@mkdir -p measurements/compl_bipartite
	$(MEASURE) $< $@ compl_bipartite

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
