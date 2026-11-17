"""Validator tests for the MSP YAML schema. Run: python test_yaml.py

Only needs PyYAML; the generator does not import pydantic.
"""

from __future__ import annotations

import sys
import tempfile
from pathlib import Path

from gen_pydantic import SchemaError, Validator, parse_document

HEADER = "version: 1\npackage: test.doc\n"

passed = 0
failed: list[str] = []


def build(body: str):
    with tempfile.NamedTemporaryFile("w", suffix=".yaml", delete=False) as handle:
        handle.write(HEADER + body)
        path = Path(handle.name)
    try:
        docs = [parse_document(path)]
        Validator(docs).run()
        return docs
    finally:
        path.unlink()


def rejects(label: str, body: str, expect: str = "") -> None:
    global passed
    try:
        build(body)
    except SchemaError as exc:
        if expect and expect not in str(exc):
            failed.append(f"{label}: rejected, but message lacked {expect!r}: {exc}")
            return
        passed += 1
        return
    failed.append(f"{label}: ACCEPTED but should have been rejected")


def accepts(label: str, body: str) -> None:
    global passed
    try:
        build(body)
        passed += 1
    except SchemaError as exc:
        failed.append(f"{label}: REJECTED but should have been accepted: {exc}")


STRUCT = """
structs:
  S:
    fields:
      a: uint8
"""

# --- document -------------------------------------------------------------
rejects("unknown top-level key", "bogus: 1\n" + STRUCT, "unknown keys")
accepts("minimal valid document", STRUCT)

# --- names ----------------------------------------------------------------
rejects(
    "duplicate type name",
    """
enums:
  Dup:
    storage: uint8
    values:
      - A = 0
structs:
  Dup:
    fields:
      a: uint8
""",
    "duplicate type name",
)
rejects(
    "duplicate message id",
    """
messages:
  M_ONE:
    id: 7
    request: null
    reply:
      fields:
        a: uint8
  M_TWO:
    id: 7
    request: null
    reply:
      fields:
        a: uint8
""",
    "duplicate message id",
)

# --- types ----------------------------------------------------------------
rejects("unknown type", "structs:\n  S:\n    fields:\n      a: Nope\n", "unknown type")
rejects(
    "array size neither constant nor field",
    "structs:\n  S:\n    fields:\n      a: uint8[MISSING]\n",
    "neither a constant",
)
accepts(
    "array sized by constant",
    "constants:\n  N: uint8 = 4\nstructs:\n  S:\n    fields:\n      a: uint8[N]\n",
)
accepts(
    "array sized by preceding field",
    "structs:\n  S:\n    fields:\n      n: uint8\n      a: uint8[n]\n",
)
rejects(
    "open array outside message payload",
    "structs:\n  S:\n    fields:\n      a: uint8[]\n",
    "open-ended array",
)

# --- aliases --------------------------------------------------------------
rejects(
    "alias targets a struct",
    STRUCT + "aliases:\n  bad_t: S\n",
    "may not target",
)
rejects(
    "alias cycle",
    "aliases:\n  a_t: b_t\n  b_t: a_t\n",
    "cycle",
)
accepts("alias chain to primitive", "aliases:\n  a_t: b_t\n  b_t: uint32\n")

# --- enums and bitmasks ---------------------------------------------------
rejects(
    "enum value overflows storage",
    "enums:\n  E:\n    storage: uint8\n    values:\n      - A = 300\n",
    "does not fit",
)
rejects(
    "bitmask position out of range",
    "bitmasks:\n  B:\n    storage: uint8\n    values:\n      - A = 9\n",
    "out of range",
)
rejects(
    "duplicate bit position",
    "bitmasks:\n  B:\n    storage: uint8\n    values:\n      - A = 1\n      - C = 1\n",
    "duplicate bit position",
)
rejects(
    "enum storage missing",
    "enums:\n  E:\n    values:\n      - A = 0\n",
    "storage",
)
rejects(
    "forward alias member",
    "enums:\n  E:\n    storage: uint8\n    values:\n      - A = B\n      - B = 1\n",
    "not an earlier member",
)
accepts(
    "duplicate enum values are legal",
    "enums:\n  E:\n    storage: uint8\n    values:\n      - A = 4\n      - B = 4\n",
)
accepts(
    "backward alias member",
    "enums:\n  E:\n    storage: uint8\n    values:\n      - A = 1\n      - B = A\n",
)
accepts(
    "conditional enum member",
    'enums:\n  E:\n    storage: uint8\n    values:\n      - A = 0\n      - if "STM32F4": B = 1\n',
)
_conditional = build(
    'enums:\n  E:\n    storage: uint8\n    values:\n      - if "STM32F4 || STM32F7": B = 1\n')
