"""Reading a member of an `enums.yaml` `values:` list.

A member is written on one line, in C style:

    - NAME = VALUE
    - if "BUILD CONDITION": NAME = VALUE

`VALUE` is a decimal integer or the name of an earlier member. The build
condition is a C preprocessor expression; it is carried through verbatim so
each backend can compile it in its own dialect. The condition applies to the
member only: the member is still declared, so decoders keep working.

This module is the one place that knows the member syntax; every generator
reads `values:` through `split_member()`.
"""

from __future__ import annotations

import re

_CONDITIONAL = re.compile(r'^if\s+"(.*)"$')


class EnumSyntaxError(ValueError):
    """A `values:` entry does not follow the member syntax."""


def split_member(entry) -> tuple[str, str | None, str | None]:
    """-> (name, value text or None when unassigned, build condition or None)."""
    if isinstance(entry, dict):
        if len(entry) != 1:
            raise EnumSyntaxError(
                f"enum member must be one 'if \"COND\": NAME = VALUE' entry, got {entry!r}")
        (head, raw), = entry.items()
        match = _CONDITIONAL.match(str(head))
        if not match:
            raise EnumSyntaxError(
                f"conditional enum member must start with 'if \"COND\":', got {head!r}")
        condition = match.group(1)
        text = str(raw).strip()
    elif isinstance(entry, str):
        condition = None
        text = entry.strip()
    else:
        raise EnumSyntaxError(f"enum member must be a string or one-line mapping, got {entry!r}")

    name, sep, value = text.partition(" = ")
    if not sep:
        return text.strip(), None, condition
    return name.strip(), value.strip(), condition
