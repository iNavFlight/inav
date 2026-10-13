from __future__ import annotations

import struct
import unittest

from inav_msp.lib import InavMSP
from inav_msp.msp_api import MSPApi


class ReadOnlySerial:
    def __init__(self) -> None:
        self.requests = []
        self.last_diag = None

    def open(self) -> None:
        pass

    def close(self) -> None:
        pass

    def request(self, code: int, payload: bytes, *, timeout: float):
        self.requests.append((code, payload, timeout))
        if code == InavMSP.MSP_RC:
            reply = struct.pack("<4H", 1000, 1500, 1750, 2000)
        elif code == InavMSP.MSP_RX_MAP:
            reply = bytes([0, 1, 3, 2])
        elif code == InavMSP.MSP2_INAV_LOCAL_TARGET:
            reply = struct.pack("<iiihhhih", 100, 200, 300, 40, 50, 60, 9000, 70)
        elif code == InavMSP.MSP2_INAV_NAV_TARGET:
            reply = struct.pack("<iiiHhI", 123456789, -234567890, 1234, 90, -25, 4567)
        elif code == InavMSP.MSP_FC_VERSION:
            reply = b"\x0a\x00\x01"
        else:
            reply = b""
        return code, reply


class ApiTest(unittest.TestCase):
    def setUp(self) -> None:
        self.serial = ReadOnlySerial()
        self.api = MSPApi(serial_transport=self.serial)

    def test_get_ch_returns_selected_channel(self) -> None:
        self.assertEqual(self.api.get_ch(2), 1750)

    def test_mapping_channel_frame_preserves_current_channels(self) -> None:
        self.assertEqual(self.api._build_channel_frame({1: 1600, 3: 1800}), [1000, 1600, 1750, 1800])

    def test_schema_capabilities_are_not_disabled_by_static_firmware_version(self) -> None:
        self.assertTrue(hasattr(MSPApi, "get_local_target"))
        self.assertEqual(
            self.api.get_local_target(),
            {
                "pos": {"x_cm": 100, "y_cm": 200, "z_cm": 300},
                "vel": {"x_cm_s": 40, "y_cm_s": 50, "z_cm_s": 60},
                "yaw_deg": 90.0,
                "climb_rate_ms": 0.7,
            },
        )

    def test_nav_target_includes_loiter_radius(self) -> None:
        self.assertEqual(
            self.api.get_nav_target(),
            {
                "latitude": 12.3456789,
                "longitude": -23.456789,
                "altitude_m": 12.34,
                "heading_deg": 90.0,
                "climb_rate_ms": -0.25,
                "loiter_radius_m": 45.67,
            },
        )

    def test_global_target_omits_or_appends_optional_loiter_radius(self) -> None:
        self.api.set_global_target(
            latitude_deg=1.0,
            longitude_deg=2.0,
            altitude_m=3.0,
        )
        self.assertEqual(len(self.serial.requests[-1][1]), 13)

        self.api.set_global_target(
            latitude_deg=1.0,
            longitude_deg=2.0,
            altitude_m=3.0,
            loiter_radius_m=45.0,
        )
        self.assertEqual(len(self.serial.requests[-1][1]), 17)

    def test_get_fc_version_reads_connected_firmware(self) -> None:
        self.assertEqual(
            self.api.get_fc_version(),
            {"major": 10, "minor": 0, "patch": 1},
        )


if __name__ == "__main__":
    unittest.main()
