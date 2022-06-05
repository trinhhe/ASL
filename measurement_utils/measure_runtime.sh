#!/bin/sh
## Usage e.g. ' measurement_utils/measure_runtime.sh build/from_scratch@-O3 measurements/from_scratch@-O3.csv '
## … or run make measurements/from_scratch@-O3.csv, which calls this script under the hood
# $1: first argument: the executable to run
# $2: second argument, output file name
# $3: optional: testcase name, by default "small"
prog="$1"
out="$2"
name="small"
[ -n "$3" ] && name="$3"

echo "n (number of vertices), total_cycle, total_flops, gbuild_cycle, prop_cycle, bel_cycle, gbuild_flops, prop_flops, bel_flops, iterations, old_prop_flops, old_bel_flops, old_total_flops, lowest_databytes, random_databytes" > "$out".tmp
inputs_base="$(dirname "$0")"/data_"$name"
echo $inputs_base
files=$inputs_base/*norm.csv

for testfile in $files; do
	echo $testfile > /dev/stderr
    "$prog" "$testfile" >> "$out".tmp
done
mv "$out".tmp "$out"
