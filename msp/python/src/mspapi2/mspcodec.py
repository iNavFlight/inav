"""MSP codec driven by the generated message table.

Everything is resolved at generation time in mspapi2.generated.messages; this
module only executes: look the message up, pack or unpack. Nothing here reads a
schema at runtime.

Decoding is lenient about trailing bytes, matching the firmware's own
forward-compatibility rule - MSP payloads only ever gain fields at the end, so a
newer peer's longer message decodes to the fields this build knows and the
remainder is ignored. Short payloads are still rejected.
"""
from __future__ import annotations

import struct
from typing import Any, Iterable, Mapping

from .generated.messages import BY_CODE, MESSAGES


class MSPCodecError(Exception):
    pass


class MSPUnpackError(MSPCodecError):
    pass


class MSPPackError(MSPCodecError):
    pass


def _spec(code) -> dict:
    key = int(code)
    entry = BY_CODE.get(key)
    if entry is None:
        raise MSPCodecError(f"unknown MSP code {key} (0x{key:04X})")
    if entry["not_implemented"]:
        raise MSPCodecError(f"{entry['name']} is not implemented by the firmware")
    return entry


def _scalar_fmt(field: dict) -> str:
    if field["kind"] == "scalar":
        return field["code"]
    if field["kind"] == "array":
        return f"{field['count']}s" if field["char"] else field["code"] * field["count"]
    raise MSPCodecError(f"not a fixed field: {field['kind']}")


_LAYOUT_FMT = {("uint8_t", 1): "B", ("int8_t", 1): "b", ("uint16_t", 2): "H",
               ("int16_t", 2): "h", ("uint32_t", 4): "I", ("int32_t", 4): "i",
               ("uint64_t", 8): "Q", ("int64_t", 8): "q"}


def _decode_layout(layout: dict, raw: bytes) -> dict:
    """Decode a type INAV declares, using the layout its own compiler reported."""
    if layout["kind"] == "bitfield":
        word = int.from_bytes(raw, "little")
        return {f["name"]: (word >> f["bit_offset"]) & ((1 << f["bits"]) - 1)
                for f in layout["fields"]}
    out = {}
    for f in layout["fields"]:
        fmt = _LAYOUT_FMT[(f["ctype"], f["size"])]
        out[f["name"]] = struct.unpack_from("<" + fmt, raw, f["offset"])[0]
    return out


def _encode_layout(layout: dict, values) -> bytes:
    if isinstance(values, (bytes, bytearray)):
        return bytes(values)
    if layout["kind"] == "bitfield":
        word = 0
        for f in layout["fields"]:
            word |= (int(values[f["name"]]) & ((1 << f["bits"]) - 1)) << f["bit_offset"]
        return word.to_bytes(layout["size"], "little")
    raw = bytearray(layout["size"])
    for f in layout["fields"]:
        fmt = _LAYOUT_FMT[(f["ctype"], f["size"])]
        struct.pack_into("<" + fmt, raw, f["offset"], int(values[f["name"]]))
    return bytes(raw)


