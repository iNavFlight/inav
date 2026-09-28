# Security Policy

INAV uses a single, organisation-wide security policy for all of its
repositories.

**Please report security vulnerabilities privately — never through public
GitHub issues, pull requests, or Discord.**

- **GitHub (preferred, where enabled):** on this repository's **Security** tab,
  use **Report a vulnerability** if that button is shown.
- **Email (always works):** security@inavflight.com

For the full policy — scope, what to include, our disclosure timeline, and safe
harbour — see the canonical Betaflight security policy:

https://github.com/iNavFlight/inav/.github/blob/main/SECURITY.md

## Supported Versions

The firmware uses semantic versioning, `MAJOR.MINOR.PATCH` (for example
`9.1.0` or `10.0.0-RC1`).

We provide security support for the **2 most recent major releases**. Security fixes
are made on `master` and backported to each supported series as point releases.

| Firmware series          | Security support                             |
| ------------------------ | -------------------------------------------- |
| `9.1.x`                  | ✅ Supported                                  |
| `9.0.x`                  | ✅ Supported                                  |
| `8.x.x` and older        | ❌ Not supported                              |

Older releases are no longer maintained; users should upgrade to a supported release.

See the [organisation-wide policy](https://github.com/iNavFlight/inav/.github/blob/main/SECURITY.md#supported-versions)
for the full support rule.
