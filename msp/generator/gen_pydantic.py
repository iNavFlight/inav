"""Generate a Pydantic package from MSP schema documents.

Usage:
    python gen_pydantic.py schema/*.yaml
    python gen_pydantic.py schema/inav_msp.yaml --output-dir out
    python gen_pydantic.py schema/inav_msp.yaml --check
"""

from __future__ import annotations

import argparse
import keyword
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

import yaml
from yaml.nodes import MappingNode, ScalarNode, SequenceNode
from yaml.tokens import (
    AliasToken,
    AnchorToken,
    FlowMappingStartToken,
    FlowSequenceStartToken,
    TagToken,
)

from enum_syntax import EnumSyntaxError, split_member

YAML_FORBIDDEN_TOKENS = (
    AliasToken,
    AnchorToken,
    TagToken,
    FlowMappingStartToken,
    FlowSequenceStartToken,
)

IDENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
PACKAGE_RE = re.compile(r"^[a-z][a-z0-9_]*(\.[a-z][a-z0-9_]*)*$")
TYPE_RE = re.compile(
    r"^(?P<optional>optional\s+)?"
    r"(?P<base>[A-Za-z_][A-Za-z0-9_]*(?:\.[A-Za-z_][A-Za-z0-9_]*)*)"
    r"(?:\[(?P<count>[^\]]*)\])?$"
)

INT_PRIMITIVES = {
    "int8": (-(2**7), 2**7 - 1),
    "int16": (-(2**15), 2**15 - 1),
    "int32": (-(2**31), 2**31 - 1),
    "int64": (-(2**63), 2**63 - 1),
    "uint8": (0, 2**8 - 1),
    "uint16": (0, 2**16 - 1),
    "uint32": (0, 2**32 - 1),
    "uint64": (0, 2**64 - 1),
}
PRIMITIVES = set(INT_PRIMITIVES) | {"bool", "float32", "float64", "char", "cstring"}

# Payload types INAV declares; see generator/extract_wire_types.py. The schema
# references them without restating their layout, so they are opaque here: a
# model carries their bytes rather than pretending to know the fields.
EXTERNAL_TYPES = {"escSensorData_t", "ledConfig_t", "boxBitmask_t"}

TOP_LEVEL_KEYS = {
    "external_types",
    "version",
    "package",
    "imports",
    "constants",
    "aliases",
    "enums",
    "bitmasks",
    "structs",
    "messages",
}
ENUM_KEYS = {"prefix", "storage", "description", "values", "zero"}
STRUCT_KEYS = {"description", "fields", "unresolved"}
MESSAGE_KEYS = {"id", "request", "reply", "description", "mspv", "group", "notes",
                "variable_len", "not_implemented", "replaced_by", "variants",
                "inav_version"}
INAV_VERSION_KEYS = {"since", "until"}
PAYLOAD_KEYS = {"fields", "repeat", "description"}
PLAIN_FIELD_KEYS = {"type", "unit", "description", "value", "bitmask", "enum"}
DISCRIMINATED_FIELD_KEYS = {"discriminated", "description"}
GROUP_FIELD_KEYS = {"repeat", "fields", "description"}
CONSTANT_KEYS = {"type", "value", "description", "configurable", "header"}


class SchemaError(RuntimeError):
    pass


# --------------------------------------------------------------------------
# model
# --------------------------------------------------------------------------


@dataclass
class TypeRef:
    base: str
    optional: bool = False
    array: str = "none"  # none | fixed | const | field | open
    count: int | str | None = None

    @property
    def is_array(self) -> bool:
        return self.array != "none"

    @property
    def is_text(self) -> bool:
        return self.base == "char" and self.is_array


@dataclass
class Constant:
    name: str
    type: str
    value: object
    description: str | None = None
    configurable: str | None = None


@dataclass
class Alias:
    name: str
    target: str


@dataclass
class EnumMember:
    name: str
    value: int | None = None
    alias_of: str | None = None
    # Set when the member only exists under a build condition; see idl_spec.md
    # section 9.1. The member is still declared, so decoders keep working.
    condition: str | None = None


@dataclass
class EnumDef:
    name: str
    storage: str
    prefix: str | None = None
    description: str | None = None
    members: list[EnumMember] = field(default_factory=list)
    is_bitmask: bool = False
    zero: str | None = None


@dataclass
class PlainField:
    name: str
    type: TypeRef
    unit: str | None = None
    description: str | None = None
    value: object = None


@dataclass
class GroupField:
    name: str
    repeat: int | str
    fields: list[PlainField]
    description: str | None = None


@dataclass
class StructDef:
    name: str
    fields: list[PlainField | GroupField]
    description: str | None = None