def _decode(fields: list, buf: bytes, offset: int, name: str) -> tuple[dict, int]:
    out: dict = {}
    for field in fields:
        kind = field["kind"]

        if kind == "group":
            count = field["count"]
            if count is None:
                count_field = field.get("count_field")
                if count_field and count_field in out:
                    count = int(out[count_field])
                else:
                    count = None  # runs to the end
            records = []
            while offset < len(buf) and (count is None or len(records) < count):
                record, offset = _decode(field["fields"], buf, offset, name)
                records.append(record)
            if count is not None and len(records) != count:
                raise MSPUnpackError(
                    f"{name}.{field['name']}: expected {count} records, got {len(records)}")
            out[field["name"]] = records
            continue

        if kind == "struct":
            out[field["name"]], offset = _decode(field["fields"], buf, offset, name)
            continue

        if kind == "discriminated":
            # A leading NUL selects the index form; anything else is a name.
            case = "index" if buf[offset:offset + 1] == b"\x00" else "name"
            value, offset = _decode(field["cases"][case], buf, offset, name)
            out[field["name"]] = {"case": case, **value}
            continue

        if kind == "layout":
            size = field["size"]
            if offset + size > len(buf):
                if field.get("optional"):
                    return out, offset
                raise MSPUnpackError(
                    f"{name}.{field['name']}: needs {size} bytes for "
                    f"{field['type']}, payload ends at {len(buf)}")
            out[field["name"]] = _decode_layout(field["layout"],
                                                buf[offset:offset + size])
            offset += size
            continue

        if kind == "opaque":
            size = field["size"]
            if offset + size > len(buf):
                if field.get("optional"):
                    return out, offset
                raise MSPUnpackError(
                    f"{name}.{field['name']}: needs {size} bytes for "
                    f"{field['type']}, payload ends at {len(buf)}")
            out[field["name"]] = bytes(buf[offset:offset + size])
            offset += size
            continue

        if kind == "cstring":
            end = buf.find(b"\x00", offset)
            if end < 0:
                raise MSPUnpackError(f"{name}.{field['name']}: unterminated string")
            out[field["name"]] = buf[offset:end].decode("utf-8", "replace")
            offset = end + 1
            continue

        if kind == "tail":
            data = buf[offset:]
            out[field["name"]] = (data.decode("utf-8", "replace").rstrip("\x00")
                                  if field["char"] else list(
                                      struct.unpack(f"<{len(data)//field['unit']}{field['code']}",
                                                    data[: len(data) // field["unit"] * field["unit"]])))
            offset = len(buf)
            continue

        size = field["size"] if kind == "scalar" else field["unit"] * field["count"]
        if offset + size > len(buf):
            if field.get("optional"):
                return out, offset          # absent trailing field: stop here
            raise MSPUnpackError(
                f"{name}.{field['name']}: payload ends at {len(buf)}, needs {offset + size}")
        raw = struct.unpack_from("<" + _scalar_fmt(field), buf, offset)
        if kind == "array" and field["char"]:
            out[field["name"]] = raw[0].decode("utf-8", "replace").rstrip("\x00")
        elif kind == "array":
            out[field["name"]] = list(raw)
        else:
            out[field["name"]] = raw[0]
        offset += size
    return out, offset


def _encode(fields: list, values: Mapping[str, Any], name: str) -> bytes:
    parts: list[bytes] = []
    for field in fields:
        kind = field["kind"]
        present = field["name"] in values

        if not present:
            if field.get("optional"):
                break                      # optional tail: stop, nothing after it
            if kind == "group":
                raise MSPPackError(f"{name}: missing records for {field['name']!r}")
            raise MSPPackError(f"{name}: missing value for field {field['name']!r}")

        value = values[field["name"]]
        if kind == "group":
            for record in value:
                parts.append(_encode(field["fields"], record, name))
            continue
        if kind == "struct":
            parts.append(_encode(field["fields"], value, name))
            continue
        if kind == "discriminated":
            case = value.get("case") or ("index" if "settingIndex" in value else "name")
            parts.append(_encode(field["cases"][case], value, name))
            continue

        if kind == "layout":
            parts.append(_encode_layout(field["layout"], value))
            continue

        if kind == "opaque":
            raw = bytes(value)
            if len(raw) != field["size"]:
                raise MSPPackError(
                    f"{name}.{field['name']}: {field['type']} is {field['size']} "
                    f"bytes, got {len(raw)}")
            parts.append(raw)
            continue

        if kind == "cstring":
            parts.append(str(value).encode("utf-8") + b"\x00")
            continue
        if kind == "tail":
            if field["char"]:
                parts.append(str(value).encode("utf-8"))
            else:
                parts.append(struct.pack(f"<{len(value)}{field['code']}", *value))
            continue
        if kind == "array" and field["char"]:
            parts.append(struct.pack(f"<{field['count']}s",
                                     str(value).encode("utf-8")[: field["count"]]))
        elif kind == "array":
            if len(value) != field["count"]:
                raise MSPPackError(
                    f"{name}.{field['name']}: expected {field['count']} items, got {len(value)}")
            parts.append(struct.pack("<" + _scalar_fmt(field), *value))
        elif field["code"] in "fd":
            parts.append(struct.pack("<" + field["code"], float(value)))
        else:
            parts.append(struct.pack("<" + field["code"], int(value)))
    return b"".join(parts)


def _decode_side(side: dict, payload: bytes, name: str):
    """Decode one payload. A payload-level `repeat` means the whole field block
    repeats, so the result is a list of records rather than one mapping."""
    buf = bytes(payload)
    if side.get("repeat") is None:
        out, _ = _decode(side["fields"], buf, 0, name)
        return out
    records, offset = [], 0
    limit = side["repeat"] if isinstance(side["repeat"], int) else None
    while offset < len(buf) and (limit is None or len(records) < limit):
        record, new_offset = _decode(side["fields"], buf, offset, name)
        if new_offset == offset:
            break
        records.append(record)
        offset = new_offset
    return records


def _encode_side(side: dict, values, name: str) -> bytes:
    if side.get("repeat") is None:
        return _encode(side["fields"], values, name)
    if isinstance(values, Mapping):
        values = [values]
    return b"".join(_encode(side["fields"], record, name) for record in values)


def _side(entry: dict, side: str) -> dict:
    spec = entry.get(side)
    if spec is None and entry.get("variants"):
        raise MSPCodecError(
            f"{entry['name']} is length-dispatched; select a variant explicitly "
            f"from {list(entry['variants'])}")
    return spec


class MSPCodec:
    """Same surface as the old runtime-interpreting codec."""

    def __init__(self, messages: Mapping[str, Any] | None = None):
        self.messages = messages or MESSAGES

    def _values(self, side: dict, values):
        if side.get("repeat") is not None and not isinstance(values, Mapping):
            return list(values)
        if isinstance(values, Mapping):
            return values
        names = [f["name"] for f in side["fields"]]
        return dict(zip(names, list(values)))

    def pack_request(self, code, values: Iterable[Any] | Mapping[str, Any] = ()) -> bytes:
        entry = _spec(code)
        side = _side(entry, "request")
        if side is None:
            return b""
        return _encode_side(side, self._values(side, values), entry["name"])

    def pack_reply(self, code, values: Iterable[Any] | Mapping[str, Any] = ()) -> bytes:
        entry = _spec(code)
        side = _side(entry, "reply")
        if side is None:
            return b""
        return _encode_side(side, self._values(side, values), entry["name"])

    def unpack_reply(self, code, payload: bytes) -> Any:
        entry = _spec(code)
        side = _side(entry, "reply")
        if side is None:
            return {}
        return _decode_side(side, payload, entry["name"])

    def unpack_request(self, code, payload: bytes) -> dict:
        entry = _spec(code)
        side = _side(entry, "request")
        if side is None:
            return {}
        return _decode_side(side, payload, entry["name"])


# Re-exported so existing imports keep working. Generated from the schema
# rather than rebuilt by parsing JSON at import time.
from .generated.messages import InavMSP  # noqa: E402,F401
