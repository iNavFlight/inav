#!/bin/bash
#
# Publish the nightly size report as release assets in the companion
# pr-test-builds repo, keyed by BRANCH (latest-tip pointer, kept for
# backward compatibility) AND by COMMIT SHA (primary — lets PR size-diff
# comparisons use the PR's exact base commit instead of the branch tip),
# then prune old per-commit baselines so the companion repo doesn't grow
# unbounded.
#
# Usage: publish-size-baseline.sh <builds-repo> <branch> <commit-sha> <report-json> [--dry-run]
#   <builds-repo>  companion repo, e.g. iNavFlight/pr-test-builds
#   <branch>       branch the nightly was pushed to (e.g. maintenance-10.x)
#   <commit-sha>   full 40-hex SHA of the pushed commit (workflow_run.head_sha)
#   <report-json>  merged size-report.json artifact
#   --dry-run      print what would be published/pruned without touching GitHub
#
# Env: GH_TOKEN with write access to <builds-repo> (Contents: write).
#
# Per-commit baseline releases carry a machine-readable `branch: <name>`
# first line in their notes so pruning can group them per branch. Tags are
# `size-baseline-<40-hex-sha>`; branch-tip tags are `size-baseline-<branch>`.
#
# Pruning policy: keep the newest KEEP_PER_BRANCH per-commit baselines per
# branch, plus a GLOBAL_CAP safety net for orphaned baselines (deleted
# branches, notes parse failures) so the repo can never grow unbounded.

set -euo pipefail

BUILDS_REPO=${1:?usage: publish-size-baseline.sh <builds-repo> <branch> <commit-sha> <report-json> [--dry-run]}
BRANCH=${2:?usage: publish-size-baseline.sh <builds-repo> <branch> <commit-sha> <report-json> [--dry-run]}
COMMIT_SHA=${3:?usage: publish-size-baseline.sh <builds-repo> <branch> <commit-sha> <report-json> [--dry-run]}
REPORT_JSON=${4:?usage: publish-size-baseline.sh <builds-repo> <branch> <commit-sha> <report-json> [--dry-run]}
DRY_RUN=${5:-}

KEEP_PER_BRANCH=50
GLOBAL_CAP=300

# --- Validate inputs: never trust artifact content or context values before
# they are used to build shell commands or release tags.
if ! [[ "$BRANCH" =~ ^[A-Za-z0-9._/-]{1,100}$ ]]; then
    echo "::error::Invalid branch name in artifact: $BRANCH" >&2
    exit 1
fi
if ! [[ "$COMMIT_SHA" =~ ^[0-9a-f]{40}$ ]]; then
    echo "::error::Invalid commit SHA: $COMMIT_SHA" >&2
    exit 1
fi
if [ ! -f "$REPORT_JSON" ]; then
    echo "::error::Size report not found: $REPORT_JSON" >&2
    exit 1
fi

# publish one baseline release: create once, then only ever replace the
# asset in place (--clobber) so the release/tag stays continuously
# resolvable for concurrent PR comparisons.
publish_asset() {
    local tag="$1" title="$2" notes="$3"
    if gh release view "$tag" --repo "$BUILDS_REPO" >/dev/null 2>&1; then
        gh release upload "$tag" "$REPORT_JSON" --repo "$BUILDS_REPO" --clobber
    else
        gh release create "$tag" "$REPORT_JSON" --repo "$BUILDS_REPO" --prerelease \
            --title "$title" --notes "$notes"
    fi
}

BRANCH_TAG="size-baseline-${BRANCH}"
COMMIT_TAG="size-baseline-${COMMIT_SHA}"

if [ "$DRY_RUN" = "--dry-run" ]; then
    echo "[dry-run] would publish ${BRANCH_TAG} and ${COMMIT_TAG} in ${BUILDS_REPO}"
else
    publish_asset "$BRANCH_TAG" "Size baseline: ${BRANCH}" \
        "Latest per-target flash/RAM size report for ${BRANCH}. Auto-updated on every push. Not for human consumption."
    publish_asset "$COMMIT_TAG" "Size baseline: ${COMMIT_SHA}" \
        "branch: ${BRANCH}
Per-commit per-target flash/RAM size report for ${COMMIT_SHA}. Consumed by PR size-diff comparisons; pruned to the newest ${KEEP_PER_BRANCH} per branch."
fi

# ---------------------------------------------------------------------------
# Prune per-commit baselines
# ---------------------------------------------------------------------------
# Emit <created_at>\t<tag>\t<branch> for every per-commit baseline release,
# newest first (GitHub release listing is newest-first). Branch comes from
# the notes' `branch: <name>` first line; '?' when unparseable. @tsv keeps
# the output raw (gh api has no -r/--raw flag; string filters would
# JSON-quote values).
list_per_commit_baselines() {
    gh api "repos/${BUILDS_REPO}/releases?per_page=100" --paginate \
        --jq '.[] | select(.tag_name | test("^size-baseline-[0-9a-f]{40}$")) |
              [.created_at, .tag_name,
               ((.body // "") | capture("^branch: (?<b>[A-Za-z0-9._/-]+)$") | .b // "?")] | @tsv'
}

prune() {
    local tmp
    tmp=$(mktemp)
    trap 'rm -f "$tmp"' RETURN

    # Pass 1 (NR==FNR): per-branch keep-set — newest KEEP_PER_BRANCH tags per
    # branch. Pass 2: apply the global cap to the kept set in input order.
    # Prints the tags to DELETE (kept per-branch but cut by the cap, or never
    # in the keep-set at all). The API lists newest-first but pagination can
    # interleave pages, so re-sort descending by created_at for a guaranteed
    # newest-first input to the keep-selection.
    list_per_commit_baselines | sort -r > "$tmp"

    awk -F '\t' -v keep="$KEEP_PER_BRANCH" -v cap="$GLOBAL_CAP" '
        NR == FNR {
            if (count[$3] < keep) { count[$3]++; keepTag[$2] = 1 }
            next
        }
        {
            if (keepTag[$2]) {
                if (kept < cap) { kept++; keepFinal[$2] = 1 }
            }
        }
        END {
            for (t in keepTag) if (!keepFinal[t]) print t
        }
    ' "$tmp" "$tmp" | while read -r tag; do
        if [ "$DRY_RUN" = "--dry-run" ]; then
            echo "[dry-run] would prune ${tag}"
        else
            gh release delete "$tag" --repo "$BUILDS_REPO" --yes --cleanup-tag \
                || echo "::warning::failed to prune ${tag}" >&2
        fi
    done
}

prune
