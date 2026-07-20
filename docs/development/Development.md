# Development

This document is primarily for developers only.

## General principals

1. Name everything well.
2. Strike a balance between simplicity and not-repeating code.
3. Methods that return a boolean should be named as a question, and should not change state.  e.g. 'isOkToArm()'
4. Methods that start with the word 'find' can return a null, methods that start with 'get' should not.
5. Methods should have verb or verb-phrase names, like `deletePage` or `save`.  Variables should not, they generally should be nouns.  Tell the system to 'do' something 'with' something.  e.g. deleteAllPages(pageList).
6. Keep methods short - it makes it easier to test.
7. Don't be afraid of moving code to a new file - it helps to reduce test dependencies.
8. Avoid noise-words in variable names, like 'data' or 'info'.  Think about what you're naming and name it well.  Don't be afraid to rename anything.
9. Avoid comments that describe what the code is doing, the code should describe itself. Comments are useful however for big-picture purposes and to document content of variables — see "Comments" below.
10. If you need to document a variable do it at the declaration, don't copy the comment to the `extern` usage since it will lead to comment rot.
11. Seek advice from other developers - know you can always learn more.
12. Be professional - attempts at humor or slating existing code in the codebase itself is not helpful when you have to change/fix it.
13. Know that there's always more than one way to do something and that code is never final - but it does have to work.

### Comments

Comments should explain WHY, not WHAT — the code itself should describe what it does. Don't add a comment that just restates clear code.

**Redundant (avoid):**
```c
// Increment the counter
counter++;
```

**Useful (keep):**
```c
// Offset by 48 because DShot commands 1-47 are reserved for special commands
throttleValue = rawThrottle + 48;
```

Comment on non-obvious business logic, workarounds for hardware/protocol quirks, and reasons a particular approach was chosen over an alternative. Avoid comments that describe what changed from a previous version ("used to do X") — that belongs in the commit message and PR description, not in code that will be read long after that history is relevant.

Before making any code contributions, take a note of the https://github.com/multiwii/baseflight/wiki/CodingStyle

It is also advised to read about clean code, here are some useful links:

* http://cleancoders.com/
* http://en.wikipedia.org/wiki/SOLID_%28object-oriented_design%29
* http://en.wikipedia.org/wiki/Code_smell
* http://en.wikipedia.org/wiki/Code_refactoring
* http://www.amazon.co.uk/Working-Effectively-Legacy-Robert-Martin/dp/0131177052

### INAV-Specific Guidelines

