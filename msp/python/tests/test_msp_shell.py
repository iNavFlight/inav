from __future__ import annotations

import io
import unittest
from contextlib import redirect_stdout
from unittest.mock import patch

from inav_msp import msp_shell


class FakeApi:
    instance = None

    def __init__(self, **kwargs) -> None:
        self.init_kwargs = kwargs
        self.info = None
        self.calls = []
        self.closed = False
        FakeApi.instance = self

    def open(self) -> None:
        pass

    def close(self) -> None:
        self.closed = True

    def set_armed(self, arm: bool):
        self.calls.append(arm)
        self.info = {
            "latency_ms": 1.25,
            "attempt": 1,
            "transport": "serial",
        }
        return {}


class ShellTest(unittest.TestCase):
    def test_boolean_words_are_real_booleans(self) -> None:
        self.assertIs(msp_shell._parse_value("true"), True)
        self.assertIs(msp_shell._parse_value("false"), False)
        self.assertIsNone(msp_shell._parse_value("null"))
        self.assertEqual(msp_shell._parse_value("'false'"), "false")

    def test_shell_invokes_api_and_always_closes(self) -> None:
        output = io.StringIO()
        with patch.object(msp_shell, "MSPApi", FakeApi), \
                patch("builtins.input", side_effect=["set_armed arm=false", "set_armed true", "quit"]), \
                redirect_stdout(output):
            status = msp_shell.main(["--port", "unused", "--read-timeout", "10"])

        api = FakeApi.instance
        self.assertEqual(status, 0)
        self.assertEqual(api.calls, [False, True])
        self.assertTrue(api.closed)
        self.assertEqual(api.init_kwargs["read_timeout_ms"], 10.0)
        self.assertIn("set_armed:", output.getvalue())
        self.assertIn("latency=1.25ms", output.getvalue())

    def test_bad_input_does_not_exit_shell(self) -> None:
        output = io.StringIO()
        with patch.object(msp_shell, "MSPApi", FakeApi), \
                patch("builtins.input", side_effect=["set_armed arm=maybe", "set_armed arm=false", "quit"]), \
                redirect_stdout(output):
            status = msp_shell.main(["--port", "unused"])

        self.assertEqual(status, 0)
        self.assertEqual(FakeApi.instance.calls, [False])
        self.assertIn("Input error:", output.getvalue())


if __name__ == "__main__":
    unittest.main()
