#!/bin/bash
#
# PG Validation for Release Preparation
#
# This script validates Parameter Group struct sizes of the reference target
# (SPEEDYBEEF745AIO) against the last shipped release to catch unversioned
# struct changes before release.
#
# The baseline is cmake/pg_struct_sizes.reference.db as committed at the newest
# final release tag (X.Y.Z) reachable from HEAD, ignoring a tag on HEAD itself.
# PG versions are 4 bits, so a version change is any difference modulo 16
# (15 -> 0 is a valid increment).
#
# Usage: ./cmake/validate-pg-for-release.sh [--baseline-tag TAG] [--current-file FILE]
#   --baseline-tag TAG   Compare against TAG instead of the newest release tag
#   --current-file FILE  Use "struct size version" lines from FILE instead of
#                        building the reference target (for testing)
#
# Exit codes:
#   0 - Validation passed
#   1 - Validation failed (size changed without version increment)
#   2 - Build or setup error
#   3 - Validation passed, but the reference database differs from the one
#       committed at HEAD: commit it before the freeze/tag (re-run to confirm)
#

set -euo pipefail

SCRIPT_DIR=$(dirname "$0")
REPO_ROOT=$(cd "$SCRIPT_DIR/.." && pwd)
REFERENCE_TARGET="SPEEDYBEEF745AIO"
BUILD_DIR="$REPO_ROOT/build"
DB_FILE="$SCRIPT_DIR/pg_struct_sizes.reference.db"
DB_PATH_IN_REPO="cmake/pg_struct_sizes.reference.db"
BASELINE_TAG=""
CURRENT_FILE=""

while [ $# -gt 0 ]; do
    case "$1" in
        --baseline-tag) [ $# -ge 2 ] || { echo "--baseline-tag needs a value" >&2; exit 2; }; BASELINE_TAG="$2"; shift 2 ;;
        --current-file) [ $# -ge 2 ] || { echo "--current-file needs a value" >&2; exit 2; }; CURRENT_FILE="$2"; shift 2 ;;
        *) echo "Unknown option: $1" >&2; exit 2 ;;
    esac
done

echo "🔍 Validating PG struct sizes for release..."
echo ""

# Current struct sizes: from a prebuilt list, or built from the reference target
TEMP_CURRENT=$(mktemp)
BASELINE_DB=$(mktemp)
NEW_DB=$(mktemp)
trap 'rm -f "$TEMP_CURRENT" "$BASELINE_DB" "$NEW_DB"' EXIT
if [ -n "$CURRENT_FILE" ]; then
    cp "$CURRENT_FILE" "$TEMP_CURRENT"
