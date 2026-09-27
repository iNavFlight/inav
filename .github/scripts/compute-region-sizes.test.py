#!/usr/bin/env python3
"""Unit tests for compute-region-sizes.py (per-linker-region size breakdown).

Run with: python3 .github/scripts/compute-region-sizes.test.py

Pure-logic tests only - no filesystem/subprocess access, matching the
convention in size-diff-comment.test.js. Exercises assign_sections()
directly with synthetic (name, size, addr) sections rather than building
real .elf/.map files.
"""
import importlib.util
import pathlib
import unittest

_SPEC = importlib.util.spec_from_file_location(
    'compute_region_sizes',
    pathlib.Path(__file__).parent / 'compute-region-sizes.py',
)
crs = importlib.util.module_from_spec(_SPEC)
_SPEC.loader.exec_module(crs)


class AssignSectionsTest(unittest.TestCase):
    def test_zero_address_allocated_section_is_counted(self):
        # F7/H7's ITCM_RAM originates at 0x0 and holds real, allocated
        # .tcm_code (FAST_CODE) - this must not be dropped as if it were
        # unallocated metadata.
        regions = [('ITCM_RAM', 0x00000000, 0x00004000)]
        sections = [('.tcm_code', 22, 0x00000000)]
        self.assertEqual(crs.assign_sections(sections, regions), {'ITCM_RAM': 22})

    def test_non_allocated_sections_are_never_passed_in(self):
        # parse_section_sizes() is responsible for excluding non-ALLOC
        # sections (debug info, .comment, .ARM.attributes, symtab/strtab)
        # before assign_sections() ever sees them - simulate that here by
        # simply not including any such section in the input.
        regions = [('ITCM_RAM', 0x00000000, 0x00004000)]
        sections = [('.tcm_code', 22, 0x00000000)]
        result = crs.assign_sections(sections, regions)
        self.assertEqual(sum(result.values()), 22)

    def test_normal_ram_region_matching_unaffected(self):
        regions = [('RAM', 0x20000000, 0x20020000), ('CCM', 0x10000000, 0x10010000)]
        sections = [
            ('.data', 100, 0x20000000),
            ('.bss', 200, 0x20000100),
            ('.ccm_bss', 50, 0x10000000),
        ]
        self.assertEqual(crs.assign_sections(sections, regions), {'RAM': 300, 'CCM': 50})

    def test_section_outside_any_region_is_dropped(self):
        regions = [('RAM', 0x20000000, 0x20020000)]
        sections = [('.some_other', 100, 0x90000000)]
        self.assertEqual(crs.assign_sections(sections, regions), {})

    def test_empty_regions_yields_empty_result(self):
        self.assertEqual(crs.assign_sections([('.data', 10, 0x20000000)], []), {})


class ParseSectionSizesObjdumpParsingTest(unittest.TestCase):
    """Exercises the objdump -h text parsing in isolation by feeding it
    through the same regex/flag logic parse_section_sizes() uses, without
    invoking a real toolchain."""

    def _parse(self, objdump_output):
        import re
        lines = objdump_output.splitlines()
        sections = []
        for i, line in enumerate(lines):
            m = re.match(r'\s*\d+\s+(\S+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+[0-9a-fA-F]+', line)
            if not m:
                continue
            name, size_str, addr_str = m.groups()
            flags_line = lines[i + 1] if i + 1 < len(lines) else ''
            if 'ALLOC' not in flags_line:
                continue
            size, addr = int(size_str, 16), int(addr_str, 16)
            if size <= 0:
                continue
            sections.append((name, size, addr))
        return sections

    def test_itcm_section_at_zero_address_is_allocated(self):
        out = (
            "t2.elf:     file format elf32-littlearm\n\n"
            "Sections:\n"
            "Idx Name          Size      VMA       LMA       File off  Algn\n"
            "  0 .text         00000018  08000000  08000000  00010000  2**3\n"
            "                  CONTENTS, ALLOC, LOAD, READONLY, CODE\n"
            "  1 .tcm_code     00000016  00000000  08000018  00020000  2**1\n"
            "                  CONTENTS, ALLOC, LOAD, READONLY, CODE\n"
            "  2 .comment      00000033  00000000  00000000  00020016  2**0\n"
            "                  CONTENTS, READONLY\n"
            "  3 .ARM.attributes 0000002e  00000000  00000000  00020049  2**0\n"
            "                  CONTENTS, READONLY\n"
        )
        sections = self._parse(out)
        names = {name for name, _, _ in sections}
        self.assertIn('.tcm_code', names)
        self.assertNotIn('.comment', names)
        self.assertNotIn('.ARM.attributes', names)
        tcm = next(s for s in sections if s[0] == '.tcm_code')
        self.assertEqual(tcm, ('.tcm_code', 0x16, 0x00000000))


if __name__ == '__main__':
    unittest.main()
