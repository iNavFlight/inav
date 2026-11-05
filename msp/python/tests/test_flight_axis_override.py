#!/usr/bin/env python3
"""
Hardcoded pulse test for MSP flight-axis overrides (default serial /dev/ttyACM0 or TCP 127.0.0.1:5760).
Sends roll angle and yaw rate overrides for 1s each, with clears in between, re-sending at 10 Hz to keep them alive.
"""

DEFAULT_TCP_ENDPOINT = "127.0.0.1:5760"
DEFAULT_BAUDRATE = 115200
DEFAULT_SERIAL_PORT = "/dev/ttyUSB0"
DEFAULT_UDP_ENDPOINT = "127.0.0.1:27072"
HOLD_SECONDS = 3.0
SEND_HZ = 10.0

import argparse
import time

from inav_msp.msp_api import MSPApi
from inav_msp.lib import boxes


def main() -> None:
    ap = argparse.ArgumentParser(description="Hardcoded MSP flight-axis override pulse test.")
    ap.add_argument("--tcp-endpoint", default=DEFAULT_TCP_ENDPOINT, help="MSP TCP endpoint in HOST:PORT format")
    ap.add_argument("--udp-endpoint", default=DEFAULT_UDP_ENDPOINT, help="MSP UDP endpoint in HOST:PORT format")
    ap.add_argument("--port", default=DEFAULT_SERIAL_PORT, help="Serial MSP port")
    ap.add_argument("--baudrate", type=int, default=DEFAULT_BAUDRATE, help="Baudrate for serial MSP")
    ap.add_argument("--force-mspv2", action="store_true", help="Force MSP v2 framing")
    args = ap.parse_args()

    init_kwargs = {}
    if args.tcp_endpoint and args.udp_endpoint:
        ap.error("Provide only one of --tcp-endpoint or --udp-endpoint")
    if args.tcp_endpoint:
        init_kwargs["tcp_endpoint"] = args.tcp_endpoint
        init_kwargs["port"] = None
    elif args.udp_endpoint:
        init_kwargs["udp_endpoint"] = args.udp_endpoint
        init_kwargs["port"] = None
    else:
        init_kwargs["port"] = args.port
        init_kwargs["baudrate"] = args.baudrate
    init_kwargs["force_msp_v2"] = args.force_mspv2

    with MSPApi(**init_kwargs) as api:
        api_version = api.get_api_version()
        print(f"Connected: API {api_version}")

        active_modes = api.get_active_modes()
        if boxes.BoxEnum.BOXMSPRCOVERRIDE not in active_modes:
            print("MSP RC OVERRIDE is not active; enable the mode before running.")

        sequence = [
            ("angle", dict(roll_deg=25.0, pitch_deg=None, yaw_deg=None)),
            ("rate", dict(roll_dps=90.0, pitch_dps=None, yaw_dps=0.0)),
        ]

        interval = 1.0 / SEND_HZ

        for kind, payload in sequence:
            if kind == "angle":
                end_time = time.monotonic() + HOLD_SECONDS
                while time.monotonic() < end_time:
                    api.set_flight_axis_angle_override(**payload)
                    time.sleep(interval)
                print(f"Sent angle override {payload} for {HOLD_SECONDS}s")
            elif kind == "rate":
                end_time = time.monotonic() + HOLD_SECONDS
                while time.monotonic() < end_time:
                    api.set_flight_axis_rate_override(**payload)
                    time.sleep(interval)
                print(f"Sent rate override {payload} for {HOLD_SECONDS}s")

            if kind == "angle":
                end_time = time.monotonic() + HOLD_SECONDS
                while time.monotonic() < end_time:
                    api.set_flight_axis_angle_override()
                    time.sleep(interval)
                print(f"Cleared angle override for {HOLD_SECONDS}s")
            elif kind == "rate":
                end_time = time.monotonic() + HOLD_SECONDS
                while time.monotonic() < end_time:
                    api.set_flight_axis_rate_override()
                    time.sleep(interval)
                print(f"Cleared rate override for {HOLD_SECONDS}s")



if __name__ == "__main__":
    main()