if _conditional[0].enums[0].members[0].condition == "STM32F4 || STM32F7":
    passed += 1
else:
    failed.append("conditional enum member: condition was not carried through")
rejects(
    "conditional enum member without quotes",
    "enums:\n  E:\n    storage: uint8\n    values:\n      - if STM32F4: A = 1\n",
    'if "COND"',
)
rejects(
    "conditional enum member without a value",
    'enums:\n  E:\n    storage: uint8\n    values:\n      - if "STM32F4": A\n',
    "needs a value",
)
accepts(
    "bitmask zero member",
    "bitmasks:\n  B:\n    storage: uint8\n    zero: NONE\n    values:\n      - A = 0\n",
)
rejects(
    "zero on an enum",
    "enums:\n  E:\n    storage: uint8\n    zero: NONE\n    values:\n      - A = 0\n",
    "only valid on a bitmask",
)
rejects(
    "zero collides with a flag",
    "bitmasks:\n  B:\n    storage: uint8\n    zero: A\n    values:\n      - A = 0\n",
    "duplicate member",
)

rejects(
    "cstring with an array size",
    "structs:\n  S:\n    fields:\n      a: cstring[4]\n",
    "takes no array size",
)
accepts(
    "cstring mid-payload",
    "messages:\n  M:\n    id: 1\n    request: null\n    reply:\n      fields:\n        a: cstring\n        b: uint8\n",
)
accepts(
    "configurable constant as array size",
    "constants:\n  N:\n    type: uint8\n    configurable: TARGET_N\n"
    "structs:\n  S:\n    fields:\n      a: uint8[N]\n",
)
rejects(
    "constant with both value and configurable",
    "constants:\n  N:\n    type: uint8\n    value: 4\n    configurable: TARGET_N\n",
    "exclusive",
)
accepts(
    "optional tail followed by open array",
    "messages:\n  M:\n    id: 1\n    request: null\n    reply:\n      fields:\n        a: uint8\n        b: optional uint8\n        c: optional uint8[]\n",
)
accepts(
    "field with a fixed wire value",
    "structs:\n  S:\n    fields:\n      reserved:\n        type: uint8\n        value: 0\n",
)

# --- fields ---------------------------------------------------------------
rejects(
    "required follows optional",
    "structs:\n  S:\n    fields:\n      a: optional uint8\n      b: uint8\n",
    "contiguous suffix",
)
accepts(
    "optional suffix",
    "structs:\n  S:\n    fields:\n      a: uint8\n      b: optional uint8\n",
)
for banned in ("required", "default", "const"):
    rejects(
        f"'{banned}' key is not in the MSP schema",
        f"structs:\n  S:\n    fields:\n      a:\n        type: uint8\n        {banned}: 1\n",
        "not an MSP schema field key",
    )
rejects(
    "empty fields",
    "structs:\n  S:\n    fields: {}\n",
    "",
)

# --- repeated groups ------------------------------------------------------
accepts(
    "repeated group by constant",
    """
constants:
  N: uint8 = 4
structs:
  S:
    fields:
      g:
        repeat: N
        fields:
          a: uint8
""",
)
rejects(
    "nested repeated group",
    """
constants:
  N: uint8 = 4
structs:
  S:
    fields:
      g:
        repeat: N
        fields:
          h:
            repeat: N
            fields:
              a: uint8
""",
    "may not nest",
)
rejects(
    "until_end outside a payload",
    """
structs:
  S:
    fields:
      g:
        repeat: until_end
        fields:
          a: uint8
""",
    "until_end",
)
accepts(
    "until_end in a message payload",
    """
messages:
  M:
    id: 1
    request: null
    reply:
      repeat: until_end
      fields:
        a: uint8
""",
)

# --- messages -------------------------------------------------------------
rejects(
    "message missing reply",
    "messages:\n  M:\n    id: 1\n    request: null\n",
    "requires 'reply'",
)
rejects(
    "message id not an integer",
    "messages:\n  M:\n    id: \"1\"\n    request: null\n    reply: null\n",
    "must be an integer",
)

# --- yaml subset ----------------------------------------------------------
rejects("tabs", "structs:\n  S:\n    fields:\n\t      a: uint8\n", "tab")
rejects(
    "duplicate yaml key",
    "structs:\n  S:\n    fields:\n      a: uint8\n      a: uint16\n",
    "duplicate key",
)
rejects(
    "flow style",
    "structs:\n  S:\n    fields: {a: uint8}\n",
    "unsupported YAML token",
)
rejects(
    "anchors",
    "structs:\n  S: &anchor\n    fields:\n      a: uint8\n",
    "unsupported YAML token",
)

print(f"{passed} passed, {len(failed)} failed")
for line in failed:
    print(f"  FAIL {line}")
sys.exit(1 if failed else 0)
