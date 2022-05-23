***Top-N Recommendation through Belief Propagation***

[Link](https://grouplens.org/datasets/movielens/) to the MovieLens data set.


# Optimizations

## Ideas
- [x] data structures for more sequential access
- [x] loop unrolling and scalar replacement (only for j-loop so far)
- [x] saving pointer derefferncing
- [x] precompute message products
- [ ] log adds instead of multiplications
- [ ] read input data more efficiently
- [ ] vector instructions
- [x] find a way to make algorithm more efficient

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
- woked, by far not as good as belief3.h but runs faster (only measured inside VM!)

### Precomputing message products
- belief3.h
- insteaf of computing almost the same product for each j, precompute one global product for all j and then tweak it for each j separately, taking just 1 division per j
- changes the time complexity from O(n · maxdeg²) to O(n + m)
- it worked, increased "logical" ILP (using the old flop counts) by 30 for the largest input, which roughly corresponds to average degree

### Padding the graph
- `#ifdef GRAPH_PADDING` in factor.h
- pad the graph so that each vertex starts on an address divisible by SIMD size
- should not incur a performance penalty (or a negligible one), paves the way for vectorisation
- originally, incurred some performance penalty because of repeated zeroing, but the zeroing was unnecessary. now comparable with unpadded graph.

### Vector instructions
- belief3_vector.h (on basis of belief3_unrolled.h)
- use the optimized belief3.h unrolle the j-loop like in belief_simpleUnroll.h and use it for vector instructions
- vecotr instructions should make flops run more efficiently
- just started working on it
