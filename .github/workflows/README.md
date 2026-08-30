# GitHub Actions Workflows

This directory contains automated CI/CD workflows for the INAV project.

## Active Workflows

### Build and Test

#### `ci.yml` - Build Firmware
**Triggers:** Pull requests (`workflow_call` too, used by `nightly-build.yml`).
Also declares an `on: push:` trigger, but it's currently a no-op — a
`branches:` list containing only a negative pattern (`'!maintenance-8.x.x'`)
matches nothing per GitHub's own docs, confirmed empirically (zero
push-triggered runs of this workflow exist in this repo's history). Direct
push builds happen via `nightly-build.yml` instead (see below).
**Purpose:** Compiles INAV firmware for all targets to verify builds succeed
**Matrix:** 15 parallel build jobs for faster CI

#### `nightly-build.yml` - "Build pre-release"
**Triggers:** `push` to `master`, `maintenance-8.x.x`, `maintenance-9.x`,
`maintenance-10.x`, `release/9.1` — **not** a schedule, despite the
filename. Invokes `ci.yml`'s jobs via `workflow_call`, then publishes a
prerelease to the companion `iNavFlight/inav-nightly` repo.
**Purpose:** Creates nightly development builds for testing, and is the
actual per-push validation + baseline-generation point for
`ci-size-report.yml` (see below) since `ci.yml`'s own push trigger doesn't
fire.

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

#### `ci-size-report.yml` - RAM/Flash Usage Delta PR Comment
**Triggers:** `workflow_run` after "Build firmware" (`ci.yml`, PR builds) or
"Build pre-release" (`nightly-build.yml`, branch-push builds) completes
**Purpose:** Posts/updates a PR comment showing flash and RAM usage delta vs.
the PR's base branch, for 4 representative targets spanning flash/RAM size
tiers (MATEKF405, MATEKF722, MATEKF765, MATEKH743 — note MATEKF722 and
MATEKF765 are both STM32F7 parts; no AT32 target is currently covered).
Surfaces RAM/flash regressions and creep before they become an overflow, on
every PR.

**How it works:**
1. `ci.yml` extracts a small per-target size report (`arm-none-eabi-size`
   on each built `.elf`) right after each build and uploads it as an
   artifact — no second build anywhere in this flow.
2. On pushes to `master`/`maintenance-9.x`/`maintenance-10.x`/`release/9.1`,
   `nightly-build.yml` ("Build pre-release") invokes `ci.yml` via
   `workflow_call` as part of building nightly releases — this already
   produces the size report above at no extra build cost. When that
   completes, `ci-size-report.yml` persists it as a release asset
   (`size-baseline-<branch>`) in the companion `iNavFlight/pr-test-builds`
   repo — the "known good" baseline for that branch, overwritten on every
   push. (`ci.yml`'s *own* `on: push:` trigger is broken — a `branches:`
   list containing only a negative pattern matches nothing per GitHub's
   docs — so this deliberately listens to `nightly-build.yml` instead of
   trying to fix that separately; verified empirically that `ci.yml` alone
   has zero push-triggered runs in this repo's history.)
3. On PR builds, it fetches the PR's base branch's persisted baseline (no
   rebuild), diffs it against the PR's own size report, and posts/updates a
   comment (marker `<!-- pr-size-diff -->`).

**Script:** `.github/scripts/extract-size-report.sh` (size extraction),
`.github/scripts/merge-size-reports.sh` (merges per-shard reports),
`.github/scripts/size-diff-comment.js` (pure diff + markdown rendering,
unit tested in `.github/scripts/size-diff-comment.test.js`)

**Uses the same `PR_BUILDS_TOKEN` secret and `workflow_run` trigger pattern
as `pr-test-builds.yml`** (secrets available even for fork PRs).

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
