# Merging a Release Branch into the Next Version

**Use this procedure when:** you want to carry a release branch's changes forward into a newer maintenance branch (e.g. `release/9.1` into `maintenance-10.x`, or any older branch into a newer one).

**Do not use GitHub's "Resolve conflicts" button.** See the warning at the bottom of this document.

---

## The Correct Workflow

### 1. Start from the target (newer) branch

```bash
git fetch upstream
git checkout maintenance-10.x
git pull upstream maintenance-10.x
```

### 2. Create a review branch off the target

Branch from `maintenance-10.x`, not from `release/9.1`. This is the key difference from what GitHub's conflict resolver does.

```bash
git checkout -b merge/9.1-into-10.x
```

### 3. Merge the older branch in

```bash
git merge upstream/release/9.1
```

If there are no conflicts, git produces a merge commit automatically. If there are conflicts:

```bash
# For each conflicting file, edit it directly — fix only the specific lines
# Do NOT copy the entire file from either branch
git add src/main/flight/servos.c   # (repeat for each resolved file)
git commit
```

The merge commit lands here on `merge/9.1-into-10.x`. `release/9.1` is never touched.

### 4. Push and open a PR

```bash
git push upstream merge/9.1-into-10.x
```

Open a PR: **`merge/9.1-into-10.x` → `maintenance-10.x`**

This PR will have no conflicts (all conflicts were resolved in step 3). It can be reviewed and merged normally through GitHub's UI.

### 5. GitHub closes the original PR automatically

Once `maintenance-10.x` contains the merge, GitHub detects that all commits from `release/9.1` are reachable from `maintenance-10.x` and closes the original `release/9.1` → `maintenance-10.x` PR as merged. If it doesn't auto-close, close it manually with a comment linking to the merge PR.

---

## Why This Works

The merge commit is on `merge/9.1-into-10.x` (and then `maintenance-10.x` after the PR merges). `release/9.1` receives nothing. Changes flowed in the correct direction: older → newer.

```
maintenance-10.x:  ... ── A ── B ──────────────── M   ← merge commit lands here
                              \                  /
merge/9.1-into-10.x:          \── (conflicts resolved here)
                                \
release/9.1:        ... ── C ── D ── E            ← completely unchanged
```

---

## ⚠️ Why Not GitHub's "Resolve conflicts" Button

GitHub's documentation states:

> "When you resolve a merge conflict on GitHub, the **entire base branch** of your pull request is merged into the head branch."

For a PR from `release/9.1` → `maintenance-10.x`, the base is `maintenance-10.x` and the head is `release/9.1`. Clicking "Resolve conflicts" merges ALL of `maintenance-10.x` into `release/9.1` — the wrong direction. The resulting commit will be named "Merge branch 'maintenance-10.x' into release/9.1" and look routine, but it has contaminated the older release branch with months of newer development.

The procedure above avoids this entirely by never touching `release/9.1`.
