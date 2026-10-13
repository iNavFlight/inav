from __future__ import annotations

import unittest

from inav_msp.msp_serial import (
    DIR_FROM_FC,
    DIR_TO_FC,
    MSPMessage,
    MSPSerial,
    _MSPParser,
)


class BufferedSerial:
    def __init__(self, payload: bytes) -> None:
        self.buffer = bytearray(payload)
        self.is_open = True
        self.read_sizes = []

    @property
    def in_waiting(self) -> int:
        return len(self.buffer)

    def read(self, size: int) -> bytes:
        self.read_sizes.append(size)
        if not self.buffer:
            return b""
        chunk = bytes(self.buffer[:size])
        del self.buffer[:size]
        return chunk

    def close(self) -> None:
        self.is_open = False


def response_frame(version: int, code: int, payload: bytes) -> bytes:
    frame = bytearray(MSPSerial._encode(version, code, payload))
    frame[2] = DIR_FROM_FC
    return bytes(frame)


class ParserTest(unittest.TestCase):
    def test_fragmented_v1_and_v2_frames(self) -> None:
        parser = _MSPParser()
        frames = response_frame(1, 1, b"\x00\x02\x06") + response_frame(2, 0x2103, b"\x46\x63\x08")
        messages = []
        for byte in frames:
            messages.extend(parser.feed(bytes([byte])))
        self.assertEqual(
            [(message.version, message.code, message.payload, message.direction) for message in messages],
            [
                (1, 1, b"\x00\x02\x06", DIR_FROM_FC),
                (2, 0x2103, b"\x46\x63\x08", DIR_FROM_FC),
            ],
        )

    def test_forced_v1_255_byte_payload_uses_jumbo_framing(self) -> None:
        payload = bytes(range(255))
        frame = response_frame(1, 42, payload)
        self.assertEqual(frame[3], 0xFF)
        self.assertEqual(frame[5:7], b"\xff\x00")
        messages = _MSPParser().feed(frame)
        self.assertEqual(len(messages), 1)
        self.assertEqual(messages[0].payload, payload)


class ReaderTest(unittest.TestCase):
    def test_reader_blocks_for_one_byte_then_drains_available_data(self) -> None:
        transport = MSPSerial("unused", read_timeout=0.1, log_path=None)
        serial = BufferedSerial(response_frame(1, 1, b"\x00\x02\x06"))
        transport._ser = serial
        original_read = serial.read

        def read_and_stop(size: int) -> bytes:
            chunk = original_read(size)
            if not serial.buffer:
                transport._stop_evt.set()
            return chunk

        serial.read = read_and_stop
        queue = transport._get_queue_for_code(1)
        transport._reader_loop()
        message = queue.get_nowait()
        self.assertEqual(message.payload, b"\x00\x02\x06")
        self.assertEqual(serial.read_sizes[0], 1)
        self.assertNotIn(4096, serial.read_sizes)

    def test_outbound_echo_does_not_satisfy_response_queue(self) -> None:
        transport = MSPSerial("unused", log_path=None)
        queue = transport._get_queue_for_code(1)
        transport._dispatch(MSPMessage(1, 1, b"echo", direction=DIR_TO_FC))
        transport._dispatch(MSPMessage(1, 1, b"reply", direction=DIR_FROM_FC))
        self.assertEqual(queue.get_nowait().payload, b"reply")
        self.assertTrue(queue.empty())


if __name__ == "__main__":
    unittest.main()
