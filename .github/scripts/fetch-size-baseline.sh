#!/bin/bash
#
# Fetch the size baseline for a PR's TRUE base commit — the merge-base of
# the PR head and base ref — rather than the base branch's latest tip, so
# the size-diff delta doesn't include unrelated changes merged to the base
# after the PR forked (see the +9,788 B stale-RAM regression on PR #11785).
#
# Strategy, in order:
#   1. Exact: size-baseline-<merge-base-sha> in the companion builds repo.
#   2. Nearest ancestor: walk first-parents of the merge-base (older commits
#      on the base branch) up to MAX_WALK steps, using the first per-commit
#      baseline tag found. Nightly baselines exist only for commits that were
#      actually pushed to the branch, so the walk finds the closest nightly
#      build at-or-before the fork point.
#   3. Otherwise report found=false (caller emits the graceful
#      "no size baseline available" comment path).
#
# The base branch's LATEST-tip baseline is deliberately NOT used as a
# fallback: comparing against it is exactly the stale-delta bug this fixes.
#
# Usage: fetch-size-baseline.sh <main-repo> <builds-repo> <base-ref> <merge-base-sha> <out-dir>
#   <main-repo>      the firmware repo, e.g. iNavFlight/inav
#   <builds-repo>    companion repo holding the baselines, e.g. iNavFlight/pr-test-builds
#   <base-ref>       PR base branch, e.g. maintenance-10.x (validated)
#   <merge-base-sha> full 40-hex SHA of the PR's merge-base commit (validated)
#   <out-dir>        directory to write size-report.json into
#
# Env: GH_TOKEN with read access to both repos (secrets.PR_BUILDS_TOKEN in
# the pr-comment job has it; the companion repo is private).
#
# Prints key=value lines to stdout for the workflow to append to
# $GITHUB_OUTPUT: found, baseline_commit (full SHA), baseline_commit_short,
# baseline_exact (true when the merge-base itself had a stored baseline).

set -euo pipefail

MAIN_REPO=${1:?usage: fetch-size-baseline.sh <main-repo> <builds-repo> <base-ref> <merge-base-sha> <out-dir>}
BUILDS_REPO=${2:?usage: fetch-size-baseline.sh <main-repo> <builds-repo> <base-ref> <merge-base-sha> <out-dir>}
BASE_REF=${3:?usage: fetch-size-baseline.sh <main-repo> <builds-repo> <base-ref> <merge-base-sha> <out-dir>}
MERGE_BASE_SHA=${4:?usage: fetch-size-baseline.sh <main-repo> <builds-repo> <base-ref> <merge-base-sha> <out-dir>}
OUT_DIR=${5:?usage: fetch-size-baseline.sh <main-repo> <builds-repo> <base-ref> <merge-base-sha> <out-dir>}

MAX_WALK=30

if ! [[ "$BASE_REF" =~ ^[A-Za-z0-9._/-]{1,100}$ ]]; then
    echo "::error::Invalid base ref: $BASE_REF" >&2
    exit 1
fi
if ! [[ "$MERGE_BASE_SHA" =~ ^[0-9a-f]{40}$ ]]; then
    echo "::error::Invalid merge-base SHA: $MERGE_BASE_SHA" >&2
    exit 1
fi

mkdir -p "$OUT_DIR"

# Set of per-commit baseline SHAs currently stored in the builds repo.
# One paginated call; tags are built by publish-size-baseline.sh and match
# the strict 40-hex pattern by construction.
list_baseline_shas() {
    gh api "repos/${BUILDS_REPO}/releases?per_page=100" --paginate \
        --jq '.[].tag_name | select(test("^size-baseline-[0-9a-f]{40}$")) | sub("^size-baseline-"; "")'
}

# publish-size-baseline replaces the asset in place (--clobber), which still
# briefly deletes-then-uploads under the hood; a few short retries absorb
# that window instead of misreporting "no baseline available".
download_baseline() { # $1 = 40-hex sha
    local tag="size-baseline-${1}"
    for attempt in 1 2 3; do
        if gh release download "$tag" --repo "$BUILDS_REPO" \
            --pattern size-report.json --dir "$OUT_DIR" 2>/dev/null; then
            return 0
        fi
        sleep 3
    done
    return 1
}

emit_found() { # $1 = 40-hex sha, $2 = true|false (exact)
    echo "found=true"
    echo "baseline_commit=${1}"
    echo "baseline_commit_short=${1:0:7}"
    echo "baseline_exact=${2}"
}

BASELINE_SHAS=$(list_baseline_shas) || true

# Exact merge-base first, then nearest ancestors along the base branch's
# first-parent chain. The loop STARTS at the merge-base itself so a
# transient download failure on the exact commit is retried here instead
# of silently degrading to a nearest-ancestor baseline.
sha="$MERGE_BASE_SHA"
first=1
for _ in $(seq 1 $((MAX_WALK + 1))); do
    if grep -qx "$sha" <<< "$BASELINE_SHAS" && download_baseline "$sha"; then
        if [ "$first" = 1 ]; then
            emit_found "$sha" true
        else
            emit_found "$sha" false
        fi
        exit 0
    fi
    first=0
    parent=$(gh api "repos/${MAIN_REPO}/commits/${sha}" --jq '.parents[0].sha // empty' 2>/dev/null || true)
    if [ -z "$parent" ] || ! [[ "$parent" =~ ^[0-9a-f]{40}$ ]]; then
        break
    fi
    sha="$parent"
done

echo "found=false"
