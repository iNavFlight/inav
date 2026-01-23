#!/bin/bash
#
# Compare PG struct sizes across multiple binaries
# Uses nm to read symbol table (no extra dependencies)
#

set -euo pipefail

if [ $# -lt 2 ]; then
    echo "Usage: $0 <elf1> <elf2> [elf3...]" >&2
    echo "Example: $0 build/MATEKF405.elf build/MATEKF722.elf build/MATEKH743.elf" >&2
    exit 1
fi

SCRIPT_DIR=$(dirname "$0")
ELF_FILES=("$@")

echo "Comparing PG struct sizes across platforms..."
echo ""

# Extract sizes from each binary into temp files
declare -a TEMP_FILES
for i in "${!ELF_FILES[@]}"; do
    TEMP_FILES[$i]=$(mktemp)
    "$SCRIPT_DIR/extract-pg-sizes-nm.sh" "${ELF_FILES[$i]}" 2>/dev/null > "${TEMP_FILES[$i]}"
done

# Get union of all struct names
ALL_STRUCTS=$(cat "${TEMP_FILES[@]}" | cut -d' ' -f1 | sort -u)

# Print header
printf "%-30s" "Struct"
for elf in "${ELF_FILES[@]}"; do
    basename=$(basename "$elf" | sed 's/.elf$//')
    printf " %15s" "$basename"
done
printf "\n"

printf "%-30s" "=============================="
for elf in "${ELF_FILES[@]}"; do
    printf " %15s" "==============="
done
printf "\n"

# For each struct, show size from each binary
while read -r struct; do
    printf "%-30s" "$struct"

    sizes=()
    for temp_file in "${TEMP_FILES[@]}"; do
        size=$(grep "^$struct " "$temp_file" 2>/dev/null | awk '{print $2}')

        if [ -z "$size" ]; then
            printf " %15s" "N/A"
        else
            printf " %15s" "${size}B"
            sizes+=("$size")
        fi
    done

    # Check if all sizes are the same
    if [ ${#sizes[@]} -gt 1 ]; then
        first="${sizes[0]}"
        all_same=1
        for s in "${sizes[@]}"; do
            if [ "$s" != "$first" ]; then
                all_same=0
                break
            fi
        done

        if [ $all_same -eq 0 ]; then
            printf " ⚠️ MISMATCH"
        fi
    fi

    printf "\n"
done <<< "$ALL_STRUCTS"

# Cleanup
for temp_file in "${TEMP_FILES[@]}"; do
    rm -f "$temp_file"
done

echo ""
echo "Legend:"
echo "  N/A = Struct not found in this platform's binary"
echo "  ⚠️ MISMATCH = Struct has different sizes across platforms"
