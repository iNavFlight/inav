from __future__ import annotations

import unittest

from inav_msp.utils import format_nested_dict, msp_override_mask


class UtilsTest(unittest.TestCase):
    def test_msp_override_mask_uses_one_based_channel_numbers(self) -> None:
        self.assertEqual(msp_override_mask([1, 4, 8]), 0x89)

    def test_empty_containers_are_visible(self) -> None:
        self.assertEqual(format_nested_dict([]), "[]")
        self.assertEqual(format_nested_dict({}), "{}")
        self.assertEqual(format_nested_dict({"nodes": []}), "nodes: []")


if __name__ == "__main__":
    unittest.main()
