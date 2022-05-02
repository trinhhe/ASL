#!/bin/sh
## Usage e.g. ' measurement_utils/measure_runtime.sh build/from_scratch@-O3 measurements/from_scratch@-O3.csv '
## … or run make measurements/from_scratch@-O3.csv, which calls this script under the hood
# $1: first argument: the executable to run
# $2: second argument, output file name
prog="$1"
out="$2"
echo "n (number of vertices), total_cycle, total_flops, gbuild_cycle, prop_cycle, gbuild_flops, prop_flops, bel_flops" > "$out".tmp
for testfile in "$(dirname "$0")"/data/*_1*norm.csv; do
	echo $testfile > /dev/stderr
    "$prog" "$testfile" >> "$out".tmp
done
mv "$out".tmp "$out"
