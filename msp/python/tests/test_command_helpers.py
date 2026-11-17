from __future__ import annotations

import struct
import unittest

from inav_msp.lib import InavMSP
from inav_msp.msp_api import MSPApi


class FakeSerial:
    def __init__(self) -> None:
        self.requests = []
        self.last_diag = None

    def open(self) -> None:
        pass

    def close(self) -> None:
        pass

    def request(self, code: int, payload: bytes, *, timeout: float):
        self.requests.append((code, payload, timeout))
        if code == InavMSP.MSP2_INAV_TIMESYNC:
            reply = struct.pack("<Q", 123456789000)
        elif code == InavMSP.MSP2_INAV_GET_LINK_STATS:
            reply = struct.pack("<BBb", 70, 99, 8)
        elif code == InavMSP.MSP2_INAV_DRONECAN_NODES:
            # nodeCount, then 13-byte records: id,health,mode,last_seen,uptime,vendor
            reply = (b"\x02"
                     + struct.pack("<BBBIIH", 10, 0, 0, 1000, 60, 7)
                     + struct.pack("<BBBIIH", 42, 1, 1, 2000, 120, 9))
        else:
            reply = b""
        return code, reply


class CommandHelperTest(unittest.TestCase):
    def setUp(self) -> None:
        self.serial = FakeSerial()
        self.api = MSPApi(serial_transport=self.serial)

    def test_set_armed_packs_boolean_request(self) -> None:
        self.assertEqual(self.api.set_armed(True), {})
        self.assertEqual(
            self.serial.requests[-1],
            (InavMSP.MSP2_INAV_ARM_DISARM, b"\x01", 1.0),
        )

    def test_activate_rth_has_empty_request(self) -> None:
        self.assertEqual(self.api.activate_rth(), {})
        self.assertEqual(
            self.serial.requests[-1],
            (InavMSP.MSP2_INAV_ACTIVATE_RTH, b"", 1.0),
        )

    def test_activate_landing_has_empty_request(self) -> None:
        self.assertEqual(self.api.activate_landing(), {})
        self.assertEqual(
            self.serial.requests[-1],
            (InavMSP.MSP2_INAV_ACTIVATE_LANDING, b"", 1.0),
        )

    def test_get_timesync_ns_decodes_uint64(self) -> None:
        self.assertEqual(self.api.get_timesync_ns(), 123456789000)
        self.assertEqual(
            self.serial.requests[-1],
            (InavMSP.MSP2_INAV_TIMESYNC, b"", 1.0),
        )

    def test_get_link_stats_decodes_signal_values(self) -> None:
        self.assertEqual(
            self.api.get_link_stats(),
            {
                "uplinkRSSI_dBm": 70,
                "uplinkLQ": 99,
                "uplinkSNR": 8,
            },
        )

    def test_get_dronecan_nodes_decodes_packed_records(self) -> None:
        self.assertEqual(
            self.api.get_dronecan_nodes(),
            [
                {"nodeID": 10, "health": 0, "mode": 0, "last_seen_ms": 1000,
                 "uptime_sec": 60, "vendor_status_code": 7},
                {"nodeID": 42, "health": 1, "mode": 1, "last_seen_ms": 2000,
                 "uptime_sec": 120, "vendor_status_code": 9},
            ],
        )

    def test_get_dronecan_nodes_decodes_empty_list(self) -> None:
        self.serial.request = lambda code, payload, timeout: (code, b"\x00")
        self.assertEqual(self.api.get_dronecan_nodes(), [])


    def test_set_waypoint_index_packs_index(self) -> None:
        self.assertEqual(self.api.set_waypoint_index(3), {})
        self.assertEqual(
            self.serial.requests[-1],
            (InavMSP.MSP2_INAV_SET_WP_INDEX, b"\x03", 1.0),
        )

    def test_set_cruise_heading_packs_centidegrees(self) -> None:
        self.assertEqual(self.api.set_cruise_heading(90.25), {})
        self.assertEqual(
            self.serial.requests[-1],
            (InavMSP.MSP2_INAV_SET_CRUISE_HEADING, struct.pack("<i", 9025), 1.0),
        )

    def test_set_aux_rc_packs_all_resolutions(self) -> None:
        self.api.set_aux_rc(12, [1000, 1500, 2000, 0], resolution_bits=2)
        self.assertEqual(
            self.serial.requests[-1],
            (InavMSP.MSP2_INAV_SET_AUX_RC, b"\x60\x6c", 1.0),
        )

        self.api.set_aux_rc(12, [1000, 1500], resolution_bits=4)
        self.assertEqual(
            self.serial.requests[-1],
            (InavMSP.MSP2_INAV_SET_AUX_RC, b"\x61\x18", 1.0),
        )

        self.api.set_aux_rc(13, [1000, 1500, 2000, 0], resolution_bits=8)
        self.assertEqual(
            self.serial.requests[-1],
            (InavMSP.MSP2_INAV_SET_AUX_RC, b"\x6a\x01\x80\xff\x00", 1.0),
        )

        self.api.set_aux_rc(12, [1600, 1400], resolution_bits=16)
        self.assertEqual(
            self.serial.requests[-1],
            (InavMSP.MSP2_INAV_SET_AUX_RC, b"\x63\x40\x06\x78\x05", 1.0),
        )


if __name__ == "__main__":
    unittest.main()
