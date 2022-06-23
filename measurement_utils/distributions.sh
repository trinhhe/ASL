fn="$1"
if [ -z "$fn" ]; then
	echo Usage: $0 input.csv
	exit1
fi

despace(){
	sed -re 's/^\s+//g' 
}

cut -d, -f1 "$fn" | uniq -c | despace | cut -d\  -f1 | sort -n | uniq -c | despace > user_stats
cut -d, -f2 "$fn" | sort -n | uniq -c | despace | cut -d\  -f1 | sort -n | uniq -c | despace > movie_stats
python mkdistplot.py <user_stats user_stats.pdf
python mkdistplot.py <movie_stats movie_stats.pdf
