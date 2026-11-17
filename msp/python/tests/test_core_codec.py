from __future__ import annotations

import struct
import unittest

from inav_msp.lib import InavMSP
from inav_msp.mspcodec import MSPCodec, MSPPackError



class CodecTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.codec = MSPCodec()

    def test_indexed_setting_encodes_the_leading_zero_discriminator(self) -> None:
        payload = self.codec.pack_request(
            InavMSP.MSP2_COMMON_SETTING,
            {"settingIdentifier": {"settingIndex": 0x04D2}},
        )
        self.assertEqual(payload, b"\x00\xd2\x04")
        self.assertEqual(
            self.codec.unpack_request(InavMSP.MSP2_COMMON_SETTING, payload),
            {"settingIdentifier": {"case": "index", "zero": 0, "settingIndex": 0x04D2}},
        )

    def test_indexed_setting_set_encodes_index_then_value(self) -> None:
        payload = self.codec.pack_request(
            InavMSP.MSP2_COMMON_SET_SETTING,
            {"settingIdentifier": {"settingIndex": 3}, "settingValue": [1, 2]},
        )
        self.assertEqual(payload, b"\x00\x03\x00\x01\x02")

    def test_explicit_index_discriminator_must_stay_zero(self) -> None:
        with self.assertRaises(MSPPackError):
            self.codec.pack_request(
                InavMSP.MSP2_COMMON_SETTING,
                {"settingIdentifier": {"case": "index", "zero": 1, "settingIndex": 3}},
            )

    def test_named_setting_still_encodes_as_a_cstring(self) -> None:
        payload = self.codec.pack_request(
            InavMSP.MSP2_COMMON_SETTING,
            {"settingIdentifier": {"case": "name", "settingName": "pid_roll_p"}},
        )
        self.assertEqual(payload, b"pid_roll_p\x00")

    def test_empty_variable_character_array_decodes(self) -> None:
        self.assertEqual(self.codec.unpack_reply(InavMSP.MSP_NAME, b""), {"craftName": ""})

    def test_float_field_keeps_its_fraction(self) -> None:
        values = {"instance": 0, "timeMs": 0, "pressurePa": 101325.75, "temp": 0}
        payload = self.codec.pack_request(InavMSP.MSP2_SENSOR_BAROMETER, values)
        self.assertEqual(self.codec.unpack_request(InavMSP.MSP2_SENSOR_BAROMETER, payload)["pressurePa"], 101325.75)

    def test_variable_request_round_trip(self) -> None:
        values = {
            "definitionByte": 0x63,
            "channelData": [0x40, 0x06, 0x78, 0x05],
        }
        payload = self.codec.pack_request(InavMSP.MSP2_INAV_SET_AUX_RC, values)
        self.assertEqual(self.codec.unpack_request(InavMSP.MSP2_INAV_SET_AUX_RC, payload), values)

    def test_optional_trailing_request_field_can_be_omitted(self) -> None:
        values = {
            "latitude": 10000000,
            "longitude": 20000000,
            "altitudeTarget": 300,
            "altitudeDatum": 0,
        }
        payload = self.codec.pack_request(InavMSP.MSP2_INAV_SET_GLOBAL_TARGET, values)
        self.assertEqual(len(payload), 13)
        self.assertEqual(self.codec.unpack_request(InavMSP.MSP2_INAV_SET_GLOBAL_TARGET, payload), values)

    def test_repeating_reply_round_trip(self) -> None:
        values = [
            {
                "modePermanentId": 0,
                "auxChannelIndex": 0,
                "rangeStartStep": 16,
                "rangeEndStep": 48,
            },
            {
                "modePermanentId": 1,
                "auxChannelIndex": 1,
                "rangeStartStep": 16,
                "rangeEndStep": 32,
            },
        ]
        payload = self.codec.pack_reply(InavMSP.MSP_MODE_RANGES, values)
        self.assertEqual(self.codec.unpack_reply(InavMSP.MSP_MODE_RANGES, payload), values)

    def test_opaque_led_config_round_trip(self) -> None:
        values = {
            "ledIndex": 2,
            # ledConfig_t's bit layout comes from INAV's own declaration via
            # extract_wire_types.py, so the codec decodes it into named fields.
            "ledConfig": {
                "led_position": 1, "led_function": 2, "led_overlay": 3,
                "led_color": 4, "led_direction": 5, "led_params": 6,
            },
        }
        payload = self.codec.pack_request(InavMSP.MSP2_INAV_SET_LED_STRIP_CONFIG_EX, values)
        self.assertEqual(self.codec.unpack_request(InavMSP.MSP2_INAV_SET_LED_STRIP_CONFIG_EX, payload), values)

    def test_esc_telemetry_struct_array_decodes(self) -> None:
        payload = b"\x01" + struct.pack("<BxhhxxiI", 2, 45, 1200, 340, 5000)
        self.assertEqual(
            self.codec.unpack_reply(InavMSP.MSP2_INAV_ESC_TELEM, payload),
            {
                "motorCount": 1,
                # escSensorData_t's field offsets come from the compiler that
                # builds INAV, not from a layout restated in the schema.
                "escData": [
                    {
                        "esc": {
                            "dataAge": 2, "temperature": 45, "voltage": 1200,
                            "current": 340, "rpm": 5000,
                        },
                    },
                ],
            },
        )


if __name__ == "__main__":
    unittest.main()