@dataclass
class Payload:
    fields: list[PlainField | GroupField]
    repeat: int | str | None = None
    description: str | None = None


@dataclass
class MessageDef:
    name: str
    id: int
    request: Payload | None = None
    reply: Payload | None = None
    description: str | None = None
    mspv: int | None = None
    group: str | None = None
    notes: str | None = None
    variable_len: object = False
    not_implemented: bool = False
    replaced_by: list[str] = field(default_factory=list)
    # Length-dispatched shapes, keyed by the condition text from the source.
    variants: dict[str, dict] = field(default_factory=dict)


@dataclass
class Document:
    package: str
    path: Path
    version: int
    imports: list[str] = field(default_factory=list)
    constants: list[Constant] = field(default_factory=list)
    aliases: list[Alias] = field(default_factory=list)
    enums: list[EnumDef] = field(default_factory=list)
    structs: list[StructDef] = field(default_factory=list)
    messages: list[MessageDef] = field(default_factory=list)


# --------------------------------------------------------------------------
# yaml subset
# --------------------------------------------------------------------------


def validate_yaml_node(path: Path, node) -> None:
    if isinstance(node, MappingNode):
        seen: set[str] = set()
        for key_node, value_node in node.value:
            if not isinstance(key_node, ScalarNode):
                raise SchemaError(f"{path}: mapping keys must be scalars")
            if key_node.value in seen:
                raise SchemaError(f"{path}: duplicate key {key_node.value!r}")
            seen.add(key_node.value)
            validate_yaml_node(path, value_node)
    elif isinstance(node, SequenceNode):
        for value_node in node.value:
            validate_yaml_node(path, value_node)
    elif not isinstance(node, ScalarNode):
        raise SchemaError(f"{path}: unsupported YAML node")


def load_yaml(path: Path) -> dict:
    text = path.read_text()
    if "\t" in text:
        raise SchemaError(f"{path}: tabs are not allowed")
    for token in yaml.scan(text):
        if isinstance(token, YAML_FORBIDDEN_TOKENS):
            raise SchemaError(f"{path}: unsupported YAML token {type(token).__name__}")
    node = yaml.compose(text)
    if node is None:
        raise SchemaError(f"{path}: empty document")
    validate_yaml_node(path, node)
    data = yaml.safe_load(text)
    if not isinstance(data, dict):
        raise SchemaError(f"{path}: document root must be a mapping")
    return data


# --------------------------------------------------------------------------
# parsing
# --------------------------------------------------------------------------


def check_keys(where: str, spec: dict, allowed: set[str]) -> None:
    unknown = sorted(set(spec) - allowed)
    if unknown:
        raise SchemaError(f"{where}: unknown keys {unknown}")


def check_ident(where: str, name: str) -> str:
    if not isinstance(name, str) or not IDENT_RE.match(name):
        raise SchemaError(f"{where}: invalid identifier {name!r}")
    return name


def parse_type(where: str, text: str) -> TypeRef:
    if not isinstance(text, str):
        raise SchemaError(f"{where}: type must be a string, got {text!r}")
    match = TYPE_RE.match(text.strip())
    if not match:
        raise SchemaError(f"{where}: invalid type expression {text!r}")
    base = match.group("base")
    optional = match.group("optional") is not None
    raw_count = match.group("count")
    if raw_count is None:
        return TypeRef(base=base, optional=optional)
    count = raw_count.strip()
    if count == "":
        return TypeRef(base, optional, "open", None)
    if re.fullmatch(r"\d+", count):
        value = int(count)
        if value <= 0:
            raise SchemaError(f"{where}: array size must be positive in {text!r}")
        return TypeRef(base, optional, "fixed", value)
    if not IDENT_RE.match(count):
        raise SchemaError(f"{where}: invalid array size {count!r} in {text!r}")
    # constant vs field is resolved during validation
    return TypeRef(base, optional, "const", count)


def parse_constants(spec: dict) -> list[Constant]:
    out = []
    for name, body in spec.items():
        where = f"constant {name}"
        check_ident(where, name)
        if isinstance(body, str):
            match = re.fullmatch(r"\s*([A-Za-z_]\w*)\s*=\s*(.+?)\s*", body)
            if not match:
                raise SchemaError(f"{where}: expected 'type = value', got {body!r}")
            ctype, raw = match.group(1), match.group(2)
            value = yaml.safe_load(raw)
            out.append(Constant(name, ctype, value))
        elif isinstance(body, dict):
            check_keys(where, body, CONSTANT_KEYS)
            configurable = body.get("configurable")
            if "type" not in body:
                raise SchemaError(f"{where}: requires 'type'")
            if configurable is None and "value" not in body:
                raise SchemaError(f"{where}: requires 'value' unless 'configurable'")
            if configurable is not None and "value" in body:
                raise SchemaError(f"{where}: 'configurable' and 'value' are exclusive")
            out.append(
                Constant(
                    name,
                    body["type"],
                    body.get("value"),
                    body.get("description"),
                    configurable,
                )
            )
        else:
            raise SchemaError(f"{where}: must be a string or mapping")
    return out


