#!/bin/bash
#
# Check if parameter group struct modifications include version increments
# This prevents settings corruption when struct layout changes without version bump
#
# Exit codes:
#   0 - No issues found
#   1 - Potential issues detected (will post comment)
#   2 - Script error

set -euo pipefail

# Output file for issues found
ISSUES_FILE=$(mktemp)
trap "rm -f $ISSUES_FILE" EXIT

# Color output for local testing
if [ -t 1 ]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    NC='\033[0m' # No Color
else
    RED=''
    GREEN=''
    YELLOW=''
    NC=''
fi

echo "🔍 Checking for Parameter Group version updates..."

# Get base and head commits
BASE_REF=${GITHUB_BASE_REF:-}
HEAD_REF=${GITHUB_HEAD_REF:-}

if [ -z "$BASE_REF" ] || [ -z "$HEAD_REF" ]; then
    echo "⚠️  Warning: Not running in GitHub Actions PR context"
    echo "Using git diff against HEAD~1 for local testing"
    BASE_COMMIT="HEAD~1"
    HEAD_COMMIT="HEAD"
else
    BASE_COMMIT="origin/$BASE_REF"
    HEAD_COMMIT="HEAD"
fi

# Get list of changed files
CHANGED_FILES=$(git diff --name-only $BASE_COMMIT..$HEAD_COMMIT | grep -E '\.(c|h)$' || true)

if [ -z "$CHANGED_FILES" ]; then
    echo "✅ No C/H files changed"
    exit 0
fi

echo "📁 Changed files:"
echo "$CHANGED_FILES" | sed 's/^/  /'

# Function to extract PG info from a file
check_file_for_pg_changes() {
    local file=$1
    local diff_output=$(git diff $BASE_COMMIT..$HEAD_COMMIT -- "$file")

    # Check if file contains PG_REGISTER in current version
    if ! git show $HEAD_COMMIT:"$file" 2>/dev/null | grep -q "PG_REGISTER"; then
        return 0
    fi

    echo "  🔎 Checking $file (contains PG_REGISTER)"

    # Extract all PG_REGISTER lines from the diff (both old and new)
    local pg_registers=$(echo "$diff_output" | grep -E "^[-+].*PG_REGISTER" || true)

    if [ -z "$pg_registers" ]; then
        # PG_REGISTER exists but wasn't changed
        # Still need to check if the struct changed
        pg_registers=$(git show $HEAD_COMMIT:"$file" | grep "PG_REGISTER" || true)
    fi

    # Process each PG registration
    while IFS= read -r pg_line; do
        [ -z "$pg_line" ] && continue

        # Extract struct name and version
        # Pattern: PG_REGISTER.*\((\w+),\s*(\w+),\s*PG_\w+,\s*(\d+)\)
        if [[ $pg_line =~ PG_REGISTER[^(]*\(([^,]+),([^,]+),([^,]+),([^)]+)\) ]]; then
            local struct_type="${BASH_REMATCH[1]}"
            local pg_name="${BASH_REMATCH[2]}"
            local pg_id="${BASH_REMATCH[3]}"
            local version="${BASH_REMATCH[4]}"

            # Clean up whitespace
            struct_type=$(echo "$struct_type" | xargs)
            version=$(echo "$version" | xargs)

            echo "    📋 Found: $struct_type (version $version)"

            # Check if this struct's typedef was modified in ANY changed file
            local struct_pattern="typedef struct ${struct_type%_t}_s"
            local struct_body_diff=""
            local struct_found_in=""

            # Search all changed files for this struct definition
            while IFS= read -r changed_file; do
                [ -z "$changed_file" ] && continue

                local file_diff=$(git diff $BASE_COMMIT..$HEAD_COMMIT -- "$changed_file")
                local struct_in_file=$(echo "$file_diff" | sed -n "/${struct_pattern}/,/\}.*${struct_type};/p")

                if [ -n "$struct_in_file" ]; then
                    struct_body_diff="$struct_in_file"
                    struct_found_in="$changed_file"
                    echo "    🔍 Found struct definition in $changed_file"
                    break
                fi
            done <<< "$CHANGED_FILES"

            local struct_changes=$(echo "$struct_body_diff" | grep -E "^[-+]" \
                | grep -v -E "^[-+]\s*(typedef struct|}|//|\*)" \
                | sed -E 's://.*$::' \
                | sed -E 's:/\*.*\*/::' \
                | tr -d '[:space:]')

            if [ -n "$struct_changes" ]; then
                echo "    ⚠️  Struct definition modified in $struct_found_in"

                # Check if version was incremented in PG_REGISTER
                local old_version=$(echo "$diff_output" | grep "^-.*PG_REGISTER.*$struct_type" | sed -nE 's/.*,[[:space:]]*([0-9]+)[[:space:]]*\).*/\1/p' || echo "")
                local new_version=$(echo "$diff_output" | grep "^+.*PG_REGISTER.*$struct_type" | sed -nE 's/.*,[[:space:]]*([0-9]+)[[:space:]]*\).*/\1/p' || echo "")

                # Find line number of PG_REGISTER for error reporting
                local line_num=$(git show $HEAD_COMMIT:"$file" | grep -n "PG_REGISTER.*$struct_type" | cut -d: -f1 | head -1)

                if [ -n "$old_version" ] && [ -n "$new_version" ]; then
                    # PG_REGISTER was modified - check if version increased
                    if [ "$new_version" -le "$old_version" ]; then
                        echo "    ❌ Version NOT incremented ($old_version → $new_version)"
                        cat >> $ISSUES_FILE << EOF
### \`$struct_type\` ($file:$line_num)
- **Struct modified:** Field changes detected in $struct_found_in
- **Version status:** ❌ Not incremented (version $version)
- **Recommendation:** Increment version from $old_version to $(($old_version + 1))

EOF
                    else
                        echo "    ✅ Version incremented ($old_version → $new_version)"
                    fi
                elif [ -z "$old_version" ] && [ -z "$new_version" ]; then
                    # PG_REGISTER wasn't modified but struct was - THIS IS THE BUG!
                    echo "    ❌ PG_REGISTER not modified, version still $version"
                    cat >> $ISSUES_FILE << EOF
### \`$struct_type\` ($file:$line_num)
- **Struct modified:** Field changes detected in $struct_found_in
- **Version status:** ❌ Not incremented (still version $version)
- **Recommendation:** Increment version to $(($version + 1)) in $file

EOF
                else
                    # One exists but not the other - unusual edge case
                    echo "    ⚠️  Unusual version change pattern detected"
                    cat >> $ISSUES_FILE << EOF
### \`$struct_type\` ($file:$line_num)
- **Struct modified:** Field changes detected in $struct_found_in
- **Version status:** ⚠️ Unusual change pattern (old: ${old_version:-none}, new: ${new_version:-none})
- **Current version:** $version
- **Recommendation:** Manually verify version increment

EOF
                fi
            else
                echo "    ✅ Struct unchanged"
            fi
        fi
    done <<< "$pg_registers"
}

