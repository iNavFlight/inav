# Creating INAV Releases

This document describes the process for creating INAV firmware and configurator releases.

> **Note:** This document is designed to be used with coding assistants (such as Claude Code) that can execute the commands and automate parts of the release process. Update this document with lessons learned after each release. Sensei has written more detailed guides for his process in the third-party repo https://github.com/sensei-hacker/inav-claude/tree/master/claude/release-manager

## CRITICAL PRINCIPLE: Verify Builds BEFORE Creating Tags

**Never tag a commit that hasn't been fully tested successfully.**

Order of operations:
1. Merge all firmware PRs to the release branch
2. **Ensure release branch is in nightly-build.yml** (add via PR if not)
3. **Push to release branch to trigger nightly build** (merge the workflow PR, or push trivial commit)
4. Wait for nightly build to complete, verify ALL jobs passed
5. **Download firmware artifacts from inav-nightly** (includes SITL binaries needed for configurator)
6. Update SITL binaries in configurator repo (and WASM SITL + PWA support for 10.x+), wait for CI, merge — this triggers the configurator nightly
7. **Dry-run the configurator signing via the nightly (no tag):** verify its macOS artifact is signed + notarized (`codesign --verify`, `xcrun stapler validate`)
8. Push the configurator version tag (triggers `release.yml`, signed + notarized macOS) — only after step 7's nightly dry-run passes
9. Download the signed configurator artifacts and verify (code signature, SITL, PWA/WASM)
10. **Manual testing on Linux and Windows**
11. **Only then** create the GitHub releases referencing the verified commits/tags

If CI fails or any verification fails, fix the issue first. Do not tag broken commits.

**Why this matters:** If you tag first and then discover the build is broken, you have a tag pointing to a broken commit. By verifying artifacts first, you only tag commits that are proven to work. The configurator is the one partial exception: its signed macOS build requires a tag push, so its "verify first" step is the **nightly dry-run** (step 7) — confirm the nightly macOS artifact is signed before you push the tag.

## CRITICAL: CI Runs on PR Creation, Not Merge

**GitHub Actions CI runs when a PR is created/updated, not when it's merged.**

This means:
- Each PR's CI artifacts only include changes from that PR's branch
- After merging multiple PRs, no single CI run contains all the merged changes
- **The nightly-build workflow must include the release branch to get complete artifacts**

### How Nightly Builds Work

The `nightly-build.yml` workflow triggers on push to specific branches and uploads complete artifacts (hex files + SITL) to the `inav-nightly` repository.

**Ensure the release branch is in the workflow triggers:**

Check `.github/workflows/nightly-build.yml`:
```yaml
on:
  push:
    branches:
      - master
      - maintenance-8.x.x
      - maintenance-9.x    # Add new maintenance branches here!
```

If the maintenance branch is not listed, create a PR to add it.

### Getting Complete Firmware Artifacts

After all PRs are merged to the release branch:

1. **Verify the branch is in nightly-build.yml triggers** (or add it)
2. **Push any commit to the release branch** to trigger the nightly build
   - This can be a trivial change (whitespace, comment) if needed
3. **Wait for the nightly build to complete**
4. **Download from inav-nightly releases:**
   ```bash
   gh release list --repo iNavFlight/inav-nightly --limit 5
   gh release download <tag> --repo iNavFlight/inav-nightly
   ```

Only artifacts from the nightly build contain all merged changes.

## Overview

INAV releases include both firmware (for flight controllers) and the configurator application (for configuration). Both repositories must be tagged with matching version numbers.

**Repositories:**
- Firmware: https://github.com/iNavFlight/inav
- Configurator: https://github.com/iNavFlight/inav-configurator

## Version Numbering

INAV uses semantic versioning: `MAJOR.MINOR.PATCH`

- **MAJOR:** Breaking changes, major new features
- **MINOR:** New features, significant improvements
- **PATCH:** Bug fixes, minor improvements