def parse_aliases(spec: dict) -> list[Alias]:
    out = []
    for name, target in spec.items():
        check_ident(f"alias {name}", name)
        if not isinstance(target, str):
            raise SchemaError(f"alias {name}: target must be a string")
        out.append(Alias(name, target.strip()))
    return out


def _member_from_value(where: str, name: str, raw, condition: str | None) -> EnumMember:
    check_ident(where, name)
    if isinstance(raw, int) and not isinstance(raw, bool):
        return EnumMember(name, value=raw, condition=condition)
    text = str(raw).strip()
    if re.fullmatch(r"-?\d+", text):
        return EnumMember(name, value=int(text), condition=condition)
    if re.fullmatch(r"0[xX][0-9a-fA-F]+", text):
        return EnumMember(name, value=int(text, 16), condition=condition)
    if IDENT_RE.match(text):
        return EnumMember(name, alias_of=text, condition=condition)
    raise SchemaError(
        f"{where}: enum value must be an integer or a member name, got {raw!r}")


def parse_enum_member(where: str, entry) -> EnumMember:
    try:
        name, raw, condition = split_member(entry)
    except EnumSyntaxError as exc:
        raise SchemaError(f"{where}: {exc}") from exc
    if raw is None:
        if condition is not None:
            raise SchemaError(f"{where}: conditional member {name!r} needs a value")
        return EnumMember(check_ident(where, name))
    return _member_from_value(where, name, raw, condition)


def parse_enum(name: str, spec: dict, is_bitmask: bool) -> EnumDef:
    kind = "bitmask" if is_bitmask else "enum"
    where = f"{kind} {name}"
    check_ident(where, name)
    if not isinstance(spec, dict):
        raise SchemaError(f"{where}: must be a mapping")
    check_keys(where, spec, ENUM_KEYS)
    storage = spec.get("storage")
    if storage not in INT_PRIMITIVES:
        raise SchemaError(f"{where}: storage must be an integer primitive, got {storage!r}")
    values = spec.get("values")
    if not isinstance(values, list) or not values:
        raise SchemaError(f"{where}: 'values' must be a non-empty list")
    members = [parse_enum_member(where, entry) for entry in values]
    zero = spec.get("zero")
    if zero is not None:
        if not is_bitmask:
            raise SchemaError(f"{where}: 'zero' is only valid on a bitmask")
        check_ident(where, zero)
    return EnumDef(
        name=name,
        storage=storage,
        prefix=spec.get("prefix"),
        description=spec.get("description"),
        members=members,
        is_bitmask=is_bitmask,
        zero=zero,
    )


def parse_field(where: str, name: str, spec) -> PlainField | GroupField:
    if isinstance(spec, dict) and "discriminated" in spec:
        check_keys(where, spec, DISCRIMINATED_FIELD_KEYS)
        d = spec["discriminated"]
        if d.get("by") != "leading-zero":
            raise SchemaError(f"{where}: unknown discriminator {d.get('by')!r}")
        # Modelled as opaque here: the Pydantic models describe one shape per
        # field, and this field is two. The codec resolves it at runtime.
        return PlainField(name=name, type=parse_type(where, "cstring"),
                          description=spec.get("description"))

    check_ident(where, name)
    if isinstance(spec, str):
        return PlainField(name, parse_type(where, spec))
    if not isinstance(spec, dict):
        raise SchemaError(f"{where}: field must be a string or mapping")
    if "repeat" in spec or ("fields" in spec and "type" not in spec):
        check_keys(where, spec, GROUP_FIELD_KEYS)
        if "repeat" not in spec or "fields" not in spec:
            raise SchemaError(f"{where}: repeated group needs both 'repeat' and 'fields'")
        inner = parse_fields(where, spec["fields"])
        for sub in inner:
            if isinstance(sub, GroupField):
                raise SchemaError(f"{where}: repeated groups may not nest")
        return GroupField(name, spec["repeat"], inner, spec.get("description"))
    for banned in ("required", "default", "const"):
        if banned in spec:
            raise SchemaError(f"{where}: '{banned}' is not an MSP schema field key")
    check_keys(where, spec, PLAIN_FIELD_KEYS)
    if "type" not in spec:
        raise SchemaError(f"{where}: expanded field requires 'type'")
    return PlainField(
        name,
        parse_type(where, spec["type"]),
        spec.get("unit"),
        spec.get("description"),
        spec.get("value"),
    )


