***Top-N Recommendation through Belief Propagation***

[Link](https://grouplens.org/datasets/movielens/) to the MovieLens data set.


# Optimizations

## Ideas
- data structures for more sequential access
- loop unrolling and scalar replacement
- saving pointer derefferncing
- precompute message products
- log adds instead of multiplications
- read input data more efficiently
- vector instructions
- find a way to make algorithm more efficient

## Implementations

### Example
- File name
- What changed
- Reasoning, why / hypothesis 
- did it work?

### Unrolled j-loop in belief
- belief_simpleUnroll.h
- unrolled the j loop with a factor of 4 and continued to use scalar replacement
- hopefully increases ilp, also preparing step vor vector instructions
- still with bugs, get a segfault