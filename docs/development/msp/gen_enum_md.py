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
- Mutually-exclusive branches (#ifdef X / #ifndef X siblings, or #else/#elif within
  one #if family) restart numbering from the branch base, because only one branch's
  members exist in any given build.
- Tracks nested #if/#ifdef/#ifndef/#elif/#else/#endif and shows Condition text.
- Handles multiline enumerators (split at the first top-level comma).
"""

import sys
import re
from pathlib import Path
from typing import List, Optional
import json
import argparse

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

RE_ENUM_START   = re.compile(r'^\s*typedef\s+enum(?:\s+[A-Za-z_]\w*)?\s*\{')
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
    """Tracks preprocessor conditionals and the auto-increment base each branch
    started from, so mutually-exclusive branches (#ifdef X / #ifndef X siblings,
    or #else / #elif within one family) restart numbering from the shared base
    instead of continuing through the other branch's members.
    """
    def __init__(self):
        # Open frames: {'text', 'base', 'sym', 'polarity'}
        self.stack: List[dict] = []
        # Most recently closed frame per nesting level, for sibling detection
        self.closed: dict = {}

    def push_ifdef(self, sym: str, base: Optional[int] = None) -> Optional[int]:
        return self._push({'text': sym, 'base': base, 'sym': sym, 'polarity': True})

    def push_ifndef(self, sym: str, base: Optional[int] = None) -> Optional[int]:
        return self._push({'text': f'!{sym}', 'base': base, 'sym': sym, 'polarity': False})

    def push_if(self, expr: str, base: Optional[int] = None) -> Optional[int]:
        return self._push({'text': normalize_condition_text(expr), 'base': base, 'sym': None, 'polarity': None})

    def _push(self, frame: dict) -> Optional[int]:
        prev = self.closed.get(len(self.stack))
        if prev and frame['sym'] is not None and prev['sym'] == frame['sym'] \
                and prev['polarity'] != frame['polarity']:
            # mutually-exclusive sibling: restart numbering from the sibling's base
            frame['base'] = prev['base']
        self.stack.append(frame)
        return frame['base']

    def elif_(self, expr: str) -> Optional[int]:
        if not self.stack:
            return None
        base = self.stack[-1]['base']
        self.stack[-1] = {'text': normalize_condition_text(expr), 'base': base, 'sym': None, 'polarity': None}
        return base

    def else_(self) -> Optional[int]:
        if not self.stack:
            return None
        base = self.stack[-1]['base']
        text = self.stack[-1]['text']
        if text.startswith('!'):
            text = text[1:]
        elif text and all(ch.isalnum() or ch == '_' for ch in text):
            text = f'!{text}'
        else:
            text = f'NOT({text})'
        self.stack[-1] = {'text': text, 'base': base, 'sym': None, 'polarity': None}
        return base

    def endif(self):
        if not self.stack:
            return
        top = self.stack.pop()
        if top['sym'] is not None:
            self.closed[len(self.stack)] = top
        else:
            self.closed.pop(len(self.stack), None)

    def note_item(self):
        """An enumerator was parsed at this nesting level, so a previously
        closed sibling here is no longer immediately preceding and must not
        be treated as the alternate of a later same-symbol block."""
        self.closed.pop(len(self.stack), None)

    def current(self) -> str:
        return " AND ".join(f['text'] for f in self.stack) if self.stack else ""

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

                            # inner preproc — reset counter when entering an
                            # exclusive alternate branch (returns new base)
                            if m := RE_IFDEF.match(bl):
                                new_base = inner.push_ifdef(m.group(1), current_numeric)
                                if new_base is not None: current_numeric = new_base
                                idx += 1; continue
                            if m := RE_IFNDEF.match(bl):
                                new_base = inner.push_ifndef(m.group(1), current_numeric)
                                if new_base is not None: current_numeric = new_base
                                idx += 1; continue
                            if m := RE_IF.match(bl):
                                new_base = inner.push_if(m.group(1), current_numeric)
                                if new_base is not None: current_numeric = new_base
                                idx += 1; continue
                            if m := RE_ELIF.match(bl):
                                new_base = inner.elif_(m.group(1))
                                if new_base is not None: current_numeric = new_base
                                idx += 1; continue
                            if RE_ELSE.match(bl):
                                new_base = inner.else_()
                                if new_base is not None: current_numeric = new_base
                                idx += 1; continue
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
                            inner.note_item()
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

def render_markdown(enums: List[EnumDef], build: dict) -> str:
    jsonfile = {}
    out = []
    out.append("# Enumerations\n")
    out.append("**Auto-generated reference for MSP, refer to source for development, not this file, due to variations with #ifdefs which needs verification.**\n")
    out.append("## Table of contents\n")
    for e in sorted(enums, key=lambda x: x.name.lower()):
        out.append(f"- [{e.name}](#enum-{e.name.lower()})")
    out.append("")
    for e in sorted(enums, key=lambda x: x.name.lower()):
        jsonfile[e.name] = {}
        out.append("---")
        out.append(f"## <a id=\"enum-{e.name.lower()}\"></a>`{e.name}`\n")
        if e.source_note:
            out.append(f"> Source: {e.source_note}\n")
            jsonfile[e.name]['_source'] = e.source_note
        out.append("| Enumerator | Value | Condition |")
        out.append("|---|---:|---|")
        for it in e.items:
            name_md = f"`{it.name}`"
            val = it.value_display
            cond = it.cond
            out.append(f"| {name_md} | {val} | {cond} |")
            jsonfile[e.name][name_md.strip('`')] = [val, cond] if len(cond)>0 else val
        # normalize source to a stable inav/src/... path
        if '_source' in jsonfile[e.name]:
            jsonfile[e.name]['_source'] = jsonfile[e.name]['_source'].replace('../../../src', 'inav/src')
        out.append("")
    wrapped = {
        "build": build,
        "enums": jsonfile,
    }
    Path("inav_enums.json").write_text(json.dumps(wrapped, indent=4), encoding="utf-8")
    return "\n".join(out)

# ---------- Main ----------

def main() -> int:
    parser = argparse.ArgumentParser(description="Generate enum markdown/json from all_enums.h")
    parser.add_argument("--fc-version-major", required=True, type=int)
    parser.add_argument("--fc-version-minor", required=True, type=int)
    parser.add_argument("--fc-version-patch-level", required=True, type=int)
    args = parser.parse_args()

    path = Path("all_enums.h")
    if not path.exists():
        print(f"Error: {path} not found", file=sys.stderr)
        return 1
    enums = parse_files([path])
    md = render_markdown(
        enums,
        {
            "fc_version": {
                "major": args.fc_version_major,
                "minor": args.fc_version_minor,
                "patch": args.fc_version_patch_level,
            },
        },
    )
    Path("inav_enums_ref.md").write_text(md, encoding="utf-8")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
