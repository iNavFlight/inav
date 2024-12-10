# Release Process

The following documents the release process used with mcuboot.

## Version numbering

MCUboot uses [semantic versioning][semver], where version numbers
follow a MAJOR.MINOR.PATCH format with the following guidelines on
incremeting the numbers:

1. MAJOR version when you make incompatible API changes,
2. MINOR version when you add functionality in a backwards-compatible
   manner, and
3. PATCH version when you make backwards-compatible bug fixes.

We add pre-release tags of the format MAJOR.MINOR.PATCH-rc1.

We mark in documentation an MCUBoot development version using
the format MAJOR.MINOR.PATCH-dev.

## Release Notes

Before making a release, be sure to update the `docs/release-notes.md`
to describe the release.  This should be a high-level description of
the changes, not a list of the git commits.

## Release Candidates

Prior to each release, tags are made (see below) for at least one
release candidate (a.b.c-rc1, followed by a.b.c-rc2, etc, followed by
the official a.b.c release).  The intent is to freeze the code for a
time, and allow testing to happen.

During the time between rc1 and the final release, the only changes
that should be merged into master are those to fix bugs found in the
rc and Mynewt metadata as described in the next section.

## imgtool release

imgtool is released through pypi.org (The Python package index) and
requires that its version to be updated by editing
`scripts/imgtool/__init__.py` and modifying the exported version:

`imgtool_version = "X.Y.ZrcN"`

where `rcX`, `aX` and `bX` are accepted pre-release versions (just
numbers for final releases). For more info see:

https://www.python.org/dev/peps/pep-0440/#pre-releases

## Mynewt release information

On Mynewt, `newt` always fetches a versioned MCUBoot release, so after
the rc step is finished, the release needs to be exported by modifying
`repository.yml` in the root directory; it must be updated with the
new release version, including updates to the pseudo keys
(`*-(latest|dev)`).

## Tagging and Release

To make a release, make sure your local repo is on the tip version by
fetching from origin.  Typically, the releaser should create a branch
named after the particular release.

Create a commit on top of the branch that modifies the version number
in the top-level `README.md`, and create a commit, with just this
change, with a commit text similar to &ldquo;Bump to version
a.b.c&rdquo;.  Having the version bump helps to make the releases
easier to find, as each release has a commit associated with it, and
not just a tag pointing to another commit.

Once this is done, the release should create a signed tag:
``` bash
git tag -s va.b.c-rcn
```
with the appropriate tag name.  The releaser will need to make sure
that git is configured to use the proper signing key, and that the
public key is signed by enough parties to be trusted.

At this point, the tag can be pushed to github to make the actual
release happen:
``` bash
git push origin HEAD:refs/heads/master
git push origin va.b.c-rcn
```

## Post release actions

Mark the MCUBoot version as a development version. The version number used
should be specified for the next expected release.
It should be larger than the last release version by incrementing the MAJOR or
the MINOR number. It is not necessary to define the next version precisely as
the next release version might still be different as it might be needed to do:

- a patch release
- a MINOR release while a MAJOR release was expected
- a MAJOR release while a MINOR release was expected


[semver]: http://semver.org/
