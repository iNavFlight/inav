# Creating INAV Releases

This document describes the process for creating INAV firmware and configurator releases.

> **Note:** This document is designed to be used with coding assistants (such as Claude Code) that can execute the commands and automate parts of the release process. Update this document with lessons learned after each release.

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
- Firmware: `CMakeLists.txt` (line ~54): `project(INAV VERSION X.Y.Z)`
- Configurator: `package.json`: `"version": "X.Y.Z"`

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

```
1. Verify release readiness
   ├── All PRs merged
   ├── CI passing
   └── Version numbers updated

2. Update SITL binaries in Configurator
   ├── Download from nightly or build for each platform
   └── Commit updated binaries to configurator repo

3. Create tags
   ├── inav: git tag <version>
   └── inav-configurator: git tag <version>

4. Generate changelog
   ├── List PRs since last tag
   ├── Categorize changes
   └── Format release notes

5. Download/build artifacts
   ├── Firmware: from nightly builds
   └── Configurator: from CI artifacts

6. Create draft releases
   ├── Upload firmware artifacts
   ├── Upload configurator artifacts
   └── Add release notes

7. Review and publish
   ├── Maintainer review
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

for f in *.hex; do
  target=$(echo "$f" | sed -E 's/inav_[0-9]+\.[0-9]+\.[0-9]+_(.*)_ci-.*/\1/')
  version=$(echo "$f" | sed -E 's/inav_([0-9]+\.[0-9]+\.[0-9]+)_.*/\1/')
  if [ -n "$RC_NUM" ]; then
    mv "$f" "inav_${version}_${RC_NUM}_${target}.hex"
  else
    mv "$f" "inav_${version}_${target}.hex"
  fi
done
```

### Configurator Builds

Download from GitHub Actions CI:

```bash
# List recent workflow runs
gh run list --repo iNavFlight/inav-configurator --limit 10

# Download artifacts
gh run download <run-id> --repo iNavFlight/inav-configurator

# Flatten directory structure
find . -mindepth 2 -type f -exec mv {} . \;
rm -rf */
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
