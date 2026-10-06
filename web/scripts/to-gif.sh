#!/usr/bin/env bash
# Converts the Playwright recordings into GIF + MP4 for the README and the portfolio.
set -euo pipefail
cd "$(dirname "$0")/../.."

RAW=docs/media/raw
OUT=docs/media

convert() {
  local name=$1 speed=$2
  local input="$RAW/$name.webm"
  ffmpeg -loglevel error -y -i "$input" \
    -vf "setpts=PTS/$speed,fps=12,scale=480:-1:flags=lanczos,split[a][b];[a]palettegen=max_colors=64[p];[b][p]paletteuse=dither=bayer" \
    "$OUT/$name.gif"
  ffmpeg -loglevel error -y -i "$input" -an \
    -vf "setpts=PTS/$speed,scale=600:-2" -c:v libx264 -pix_fmt yuv420p -movflags +faststart \
    "$OUT/$name.mp4"
  echo "$name: $(du -k "$OUT/$name.gif" | cut -f1) KB gif, $(du -k "$OUT/$name.mp4" | cut -f1) KB mp4"
}

convert human-vs-hard 1.5
convert ai-vs-ai 2
convert how-it-thinks 1
