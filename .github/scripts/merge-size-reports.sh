#!/bin/bash
#
# Merge multiple per-shard size-report.json files (see extract-size-report.sh)
# into one aggregate JSON, used by the "upload-artifacts" job in ci.yml.
#
# Usage: merge-size-reports.sh <output-json> <input-json>...

set -euo pipefail

OUTPUT_JSON=${1:?usage: merge-size-reports.sh <output-json> <input-json>...}
shift

if [ "$#" -eq 0 ]; then
    echo '{}' > "$OUTPUT_JSON"
    exit 0
fi

jq -s 'add' "$@" > "$OUTPUT_JSON"
echo "Merged $# size report(s) into $OUTPUT_JSON"
