#!/usr/bin/env bash
# build_pr.sh - Build a specific INAV target from a pull request.
#
# Usage:
#   build_pr.sh <PR_NUMBER> <TARGET> <JOB_ID>
#
# Environment variables:
#   INAV_DIR       - Path to the INAV source repo (default: parent of script's parent)
#   SERVER_DIR     - Path to the pr-build-server directory
#   BASE_BRANCH    - Base branch to merge PR into (default: maintenance-9.x)
#   NUM_CORES      - Number of parallel build jobs (default: nproc)
#   STATUS_FILE    - Path to write JSON status (default: builds/<JOB_ID>/status.json)
#
# Outputs:
#   - builds/<JOB_ID>/status.json  (updated throughout build)
#   - builds/<JOB_ID>/<TARGET>.hex (on success)
#   - builds/<JOB_ID>/build.log    (build output)

set -euo pipefail

PR_NUMBER="${1:?Usage: build_pr.sh <PR_NUMBER> <TARGET> <JOB_ID>}"
TARGET="${2:?Usage: build_pr.sh <PR_NUMBER> <TARGET> <JOB_ID>}"
JOB_ID="${3:?Usage: build_pr.sh <PR_NUMBER> <TARGET> <JOB_ID>}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_DIR="${SERVER_DIR:-$(cd "$SCRIPT_DIR/.." && pwd)}"
INAV_DIR="${INAV_DIR:-$(cd "$SERVER_DIR/.." && pwd)}"
BUILD_BASE_DIR="$SERVER_DIR/build_base"
WORKTREE_DIR="$SERVER_DIR/worktrees/pr-$PR_NUMBER"
JOB_DIR="$SERVER_DIR/builds/$JOB_ID"
BUILD_DIR="$JOB_DIR/cmake_build"
STATUS_FILE="$JOB_DIR/status.json"
LOG_FILE="$JOB_DIR/build.log"
BASE_BRANCH="${BASE_BRANCH:-maintenance-9.x}"
NUM_CORES="${NUM_CORES:-$(nproc)}"

# ── helpers ──────────────────────────────────────────────────────────────────

write_status() {
    local phase="$1" message="$2" extra="${3:-}"
    local ts
    ts=$(date -u +%Y-%m-%dT%H:%M:%SZ)
    printf '{"job_id":"%s","pr":%s,"target":"%s","phase":"%s","message":"%s","updated_at":"%s"%s}\n' \
        "$JOB_ID" "$PR_NUMBER" "$TARGET" "$phase" "$message" "$ts" "$extra" \
        > "$STATUS_FILE"
}

fail() {
    local msg="$1"
    echo "ERROR: $msg" | tee -a "$LOG_FILE"
    write_status "failed" "$msg"
    exit 1
}

# ── setup ─────────────────────────────────────────────────────────────────────

mkdir -p "$JOB_DIR"
exec > >(tee -a "$LOG_FILE") 2>&1

echo "=== INAV PR Build: PR #$PR_NUMBER  Target: $TARGET  Job: $JOB_ID ==="
echo "Date: $(date -u --iso-8601=seconds)"

write_status "fetching" "Fetching PR #$PR_NUMBER from GitHub"

# ── fetch the PR ──────────────────────────────────────────────────────────────

cd "$INAV_DIR"
git fetch origin "pull/$PR_NUMBER/head:pr/$PR_NUMBER" \
    || fail "Failed to fetch PR #$PR_NUMBER"

PR_SHA=$(git rev-parse "pr/$PR_NUMBER")
BASE_SHA=$(git rev-parse "origin/$BASE_BRANCH")
echo "PR SHA: $PR_SHA"
echo "Base SHA: $BASE_SHA"

# ── create / update worktree ──────────────────────────────────────────────────

write_status "merging" "Merging PR #$PR_NUMBER into $BASE_BRANCH"

if [ -d "$WORKTREE_DIR" ]; then
    echo "Updating existing worktree at $WORKTREE_DIR"
    cd "$WORKTREE_DIR"
    git checkout "$BASE_BRANCH" 2>/dev/null || git checkout -b "$BASE_BRANCH" "origin/$BASE_BRANCH"
    git reset --hard "origin/$BASE_BRANCH"
else
    echo "Creating worktree at $WORKTREE_DIR"
    mkdir -p "$(dirname "$WORKTREE_DIR")"
    git worktree add "$WORKTREE_DIR" "origin/$BASE_BRANCH" \
        || fail "Failed to create worktree"
    cd "$WORKTREE_DIR"
fi

# Merge the PR (non-interactive; abort on conflict)
git merge --no-edit "pr/$PR_NUMBER" \
    || { git merge --abort 2>/dev/null || true
         fail "Merge conflict: PR #$PR_NUMBER does not merge cleanly into $BASE_BRANCH"; }

MERGED_SHA=$(git rev-parse HEAD)
echo "Merged SHA: $MERGED_SHA"

# ── set up cmake build directory ──────────────────────────────────────────────

write_status "cmake" "Initialising cmake build directory"

# If there is a pre-built base that matches the current base SHA, use it as a
# starting point (rsync preserves timestamps so ninja only rebuilds what changed).
PREBUILD_SHA_FILE="$BUILD_BASE_DIR/.built_sha"
if [ -d "$BUILD_BASE_DIR" ] && [ -f "$PREBUILD_SHA_FILE" ] \
   && [ "$(cat "$PREBUILD_SHA_FILE")" = "$BASE_SHA" ]; then
    echo "Seeding build from pre-built base..."
    rsync -a --delete "$BUILD_BASE_DIR/" "$BUILD_DIR/"
    # Re-run cmake so it points at the new worktree source directory
    cd "$BUILD_DIR"
    cmake -DWARNINGS_AS_ERRORS=OFF \
          -DMAIN_COMPILE_OPTIONS=-pipe \
          -G Ninja \
          "$WORKTREE_DIR" \
        || fail "cmake reconfiguration failed"
else
    echo "No matching pre-built base found; running full cmake configure"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake -DWARNINGS_AS_ERRORS=OFF \
          -DMAIN_COMPILE_OPTIONS=-pipe \
          -G Ninja \
          "$WORKTREE_DIR" \
        || fail "cmake configuration failed"
fi

# ── build the target ──────────────────────────────────────────────────────────

write_status "building" "Compiling $TARGET (PR #$PR_NUMBER)"

echo "--- Starting ninja build ---"
START_TIME=$(date +%s)
ninja -j"$NUM_CORES" "$TARGET" \
    || fail "Build failed - check build.log for details"
END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))
echo "--- Build complete in ${ELAPSED}s ---"

# ── collect the hex ───────────────────────────────────────────────────────────

HEX_FILE=$(find "$BUILD_DIR" -maxdepth 1 -name "${TARGET}*.hex" | head -1)
if [ -z "$HEX_FILE" ]; then
    fail "Build succeeded but no .hex file found for $TARGET"
fi

DEST_HEX="$JOB_DIR/$(basename "$HEX_FILE")"
cp "$HEX_FILE" "$DEST_HEX"
HEX_BASENAME=$(basename "$DEST_HEX")
HEX_SIZE=$(stat -c%s "$DEST_HEX")

echo "Hex file: $DEST_HEX ($HEX_SIZE bytes)"

write_status "done" "Build complete in ${ELAPSED}s" \
    ",\"hex_file\":\"$HEX_BASENAME\",\"hex_size\":$HEX_SIZE,\"elapsed_s\":$ELAPSED,\"pr_sha\":\"$PR_SHA\",\"merged_sha\":\"$MERGED_SHA\""

echo "=== SUCCESS ==="
