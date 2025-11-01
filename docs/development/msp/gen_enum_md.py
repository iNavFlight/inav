#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
enumdoc.py — Generate Markdown documentation from C enums (no expression eval).

Rules:
- One Value column only.
  * If explicit assignment is a plain int literal (dec/hex/bin/oct) -> show that number.
  * If explicit assignment is anything else -> show the raw expression text.
  * If no assignment -> auto-increment.
- If auto-increment occurs inside an active preprocessor condition, wrap the number
  in parentheses to indicate conditional numbering: e.g., 3, 4, #ifdef, (5), (6).
- Tracks nested #if/#ifdef/#ifndef/#elif/#else/#endif and shows Condition text.
- Handles multiline enumerators (split at the first top-level comma).
"""

import sys
import re
from pathlib import Path
from typing import List, Optional

# ---------- Helpers ----------

BLOCK_COMMENT_RE = re.compile(r'/\*.*?\*/', re.DOTALL)

def strip_comments(s: str) -> str:
    s = BLOCK_COMMENT_RE.sub('', s)
    s = re.sub(r'//.*', '', s)
    return s

def find_top_level_comma(s: str) -> int:
    depth = 0
    for i, ch in enumerate(s):
        if ch == '(':
            depth += 1
        elif ch == ')':
            depth = max(0, depth - 1)
        elif ch == ',' and depth == 0:
            return i
    return -1

def is_plain_int_literal(expr: str) -> Optional[int]:
    """
    Return int value if expr is a plain integer literal (dec/hex/bin/oct),
    otherwise None. Whitespace ok; no unary ops/casts/suffixes.
    """
    t = expr.strip()
    if not t:
        return None
    if re.fullmatch(r'0[xX][0-9A-Fa-f]+', t) or \
       re.fullmatch(r'0[bB][01]+', t) or \
       re.fullmatch(r'0[0-7]*', t) or \
       re.fullmatch(r'[1-9][0-9]*', t) or \
       t == '0':
        try:
            return int(t, 0)
        except Exception:
            return None
    return None

# ---------- Parsing regexes ----------

RE_ENUM_START   = re.compile(r'^\s*typedef\s+enum\s*\{')
RE_ENUM_END     = re.compile(r'^\s*\}\s*([A-Za-z_]\w*)\s*;')
RE_LINE_COMMENT = re.compile(r'^\s*//\s*(.+?)\s*$')

RE_IFDEF   = re.compile(r'^\s*#\s*ifdef\s+(\w+)')
RE_IFNDEF  = re.compile(r'^\s*#\s*ifndef\s+(\w+)')
RE_IF      = re.compile(r'^\s*#\s*if\s+(.+)$')
RE_ELIF    = re.compile(r'^\s*#\s*elif\s+(.+)$')
RE_ELSE    = re.compile(r'^\s*#\s*else\s*$')
RE_ENDIF   = re.compile(r'^\s*#\s*endif\b')

def normalize_condition_text(text: str) -> str:
    t = text.strip()
    t = re.sub(r'\bdefined\s*\(\s*(\w+)\s*\)', r'\1', t)
    t = re.sub(r'\s+', ' ', t)
    return t

class ConditionStack:
    def __init__(self):
        self.stack: List[str] = []
    def push_ifdef(self, sym: str):  self.stack.append(sym)
    def push_ifndef(self, sym: str): self.stack.append(f'!{sym}')
    def push_if(self, expr: str):    self.stack.append(normalize_condition_text(expr))
    def elif_(self, expr: str):
        if self.stack: self.stack.pop()
        self.stack.append(normalize_condition_text(expr))
    def else_(self):
        if not self.stack: return
        top = self.stack.pop()
        if top.startswith('!'): self.stack.append(top[1:])
        elif top and all(ch.isalnum() or ch == '_' for ch in top): self.stack.append(f'!{top}')
        else: self.stack.append(f'NOT({top})')
    def endif(self):
        if self.stack: self.stack.pop()
    def current(self) -> str:
        return " AND ".join(self.stack) if self.stack else ""
    def has_active(self) -> bool:
        return bool(self.stack)

# ---------- Model ----------

class EnumItem:
    def __init__(self, name: str, value_display: str, cond: str):
        self.name = name
        self.value_display = value_display  # number, (number), or raw expr string
        self.cond = cond

class EnumDef:
    def __init__(self, name: str, source_note: str):
        self.name = name
        self.source_note = source_note
        self.items: List[EnumItem] = []

# ---------- Core parsing ----------

def parse_files(paths: List[Path]) -> List[EnumDef]:
    enums: List[EnumDef] = []
    outer_cond = ConditionStack()

    for path in paths:
        lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()
        i = 0
        recent_comment: Optional[str] = None

        while i < len(lines):
            line = lines[i]

            # Track outer preproc
            if m := RE_IFDEF.match(line):   outer_cond.push_ifdef(m.group(1)); i += 1; continue
            if m := RE_IFNDEF.match(line):  outer_cond.push_ifndef(m.group(1)); i += 1; continue
            if m := RE_IF.match(line):      outer_cond.push_if(m.group(1)); i += 1; continue
            if m := RE_ELIF.match(line):    outer_cond.elif_(m.group(1)); i += 1; continue
            if RE_ELSE.match(line):         outer_cond.else_(); i += 1; continue
            if RE_ENDIF.match(line):        outer_cond.endif(); i += 1; continue

            # Source comment directly above typedef
            mcom = RE_LINE_COMMENT.match(line)
            if mcom:
                recent_comment = mcom.group(1)

            if RE_ENUM_START.match(line):
                source_note = recent_comment or str(path)
                recent_comment = None

                body_lines: List[str] = []
                i += 1
                local_i = i
                while local_i < len(lines):
                    ln = lines[local_i]
                    if RE_ENUM_END.match(ln):
                        enum_name = RE_ENUM_END.match(ln).group(1)
                        enum = EnumDef(enum_name, source_note)

                        # second pass: parse enumerators
                        inner = ConditionStack()
                        current_numeric: Optional[int] = -1  # known numeric head; None means unknown

                        idx = 0
                        while idx < len(body_lines):
                            bl = body_lines[idx]

                            # inner preproc
                            if m := RE_IFDEF.match(bl):   inner.push_ifdef(m.group(1)); idx += 1; continue
                            if m := RE_IFNDEF.match(bl):  inner.push_ifndef(m.group(1)); idx += 1; continue
                            if m := RE_IF.match(bl):      inner.push_if(m.group(1)); idx += 1; continue
                            if m := RE_ELIF.match(bl):    inner.elif_(m.group(1)); idx += 1; continue
                            if RE_ELSE.match(bl):         inner.else_(); idx += 1; continue
                            if RE_ENDIF.match(bl):        inner.endif(); idx += 1; continue

                            # accumulate one item across lines
                            buf = [bl]
                            while True:
                                combined = strip_comments(" ".join(buf)).strip()
                                if not combined:
                                    break
                                comma_pos = find_top_level_comma(combined)
                                if comma_pos != -1:
                                    item_text = combined[:comma_pos].strip()
                                    break
                                if idx + 1 >= len(body_lines):
                                    item_text = combined
                                    break
                                nxt = body_lines[idx + 1]
                                if RE_ENUM_END.match(nxt):
                                    item_text = combined
                                    break
                                idx += 1
                                buf.append(body_lines[idx])

                            if not combined:
                                idx += 1
                                continue

                            # NAME or NAME = expr
                            mitem = re.match(r'^\s*([A-Za-z_]\w*)\s*(?:=\s*(.*))?$', item_text)
                            if not mitem:
                                idx += 1
                                continue

                            name = mitem.group(1)
                            expr = (mitem.group(2) or "").strip()

                            # active condition text
                            cond_parts = [p for p in (outer_cond.current(), inner.current()) if p]
                            cond_text = " AND ".join(cond_parts)

                            # determine display value
                            if expr:
                                lit = is_plain_int_literal(expr)
                                if lit is not None:
                                    # explicit numeric literal
                                    value_display = str(lit)
                                    current_numeric = lit
                                else:
                                    # show raw expression; numeric chain becomes unknown
                                    value_display = expr
                                    current_numeric = None
                            else:
                                # auto-increment if we know a numeric head; else unknown
                                if current_numeric is None:
                                    value_display = ""
                                else:
                                    current_numeric += 1
                                    if inner.has_active():
                                        value_display = f"({current_numeric})"
                                    else:
                                        value_display = str(current_numeric)

                            enum.items.append(EnumItem(name=name, value_display=value_display, cond=cond_text))
                            idx += 1

                        enums.append(enum)
                        i = local_i + 1
                        break
                    else:
                        body_lines.append(lines[local_i])
                        local_i += 1
                else:
                    i = local_i
                    continue
            else:
                i += 1

    return enums

# ---------- Markdown rendering ----------

def render_markdown(enums: List[EnumDef]) -> str:
    out = []
    out.append("# Enumerations\n")
    out.append("**Auto-generated reference for MSP, refer to source for development, not this file, due to variations with #ifdefs which needs verification.**\n")
    out.append("## Table of contents\n")
    for e in sorted(enums, key=lambda x: x.name.lower()):
        out.append(f"- [{e.name}](#enum-{e.name.lower()})")
    out.append("")
    for e in sorted(enums, key=lambda x: x.name.lower()):
        out.append("---")
        out.append(f"## <a id=\"enum-{e.name.lower()}\"></a>`{e.name}`\n")
        if e.source_note:
            out.append(f"> Source: {e.source_note}\n")
        out.append("| Enumerator | Value | Condition |")
        out.append("|---|---:|---|")
        for it in e.items:
            name_md = f"`{it.name}`"
            val = it.value_display
            cond = it.cond
            out.append(f"| {name_md} | {val} | {cond} |")
        out.append("")
    return "\n".join(out)

# ---------- Main ----------

def main() -> int:
    path = Path("all_enums.h")
    if not path.exists():
        print(f"Error: {path} not found", file=sys.stderr)
        return 1
    enums = parse_files([path])
    md = render_markdown(enums)
    Path("inav_enums_ref.md").write_text(md, encoding="utf-8")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
