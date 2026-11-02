#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INAV_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
INAV_MAIN_PATH="$INAV_ROOT/src/main"
BUILD_INFO_SCRIPT="$SCRIPT_DIR/get_fc_build_info.sh"
KEEP_HEADERS=0

for arg in "$@"; do
  if [[ "$arg" == "--keep_headers" ]]; then
    KEEP_HEADERS=1
  fi
done

eval "$(bash "$BUILD_INFO_SCRIPT" "$INAV_ROOT")"

echo "###########"
echo get_all_inav_enums_h.py
python "$SCRIPT_DIR/get_all_inav_enums_h.py" --inav-root "$INAV_MAIN_PATH"

echo "###########"
echo gen_msp_md.py
python "$SCRIPT_DIR/gen_msp_md.py"

echo "###########"
echo gen_enum_md.py
python "$SCRIPT_DIR/gen_enum_md.py" \
  --fc-version-major "$FC_VERSION_MAJOR" \
  --fc-version-minor "$FC_VERSION_MINOR" \
  --fc-version-patch-level "$FC_VERSION_PATCH_LEVEL"
if [[ "$KEEP_HEADERS" -eq 0 ]]; then
  rm "$SCRIPT_DIR/all_enums.h"
fi