else
    # Check prerequisites
    if ! command -v arm-none-eabi-gcc &> /dev/null; then
        echo "❌ Error: arm-none-eabi-gcc not found" >&2
        echo "   Install ARM toolchain first" >&2
        exit 2
    fi

    if ! command -v cmake &> /dev/null; then
        echo "❌ Error: cmake not found" >&2
        exit 2
    fi

    # Setup build directory
    if [ ! -d "$BUILD_DIR" ]; then
        echo "📁 Creating build directory..."
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        cmake -G "Unix Makefiles" ..
    else
        echo "📁 Using existing build directory"
        cd "$BUILD_DIR"
    fi

    # Build reference target
    echo "🔨 Building reference target: $REFERENCE_TARGET..."
    echo "   (This may take a few minutes on first build)"
    echo ""

    BUILD_LOG=$(mktemp)
    if ! make "$REFERENCE_TARGET.elf" > "$BUILD_LOG" 2>&1; then
        echo "❌ Build failed. See $BUILD_LOG for details" >&2
        tail -30 "$BUILD_LOG" >&2
        exit 2
    fi
    rm -f "$BUILD_LOG"

    ELF_FILE="$BUILD_DIR/bin/$REFERENCE_TARGET.elf"

    if [ ! -f "$ELF_FILE" ]; then
        echo "❌ Error: ELF file not found: $ELF_FILE" >&2
        exit 2
    fi

    echo "✓ Build completed successfully"
    echo ""

    # Extract PG struct sizes from ELF binary
    echo "📊 Extracting PG struct sizes from binary..."
    echo ""

    # Detect architecture for correct nm command
    if command -v arm-none-eabi-nm &> /dev/null && [[ $(file "$ELF_FILE") == *"ARM"* ]]; then
        NM_CMD="arm-none-eabi-nm"
    else
        NM_CMD="nm"
    fi

    # Extract current sizes and versions
    cd "$REPO_ROOT"

    $NM_CMD --print-size "$ELF_FILE" 2>/dev/null | grep "pgResetTemplate_" | \
        while read addr size_hex type symbol; do
            # Extract config name from symbol
            config_name="${symbol#pgResetTemplate_}"

            # Convert hex size to decimal
            size_dec=$((16#$size_hex))

            # Find corresponding struct type and version from PG_REGISTER
            pg_register_line=$(grep -rh "PG_REGISTER.*$config_name" src/main --include="*.c" 2>/dev/null | head -1)

            struct_type=$(echo "$pg_register_line" | grep -oP 'PG_REGISTER[^(]*\(\K[^,]+' | head -1)

            if [ -z "$struct_type" ]; then
                # Fallback: convert config name to struct type
                struct_type="${config_name}_t"
            fi

            # Extract PG version (4th parameter in PG_REGISTER)
            version=$(echo "$pg_register_line" | grep -oP 'PG_REGISTER[^(]*\([^,]+,[^,]+,[^,]+,\s*\K\d+' | head -1)

            if [ -z "$version" ]; then
                version="?"
            fi

            printf "%-30s %3s %s\n" "$struct_type" "$size_dec" "$version"
        done | sort -u > "$TEMP_CURRENT"
fi

# Baseline: the reference database as shipped at the last release tag
cd "$REPO_ROOT"
if [ -z "$BASELINE_TAG" ]; then
    BASELINE_TAG=$(git tag --merged HEAD --sort=-v:refname | grep -E '^[0-9]+\.[0-9]+\.[0-9]+$' | grep -vxF -f <(git tag --points-at HEAD) | head -1 || true)
fi
if [ -z "$BASELINE_TAG" ]; then
    echo "❌ Error: no release tag found to use as baseline (use --baseline-tag)" >&2
    exit 2
fi

if ! git show "$BASELINE_TAG:$DB_PATH_IN_REPO" > "$BASELINE_DB" 2>/dev/null; then
    echo "❌ Error: $DB_PATH_IN_REPO does not exist at tag $BASELINE_TAG (use --baseline-tag)" >&2
    exit 2
fi
if [ ! -s "$TEMP_CURRENT" ]; then
    echo "❌ Error: no PG struct sizes found for the current build" >&2
    exit 2
fi

echo "📌 Baseline: last shipped release $BASELINE_TAG"
echo ""

# Validate against the shipped baseline
echo "🔍 Validating against baseline $BASELINE_TAG..."
echo ""

FAILED=0
ISSUES=""

while read -r struct_type current_size current_version; do
    [ -z "$struct_type" ] && continue

    if ! [[ "$current_version" =~ ^[0-9]+$ ]]; then
        echo "  ⚠️  Warning: Cannot find PG version for $struct_type - skipping" >&2
        continue
    fi

    db_entry=$(grep "^$struct_type " "$BASELINE_DB" 2>/dev/null || echo "")

    if [ -z "$db_entry" ]; then
        echo "  ➕ New: $struct_type (${current_size}B, v$current_version)"
        continue
    fi

    db_size=$(echo "$db_entry" | awk '{print $2}')
    db_version=$(echo "$db_entry" | awk '{print $3}')

    if [ "$current_size" != "$db_size" ]; then
        # PG versions are 4 bits: any difference modulo 16 changes the stored version
        if [ $(( (current_version - db_version) & 15 )) -eq 0 ]; then
            echo "  ❌ $struct_type: size changed ${db_size}B → ${current_size}B but version not incremented (still v$current_version)"
            ISSUES="$ISSUES\n  • $struct_type: ${db_size}B → ${current_size}B (version $current_version should be $(( (current_version + 1) & 15 )))"
            FAILED=1
        else
            echo "  ✅ $struct_type: size changed ${db_size}B → ${current_size}B with version increment v$db_version → v$current_version"
        fi
    else
        echo "  ✓ $struct_type (${current_size}B)"
    fi
done < "$TEMP_CURRENT"

# Report results
echo ""

if [ $FAILED -eq 1 ]; then
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "❌ PG VALIDATION FAILED - DO NOT PROCEED WITH RELEASE"
    echo ""
    echo "The following structs changed size without version increments:"
    echo -e "$ISSUES"
    echo ""
    echo "Action required:"
    echo "  1. Identify which PR(s) changed the affected struct(s)"
    echo "  2. Create hotfix PR to increment PG version(s)"
    echo "  3. Merge hotfix to target branch"
    echo "  4. Re-run this validation"
    echo ""
    echo "Fix: Increment PG version in PG_REGISTER for affected structs"
    echo ""
    echo "See claude/release-manager/guides/7-pg-validation.md for details"
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    exit 1
fi

# The release tag must carry the validated sizes, because the next release
# validates against this database as committed at that tag. Structs no longer in
# the build are dropped; structs without a known version are not recorded.
sort -u "$TEMP_CURRENT" | while read -r struct_type size version; do
    [[ "$version" =~ ^[0-9]+$ ]] || continue
    printf "%-30s %3s %s\n" "$struct_type" "$size" "$version"
done > "$NEW_DB"

# Line order is not significant, so compare sorted.
if ! git show "HEAD:$DB_PATH_IN_REPO" 2>/dev/null | sort | cmp -s - "$NEW_DB"; then
    # A --current-file run is a test and leaves the working copy alone.
    [ -z "$CURRENT_FILE" ] && cp "$NEW_DB" "$DB_FILE"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "⚠️  PG sizes validated, but $DB_PATH_IN_REPO differs from the committed copy"
    echo ""
    echo "Commit the updated database BEFORE the freeze/tag, then re-run this script."
    echo "Otherwise the next release will validate against a stale baseline."
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    exit 3
fi
echo "✓ $DB_PATH_IN_REPO is up to date"
echo ""

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "✅ PG VALIDATION PASSED"
echo ""
echo "All struct sizes validated successfully against the last shipped release."
echo "Safe to proceed with release preparation."
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

exit 0
