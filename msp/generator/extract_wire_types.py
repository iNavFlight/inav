#!/usr/bin/env python3
"""Vendor the wire support types verbatim from INAV source.

Three payload types are C structs INAV declares itself:

    escSensorData_t   sensors/esc_sensor.h   written to the wire as-is
    ledConfig_t       io/ledstrip.h          a packed bitfield struct
    boxBitmask_t      fc/rc_modes.h          a bitarray sized by CHECKBOX_ITEM_COUNT

A firmware build gets these from INAV's own headers. Everyone else -- the
Arduino library, standalone C users -- needs them too, and the only correct
source is INAV. Reproducing them from the schema means guessing at a layout
that already exists: a bitfield struct becomes an opaque byte blob, and an
unpacked struct has to have its compiler padding hand-reconstructed.

So they are copied, not modelled. Same treatment as bitarray.c/h, which is
what boxBitmask_t is built on.

Usage:
    python3 extract_wire_types.py --inav-dir <path> [--out FILE]
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

SDK_ROOT = Path(__file__).resolve().parent.parent


class ExtractError(Exception):
    pass


# what to lift, and from where
WANTED = [
    ("sensors/esc_sensor.h", [
        (r"typedef struct \{\s*\n(?:.*?\n)*?\} escSensorData_t;", "escSensorData_t"),
    ]),
    ("io/ledstrip.h", [
        (r"#define LED_POS_BITCNT\s+\d+", "LED_POS_BITCNT"),
        (r"#define LED_FUNCTION_BITCNT\s+\d+", "LED_FUNCTION_BITCNT"),
        (r"#define LED_OVERLAY_BITCNT\s+\d+", "LED_OVERLAY_BITCNT"),
        (r"#define LED_COLOR_BITCNT\s+\d+", "LED_COLOR_BITCNT"),
        (r"#define LED_DIRECTION_BITCNT\s+\d+", "LED_DIRECTION_BITCNT"),
        (r"#define LED_PARAMS_BITCNT\s+\d+", "LED_PARAMS_BITCNT"),
        (r"typedef struct ledConfig_s \{\s*\n(?:.*?\n)*?\} __attribute__\(\(packed\)\) ledConfig_t;",
         "ledConfig_t"),
    ]),
    ("fc/rc_modes.h", [
        # boxBitmask_t is sized by CHECKBOX_ITEM_COUNT, the terminator of
        # boxId_e. Taking one without the other is what let the two drift:
        # adding a flight mode silently changes the wire width.
        (r"typedef enum \{\s*\n(?:.*?\n)*?\} boxId_e;", "boxId_e"),
        (r"typedef struct boxBitmask_s \{[^\n]*\} boxBitmask_t;", "boxBitmask_t"),
    ]),
]

HEADER = '''#pragma once
/*
 * Wire support types, copied verbatim from INAV by
 * generator/extract_wire_types.py. Do not edit, and do not reproduce these
 * from the schema: a bitfield struct and a compiler-padded struct cannot be
 * restated without guessing at a layout INAV already defines.
 *
 * A firmware build must NOT include this file -- it already has these from
 * sensors/esc_sensor.h, io/ledstrip.h and fc/rc_modes.h, and two declarations
 * of the same type conflict. msp_msgs.h therefore does not include it either.
 *
 * Everyone else (the Arduino library, standalone C users) includes this before
 * msp_msgs.h.
 */

#include <stdint.h>

#include "bitarray.h"   /* boxBitmask_t is a bitarray */
'''


def extract(inav: Path) -> str:
    out = [HEADER]
    for relative, patterns in WANTED:
        source = inav / "src" / "main" / relative
        if not source.exists():
            raise ExtractError(f"{source} not found")
        text = source.read_text()
        out.append(f"\n/* ---- from src/main/{relative} ---- */\n")
        for pattern, label in patterns:
            match = re.search(pattern, text, re.M)
            if not match:
                raise ExtractError(
                    f"{label} not found in {relative}; INAV moved or changed it")
            out.append(match.group(0).rstrip() + "\n")
    return "\n".join(out)


PROBE = """#include <stdio.h>
#include <stddef.h>
#include "msp_wire_types.h"
int main(void) {
%s  return 0;
}
"""


def derive_layouts(header: Path, out: Path) -> dict:
    """Recover the on-wire layout of the vendored types without restating it.

    escSensorData_t is unpacked, so its padding is the compiler's business:
    a probe is compiled and run to read each field's offset/size via offsetof
    and sizeof. ledConfig_t is a packed bitfield struct, so its bit offsets are
    accumulated from INAV's own LED_*_BITCNT defines (extracted above) in
    declaration order -- the layout gcc-arm produces for a little-endian target,
    which is what INAV builds for. Field names come from INAV's source in both
    cases; nothing here is a hand-guessed layout.
    """
    import json, re, subprocess, tempfile

    text = header.read_text()
    layouts: dict = {}

    # plain struct: names from source, offsets from the compiler
    plain = re.search(r"typedef struct \{\s*\n((?:.*?\n)*?)\} escSensorData_t;", text)
    fields = re.findall(r"^\s*(u?int\d+_t)\s+(\w+);", plain.group(1), re.M)
    probe_lines = "".join(
        f'  printf("escSensorData_t {n} %zu %zu {t}\\n", offsetof(escSensorData_t, {n}),'
        f' sizeof(((escSensorData_t *)0)->{n}));\n'
        for t, n in fields)
    probe_lines += ('  printf("escSensorData_t . %zu %zu -\\n", (size_t)0,'
                    ' sizeof(escSensorData_t));\n')

    with tempfile.TemporaryDirectory() as tmp:
        src = Path(tmp) / "probe.c"
        src.write_text(PROBE % probe_lines)
        binary = Path(tmp) / "probe"
        subprocess.run(["gcc", "-std=c11", "-w", f"-I{header.parent}",
                        str(src), str(header.parent / "bitarray.c"), "-o", str(binary)],
                       check=True)
        result = subprocess.run([str(binary)], capture_output=True, text=True, check=True)

    esc = {"kind": "struct", "fields": []}
    for line in result.stdout.strip().split("\n"):
        _, name, offset, size, ctype = line.split()
        if name == ".":
            esc["size"] = int(size)
        else:
            esc["fields"].append({"name": name, "offset": int(offset),
                                  "size": int(size), "ctype": ctype})
    layouts["escSensorData_t"] = esc

    # bitfield struct: widths come from the extracted BITCNT defines, so the
    # bit offsets are derived from INAV's own numbers rather than assumed
    widths = dict(re.findall(r"#define (LED_\w+_BITCNT)\s+(\d+)", text))
    bits = re.findall(r"^\s*uint16_t\s+(\w+)\s*:\s*(\w+);", text, re.M)
    offset, led = 0, {"kind": "bitfield", "fields": []}
    for name, width_name in bits:
        width = int(widths[width_name])
        led["fields"].append({"name": name, "bit_offset": offset, "bits": width})
        offset += width
    led["size"] = (offset + 7) // 8
    layouts["ledConfig_t"] = led

    out.write_text(json.dumps(layouts, indent=2) + "\n")
    return layouts


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--inav-dir", type=Path, default=SDK_ROOT / "inav")
    ap.add_argument("--out", type=Path,
                    default=SDK_ROOT / "clib" / "support" / "msp_wire_types.h")
    args = ap.parse_args()

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(extract(args.inav_dir))
    print(f"wrote {args.out}")

    layouts = derive_layouts(args.out, args.out.parent / "wire_layouts.json")
    for name, body in layouts.items():
        print(f"  {name}: {len(body['fields'])} fields, {body['size']} bytes "
              f"({body['kind']}, from the compiler)")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except ExtractError as exc:
        print(f"error: {exc}", file=sys.stderr)
        sys.exit(1)
