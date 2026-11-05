#!/usr/bin/env python3
"""Harvest the constants msp_v2.yaml uses into schema/constants.yaml.

Reuses harvest_enums for the C-value resolver and the enum symbol table: every
`#define` under src/main is collected, then resolved against a combined symbol
table (enum members + already-resolved defines) with the same expression
evaluator, so a define written in terms of other defines
(OSD_CHAR_VISIBLE_BYTES = OSD_CHAR_WIDTH * OSD_CHAR_HEIGHT * ... / 8) resolves.

A constant with a single resolvable definition is emitted as a fixed value; one
defined per target (multiple distinct definitions, or in terms of a target
macro that is not visible here) is emitted `configurable`. Nothing hand-authored.

Usage:
    python3 harvest_consts.py [--src DIR] [--schema DIR]
"""
from __future__ import annotations

import argparse
import re
import sys
import yaml
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import harvest_enums as he   # collect/parse/resolve + clean_expr, all reused

MSP_ROOT = HERE.parent
REPO_ROOT = MSP_ROOT.parent

SIZE_RE = re.compile(r"\[([A-Za-z_]\w*)\]")
DEFINE_RE = re.compile(r"^\s*#\s*define\s+([A-Za-z_]\w*)\s+(.+?)\s*$")


def referenced_constants(doc: dict) -> set:
    names: set = set()

    def payload(v):
        """A request/reply/variant/struct shape: may carry its own repeat."""
        if not isinstance(v, dict):
            return
        r = v.get("repeat")
        if isinstance(r, str) and r != "until_end":
            names.add(r)
        scan(v.get("fields"))

    def scan(fields):
        for spec in (fields or {}).values():
            if isinstance(spec, dict):
                if "repeat" in spec:
                    r = spec["repeat"]
                    if isinstance(r, str) and r != "until_end":
                        names.add(r)
                    scan(spec.get("fields"))
                t = spec.get("type")
                if isinstance(t, str):
                    names.update(SIZE_RE.findall(t))
            elif isinstance(spec, str):
                names.update(SIZE_RE.findall(spec))

    for m in doc["messages"].values():
        for s in ("request", "reply"):
            payload(m.get(s))
        for sh in (m.get("variants") or {}).values():
            for s in ("request", "reply"):
                payload(sh.get(s))
    for b in (doc.get("structs") or {}).values():
        payload(b)
    return names


def enum_symbols(src_main: Path) -> dict:
    """All resolved enum member values, via harvest_enums."""
    enums = he.parse_lines("".join(he.collect(src_main)).splitlines())
    by_name, _failed, _unresolved = he.resolve(enums)
    syms = {}
    for rec in by_name.values():
        for member, (val, _cond) in rec["members"].items():
            syms[member] = val
    return syms


def collect_defines(src_main: Path) -> dict:
    """name -> list of distinct RHS strings (object-like #defines only)."""
    defs: dict = {}
    for fn in src_main.rglob("*"):
        if fn.suffix not in (".c", ".h"):
            continue
        for line in fn.read_text(errors="ignore").splitlines():
            line = re.sub(r"/\*.*?\*/", "", line)
            line = re.sub(r"//.*", "", line)
            m = DEFINE_RE.match(line)
            if not m:
                continue
            name, rhs = m.group(1), m.group(2).strip()
            if name.endswith("("):        # never matches (name is \w+), kept for clarity
                continue
            defs.setdefault(name, [])
            if rhs not in defs[name]:
                defs[name].append(rhs)
    return defs


def try_int(expr: str, table: dict):
    try:
        return int(eval(he.clean_expr(expr), {"__builtins__": {}}, table))
    except Exception:
        return None


def build_symbol_table(src_main: Path):
    """-> (values {name:int}, configurable set). Single-definition defines are
    resolved to fixed integers by repeated passes over a combined table (enum
    members + resolved defines); multiply-defined or unresolvable ones are
    configurable (target-supplied)."""
    table = enum_symbols(src_main)
    defs = collect_defines(src_main)
    configurable = {n for n, rhs in defs.items() if len(rhs) > 1}
    pending = {n: rhs[0] for n, rhs in defs.items() if len(rhs) == 1 and n not in configurable}
    changed = True
    while changed:
        changed = False
        for n, rhs in list(pending.items()):
            v = try_int(rhs, table)
            if v is not None:
                table[n] = v
                del pending[n]
                changed = True
    configurable |= set(pending)   # single def but references a symbol not visible here
    return table, configurable


def uint_type(v: int) -> str:
    for t, bits in (("uint8", 8), ("uint16", 16), ("uint32", 32)):
        if 0 <= v < (1 << bits):
            return t
    return "uint64"


def emit_yaml(consts: dict) -> str:
    out = ["# Generated from INAV C source by msp/generator/harvest_consts.py. Do not edit by hand.",
           "version: 1", "package: inav.constants", "", "constants:"]
    first = True
    for name in sorted(consts):
        kind, val = consts[name]
        if not first:
            out.append("")      # blank line between constant definitions
        first = False
        out.append(f"  {name}:")
        if kind == "configurable":
            out.append("    type: uint16")     # target-supplied; nominal width
            out.append(f"    configurable: {name}")
        else:
            out.append(f"    type: {uint_type(val)}")
            out.append(f"    value: {val}")
    return "\n".join(out) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--src", type=Path, default=REPO_ROOT / "src/main")
    ap.add_argument("--schema", type=Path, default=MSP_ROOT / "schema")
    args = ap.parse_args()

    doc = yaml.safe_load((args.schema / "msp_v2.yaml").read_text())
    table, configurable = build_symbol_table(args.src)

    consts = {}
    for name in sorted(referenced_constants(doc)):
        if name in configurable:
            consts[name] = ("configurable", None)
        elif name in table:
            consts[name] = ("fixed", table[name])
        # else: a preceding count field, not a constant -> skip

    (args.schema / "constants.yaml").write_text(emit_yaml(consts))
    fixed = sum(1 for k, _ in consts.values() if k == "fixed")
    cfg = sum(1 for k, _ in consts.values() if k == "configurable")
    print(f"wrote {args.schema / 'constants.yaml'}  ({fixed} fixed, {cfg} configurable)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
