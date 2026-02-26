#!/usr/bin/env python3
"""
update_prs.py - Fetch open pull requests from GitHub and update prs.json

Run this via cron 4 times a day, e.g.:
  0 */6 * * * /path/to/pr-build-server/scripts/update_prs.py

Requires: python3, urllib (stdlib only - no extra packages needed)
Optional: Set GITHUB_TOKEN env var to avoid rate limiting (5000 req/hr vs 60/hr)
"""

import json
import os
import sys
import urllib.request
import urllib.error
from datetime import datetime, timezone

REPO_OWNER = "iNavFlight"
REPO_NAME = "inav"
BASE_BRANCH = "maintenance-9.x"

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_DIR = os.path.join(SCRIPT_DIR, "..", "data")
OUTPUT_FILE = os.path.join(DATA_DIR, "prs.json")


def fetch_prs(page=1):
    url = (
        f"https://api.github.com/repos/{REPO_OWNER}/{REPO_NAME}/pulls"
        f"?state=open&base={BASE_BRANCH}&per_page=100&page={page}"
    )
    req = urllib.request.Request(url)
    req.add_header("Accept", "application/vnd.github.v3+json")
    req.add_header("User-Agent", "inav-pr-build-server/1.0")

    token = os.environ.get("GITHUB_TOKEN")
    if token:
        req.add_header("Authorization", f"token {token}")

    with urllib.request.urlopen(req, timeout=30) as resp:
        return json.loads(resp.read().decode())


def main():
    print(f"[{datetime.now(timezone.utc).isoformat()}] Fetching open PRs for "
          f"{REPO_OWNER}/{REPO_NAME} targeting {BASE_BRANCH}...")

    all_prs = []
    page = 1
    while True:
        try:
            prs = fetch_prs(page)
        except urllib.error.HTTPError as e:
            print(f"ERROR: GitHub API returned {e.code}: {e.reason}", file=sys.stderr)
            sys.exit(1)
        except urllib.error.URLError as e:
            print(f"ERROR: Network error: {e.reason}", file=sys.stderr)
            sys.exit(1)

        if not prs:
            break

        for pr in prs:
            all_prs.append({
                "number": pr["number"],
                "title": pr["title"],
                "author": pr["user"]["login"],
                "branch": pr["head"]["ref"],
                "head_sha": pr["head"]["sha"],
                "base_branch": pr["base"]["ref"],
                "url": pr["html_url"],
                "created_at": pr["created_at"],
                "updated_at": pr["updated_at"],
                "draft": pr.get("draft", False),
                "labels": [lbl["name"] for lbl in pr.get("labels", [])],
            })

        if len(prs) < 100:
            break
        page += 1

    # Sort by PR number descending (newest first)
    all_prs.sort(key=lambda p: p["number"], reverse=True)

    os.makedirs(DATA_DIR, exist_ok=True)
    with open(OUTPUT_FILE, "w") as f:
        json.dump({
            "updated_at": datetime.now(timezone.utc).isoformat(),
            "base_branch": BASE_BRANCH,
            "prs": all_prs,
        }, f, indent=2)

    print(f"Wrote {len(all_prs)} open PRs to {OUTPUT_FILE}")


if __name__ == "__main__":
    main()
