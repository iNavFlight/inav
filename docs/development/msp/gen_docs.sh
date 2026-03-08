#!/usr/bin/env bash

set -euo pipefail

INAV_MAIN_PATH="../../../src/main"
INAV_ROOT="../../.."
BUILD_INFO_SCRIPT="./get_fc_build_info.sh"
KEEP_HEADERS=0

for arg in "$@"; do
  if [[ "$arg" == "--keep_headers" ]]; then
    KEEP_HEADERS=1
  fi
done

eval "$(bash "$BUILD_INFO_SCRIPT" "$INAV_ROOT")"

echo "###########"
echo get_all_inav_enums_h.py
python get_all_inav_enums_h.py --inav-root "$INAV_MAIN_PATH"

echo "###########"
echo gen_msp_md.py
python gen_msp_md.py

echo "###########"
echo gen_enum_md.py
python gen_enum_md.py \
  --fc-version-major "$FC_VERSION_MAJOR" \
  --fc-version-minor "$FC_VERSION_MINOR" \
  --fc-version-patch-level "$FC_VERSION_PATCH_LEVEL" \
  --git-revision "$GIT_REVISION"
if [[ "$KEEP_HEADERS" -eq 0 ]]; then
  rm all_enums.h
fi
