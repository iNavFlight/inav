#!/usr/bin/env python3
import datetime
import re
from pathlib import Path

BASE = Path('../../../src/main')
SUBDIRS = [
    'common',
    'navigation',
    'sensors',
    'programming',
    'rx',
    'telemetry',
    'io',
    'flight',
    'fc',
]

def strip_comments(text: str) -> str:
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.DOTALL)   # block comments
    text = re.sub(r'//.*', '', text)                         # line comments
    return text

def extract_enums(fn: str, text: str):
    text = strip_comments(text)
    enums = []
    i = 0
    n = len(text)
    while True:
        m = re.search(r'\btypedef\s+enum\b', text[i:])
        if not m:
            break
        start = i + m.start()
        # find first '{'
        brace_pos = text.find('{', start)
        if brace_pos == -1:
            break
        depth = 0
        k = brace_pos
        while k < n:
            c = text[k]
            if c == '{':
                depth += 1
            elif c == '}':
                depth -= 1
                if depth == 0:
                    end = text.find(';', k)
                    if end == -1:
                        end = n
                    block = text[start:end+1]
                    if re.search(r'\}\s*[A-Za-z_]\w*\s*;', block):
                        enums.append(f'// {fn}\n')
                        enums.append(block.strip() + '\n\n')
                    i = end + 1
                    break
            k += 1
        else:
            break
    return enums

all_enums = []
for sd in SUBDIRS:
    root = BASE / sd
    if not root.is_dir():
        continue
    for fn in root.rglob('*'):
        print(fn)
        if fn.suffix in ('.c', '.h'):
            txt = fn.read_text(errors='ignore')
            ret = extract_enums(fn, txt)
            if ret: print(fn)
            all_enums.extend(ret)

with open('all_enums.h', 'w') as out:
    out.write(f"// Consolidated enums — generated on {datetime.datetime.now()}\n\n")
    out.writelines(all_enums)

print(f"Found {len(all_enums)} enums. Wrote all_enums.h.")
