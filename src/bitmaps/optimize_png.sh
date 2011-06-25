#!/bin/bash
#convert to png
for pic in $(ls *.png)
do
  echo "optimizing $pic"
  optipng -O7 $pic
done

echo "Finished!"
