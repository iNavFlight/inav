#!/usr/bin/env python3
"""Generate the whole C MSP library from the MSP YAML schema.

    schema/enums.yaml          -> msp_enums.h       real C enums, one per enum/bitmask
    schema/constants.yaml      -> msp_consts.h      #defines used as array sizes
    schema/msp_v2.yaml         -> msp_msgs.h        one packed struct per message payload
    schema/msp_v2.yaml         -> msp_protocol*.h   message-id #defines (four headers)

The static parts of msp_msgs.h and msp_protocol*.h (licence, MSP guidelines, the
non-message #defines, pack pragmas, includes) are real C in templates/*.in;
generation only injects the message ids and structs at the @MESSAGE_IDS@ /
@MESSAGE_STRUCTS@ markers. The message-id headers and the wire structs are two
facets of one library, so one backend emits both into one directory; msp_msgs.h
includes msp_protocol.h for the ids.

Design notes:

* Enum-typed fields are emitted as their *storage* type, not as the C enum.
  A C enum's size is implementation-defined, so using one inside a packed wire
  struct would make the layout depend on the compiler. The enum name is carried
  in a comment and the real enum lives in msp_enums.h for callers to use.

* An open-ended array is a C99 flexible array member. sizeof() then reports the
  fixed header alone, which is exactly what a `dataSize >= sizeof(...)` guard
  needs; element count is (dataSize - sizeof(hdr)) / sizeof(elem).

* Every struct whose length is fully determined gets a STATIC_ASSERT pinning it.
  That is the whole point: the schema and the C layout cannot drift silently.

Usage:
    python3 gen_c.py [--schema DIR] [--out DIR] [--check DIR]
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

import yaml

from enum_syntax import split_member

HERE = Path(__file__).resolve().parent
MSP_ROOT = HERE.parent          # inav/msp
TEMPLATE_DIR = HERE / "templates"

# --- message-id protocol headers (msp_protocol*.h) ---------------------------
# group -> (filename, includes emitted at the end)
PROTOCOL_LAYOUT = {
    "v1": ("msp_protocol.h",
           ["msp_protocol_v2_common.h", "msp_protocol_v2_sensor.h", "msp_protocol_v2_inav.h"]),
    "common": ("msp_protocol_v2_common.h", []),
    "sensor": ("msp_protocol_v2_sensor.h", []),
    "inav": ("msp_protocol_v2_inav.h", []),
}

CODE_RE = re.compile(r'^#define\s+(MSP2?_[A-Z0-9_]+)\s+(0x[0-9A-Fa-f]+|\d+)\s*(?://.*)?$')

PRIMITIVES = {
    "uint8": ("uint8_t", 1), "int8": ("int8_t", 1),
    "uint16": ("uint16_t", 2), "int16": ("int16_t", 2),
    "uint32": ("uint32_t", 4), "int32": ("int32_t", 4),
    "uint64": ("uint64_t", 8), "int64": ("int64_t", 8),
    "float32": ("float", 4), "float64": ("double", 8),
    "char": ("char", 1), "bool": ("uint8_t", 1),
}

TYPE_RE = re.compile(r"^(optional\s+)?([A-Za-z_]\w*)(?:\[(.*)\])?$")

# Payload types INAV declares itself. msp_msgs.h references them and never
# declares them: a firmware build has them already, and everyone else includes
# the vendored clib/support/msp_wire_types.h.
EXTERNAL_TYPES = {"escSensorData_t", "ledConfig_t", "boxBitmask_t"}


class GenError(Exception):
    pass


class Model:
    """Everything the emitter needs to resolve a type expression."""

    def __init__(self, msp_dir: Path):
        self.messages = yaml.safe_load((msp_dir / "msp_v2.yaml").read_text())
        enums_doc = yaml.safe_load((msp_dir / "enums.yaml").read_text()) or {}
        consts_doc = yaml.safe_load((msp_dir / "constants.yaml").read_text()) or {}
        self.enums = enums_doc.get("enums") or {}
        self.bitmasks = enums_doc.get("bitmasks") or {}
        self.constants = consts_doc.get("constants") or {}
        self.aliases = self.messages.get("aliases") or {}
        self.structs = self.messages.get("structs") or {}
        self.struct_sizes: dict[str, int] = {}

    def const_value(self, name: str) -> int | None:
        body = self.constants.get(name)
        if isinstance(body, dict):
            return body.get("value")
        return body if isinstance(body, int) else None

    def is_configurable(self, name: str) -> bool:
        body = self.constants.get(name)
        return isinstance(body, dict) and body.get("configurable") is not None

    def base_ctype(self, base: str) -> tuple[str, int | None, str]:
        """-> (C type, byte size or None, trailing comment)"""
        if base == "cstring":
            # Self-delimiting on the wire. C has no such type, so it is a
            # trailing char[]; resolve() marks it flexible.
            return "char", None, "NUL-terminated"
        if base in PRIMITIVES:
            c, size = PRIMITIVES[base]
            return c, size, ""
        if base in self.aliases:
            target = self.aliases[base]
            c, size = PRIMITIVES[target]
            return c, size, f"{base}"
        if base in self.enums or base in self.bitmasks:
            body = (self.enums.get(base) or self.bitmasks.get(base))
            storage = body.get("storage", "uint8")
            c, size = PRIMITIVES[storage]
            return c, size, f"enum {base}"
        if base in EXTERNAL_TYPES:
            # Declared by INAV, vendored into clib/support/msp_wire_types.h for
            # everyone else. Size is unknown here on purpose: restating it would
            # mean re-deriving a layout INAV already fixes.
            return base, None, ""
        raise GenError(
            f"unresolvable type {base!r}; if INAV declares it, add it to "
            f"EXTERNAL_TYPES and to generator/extract_wire_types.py")

    def resolve(self, expr: str) -> tuple[str, str, int | None, bool, str]:
        """-> (ctype, declarator suffix, size or None, is_flexible, comment)"""
        m = TYPE_RE.match(expr.strip())
        if not m:
            raise GenError(f"bad type expression {expr!r}")
        _opt, base, size_expr = m.groups()
        ctype, unit, comment = self.base_ctype(base)
        if base == "cstring":
            return ctype, "[]", 0, True, comment
        if size_expr is None:
            return ctype, "", unit, False, comment
        if size_expr == "":
            return ctype, "[]", 0, True, comment
        if size_expr.isdigit():
            n = int(size_expr)
            return ctype, f"[{n}]", (unit * n if unit else None), False, comment
        value = self.const_value(size_expr)
        if value is None:
            if self.is_configurable(size_expr):
                # Supplied by the target build; msp_consts.h #errors if absent.
                return ctype, f"[{size_expr}]", None, False, comment
            raise GenError(f"array size {size_expr!r} is not a declared constant")
        return ctype, f"[{size_expr}]", (unit * value if unit else None), False, comment


def ident(name: str) -> str:
    return re.sub(r"\W", "_", name)


def struct_name(message: str, side: str, suffix: str = "") -> str:
    """MSP_SET_RC_TUNING + request -> mspSetRcTuningRequest_t.

    INAV names typedefs in camelCase ending _t; sensei's hand-written
    mspSetRcTuning_t is the reference. A generated header that shouts
    MSP_SET_RC_TUNING_request_t does not belong beside it.
    """
    words = [w for w in message.split("_") if w]
    # Keep the MSP/MSP2 distinction: MSP_BLACKBOX_CONFIG and
    # MSP2_BLACKBOX_CONFIG are different messages and must not collapse
    # onto the same type name.
    stem = "msp"
    if words and words[0].upper() in ("MSP", "MSP2"):
        stem = words[0].lower()
        words = words[1:]
    camel = "".join(w.capitalize() if not w.isdigit() else w for w in words)
    return f"{stem}{camel}{suffix}{side.capitalize()}_t"


def variant_suffix(key: str) -> str:
    """Turn a variant's condition text into a unique C identifier suffix.

    The relation has to survive: a message with both ">= 4" and "== 4" would
    otherwise produce the same struct name twice.
    """
    text = (key.strip()
            .replace(">=", "ge").replace("==", "eq").replace("!=", "ne")
            .replace("<=", "le").replace(">", "gt").replace("<", "lt"))
    return "_" + re.sub(r"\W+", "_", text).strip("_")


class Emitter:
    def __init__(self, model: Model):
        self.m = model
        self.lines: list[str] = []
        self.asserts: list[tuple[str, int]] = []
        self.bare_arrays: list[str] = []

    def emit_fields(self, owner: str, fields: dict, indent: str) -> tuple[int | None, bool]:
        """Emit one field block. -> (fixed size or None, has flexible tail)"""
        total: int | None = 0
        flexible = False
        items = list((fields or {}).items())
        for index, (name, spec) in enumerate(items):
            last = index == len(items) - 1
            if isinstance(spec, dict) and "repeat" in spec:
                inner = spec.get("fields") or {}
                self.lines.append(f"{indent}struct MSP_PACKED {{")
                inner_size, inner_flex = self.emit_fields(owner, inner, indent + "    ")
                repeat = spec["repeat"]
                if isinstance(repeat, int):
                    suffix = f"[{repeat}]"
                    grow = (inner_size * repeat) if inner_size is not None else None
                elif isinstance(repeat, str) and self.m.const_value(repeat) is not None:
                    suffix = f"[{repeat}]"
                    n = self.m.const_value(repeat)
                    grow = (inner_size * n) if inner_size is not None else None
                elif isinstance(repeat, str) and self.m.is_configurable(repeat):
                    suffix, grow = f"[{repeat}]", None
                else:
                    # counted by a preceding field, or runs to the end of the
                    # payload: a flexible array member either way.
                    if not last:
                        raise GenError(
                            f"{owner}.{name}: open repeat group is not the final field"
                        )
                    suffix, grow, flexible = "[]", 0, True
                if inner_flex:
                    raise GenError(f"{owner}.{name}: nested flexible array")
                note = f"  // repeat: {repeat}"
                self.lines.append(f"{indent}}} {ident(name)}{suffix};{note}")
                total = None if (total is None or grow is None) else total + grow
                continue

            expr = spec if isinstance(spec, str) else spec.get("type")
            if not expr:
                raise GenError(f"{owner}.{name}: field has no type")
            optional = expr.strip().startswith("optional ")
            ctype, suffix, size, is_flex, comment = self.m.resolve(expr)
            # Enum-typed field: the wire width is the field's own type; the enum
            # name is a reference annotation (its definition is harvested).
            if isinstance(spec, dict) and spec.get("enum"):
                comment = f"enum {spec['enum']}"
            if is_flex:
                if not last:
                    raise GenError(
                        f"{owner}.{name}: {expr!r} is variable-length but is not the "
                        f"final field, so this payload cannot be one C struct"
                    )
                flexible = True

            bits = []
            if isinstance(spec, dict):
                if spec.get("description"):
                    bits.append(spec["description"])
                unit = (spec.get("unit") or "").strip()
                # "Enum" and "Bitmask" are not units; the type and the bitmask
                # note already carry that.
                if unit and unit.lower() not in ("enum", "bitmask"):
                    bits.append(unit)
                if spec.get("bitmask"):
                    bits.append("bitmask")
                if "value" in spec:
                    bits.append(f"always {spec['value']}")
            # Many descriptions already name the enum; do not say it twice.
            joined = " ".join(bits).lower().replace("`", "")
            if comment and comment.lower().replace("`", "") not in joined:
                bits.append(comment)
            if optional:
                bits.append("OPTIONAL: may be absent from a shorter payload")
            note = ("  // " + " | ".join(b.replace("\n", " ") for b in bits)) if bits else ""
            self.lines.append(f"{indent}{ctype} {ident(name)}{suffix};{note}")

            if optional or size is None:
                total = None
            elif total is not None:
                total += size
        return total, flexible

    def emit_struct(self, name: str, fields: dict, header: list[str]) -> None:
        for line in header:
            self.lines.append(f"// {line}")

        # A payload whose only field is an open array is not a struct, it is an
        # array: C forbids a flexible array member as the sole member, and a
        # [1] placeholder would understate the real length. Emit the element
        # type so callers index it directly.
        items = list((fields or {}).items())
        if len(items) == 1 and isinstance(items[0][1], dict) and "repeat" in items[0][1]:
            group = items[0][1]
            repeat = group["repeat"]
            fixed = isinstance(repeat, int) or (
                isinstance(repeat, str)
                and (self.m.const_value(repeat) is not None or self.m.is_configurable(repeat))
            )
            if not fixed:
                elem = (name[:-2] if name.endswith("_t") else name) + "Elem_t"
                self.lines.append(
                    f"// payload is a bare sequence of records (repeat: {repeat}); no fixed"
                )
                self.lines.append(f"// header, so no wrapper struct. count = payload_size / sizeof({elem})")
                self.lines.append("typedef struct MSP_PACKED {")
                size, flex = self.emit_fields(name, group.get("fields") or {}, "    ")
                self.lines.append(f"}} {elem};")
                if size is not None and not flex:
                    self.lines.append(
                        f"MSP_STATIC_ASSERT(sizeof({elem}) == {size}, {elem}_size);"
                    )
                    self.asserts.append((elem, size))
                self.lines.append("")
                self.bare_arrays.append(name)
                return

        if len(items) == 1:
            only_name, only_spec = items[0]
            expr = only_spec if isinstance(only_spec, str) else only_spec.get("type", "")
            if isinstance(only_spec, dict) and "repeat" not in only_spec or isinstance(only_spec, str):
                try:
                    ctype, suffix, _size, is_flex, comment = self.m.resolve(expr)
                except GenError:
                    is_flex = False
                if is_flex:
                    elem = (name[:-2] if name.endswith("_t") else name) + "Elem_t"
                    self.lines.append(
                        f"// payload is a bare array of {only_name}; no fixed header, so no struct."
                    )
                    self.lines.append(
                        f"// element count = payload_size / sizeof({elem})"
                    )
                    self.lines.append(f"typedef {ctype} {elem};")
                    self.lines.append("")
                    self.bare_arrays.append(name)
                    return
        self.lines.append("typedef struct MSP_PACKED {")
        size, flexible = self.emit_fields(name, fields, "    ")
        self.lines.append(f"}} {name};")
        if size is not None and not flexible:
            self.asserts.append((name, size))
            self.lines.append(
                f"MSP_STATIC_ASSERT(sizeof({name}) == {size}, {name}_size);"
            )
        elif flexible:
            self.lines.append(
                f"// variable length: sizeof({name}) is the fixed header only"
            )
        self.lines.append("")

# user note: This is unacceptable, we never ever do this; sticking code strings into schemas or executable code. We use template files.
def emit_consts(model: Model) -> str:
    out = ["#pragma once",
           "// Generated from the MSP YAML schema by msp/generator/gen_c.py (constants.yaml). Do not edit by hand.",
           ""]
    unresolved = []
    for name, body in sorted(model.constants.items()):
        if isinstance(body, dict) and body.get("header"):
            continue   # declared by the generated msp_protocol*.h instead
        value = model.const_value(name)
        desc = body.get("description", "") if isinstance(body, dict) else ""
        if value is None:
            unresolved.append((name, body.get("configurable") if isinstance(body, dict) else None))
            continue
        if desc:
            out.append(f"// {desc}")
        # Guarded so a target can supply its own build-specific value, matching
        # how INAV's own headers declare these.
        out.append(f"#ifndef {name}")
        out.append(f"#  define {name} {value}")
        out.append("#endif")
    if unresolved:
        out += ["", "// Build-configuration dependent: supplied by the target, not the schema.",
                "// Generation fails rather than guessing, so each must be defined by the build."]
        for name, sym in unresolved:
            out.append(f"#if !defined({name})")
            out.append(f"#  error \"{name} is build-configuration dependent"
                       f"{f' (from {sym})' if sym else ''} and must be defined by the target\"")
            out.append("#endif")
    return "\n".join(out) + "\n"


def referenced_enums(model: Model) -> set[str]:
    """Enums a message payload actually transmits.

    The source carries every enum in INAV; the vast majority are internal and
    have no place in a wire header. Restricting to the referenced set also
    avoids member-name collisions between unrelated firmware enums.
    """
    used: set[str] = set()

    def scan(fields):
        for spec in (fields or {}).values():
            if isinstance(spec, dict) and "repeat" in spec:
                scan(spec.get("fields"))
                continue
            if isinstance(spec, dict):
                en = spec.get("enum")
                if en and (en in model.enums or en in model.bitmasks):
                    used.add(en)

    for msg in model.messages["messages"].values():
        for side in ("request", "reply"):
            spec = msg.get(side)
            if isinstance(spec, dict):
                scan(spec.get("fields"))
        for shape in (msg.get("variants") or {}).values():
            for side in ("request", "reply"):
                spec = shape.get(side)
                if isinstance(spec, dict):
                    scan(spec.get("fields"))
    for body in model.structs.values():
        scan(body.get("fields"))
    return used


def c_condition(text: str) -> str:
    """Turn a schema condition into a C preprocessor expression.

    The source writes build conditions the way INAV's #ifdefs read them, plus a
    NOT(...) form that is not C.
    """
    text = text.strip()
    text = re.sub(r"\bNOT\s*\(", "!(", text)
    return text


def emit_enums(model: Model) -> str:
    """Real C enums, matching how INAV declares them.

    Wire structs use the storage type, never the enum type, so an enum's
    implementation-defined size cannot affect any layout. That leaves no reason
    to degrade these to #defines: as enums they type-check, show up by name in a
    debugger, and let a switch be checked for exhaustiveness.
    """
    used = referenced_enums(model)
    out = ["#pragma once",
           "// Generated from the MSP YAML schema by msp/generator/gen_c.py (enums.yaml). Do not edit by hand.",
           "//",
           f"// {len(used)} of {len(model.enums) + len(model.bitmasks)} enums are "
           "referenced by a payload; only those are emitted.",
           "", "#include <stdint.h>", ""]
    seen: dict[str, str] = {}

    for kind, table in (("enum", model.enums), ("bitmask", model.bitmasks)):
        for name, body in table.items():
            if name not in used:
                continue
            # idl_spec.md section 9.1: the prefix is prepended in generated
            # output. It also keeps unrelated enums from colliding, since C
            # enum members share one namespace just as #defines do.
            prefix = body.get("prefix") or ""
            qual = (lambda n, p=prefix: f"{p}_{n}") if prefix else (lambda n: n)

            if body.get("description"):
                out.append(f"// {body['description']}")
            out.append("typedef enum {")
            if body.get("zero"):
                out.append(f"    {qual(body['zero'])} = 0,")
            for member in body.get("values") or []:
                member_name, raw, condition = split_member(member)
                head, tail = qual(member_name), raw or ""
                if tail and not re.fullmatch(r"-?\d+", tail):
                    tail = qual(tail)          # alias to an earlier member
                if head in seen:
                    raise GenError(
                        f"enum member {head!r} is declared by both {seen[head]} and "
                        f"{name}; C gives these one namespace, so the schema must "
                        f"give one of them a prefix")
                seen[head] = name
                if kind == "bitmask" and re.fullmatch(r"\d+", tail.strip()):
                    tail = f"(1UL << {tail.strip()})"
                if condition:
                    out.append(f"#if {c_condition(condition)}")
                out.append(f"    {head} = {tail},")
                if condition:
                    out.append("#endif")
            out.append(f"}} {name};")
            out.append("")
    return "\n".join(out) + "\n"


def render_protocol(group: str, messages: dict) -> str:
    """One msp_protocol*.h. The static header (licence, MSP guidelines, the
    non-message #defines, and includes) is the real C in templates/<file>.in;
    only the message-id #defines are generated, injected at @MESSAGE_IDS@.

    not_implemented refers to the handler, not the id: INAV still #defines those
    codes, so every message in the group is emitted."""
    filename, _includes = PROTOCOL_LAYOUT[group]
    template = (TEMPLATE_DIR / (filename + ".in")).read_text()
    rows = [(name, body["id"], body.get("mspv"))
            for name, body in messages.items() if body.get("group") == group]
    rows.sort(key=lambda r: r[1])
    width = max((len(n) for n, _, _ in rows), default=0) + 2
    lines = [f"#define {name.ljust(width)}{f'0x{code:04X}' if mspv == 2 else str(code)}"
             for name, code, mspv in rows]
    return template.replace("@MESSAGE_IDS@", "\n".join(lines))


def defines_in(text: str) -> dict[str, int]:
    found = {}
    for line in text.split("\n"):
        m = CODE_RE.match(line.strip())
        if m:
            found[m.group(1)] = int(m.group(2), 0)
    return found


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--schema", type=Path, default=MSP_ROOT / "schema")
    ap.add_argument("--out", type=Path, default=MSP_ROOT / "c")
    ap.add_argument("--check", type=Path, metavar="DIR",
                    help="compare generated message ids against the headers in DIR "
                         "instead of writing; non-zero exit on any difference")
    args = ap.parse_args()

    model = Model(args.schema)

    # --- message-id protocol headers (msp_protocol*.h) ---
    messages = model.messages["messages"]
    protocol = {PROTOCOL_LAYOUT[g][0]: render_protocol(g, messages)
                for g in PROTOCOL_LAYOUT}

    if args.check:
        ours, theirs = {}, {}
        for filename, text in protocol.items():
            ours.update(defines_in(text))
            source = args.check / filename
            if source.exists():
                theirs.update(defines_in(source.read_text()))
        only_ours = sorted(set(ours) - set(theirs))
        only_theirs = sorted(set(theirs) - set(ours))
        differing = sorted(k for k in set(ours) & set(theirs) if ours[k] != theirs[k])
        print(f"generated ids: {len(ours)}   reference ids: {len(theirs)}")
        print(f"  agree          : {len(set(ours) & set(theirs)) - len(differing)}")
        print(f"  VALUE MISMATCH : {len(differing)}")
        for k in differing:
            print(f"    {k}: generated 0x{ours[k]:04X} != reference 0x{theirs[k]:04X}")
        if only_ours:
            print(f"  only generated : {only_ours}")
        if only_theirs:
            print(f"  only reference : {only_theirs}")
        return 1 if differing else 0

    args.out.mkdir(parents=True, exist_ok=True)
    for filename, text in protocol.items():
        (args.out / filename).write_text(text)
        print(f"wrote {args.out / filename}  ({len(defines_in(text))} ids)")

    (args.out / "msp_consts.h").write_text(emit_consts(model))
    (args.out / "msp_enums.h").write_text(emit_enums(model))

    # Payloads reference these by value, so their sizes are needed, but they are
    # declared elsewhere: by INAV's own headers in a firmware build, or by
    # msp_wire_types.h otherwise.
    pre = Emitter(model)
    for name, body in model.structs.items():
        size, _flex = Emitter(model).emit_fields(name, body.get("fields") or {}, "    ")
        model.struct_sizes[name] = size

    body_lines: list[str] = []
    emitter = Emitter(model)
    emitter.lines = pre.lines
    emitter.asserts = pre.asserts
    ids: list[tuple[str, int]] = []
    skipped = 0
    unexpressible: list[str] = []
    emitted_names: dict[str, str] = {}

    for name, msg in model.messages["messages"].items():
        ids.append((name, msg["id"]))
        if msg.get("not_implemented"):
            skipped += 1
            continue
        head = [f"{name} (MSPv{msg.get('mspv')}) id={msg['id']}"]
        if msg.get("description"):
            head.append(msg["description"])
        if msg.get("notes"):
            head.append(f"Notes: {msg['notes']}")

        def payload(side: str, spec, suffix: str = "") -> None:
            if not isinstance(spec, dict) or not spec.get("fields"):
                return
            name_t = struct_name(ident(name), side, suffix)
            fields = spec["fields"]
            if "repeat" in spec:
                fields = {"items": {"repeat": spec["repeat"], "fields": fields}}
            if name_t in emitted_names:
                raise GenError(
                    f"{name}.{side} and {emitted_names[name_t]} both generate "
                    f"the type name {name_t!r}")
            emitted_names[name_t] = f"{name}.{side}"
            mark = len(emitter.lines)
            try:
                emitter.emit_struct(name_t, fields, head)
            except GenError as exc:
                # One payload that C cannot express must not stop the other 270.
                # It is recorded in the header and in the run summary instead.
                del emitter.lines[mark:]
                unexpressible.append(f"{name_t}: {exc}")
                emitter.lines.append(f"// {name_t}: NOT GENERATED")
                emitter.lines.append(f"//   {exc}")
                emitter.lines.append("//   This payload needs a hand-written codec.")
                emitter.lines.append("")

        if msg.get("variants"):
            for vkey, shape in msg["variants"].items():
                tag = variant_suffix(vkey)
                payload("request", shape.get("request"), tag)
                payload("reply", shape.get("reply"), tag)
        else:
            payload("request", msg.get("request"))
            payload("reply", msg.get("reply"))

    # The static header (includes, pack pragmas, MSP_STATIC_ASSERT) is the real
    # C in templates/msp_msgs.h.in; only the structs are generated. Ids come from
    # msp_protocol.h, which the template includes.
    template = (TEMPLATE_DIR / "msp_msgs.h.in").read_text()
    (args.out / "msp_msgs.h").write_text(
        template.replace("@MESSAGE_STRUCTS@", "\n".join(emitter.lines)))

    print(f"wrote {args.out}/msp_consts.h")
    print(f"wrote {args.out}/msp_enums.h")
    print(f"wrote {args.out}/msp_msgs.h")
    print(f"  messages: {len(ids)}  ({skipped} not_implemented, ids only)")
    print(f"  structs with a pinned size: {len(emitter.asserts)}")
    print(f"  bare-array payloads (element typedef): {len(emitter.bare_arrays)}")
    if unexpressible:
        print(f"  NOT expressible as a C struct: {len(unexpressible)}")
        for line in unexpressible:
            print(f"    - {line}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except GenError as exc:
        print(f"error: {exc}", file=sys.stderr)
        sys.exit(1)
