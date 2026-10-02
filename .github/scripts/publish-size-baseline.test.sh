#!/bin/bash
#
# Direct regression test for publish-size-baseline.sh's
# list_per_commit_baselines() jq filter: a release whose notes lack a
# matching `branch: <name>` first line must still emit a full 3-column
# TSV row with "?" in the branch column, not be silently truncated to 2
# columns.
#
# This test extracts the function body verbatim from the real script (via
# sed) rather than duplicating the jq filter, so it exercises the actual
# production code and will catch drift if the filter is edited later.
# `gh` is stubbed to run the extracted filter against a fixture instead of
# hitting the network.
#
# Run: bash .github/scripts/publish-size-baseline.test.sh

set -euo pipefail
cd "$(dirname "$0")"

FN_BODY=$(sed -n '/^list_per_commit_baselines() {/,/^}/p' publish-size-baseline.sh)
if [ -z "$FN_BODY" ]; then
    echo "FAIL: could not extract list_per_commit_baselines() from publish-size-baseline.sh"
    exit 1
fi

MALFORMED_TAG="size-baseline-$(printf 'a%.0s' $(seq 1 40))"
WELLFORMED_TAG="size-baseline-$(printf 'b%.0s' $(seq 1 40))"
# The well-formed body is multi-line, mirroring what publish_asset()
# actually writes (branch line + trailing description) — a single-line
# body would match `^...$` the same with or without the (?m) flag and so
# wouldn't guard against that separate, previously-hit multiline-anchor
# regression.
FIXTURE=$(jq -n --arg mtag "$MALFORMED_TAG" --arg wtag "$WELLFORMED_TAG" '[
    {created_at: "2020-01-01T00:00:00Z", tag_name: $mtag, body: "malformed notes with no branch line"},
    {created_at: "2020-01-02T00:00:00Z", tag_name: $wtag, body: "branch: release/9.1\nPer-commit per-target flash/RAM size report for abc123."}
]')

# Stub for `gh api <url> --paginate --jq FILTER`: run the real filter
# (extracted above) against the fixture instead of calling the network.
gh() {
    local filter=""
    while [ "$#" -gt 0 ]; do
        case "$1" in
            --jq) filter=$2; shift 2 ;;
            *) shift ;;
        esac
    done
    echo "$FIXTURE" | jq -r "$filter"
}
export -f gh

# shellcheck disable=SC2034 -- consumed inside the eval'd function body
BUILDS_REPO="fake-org/fake-builds-repo"
eval "$FN_BODY"
out=$(list_per_commit_baselines)

malformed_row=$(echo "$out" | grep "$MALFORMED_TAG" || true)
wellformed_row=$(echo "$out" | grep "$WELLFORMED_TAG" || true)

fail=0

if [ -z "$malformed_row" ]; then
    echo "FAIL: malformed-notes row is missing from the output entirely"
    fail=1
elif [ "$(awk -F'\t' '{print NF}' <<<"$malformed_row")" != "3" ]; then
    echo "FAIL: malformed-notes row does not have 3 TSV columns: $malformed_row"
    fail=1
elif [ "$(awk -F'\t' '{print $3}' <<<"$malformed_row")" != "?" ]; then
    echo "FAIL: malformed-notes row's branch column is not '?': $malformed_row"
    fail=1
fi

if [ -z "$wellformed_row" ]; then
    echo "FAIL: well-formed row is missing from the output entirely"
    fail=1
elif [ "$(awk -F'\t' '{print $3}' <<<"$wellformed_row")" != "release/9.1" ]; then
    echo "FAIL: well-formed row's branch column is wrong: $wellformed_row"
    fail=1
fi

if [ "$fail" = 0 ]; then
    echo "PASS: list_per_commit_baselines() (2 checks)"
else
    exit 1
fi
