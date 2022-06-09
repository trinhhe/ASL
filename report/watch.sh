#!/bin/sh

inotifywait -m -e close_write,moved_to --format %e/%f . macros img |
while IFS=/ read -r events file; do
	if echo "$file" | grep -q -E '\.(tex|bib|asy|xmpdata)$'; then
		make
	fi
done
