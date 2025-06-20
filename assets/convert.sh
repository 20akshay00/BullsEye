#!/bin/bash

BLOCKSDS="${BLOCKSDS:-/opt/blocksds/core/}"
GRIT=$BLOCKSDS/tools/grit/grit

# Process fonts (flat directory)
if [ -d ./assets/fonts ]; then
    [ -d ../nitrofiles/fonts ] || mkdir -p ./nitrofiles/fonts
    for file in ./assets/fonts/*.png; do
        $GRIT "$file" -ftB -fh! -gTFF00FF -gt -gB8 -m!
        mv ./*.pal ./*.img ./nitrofiles/fonts/
    done
fi

# Process images (recursive)
if [ -d ./assets/images ]; then
    [ -d ./nitrofiles/images ] || mkdir -p ./nitrofiles/images
    find ./assets/images -name "*.png" | while read -r file; do
        rel_path="${file#./assets/images/}"
        out_dir="./nitrofiles/images/$(dirname "$rel_path")"
        mkdir -p "$out_dir"
        $GRIT "$file" -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
        mv ./*.pal ./*.img ./*.map "$out_dir/"
    done
fi
