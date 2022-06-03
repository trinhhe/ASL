#!/bin/bash
### downloads file ###
# file="ml-latest-small.zip"
base="ml-25m"
file="$base.zip"
dataURL="https://files.grouplens.org/datasets/movielens/$file"

cd "$(realpath "$(dirname "$0")")"

# DIR="$(dirname "${BASH_SOURCE[0]}")"

dataFolder="data_big"

mkdir -p ../Data
test -f ../Data/$file || wget -O ../Data/$file $dataURL
mkdir -p $dataFolder
test -f $dataFolder/ratings_all.csv || (unzip -o ../Data/$file -d ../Data/ && mv ../Data/${file%.*}/ratings.csv $dataFolder/ratings_all.csv && rm -r ../Data/${file%.*})

test -f $dataFolder/ratings_all_norm.csv || (python normcsv.py $dataFolder/ratings_all.csv $dataFolder/ratings_all_norm.csv)

python generate_subsets.py "$dataFolder"

for csvfile in $dataFolder/*user.csv
do
    python normcsv.py "$csvfile" "${csvfile%.csv}_norm.csv"
done
