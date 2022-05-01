#!/bin/sh
## Usage e.g. ' ./measure_runtime.sh "-Ofast" "scratch__Ofast.csv" '
# $1: first argument, additional compiler flags e.g. '-O3 -ffast-math'
# $2: second argument, output file name
gcc ../from_scratch/src/main_timer.c -o main_timer -g -Wall -lm "$1"
echo "n (number of vertices), total_cycle, total_flops, gbuild_cycle, prop_cycle, bel_cycle, gbuild_flops, prop_flops, bel_flops" > "$2"
for testfile in ../from_scratch/test/*norm.csv; do
    ./main_timer "$testfile" >> "$2" 
done
rm main_timer