#!/bin/bash
#
# Run the size tool on every built .elf and write a small JSON report of
# flash/RAM usage per target, consumed by ci-size-report.yml.
#
# Usage: extract-size-report.sh <build-dir> <output-json> [size-tool]
#
# flash = .text + .data (what's programmed into flash)
# ram   = .data + .bss  (what's reserved in RAM at runtime)
#
# Runs inside the (unprivileged) build job on the PR's own checkout, so a
# PR could in principle modify this script to misreport its own numbers.
# Accepted tradeoff: this feature is informational/non-gating, and a real
# overflow still fails the link step regardless of what this script says.

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

JQ_ARGS=()
for elf in "${ELFS[@]}"; do
    target=$(basename "$elf" .elf)

    # Berkeley format: "   text    data     bss     dec     hex filename"
    read -r text data bss _dec _hex _name < <("$SIZE_TOOL" -B "$elf" | tail -n1)

    flash=$((text + data))
    ram=$((data + bss))

    JQ_ARGS+=(--argjson "entry_${#JQ_ARGS[@]}" "{\"target\":\"${target}\",\"flash\":${flash},\"ram\":${ram}}")
done

# Build via jq rather than manual string concatenation, so the target name
# (an .elf basename, not otherwise validated) is JSON-escaped properly
# instead of relying on it never containing a special character.
jq -n "${JQ_ARGS[@]}" 'reduce $ARGS.named[] as $e ({}; .[$e.target] = {flash: $e.flash, ram: $e.ram})' \
    > "$OUTPUT_JSON"

echo "Wrote size report for ${#ELFS[@]} target(s) to $OUTPUT_JSON"
