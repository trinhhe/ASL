#!/bin/bash
### downloads file ###
# file="ml-latest-small.zip"
file="ml-25m.zip"
dataURL="https://files.grouplens.org/datasets/movielens/$file"

# DIR="$(dirname "${BASH_SOURCE[0]}")"

test -f ../Data/$file || wget -O ../Data/$file $dataURL
test -f data/ratings_all.csv || (unzip ../Data/$file -d ../Data/ && mv ../Data/${file%.*}/ratings.csv data/ratings_all.csv && rm -r ../Data/${file%.*})

test -f data/ratings_all_norm.csv || (python normcsv.py data/ratings_all.csv data/ratings_all_norm.csv)

python generate_subsets.py

for csvfile in data/*user.csv
do
    python normcsv.py "$csvfile" "${csvfile%.csv}_norm.csv"
done