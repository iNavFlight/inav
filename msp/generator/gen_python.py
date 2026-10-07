#!/usr/bin/env python3
"""Generate the Python message table from the MSP YAML schema.

Emits python/src/mspapi2/generated/messages.py: every payload resolved to a
`struct` format string at generation time, so the runtime codec never inspects
a schema. It looks a message up and calls struct.pack/unpack.

Usage:
    python3 gen_python.py [--schema DIR] [--out FILE]
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

import yaml

HERE = Path(__file__).resolve().parent
MSP_ROOT = HERE.parent          # inav/msp

# schema primitive -> (struct code, size)
CODES = {
    "uint8": ("B", 1), "int8": ("b", 1),
    "uint16": ("H", 2), "int16": ("h", 2),
    "uint32": ("I", 4), "int32": ("i", 4),
    "uint64": ("Q", 8), "int64": ("q", 8),
    "float32": ("f", 4), "float64": ("d", 8),
    "char": ("c", 1), "bool": ("B", 1),
}

TYPE_RE = re.compile(r"^(optional\s+)?([A-Za-z_]\w*)(?:\[(.*)\])?$")


class GenError(Exception):
    pass


class Model:
    def __init__(self, msp_dir: Path):
        self.doc = yaml.safe_load((msp_dir / "msp_v2.yaml").read_text())
        enums = yaml.safe_load((msp_dir / "enums.yaml").read_text()) or {}
        consts = yaml.safe_load((msp_dir / "constants.yaml").read_text()) or {}
        self.enums = enums.get("enums") or {}
        self.bitmasks = enums.get("bitmasks") or {}
        self.constants = consts.get("constants") or {}
        self.aliases = self.doc.get("aliases") or {}
        self.structs = self.doc.get("structs") or {}
        self.external = self.doc.get("external_types") or {}
        # Field layouts derived from INAV's own declarations by
        # extract_wire_types.py, so Python decodes these into named fields
        # without the schema restating a layout it does not own.
        layouts = MSP_ROOT / "c" / "wire_layouts.json"
        self.layouts = json.loads(layouts.read_text()) if layouts.exists() else {}

    def const(self, name: str):
        body = self.constants.get(name)
        if isinstance(body, dict):
            return body.get("value")
        return body if isinstance(body, int) else None

    def storage_of(self, base: str) -> str | None:
        if base in CODES:
            return base
        if base in self.aliases:
            return self.aliases[base]
        body = self.enums.get(base) or self.bitmasks.get(base)
        if body:
            return body.get("storage", "uint8")
        return None


def flatten(model: Model, owner: str, fields: dict, out: list) -> None:
    """Resolve a field block into a flat list of runtime field descriptors."""
    items = list((fields or {}).items())
    for index, (name, spec) in enumerate(items):
        last = index == len(items) - 1

        if isinstance(spec, dict) and "discriminated" in spec:
            d = spec["discriminated"]
            if d.get("by") != "leading-zero":
                raise GenError(f"{owner}.{name}: unknown discriminator {d.get('by')!r}")
            cases = {}
            for case, fields in d["cases"].items():
                inner: list = []
                flatten(model, owner, fields, inner)
                cases[case] = inner
            out.append({"name": name, "kind": "discriminated",
                        "by": "leading-zero", "cases": cases})
            continue

        if isinstance(spec, dict) and "repeat" in spec:
            inner: list = []
            flatten(model, owner, spec.get("fields") or {}, inner)
            repeat = spec["repeat"]
            count = repeat if isinstance(repeat, int) else model.const(repeat)
            out.append({
                "name": name, "kind": "group", "fields": inner,
                "count": count,
                "count_field": repeat if (isinstance(repeat, str) and count is None
                                          and repeat != "until_end") else None,
                "until_end": repeat == "until_end" or (count is None and
                             isinstance(repeat, str) and repeat != "until_end"),
            })
            continue

        expr = spec if isinstance(spec, str) else spec.get("type")
        if not expr:
            raise GenError(f"{owner}.{name}: no type")
        m = TYPE_RE.match(expr.strip())
        if not m:
            raise GenError(f"{owner}.{name}: bad type {expr!r}")
        opt, base, size_expr = m.groups()
        optional = bool(opt)

        if base in model.layouts:
            layout = model.layouts[base]
            out.append({"name": name, "kind": "layout", "type": base,
                        "layout": layout, "size": layout["size"],
                        "optional": optional})
            continue

        if base in model.external:
            # Opaque on this side: the schema states the width, INAV owns the
            # interior. Callers get the raw bytes.
            out.append({"name": name, "kind": "opaque", "type": base,
                        "size": int(model.external[base]["size"]),
                        "optional": optional})
            continue

        if base in model.structs:
            inner: list = []
            flatten(model, owner, model.structs[base]["fields"], inner)
            out.append({"name": name, "kind": "struct", "type": base, "fields": inner,
                        "optional": optional})
            continue

        if base == "cstring":
            out.append({"name": name, "kind": "cstring", "optional": optional})
            continue

        storage = model.storage_of(base)
        if storage is None:
            raise GenError(f"{owner}.{name}: unresolvable type {base!r}")
        code, unit = CODES[storage]

        if size_expr is None:
            out.append({"name": name, "kind": "scalar", "code": code, "size": unit,
                        "optional": optional,
                        "enum": (spec.get("enum") if isinstance(spec, dict) else None),
                        "bitmask": bool(isinstance(spec, dict) and spec.get("bitmask")),
                        "description": (spec.get("description") or "")
                                       if isinstance(spec, dict) else ""})
            continue
        if size_expr == "":
            out.append({"name": name, "kind": "tail", "code": code, "unit": unit,
                        "char": base == "char", "optional": optional})
            continue
        count = int(size_expr) if size_expr.isdigit() else model.const(size_expr)
        if count is None:
            # target-supplied size: length comes from the payload at runtime
            out.append({"name": name, "kind": "tail", "code": code, "unit": unit,
                        "char": base == "char", "optional": optional})
            continue
        out.append({"name": name, "kind": "array", "code": code, "unit": unit,
                    "count": count, "char": base == "char", "optional": optional})


def payload_table(model: Model, owner: str, spec) -> dict | None:
    if not isinstance(spec, dict) or not spec.get("fields"):
        return None
    flat: list = []
    flatten(model, owner, spec["fields"], flat)
    repeat = spec.get("repeat")
    return {
        "fields": flat,
        "repeat": repeat if isinstance(repeat, (int, str)) else None,
    }


def message_table(model: Model) -> tuple[dict, dict]:
    """Every message resolved to its runtime entry, keyed by name, plus counts.

    The one place a message becomes a table entry; each language backend only
    serialises the result.
    """
    entries: dict = {}
    counts = {"messages": 0, "not_implemented": 0, "variants": 0}

    for name, msg in model.doc["messages"].items():
        counts["messages"] += 1
        entry = {
            "name": name, "code": msg["id"], "mspv": msg.get("mspv"),
            "not_implemented": bool(msg.get("not_implemented")),
            "variable_len": bool(msg.get("variable_len")),
        }
        if msg.get("not_implemented"):
            counts["not_implemented"] += 1
            entry["request"] = entry["reply"] = None
            entries[name] = entry
            continue
        try:
            if msg.get("variants"):
                counts["variants"] += 1
                entry["variants"] = {
                    key: {"request": payload_table(model, name, shape.get("request")),
                          "reply": payload_table(model, name, shape.get("reply"))}
                    for key, shape in msg["variants"].items()
                }
                entry["request"] = entry["reply"] = None
            else:
                entry["request"] = payload_table(model, name, msg.get("request"))
                entry["reply"] = payload_table(model, name, msg.get("reply"))
        except GenError as exc:
            # Never drop a message: a schema the generator cannot represent is
            # a defect to fix, not 277 messages to ship quietly.
            raise GenError(f"{name}: {exc}") from exc
        entries[name] = entry
    return entries, counts


def enum_tables(model: Model) -> tuple[dict, dict]:
    """Enum members with their values, and the members that exist only under a
    build condition, each keyed by the enum's C typedef name.

    Member names carry the enum's prefix (idl_spec.md section 9.1), so they
    match the C names callers grep for.
    """
    tables: dict = {}
    conditional: dict = {}
    for table in (model.enums, model.bitmasks):
        for name, body in table.items():
            prefix = body.get("prefix") or ""
            qual = (lambda n: f"{prefix}_{n}") if prefix else (lambda n: n)
            members: dict[str, int] = {}
            conditions: dict[str, str] = {}
            for member in body.get("values") or []:
                if isinstance(member, dict):
                    key, raw = qual(member["name"]), member.get("value")
                    conditions[key] = member.get("condition", "")
                else:
                    key, _, raw = str(member).partition(" = ")
                    key = qual(key)
                if isinstance(raw, int):
                    value = raw
                elif isinstance(raw, str) and re.fullmatch(r"-?\d+", raw.strip()):
                    value = int(raw)
                else:
                    alias = qual(str(raw).strip())
                    if alias not in members:
                        continue
                    value = members[alias]
                members.setdefault(key, value)
            if body.get("zero"):
                members.setdefault(qual(body["zero"]), 0)
            if not members:
                continue
            tables[name] = members
            if conditions:
                conditional[name] = conditions
    return tables, conditional


def constant_values(model: Model) -> dict:
    """Constants with a fixed value; target-supplied ones have none and are left out."""
    consts = {
        name: (body.get("value") if isinstance(body, dict) else body)
        for name, body in sorted(model.constants.items())
    }
    return {k: v for k, v in consts.items() if v is not None}


HEADER = '''"""Generated by gen_python.py from schema/msp_v2.yaml. Do not edit by hand.

