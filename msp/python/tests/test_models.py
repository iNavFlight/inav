"""The generated Pydantic models (optional ``pydantic`` extra)."""

import unittest

try:
    import pydantic
except ImportError:  # pydantic is an optional extra: inav-msp[models]
    pydantic = None


@unittest.skipIf(pydantic is None, "pydantic is not installed (inav-msp[models])")
class GeneratedModelsTest(unittest.TestCase):
    def test_every_model_resolves_its_annotations(self) -> None:
        from pydantic import BaseModel

        from inav_msp.models import inav_msp

        models = [
            obj
            for obj in vars(inav_msp).values()
            if isinstance(obj, type) and issubclass(obj, BaseModel) and obj is not BaseModel
        ]
        self.assertGreater(len(models), 250)
        for model in models:
            model.model_rebuild(force=True)

    def test_esc_telemetry_keeps_the_opaque_record_as_bytes(self) -> None:
        from inav_msp.models.inav_msp import MSP2_INAV_ESC_TELEM_reply_t

        reply = MSP2_INAV_ESC_TELEM_reply_t.model_validate(
            {
                "motorCount": 1,
                "escData": [{"esc": bytes(range(16))}],
            }
        )
        self.assertIsInstance(reply.escData[0].esc, bytes)
        self.assertEqual(bytes(reply.escData[0].esc), bytes(range(16)))

    def test_led_config_record_is_bytes(self) -> None:
        from inav_msp.models.inav_msp import MSP2_INAV_LED_STRIP_CONFIG_EX_reply_t

        reply = MSP2_INAV_LED_STRIP_CONFIG_EX_reply_t.model_validate(
            {"items": [{"ledConfig": b"\x01\x02\x03\x04\x05"}]}
        )
        self.assertIsInstance(reply.items[0].ledConfig, bytes)
        self.assertEqual(len(reply.items[0].ledConfig), 5)
