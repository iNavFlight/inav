#!/bin/bash
# Exit 0: checked, 1: potential PG version issue, 2: checker error.
set -euo pipefail
exec python3 "$(dirname "$0")/check-pg-versions.py"
