#!/bin/bash
#
# Extract PG struct sizes using nm (no extra dependencies)
# Reads pgResetTemplate symbol sizes from symbol table
#
# Usage: extract-pg-sizes-nm.sh <elf_file>
#

set -euo pipefail

ELF_FILE=$1

if [ ! -f "$ELF_FILE" ]; then
    echo "Error: ELF file not found: $ELF_FILE" >&2
    exit 1
fi

echo "Extracting PG struct sizes from $ELF_FILE using nm..." >&2

# Detect architecture for correct nm command
if command -v arm-none-eabi-nm &> /dev/null && [[ $(file "$ELF_FILE") == *"ARM"* ]]; then
    NM_CMD="arm-none-eabi-nm"
else
    NM_CMD="nm"
fi

# Extract pgResetTemplate symbols
# Format: <addr> <size_hex> D pgResetTemplate_<name>
$NM_CMD --print-size "$ELF_FILE" 2>/dev/null | grep "pgResetTemplate_" | \
    while read addr size_hex type symbol; do
        # Extract config name from symbol
        # pgResetTemplate_blackboxConfig -> blackboxConfig
        config_name="${symbol#pgResetTemplate_}"

        # Convert hex size to decimal
        size_dec=$((16#$size_hex))

        # Find corresponding struct type and version from PG_REGISTER
        pg_register_line=$(grep -rh "PG_REGISTER.*$config_name" src/main --include="*.c" 2>/dev/null | head -1)

        struct_type=$(echo "$pg_register_line" | grep -oP 'PG_REGISTER[^(]*\(\K[^,]+' | head -1)

        if [ -z "$struct_type" ]; then
            # Fallback: convert config name to struct type
            # blackboxConfig -> blackboxConfig_t
            struct_type="${config_name}_t"
        fi

        # Extract PG version (4th parameter in PG_REGISTER)
        version=$(echo "$pg_register_line" | grep -oP 'PG_REGISTER[^(]*\([^,]+,[^,]+,[^,]+,\s*\K\d+' | head -1)

        if [ -z "$version" ]; then
            version="0"
        fi

        printf "%-30s %3s %s\n" "$struct_type" "$size_dec" "$version"
    done | sort -u
