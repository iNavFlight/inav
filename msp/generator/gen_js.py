#!/usr/bin/env python3
"""Generate the JavaScript message table, enums and constants from the schema.

The table is the one gen_python.py builds (message_table, enum_tables,
constant_values), serialised as JSON object literals into templates/*.js.in, so
both languages decode from the same resolved layout. The runtime that reads it
is msp/js/mspcodec.js.

Usage:
    python3 gen_js.py [--schema DIR] [--out DIR]
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from gen_python import GenError, MSP_ROOT, Model, constant_values, enum_tables, message_table

TEMPLATE_DIR = Path(__file__).resolve().parent / "templates"


def object_literal(entries: dict) -> str:
    """One key per line, so a schema change shows up as a readable diff."""
    lines = [f"    {json.dumps(key)}: {json.dumps(value)}," for key, value in entries.items()]
    return "{\n" + "\n".join(lines) + "\n}"


def render(template: str, **markers: str) -> str:
    text = (TEMPLATE_DIR / template).read_text()
    for marker, value in markers.items():
        text = text.replace(f"@{marker}@", value)
    return text


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--schema", type=Path, default=MSP_ROOT / "schema")
    ap.add_argument("--out", type=Path, default=MSP_ROOT / "js" / "generated")
    args = ap.parse_args()

    model = Model(args.schema)
    table, counts = message_table(model)
    enums, conditional = enum_tables(model)

    args.out.mkdir(parents=True, exist_ok=True)
    outputs = {
        "messages.js": render("messages.js.in", MESSAGES=object_literal(table)),
        "enums.js": render("enums.js.in", ENUMS=object_literal(enums),
                           CONDITIONAL=object_literal(conditional)),
        "constants.js": render("constants.js.in", CONSTANTS=object_literal(constant_values(model))),
    }
    for filename, text in outputs.items():
        (args.out / filename).write_text(text)
        print(f"wrote {args.out / filename}")
    print(f"  messages: {counts['messages']}  variants: {counts['variants']}  "
          f"not_implemented: {counts['not_implemented']}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except GenError as exc:
        print(f"error: {exc}", file=sys.stderr)
        sys.exit(1)
