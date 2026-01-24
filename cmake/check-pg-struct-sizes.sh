#!/bin/bash
#
# Check PG struct sizes against database
# Fails build if size changed but PG version wasn't incremented
#
# Usage: check-pg-struct-sizes.sh <elf_file>
#

set -euo pipefail

# Validate arguments
if [ $# -ne 1 ]; then
    echo "Error: Missing required argument" >&2
    echo "Usage: $0 <elf_file>" >&2
    exit 2
fi

ELF_FILE=$1
SCRIPT_DIR=$(dirname "$0")

if [ ! -f "$ELF_FILE" ]; then
    echo "Error: ELF file not found: $ELF_FILE" >&2
    exit 1
fi

# Detect architecture from ELF file
ARCH=$(file "$ELF_FILE" | grep -oE "(ARM|x86-64|x86_64)" | head -1 | tr '[:upper:]' '[:lower:]' | sed 's/x86-64/x86_64/')

if [ -z "$ARCH" ]; then
    echo "⚠️  Warning: Cannot detect architecture from $ELF_FILE" >&2
    echo "   Skipping validation." >&2
    exit 0
fi

# Select architecture-specific database
if [ "$ARCH" = "arm" ]; then
    DB_FILE="$SCRIPT_DIR/pg_struct_sizes.arm.db"
elif [ "$ARCH" = "x86_64" ]; then
    DB_FILE="$SCRIPT_DIR/pg_struct_sizes.x86_64.db"
else
    echo "⚠️  Warning: Unsupported architecture: $ARCH" >&2
    echo "   Skipping validation." >&2
    exit 0
fi

if [ ! -f "$DB_FILE" ]; then
    echo "⚠️  Warning: PG struct sizes database not found: $DB_FILE" >&2
    echo "   Run: $SCRIPT_DIR/extract-pg-sizes-nm.sh $ELF_FILE > $DB_FILE" >&2
    echo "   Skipping validation." >&2
    exit 0
fi

echo "🔍 Checking PG struct sizes against database ($ARCH)..."

# Extract current sizes and versions
TEMP_CURRENT=$(mktemp)
"$SCRIPT_DIR/extract-pg-sizes-nm.sh" "$ELF_FILE" 2>/dev/null > "$TEMP_CURRENT"

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

if [ $FAILED -eq 1 ]; then
    echo ""
    echo "❌ PG STRUCT SIZE VALIDATION FAILED"
    echo ""
    echo "The following structs changed size without version increments:"
    echo -e "$ISSUES"
    echo ""
    echo "Fix: Increment PG version in PG_REGISTER for affected structs"
    echo ""
    exit 1
fi

if [ -n "$UPDATED" ]; then
    echo ""
    echo "📝 Database auto-updated for structs with version increments:"
    echo -e "$UPDATED"
    echo ""
fi

echo "✅ All PG struct sizes validated successfully"
exit 0
