#!/usr/bin/env python3
"""Write payload vectors for every message, packed by the Python codec.

For each request and reply (and each variant of a length-dispatched message)
this builds sample values from the generated table, packs them, and records the
bytes with the values the Python codec decodes back from them. crosscheck.mjs
then requires the JS codec to decode the same bytes to the same values and to
pack those values to the same bytes.

A message with optional trailing fields gets a second vector that stops before
the first optional field.

Usage:
    python3 make_vectors.py OUT.json
"""
from __future__ import annotations

import json
import random
import string
import sys
from pathlib import Path

from mspapi2.generated.messages import MESSAGES
from mspapi2.mspcodec import _decode_side, _encode_side

INT_RANGES = {
    "B": (0, 0xFF), "b": (-0x80, 0x7F), "H": (0, 0xFFFF), "h": (-0x8000, 0x7FFF),
    "I": (0, 0xFFFFFFFF), "i": (-0x80000000, 0x7FFFFFFF),
    "Q": (0, 0xFFFFFFFFFFFFFFFF), "q": (-0x8000000000000000, 0x7FFFFFFFFFFFFFFF),
}
LAYOUT_CODES = {"uint8_t": "B", "int8_t": "b", "uint16_t": "H", "int16_t": "h",
                "uint32_t": "I", "int32_t": "i", "uint64_t": "Q", "int64_t": "q"}
TEXT = string.ascii_letters + string.digits + " _-"
SAFE_INTEGER = 2 ** 53


def number(rng: random.Random, code: str):
    if code in "fd":
        return rng.uniform(-100000.0, 100000.0)
    low, high = INT_RANGES[code]
    return rng.randint(low, high)


def text(rng: random.Random, low: int, high: int) -> str:
    return "".join(rng.choice(TEXT) for _ in range(rng.randint(low, high)))


def layout_value(rng: random.Random, layout: dict) -> dict:
    if layout["kind"] == "bitfield":
        return {f["name"]: rng.randint(0, (1 << f["bits"]) - 1) for f in layout["fields"]}
    return {f["name"]: number(rng, LAYOUT_CODES[f["ctype"]]) for f in layout["fields"]}


def record(rng: random.Random, fields: list, stop_at_optional: bool) -> dict:
    out: dict = {}
    for field in fields:
        if stop_at_optional and field.get("optional"):
            break
        kind = field["kind"]
        if kind == "scalar":
            out[field["name"]] = number(rng, field["code"])
        elif kind == "array":
            out[field["name"]] = (text(rng, 0, field["count"]) if field["char"]
                                  else [number(rng, field["code"]) for _ in range(field["count"])])
        elif kind == "tail":
            out[field["name"]] = (text(rng, 0, 12) if field["char"]
                                  else [number(rng, field["code"]) for _ in range(rng.randint(0, 6))])
        elif kind == "cstring":
            out[field["name"]] = text(rng, 1, 12)
        elif kind == "opaque":
            out[field["name"]] = bytes(rng.randint(0, 255) for _ in range(field["size"]))
        elif kind == "layout":
            out[field["name"]] = layout_value(rng, field["layout"])
        elif kind == "struct":
            out[field["name"]] = record(rng, field["fields"], False)
        elif kind == "discriminated":
            case = rng.choice(sorted(field["cases"]))
            value = record(rng, field["cases"][case], False)
            if case == "index":
                # The leading byte is what selects this case.
                value[field["cases"][case][0]["name"]] = 0
            out[field["name"]] = {"case": case, **value}
        elif kind == "group":
            count = field["count"]
            if count is None:
                count = rng.randint(1, 3)
                if field.get("count_field"):
                    out[field["count_field"]] = count
            out[field["name"]] = [record(rng, field["fields"], False) for _ in range(count)]
        else:
            raise ValueError(f"unhandled field kind {kind!r}")
    return out


def side_values(rng: random.Random, side: dict, stop_at_optional: bool):
    if side.get("repeat") is None:
        return record(rng, side["fields"], stop_at_optional)
    count = side["repeat"] if isinstance(side["repeat"], int) else 2
    return [record(rng, side["fields"], stop_at_optional) for _ in range(count)]


def has_optional(fields: list) -> bool:
    return any(field.get("optional") for field in fields)


def to_json(value):
    """JSON cannot carry bytes or integers past 2**53 exactly; tag both."""
    if isinstance(value, (bytes, bytearray)):
        return {"$bytes": bytes(value).hex()}
    if isinstance(value, bool):
        return value
    if isinstance(value, int) and abs(value) > SAFE_INTEGER:
        return {"$bigint": str(value)}
    if isinstance(value, dict):
        return {key: to_json(item) for key, item in value.items()}
    if isinstance(value, list):
        return [to_json(item) for item in value]
    return value


def sides(entry: dict):
    if entry.get("variants"):
        for variant, shape in entry["variants"].items():
            for direction in ("request", "reply"):
                if shape[direction]:
                    yield direction, variant, shape[direction]
        return
    for direction in ("request", "reply"):
        if entry[direction]:
            yield direction, None, entry[direction]


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__, file=sys.stderr)
        return 2
    rng = random.Random(0x4D5350)
    vectors = []
    for name, entry in MESSAGES.items():
        if entry["not_implemented"]:
            continue
        for direction, variant, side in sides(entry):
            truncations = (False, True) if has_optional(side["fields"]) else (False,)
            for stop_at_optional in truncations:
                payload = _encode_side(side, side_values(rng, side, stop_at_optional), name)
                decoded = _decode_side(side, payload, name)
                vectors.append({
                    "message": name, "code": entry["code"], "side": direction,
                    "variant": variant, "truncated": stop_at_optional,
                    "payload": payload.hex(), "values": to_json(decoded),
                })
    Path(sys.argv[1]).write_text(json.dumps(vectors))
    print(f"  {len(vectors)} vectors from {sum(1 for e in MESSAGES.values() if not e['not_implemented'])} messages")
    return 0


if __name__ == "__main__":
    sys.exit(main())
