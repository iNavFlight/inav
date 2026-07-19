# GCC Preprocessing Techniques

## Overview

When using `arm-none-eabi-gcc -E` for preprocessing C code (e.g., for verification or comparison), the output can be extremely verbose with system headers expanded to thousands of lines. This document covers techniques to reduce and filter the output.

## Problem

Default GCC preprocessing includes:
- All `#include` files expanded recursively (system headers, CMSIS, etc.)
- Line markers (`# 1 "file.h"`) showing file origins
- Blank lines from removed preprocessor directives
- Output can be 100x-1000x larger than the original file

## Techniques

### 1. Suppress Line Markers with `-P`

**Best for most use cases**

```bash
arm-none-eabi-gcc -E -P -DTARGET -DSTM32F405xx \
  -Isrc/main \
  -Ilib/main/STM32F4/Drivers/CMSIS/... \
  target.h
```

**What it does:**
- Removes all `# line` directives (e.g., `# 1 "file.h"`, `# 123 "stm32f4xx.h"`)
- Keeps only the actual preprocessed code
- Reduces output by 50-70% typically

**Example:**
```bash
# Without -P: 10,000 lines
# With -P: 3,000 lines
```

### 2. Remove Blank Lines

Combine `-P` with `grep` to remove blank lines:

```bash
arm-none-eabi-gcc -E -P ... target.h | grep -v '^[[:space:]]*$'
```

Reduces output by another 20-30%.

### 3. Filter to User Code Only

Show only lines from your source files, not system headers:

```bash
arm-none-eabi-gcc -E ... target.h | \
  grep -E '^(# .*target\.(h|c)|[^#])'
```

**What it does:**
- Keeps line markers only for target files (`# 1 "target.h"`)
- Keeps all non-line-marker lines (actual code)
- Removes system header markers

### 4. Show Only Macro Definitions

Use `-dM` to dump only `#define` macros:

```bash
arm-none-eabi-gcc -E -dM -DTARGET ... target.h
```

**Output:**
```c
#define __GNUC__ 10
#define TARGET_BOARD_IDENTIFIER "OBF4"
#define STM32F405xx 1
```

**Filter to specific defines:**
```bash
arm-none-eabi-gcc -E -dM ... target.h | \
  grep -E '^#define (TARGET|DYSF4|OMNIBUS)'
```

### 5. Suppress Header Expansion

Use `-H` to see which headers are included without expanding them:

```bash
arm-none-eabi-gcc -H -DTARGET ... target.h 2>&1 | head -20
```

Shows dependency tree without full expansion.

### 6. Limit to Specific Sections

Extract only specific sections of code:

```bash
# Show only lines between specific markers
arm-none-eabi-gcc -E -P ... target.h | \
  sed -n '/START_MARKER/,/END_MARKER/p'
```

## Recommended Approach for Verification Scripts

For comparing preprocessed output before/after changes:

```bash
# 1. Preprocess with -P (no line markers)
arm-none-eabi-gcc -E -P -DTARGET ... target.h > output.i

# 2. Remove blank lines and normalize whitespace
grep -v '^[[:space:]]*$' output.i > output_clean.i

# 3. Compare
diff -u before.i after.i
```

**Alternative: Just compare effective content**
```bash
# Normalize and compare in one step
diff -u \
  <(arm-none-eabi-gcc -E -P ... before.h | grep -v '^[[:space:]]*$') \
  <(arm-none-eabi-gcc -E -P ... after.h | grep -v '^[[:space:]]*$')
```

## Performance Tips

### Redirect stderr

Warnings can add significantly to output:

```bash
# Suppress warnings
arm-none-eabi-gcc -E -P ... target.h 2>/dev/null

# Or redirect to file
arm-none-eabi-gcc -E -P ... target.h 2>warnings.log
```

### Use -w to disable warnings

```bash
arm-none-eabi-gcc -E -P -w ... target.h
```

### Limit output with head

```bash
# For debugging, show only first 100 lines
arm-none-eabi-gcc -E -P ... target.h 2>&1 | head -100
```

## Common Pitfalls

### 1. Path-specific line markers

Line markers contain full paths which differ between before/after:
```c
# 1 "/path/to/OMNIBUSF4/target.h"
# 1 "/path/to/DYSF4/target.h"
```

**Solution:** Normalize paths or use `-P`

### 2. Macro redefinition warnings

When comparing, you may see warnings about macros being redefined on command line. These are noise.

**Solution:** Use `2>&1 | grep -v 'warning.*redefined'`

### 3. Empty output

If you see empty output:
- Check file paths are correct
- Ensure target file doesn't have `#pragma once` as the only content
- Verify includes are available

## Example: Target Split Verification

When splitting one multi-board target directory into several single-board ones, preprocess both the original and each new target with its own `-D` define and diff the results — this catches any variant whose behavior actually changed, even if the source edit looked correct by eye:

```bash
#!/bin/bash
TARGET="NEWTARGET"
BEFORE_DIR="src/main/target/ORIGINAL_MULTI_TARGET"
AFTER_DIR="src/main/target/NEWTARGET"

GCC_CMD="arm-none-eabi-gcc -E -P -w -D${TARGET} -DSTM32F405xx \
  -Isrc/main \
  -Ilib/main/STM32F4/Drivers/CMSIS/Device/ST/STM32F4xx/Include \
  -Ilib/main/STM32F4/Drivers/CMSIS/Include"

# Preprocess both versions
${GCC_CMD} ${BEFORE_DIR}/target.h | grep -v '^[[:space:]]*$' > /tmp/before.i
${GCC_CMD} ${AFTER_DIR}/target.h | grep -v '^[[:space:]]*$' > /tmp/after.i

# Compare
if diff -q /tmp/before.i /tmp/after.i >/dev/null; then
    echo "✅ Files are identical"
else
    echo "❌ Files differ:"
    diff -u /tmp/before.i /tmp/after.i | head -50
fi
```

See [target-split-verification.md](target-split-verification.md) for the broader multi-tool verification strategy this technique fits into.

## Summary

| Technique | Use Case | Output Reduction |
|-----------|----------|------------------|
| `-P` | Remove line markers | 50-70% |
| `grep -v '^[[:space:]]*$'` | Remove blank lines | 20-30% |
| `-dM` | Show only defines | 95%+ |
| `-w` | Suppress warnings | Varies |
| `2>/dev/null` | Suppress stderr | Varies |
| Filter with grep | User code only | 80-90% |

**Most common combination:**
```bash
arm-none-eabi-gcc -E -P -w ... target.h 2>&1 | grep -v '^[[:space:]]*$'
```

This gives clean, comparable output with minimal noise.
