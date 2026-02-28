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

    # Check if file contains PG_REGISTER (in either old or new version)
    if ! echo "$diff_output" | grep -q "PG_REGISTER"; then
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

            # Check if this struct's typedef was modified
            local struct_pattern="typedef struct ${struct_type%_t}_s"
            # Isolate struct body from diff, remove comments and empty lines, then check for remaining changes
            local struct_body_diff=$(echo "$diff_output" | sed -n "/${struct_pattern}/,/\}.*${struct_type};/p")
            local struct_changes=$(echo "$struct_body_diff" | grep -E "^[-+]" \
                | grep -v -E "^[-+]\s*(typedef struct|}|//|\*)" \
                | sed -E 's://.*$::' \
                | sed -E 's:/\*.*\*/::' \
                | tr -d '[:space:]')

            if [ -n "$struct_changes" ]; then
                echo "    ⚠️  Struct definition modified"

                # Check if version was incremented in this diff
                local old_version=$(echo "$diff_output" | grep "^-.*PG_REGISTER.*$struct_type" | grep -oP ',\s*\K\d+(?=\s*\))' || echo "")
                local new_version=$(echo "$diff_output" | grep "^+.*PG_REGISTER.*$struct_type" | grep -oP ',\s*\K\d+(?=\s*\))' || echo "")

                if [ -n "$old_version" ] && [ -n "$new_version" ]; then
                    if [ "$new_version" -le "$old_version" ]; then
                        echo "    ❌ Version NOT incremented ($old_version → $new_version)"

                        # Find line number of PG_REGISTER
                        local line_num=$(git show $HEAD_COMMIT:"$file" | grep -n "PG_REGISTER.*$struct_type" | cut -d: -f1 | head -1)

                        # Add to issues list
                        cat >> $ISSUES_FILE << EOF
### \`$struct_type\` ($file:$line_num)
- **Struct modified:** Field changes detected
- **Version status:** ❌ Not incremented (version $version)
- **Recommendation:** Review changes and increment version if needed

EOF
                    else
                        echo "    ✅ Version incremented ($old_version → $new_version)"
                    fi
                elif [ -z "$old_version" ] || [ -z "$new_version" ]; then
                    # Couldn't determine version change, but struct was modified
                    echo "    ⚠️  Could not determine if version was incremented"

                    local line_num=$(git show $HEAD_COMMIT:"$file" | grep -n "PG_REGISTER.*$struct_type" | cut -d: -f1 | head -1)

                    cat >> $ISSUES_FILE << EOF
### \`$struct_type\` ($file:$line_num)
- **Struct modified:** Field changes detected
- **Version status:** ⚠️  Unable to verify version increment
- **Current version:** $version
- **Recommendation:** Verify version was incremented if struct layout changed

EOF
                fi
            else
                echo "    ✅ Struct unchanged"
            fi
        fi
    done <<< "$pg_registers"
}

# Check each changed file
while IFS= read -r file; do
    check_file_for_pg_changes "$file"
done <<< "$CHANGED_FILES"

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
