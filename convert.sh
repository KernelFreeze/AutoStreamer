#!/bin/bash
mkdir -p videos
for f in videos/*.mp4; do
  ffmpeg -i $f -c:v libx264 -ar 22050 -crf 28 videos/$(basename ${f%.*}.flv)
  rm $f
done

