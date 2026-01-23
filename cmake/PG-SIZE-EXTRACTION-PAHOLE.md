# PG Struct Size Extraction Using pahole

## Overview

Extract PG struct sizes from compiled binaries using `pahole` (part of dwarves package).

## Dependencies

```bash
# Install pahole
sudo apt install dwarves
```

## How It Works

1. Build with debug info (default): `make sitl`
2. Binary contains DWARF debug information with complete struct layouts
3. `pahole` reads DWARF and displays struct members, sizes, padding

## Usage

### Extract Single Struct

```bash
pahole -C barometerConfig_s build_sitl/bin/SITL.elf
```

**Output:**
```
struct barometerConfig_s {
	uint8_t                    baro_hardware;        /*     0     1 */

	/* XXX 1 byte hole, try to pack */

	uint16_t                   baro_calibration_tolerance; /*     2     2 */
	float                      baro_temp_correction; /*     4     4 */

	/* size: 8, cachelines: 1, members: 3 */
	/* sum members: 7, holes: 1, sum holes: 1 */
	/* last cacheline: 8 bytes */
};
```

### Extract Just Size

```bash
pahole -C barometerConfig_s build_sitl/bin/SITL.elf | grep "size:"
# Output: /* size: 8, cachelines: 1, members: 3 */

# Extract number only:
pahole -C barometerConfig_s build_sitl/bin/SITL.elf | grep "size:" | grep -oP 'size: \K\d+'
# Output: 8
```

### Extract All PG Struct Sizes

```bash
for struct in barometerConfig_s blackboxConfig_s navConfig_s; do
  size=$(pahole -C "$struct" build_sitl/bin/SITL.elf 2>/dev/null | grep "size:" | grep -oP 'size: \K\d+')
  echo "${struct%_s}_t: ${size:-N/A} bytes"
done
```

**Output:**
```
barometerConfig_t: 8 bytes
blackboxConfig_t: 16 bytes
navConfig_t: 156 bytes
```

## Tested Results

From SITL build on 2026-01-22:

| Struct Type | Size (bytes) | Notes |
|-------------|--------------|-------|
| barometerConfig_t | 8 | 1 byte hole for alignment |
| blackboxConfig_t | 16 | - |
| navConfig_t | 156 | - |

## Advantages

✅ **Detailed layout information** - Shows field offsets, padding, holes
✅ **Any architecture** - Works on ARM, x86-64, etc.
✅ **Reliable** - Uses standard DWARF debug format
✅ **Helps optimization** - Identifies padding that could be eliminated

## Disadvantages

❌ **Extra dependency** - Requires dwarves package (not in standard build tools)
❌ **Requires debug info** - Won't work on stripped/release builds
❌ **Slower** - Parsing DWARF is more expensive than reading symbol table

## Alternative: nm-based approach

For build-time validation without extra dependencies, use `nm` to read symbol sizes directly (see PG-SIZE-EXTRACTION-NM.md).
