#!/usr/bin/env bash

set -euo pipefail

INAV_ROOT="${1:-$(git rev-parse --show-toplevel)}"
CMAKE_FILE="$INAV_ROOT/CMakeLists.txt"

version_fields="$(sed -nE 's/^project\(INAV VERSION ([0-9]+)\.([0-9]+)\.([0-9]+)\).*/\1 \2 \3/p' "$CMAKE_FILE")"
read -r FC_VERSION_MAJOR FC_VERSION_MINOR FC_VERSION_PATCH_LEVEL <<< "$version_fields"
GIT_REVISION="$(git -C "$INAV_ROOT" rev-parse --short=8 HEAD)"

printf 'FC_VERSION_MAJOR=%s\n' "$FC_VERSION_MAJOR"
printf 'FC_VERSION_MINOR=%s\n' "$FC_VERSION_MINOR"
printf 'FC_VERSION_PATCH_LEVEL=%s\n' "$FC_VERSION_PATCH_LEVEL"
printf 'GIT_REVISION=%s\n' "$GIT_REVISION"