def parse_fields(where: str, spec) -> list[PlainField | GroupField]:
    if not isinstance(spec, dict) or not spec:
        raise SchemaError(f"{where}: 'fields' must be a non-empty mapping")
    return [parse_field(f"{where}.{n}", n, body) for n, body in spec.items()]


def parse_struct(name: str, spec: dict) -> StructDef:
    where = f"struct {name}"
    check_ident(where, name)
    if not isinstance(spec, dict):
        raise SchemaError(f"{where}: must be a mapping")
    check_keys(where, spec, STRUCT_KEYS)
    if "fields" not in spec:
        raise SchemaError(f"{where}: requires 'fields'")
    return StructDef(name, parse_fields(where, spec["fields"]), spec.get("description"))


def parse_payload(where: str, spec) -> Payload | None:
    if spec is None:
        return None
    if not isinstance(spec, dict):
        raise SchemaError(f"{where}: payload must be null or a mapping")
    check_keys(where, spec, PAYLOAD_KEYS)
    if "fields" not in spec:
        raise SchemaError(f"{where}: payload requires 'fields'")
    return Payload(
        parse_fields(where, spec["fields"]), spec.get("repeat"), spec.get("description")
    )


def parse_message(name: str, spec: dict) -> MessageDef:
    where = f"message {name}"
    check_ident(where, name)
    if not isinstance(spec, dict):
        raise SchemaError(f"{where}: must be a mapping")
    check_keys(where, spec, MESSAGE_KEYS)
    if "id" not in spec:
        raise SchemaError(f"{where}: requires 'id'")
    if not isinstance(spec["id"], int) or isinstance(spec["id"], bool):
        raise SchemaError(f"{where}: 'id' must be an integer")

    iv = spec.get("inav_version")
    if iv is not None:
        if not isinstance(iv, dict):
            raise SchemaError(f"{where}.inav_version: must be a mapping")
        check_keys(f"{where}.inav_version", iv, INAV_VERSION_KEYS)
        if "since" not in iv:
            raise SchemaError(f"{where}.inav_version: requires 'since' (the version it was introduced)")

    variants: dict[str, dict] = {}
    if spec.get("variants"):
        # A length-dispatched message declares its shapes instead of a single
        # request/reply pair, so those keys are absent by design.
        for key, shape in spec["variants"].items():
            if not isinstance(shape, dict):
                raise SchemaError(f"{where}.variants[{key}]: must be a mapping")
            variants[key] = {
                side: parse_payload(f"{where}.variants[{key}].{side}", shape.get(side))
                for side in ("request", "reply")
            }
    elif not spec.get("not_implemented"):
        for required in ("request", "reply"):
            if required not in spec:
                raise SchemaError(f"{where}: requires '{required}'")

    return MessageDef(
        name=name,
        id=spec["id"],
        request=parse_payload(f"{where}.request", spec.get("request")),
        reply=parse_payload(f"{where}.reply", spec.get("reply")),
        description=spec.get("description"),
        mspv=spec.get("mspv"),
        group=spec.get("group"),
        notes=spec.get("notes"),
        variable_len=spec.get("variable_len", False),
        not_implemented=bool(spec.get("not_implemented")),
        replaced_by=list(spec.get("replaced_by") or []),
        variants=variants,
    )


def parse_document(path: Path) -> Document:
    data = load_yaml(path)
    check_keys(str(path), data, TOP_LEVEL_KEYS)
    for required in ("version", "package"):
        if required not in data:
            raise SchemaError(f"{path}: missing '{required}'")
    package = data["package"]
    if not isinstance(package, str) or not PACKAGE_RE.match(package):
        raise SchemaError(f"{path}: invalid package name {package!r}")

    doc = Document(package=package, path=path, version=data["version"])
    doc.imports = list(data.get("imports") or [])
    doc.constants = parse_constants(data.get("constants") or {})
    doc.aliases = parse_aliases(data.get("aliases") or {})
    doc.enums = [
        parse_enum(n, s, False) for n, s in (data.get("enums") or {}).items()
    ] + [parse_enum(n, s, True) for n, s in (data.get("bitmasks") or {}).items()]
    doc.structs = [parse_struct(n, s) for n, s in (data.get("structs") or {}).items()]
    doc.messages = [
        parse_message(n, s) for n, s in (data.get("messages") or {}).items()
    ]
    return doc


# --------------------------------------------------------------------------
# validation
# --------------------------------------------------------------------------


