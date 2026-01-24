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

# Run validation
echo "📊 Extracting and validating PG struct sizes..."
echo ""

cd "$REPO_ROOT"
if ! bash "$SCRIPT_DIR/check-pg-struct-sizes.sh" "$ELF_FILE"; then
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "⚠️  VALIDATION FAILED - DO NOT PROCEED WITH RELEASE"
    echo ""
    echo "Action required:"
    echo "  1. Identify which PR(s) changed the affected struct(s)"
    echo "  2. Create hotfix PR to increment PG version(s)"
    echo "  3. Merge hotfix to master"
    echo "  4. Re-run this validation"
    echo ""
    echo "See claude/release-manager/guides/7-pg-validation.md for details"
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    exit 1
fi

echo ""
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
