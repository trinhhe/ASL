orig_dir="measurements~"
bytes_dir="measurements"

do_one(){
	in="$1"
	bytes="$2"
	out="$1".aug

	cut -d, -f1-13 "$in" > "$in.aug.tmp"
	( echo \ bytes_seq, bytes_rnd; cut -d, -f14- "$bytes" | tail -n+2 ) > "$bytes.aug.tmp"
	paste -d, "$in.aug.tmp" "$bytes.aug.tmp" > "$out"
	rm "$in.aug.tmp" "$bytes.aug.tmp"
}

get_type(){
	o="$(head -n2 ../small/$1 | tail -n1 | cut -d, -f14)"
	if [ -z "$o" ]; then
		echo empty
	elif [ $o = 61140 ]; then
		echo small
	else
		echo large
	fi
}

cd "$(dirname "$0")/../$orig_dir" || exit 1
for j in small big compl_bipartite; do
	cd $j
	for i in *.csv; do
		t=`get_type $i`
		[ $t = empty ] && continue
		if [ $t = small ]; then
			src=from_scratch@-DOPTVARIANT__3@-Ofast@-march__native.csv
		else
			src=from_scratch@-DOPTVARIANT__7@-Ofast@-march__native.csv
		fi
		do_one "$i" "../../$bytes_dir/$j/$src"
	done
	cd ..
done
