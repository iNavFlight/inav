#!/usr/bin/env python3
"""
Usage:
  conda run -n drone python mydev/branch/mav_multi/mavlink_test_rc.py --master tcp:127.0.0.1:5761
  conda run -n drone python mydev/branch/mav_multi/mavlink_test_rc.py --master tcp:127.0.0.1:5761 --duration 20 --tx-hz 100 --roll MID --pitch MID --yaw MID --throttle LOW
"""

from __future__ import annotations

import argparse
import time
from collections import Counter
from typing import List

from pymavlink import mavutil


CHANNEL_ORDER = [
    "roll",
    "pitch",
    "throttle",
    "yaw",
    "ch5",
    "ch6",
    "ch7",
    "ch8",
    "ch9",
    "ch10",
    "ch11",
    "ch12",
    "ch13",
    "ch14",
    "ch15",
    "ch16",
    "ch17",
    "ch18",
]
DEFAULT_CHANNELS = [900] * 18
DEFAULT_CHANNELS[0] = 1500
DEFAULT_CHANNELS[1] = 1500
DEFAULT_CHANNELS[3] = 1500
LEVEL_TO_PWM = {"LOW": 900, "MID": 1500, "HIGH": 2100}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Send MAVLink RC_CHANNELS_OVERRIDE stream and monitor RC echo.")
    parser.add_argument("--master", required=True, help='pymavlink master endpoint, e.g. "tcp:127.0.0.1:5761"')
    parser.add_argument("--tx-hz", type=float, default=100.0, help="RC override transmit rate in Hz")
    parser.add_argument("--duration", type=float, default=0.0, help="Test duration in seconds, 0 means run forever")
    parser.add_argument("--source-system", type=int, default=240, help="MAVLink source system ID")
    parser.add_argument("--source-component", type=int, default=191, help="MAVLink source component ID")
    parser.add_argument("--echo-tolerance", type=int, default=20, help="Allowed absolute PWM error for RC_CHANNELS echo checks")
    for channel_name in CHANNEL_ORDER:
        parser.add_argument(f"--{channel_name}", default=None, help="LOW/MID/HIGH or integer PWM")
    args = parser.parse_args()

    channels: List[int] = list(DEFAULT_CHANNELS)
    for idx, channel_name in enumerate(CHANNEL_ORDER):
        raw_value = getattr(args, channel_name)
        if raw_value is None:
            continue
        level_name = raw_value.upper()
        if level_name in LEVEL_TO_PWM:
            channels[idx] = LEVEL_TO_PWM[level_name]
        else:
            channels[idx] = int(raw_value)

    print(
        f"master={args.master} tx_hz={args.tx_hz} duration={args.duration} "
        f"source_system={args.source_system} source_component={args.source_component} channels={channels}",
        flush=True,
    )

    master = mavutil.mavlink_connection(
        args.master,
        source_system=args.source_system,
        source_component=args.source_component,
        autoreconnect=True,
    )
    heartbeat = master.wait_heartbeat(timeout=10)
    if heartbeat is None:
        raise TimeoutError("No heartbeat received from FC")
    target_system = heartbeat.get_srcSystem()
    target_component = heartbeat.get_srcComponent()
    print(f"target_system={target_system} target_component={target_component}", flush=True)

    period_s = 1.0 / args.tx_hz
    tx_count = 0
    rx_count = 0
    mismatch_count = 0
    decode_errors = 0
    last_rx_time = 0.0
    loop_start = time.monotonic()
    next_tx_time = loop_start
    next_report_time = loop_start + 1.0
    mismatch_hist = Counter()

    while True:
        now = time.monotonic()
        if args.duration > 0 and (now - loop_start) >= args.duration:
            break

        if now >= next_tx_time:
            master.mav.rc_channels_override_send(target_system, target_component, *channels[:8])
            tx_count += 1
            next_tx_time += period_s
            if next_tx_time < now:
                next_tx_time = now + period_s

        message = master.recv_match(type=["RC_CHANNELS", "RC_CHANNELS_RAW"], blocking=False)
        if message is not None:
            rx_count += 1
            last_rx_time = now
            if message.get_type() == "RC_CHANNELS":
                for i in range(4):
                    key = f"chan{i + 1}_raw"
                    observed = int(getattr(message, key))
                    err = abs(observed - channels[i])
                    if err > args.echo_tolerance:
                        mismatch_count += 1
                        mismatch_hist[i + 1] += 1
            else:
                decode_errors += 1

        if now >= next_report_time:
            rx_age = -1.0 if last_rx_time == 0.0 else (now - last_rx_time)
            print(
                f"status_s={now - loop_start:.1f} tx={tx_count} rx={rx_count} mismatches={mismatch_count} "
                f"decode_errors={decode_errors} rx_age_s={rx_age:.3f} mismatch_hist={dict(mismatch_hist)}",
                flush=True,
            )
            next_report_time += 1.0

        time.sleep(0.001)

    elapsed = max(time.monotonic() - loop_start, 1e-6)
    print(
        f"summary elapsed_s={elapsed:.2f} tx={tx_count} rx={rx_count} mismatch_count={mismatch_count} "
        f"mismatch_rate={mismatch_count / max(rx_count, 1):.6f} tx_hz={tx_count / elapsed:.2f} rx_hz={rx_count / elapsed:.2f}",
        flush=True,
    )
