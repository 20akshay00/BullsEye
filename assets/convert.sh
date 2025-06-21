#!/bin/bash

BLOCKSDS="${BLOCKSDS:-/opt/blocksds/core/}"
GRIT=$BLOCKSDS/tools/grit/grit

process_recursive() {
    local src_dir="$1"
    local out_dir_base="$2"
    shift 2
    local grit_flags=("$@")

    if [ -d "$src_dir" ]; then
        [ -d "$out_dir_base" ] || mkdir -p "$out_dir_base"
        find "$src_dir" -name "*.png" | while read -r file; do
            rel_path="${file#$src_dir/}"
            out_dir="$out_dir_base/$(dirname "$rel_path")"
            mkdir -p "$out_dir"
            $GRIT "$file" "${grit_flags[@]}"
            mv ./*.pal ./*.img ./*.map "$out_dir/" 2>/dev/null
        done
    fi
}

# Process fonts
process_recursive "./assets/fonts" "./nitrofiles/fonts" \
    -ftB -fh! -gTFF00FF -gt -gB8 -m!

# Process backgrounds (was images)
process_recursive "./assets/backgrounds" "./nitrofiles/backgrounds" \
    -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs

# Process sprites
process_recursive "./assets/sprites" "./nitrofiles/sprites" \
    -ftB -fh! -gTFF00FF -gt -gB8 -m!

# Process animated sprites
process_recursive "./assets/animatedsprites" "./nitrofiles/animatedsprites" \
    -ftB -fh! -gTFF00FF -gt -gB8 -m!
