cd "$(dirname "$0")"
cd ..
# TODO: this assumes that all custom compiler flag variants start with -O
for base in `cd measurements; ls | sed -re 's/@-O.*//g' | uniq`; do
	measurement_utils/plot_perf.py -e pdf -o plots/$base -n measurements/$base@-O*;
done
