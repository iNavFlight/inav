#!/usr/bin/env python3
import argparse
import datetime
import re
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parents[1]
ALL_ENUMS_H = SCRIPT_DIR / 'all_enums.h'

SUBDIRS = [
    'common',
    'blackbox',
    'navigation',
    'sensors',
    'programming',
    'rx',
    'telemetry',
    'io',
    'flight',
    'fc',
    'drivers',
]

def strip_comments(text: str) -> str:
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.DOTALL)   # block comments
    text = re.sub(r'//.*', '', text)                         # line comments
    return text

def extract_enums(fn: str, text: str):
    src = strip_comments(text)
    out = []

    # typedef enum { ... } Alias;
    i = 0
    while True:
        m = re.search(r'\btypedef\s+enum\b', src[i:])
        if not m: break
        start = i + m.start()
        lb = src.find('{', i + m.end())
        if lb == -1: break
        depth = 0
        k = lb
        while k < len(src):
            if src[k] == '{': depth += 1
            elif src[k] == '}':
                depth -= 1
                if depth == 0:
                    semi = src.find(';', k)
                    if semi == -1: break
                    tail = src[k+1:semi]
                    alias = re.findall(r'\b([A-Za-z_]\w*)\b', tail)
                    if alias:
                        block = src[start:semi+1].strip()
                        out += [f'// {fn}\n', block + '\n\n']
                    i = semi + 1
                    break
            k += 1
        else:
            break

    # enum Tag { ... };  → also emit typedef enum Tag Tag;
    i = 0
    while True:
        m = re.search(r'\benum\s+([A-Za-z_]\w*)\s*{', src[i:])
        if not m: break
        start = i + m.start()
        # `typedef enum Tag { ... } Alias;` also matches here, but the typedef
        # pass above already emitted it. Emitting it again produces duplicate
        # sections and colliding anchors in the generated reference.
        if src[:start].rstrip().endswith('typedef'):
            i = i + m.end()
            continue
        tag = m.group(1)
        lb = src.find('{', i + m.end() - 1)
        if lb == -1: break
        depth = 0
        k = lb
        while k < len(src):
            if src[k] == '{': depth += 1
            elif src[k] == '}':
                depth -= 1
                if depth == 0:
                    semi = src.find(';', k)
                    if semi == -1: break
                    block = src[start:semi+1].strip()
                    out += [
                        f'// {fn}\n',
                        block + '\n',
                        f'typedef enum {tag} {tag};\n\n'
                    ]
                    i = semi + 1
                    break
            k += 1
        else:
            break

    return out



all_enums = []
def parse_args():
    parser = argparse.ArgumentParser(description='Collect all enums from INAV sources.')
    parser.add_argument(
        '--inav-root',
        default=str(REPO_ROOT / 'src/main'),
        help="Path to the INAV 'src/main' directory (default: %(default)s)",
    )
    return parser.parse_args()


args = parse_args()
base_dir = Path(args.inav_root).expanduser().resolve()
for sd in SUBDIRS:
    root = base_dir / sd
    if not root.is_dir():
        continue
    for fn in sorted(root.rglob('*')):
        print(fn)
        if fn.suffix in ('.c', '.h'):
            txt = fn.read_text(errors='ignore')
            # Label enums with a repo-relative path so the generated output does
            # not depend on where this script was run from.
            ret = extract_enums(fn.relative_to(REPO_ROOT), txt)
            if ret: print(fn)
            all_enums.extend(ret)

with open(ALL_ENUMS_H, 'w') as out:
    out.write(f"// Consolidated enums — generated on {datetime.datetime.now()}\n\n")
    out.writelines(all_enums)

print(f"Found {len(all_enums)} enums. Wrote {ALL_ENUMS_H}.")