Version numbers are set in:
- Firmware: in `CMakeLists.txt` via `project(INAV VERSION X.Y.Z)`
  Verify/update:
  - View: `grep -E 'project\\(INAV VERSION' CMakeLists.txt`
  - Update: edit `CMakeLists.txt` to set the desired version
- Configurator: in `package.json` field `"version"`
  Verify/update:
  - View: `jq -r .version package.json` (or `node -p "require('./package.json').version"`)
  - Update: `npm version <X.Y.Z> --no-git-tag-version`

## Version String Format (RC Releases)

**CRITICAL:** Establish the canonical version string before starting any release work.

RC version strings must use **lowercase `rc`** joined to the version with a **hyphen**:

| Correct | Wrong |
|---------|-------|
| `9.1.0-rc1` | `9.1.0-RC1` |
| `9.1.0-rc2` | `9.1.0_RC2` |
| `9.0.0-rc3` | `9.0.0-rc_3` |

The Configurator firmware flasher uses a case-sensitive regex to parse firmware filenames. Uppercase `RC` or underscore separators cause the target board name to be misread, making the firmware invisible in the flasher even after a successful release upload.

## Reserving the Previous Major's Patch Number (New Major Versions Only)

**Applies only the first time a new major's version string is set** (e.g. first setting `10.0.0` on `maintenance-10.x`). Not needed for RC-to-RC or patch releases.

**The hazard:** once the new major's version is set, a later patch fix on the previous major's release branch has to be merged forward into the new major's branch. If that forward-merge happens after the new version was set, it can carry the old branch's version-string commit along and silently overwrite it.