**Multi-platform support:** INAV supports F4, F7, H7, and AT32 microcontrollers. When working with target-specific code, check `target.h` for pin mappings and hardware configuration, use hardware abstraction layers where possible, and test on SITL before flashing to hardware. F411 targets are excluded from official release builds (flagged `SKIP_RELEASES` in every F411 target's `CMakeLists.txt`) — don't assume they get the same test coverage as F4/F7/H7/AT32.

**Changing settings:** Settings are defined in `src/main/fc/settings.yaml`, not written directly as C code — the build regenerates the C code from the YAML. After editing `settings.yaml`, run:

```bash
python3 src/utils/update_cli_docs.py
```

to regenerate `docs/Settings.md` to match. CI checks that this file is up to date and will fail the build if it's stale. Settings are persisted to EEPROM automatically via the parameter group (PG) system (`src/main/config/parameter_group.h`) — no manual save/load code is needed per setting.

## Unit testing

Ideally, there should be tests for any new code. However, since this is a legacy codebase which was not designed to be tested this might be a bit difficult.

If you want to make changes and want to make sure it's tested then focus on the minimal set of changes required to add a test.

Tests currently live in the `test` folder and they use the google test framework.
The tests are compiled and run natively on your development machine and not on the target platform.
This allows you to develop tests and code and actually execute it to make sure it works without needing a development board or simulator.

This project could really do with some functional tests which test the behaviour of the application.

All pull requests to add/improve the testability of the code or testing methods are highly sought!

Note: Tests are written in C++ and linked with with firmware's C code.

### Running the tests.

The tests and test build system is very simple and based off the googletest example files, it may be improved in due course. From the root folder of the project simply do:

Test are configured from the top level directory. It is recommended to use a separate test directory, here named `testing`.

```
mkdir testing
cd testing
# define NULL toolchain ...
cmake -DTOOLCHAIN= ..
# Run the tests
make check
```

This will build a set of executable files in the `src/test/unit` folder (below `testing`), one for each `*_unittest.cc` file.

After they have been executed by the make invocation, you can still run them on the command line to execute the tests and to see the test report, for example:

```
src/test/unit/time_unittest
Running main() from /home/jrh/Projects/fc/inav/testing/src/test/googletest-src/googletest/src/gtest_main.cc
[==========] Running 2 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 2 tests from TimeUnittest
[ RUN      ] TimeUnittest.TestMillis
[       OK ] TimeUnittest.TestMillis (0 ms)
[ RUN      ] TimeUnittest.TestMicros
[       OK ] TimeUnittest.TestMicros (0 ms)
[----------] 2 tests from TimeUnittest (0 ms total)

[----------] Global test environment tear-down
[==========] 2 tests from 1 test suite ran. (0 ms total)
[  PASSED  ] 2 tests.
```

You can also step-debug the tests in `gdb` (or IDE debugger).

The tests are currently always compiled with debugging information enabled, there may be additional warnings, if you see any warnings please attempt to fix them and submit pull requests with the fixes.

Tests are verified and working with (native) GCC 11.20.

## Using git and github

Ensure you understand the github workflow: https://guides.github.com/introduction/flow/index.html

Please keep pull requests focused on one thing only, since this makes it easier to merge and test in a timely manner.

If you need help with pull requests there are guides on github here:

https://help.github.com/articles/creating-a-pull-request/

The main flow for a contributing is as follows:

1. Login to github, go to the INAV repository and press `fork`.
2. Then using the command line/terminal on your computer: `git clone <url to YOUR fork>`
3. `cd inav`
4. `git checkout maintenance-10.x`
5. `git checkout -b my-new-code`
6. Make changes
7. `git add <files that have changed>`
8. `git commit`
9. `git push origin my-new-code`
10. Create pull request using github UI to merge your changes from your new branch into the appropriate target branch (see "Branching and release workflow" below)
11. Repeat from step 4 for new other changes.

The primary thing to remember is that separate pull requests should be created for separate branches.  Never create a pull request from your `master` branch.

**Important:** Most contributions should target a maintenance branch, not `master`. See the branching section below for guidance on choosing the correct target branch.

Later, you can get the changes from the INAV repo into your version branch by adding INAV as a git remote and merging from it as follows:

1. `git remote add upstream https://github.com/iNavFlight/inav.git`
2. `git checkout maintenance-10.x`
3. `git fetch upstream`
4. `git merge upstream/maintenance-10.x`
5. `git push origin` is an optional step that will update your fork on github


You can also perform the git commands using the git client inside Eclipse.  Refer to the Eclipse git manual.

### Generated files that must be regenerated before pushing

Some files are generated from other source files. If your change touches the source, regenerate the derived file before pushing:

| Source changed | Regenerate with | Output file(s) | CI-enforced? |
|---|---|---|---|
| `src/main/fc/settings.yaml` | `python3 src/utils/update_cli_docs.py` | `docs/Settings.md` | Yes — `.github/workflows/docs.yml` diffs against a freshly regenerated copy and fails the build if stale |
| Source enum headers under `src/main`, or `docs/development/msp/msp_messages.json` | `docs/development/msp/gen_docs.sh` | `docs/development/msp/inav_enums.json`, `docs/development/msp/README.md` | No — nothing regenerates or diff-checks these in CI, so a forgotten regeneration will ship stale MSP docs silently |

`msp_messages.json` itself is hand-authored — there is no script that generates it. When a new MSP handler is added to `fc_msp.c`, a corresponding entry must be added to `msp_messages.json` by hand. See `docs/development/msp/README.md` for the full regeneration and versioning rules.

### Force pushing

Never force push to `maintenance-9.x`, `maintenance-10.x`, or any other shared branch — once something is pushed to a shared branch, other people may have already fetched it, and rewriting history breaks their local repos and corrupts CI/PR history. Force pushing (or even an ordinary push) to a branch after its PR has merged is especially damaging: GitHub's "Files changed" tab can keep diffing against that branch's current head, so later pushes can make a merged PR display code that was never actually reviewed. Delete your branch once its PR merges to avoid this entirely.

Force push is only acceptable on your own feature branch, and only if it hasn't been merged and no one else is using it. If unsure, ask before force pushing.

### Resolving merge conflicts

Never blindly accept `git checkout --ours <file>` or `--theirs <file>` for a conflict without verifying the result — both silently discard one side entirely. Understand what each side changed relative to the common ancestor (`git diff <merge-base> <branch> -- <file>`) and construct the resolution so both intents are preserved. (These flags describe `git merge` conflicts; during a `git rebase`, `--ours`/`--theirs` swap meaning.)

This matters most for the generated files above: if `--ours`/`--theirs` is used on `inav_enums.json` or `msp_messages.json` during a conflict, cross-check the result against the actually-merged source (`fc_msp.c` for MSP handlers, the relevant headers for enums) rather than trusting either branch's snapshot — a mechanical `--theirs` resolution has silently dropped real message/enum entries before.

## Branching and release workflow

INAV uses maintenance branches for active development and releases. **`master` is not an active development branch and is not part of the merge flow.** It was retired because merges into it were too often misused — compatible and incompatible changes ended up tangled together on the same branch, which made isolating a clean release nearly impossible. Do not branch from `master`, merge into it, or target it with a pull request.

### Branch Types

#### Maintenance Branches (Current and Next Major Version)

**Current version branch** (e.g., `maintenance-9.x`):
- Used for backward-compatible changes
- Bug fixes, new features, and improvements that don't break compatibility
- Changes here will be included in the next release of the current major version (e.g., 9.1, 9.2)
- Does not create compatibility issues between firmware and configurator within the same major version

**Next major version branch** (e.g., `maintenance-10.x`):
- Used for changes that introduce compatibility requirements
- Breaking changes that would cause issues between different major versions
- New features that require coordinated firmware and configurator updates
- Changes here will be included in the next major version release (e.g., 10.0)

### Choosing the Right Target Branch

When creating a pull request, target the appropriate branch:

**Target the current version branch** (e.g., `maintenance-9.x`) if your change:
- Fixes a bug
- Adds a new feature that is backward-compatible
- Updates documentation
- Adds or updates hardware targets
- Makes improvements that work with existing releases

**Target the next major version branch** (e.g., `maintenance-10.x`) if your change:
- Breaks compatibility with the current major version
- Requires coordinated firmware and configurator updates
- Changes MSP protocol in an incompatible way
- Modifies data structures in a breaking way

### Release Workflow

1. Development occurs on the current version maintenance branch (e.g., `maintenance-9.x`)
2. When ready for release, a release candidate is tagged from the maintenance branch
3. Bug fixes during the RC period continue on the maintenance branch
4. Fixes that also apply to the next major version are carried forward directly to it — see "Propagating Changes Between Branches" below. `master` plays no role in this.
5. The cycle continues with the maintenance branch receiving new changes for the next release

### Propagating Changes Between Branches

Changes needed on both the current and next major version branch flow **directly** from the current branch to the next — lower version to higher version, never through `master`. The reverse direction (merging a newer branch into an older one) is never done — it would drag months of newer development into an old release branch.

**For an individual fix**, cherry-pick the specific commit:

```bash
git checkout maintenance-10.x
git cherry-pick <commit-from-maintenance-9.x>
git push upstream maintenance-10.x
```

**For carrying forward a whole release branch** (e.g. after a point release is cut), don't merge directly into the target branch and push — that skips review. Also never use GitHub's "Resolve conflicts" button for this: for a PR from the older branch into the newer one, that button always merges the *newer* branch into the *older* one — the wrong direction, unconditionally, no matter how carefully it's used. Instead, branch off the target branch, merge the older branch into that new branch, and open a PR back to the target, leaving the older branch untouched. See [`merging-release-into-next-version.md`](merging-release-into-next-version.md) for the full procedure.

### Example Timeline

**Current state (example - during 9.x series):**
- `maintenance-9.x` - Active development for INAV 9.1, 9.2, etc.
- `maintenance-10.x` - Breaking changes for future INAV 10.0
- `master` - Retired, not part of active development

**After INAV 10.0 is released:**
- `maintenance-10.x` - Becomes active development for INAV 10.1, 10.2, etc.
- `maintenance-11.x` - Breaking changes for future INAV 11.0
- `master` - Still retired

### Working with Maintenance Branches

To branch from the current maintenance branch instead of master:

```bash
# Fetch latest changes
git fetch upstream

# Create your feature branch from the maintenance branch
git checkout -b my-new-feature upstream/maintenance-9.x

# Make changes, commit, and push
git push origin my-new-feature

# Create PR targeting maintenance-9.x (not master)
```

When updating your fork:

```bash
# Get the latest maintenance branch changes
git fetch upstream

# Push directly from upstream to your fork (no local checkout needed)
git push origin upstream/maintenance-9.x:maintenance-9.x
```
