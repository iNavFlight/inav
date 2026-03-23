# GitHub Actions Workflows

This directory contains automated CI/CD workflows for the INAV project.

## Active Workflows

### Build and Test

#### `ci.yml` - Build Firmware
**Triggers:** Pull requests, pushes to maintenance branches
**Purpose:** Compiles INAV firmware for all targets to verify builds succeed
**Matrix:** 15 parallel build jobs for faster CI

#### `nightly-build.yml` - Nightly Builds
**Triggers:** Scheduled nightly
**Purpose:** Creates nightly development builds for testing

### Documentation

#### `docs.yml` - Documentation Build
**Triggers:** Pull requests affecting documentation
**Purpose:** Validates documentation builds correctly

### Code Quality

#### `pg-version-check.yml` - Parameter Group Version Check
**Triggers:** Pull requests to maintenance-9.x and maintenance-10.x
**Purpose:** Detects parameter group struct modifications and verifies version increments
**Why:** Prevents settings corruption when struct layout changes without version bump

**How it works:**
1. Scans changed .c/.h files for `PG_REGISTER` entries
2. Detects if associated struct typedefs were modified
3. Checks if the PG version parameter was incremented
4. Posts helpful comment if version not incremented

**Reference:** See `docs/development/parameter_groups/` for PG system documentation

**Script:** `.github/scripts/check-pg-versions.sh`

**When to increment PG versions:**
- ✅ Adding/removing fields from struct
- ✅ Changing field types or sizes
- ✅ Reordering fields
- ✅ Adding/removing packing attributes
- ❌ Only changing `PG_RESET_TEMPLATE` default values
- ❌ Only changing comments

### Pull Request Helpers

#### `pr-branch-suggestion.yml` - Branch Targeting Suggestion
**Triggers:** PRs targeting master branch
**Purpose:** Suggests using maintenance-9.x or maintenance-10.x instead

#### `non-code-change.yaml` - Non-Code Change Detection
**Triggers:** Pull requests
**Purpose:** Detects PRs with only documentation/formatting changes

## Configuration Files

- `../.github/stale.yml` - Stale issue/PR management
- `../.github/no-response.yml` - Auto-close issues without response
- `../.github/issue_label_bot.yaml` - Automatic issue labeling

## Adding New Workflows

When adding workflows:

1. **Use descriptive names** - Make purpose clear from filename
2. **Document in this README** - Add entry above with purpose and triggers
3. **Set appropriate permissions** - Principle of least privilege
4. **Test in fork first** - Verify before submitting to main repo
5. **Handle errors gracefully** - Don't block CI unnecessarily

### Common Patterns

**Checkout with history:**
```yaml
- uses: actions/checkout@v4
  with:
    fetch-depth: 0
```

**Post PR comments:**
```yaml
- uses: actions/github-script@v7
  with:
    script: |
      await github.rest.issues.createComment({
        owner: context.repo.owner,
        repo: context.repo.repo,
        issue_number: context.issue.number,
        body: 'Comment text'
      });
```

**Run bash scripts:**
```yaml
- run: bash .github/scripts/script-name.sh
  env:
    GITHUB_BASE_REF: ${{ github.base_ref }}
```

## Permissions

Workflows use GitHub's fine-grained permissions:

- `contents: read` - Read repository code
- `pull-requests: write` - Post/update PR comments
- `actions: read` - Read workflow run data

## Local Testing

Scripts in `.github/scripts/` can be run locally:

```bash
cd inav
export GITHUB_BASE_REF=maintenance-9.x
export GITHUB_HEAD_REF=feature-branch
bash .github/scripts/check-pg-versions.sh
```

## References

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Workflow Syntax](https://docs.github.com/en/actions/reference/workflow-syntax-for-github-actions)
- [GitHub Script Action](https://github.com/actions/github-script)
