#!/bin/bash
mkdir -p videos
shopt -s extglob

for f in videos/!(*.flv); do
  ffmpeg -i $f -c:v libx264 -ar 22050 -crf 28 videos/$(basename ${f%.*}.flv)
  rm $f
done