**Before setting the new major's version number:**
1. Bump the patch level on the previous major's release branch — even with no pending fix, to reserve the number. Check each repo's actual next-unused patch number rather than assuming firmware and configurator match (a configurator-only patch release can use up a number on one repo but not the other).
2. Ensure a GitHub milestone exists for that version (and for the new major version) on both repos.
3. Commit, then PR that bump forward into the new major's branch — **never use GitHub's "Resolve conflicts" button** on that PR (see [Maintenance Branches](#maintenance-branches) below for why).
4. Only after that PR merges, set the new major's version number.

The PR that sets the new major's version number should remind the maintainer of this merge order in its description, since GitHub won't enforce it. See the third-party repo linked above for the full procedure and rationale.

---

## Pre-Release Checklist

### Code Readiness

- [ ] All planned PRs merged
- [ ] CI passing on target branch
- [ ] No critical open issues blocking release
- [ ] Version numbers updated in both repositories
- [ ] **GitHub milestone for this version exists on both repos** (create if missing)
- [ ] SITL binaries updated in configurator
- [ ] WASM SITL built and added to configurator `js/web/WASM/` (10.x+)
- [ ] **PG struct validation passed** (see [PG Validation](#pg-parameter-group-validation))

### Documentation

- [ ] Release notes drafted
- [ ] Breaking changes documented
- [ ] New features documented
- [ ] **Configurator migration profile created** for major version bumps (see [Backup Restore Architecture](Backup%20Restore%20Architecture.md#adding-a-new-migration-profile)) — must land in the same PR as the SITL update (step 3 below), **before** the RC configurator build; the prerequisite is the freeze point, not the GitHub draft release, which happens too late (artifacts already built by then)

## Release Workflow

**IMPORTANT:** Verify builds BEFORE creating tags. See "CRITICAL PRINCIPLE" section above.

```
1. Verify firmware release readiness
   ├── All PRs merged to firmware repo
   ├── Version numbers updated
   ├── CI passing on firmware target commit
   └── PG struct validation passed

1.5. Open draft releases with auto-generated notes (both repos)
   ├── gh release create <version> --target <freeze-commit> --draft --prerelease --generate-notes
   ├── No tag required yet (drafts don't create a real tag until published)
   └── This PR list is the source for the changelog (step 6) and the migration profile's settings cross-check

2. Download firmware artifacts FIRST
   ├── Download firmware hex files from CI
   ├── Download SITL binaries from same CI run
   ├── Build Linux x64 SITL locally if needed (for glibc ≤2.35 compatibility)
   ├── Build the WASM SITL firmware (10.x+)
   └── This provides SITL binaries + WASM SITL needed for configurator

3. Update SITL (and WASM SITL) in configurator
   ├── Create PR with SITL binaries from step 2 (and WASM artifacts + SITL-Webassembly.js import fix for 10.x+)
   ├── Add the migration profile here too, for major versions (see Pre-Release Checklist above)
   ├── Wait for configurator CI to pass
   └── Merge SITL update PR

4. Build the PWA, dry-run signing, then push the tag and download configurator artifacts
   ├── Build the PWA (yarn web:build) after the WASM SITL is in place (10.x+)
   ├── Dry-run the signing via the nightly (no tag): codesign --verify + stapler validate on the nightly macOS artifact
   ├── Push the version tag (v*.*.*) to trigger release.yml AFTER the nightly dry-run passes (signed + notarized macOS)
   ├── Download from that release.yml run
   ├── Verify macOS DMGs are signed and have no cross-platform contamination
   ├── Verify Windows SITL (cygwin1.dll present)
   ├── Verify Linux SITL (glibc <= 2.35 for Ubuntu 22.04 compatibility)
   └── Automated SITL verification (glibc check, binary runs)

5. Manual testing
   ├── Test configurator + SITL on Linux
   ├── Test configurator + SITL on Windows
   ├── Test configurator + SITL on macOS (if available)
   └── Verify basic functionality works on each platform
   (The configurator tag is already pushed by now — that's what produced the signed build.
    The firmware tag is still verify-before-tag.)

6. Generate changelog
   ├── List PRs since last tag
   ├── Categorize changes
   └── Format release notes

7. Create draft releases (ONLY after manual testing passed)
   ├── Create tag + draft release for firmware (targeting verified commit) — verify-before-tag
   ├── Create the configurator release referencing the already-pushed tag
   ├── Upload verified artifacts (including the PWA output for 10.x+)
   └── Add release notes

8. Review and publish
   ├── Final review of draft releases
   ├── Maintainer approval
   └── Publish releases
```

## Updating SITL Binaries

SITL binaries must be updated in the configurator repository before release. They are stored in:
```
inav-configurator/resources/public/sitl/
├── linux/
│   ├── inav_SITL
│   └── arm64/inav_SITL
├── macos/
│   └── inav_SITL
└── windows/
    ├── inav_SITL.exe
    └── cygwin1.dll
```

### Download from Nightly

```bash
# Find matching nightly release
gh release list --repo iNavFlight/inav-nightly --limit 5

# Download SITL resources
curl -L -o /tmp/sitl-resources.zip \
  "https://github.com/iNavFlight/inav-nightly/releases/download/<tag>/sitl-resources.zip"
unzip /tmp/sitl-resources.zip -d /tmp/sitl-extract

# Copy to configurator
cd inav-configurator
cp /tmp/sitl-extract/resources/sitl/linux/inav_SITL resources/public/sitl/linux/
cp /tmp/sitl-extract/resources/sitl/linux/arm64/inav_SITL resources/public/sitl/linux/arm64/
cp /tmp/sitl-extract/resources/sitl/macos/inav_SITL resources/public/sitl/macos/
cp /tmp/sitl-extract/resources/sitl/windows/inav_SITL.exe resources/public/sitl/windows/

# Commit
git add resources/public/sitl/
git commit -m "Update SITL binaries for <version>"
```

### Building SITL Locally (Recommended for Linux x64)

**IMPORTANT:** The CI-built Linux x64 SITL binary may require a newer glibc version than Ubuntu 22.04 LTS provides. To ensure compatibility with all supported Ubuntu LTS releases, build the Linux x64 SITL binary locally on Ubuntu 22.04 (glibc 2.35).

```bash
cd inav
mkdir -p build_sitl
cd build_sitl
cmake -DSITL=ON ..
make -j$(nproc)
```

The binary will be at: `build_sitl/bin/SITL.elf`

Verify the glibc requirement:
```bash
objdump -T build_sitl/bin/SITL.elf | grep GLIBC | sed 's/.*GLIBC_//;s/ .*//' | sort -V | tail -1
# Should output 2.35 or lower
```

**When to build locally vs use CI artifacts:**
- **Build locally:** Linux x64 (to ensure glibc ≤ 2.35 compatibility)
- **Use CI artifacts:** Windows (includes cygwin1.dll), macOS, Linux arm64

## Verifying SITL in Packaged Builds

After downloading configurator artifacts, verify SITL files are correctly included.

### Windows SITL Verification

**CRITICAL:** Windows SITL requires `cygwin1.dll` to run. Without it, users get "cygwin1.dll not found" errors.

```bash
# Check Windows zip contains both required files
# Note: Packaged builds use resources/sitl/ (not resources/public/sitl/)
unzip -l INAV-Configurator_win_x64_9.0.0.zip | grep -E "(cygwin1.dll|inav_SITL.exe)"

# Expected output (both files must be present):
#    2953269  12-19-2024 01:41   resources/sitl/windows/cygwin1.dll
#    1517041  12-21-2024 17:25   resources/sitl/windows/inav_SITL.exe
```

If `cygwin1.dll` is missing: **DO NOT release** - Windows SITL will be broken.

### Linux SITL glibc Verification

**CRITICAL:** Linux SITL binaries must be compiled with glibc old enough to support all non-EOL Ubuntu LTS releases.

| Period | Oldest Supported Ubuntu LTS | Required glibc |
|--------|----------------------------|----------------|
| 2025-2027 | Ubuntu 22.04.3 LTS | <= 2.35 |

```bash
# Check glibc version requirement (should output 2.35 or lower)
objdump -T inav_SITL | grep GLIBC | sed 's/.*GLIBC_//;s/ .*//' | sort -V | tail -1
```

If glibc > 2.35, the binary will fail on Ubuntu 22.04 with:
```
/lib/x86_64-linux-gnu/libc.so.6: version `GLIBC_2.38' not found
```

### Path Differences

| Context | SITL Path |
|---------|-----------|
| Source repo | `resources/public/sitl/` |
| Packaged builds | `resources/sitl/` |

The `extraResource` config in `forge.config.js` copies `resources/public/sitl` to `resources/sitl` in packaged builds.

## macOS Code Signing + PWA (10.x+)

### macOS Signing Sequencing

The release-ready macOS build is **code-signed and notarized** by `.github/workflows/release.yml`, which triggers on a **tag push** (`v*.*.*` or `*.*.*`). PR CI never signs; the nightly build signs+notarizes whenever the full secret set is present. Three rules:

1. **Verify signing via the nightly before you tag.** A pushed tag is effectively immutable, so don't tag blind. After the SITL PR merges, the nightly runs automatically — check its macOS artifact with `codesign --verify` and `xcrun stapler validate`. Only push the tag once both pass (if either fails, the six signing secrets are missing and `release.yml` would fail anyway).
2. **Push the tag only AFTER the SITL is in place** — the native SITL binaries (and, for 10.x+, the WASM SITL) must already be committed and merged into the configurator repo. Otherwise the signed `.app`/`.dmg` ships stale SITL.
3. **Never modify the signed macOS artifacts after that run** — no re-zipping, re-bundling, or re-signing. If a fix is needed, commit it and push a new tag.

The code enforces this: SITL pruning runs in the `afterCopyExtraResources` hook (before signing); files must not be deleted from the bundle after signing or notarization fails. See `inav-configurator/CLAUDE.md` ("macOS Code Signing & Notarization").

**If the nightly doesn't fire:** prefer adding `workflow_dispatch:` to `release.yml` and running `gh workflow run release.yml --ref <commit>` (same fail-closed test, no tag). Last resort is a throwaway test tag (`10.0.0-sign-test`, which matches `*.*.*`), verified then deleted before pushing the real `10.0.0-RC1` tag.

### WASM SITL + PWA Build

For 10.x+, the browser-based PWA Configurator build bundles an in-browser WASM build of SITL, in addition to the native per-platform SITL binaries:

1. Build the WASM SITL firmware from `feature/wasm-sitl-firmware` (`cmake .. -DTOOLCHAIN=wasm; make SITL`).
2. Rename the output to `inav_<ver>_WASM.js`/`.wasm` and copy it into `inav-configurator/js/web/WASM/`, updating the hardcoded import in `js/web/SITL-Webassembly.js`.
3. Build the PWA: `yarn web:build` → `dist-web/`.
4. Upload the PWA output as an additional configurator asset (confirm the packaging format with maintainers).

Both the WASM SITL and the native SITL must be in place before the release CI runs.

## Tagging and Publishing

**IMPORTANT:** Tags should only be created AFTER testing artifacts and confirming the release is ready to publish — **except the configurator**, which is tag-first: its signed macOS build is produced by pushing the tag (which triggers `release.yml`). Before pushing that tag, dry-run the signing via the nightly (no tag) and confirm its macOS artifact is signed + notarized. Firmware stays verify-before-tag.

### Check Latest Tags

```bash
# Firmware
cd inav
git fetch --tags
git tag --sort=-v:refname | head -10

# Configurator
cd inav-configurator
git fetch --tags
git tag --sort=-v:refname | head -10
```

### Create and Push Tags

Firmware: verify first, then tag. Configurator: dry-run signing via the nightly first, then tag (to trigger the signed build), then create the release.

```bash
# Firmware (verify artifacts first)
cd inav
git pull
git tag -a <version> -m "INAV <version>"
git push origin <version>

# Configurator — dry-run signing via the nightly FIRST (codesign --verify + stapler validate),
# then push the tag (this push triggers release.yml → signed + notarized macOS)
cd inav-configurator
git pull
git tag -a <version> -m "INAV Configurator <version>"   # or v<version>
git push origin <version>
```

## Changelog Generation

### List PRs Since Last Tag

```bash
cd inav
LAST_TAG=$(git describe --tags --abbrev=0)
gh pr list --state merged --search "merged:>=$(git log -1 --format=%ai $LAST_TAG | cut -d' ' -f1)" --limit 100
```

### Verify Each PR Is on the Correct Branch

**Before including a PR in release notes**, confirm it is actually merged into the release branch, not a future branch. `gh pr list` shows PRs by merge date regardless of target branch — a PR merged to `maintenance-10.x` will appear even though it's not in the current release.

```bash
# Confirm a PR's merge commit exists on the release branch
git log upstream/maintenance-9.x --oneline | grep <short-sha>

# Or check all recent merge commits on the branch
git log upstream/maintenance-9.x --oneline --merges | head -30
```

If a PR is not in that output, exclude it from the release notes.

### Using git log

```bash
LAST_TAG=$(git describe --tags --abbrev=0)
git log $LAST_TAG..HEAD --oneline --merges
```

### Changelog Format

```markdown
## INAV <version> Release Notes

### Firmware Changes

#### New Features
- PR #1234: Description (@contributor)

#### Bug Fixes
- PR #1236: Description (@contributor)

#### Improvements
- PR #1237: Description (@contributor)

### Configurator Changes

#### New Features
- PR #100: Description (@contributor)

### Full Changelog
**Firmware:** https://github.com/iNavFlight/inav/compare/<prev-tag>...<new-tag>
**Configurator:** https://github.com/iNavFlight/inav-configurator/compare/<prev-tag>...<new-tag>
```

## PG (Parameter Group) Validation

**Run before creating tags to prevent EEPROM corruption bugs:**

```bash
cd inav
./cmake/validate-pg-for-release.sh
```
THis builds one target and checks that the parameter group structs haven't been changed without updating their version numbers.

**✅ Pass:** Proceed with release
**❌ Fail:** Create hotfix PR to increment PG version in affected struct's `PG_REGISTER` macro, then re-run


## Downloading Release Artifacts

### Firmware Hex Files

Firmware is available from the nightly build system:

```bash
# List recent nightlies
gh release list --repo iNavFlight/inav-nightly --limit 5

# Download hex files
gh release download <nightly-tag> --repo iNavFlight/inav-nightly --pattern "*.hex"
```

#### Building Firmware Locally (if needed)

**⚠️ Important:** Always use Release mode when building firmware for releases to save disk space:

```bash
cd inav
mkdir build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build all official release targets
make release

# Or build specific targets
make MATEKF405 MATEKF722
```

**Disk usage:** Release mode uses ~4-6 GB vs ~109 GB for default RelWithDebInfo mode (96% reduction). The debug symbols are stripped from final `.hex` files anyway, so Release mode produces identical output.

#### Renaming Firmware Files

Remove CI suffix and add RC number for RC releases:

```bash
RC_NUM="rc2"  # Empty for final releases

# Check if any .hex files exist to avoid errors with the glob
if compgen -G "*.hex" > /dev/null; then
  for f in *.hex; do
    target=$(echo "$f" | sed -E 's/inav_[0-9]+\.[0-9]+\.[0-9]+_(.*)_ci-.*/\1/')
    version=$(echo "$f" | sed -E 's/inav_([0-9]+\.[0-9]+\.[0-9]+)_.*/\1/')
    if [ -n "$RC_NUM" ]; then
      mv "$f" "inav_${version}-${RC_NUM}_${target}.hex"
    else
      mv "$f" "inav_${version}_${target}.hex"
    fi
  done
else
  echo "No .hex files found to rename."
fi
```

### Configurator Builds

Download from GitHub Actions CI:

```bash
# List recent workflow runs
gh run list --repo iNavFlight/inav-configurator --limit 10

# Download artifacts (creates one subdirectory per platform artifact)
gh run download <run-id> --repo iNavFlight/inav-configurator

# CRITICAL: Organize by platform — NEVER flatten all files into one directory.
# Flattening can put Windows .exe files inside macOS DMGs (caused a 9.0.0 release incident).
mkdir -p linux/ macos/ windows/
mv INAV-Configurator_linux_*/* linux/
mv INAV-Configurator_macOS*/* macos/
mv INAV-Configurator_win_*/* windows/
rmdir INAV-Configurator_*
```

## Creating GitHub Releases

### Create Draft Release

For RC releases, add `--prerelease` so GitHub marks them as pre-release and they don't appear as the latest stable release. Use `--target <commit-sha>` to tag a specific commit (safer than tagging the current HEAD, and works even when the local repo is locked).

```bash
# Firmware (RC release)
gh release create 9.1.0-rc1 \
  --repo iNavFlight/inav \
  --target <commit-sha> \
  --title "INAV 9.1.0-rc1 release candidate for testing" \
  --notes-file release-notes.md \
  --prerelease \
  --draft
gh release upload 9.1.0-rc1 firmware-dir/*.hex --repo iNavFlight/inav

# Configurator (RC release)
gh release create 9.1.0-rc1 \
  --repo iNavFlight/inav-configurator \
  --target <commit-sha> \
  --title "INAV Configurator 9.1.0-rc1 release candidate for testing" \
  --notes-file release-notes.md \
  --prerelease \
  --draft
gh release upload 9.1.0-rc1 linux/* macos/* windows/* --repo iNavFlight/inav-configurator

# Final releases: same commands, omit --prerelease
```

### Managing Release Assets

#### Rename Assets via API

```bash
# Get release and asset IDs
gh api repos/iNavFlight/inav/releases --jq '.[] | select(.draft == true) | {id: .id, name: .name}'
gh api repos/iNavFlight/inav/releases/RELEASE_ID/assets --paginate --jq '.[] | "\(.id) \(.name)"'

# Rename an asset
gh api -X PATCH "repos/iNavFlight/inav/releases/assets/ASSET_ID" -f name="new-filename.hex"
```

#### Delete Outdated Assets from Draft Release

If a draft release has outdated assets that need to be replaced (e.g., from a previous upload attempt), delete them before uploading new ones:

```bash
gh api -X DELETE "repos/iNavFlight/inav/releases/assets/ASSET_ID"
```

### Publish Release

**Publish firmware first, then verify the Configurator can see it before publishing the Configurator release.**

```bash
# Step 1: Publish firmware release
gh release edit <version> --repo iNavFlight/inav --draft=false
```

**Step 2: Verify firmware appears in Configurator Firmware Flasher (human step)**

Open INAV Configurator → Firmware Flasher tab → enable "Show unstable releases". The new firmware version must appear in the release list. This confirms the GitHub release is properly formatted and the filename regex parsed correctly.

Also select a target whose name contains spaces (e.g., `MAMBAH743 2022B GYRO2`) and confirm it displays with spaces, not underscores — this validates that multi-word target names parsed correctly.

If the firmware does not appear: check that filenames follow `inav_<version>-rc<n>_<TARGET>.hex` exactly (lowercase `rc`, hyphen separator). See [Asset Naming Conventions](#asset-naming-conventions).

```bash
# Step 3: Publish configurator release (only after firmware verified in flasher)
gh release edit <version> --repo iNavFlight/inav-configurator --draft=false
```

## Asset Naming Conventions

**Firmware (RC releases):** `inav_<version>-rc<n>_<TARGET>.hex`
**Firmware (final):** `inav_<version>_<TARGET>.hex`

**Configurator (RC releases):** `INAV-Configurator_<platform>_<version>-rc<n>.<ext>`
**Configurator (final):** `INAV-Configurator_<platform>_<version>.<ext>`

## Maintenance Branches

When releasing a new major version, create maintenance branches:

- **maintenance-X.x** - For bugfixes to version X
- **maintenance-(X+1).x** - For breaking changes targeting the next major version

**When to create the next major's branch:** the first RC of a new major version is a good time (e.g. create `maintenance-11.x` around `10.0.0-RC1`). Once a version enters RC/stabilization, its maintenance branch should take only fixes; new breaking work goes to the next major's branch instead.

### Creating Maintenance Branches

```bash
COMMIT_SHA="<full-40-char-sha>"

# inav
gh api repos/iNavFlight/inav/git/refs -f ref="refs/heads/maintenance-9.x" -f sha="$COMMIT_SHA"

# inav-configurator
gh api repos/iNavFlight/inav-configurator/git/refs -f ref="refs/heads/maintenance-9.x" -f sha="$COMMIT_SHA"
```

**Also create a matching GitHub milestone** for the new major version on both repos if one doesn't already exist.

**Also update the PR branch-suggestion workflow** in both repos (`.github/workflows/pr-branch-suggestion.yml`), which comments on PRs targeting `master` to suggest the right version branch — update the branch names it mentions to the current pair (compatible / breaking).

### Branch Usage

- **Changes maintaining backward compatibility** → PR to maintenance-X.x (e.g., maintenance-9.x)
- **Breaking changes** (MSP protocol, settings structure) → PR to maintenance-(X+1).x (e.g., maintenance-10.x)
  - When breaking changes affect CLI settings (renames, removals, value changes), a **Configurator migration profile** must be created. See [Backup Restore Architecture](Backup%20Restore%20Architecture.md#adding-a-new-migration-profile)
- **Master** → NOT a PR target (receives merges only)

Lower version branches are periodically merged into higher version branches (e.g., maintenance-9.x → master → maintenance-10.x).

## Hotfix Releases

For critical bugs discovered after release:

1. Create hotfix branch from release tag
2. Cherry-pick or create fix
3. Tag as `X.Y.Z+1` (patch increment)
4. Build and release following normal process
5. Document as hotfix in release notes

## Post-Release Tasks

- [ ] Announce release (Discord, forums, etc.)
- [ ] Update any pinned issues
- [ ] Monitor for critical bug reports
- [ ] Prepare hotfix if needed
- [ ] Update this document with any lessons learned