Every payload is resolved here at generation time. The runtime codec reads this
table; it never parses a schema.
"""
from __future__ import annotations

'''


def emit_enums(model: Model) -> str:
    """InavEnums, generated."""
    out = ['"""Generated by gen_python.py from schema/enums.yaml. Do not edit by hand."""',
           "from __future__ import annotations", "", "import enum", "", "",
           "class InavEnums:", '    """Namespace of INAV enums, keyed by their C typedef name."""', "",
           "", "_CONDITIONAL: dict[str, dict[str, str]] = {}", ""]
    tables, conditional = enum_tables(model)
    for name, members in tables.items():
        out.append(f"InavEnums.{name} = enum.IntEnum({name!r}, {members!r})")
        if name in conditional:
            out.append(f"_CONDITIONAL[{name!r}] = {conditional[name]!r}")
    out += ["", "", "CONDITIONAL_MEMBERS = _CONDITIONAL",
            '"""Members that exist only under a build condition, by enum then member."""', ""]
    return "\n".join(out)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--schema", type=Path, default=MSP_ROOT / "schema")
    ap.add_argument("--out", type=Path,
                    default=MSP_ROOT / "python" / "src" / "mspapi2" / "generated" / "messages.py")
    args = ap.parse_args()

    model = Model(args.schema)
    out: list[str] = [HEADER]
    table, counts = message_table(model)
    entries = [f"    {name!r}: {entry!r}," for name, entry in table.items()]

    out.append("MESSAGES: dict = {\n" + "\n".join(entries) + "\n}\n")
    out.append("BY_CODE = {m['code']: m for m in MESSAGES.values()}\n")
    out.append(
        "import enum\n\n"
        "InavMSP = enum.IntEnum(  # message codes, generated from the schema\n"
        "    'InavMSP', {name: body['code'] for name, body in MESSAGES.items()}\n"
        ")\n"
    )
    args.out.parent.mkdir(parents=True, exist_ok=True)
    (args.out.parent / "__init__.py").write_text(
        '"""Generated artifacts. Do not edit by hand."""\n')
    consts_path = args.out.parent / "constants.py"
    consts_path.write_text(
        '"""Generated by gen_python.py from schema/constants.yaml. '
        'Do not edit by hand."""\n\n\n'
        "class InavConstants:\n"
        '    """MSP protocol constants, as declared in the schema."""\n\n'
        + "".join(f"    {k} = {v!r}\n" for k, v in constant_values(model).items())
    )

    args.out.write_text("\n".join(out))
    enums_path = args.out.parent / "enums.py"
    enums_path.write_text(emit_enums(model))

    print(f"wrote {args.out}")
    print(f"wrote {enums_path}")
    print(f"wrote {consts_path}")
    print(f"  messages: {counts['messages']}  variants: {counts['variants']}  "
          f"not_implemented: {counts['not_implemented']}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except GenError as exc:
        print(f"error: {exc}", file=sys.stderr)
        sys.exit(1)
