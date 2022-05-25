***Top-N Recommendation through Belief Propagation***

[Link](https://grouplens.org/datasets/movielens/) to the MovieLens data set.


# Optimizations

## Ideas
- [x] data structures for more sequential access
- [x] loop unrolling and scalar replacement (only for j-loop so far)
- [x] saving pointer dereferencing
- [x] precompute message products
- [ ] log adds instead of multiplications
- [ ] read input data more efficiently
- [ ] vector instructions
- [x] find a way to make algorithm more efficient
- [ ] compact messages/node potentials/beliefs (storing only single state)
- [ ] unroll i-loop (?)
- [ ] strength reduction (?)
- [ ] function inlining (?)
- [ ] compiler flags (?)
- [ ] sequential access for out messages (?)

## Implementations

### Example
- File name
- What changed
- Reasoning, why / hypothesis 
- did it work?

### Unrolled j-loop in belief
- belief_simpleUnroll.h
- unrolled the j loop with a factor of 4 and continued to use scalar replacement
- hopefully increases ilp, also preparing step for vector instructions
- worked, by far not as good as belief3.h but runs faster (only measured inside VM!)

### Precomputing message products
- belief3.h
- insteaf of computing almost the same product for each j, precompute one global product for all j and then tweak it for each j separately, taking just 1 division per j
- changes the time complexity from O(n · maxdeg²) to O(n + m)
- it worked, increased "logical" ILP (using the old flop counts) by 30 for the largest input, which roughly corresponds to average degree

### Unrolled j-loop in belief + Precomputing message products
- belief4.h
- combined belief_simpleUnroll.h and belief3.h
- unrolling j-loop with precomputed msg products should increase ilp
- Did not work, runtime is bounded by divisions, not enough registers or "it's also interesting that -DOPTVARIANT=5 + -O3 is way faster than -DOPTVARIANT=3 + -O3, but the opposite is true with -Ofast, which suggests some -ffast-math flop reordering (i.e. some algebraic simplification) is taking place with -Ofast but not O3 and loop unrolling just messes with it?" ????

### Padding the graph
- `#ifdef GRAPH_PADDING` in factor.h
- pad the graph so that each vertex starts on an address divisible by SIMD size
- should not incur a performance penalty (or a negligible one), paves the way for vectorisation
- originally, incurred some performance penalty because of repeated zeroing, but the zeroing was unnecessary. now comparable with unpadded graph.

### Vector instructions
- belief3_vector.h (on basis of belief3_unrolled.h)
- use the optimized belief3.h unrolle the j-loop like in belief_simpleUnroll.h and use it for vector instructions
- vector instructions should make flops run more efficiently
- just started working on it

### Compact messages/node potentials/beliefs
- belief5.h (on basis of belief3.h & uses factor.h with -DCOMPACT_MESSAGE)
- G.in and G.in_old only store single float_t for the LIKE state instead of msg_t of two float_t
- increase memory throughput
- yes, it increases flops/cycle slightly 
