#!/usr/bin/env python3
"""Compare changed PG structs against registrations across the repository.

Preprocessor branches are compared symbolically. Complex #if expressions are
conservative independent conditions; this is not a C ABI or macro-expansion check.
"""
import ast
import functools
import itertools
import os
import re
import subprocess
import sys


def git(*args, allow_missing=False):
    result = subprocess.run(['git', *args], capture_output=True, text=True)
    if result.returncode and not (allow_missing and result.returncode == 1):
        raise RuntimeError(result.stderr.strip() or 'git command failed')
    return result.stdout


def clean(source):
    return re.sub(r'/\*.*?\*/|//[^\n]*', lambda m: '\n' * m[0].count('\n'), source, flags=re.S)


def condition(text):
    text = re.sub(r'\s+', '', text)
    match = re.fullmatch(r'(!?)defined\(?([A-Za-z_]\w*)\)?', text)
    return (('defined:' + match[2], not bool(match[1])) if match else (text, True))


def annotated(source):
    """Attach surrounding #if/#elif/#else predicates to each non-directive line."""
    stack = []
    result = []
    for line in source.splitlines(keepends=True):
        match = re.match(r'\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)', line)
        if match:
            directive, value = match.groups()
            if directive in ('if', 'ifdef', 'ifndef'):
                atom = condition(value) if directive == 'if' else ('defined:' + value.strip(), directive == 'ifdef')
                stack.append(([atom], [atom]))
            elif directive == 'elif':
                previous, _ = stack[-1]
                atom = condition(value)
                stack[-1] = (previous + [atom], [(a, not b) for a, b in previous] + [atom])
            elif directive == 'else':
                previous, _ = stack[-1]
                stack[-1] = (previous, [(a, not b) for a, b in previous])
            else:
                stack.pop()
            result.append(('', ()))
        elif re.match(r'\s*#', line):
            result.append(('', ()))
        else:
            result.append((line, tuple(item for _, active in stack for item in active)))
    return result


def structures(source):
    source = clean(source)
    lines = annotated(source)
    result = {}
    for match in re.finditer(r'\btypedef\s+struct(?:\s+[A-Za-z_]\w*)?\s*\{', source):
        depth, end = 1, match.end()
        while end < len(source) and depth:
            depth += (source[end] == '{') - (source[end] == '}')
            end += 1
        alias = re.match(r'\s*([A-Za-z_]\w*)\s*;', source[end:])
        if not alias:
            continue
        first = source.count('\n', 0, match.start())
        last = source.count('\n', 0, end) + 1
        result[alias[1]] = lines[first:last]
    return result


def registrations(ref):
    paths = git('grep', '-l', '-E', 'PG_REGISTER', ref, '--', '*.c', '*.h', allow_missing=True).splitlines()
    result = {}
    for entry in paths:
        path = entry[len(ref) + 1:]
        lines = annotated(clean(git('show', ref + ':' + path)))
        text = ''.join(line if line.endswith('\n') else line + '\n' for line, _ in lines)
        for match in re.finditer(r'\bPG_REGISTER\w*\s*\(([^;]+?)\)\s*;', text):
            args = [arg.strip() for arg in match[1].split(',')]
            if len(args) < 4 or not re.fullmatch(r'[A-Za-z_]\w*', args[0]) or not args[-1].isdigit():
                continue
            line = text.count('\n', 0, match.start())
            result.setdefault(args[0], []).append((args[-2], int(args[-1]), lines[line][1], path))
    return result


