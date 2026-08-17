#!/bin/bash
#
# Run the size tool on every built .elf and write a small JSON report of
# flash/RAM usage per target, consumed by ci-size-report.yml.
#
# Usage: extract-size-report.sh <build-dir> <output-json> [size-tool]
#
# flash = .text + .data (what's programmed into flash)
# ram   = .data + .bss  (what's reserved in RAM at runtime)

set -euo pipefail

BUILD_DIR=${1:?usage: extract-size-report.sh <build-dir> <output-json> [size-tool]}
OUTPUT_JSON=${2:?usage: extract-size-report.sh <build-dir> <output-json> [size-tool]}
SIZE_TOOL=${3:-arm-none-eabi-size}

# CMake's RUNTIME_OUTPUT_DIRECTORY puts built executables under
# <build-dir>/bin/ (see cmake/main.cmake), not <build-dir> directly — search
# instead of assuming a fixed depth, in case that ever changes.
mapfile -t ELFS < <(find "$BUILD_DIR" -maxdepth 3 -name '*.elf' | sort)
if [ "${#ELFS[@]}" -eq 0 ]; then
    echo "::warning::No .elf files found under $BUILD_DIR, writing empty size report"
    echo '{}' > "$OUTPUT_JSON"
    exit 0
fi

ENTRIES=()
for elf in "${ELFS[@]}"; do
    target=$(basename "$elf" .elf)

    # Berkeley format: "   text    data     bss     dec     hex filename"
    read -r text data bss _dec _hex _name < <("$SIZE_TOOL" -B "$elf" | tail -n1)

    flash=$((text + data))
    ram=$((data + bss))

    ENTRIES+=("\"$target\":{\"flash\":$flash,\"ram\":$ram}")
done

{
    printf '{'
    printf '%s' "${ENTRIES[0]}"
    for entry in "${ENTRIES[@]:1}"; do
        printf ',%s' "$entry"
    done
    printf '}'
} > "$OUTPUT_JSON"

echo "Wrote size report for ${#ENTRIES[@]} target(s) to $OUTPUT_JSON"
