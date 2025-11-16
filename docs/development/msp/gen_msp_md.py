#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Generate Markdown documentation from an MSP message definitions JSON.

Strict + Index:
- STRICT: If a code exists in one (MSPCodes vs JSON) but not the other, crash with details.
- Index items link to headings via GitHub-style auto-anchors.
- Tight layout; identical Request/Reply tables; skip complex=true with a stub.
- Default input: msp_messages.json ; default output: MSP_Doc.md
"""

import sys
import json
import re
import unicodedata
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple, Type

import enum

def build_msp_codes_enum(defs: Dict[str, Any]) -> Type[enum.IntEnum]:
    members: Dict[str, int] = {}
    for name, body in defs.items():
        try:
            code = int(body.get("code", -1))
        except (TypeError, ValueError):
            continue
        members[name] = code
    return enum.IntEnum("MSPCodes", members)


# ---- C type size helpers ----------------------------------------------------

BASE_SIZES = {
    "uint8_t": 1, "int8_t": 1, "char": 1,
    "uint16_t": 2, "int16_t": 2,
    "uint32_t": 4, "int32_t": 4,
    "uint64_t": 8, "int64_t": 8,
    "float": 4, "double": 8,
}

array_brackets_re = re.compile(r"^(?P<base>[A-Za-z_0-9]+)\[(?P<size>.*)\]$")

def parse_ctype(ctype: str) -> Tuple[str, Optional[str]]:
    m = array_brackets_re.match(ctype.strip())
    if not m:
        return ctype.strip(), None
    return m.group("base").strip(), m.group("size").strip()

def format_ctype(field: Dict[str, Any]) -> str:
    raw = (field.get("ctype") or "").strip()
    if not raw:
        return "-"

    base, bracket = parse_ctype(raw)
    has_array_meta = bool(field.get("array", False))
    is_array = has_array_meta or (bracket is not None)
    if not is_array:
        return raw

    size_define = (field.get("array_size_define") or "").strip()
    array_size = field.get("array_size")
    size_expr = ""

    if size_define:
        size_expr = size_define
    else:
        if isinstance(array_size, int):
            if array_size > 0:
                size_expr = str(array_size)
        elif isinstance(array_size, str):
            cleaned = array_size.strip()
            if cleaned and cleaned != "0":
                size_expr = cleaned

    if not size_expr and bracket is not None:
        size_expr = bracket.strip()

    if size_expr == "0":
        size_expr = ""

    base_part = base or raw
    return f"{base_part}[{size_expr}]"

def describe_array_bytes(array_size_meta: Any, base_bytes: Optional[int], base_name: str) -> str:
    """
    Returns a printable byte-count (or symbolic string) for an array entry.
    """
    if isinstance(array_size_meta, int):
        if array_size_meta <= 0:
            return "array"
        if base_bytes is None:
            return str(array_size_meta)
        return str(array_size_meta * base_bytes)

    if isinstance(array_size_meta, str):
        expr = array_size_meta.strip()
        if not expr:
            return "array"
        if base_bytes is None or base_name == "char":
            return expr
        return f"{expr} * {base_bytes}"

    return "array"

def sizeof_entry(field: Dict[str, Any]) -> str:
    ctype = field.get("ctype", "").strip()
    base, bracket = parse_ctype(ctype)

    is_array = bool(field.get("array", False))
    array_size_meta = field.get("array_size", None)
    array_size_define = (field.get("array_size_define") or "").strip()

    if is_array or bracket is not None:
        base_for_size = base if (base and is_array) else (base or ctype)
        base_bytes = BASE_SIZES.get(base_for_size, None)

        if is_array:
            size_str = describe_array_bytes(array_size_meta, base_bytes, base_for_size)
            if array_size_define:
                if size_str in {"array", "-"}:
                    size_str = array_size_define
                else:
                    size_str = f"{size_str} ({array_size_define})"
            return size_str

        if bracket is not None:
            if bracket == "":
                return "array"
            if bracket.isdigit():
                n = int(bracket)
                return str(n * base_bytes) if base_bytes is not None else str(n)
            if base_bytes is None or base == "char":
                return bracket
            return f"{bracket} * {base_bytes}"

        return "array"

    base_bytes = BASE_SIZES.get(base, None)
    return str(base_bytes) if base_bytes is not None else "-"


# ---- Markdown rendering -----------------------------------------------------

#inav_wiki_url = "https://github.com/xznhj8129/msp_documentation/blob/master/docs/inav_enums_ref.md"
inav_wiki_url = "https://github.com/iNavFlight/inav/wiki/Enums-reference"

def units_cell(field: Dict[str, Any]) -> str:
    if "enum" in field:
        if field["enum"]=="?_e":
            return "[ENUM_NAME](LINK_TO_ENUM)"
        else:
            return f"[{field['enum']}]({inav_wiki_url}#enum-{field['enum'].lower()})"
    u = (field.get("units") or "").strip()
    return u if u else "-"

def has_fields(section: Any) -> bool:
    if not isinstance(section, dict):
        return False
    payload = section.get("payload")
    return isinstance(payload, list) and len(payload) > 0

def get_fields(section: Any) -> List[Dict[str, Any]]:
    if not isinstance(section, dict):
        return []
    payload = section.get("payload")
    return payload if isinstance(payload, list) else []

def flatten_fields_with_repeats(fields: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
    """
    Flattens one level of partially repeating payload blocks:
    Items with {"repeating": "SOME_SYMBOL", "payload": [...]} are expanded so each child
    field gets a symbolic multiplier in the size column.
    """
    out: List[Dict[str, Any]] = []
    for f in fields:
        if isinstance(f, dict) and "repeating" in f and isinstance(f.get("payload"), list):
            repeat_sym = str(f["repeating"])
            for child in f["payload"]:
                if isinstance(child, dict):
                    c = dict(child)
                    # Mark repeat multiplier for the size column
                    c["_repeat_multiplier"] = repeat_sym
                    out.append(c)
        else:
            out.append(f)
    return out


def table_with_units(fields: List[Dict[str, Any]], label: str) -> str:
    flat_fields = flatten_fields_with_repeats(fields)
    has_repeats = any(isinstance(f, dict) and f.get("_repeat_multiplier") for f in flat_fields)
    has_units = any(
        isinstance(f, dict) and (
            ((f.get("units") or "").strip()) or ("enum" in f)
        )
        for f in flat_fields
    )

    # Build dynamic header
    cols = ["Field", "C Type"]
    if has_repeats:
        cols.append("Repeats")
    cols.append("Size (Bytes)")
    if has_units:
        cols.append("Units")
    cols.append("Description")

    header = "  \n**{label}:**\n".format(label=label)
    header += "|" + "|".join(cols) + "|\n"
    header += "|" + "|".join(["---"] * len(cols)) + "|\n"

    # Rows
    rows: List[str] = []
    for f in flat_fields:
        name = f.get("name", "")
        size = sizeof_entry(f)
        if size == "0":
            size = "-"

        row_cells = [f"`{name}`", f"`{format_ctype(f)}`"]

        if has_repeats:
            repeats = f.get("_repeat_multiplier") or "-"
            row_cells.append(repeats)

        row_cells.append(size)

        if has_units:
            units = units_cell(f)
            row_cells.append(units)

        desc = (f.get("desc") or "").strip()
        row_cells.append(desc)

        rows.append("| " + " | ".join(row_cells) + " |")

    return header + "\n".join(rows) + "\n"



def render_variant(parent_name: str, variant_name: str, variant_def: Dict[str, Any]) -> str:
    """
    Renders a single variant block (subsection header, description, request/reply tables).
    """
    out: List[str] = []
    vdesc = (variant_def.get("description") or "").strip()

    # GitHub auto-anchors will work off this header text
    out.append(f"#### Variant: `{variant_name}`\n\n")
    if vdesc:
        out.append(f"**Description:** {vdesc}  \n")

    req = variant_def.get("request", None)
    rep = variant_def.get("reply", None)

    if has_fields(req):
        out.append(table_with_units(get_fields(req), "Request Payload"))
    else:
        out.append("\n**Request Payload:** **None**  \n")

    if has_fields(rep):
        out.append(table_with_units(get_fields(rep), "Reply Payload"))
    else:
        out.append("\n**Reply Payload:** **None**  \n")

    out.append("\n")
    return "".join(out)

def render_message(name: str, msg: Dict[str, Any]) -> Tuple[str, str]:
    """
    Returns (section_markdown, heading_text_for_anchor)
    """
    code = msg.get("code", 0)
    hex_str = msg.get("hex", hex(code))
    description = (msg.get("description") or "").strip()
    notes = (msg.get("notes") or "").strip()
    complex_flag = bool(msg.get("complex", False))

    heading = f'## <a id="{name.lower()}"></a>`{name} ({code} / {hex_str})`'
    out = [heading + "\n"]

    if description:
        out.append(f"**Description:** {description}  \n")

    #if complex_flag:
    #    out.append("**Special case, skipped for now**\n\n")
    #    return "".join(out), heading

    # NEW: variant-aware rendering
    variants = msg.get("variants")
    if isinstance(variants, dict) and variants:
        # For variant messages, render a compact per-variant table set
        for vname, vdef in variants.items():
            out.append(render_variant(name, vname, vdef))
    else:
        # Fallback: single request/reply like before
        req = msg.get("request", None)
        rep = msg.get("reply", None)

        if has_fields(req):
            out.append(table_with_units(get_fields(req), "Request Payload"))
        else:
            out.append("\n**Request Payload:** **None**  \n")

        if has_fields(rep):
            out.append(table_with_units(get_fields(rep), "Reply Payload"))
        else:
            out.append("\n**Reply Payload:** **None**  \n")

    if notes:
        out.append(f"\n**Notes:** {notes}\n")

    out.append("\n")
    return "".join(out), heading

# ---- Index + strict consistency --------------------------------------------

def build_maps(defs: Dict[str, Any], codes_cls: Type[enum.IntEnum]) -> Tuple[Dict[int, str], Dict[int, str]]:
    """
    Returns:
      json_by_code: {code -> message_name_from_json}
      mw_by_code:   {code -> enum_name_from codes_cls}
    Only for codes in the enforced ranges (v1 and v2).
    """
    v1_range = range(0, 255)
    v2_range = range(4096, 20001)

    # JSON: build by code (restrict to ranges)
    json_by_code: Dict[int, str] = {}
    for name, body in defs.items():
        code = int(body.get("code", -1))
        if code in v1_range or code in v2_range:
            json_by_code[code] = name

    # MSPCodes: probe the same ranges
    mw_by_code: Dict[int, str] = {}
    def try_get(code: int) -> Optional[str]:
        try:
            e = codes_cls(code)
            return e.name
        except Exception:
            return None

    for code in list(v1_range) + list(v2_range):
        ename = try_get(code)
        if ename is not None:
            mw_by_code[code] = ename

    return json_by_code, mw_by_code

def enforce_strict_match(json_by_code: Dict[int, str], mw_by_code: Dict[int, str]) -> None:
    json_codes = set(json_by_code.keys())
    mw_codes = set(mw_by_code.keys())

    only_in_json = sorted(json_codes - mw_codes)
    only_in_mw   = sorted(mw_codes - json_codes)

    if only_in_json or only_in_mw:
        lines = ["MSP code mismatch detected:"]
        if only_in_json:
            lines.append("  Present in JSON but missing in MSPCodes:")
            for c in only_in_json:
                lines.append(f"    {c}\t{json_by_code[c]}")
        if only_in_mw:
            lines.append("  Present in MSPCodes but missing in JSON:")
            for c in only_in_mw:
                lines.append(f"    {c}\t{mw_by_code[c]}")
        raise SystemExit("\n".join(lines))

def build_index(json_by_code: Dict[int, str]) -> str:
    """
    Build a compact index linking to each heading.
    """
    v1 = []
    v2 = []
    for code, name in sorted(json_by_code.items()):
        hex_str = hex(code)
        item = f"[{code} - {name}](#{name.lower()})  "
        if 0 <= code <= 255:
            v1.append(item)
        elif 4096 <= code <= 20000:
            v2.append(item)

    parts = ["## Index", "### MSPv1"]
    parts.extend(v1)
    parts.append("\n### MSPv2")
    parts.extend(v2)
    parts.append("")  # trailing newline
    return "\n".join(parts)

# ---- Orchestration ----------------------------------------------------------

def generate_markdown(defs: Dict[str, Any]) -> str:
    # Strict maps & check
    codes_enum = build_msp_codes_enum(defs)
    json_by_code, mw_by_code = build_maps(defs, codes_enum)
    enforce_strict_match(json_by_code, mw_by_code)

    # Build sections, remembering headings for slugging (already handled in index)
    items = sorted(((int(body.get("code", 0)), name, body) for name, body in defs.items()),
                   key=lambda t: t[0])

    sections = []
    for _, name, body in items:
        sec, _heading = render_message(name, body)
        sections.append(sec)

    with open("docs_v2_header.md", "r", encoding="utf-8") as f:
        header = f.read()

    with open("format.md", "r", encoding="utf-8") as f:
        fmt = f.read()

    with open("msp_messages.checksum", "r", encoding="utf-8") as f:
        chksum = f.read().split(' ')[0]
    with open("rev", "r", encoding="utf-8") as f:
        rev = f.read()

    header = header.replace('<format>',fmt)
    header = header.replace('<file_rev>',rev)
    header = header.replace('<file_hash>',chksum)

    index_md = build_index(json_by_code)
    return header + "\n" + index_md + "\n" + "".join(sections)

def main():
    in_path = Path(sys.argv[1]) if len(sys.argv) >= 2 else Path("msp_messages.json")
    out_path = Path(sys.argv[2]) if len(sys.argv) >= 3 else Path("msp_ref.md")

    with in_path.open("r", encoding="utf-8") as f:
        defs = json.load(f)

    md = generate_markdown(defs)
    out_path.write_text(md, encoding="utf-8")
    print(f"Wrote {out_path}")

if __name__ == "__main__":
    main()
