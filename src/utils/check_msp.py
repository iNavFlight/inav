#!/usr/bin/env python3

"""
Check that INAV MSP command definitions and msp_messages.json agree.

This intentionally does not attempt to validate MSP payload implementation in
fc_msp.c. It checks only the command catalogue:

- Every MSP command defined in the authoritative C headers exists in JSON.
- Every MSP command documented in JSON exists in the C headers, unless the
  JSON entry is flagged "not_implemented".
- Matching command names have matching numeric IDs.
- C command names are unique.
- C numeric command IDs are unique.
- JSON object keys are unique.
- JSON numeric command IDs are unique.

The C headers remain authoritative. This script does not generate C code.
"""

from __future__ import annotations

import json
import re
import sys
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Any


SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parents[1]

MSP_JSON = REPO_ROOT / "docs/development/msp/msp_messages.json"

MSP_HEADERS = [
    REPO_ROOT / "src/main/msp/msp_protocol.h",
    REPO_ROOT / "src/main/msp/msp_protocol_v2_common.h",
    REPO_ROOT / "src/main/msp/msp_protocol_v2_sensor.h",
    REPO_ROOT / "src/main/msp/msp_protocol_v2_inav.h",
]

# Numeric MSP-prefixed macros that are protocol constants rather than entries
# in the MSP message catalogue.
IGNORED_C_DEFINES = {
    "MSP_PROTOCOL_VERSION",
}


# Match object-like MSP macros only.
#
# Matches:
#   #define MSP_API_VERSION       1
#   #define MSP2_INAV_STATUS      0x2000
#
# Does not match function-like macros such as:
#   #define MSP2_IS_SENSOR_MESSAGE(x) ...
DEFINE_RE = re.compile(
    r"^\s*#\s*define\s+"
    r"(?P<name>MSP[A-Z0-9_]+)"
    r"\s+"
    r"(?P<value>.+?)"
    r"\s*$"
)

INTEGER_LITERAL_RE = re.compile(
    r"^\(?\s*"
    r"(?P<number>0[xX][0-9A-Fa-f]+|[0-9]+)"
    r"(?P<suffix>[uUlL]*)"
    r"\s*\)?$"
)


class DuplicateJsonKeyError(ValueError):
    pass


@dataclass(frozen=True)
class Symbol:
    name: str
    code: int
    location: str
    not_implemented: bool = False


def reject_duplicate_json_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    """
    json.load() normally silently accepts duplicate object keys and keeps the
    last one. That is dangerous for a protocol specification, so reject them.
    """
    result: dict[str, Any] = {}

    for key, value in pairs:
        if key in result:
            raise DuplicateJsonKeyError(f"Duplicate JSON object key: {key}")
        result[key] = value

    return result


def strip_block_comments_preserve_lines(text: str) -> str:
    """
    Remove C block comments without changing line numbers.
    """

    def replace_comment(match: re.Match[str]) -> str:
        return "\n" * match.group(0).count("\n")

    return re.sub(
        r"/\*.*?\*/",
        replace_comment,
        text,
        flags=re.DOTALL,
    )


def parse_c_integer(value: str) -> int | None:
    """
    Parse a simple C integer literal.

    Accepted examples:
        123
        123U
        0x2000
        0x2000U
        (0x2000)

    Expressions are deliberately not evaluated.
    """
    match = INTEGER_LITERAL_RE.fullmatch(value.strip())
    if not match:
        return None

    number = match.group("number")

    if number.lower().startswith("0x"):
        return int(number, 16)

    return int(number, 10)


def parse_c_header(path: Path) -> list[Symbol]:
    if not path.is_file():
        raise FileNotFoundError(f"MSP header not found: {path}")

    text = path.read_text(encoding="utf-8")
    text = strip_block_comments_preserve_lines(text)

    symbols: list[Symbol] = []

    for line_number, raw_line in enumerate(text.splitlines(), start=1):
        # Remove C++ style comments.
        line = raw_line.split("//", 1)[0].rstrip()

        if not line:
            continue

        match = DEFINE_RE.match(line)
        if not match:
            continue

        name = match.group("name")

        if name in IGNORED_C_DEFINES:
            continue

        code = parse_c_integer(match.group("value"))

        # Ignore MSP-prefixed macros whose value is not a plain integer.
        #
        # This excludes helper macros and expressions without pretending to
        # understand the C preprocessor.
        if code is None:
            continue

        relative_path = path.relative_to(REPO_ROOT)

        symbols.append(
            Symbol(
                name=name,
                code=code,
                location=f"{relative_path}:{line_number}",
            )
        )

    return symbols


def parse_all_c_headers() -> list[Symbol]:
    symbols: list[Symbol] = []

    for header in MSP_HEADERS:
        symbols.extend(parse_c_header(header))

    return symbols


def parse_json_integer(name: str, value: Any) -> int:
    if isinstance(value, bool):
        raise ValueError(
            f"{name}: JSON 'code' must be an integer, not boolean"
        )

    if isinstance(value, int):
        return value

    if isinstance(value, str):
        value = value.strip()

        if re.fullmatch(r"0[xX][0-9A-Fa-f]+", value):
            return int(value, 16)

        if re.fullmatch(r"[0-9]+", value):
            return int(value, 10)

    raise ValueError(
        f"{name}: invalid JSON 'code' value: {value!r}"
    )


