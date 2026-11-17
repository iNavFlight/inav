import enum
from typing import List

from .mspcodec import InavMSP

globals().update(InavMSP.__members__)

def msp_override_mask(channels: List[int]) -> int:
    """Bitmask for `set msp_override_channels`, 1-based channel positions."""
    result = 0
    for position in channels:
        result |= 1 << (position - 1)
    return result

def format_nested_dict(data, *, indent_step: int = 4, start_indent: int = 0) -> str:
    """
    Render nested dicts/lists as indented lines keyed by depth.
    """
    lines = []
    base_pad = start_indent * indent_step

    def render_scalar(value):
        if isinstance(value, enum.Enum):
            return repr(value)
        if isinstance(value, str):
            return value
        return repr(value)

    def walk(node, depth: int) -> None:
        pad = " " * (base_pad + depth * indent_step)
        if isinstance(node, dict):
            if not node:
                lines.append(f"{pad}{{}}")
                return
            for key, value in node.items():
                if isinstance(value, (dict, list, tuple)):
                    if not value:
                        lines.append(f"{pad}{key}: {'{}' if isinstance(value, dict) else '[]'}")
                        continue
                    lines.append(f"{pad}{key}:")
                    walk(value, depth + 1)
                    continue
                lines.append(f"{pad}{key}: {render_scalar(value)}")
            return
        if isinstance(node, (list, tuple)):
            if not node:
                lines.append(f"{pad}[]")
                return
            for idx, item in enumerate(node):
                if isinstance(item, (dict, list, tuple)):
                    if not item:
                        lines.append(f"{pad}{idx}: {'{}' if isinstance(item, dict) else '[]'}")
                        continue
                    lines.append(f"{pad}{idx}:")
                    walk(item, depth + 1)
                    continue
                lines.append(f"{pad}{idx}: {render_scalar(item)}")
            return
        lines.append(f"{pad}{render_scalar(node)}")

    walk(data, 0)
    return "\n".join(lines)
