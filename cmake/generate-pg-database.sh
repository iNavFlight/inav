#!/bin/bash
#
# Generate PG struct sizes database with versions
# Extracts sizes from binary and versions from source code
#
# Usage: generate-pg-database.sh <elf_file> > pg_struct_sizes.db
#

set -euo pipefail

ELF_FILE=$1
SCRIPT_DIR=$(dirname "$0")

if [ ! -f "$ELF_FILE" ]; then
    echo "Error: ELF file not found: $ELF_FILE" >&2
    exit 1
fi

echo "# PG Struct Size Database" >&2
echo "# Format: struct_type size version" >&2
echo "# Generated from: $ELF_FILE" >&2
echo "# Date: $(date -u '+%Y-%m-%d %H:%M:%S UTC')" >&2
echo "" >&2

# Function to get PG version from source code
get_pg_version() {
    local struct_type=$1

    # Remove _t suffix to get config name
    # e.g., blackboxConfig_t -> blackboxConfig
    local config_name="${struct_type%_t}"

    # Search for PG_REGISTER in source files
    # Format: PG_REGISTER_WITH_RESET_TEMPLATE(type, name, pgn, version)
    #     or: PG_REGISTER_WITH_RESET_FN(type, name, pgn, version)
    #     or: PG_REGISTER(type, name, pgn, version)
    local version=$(grep -rh "PG_REGISTER.*$config_name" src/main --include="*.c" 2>/dev/null | \
        grep -oP 'PG_REGISTER[^(]*\([^,]+,[^,]+,[^,]+,\s*\K\d+' | head -1)

    echo "$version"
}

# Extract sizes from binary
TEMP_SIZES=$(mktemp)
"$SCRIPT_DIR/extract-pg-sizes-nm.sh" "$ELF_FILE" 2>/dev/null > "$TEMP_SIZES"

# For each struct, add version from source
while read -r struct_type size; do
    [ -z "$struct_type" ] && continue

    version=$(get_pg_version "$struct_type")

    if [ -z "$version" ]; then
        echo "Warning: Cannot find PG version for $struct_type" >&2
        version="0"
    fi

    # Output: struct_type size version
    printf "%-30s %3s %s\n" "$struct_type" "$size" "$version"

done < "$TEMP_SIZES"

rm -f "$TEMP_SIZES"