class Validator:
    def __init__(self, docs: list[Document]):
        self.docs = docs
        self.errors: list[str] = []
        self.types: dict[str, str] = {}
        self.constants: dict[str, Constant] = {}
        self.structs: dict[str, StructDef] = {}
        self.enums: dict[str, EnumDef] = {}
        self.aliases: dict[str, Alias] = {}

    def error(self, message: str) -> None:
        self.errors.append(message)

    def run(self) -> None:
        self.index()
        for doc in self.docs:
            for alias in doc.aliases:
                self.check_alias(alias)
            for enum in doc.enums:
                self.check_enum(enum)
            for struct in doc.structs:
                self.check_fields(f"struct {struct.name}", struct.fields, in_payload=False)
            for message in doc.messages:
                for side in ("request", "reply"):
                    payload = getattr(message, side)
                    if payload is not None:
                        self.check_payload(f"message {message.name}.{side}", payload)
        if self.errors:
            raise SchemaError("\n".join(f"  - {e}" for e in self.errors))

    def index(self) -> None:
        seen_packages: set[str] = set()
        for doc in self.docs:
            if doc.package in seen_packages:
                self.error(f"duplicate package {doc.package}")
            seen_packages.add(doc.package)
            for collection, store in (
                (doc.aliases, self.aliases),
                (doc.enums, self.enums),
                (doc.structs, self.structs),
            ):
                for item in collection:
                    if item.name in self.types:
                        self.error(f"duplicate type name {item.name}")
                    self.types[item.name] = doc.package
                    store[item.name] = item
            for constant in doc.constants:
                if constant.name in self.constants:
                    self.error(f"duplicate constant {constant.name}")
                self.constants[constant.name] = constant
            ids: dict[int, str] = {}
            names: set[str] = set()
            for message in doc.messages:
                if message.name in names:
                    self.error(f"duplicate message {message.name}")
                names.add(message.name)
                if message.id in ids:
                    self.error(
                        f"duplicate message id {message.id} "
                        f"({ids[message.id]} and {message.name})"
                    )
                ids[message.id] = message.name

    def check_alias(self, alias: Alias) -> None:
        seen = {alias.name}
        target = alias.target
        while target not in PRIMITIVES:
            if target in self.enums or target in self.structs:
                self.error(f"alias {alias.name}: may not target a struct, enum, or bitmask")
                return
            if target not in self.aliases:
                self.error(f"alias {alias.name}: unknown target {target!r}")
                return
            if target in seen:
                self.error(f"alias {alias.name}: alias cycle")
                return
            seen.add(target)
            target = self.aliases[target].target

    def check_enum(self, enum: EnumDef) -> None:
        where = ("bitmask " if enum.is_bitmask else "enum ") + enum.name
        low, high = INT_PRIMITIVES[enum.storage]
        width = high.bit_length() + (1 if low < 0 else 0)
        names: set[str] = set()
        literals: dict[int, str] = {}
        if enum.zero is not None:
            names.add(enum.zero)
        for member in enum.members:
            if member.name in names:
                self.error(f"{where}: duplicate member {member.name}")
            names.add(member.name)
            if member.alias_of is not None:
                if member.alias_of not in names:
                    self.error(
                        f"{where}: member {member.name} aliases "
                        f"{member.alias_of!r}, which is not an earlier member"
                    )
                continue
            if member.value is None:
                continue
            if enum.is_bitmask:
                if not 0 <= member.value < width:
                    self.error(
                        f"{where}: bit position {member.value} out of range "
                        f"for {enum.storage}"
                    )
            elif not low <= member.value <= high:
                self.error(
                    f"{where}: value {member.value} does not fit in {enum.storage}"
                )
            # Distinct enum names may share a value (SERVO_BICOPTER_LEFT and
            # SERVO_FLAPPERON_2 are both 4). Bit positions may not.
            if enum.is_bitmask and member.value in literals:
                self.error(
                    f"{where}: duplicate bit position {member.value} "
                    f"({literals[member.value]} and {member.name})"
                )
            literals[member.value] = member.name

    def check_payload(self, where: str, payload: Payload) -> None:
        self.check_repeat(where, payload.repeat, [], allow_until_end=True)
        self.check_fields(where, payload.fields, in_payload=True)

    def check_repeat(self, where: str, repeat, preceding, allow_until_end: bool) -> None:
        if repeat is None:
            return
        if isinstance(repeat, int) and not isinstance(repeat, bool):
            if repeat <= 0:
                self.error(f"{where}: repeat must be positive")
            return
        if repeat == "until_end":
            if not allow_until_end:
                self.error(f"{where}: 'until_end' is only valid in a message payload")
            return
        if not isinstance(repeat, str) or not IDENT_RE.match(repeat):
            self.error(f"{where}: invalid repeat {repeat!r}")
            return
        if repeat in self.constants:
            return
        if repeat in preceding:
            return
        self.error(f"{where}: repeat {repeat!r} is not a constant or preceding field")

    def check_fields(self, where: str, fields, in_payload: bool) -> None:
        seen: list[str] = []
        saw_optional = False
        saw_open = False
        for index, item in enumerate(fields):
            last = index == len(fields) - 1
            if item.name in seen:
                self.error(f"{where}: duplicate field {item.name}")
            if isinstance(item, GroupField):
                self.check_repeat(
                    f"{where}.{item.name}",
                    item.repeat,
                    seen,
                    allow_until_end=in_payload and last,
                )
                self.check_fields(f"{where}.{item.name}", item.fields, in_payload=False)
                seen.append(item.name)
                continue

            ref = item.type
            self.check_type(f"{where}.{item.name}", ref, seen)
            if ref.optional:
                saw_optional = True
            elif saw_optional:
                self.error(
                    f"{where}.{item.name}: required field follows an optional field; "
                    f"optional fields must form a contiguous suffix"
                )
            if ref.array == "open":
                if not (in_payload and last):
                    self.error(
                        f"{where}.{item.name}: open-ended array is only valid "
                        f"as the final field of a message payload"
                    )
                saw_open = True
            seen.append(item.name)
        # A trailing open array may follow optional fields: the optionals are
        # fixed width and consumed in order, then the array takes the rest.

    def check_type(self, where: str, ref: TypeRef, preceding: list[str]) -> None:
        base = ref.base.split(".")[-1] if "." in ref.base else ref.base
        if base == "cstring" and ref.is_array:
            self.error(f"{where}: cstring is self-delimiting and takes no array size")
        if base not in PRIMITIVES and base not in self.types and base not in EXTERNAL_TYPES:
            self.error(f"{where}: unknown type {ref.base!r}")
        if ref.array == "const":
            name = str(ref.count)
            if name in self.constants:
                constant = self.constants[name]
                if constant.configurable is None and (
                    not isinstance(constant.value, int) or constant.value <= 0
                ):
                    self.error(f"{where}: array size constant {name!r} must be a positive integer")
                ref.array = "const"
            elif name in preceding:
                ref.array = "field"
            else:
                self.error(
                    f"{where}: array size {name!r} is neither a constant "
                    f"nor a preceding field"
                )


