#!/bin/sh
## Usage e.g. ' measurement_utils/measure_runtime.sh build/from_scratch@-O3 measurements/from_scratch@-O3.csv '
## … or run make measurements/from_scratch@-O3.csv, which calls this script under the hood
# $1: first argument: the executable to run
# $2: second argument, output file name
# $3: optional: if -q, runs just a few tests
prog="$1"
out="$2"
echo "n (number of vertices), total_cycle, total_flops, gbuild_cycle, prop_cycle, bel_cycle, gbuild_flops, prop_flops, bel_flops" > "$out".tmp
inputs_base="$(dirname "$0")"/data
if [ "$3" = -q ]; then
	files=$inputs_base/*all*norm.csv\ $inputs_base/*_??user*norm.csv
else
	files=$inputs_base/*norm.csv
fi

for testfile in $files; do
	echo $testfile > /dev/stderr
    "$prog" "$testfile" >> "$out".tmp
done
mv "$out".tmp "$out"
