#!/bin/bash
#
# PG Validation for Release Preparation
#
# This script validates Parameter Group struct sizes against the reference
# target (SPEEDYBEEF745AIO) to catch unversioned struct changes before release.
#
# Usage: ./cmake/validate-pg-for-release.sh
#
# Exit codes:
#   0 - Validation passed
#   1 - Validation failed (size changed without version increment)
#   2 - Build or setup error
#

set -euo pipefail

SCRIPT_DIR=$(dirname "$0")
REPO_ROOT=$(cd "$SCRIPT_DIR/.." && pwd)
REFERENCE_TARGET="SPEEDYBEEF745AIO"
BUILD_DIR="$REPO_ROOT/build"
DB_FILE="$SCRIPT_DIR/pg_struct_sizes.reference.db"

echo "🔍 Validating PG struct sizes for release..."
echo ""

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

if [ ! -f "$DB_FILE" ]; then
    echo "❌ Error: Reference database not found: $DB_FILE" >&2
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

if ! make "$REFERENCE_TARGET.elf" > /tmp/pg-validation-build.log 2>&1; then
    echo "❌ Build failed. See /tmp/pg-validation-build.log for details" >&2
    tail -30 /tmp/pg-validation-build.log >&2
    exit 2
fi

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
TEMP_CURRENT=$(mktemp)
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
            version="0"
        fi

        printf "%-30s %3s %s\n" "$struct_type" "$size_dec" "$version"
    done | sort -u > "$TEMP_CURRENT"

# Validate against reference database
echo "🔍 Validating against reference database..."
echo ""

FAILED=0
ISSUES=""
UPDATED=""

# Check each struct in current binary
while read -r struct_type current_size current_version; do
    [ -z "$struct_type" ] && continue

    # Look up in database
    db_entry=$(grep "^$struct_type " "$DB_FILE" 2>/dev/null || echo "")

    if [ -z "$db_entry" ]; then
        # New struct not in database - add it automatically
        if [ -z "$current_version" ]; then
            echo "  ⚠️  Warning: Cannot find PG version for new struct $struct_type" >&2
            echo "  ℹ️  New: $struct_type (${current_size}B) - skipping (no PG_REGISTER found)"
            continue
        fi

        echo "  ➕ New: $struct_type (${current_size}B, v$current_version)"

        # Add to database
        printf "%-30s %3s %s\n" "$struct_type" "$current_size" "$current_version" >> "$DB_FILE"
        UPDATED="$UPDATED\n  • $struct_type: NEW (${current_size}B, v$current_version)"
        continue
    fi

    db_size=$(echo "$db_entry" | awk '{print $2}')
    db_version=$(echo "$db_entry" | awk '{print $3}')

    if [ "$current_size" != "$db_size" ]; then
        # Size changed - check if version was incremented
        if [ -z "$current_version" ]; then
            echo "  ⚠️  Warning: Cannot find PG version for $struct_type" >&2
            continue
        fi

        if [ "$current_version" -le "$db_version" ]; then
            # SIZE CHANGED BUT VERSION NOT INCREMENTED - FAIL BUILD
            echo "  ❌ $struct_type: size changed ${db_size}B → ${current_size}B but version not incremented (still v$current_version)"
            ISSUES="$ISSUES\n  • $struct_type: ${db_size}B → ${current_size}B (version $current_version should be $((current_version + 1)))"
            FAILED=1
        else
            # Version was incremented - this is valid, update database
            echo "  ✅ $struct_type: size changed ${db_size}B → ${current_size}B with version increment v$db_version → v$current_version"

            # Update database entry with new size and version
            sed -i "s|^$struct_type[[:space:]]\+[0-9]\+[[:space:]]\+[0-9]\+$|$(printf "%-30s %3s %s" "$struct_type" "$current_size" "$current_version")|" "$DB_FILE"
            UPDATED="$UPDATED\n  • $struct_type: ${db_size}B → ${current_size}B (v$db_version → v$current_version)"
        fi
    else
        # Size unchanged
        echo "  ✓ $struct_type (${current_size}B)"
    fi
done < "$TEMP_CURRENT"

rm -f "$TEMP_CURRENT"

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

if [ -n "$UPDATED" ]; then
    echo "📝 Database auto-updated for structs with version increments:"
    echo -e "$UPDATED"
    echo ""
fi

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "✅ PG VALIDATION PASSED"
echo ""
echo "All struct sizes validated successfully against reference target."
echo "Safe to proceed with release preparation."
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

exit 0
