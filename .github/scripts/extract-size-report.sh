#!/bin/bash
#
# Run the size tool on every built .elf and write a small JSON report of
# flash/RAM usage per target, consumed by ci-size-report.yml.
#
# Usage: extract-size-report.sh <build-dir> <output-json> [size-tool]
#
# flash = .text + .data (what's programmed into flash)
# ram   = .data + .bss  (what's reserved in RAM at runtime, summed across
#         EVERY writable memory region the target has — e.g. RAM+CCM on
#         F4/F7 parts, RAM+DTCM on H7. It's a total, not one region.)
# regions = { "<name>": <bytes>, ... } — the same total broken out per
#         linker memory region (RAM, CCM, DTCM, ...), computed from the
#         build's own .map file via compute-region-sizes.py. Omitted for a
#         target whose .map file is missing, so consumers must treat it as
#         optional.
#
# Runs inside the (unprivileged) build job on the PR's own checkout, so a
# PR could in principle modify this script to misreport its own numbers.
# Accepted tradeoff: this feature is informational/non-gating, and a real
# overflow still fails the link step regardless of what this script says.

set -euo pipefail

BUILD_DIR=${1:?usage: extract-size-report.sh <build-dir> <output-json> [size-tool]}
OUTPUT_JSON=${2:?usage: extract-size-report.sh <build-dir> <output-json> [size-tool]}
SIZE_TOOL=${3:-}

if [ -z "$SIZE_TOOL" ]; then
    if command -v arm-none-eabi-size >/dev/null 2>&1; then
        SIZE_TOOL=arm-none-eabi-size
    else
        # cmake/arm-none-eabi-checks.cmake downloads its own toolchain into
        # tools/ and adds it to PATH — but only inside that cmake process
        # via set(ENV{PATH} ...), which doesn't persist to a later CI step's
        # shell. Fall back to the same location CMake would have used,
        # relative to the repo root (this script must be run from there).
        SIZE_TOOL=$(compgen -G 'tools/arm-gnu-toolchain-*/bin/arm-none-eabi-size' 2>/dev/null | head -n1 || true)
        if [ -z "$SIZE_TOOL" ]; then
            echo "::error::arm-none-eabi-size not found on PATH or under tools/arm-gnu-toolchain-*/bin/" >&2
            exit 1
        fi
    fi
fi

# CMake's RUNTIME_OUTPUT_DIRECTORY puts built executables under
# <build-dir>/bin/ (see cmake/main.cmake), not <build-dir> directly — search
# instead of assuming a fixed depth, in case that ever changes.
mapfile -t ELFS < <(find "$BUILD_DIR" -maxdepth 3 -name '*.elf' | sort)
if [ "${#ELFS[@]}" -eq 0 ]; then
    echo "::warning::No .elf files found under $BUILD_DIR, writing empty size report"
    echo '{}' > "$OUTPUT_JSON"
    exit 0
fi

SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")

JQ_ARGS=()
for elf in "${ELFS[@]}"; do
    target=$(basename "$elf" .elf)

    # Berkeley format: "   text    data     bss     dec     hex filename"
    read -r text data bss _dec _hex _name < <("$SIZE_TOOL" -B "$elf" | tail -n1)

    flash=$((text + data))
    ram=$((data + bss))

    # cmake's stm32.cmake/at32.cmake link every target with -Wl,-Map,<elf>.map
    # (alongside -Wl,--print-memory-usage), so the per-region breakdown this
    # script computes here matches exactly what the linker itself reported at
    # build time. A missing map (e.g. a toolchain change that stops emitting
    # one) degrades to just the flat flash/ram totals above, not a hard error.
    map="${elf}.map"
    regions='{}'
    if [ -f "$map" ]; then
        regions=$(python3 "${SCRIPT_DIR}/compute-region-sizes.py" "$elf" "$map" "$SIZE_TOOL") || regions='{}'
    fi

    JQ_ARGS+=(--argjson "entry_${#JQ_ARGS[@]}" "{\"target\":\"${target}\",\"flash\":${flash},\"ram\":${ram},\"regions\":${regions}}")
done

# Build via jq rather than manual string concatenation, so the target name
# (an .elf basename, not otherwise validated) is JSON-escaped properly
# instead of relying on it never containing a special character. Omit
# "regions" entirely when empty rather than storing a misleading {} that
# would read as "this target has no writable memory regions".
jq -n "${JQ_ARGS[@]}" '
    reduce $ARGS.named[] as $e ({};
        .[$e.target] = {flash: $e.flash, ram: $e.ram}
            + (if ($e.regions | length) > 0 then {regions: $e.regions} else {} end)
    )' > "$OUTPUT_JSON"

echo "Wrote size report for ${#ELFS[@]} target(s) to $OUTPUT_JSON"
