# Creating INAV Releases

This document describes the process for creating INAV firmware and configurator releases.

> **Note:** This document is designed to be used with coding assistants (such as Claude Code) that can execute the commands and automate parts of the release process. Update this document with lessons learned after each release.

## CRITICAL PRINCIPLE: Verify Builds BEFORE Creating Tags

**Never tag a commit that hasn't been proven to build successfully.**

Order of operations:
1. Merge all firmware PRs to the release branch
2. **Ensure release branch is in nightly-build.yml** (add via PR if not)
3. **Push to release branch to trigger nightly build** (merge the workflow PR, or push trivial commit)
4. Wait for nightly build to complete, verify ALL jobs passed
5. **Download firmware artifacts from inav-nightly** (includes SITL binaries needed for configurator)
6. Update SITL binaries in configurator repo, wait for CI, merge
7. Download configurator artifacts after SITL update merged
8. Verify all artifacts (automated checks)
9. **Manual testing on Linux and Windows** (required before tagging)
10. **Only then** create tags pointing to the verified commits

If CI fails or any verification fails, fix the issue first. Do not tag broken commits.

**Why this matters:** If you tag first and then discover the build is broken, you have a tag pointing to a broken commit. By verifying artifacts first, you only tag commits that are proven to work.

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

## Pre-Release Checklist

### Code Readiness

- [ ] All planned PRs merged
- [ ] CI passing on master branch
- [ ] No critical open issues blocking release
- [ ] Version numbers updated in both repositories
- [ ] SITL binaries updated in configurator

### Documentation

- [ ] Release notes drafted
- [ ] Breaking changes documented
- [ ] New features documented

## Release Workflow

**IMPORTANT:** Verify builds BEFORE creating tags. See "CRITICAL PRINCIPLE" section above.

```
1. Verify firmware release readiness
   ├── All PRs merged to firmware repo
   ├── Version numbers updated
   └── CI passing on firmware target commit

2. Download firmware artifacts FIRST
   ├── Download firmware hex files from CI
   ├── Download SITL binaries from same CI run
   ├── Build Linux x64 SITL locally if needed (for glibc ≤2.35 compatibility)
   └── This provides SITL binaries needed for configurator

3. Update SITL in configurator
   ├── Create PR with SITL binaries from step 2
   ├── Wait for configurator CI to pass
   └── Merge SITL update PR

4. Download and verify configurator artifacts
   ├── Download from CI run after SITL PR merged
   ├── Verify macOS DMGs (no cross-platform contamination)
   ├── Verify Windows SITL (cygwin1.dll present)
   ├── Verify Linux SITL (glibc <= 2.35 for Ubuntu 22.04 compatibility)
   └── Automated SITL verification (glibc check, binary runs)

5. Manual testing (REQUIRED before creating tags)
   ├── Test configurator + SITL on Linux
   ├── Test configurator + SITL on Windows
   ├── Test configurator + SITL on macOS (if available)
   └── Verify basic functionality works on each platform

6. Generate changelog
   ├── List PRs since last tag
   ├── Categorize changes
   └── Format release notes

7. Create tags and draft releases (ONLY after manual testing passed)
   ├── Create tag + draft release for firmware (targeting verified commit)
   ├── Create tag + draft release for configurator (targeting verified commit)
   ├── Upload verified artifacts
   └── Add release notes

7. Review and publish
   ├── Final review of draft releases
   ├── Maintainer approval
   └── Publish releases
```

## Updating SITL Binaries

SITL binaries must be updated before tagging the configurator. They are stored in:
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

## Tagging

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

### Create New Tags

```bash
# Firmware
cd inav
git checkout master && git pull
git tag -a <version> -m "INAV <version>"
git push origin <version>

# Configurator
cd inav-configurator
git checkout master && git pull
git tag -a <version> -m "INAV Configurator <version>"
git push origin <version>
```

## Changelog Generation

### List PRs Since Last Tag

```bash
cd inav
LAST_TAG=$(git describe --tags --abbrev=0)
gh pr list --state merged --search "merged:>=$(git log -1 --format=%ai $LAST_TAG | cut -d' ' -f1)" --limit 100
```

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

## Downloading Release Artifacts

### Firmware Hex Files

Firmware is available from the nightly build system:

```bash
# List recent nightlies
gh release list --repo iNavFlight/inav-nightly --limit 5

# Download hex files
gh release download <nightly-tag> --repo iNavFlight/inav-nightly --pattern "*.hex"
```

#### Renaming Firmware Files

Remove CI suffix and add RC number for RC releases:

```bash
RC_NUM="RC2"  # Empty for final releases

# Check if any .hex files exist to avoid errors with the glob
if compgen -G "*.hex" > /dev/null; then
  for f in *.hex; do
    target=$(echo "$f" | sed -E 's/inav_[0-9]+\.[0-9]+\.[0-9]+_(.*)_ci-.*/\1/')
    version=$(echo "$f" | sed -E 's/inav_([0-9]+\.[0-9]+\.[0-9]+)_.*/\1/')
    if [ -n "$RC_NUM" ]; then
      mv "$f" "inav_${version}_${RC_NUM}_${target}.hex"
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

# Download artifacts
gh run download <run-id> --repo iNavFlight/inav-configurator

# Flatten directory structure
find . -mindepth 2 -type f -exec mv -t . {} +
# Remove the now-empty subdirectories
find . -mindepth 1 -type d -empty -delete
```

## Creating GitHub Releases

### Create Draft Release

```bash
# Firmware
cd inav
gh release create <version> --draft --title "INAV <version>" --notes-file release-notes.md
gh release upload <version> *.hex

# Configurator
cd inav-configurator
gh release create <version> --draft --title "INAV Configurator <version>" --notes-file release-notes.md
gh release upload <version> *.zip *.dmg *.exe *.AppImage *.deb *.rpm *.msi
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

```bash
gh release edit <version> --draft=false
```

## Asset Naming Conventions

**Firmware (RC releases):** `inav_<version>_RC<n>_<TARGET>.hex`
**Firmware (final):** `inav_<version>_<TARGET>.hex`

**Configurator (RC releases):** `INAV-Configurator_<platform>_<version>_RC<n>.<ext>`
**Configurator (final):** `INAV-Configurator_<platform>_<version>.<ext>`

## Maintenance Branches

When releasing a new major version, create maintenance branches:

- **maintenance-X.x** - For bugfixes to version X
- **maintenance-(X+1).x** - For breaking changes targeting the next major version

### Creating Maintenance Branches

```bash
COMMIT_SHA="<full-40-char-sha>"

# inav
gh api repos/iNavFlight/inav/git/refs -f ref="refs/heads/maintenance-9.x" -f sha="$COMMIT_SHA"

# inav-configurator
gh api repos/iNavFlight/inav-configurator/git/refs -f ref="refs/heads/maintenance-9.x" -f sha="$COMMIT_SHA"
```

### Branch Usage

- **X.x bugfixes** → PR to maintenance-X.x
- **Breaking changes** → PR to maintenance-(X+1).x
- **Non-breaking features** → PR to master

Lower version branches are periodically merged into higher version branches (e.g., maintenance-9.x → maintenance-10.x → master).

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
