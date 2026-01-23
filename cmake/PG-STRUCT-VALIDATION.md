# PG Struct Layout Validation

Build-time validation to detect parameter group struct changes without version increments.

## How It Works

1. **DWARF Debug Info** - Compiler embeds complete struct layout in `.elf` debug sections
2. **Layout Extraction** - `readelf` extracts field offsets and types (no extra dependencies)
3. **Checksum Database** - Stores MD5 of each PG struct's layout
4. **Build Check** - Compares current layout vs database, fails if mismatch without version increment

## Advantages Over Git Hook

✅ **No filename assumptions** - Works even when struct is in `battery_config_structs.h` but PG_REGISTER in `battery.c`

✅ **Catches all layout changes:**
- Field additions/removals (git hook ✓)
- Field reordering same-size types (git hook ✗)
- Type changes with same size (git hook ✗)
- Packing attribute changes (git hook ✗)

✅ **No git required** - Works on fresh clone without history

✅ **Compiler-verified** - Uses same preprocessed view compiler sees

## Setup

### Initial Database Generation

```bash
# Build SITL with debug info (default)
make clean_sitl
make sitl

# Generate initial database
cmake/generate-pg-struct-database.sh build/inav_SITL

# Commit the database
git add cmake/pg_struct_checksums.db
git commit -m "Add PG struct layout database"
```

### Integration Into Makefile

Add to `Makefile`:

```makefile
# After SITL build completes
sitl: $(SITL_TARGET)
	@echo "Validating PG struct layouts..."
	@cmake/check-pg-struct-layouts.sh $(SITL_TARGET) || \
		(echo "ERROR: PG struct layout changed without version increment!" && false)
```

## Workflow

### Normal Development (No PG Changes)

```bash
make sitl
# → Build succeeds, validation passes
```

### Developer Modifies PG Struct (Forgot Version Increment)

```bash
# Edit src/main/blackbox/blackbox.h
# - Remove field from blackboxConfig_s

make sitl
# → Build completes
# → Validation FAILS:
#
#   ❌ ERROR: blackboxConfig_t layout changed but version NOT incremented
#      File: src/main/blackbox/blackbox.c:102
#      ACTION REQUIRED: Increment PG version from 4 to 5
```

### Developer Fixes Version

```bash
# Edit src/main/blackbox/blackbox.c
# Change: PG_REGISTER(blackboxConfig_t, ..., 4);
# To:     PG_REGISTER(blackboxConfig_t, ..., 5);

make sitl
# → Validation passes but warns:
#
#   ✅ Version incremented (4 → 5)
#      Note: Update database with: make update-pg-db
```

### Update Database

```bash
make update-pg-db
# Or manually:
cmake/generate-pg-struct-database.sh build/inav_SITL

git add cmake/pg_struct_checksums.db
git commit -m "Update PG database after blackbox version increment"
```

## Files

- `extract-pg-struct-layout.sh` - Extract struct layout from DWARF debug info
- `check-pg-struct-layouts.sh` - Validate layouts against database
- `generate-pg-struct-database.sh` - Create/update database from current build
- `pg_struct_checksums.db` - Database of struct checksums (committed to git)

## Limitations

- **Size-only for Release Builds** - Release builds strip debug info, can only check sizeof() not layout
- **Requires Debug Build** - Full validation needs `-g` flag (default for SITL)
- **Database Maintenance** - Database must be updated after legitimate version increments

## Alternative: Size-Only Check

For release builds without debug info, fallback to size-only validation:

```bash
# Extract sizes using nm or objdump instead of DWARF
# Less comprehensive but works without debug info
```

## Why Better Than Git Hook?

**Git Hook Issues:**
1. Assumes matching filenames (`blackbox.h` ↔ `blackbox.c`)
2. Fails when struct in `battery_config_structs.h` but PG in `battery.c`
3. Can't detect field reordering
4. Needs full git history

**Build-Time Check:**
1. Works regardless of file names
2. Detects all layout changes
3. Works on fresh clones
4. Validated by compiler preprocessor
