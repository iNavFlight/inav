#!/usr/bin/env python3
"""Harvest INAV's C enums into schema/enums.yaml.

INAV source is the origin of every enum: this walks src/main, extracts each
`enum`/`typedef enum`, resolves member values to integers, and writes them to
schema/enums.yaml. The MSP schema (msp_v2.yaml) references enums by name; their
definitions live here, regenerated per firmware version — never hand-authored.

Member names are the real C names, so no prefixing or de-duplication is needed.
Values are resolved (auto-increment, literals, bit-shifts, intra/inter-enum
aliases, char literals). An enum whose values depend on symbols not visible here
(a #define, another module's constant) is reported and skipped, never guessed.

Usage:
    python3 harvest_enums.py [--src DIR] [--out FILE] [--fc-version X.Y.Z]
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from typing import List, Optional

HERE = Path(__file__).resolve().parent
MSP_ROOT = HERE.parent            # inav/msp
REPO_ROOT = MSP_ROOT.parent       # inav

# Subdirectories under src/main that declare wire-relevant enums.
SUBDIRS = ["common", "blackbox", "navigation", "sensors", "programming",
           "rx", "telemetry", "io", "flight", "fc", "drivers"]


# --- collection: pull raw enum blocks out of the C sources -------------------

def strip_block_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"//.*", "", text)
    return text


def extract_enum_blocks(label: str, text: str) -> List[str]:
    """Return `// label` + enum-block strings for every enum in one file."""
    src = strip_block_comments(text)
    out: List[str] = []

    # typedef enum { ... } Alias;
    i = 0
    while True:
        m = re.search(r"\btypedef\s+enum\b", src[i:])
        if not m:
            break
        start = i + m.start()
        lb = src.find("{", i + m.end())
        if lb == -1:
            break
        depth, k = 0, lb
        while k < len(src):
            if src[k] == "{":
                depth += 1
            elif src[k] == "}":
                depth -= 1
                if depth == 0:
                    semi = src.find(";", k)
                    if semi == -1:
                        break
                    if re.findall(r"\b([A-Za-z_]\w*)\b", src[k + 1:semi]):
                        out += [f"// {label}\n", src[start:semi + 1].strip() + "\n\n"]
                    i = semi + 1
                    break
            k += 1
        else:
            break

    # enum Tag { ... };  -> emit with a synthesized typedef so the parser names it
    i = 0
    while True:
        m = re.search(r"\benum\s+([A-Za-z_]\w*)\s*{", src[i:])
        if not m:
            break
        start = i + m.start()
        if src[:start].rstrip().endswith("typedef"):
            i += m.end()
            continue
        tag = m.group(1)
        lb = src.find("{", i + m.end() - 1)
        if lb == -1:
            break
        depth, k = 0, lb
        while k < len(src):
            if src[k] == "{":
                depth += 1
            elif src[k] == "}":
                depth -= 1
                if depth == 0:
                    semi = src.find(";", k)
                    if semi == -1:
                        break
                    out += [f"// {label}\n", src[start:semi + 1].strip() + "\n",
                            f"typedef enum {tag} {tag};\n\n"]
                    i = semi + 1
                    break
            k += 1
        else:
            break

    return out


def collect(src_main: Path) -> List[str]:
    blocks: List[str] = []
    for sd in SUBDIRS:
        root = src_main / sd
        if not root.is_dir():
            continue
        for fn in sorted(root.rglob("*")):
            if fn.suffix not in (".c", ".h"):
                continue
            try:
                label = "inav/" + str(fn.relative_to(REPO_ROOT))
            except ValueError:
                label = str(fn)
            blocks += extract_enum_blocks(label, fn.read_text(errors="ignore"))
    return blocks


# --- parsing: enumerators, preprocessor conditions, auto-increment ------------
# Enum-body parser (no value eval here;
# resolution is a separate, tested pass below).

RE_ENUM_START = re.compile(r"^\s*typedef\s+enum(?:\s+[A-Za-z_]\w*)?\s*\{")
RE_ENUM_END = re.compile(r"^\s*\}\s*([A-Za-z_]\w*)\s*;")
RE_ENUM_HEAD = re.compile(r"^(\s*(?:typedef\s+)?enum(?:\s+[A-Za-z_]\w*)?)\s*$")
RE_OPEN_BRACE = re.compile(r"^\s*\{\s*$")
RE_LINE_COMMENT = re.compile(r"^\s*//\s*(.+?)\s*$")
RE_IFDEF = re.compile(r"^\s*#\s*ifdef\s+(\w+)")
RE_IFNDEF = re.compile(r"^\s*#\s*ifndef\s+(\w+)")
RE_IF = re.compile(r"^\s*#\s*if\s+(.+)$")
RE_ELIF = re.compile(r"^\s*#\s*elif\s+(.+)$")
RE_ELSE = re.compile(r"^\s*#\s*else\s*$")
RE_ENDIF = re.compile(r"^\s*#\s*endif\b")


def strip_comments(s: str) -> str:
    s = re.sub(r"/\*.*?\*/", "", s, flags=re.DOTALL)
    return re.sub(r"//.*", "", s)


def find_top_level_comma(s: str) -> int:
    depth = 0
    for i, ch in enumerate(s):
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth = max(0, depth - 1)
        elif ch == "," and depth == 0:
            return i
    return -1


def is_plain_int_literal(expr: str) -> Optional[int]:
    t = expr.strip()
    if re.fullmatch(r"0[xX][0-9A-Fa-f]+", t) or re.fullmatch(r"0[bB][01]+", t) or \
       re.fullmatch(r"0[0-7]*", t) or re.fullmatch(r"[1-9][0-9]*", t) or t == "0":
        try:
            return int(t, 0)
        except ValueError:
            return None
    return None


def normalize_condition_text(text: str) -> str:
    t = re.sub(r"\bdefined\s*\(\s*(\w+)\s*\)", r"\1", text.strip())
    return re.sub(r"\s+", " ", t)


def join_allman_braces(lines: List[str]) -> List[str]:
    out: List[str] = []
    i = 0
    while i < len(lines):
        head = RE_ENUM_HEAD.match(lines[i])
        if head and i + 1 < len(lines) and RE_OPEN_BRACE.match(lines[i + 1]):
            out.append(head.group(1) + " {")
            i += 2
            continue
        out.append(lines[i])
        i += 1
    return out


class ConditionStack:
    def __init__(self):
        self.stack: List[dict] = []
        self.closed: dict = {}

    def _push(self, frame):
        prev = self.closed.get(len(self.stack))
        if prev and frame["sym"] is not None and prev["sym"] == frame["sym"] \
                and prev["polarity"] != frame["polarity"]:
            frame["base"] = prev["base"]
        self.stack.append(frame)
        return frame["base"]

    def push_ifdef(self, sym, base=None):
        return self._push({"text": sym, "base": base, "sym": sym, "polarity": True})

    def push_ifndef(self, sym, base=None):
        return self._push({"text": f"!{sym}", "base": base, "sym": sym, "polarity": False})

    def push_if(self, expr, base=None):
        return self._push({"text": normalize_condition_text(expr), "base": base,
                           "sym": None, "polarity": None})

    def elif_(self, expr):
        if not self.stack:
            return None
        base = self.stack[-1]["base"]
        self.stack[-1] = {"text": normalize_condition_text(expr), "base": base,
                          "sym": None, "polarity": None}
        return base

    def else_(self):
        if not self.stack:
            return None
        base = self.stack[-1]["base"]
        text = self.stack[-1]["text"]
        if text.startswith("!"):
            text = text[1:]
        elif text and all(c.isalnum() or c == "_" for c in text):
            text = f"!{text}"
        else:
            text = f"NOT({text})"
        self.stack[-1] = {"text": text, "base": base, "sym": None, "polarity": None}
        return base

    def endif(self):
        if not self.stack:
            return
        top = self.stack.pop()
        if top["sym"] is not None:
            self.closed[len(self.stack)] = top
        else:
            self.closed.pop(len(self.stack), None)

    def note_item(self):
        self.closed.pop(len(self.stack), None)

    def current(self) -> str:
        return " AND ".join(f["text"] for f in self.stack) if self.stack else ""

    def has_active(self) -> bool:
        return bool(self.stack)


class EnumItem:
    def __init__(self, name, value_display, cond):
        self.name = name
        self.value_display = value_display
        self.cond = cond


class EnumDef:
    def __init__(self, name, source_note):
        self.name = name
        self.source_note = source_note
        self.items: List[EnumItem] = []


def parse_lines(lines: List[str]) -> List[EnumDef]:
    lines = join_allman_braces(lines)
    enums: List[EnumDef] = []
    outer = ConditionStack()
    i = 0
    recent_comment: Optional[str] = None
    while i < len(lines):
        line = lines[i]
        if m := RE_IFDEF.match(line):
            outer.push_ifdef(m.group(1)); i += 1; continue
        if m := RE_IFNDEF.match(line):
            outer.push_ifndef(m.group(1)); i += 1; continue
        if m := RE_IF.match(line):
            outer.push_if(m.group(1)); i += 1; continue
        if m := RE_ELIF.match(line):
            outer.elif_(m.group(1)); i += 1; continue
        if RE_ELSE.match(line):
            outer.else_(); i += 1; continue
        if RE_ENDIF.match(line):
            outer.endif(); i += 1; continue
        if mcom := RE_LINE_COMMENT.match(line):
            recent_comment = mcom.group(1)
        if RE_ENUM_START.match(line):
            source_note = recent_comment or ""
            recent_comment = None
            body: List[str] = []
            i += 1
            local = i
            while local < len(lines):
                ln = lines[local]
                if end := RE_ENUM_END.match(ln):
                    enum = EnumDef(end.group(1), source_note)
                    inner = ConditionStack()
                    cur: Optional[int] = -1
                    idx = 0
                    while idx < len(body):
                        bl = body[idx]
                        if m := RE_IFDEF.match(bl):
                            nb = inner.push_ifdef(m.group(1), cur)
                            if nb is not None: cur = nb
                            idx += 1; continue
                        if m := RE_IFNDEF.match(bl):
                            nb = inner.push_ifndef(m.group(1), cur)
                            if nb is not None: cur = nb
                            idx += 1; continue
                        if m := RE_IF.match(bl):
                            nb = inner.push_if(m.group(1), cur)
                            if nb is not None: cur = nb
                            idx += 1; continue
                        if m := RE_ELIF.match(bl):
                            nb = inner.elif_(m.group(1))
                            if nb is not None: cur = nb
                            idx += 1; continue
                        if RE_ELSE.match(bl):
                            nb = inner.else_()
                            if nb is not None: cur = nb
                            idx += 1; continue
                        if RE_ENDIF.match(bl):
                            inner.endif(); idx += 1; continue
                        buf = [bl]
                        while True:
                            combined = strip_comments(" ".join(buf)).strip()
                            if not combined:
                                break
                            cp = find_top_level_comma(combined)
                            if cp != -1:
                                item_text = combined[:cp].strip(); break
                            if idx + 1 >= len(body) or RE_ENUM_END.match(body[idx + 1]):
                                item_text = combined; break
                            idx += 1; buf.append(body[idx])
                        if not combined:
                            idx += 1; continue
                        mitem = re.match(r"^\s*([A-Za-z_]\w*)\s*(?:=\s*(.*))?$", item_text)
                        if not mitem:
                            idx += 1; continue
                        name = mitem.group(1)
                        expr = (mitem.group(2) or "").strip()
                        cond = " AND ".join(p for p in (outer.current(), inner.current()) if p)
                        if expr:
                            lit = is_plain_int_literal(expr)
                            if lit is not None:
                                vd = str(lit); cur = lit
                            else:
                                vd = expr; cur = None
                        else:
                            if cur is None:
                                vd = ""
                            else:
                                cur += 1
                                vd = f"({cur})" if inner.has_active() else str(cur)
                        enum.items.append(EnumItem(name, vd, cond))
                        inner.note_item()
                        idx += 1
                    enums.append(enum)
                    i = local + 1
                    break
                body.append(lines[local]); local += 1
            else:
                i = local
                continue
        else:
            i += 1
    return enums


# --- resolution: enumerator display -> integer -------------------------------

def clean_expr(s: str) -> str:
    s = re.sub(r"(0[xX][0-9A-Fa-f]+|\d+)[uUlL]+", r"\1", s)   # C int suffixes
    s = re.sub(r"'\\0'", "0", s)
    s = re.sub(r"'\\n'", str(ord("\n")), s)
    s = re.sub(r"'([^\\'])'", lambda m: str(ord(m.group(1))), s)
    return s


def resolve(enums: List[EnumDef]):
    """-> (by_name {enum: {_source, members{member:(value,cond)}}}, failed set,
    unresolved list). Later definitions win on duplicates, matching the C
    preprocessor. A global symbol table lets one enum reference another's
    members. An enum with ANY unresolvable member is skipped whole (recorded in
    `failed` and reported), never emitted partial."""
    by_name: dict = {}
    gsyms: dict = {}
    failed: set = set()
    unresolved: List[str] = []
    for e in enums:
        members: dict = {}
        last = -1
        for it in e.items:
            vd = it.value_display.strip()
            try:
                if vd == "":
                    last += 1
                    val = last
                else:
                    core = vd.strip("()")
                    if re.fullmatch(r"-?(0[xX][0-9A-Fa-f]+|0[bB][01]+|\d+)", core):
                        val = int(core, 0)
                    else:
                        ns = dict(gsyms); ns.update({k: v[0] for k, v in members.items()})
                        val = int(eval(clean_expr(vd), {"__builtins__": {}}, ns))
                    last = val
            except Exception as ex:
                unresolved.append(f"{e.name}.{it.name} = {vd!r} ({ex})")
                failed.add(e.name)
                continue
            members[it.name] = (val, it.cond)
            gsyms[it.name] = val
        rec = by_name.setdefault(e.name, {"_source": e.source_note, "members": {}})
        if e.source_note:
            rec["_source"] = e.source_note
        rec["members"].update(members)
    return by_name, failed, unresolved


# --- emit --------------------------------------------------------------------

def infer_storage(values: list) -> str:
    """Smallest C integer type that holds every member value. The wire width of
    a field is declared on the field; this is the enum type's own width, used by
    the model generators."""
    signed = any(v < 0 for v in values)
    hi = max((v for v in values), default=0)
    lo = min((v for v in values), default=0)
    if signed:
        for t, bits in (("int8", 8), ("int16", 16), ("int32", 32), ("int64", 64)):
            if -(1 << (bits - 1)) <= lo and hi <= (1 << (bits - 1)) - 1:
                return t
        return "int64"
    for t, bits in (("uint8", 8), ("uint16", 16), ("uint32", 32), ("uint64", 64)):
        if hi < (1 << bits):
            return t
    return "uint64"


def emit_yaml(by_name: dict, failed: set, fc_version: str) -> str:
    out = [f"# Generated from INAV C source by msp/generator/harvest_enums.py (INAV {fc_version}). Do not edit by hand.",
           "version: 1", "package: inav.enums", "", "enums:"]
    first = True
    for name in sorted(by_name):
        rec = by_name[name]
        if name in failed or not rec["members"]:
            continue
        if not first:
            out.append("")      # blank line between enum definitions
        first = False
        out.append(f"  {name}:")
        if rec["_source"]:
            out.append(f"    description: From {rec['_source']}")
        out.append(f"    storage: {infer_storage([v for v, _ in rec['members'].values()])}")
        out.append("    values:")
        for member, (val, cond) in rec["members"].items():
            if cond:
                # one line, C style; see generator/enum_syntax.py
                out.append(f'      - if "{cond}": {member} = {val}')
            else:
                out.append(f"      - {member} = {val}")
    return "\n".join(out) + "\n"


def read_fc_version() -> str:
    cml = (REPO_ROOT / "CMakeLists.txt").read_text()
    m = re.search(r"project\(\s*INAV\s+VERSION\s+(\d+)\.(\d+)\.(\d+)", cml)
    return ".".join(m.groups()) if m else "0.0.0"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--src", type=Path, default=REPO_ROOT / "src/main")
    ap.add_argument("--out", type=Path, default=MSP_ROOT / "schema" / "enums.yaml")
    ap.add_argument("--fc-version", default=None)
    args = ap.parse_args()

    blocks = collect(args.src)
    enums = parse_lines("".join(blocks).splitlines())
    by_name, failed, unresolved = resolve(enums)
    fc = args.fc_version or read_fc_version()
    args.out.write_text(emit_yaml(by_name, failed, fc))

    emitted = sum(1 for n, r in by_name.items() if r["members"] and n not in failed)
    print(f"wrote {args.out}  ({emitted} enums, fc {fc})")
    if failed:
        print(f"skipped {len(failed)} enum(s) with unresolvable members "
              f"(values depend on #defines / external constants not visible to the harvester):",
              file=sys.stderr)
        for u in unresolved:
            print(f"  {u}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
