#!/usr/bin/env bash
# prebuild.sh - Pre-build a seed target from maintenance-9.x to warm the object cache.
#
# Run via cron periodically to keep the base build up to date, e.g.:
#   30 2 * * * /path/to/pr-build-server/scripts/prebuild.sh >> /path/to/pr-build-server/logs/prebuild.log 2>&1
#
# This script builds MATEKF405SE from maintenance-9.x into build_base/.
# When a user requests a PR+target build, the build script copies build_base/
# and then only rebuilds files changed by the PR plus target-specific files.
#
# Requirements: cmake, ninja, arm-none-eabi-gcc toolchain

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
INAV_DIR="$(cd "$SERVER_DIR/.." && pwd)"
BUILD_BASE_DIR="$SERVER_DIR/build_base"
LOG_DIR="$SERVER_DIR/logs"
SEED_TARGET="${SEED_TARGET:-MATEKF405SE}"
BASE_BRANCH="${BASE_BRANCH:-maintenance-9.x}"
NUM_CORES="${NUM_CORES:-$(nproc)}"

mkdir -p "$LOG_DIR"
LOGFILE="$LOG_DIR/prebuild-$(date +%Y%m%d-%H%M%S).log"

echo "=== INAV PR Build Server - Prebuild ===" | tee -a "$LOGFILE"
echo "Date: $(date -u --iso-8601=seconds)" | tee -a "$LOGFILE"
echo "INAV dir: $INAV_DIR" | tee -a "$LOGFILE"
echo "Build base dir: $BUILD_BASE_DIR" | tee -a "$LOGFILE"
echo "Seed target: $SEED_TARGET" | tee -a "$LOGFILE"
echo "Base branch: $BASE_BRANCH" | tee -a "$LOGFILE"

# Update the local inav repo to the latest maintenance-9.x
echo "--- Fetching $BASE_BRANCH ---" | tee -a "$LOGFILE"
cd "$INAV_DIR"
git fetch origin "$BASE_BRANCH" 2>&1 | tee -a "$LOGFILE"
git checkout "$BASE_BRANCH" 2>&1 | tee -a "$LOGFILE"
git reset --hard "origin/$BASE_BRANCH" 2>&1 | tee -a "$LOGFILE"

CURRENT_SHA=$(git rev-parse HEAD)
echo "Current HEAD: $CURRENT_SHA" | tee -a "$LOGFILE"

# Check if we already built this SHA
SHA_FILE="$BUILD_BASE_DIR/.built_sha"
if [ -f "$SHA_FILE" ] && [ "$(cat "$SHA_FILE")" = "$CURRENT_SHA" ]; then
    echo "Build base is already up to date for $CURRENT_SHA - skipping" | tee -a "$LOGFILE"
    exit 0
fi

# Initialize or re-initialize the cmake build directory
echo "--- Initializing cmake build in $BUILD_BASE_DIR ---" | tee -a "$LOGFILE"
mkdir -p "$BUILD_BASE_DIR"
cd "$BUILD_BASE_DIR"

cmake -DWARNINGS_AS_ERRORS=OFF \
      -DMAIN_COMPILE_OPTIONS=-pipe \
      -G Ninja \
      "$INAV_DIR" \
      2>&1 | tee -a "$LOGFILE"

# Build the seed target to warm up common object compilation
echo "--- Building seed target: $SEED_TARGET ---" | tee -a "$LOGFILE"
START_TIME=$(date +%s)
ninja -j"$NUM_CORES" "$SEED_TARGET" 2>&1 | tee -a "$LOGFILE"
END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

echo "--- Seed build complete in ${ELAPSED}s ---" | tee -a "$LOGFILE"

# Record which SHA this build corresponds to
echo "$CURRENT_SHA" > "$SHA_FILE"

# Keep only last 10 log files
cd "$LOG_DIR"
ls -t prebuild-*.log 2>/dev/null | tail -n +11 | xargs -r rm --

echo "=== Prebuild complete ===" | tee -a "$LOGFILE"