@functools.lru_cache(maxsize=None)
def boolean_expression(expression):
    names = []
    def replace_defined(match):
        names.append('defined:' + (match[1] or match[2]))
        return 'v' + str(len(names) - 1)
    translated = re.sub(r'defined(?:\(([A-Za-z_]\w*)\)|([A-Za-z_]\w*))', replace_defined, expression)
    if not names:
        return None
    translated = translated.replace('&&', ' and ').replace('||', ' or ').replace('!', ' not ').strip()
    try:
        tree = ast.parse(translated, mode='eval')
    except SyntaxError:
        return None
    allowed = (ast.Expression, ast.BoolOp, ast.And, ast.Or, ast.UnaryOp, ast.Not, ast.Name, ast.Load)
    if any(not isinstance(node, allowed) for node in ast.walk(tree)):
        return None
    if any(isinstance(node, ast.Name) and node.id not in {'v' + str(i) for i in range(len(names))} for node in ast.walk(tree)):
        return None
    return tree.body, names


def variables(expression):
    parsed = boolean_expression(expression)
    return parsed[1] if parsed else [expression]


def evaluate(expression, values):
    if expression in ('0', '1'):
        return bool(int(expression))
    parsed = boolean_expression(expression)
    if not parsed:
        return values[expression]
    tree, names = parsed
    def visit(node):
        if isinstance(node, ast.Name):
            return values[names[int(node.id[1:])]]
        if isinstance(node, ast.UnaryOp):
            return not visit(node.operand)
        operands = [visit(value) for value in node.values]
        return all(operands) if isinstance(node.op, ast.And) else any(operands)
    return visit(tree)


def active(predicates, values):
    return all(evaluate(atom, values) == expected for atom, expected in predicates)


def layout(lines, values):
    return ''.join(re.sub(r'\s+', '', line) for line, predicates in lines if active(predicates, values))


def check(base, head):
    base = git('merge-base', base, head).strip()
    changed = [path for path in git('diff', '--name-only', base + '..' + head).splitlines() if path.endswith(('.c', '.h'))]
    if not changed:
        print('No C/H files changed')
        return 0
    old_paths = set(git('ls-tree', '-r', '--name-only', base).splitlines())
    new_paths = set(git('ls-tree', '-r', '--name-only', head).splitlines())
    old_structs, new_structs = {}, {}
    for path in changed:
        if path in old_paths:
            old_structs.update(structures(git('show', base + ':' + path)))
        if path in new_paths:
            new_structs.update(structures(git('show', head + ':' + path)))
    old_regs, new_regs = registrations(base), registrations(head)
    issues = []
    for name in old_structs.keys() & new_structs.keys() & new_regs.keys():
        before, after = old_structs[name], new_structs[name]
        if before == after:
            continue
        previous = old_regs.get(name, [])
        current = new_regs[name]
        if not previous:
            continue  # No persisted instance existed before this change.
        predicates = [p for _, p in before + after] + [r[2] for r in previous + current]
        atoms = sorted({variable for predicate in predicates for atom, _ in predicate for variable in variables(atom)} - {'0', '1'})
        if len(atoms) > 10:
            issues.append(f'{name}: more than 10 conditional expressions; manually verify the PG versions')
            continue
        for flags in itertools.product((False, True), repeat=len(atoms)):
            values = dict(zip(atoms, flags))
            old_layout, new_layout = layout(before, values), layout(after, values)
            if old_layout == new_layout or not old_layout:
                continue
            old_versions = {r[0]: r[1] for r in previous if active(r[2], values)}
            new_versions = {r[0]: r[1] for r in current if active(r[2], values)}
            if any(pg not in new_versions or new_versions[pg] <= version for pg, version in old_versions.items()):
                issues.append(f'{name}: changed layout without a version increase in {", ".join(sorted({r[3] for r in current}))}; conditions {values}')
                break
    for issue in issues:
        print('PG version issue: ' + issue)
    if not issues:
        print('No PG version issues detected')
    return int(bool(issues))


if __name__ == '__main__':
    try:
        base = 'origin/' + os.environ['GITHUB_BASE_REF'] if os.environ.get('GITHUB_BASE_REF') and os.environ.get('GITHUB_HEAD_REF') else 'HEAD~1'
        sys.exit(check(base, 'HEAD'))
    except (RuntimeError, ValueError, IndexError, OSError) as error:
        print('PG checker error: ' + str(error), file=sys.stderr)
        sys.exit(2)
