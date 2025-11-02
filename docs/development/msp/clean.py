#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
from pathlib import Path
from typing import Any, Iterable

REMOVE_KEYS: frozenset[str] = frozenset({"struct", "size","hex"})

def scrub(node: Any, remove: Iterable[str] = REMOVE_KEYS) -> Any:
    if isinstance(node, dict):
        # delete targeted keys first, then recurse into remaining values
        for key in list(node.keys()):
            if key in remove:
                del node[key]
            else:
                scrub(node[key], remove)
    elif isinstance(node, list):
        for item in node:
            scrub(item, remove)
    return node

def main(in_path: str = "msp_messages.json", out_path: str = "msp_messages.json") -> None:
    data = json.loads(Path(in_path).read_text(encoding="utf-8"))
    scrub(data)
    Path(out_path).write_text(json.dumps(data, indent=4, ensure_ascii=False), encoding="utf-8")

if __name__ == "__main__":
    main()
