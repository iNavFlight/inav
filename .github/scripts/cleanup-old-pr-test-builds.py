#!/usr/bin/env python3
"""Bulk-delete iNavFlight/pr-test-builds releases for merged or stale PRs.

See docs/development/cleanup-pr-test-builds.md for full usage, auth setup, and
troubleshooting. Complements .github/workflows/cleanup-pr-test-builds.yml,
which deletes a release immediately when its PR merges; this script is the
manual/scheduled sweep for anything that workflow missed plus backlog
cleanup.

Usage:
    python3 .github/scripts/cleanup-old-pr-test-builds.py [--dry-run] [--older-than DAYS]

Auth: requires the `gh` CLI, authenticated via the GITHUB_TOKEN (or GH_TOKEN)
environment variable, or a prior `gh auth login`. The token needs Contents:
write access to iNavFlight/pr-test-builds to delete releases (read access is
enough for --dry-run).
"""
import argparse
import json
import re
import subprocess
import sys
from datetime import datetime, timezone

PR_TEST_BUILDS_REPO = "iNavFlight/pr-test-builds"
SOURCE_REPO = "iNavFlight/inav"
TAG_RE = re.compile(r"^pr-(\d+)$")


def gh_json(*args):
    result = subprocess.run(["gh", *args], capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip() or f"gh {' '.join(args)} failed")
    return json.loads(result.stdout)


def list_releases():
    return gh_json(
        "release", "list", "--repo", PR_TEST_BUILDS_REPO,
        "--json", "tagName,publishedAt", "--limit", "1000",
    )


def get_pr_status(pr_number):
    return gh_json(
        "pr", "view", str(pr_number), "--repo", SOURCE_REPO,
        "--json", "state",
    )


def delete_release(tag):
    result = subprocess.run(
        ["gh", "release", "delete", tag, "--repo", PR_TEST_BUILDS_REPO,
         "--cleanup-tag", "--yes"],
        capture_output=True, text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip() or "delete failed")


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--dry-run", action="store_true",
                         help="Print what would be deleted without deleting anything")
    parser.add_argument("--older-than", type=int, metavar="DAYS",
                         help="Also delete releases published DAYS+ ago, regardless of PR merge status")
    return parser.parse_args()


def main():
    args = parse_args()

    if subprocess.run(["gh", "auth", "status"], capture_output=True).returncode != 0:
        sys.exit("gh is not authenticated - set GITHUB_TOKEN/GH_TOKEN or run `gh auth login`")

    try:
        releases = list_releases()
    except RuntimeError as exc:
        sys.exit(f"could not list releases: {exc}")

    now = datetime.now(timezone.utc)
    deleted, skipped, errors = [], [], []

    for release in releases:
        tag = release["tagName"]
        match = TAG_RE.match(tag)
        if not match:
            skipped.append((tag, "tag doesn't match pr-<number>, leaving alone"))
            continue

        pr_number = match.group(1)
        published_at = datetime.fromisoformat(release["publishedAt"].replace("Z", "+00:00"))
        age_days = (now - published_at).days

        try:
            pr = get_pr_status(pr_number)
        except RuntimeError as exc:
            errors.append((tag, f"could not look up PR #{pr_number}: {exc}"))
            continue

        if pr["state"] == "MERGED":
            reason = "PR merged"
        elif args.older_than is not None and age_days >= args.older_than:
            reason = f"release is {age_days}d old (>= {args.older_than}d), PR state={pr['state']}"
        else:
            skipped.append((tag, f"PR #{pr_number} state={pr['state']}, release is {age_days}d old"))
            continue

        if not args.dry_run:
            try:
                delete_release(tag)
            except RuntimeError as exc:
                errors.append((tag, f"delete failed: {exc}"))
                continue

        deleted.append((tag, reason))

    verb = "Would delete" if args.dry_run else "Deleted"
    for tag, reason in deleted:
        print(f"{verb}: {tag} ({reason})")
    for tag, reason in skipped:
        print(f"Skipped: {tag} ({reason})")
    for tag, reason in errors:
        print(f"ERROR: {tag} ({reason})", file=sys.stderr)

    print()
    print(f"Summary: {len(deleted)} {'to delete' if args.dry_run else 'deleted'}, "
          f"{len(skipped)} skipped, {len(errors)} errors")

    if errors:
        sys.exit(1)


if __name__ == "__main__":
    main()
