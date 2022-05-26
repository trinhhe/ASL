echo "TODO, I just copied the commands I've used. Look at the file and edit it to something sensible"
exit 1
cut -d, -f1 ratings_big_25m.csv | uniq -c | sed -re 's/^\s+//g' | cut -d\  -f1 | sort -n | uniq -c > user_stats
cut -d, -f2 ratings_big_25m.csv | sort -n | uniq -c | sed -re 's/^\s+//g' | cut -d\  -f1 | sort -n | uniq -c > movie_stats
python mkdistplot.py user_stats user_stats.pdf
python mkdistplot.py movie_stats movie_stats.pdf
