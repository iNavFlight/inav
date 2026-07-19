# Target Directory Split Verification

When splitting a multi-target directory (one `target.h`/`target.c` shared by several board variants via `#if defined(BOARD_X)` blocks) into separate single-board target directories, verify the split preserved behavior for every variant before deleting the original.

## Key Insight: Preprocessor vs Source Analysis

Preprocessor testing validates **what goes into builds**, not **what's in source files**.

```c
#if defined(TARGET_A)
  // This appears in output
#else
  // DEAD CODE - preprocessor ignores, but still in source!
#endif
```

A file can compile identically for every target and still contain dead conditional branches nobody will ever build — those won't show up in a preprocessor diff, only in a source-level read of the file.

## Multi-Tool Verification Strategy

No single tool catches everything:

| Approach | Catches | Misses |
|------|---------|--------|
| Simple conditional-stripping tools (e.g. `unifdef`) | Simple `#if`/`#ifdef` conditionals | Complex boolean expressions |
| `gcc -E` output diff (see [gcc-preprocessing-techniques.md](gcc-preprocessing-techniques.md)) | Functional correctness of what actually compiles | Dead code left behind in source |
| Pattern matching over the source for known violation shapes | Known violation patterns | Complex/novel logic errors |
| Manual cross-validation | Edge cases | Nothing (slowest) |

**Use defense-in-depth:** combine at least a preprocessor diff with a source-level read for dead conditional branches — either one alone misses a different class of bug.

## Workflow

```bash
# 1. Before split - capture a preprocessor baseline for every target variant
for target in TARGET1 TARGET2; do
    arm-none-eabi-gcc -E -D${target} orig/target.h > /tmp/before_${target}.i
done

# 2. Perform the split (e.g. with unifdef, or by hand)

# 3. Re-run the same preprocessor command against each new target directory
#    and diff against the matching /tmp/before_*.i baseline

# 4. Separately, read each new target.h/target.c by eye (or grep for
#    unreachable #if branches) to catch dead code the preprocessor diff
#    can't see, since it only reflects what compiles for THIS target

# 5. Build all resulting targets
make TARGET1 TARGET2 ...
```

## Automation Boundaries

**Safe to automate:**
- Detection/reporting of issues
- Preprocessor comparison
- Pattern matching

**Risky to automate:**
- Removing dead code (nesting complexity makes false positives easy)
- Simplifying boolean expressions

## Related

- [gcc-preprocessing-techniques.md](gcc-preprocessing-techniques.md) — the preprocessor-diff technique this workflow relies on