# --------------------------------------------------------------------------
# emission
# --------------------------------------------------------------------------



def safe_attr(name: str) -> str:
    if keyword.iskeyword(name) or name in {"model_config", "model_fields"}:
        return name + "_"
    return name


def module_name(package: str) -> str:
    return package.replace(".", "_")


class Emitter:
    def __init__(self, doc: Document, validator: Validator):
        self.doc = doc
        self.v = validator
        self.lines: list[str] = []

    def write(self, text: str = "") -> None:
        self.lines.append(text)

    def render(self) -> str:
        self.header()
        self.constants()
        self.aliases()
        self.enums()
        self.structs()
        self.messages()
        return "\n".join(self.lines).rstrip() + "\n"

    def header(self) -> None:
        self.write('"""Generated by gen_pydantic.py from the MSP YAML schema. Do not edit."""')
        self.write()
        self.write("from __future__ import annotations")
        self.write()
        self.write("from enum import IntEnum, IntFlag")
        self.write("from typing import Annotated, Final")
        self.write()
        self.write("from pydantic import BaseModel, ConfigDict, Field")
        self.write()
        for name in self.doc.imports:
            self.write(f"from .{module_name(name)} import *  # noqa: F403")
        if self.doc.imports:
            self.write()
        self.write()
        used = self.used_int_primitives()
        if used:
            self.write("# width-constrained integer aliases")
            for name in sorted(used, key=lambda n: (n[0] != "i", n)):
                low, high = INT_PRIMITIVES[name]
                self.write(f"{name} = Annotated[int, Field(ge={low}, le={high})]")
            self.write()
            self.write()

    def used_int_primitives(self) -> set[str]:
        used: set[str] = set()

        def visit_fields(fields):
            for item in fields:
                if isinstance(item, GroupField):
                    visit_fields(item.fields)
                    continue
                base = self.resolve_primitive(item.type.base)
                if base in INT_PRIMITIVES and not item.type.is_text:
                    used.add(base)

        for struct in self.doc.structs:
            visit_fields(struct.fields)
        for message in self.doc.messages:
            for payload in (message.request, message.reply):
                if payload:
                    visit_fields(payload.fields)
        return used

    def resolve_primitive(self, name: str) -> str:
        seen = set()
        while name in self.v.aliases and name not in seen:
            seen.add(name)
            name = self.v.aliases[name].target
        return name

    def annotation(self, ref: TypeRef) -> str:
        base = ref.base.split(".")[-1]
        if ref.is_text:
            inner = "str"
        else:
            resolved = self.resolve_primitive(base)
            if resolved == "bool":
                inner = "bool"
            elif resolved in ("float32", "float64"):
                inner = "float"
            elif resolved in ("char", "cstring"):
                inner = "str"
            elif resolved in INT_PRIMITIVES:
                inner = base if base in self.v.aliases or base in self.v.enums else resolved
            elif resolved in EXTERNAL_TYPES:
                inner = "bytes"
            else:
                inner = base
            if ref.is_array:
                inner = f"list[{inner}]"
        if ref.optional:
            inner = f"{inner} | None"
        return inner

    def field_line(self, item: PlainField, indent: str = "    ") -> list[str]:
        annotation = self.annotation(item.type)
        name = safe_attr(item.name)
        bits = []
        if item.type.optional:
            bits.append("default=None")
        if item.type.array == "fixed" and not item.type.is_text:
            bits.append(f"min_length={item.type.count}")
            bits.append(f"max_length={item.type.count}")
        if item.type.is_text and item.type.array == "fixed":
            bits.append(f"max_length={item.type.count}")
        doc = " ".join(x for x in (item.description, f"[{item.unit}]" if item.unit else None) if x)
        if doc:
            bits.append(f"description={doc.strip()!r}")
        if name != item.name:
            bits.append(f"alias={item.name!r}")
        if bits:
            assignment = f" = Field({', '.join(bits)})"
        else:
            assignment = ""
        return [f"{indent}{name}: {annotation}{assignment}"]

    def render_fields(self, fields, prefix: str, indent: str = "    ") -> tuple[list[str], list[str]]:
        """Return (body lines, nested class blocks)."""
        body: list[str] = []
        nested: list[str] = []
        for item in fields:
            if isinstance(item, GroupField):
                group_name = f"{prefix}{item.name}Item"
                inner_body, inner_nested = self.render_fields(item.fields, group_name)
                nested.extend(inner_nested)
                block = [f"class {group_name}(BaseModel):"]
                block.append("    model_config = ConfigDict(populate_by_name=True)")
                block.append("")
                block.extend(inner_body)
                nested.append("\n".join(block))
                body.append(f"{indent}{safe_attr(item.name)}: list[{group_name}]")
            else:
                body.extend(self.field_line(item, indent))
        return body, nested

    def model_block(self, name: str, fields, description: str | None) -> str:
        body, nested = self.render_fields(fields, name)
        out = []
        for block in nested:
            out.append(block)
            out.append("")
            out.append("")
        out.append(f"class {name}(BaseModel):")
        if description:
            out.append(f'    """{description}"""')
            out.append("")
        out.append("    model_config = ConfigDict(populate_by_name=True)")
        out.append("")
        out.extend(body)
        return "\n".join(out)

    def constants(self) -> None:
        if not self.doc.constants:
            return
        for constant in self.doc.constants:
            hint = "int" if isinstance(constant.value, int) else type(constant.value).__name__
            self.write(f"{constant.name}: Final[{hint}] = {constant.value!r}")
        self.write()
        self.write()

    def aliases(self) -> None:
        if not self.doc.aliases:
            return
        for alias in self.doc.aliases:
            resolved = self.resolve_primitive(alias.target)
            if resolved in INT_PRIMITIVES:
                low, high = INT_PRIMITIVES[resolved]
                self.write(f"{alias.name} = Annotated[int, Field(ge={low}, le={high})]")
            elif resolved in ("float32", "float64"):
                self.write(f"{alias.name} = float")
            elif resolved == "char":
                self.write(f"{alias.name} = str")
            else:
                self.write(f"{alias.name} = {resolved}")
        self.write()
        self.write()

    def enums(self) -> None:
        for enum in self.doc.enums:
            base = "IntFlag" if enum.is_bitmask else "IntEnum"
            self.write(f"class {enum.name}({base}):")
            if enum.description:
                self.write(f'    """{enum.description}"""')
                self.write()
            emitted: dict[str, int] = {}
            counter = 0
            if enum.zero is not None:
                label = enum.zero
                if enum.prefix and not label.startswith(enum.prefix):
                    label = f"{enum.prefix}_{label}"
                self.write(f"    {label} = 0")
            for member in enum.members:
                label = member.name
                if enum.prefix and not label.startswith(enum.prefix):
                    label = f"{enum.prefix}_{label}"
                if member.alias_of is not None:
                    value = emitted.get(member.alias_of, 0)
                else:
                    value = member.value if member.value is not None else counter
                emitted[member.name] = value
                counter = value + 1
                literal = f"1 << {value}" if enum.is_bitmask else str(value)
                self.write(f"    {label} = {literal}")
            self.write()
            self.write()

    def structs(self) -> None:
        for struct in self.order_structs():
            self.write(self.model_block(struct.name, struct.fields, struct.description))
            self.write()
            self.write()

    def order_structs(self) -> list[StructDef]:
        by_name = {s.name: s for s in self.doc.structs}
        ordered: list[StructDef] = []
        state: dict[str, int] = {}

        def visit(struct: StructDef) -> None:
            if state.get(struct.name) == 2:
                return
            if state.get(struct.name) == 1:
                raise SchemaError(f"struct dependency cycle at {struct.name}")
            state[struct.name] = 1
            for item in struct.fields:
                refs = item.fields if isinstance(item, GroupField) else [item]
                for sub in refs:
                    dep = sub.type.base.split(".")[-1]
                    if dep in by_name and dep != struct.name:
                        visit(by_name[dep])
            state[struct.name] = 2
            ordered.append(struct)

        for struct in self.doc.structs:
            visit(struct)
        return ordered

    def messages(self) -> None:
        if not self.doc.messages:
            return
        registry: list[str] = []
        for message in self.doc.messages:
            base = message.name
            self.write(f"{message.name}: Final[int] = {message.id}")
            self.write()
            entry: list[str] = []
            for side in ("request", "reply"):
                payload = getattr(message, side)
                if payload is None:
                    entry.append("None")
                    continue
                cls = f"{base}_{side}_t"   # matches the C struct names
                if payload.repeat is not None:
                    item_cls = f"{cls}Item"
                    self.write(self.model_block(item_cls, payload.fields, payload.description))
                    self.write()
                    self.write()
                    self.write(f"class {cls}(BaseModel):")
                    self.write("    model_config = ConfigDict(populate_by_name=True)")
                    self.write()
                    self.write(f"    items: list[{item_cls}]")
                    repeat = payload.repeat
                    self.write(f"    # repeat: {repeat}")
                else:
                    self.write(
                        self.model_block(
                            cls, payload.fields, payload.description or message.description
                        )
                    )
                self.write()
                self.write()
                entry.append(cls)
            registry.append(f"    {message.name}: ({', '.join(entry)}),")
        self.write("# message id -> (request model, reply model), None where absent")
        self.write("MESSAGES: Final[dict[int, tuple]] = {")
        for line in registry:
            self.write(line)
        self.write("}")
        self.write()


