# Cleaning up pr-test-builds releases

`.github/workflows/pr-test-builds.yml` publishes a test firmware build to
[iNavFlight/pr-test-builds](https://github.com/iNavFlight/pr-test-builds) for every open
PR (tag `pr-<number>`, e.g. `pr-11675`). Nothing used to delete these once the PR closed,
so the repo accumulated releases forever (157 at the time this was written). Three
mechanisms now keep it clean:

1. **`cleanup-pr-test-builds.yml`** — deletes a PR's release immediately when the PR
   merges. Primary mechanism, handles the common case.
2. **`cleanup-pr-test-builds-scheduled.yml`** — a daily cron sweep that runs the script
   below with `--older-than 14`. Safety net for anything (1) missed, and gradually clears
   the pre-existing backlog.
3. **`.github/scripts/cleanup-old-pr-test-builds.py`** — the script the scheduled sweep uses
   for its logic, also runnable by hand. The merge-triggered workflow has its own simpler
   inline `gh` logic instead, since it only ever handles the single-release, definitely-merged
   case — the two independent delete-a-release implementations should be kept in sync by hand
   if either changes.

Only `inav` needs this — `inav-configurator`'s PR test builds are plain GitHub Actions
artifacts linked from a PR comment, not `pr-test-builds` releases, so they expire under
GitHub's own artifact retention policy with no cleanup needed.

## Token setup

Both workflows use the existing `PR_BUILDS_TOKEN` repository secret (the same one
`pr-test-builds.yml` uses to publish releases) — no new secret is required. It must be a
PAT (fine-grained or classic with `repo` scope) with Contents: write access to
`iNavFlight/pr-test-builds`; the default `GITHUB_TOKEN` can't write to a different repo.

For the local script, export the same kind of token as `GITHUB_TOKEN` or `GH_TOKEN`, or
just use an already-authenticated `gh` CLI (`gh auth login`) — the script shells out to
`gh` rather than talking to the API directly, so it picks up whatever `gh` is configured
to use.

## Running the script manually

```bash
# Preview only, deletes releases for merged PRs
python3 .github/scripts/cleanup-old-pr-test-builds.py --dry-run

# Actually delete releases for merged PRs
python3 .github/scripts/cleanup-old-pr-test-builds.py

# Also sweep releases 14+ days old regardless of PR merge status (open or closed)
python3 .github/scripts/cleanup-old-pr-test-builds.py --dry-run --older-than 14
```

Output looks like:

```
Would delete: pr-11614 (PR merged)
Would delete: pr-11631 (release is 21d old (>= 14d), PR state=OPEN)
Skipped: pr-11675 (PR #11675 state=OPEN, release is 1d old)

Summary: 2 to delete, 1 skipped, 0 errors
```

Any lookup or delete failure is reported as `ERROR:` on stderr and makes the script exit
non-zero — it does not silently skip failures.

## Manually retrying/testing the merge-triggered workflow

`cleanup-pr-test-builds.yml` also accepts `workflow_dispatch`, so you can retry cleanup
for one specific PR (e.g. if the automatic run failed) without waiting for another merge:

```bash
gh workflow run cleanup-pr-test-builds.yml --repo iNavFlight/inav -f pr_number=12345
```

## Troubleshooting

- **"No pr-test-builds release found for PR #N"** in the workflow log is expected and not
  an error — it means that PR never had a successful test build (e.g. all CI runs failed),
  so there's nothing to delete.
- **Auth failures** (workflow): confirm `PR_BUILDS_TOKEN` hasn't expired — it's a PAT, so
  it has an expiry date set when it was created, unlike the auto-rotating `GITHUB_TOKEN`.
- **Auth failures** (local script): run `gh auth status` to check; `gh auth login` or set
  `GITHUB_TOKEN`/`GH_TOKEN`.
- **Rate limiting**: the script makes two `gh` API calls per release (list release info,
  then look up the source PR). For very large backlogs this can approach GitHub's 5,000
  requests/hour authenticated limit; `gh` will report rate-limit errors as they occur — if
  you see them, wait for the reset time shown and re-run. It's idempotent: already-deleted
  releases simply won't appear in the next `gh release list`, so a re-run picks up where it
  left off.

## Limitations

- Only handles `iNavFlight/pr-test-builds` releases tagged `pr-<number>`; anything else in
  that repo is left alone.
- The script assumes every `pr-<number>` tag corresponds to an `iNavFlight/inav` PR. If
  `pr-test-builds` is ever shared with another source repo, the script's hardcoded
  `SOURCE_REPO` constant will need to become configurable.
- `--older-than` deletes based on the release's `published_at` timestamp, not
  `created_at` — the two differ in this repo (see git history / commit message for this
  change if you want the details), and `created_at` is not a reliable age signal here.
- The script lists at most 1000 releases per run (`gh release list --limit 1000`, newest
  first). If the backlog ever exceeds that, the oldest — most overdue — releases would
  silently fall off the list. Not a concern at current scale (~150 releases), but worth
  revisiting (pagination, or `--order asc`) if the repo grows much larger.
