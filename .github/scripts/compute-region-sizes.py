#!/usr/bin/env python3
"""Compute per-linker-region byte usage for one .elf, from its companion
.map file (Memory Configuration table) and `arm-none-eabi-size -A` output.

Why: `arm-none-eabi-size -B` (Berkeley format, used for the flat flash/ram
totals elsewhere in this pipeline) sums ALL writable sections into one
"data"/"bss" pair regardless of which physical memory region they're linked
into (e.g. F4/F7 parts split writable memory across RAM and CCM; H7 across
RAM and DTCM). That single combined number is what CI historically reported
as "RAM Delta", which misattributes growth that actually landed in a
different, smaller, often more memory-pressured region. This script instead
matches each section's load address against the target's own linker-defined
memory regions (from its .map file), so the breakdown is exact and requires
no per-MCU-family section-name table to maintain.

Usage: compute-region-sizes.py <elf> <map-file> [size-tool]
Prints a JSON object of {"<region>": <bytes>, ...} to stdout, one entry per
writable region (attributes containing "w") that has nonzero used bytes.
This is informational/non-gating, so any failure (missing/unparsable map,
`size` tool error) prints "{}" and exits 0 rather than aborting the
caller's build-artifact pipeline over a region breakdown it can live
without - callers degrade gracefully to the flat flash/ram figures.
"""
import json
import re
import subprocess
import sys


def parse_memory_regions(map_path):
    """Returns [(name, origin, end), ...] for writable, nonzero-length
    regions, parsed from the map file's "Memory Configuration" table."""
    try:
        with open(map_path, encoding='utf-8', errors='replace') as f:
            text = f.read()
    except OSError:
        return []

    m = re.search(r'^Memory Configuration\s*\n\s*\n[^\n]*\n(.*?)\n\s*\nLinker script and memory map',
                  text, re.MULTILINE | re.DOTALL)
    if not m:
        return []

    regions = []
    for line in m.group(1).splitlines():
        parts = line.split()
        if len(parts) < 3 or parts[0] == '*default*':
            continue
        name, origin_str, length_str = parts[0], parts[1], parts[2]
        attrs = parts[3] if len(parts) > 3 else ''
        try:
            origin = int(origin_str, 16)
            length = int(length_str, 16)
        except ValueError:
            continue
        if length <= 0 or 'w' not in attrs:
            continue
        regions.append((name, origin, origin + length))
    return regions


def parse_section_sizes(elf_path, size_tool):
    """Returns [(section_name, size, addr), ...] via `size -A` (sysv), the
    one format that reports per-section addresses needed for region
    matching (Berkeley's -B only gives family totals, no addresses)."""
    out = subprocess.run([size_tool, '-A', elf_path], capture_output=True, text=True, check=True).stdout
    sections = []
    for line in out.splitlines():
        parts = line.split()
        if len(parts) != 3:
            continue
        name, size_str, addr_str = parts
        if name in ('section', 'Total'):
            continue
        try:
            size, addr = int(size_str), int(addr_str)
        except ValueError:
            continue
        sections.append((name, size, addr))
    return sections


def compute(elf_path, map_path, size_tool):
    regions = parse_memory_regions(map_path)
    if not regions:
        return {}

    usage = {name: 0 for name, _, _ in regions}
    for _section_name, size, addr in parse_section_sizes(elf_path, size_tool):
        # Non-allocated sections (debug info, symbol/string tables, comments)
        # report addr 0 - they're never actually placed in memory, so they
        # must be excluded explicitly rather than relying on address-range
        # matching alone: a region whose own origin is 0x0 (e.g. some parts'
        # ITCM alias) would otherwise false-match every one of them and
        # report several megabytes of phantom "usage".
        if size <= 0 or addr == 0:
            continue
        for name, start, end in regions:
            if start <= addr < end:
                usage[name] += size
                break

    return {name: bytes_ for name, bytes_ in usage.items() if bytes_ > 0}


def main():
    if len(sys.argv) not in (3, 4):
        print('usage: compute-region-sizes.py <elf> <map-file> [size-tool]', file=sys.stderr)
        sys.exit(1)
    elf_path, map_path = sys.argv[1], sys.argv[2]
    size_tool = sys.argv[3] if len(sys.argv) == 4 else 'arm-none-eabi-size'

    try:
        result = compute(elf_path, map_path, size_tool)
    except (OSError, subprocess.CalledProcessError) as e:
        print(f'compute-region-sizes.py: {e} - falling back to no region breakdown', file=sys.stderr)
        result = {}

    print(json.dumps(result))


if __name__ == '__main__':
    main()