# --------------------------------------------------------------------------
# driver
# --------------------------------------------------------------------------


def render_init(docs: list[Document]) -> str:
    lines = ['"""Generated by gen_pydantic.py from the MSP YAML schema. Do not edit."""', ""]
    for doc in docs:
        lines.append(f"from . import {module_name(doc.package)}  # noqa: F401")
    lines.append("")
    lines.append("__all__ = [")
    for doc in docs:
        lines.append(f'    "{module_name(doc.package)}",')
    lines.append("]")
    return "\n".join(lines) + "\n"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate Pydantic models from the MSP YAML schema.")
    parser.add_argument("sources", nargs="+", type=Path, help="MSP schema YAML documents")
    parser.add_argument("--output-dir", type=Path, default=Path("schema"))
    parser.add_argument(
        "--check", action="store_true", help="validate only, write nothing"
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    paths: list[Path] = []
    for source in args.sources:
        paths.extend(sorted(source.glob("*.yaml")) if source.is_dir() else [source])

    try:
        docs = [parse_document(path) for path in paths]
        validator = Validator(docs)
        validator.run()
    except SchemaError as exc:
        print(f"error:\n{exc}", file=sys.stderr)
        return 1

    counts = {
        "documents": len(docs),
        "constants": sum(len(d.constants) for d in docs),
        "aliases": sum(len(d.aliases) for d in docs),
        "enums": sum(len(d.enums) for d in docs),
        "structs": sum(len(d.structs) for d in docs),
        "messages": sum(len(d.messages) for d in docs),
    }
    summary = ", ".join(f"{v} {k}" for k, v in counts.items())

    if args.check:
        print(f"ok: {summary}")
        return 0

    args.output_dir.mkdir(parents=True, exist_ok=True)
    for doc in docs:
        target = args.output_dir / f"{module_name(doc.package)}.py"
        target.write_text(Emitter(doc, validator).render())
    (args.output_dir / "__init__.py").write_text(render_init(docs))
    print(f"wrote {len(docs) + 1} files to {args.output_dir}: {summary}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
