#!/usr/bin/env bash
# The schema declares a width for each type INAV owns (external_types in
# msp_v2.yaml). Those widths drive the Python codec, so they must match the
# real declarations -- compile the vendored header and check.
set -euo pipefail

MSP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"   # inav/msp
WORK="$(mktemp -d)"
trap 'rm -rf "${WORK}"' EXIT

python3 - "${MSP_ROOT}" > "${WORK}/probe.c" <<'PY'
import sys, yaml, pathlib
root = pathlib.Path(sys.argv[1])
ext = yaml.safe_load((root / "schema" / "msp_v2.yaml").read_text())["external_types"]
print('#include <stdio.h>')
print('#include "msp_wire_types.h"')
print("int main(void){ int bad = 0;")
for name, body in ext.items():
    size = int(body["size"])
    print(f'  if (sizeof({name}) != {size}) {{ bad = 1;')
    print(f'    printf("  {name}: schema says {size}, INAV says %zu\\n", sizeof({name})); }}')
    print(f'  else printf("  {name} = {size} bytes, matches INAV\\n");')
print("  return bad; }")
PY

gcc -std=c11 -w -I "${MSP_ROOT}/c" \
    "${WORK}/probe.c" "${MSP_ROOT}/c/bitarray.c" -o "${WORK}/probe"
"${WORK}/probe"
