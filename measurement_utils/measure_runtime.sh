#!/bin/sh
#from_scatch
filename='from_scratch_O3.csv'
# gcc ../from_scratch/src/main_timer.c -o ../from_scratch/src/main_timer_O3 -g -Wall -lm -O3
echo "n (number of vertices), total_cycle, total_flops, gbuild_cycle, prop_cycle, bel_cycle, gbuild_flops, prop_flops, bel_flops" > $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_10user_norm.csv >> $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_20user_norm.csv >> $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_30user_norm.csv >> $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_60user_norm.csv >> $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_90user_norm.csv >> $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_120user_norm.csv >> $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_150user_norm.csv >> $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_210user_norm.csv >> $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_270user_norm.csv >> $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_330user_norm.csv >> $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_390user_norm.csv >> $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_450user_norm.csv >> $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_540user_norm.csv >> $filename
./../from_scratch/src/main_timer_O3 ../from_scratch/test/ratings_all_norm.csv >> $filename