def parse_json_messages(path: Path) -> list[Symbol]:
    if not path.is_file():
        raise FileNotFoundError(f"MSP JSON file not found: {path}")

    with path.open("r", encoding="utf-8") as file:
        payload = json.load(
            file,
            object_pairs_hook=reject_duplicate_json_keys,
        )

    if not isinstance(payload, dict):
        raise ValueError("Top-level MSP JSON value must be an object")

    messages = payload.get("messages")

    if not isinstance(messages, dict):
        raise ValueError(
            "msp_messages.json must contain a top-level 'messages' object"
        )

    symbols: list[Symbol] = []

    for name, body in messages.items():
        if not isinstance(name, str):
            raise ValueError(f"Invalid MSP message name: {name!r}")

        if not isinstance(body, dict):
            raise ValueError(
                f"{name}: message definition must be an object"
            )

        if "code" not in body:
            raise ValueError(
                f"{name}: message definition has no 'code'"
            )

        code = parse_json_integer(name, body["code"])

        symbols.append(
            Symbol(
                name=name,
                code=code,
                location="docs/development/msp/msp_messages.json",
                not_implemented=bool(body.get("not_implemented")),
            )
        )

    return symbols


def group_by_name(symbols: list[Symbol]) -> dict[str, list[Symbol]]:
    result: dict[str, list[Symbol]] = defaultdict(list)

    for symbol in symbols:
        result[symbol.name].append(symbol)

    return dict(result)


def group_by_code(symbols: list[Symbol]) -> dict[int, list[Symbol]]:
    result: dict[int, list[Symbol]] = defaultdict(list)

    for symbol in symbols:
        result[symbol.code].append(symbol)

    return dict(result)


def format_code(code: int) -> str:
    return f"{code} (0x{code:X})"


def print_section(title: str, lines: list[str]) -> None:
    if not lines:
        return

    print()
    print(title)

    for line in lines:
        print(f"  {line}")


def check() -> int:
    try:
        c_symbols = parse_all_c_headers()
        json_symbols = parse_json_messages(MSP_JSON)
    except (
        OSError,
        json.JSONDecodeError,
        DuplicateJsonKeyError,
        ValueError,
    ) as error:
        print(f"MSP consistency check failed: {error}", file=sys.stderr)
        return 1

    errors_found = False

    c_by_name = group_by_name(c_symbols)
    json_by_name = group_by_name(json_symbols)

    c_by_code = group_by_code(c_symbols)
    json_by_code = group_by_code(json_symbols)

    # Duplicate C names.
    duplicate_c_names: list[str] = []

    for name, entries in sorted(c_by_name.items()):
        if len(entries) <= 1:
            continue

        errors_found = True
        duplicate_c_names.append(name)

        for entry in entries:
            duplicate_c_names.append(
                f"    {format_code(entry.code)} at {entry.location}"
            )

    print_section(
        "Duplicate MSP names in C:",
        duplicate_c_names,
    )

    # Duplicate C numeric IDs.
    duplicate_c_codes: list[str] = []

    for code, entries in sorted(c_by_code.items()):
        if len(entries) <= 1:
            continue

        errors_found = True
        duplicate_c_codes.append(format_code(code))

        for entry in entries:
            duplicate_c_codes.append(
                f"    {entry.name} at {entry.location}"
            )

    print_section(
        "Duplicate MSP numeric IDs in C:",
        duplicate_c_codes,
    )

    # Duplicate JSON numeric IDs.
    #
    # Duplicate JSON names are already rejected while parsing because JSON
    # duplicate object keys would otherwise be silently overwritten.
    duplicate_json_codes: list[str] = []

    for code, entries in sorted(json_by_code.items()):
        if len(entries) <= 1:
            continue

        errors_found = True
        duplicate_json_codes.append(format_code(code))

        for entry in entries:
            duplicate_json_codes.append(
                f"    {entry.name}"
            )

    print_section(
        "Duplicate MSP numeric IDs in JSON:",
        duplicate_json_codes,
    )

    c_names = set(c_by_name)
    json_names = set(json_by_name)

    # Commands defined by firmware but absent from the specification.
    c_only_lines: list[str] = []

    for name in sorted(c_names - json_names):
        errors_found = True

        for entry in c_by_name[name]:
            c_only_lines.append(
                f"{name} = {format_code(entry.code)} "
                f"at {entry.location}"
            )

    print_section(
        "MSP commands present in C but missing from msp_messages.json:",
        c_only_lines,
    )

    # Commands documented in the specification but absent from firmware.
    #
    # Entries flagged "not_implemented" are exempt: the specification documents
    # commands that were never implemented or have been retired, and those may
    # legitimately have no macro left in the headers.
    json_only_lines: list[str] = []

    for name in sorted(json_names - c_names):
        entries = [
            entry
            for entry in json_by_name[name]
            if not entry.not_implemented
        ]

        if not entries:
            continue

        errors_found = True

        for entry in entries:
            json_only_lines.append(
                f"{name} = {format_code(entry.code)}"
            )

    print_section(
        "MSP commands present in msp_messages.json but missing from C:",
        json_only_lines,
    )

    # Same symbolic command name but different numeric ID.
    mismatch_lines: list[str] = []

    for name in sorted(c_names & json_names):
        c_entries = c_by_name[name]
        json_entries = json_by_name[name]

        # Duplicate-name errors were already reported above. Compare the first
        # value here so a numeric mismatch is still visible in the same run.
        c_entry = c_entries[0]
        json_entry = json_entries[0]

        if c_entry.code == json_entry.code:
            continue

        errors_found = True

        mismatch_lines.append(
            f"{name}: "
            f"C = {format_code(c_entry.code)} "
            f"at {c_entry.location}; "
            f"JSON = {format_code(json_entry.code)}"
        )

    print_section(
        "MSP command ID mismatches:",
        mismatch_lines,
    )

    if errors_found:
        print()
        print("MSP consistency check FAILED.")
        return 1

    print(
        "MSP consistency check passed: "
        f"{len(c_symbols)} C definitions match "
        f"{len(json_symbols)} JSON definitions."
    )

    return 0


if __name__ == "__main__":
    raise SystemExit(check())
