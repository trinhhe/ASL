# Mapping input size to level in memory hierarchy of Intel(R) Core(TM) i5-7200U

981, 24456, 23.88KB --> L1
1879, 59512, 58.12KB --> L2
2442, 84048, 82.08KB --> L2
3128, 141408
4142, 217680, 212.58 --> L2
4847, 285016, 278.34KB --> L3
5167, 335880
5879, 445384, 434.95KB --> L3
6452, 569440
7272, 733104
7766, 861360, 841.17KB --> L3
8522, 1002592
9278, 1188768
10334, 1418480, 1.35MB --> L3
45729, 32512088, 31MB --> Memory
...
221588, 333261152, 317.82MB --> Memory

Calculated with base 1024 (the border between L3 and Memory should not change if using 1000)