# Build list of files to check (changed files + companions with PG_REGISTER)
echo "🔍 Building file list including companions with PG_REGISTER..."
FILES_TO_CHECK=""
ALREADY_ADDED=""

while IFS= read -r file; do
    [ -z "$file" ] && continue

    # Add this file to check list
    if ! echo "$ALREADY_ADDED" | grep -qw "$file"; then
        FILES_TO_CHECK="$FILES_TO_CHECK$file"$'\n'
        ALREADY_ADDED="$ALREADY_ADDED $file"
    fi

    # Determine companion file (.c <-> .h)
    # (this loop runs at top level, so no "local" here: bash would abort the script)
    companion=""
    if [[ "$file" == *.c ]]; then
        companion="${file%.c}.h"
    elif [[ "$file" == *.h ]]; then
        companion="${file%.h}.c"
    fi

    # If companion exists and contains PG_REGISTER, add it to check list
    if [ -n "$companion" ]; then
        if git show $HEAD_COMMIT:"$companion" 2>/dev/null | grep -q "PG_REGISTER"; then
            if ! echo "$ALREADY_ADDED" | grep -qw "$companion"; then
                echo "  📎 Adding $companion (companion of $file with PG_REGISTER)"
                FILES_TO_CHECK="$FILES_TO_CHECK$companion"$'\n'
                ALREADY_ADDED="$ALREADY_ADDED $companion"
            fi
        fi
    fi
done <<< "$CHANGED_FILES"

# Check each file (including companions)
while IFS= read -r file; do
    [ -z "$file" ] && continue
    check_file_for_pg_changes "$file"
done <<< "$FILES_TO_CHECK"

# Check if any issues were found
if [ -s $ISSUES_FILE ]; then
    echo ""
    echo "${YELLOW}⚠️  Potential PG version issues detected${NC}"
    echo "Output saved to: $ISSUES_FILE"
    cat $ISSUES_FILE
    exit 1
else
    echo ""
    echo "${GREEN}✅ No PG version issues detected${NC}"
    exit 0
fi
