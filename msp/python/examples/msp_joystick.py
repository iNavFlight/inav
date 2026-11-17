#!/usr/bin/env python3
"""
Usage:
  python msp_joystick.py --port /dev/ttyACM0 --baudrate 115200 --rate 50 --joystick-index 0
  python msp_joystick.py --tcp 127.0.0.1:5760 --rate 50 --joystick-index 0 --write-timeout-ms 50

Sends joystick channels as MSP_SET_RAW_RC (16 channels).
Requires pygame: pip install pygame
"""

import argparse
import struct
import time

import pygame

from inav_msp import MSPApi
from inav_msp.lib import InavMSP

MIN_US = 910
MAX_US = 2099
MID_US = 1500
DEFAULT_RATE_HZ = 50.0
DEFAULT_PORT = "/dev/ttyACM0"
DEFAULT_BAUDRATE = 115200
DEFAULT_READ_TIMEOUT_MS = 10.0
DEFAULT_WRITE_TIMEOUT_MS = 50.0
CHANNEL_PRINT_HZ = 5.0
AXIS_COUNT = 4
BUTTON_COUNT = 12


def axis_to_us(value: float) -> int:
    clamped = max(-1.0, min(1.0, float(value)))
    span = MAX_US - MIN_US
    return int(MIN_US + ((clamped + 1.0) * 0.5 * span))


def button_to_us(pressed: int) -> int:
    return MAX_US if pressed else MIN_US


def get_joystick_state(joystick, min_axes):
    pygame.event.pump()
    axes = [round(joystick.get_axis(i), 3) for i in range(joystick.get_numaxes())]
    buttons = [joystick.get_button(i) for i in range(joystick.get_numbuttons())]
    while len(axes) < min_axes:
        axes.append(0.0)
    while len(buttons) < BUTTON_COUNT:
        buttons.append(0)
    return axes[:min_axes], buttons[:BUTTON_COUNT]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", default=DEFAULT_PORT, help="Serial port for MSP")
    parser.add_argument("--baudrate", type=int, default=DEFAULT_BAUDRATE, help="Serial baudrate")
    parser.add_argument("--tcp", help="TCP endpoint for MSP (HOST:PORT), overrides serial")
    parser.add_argument("--rate", type=float, default=DEFAULT_RATE_HZ, help="Send rate in Hz")
    parser.add_argument("--read-timeout-ms", type=float, default=DEFAULT_READ_TIMEOUT_MS, help="MSP read timeout in milliseconds")
    parser.add_argument("--write-timeout-ms", type=float, default=DEFAULT_WRITE_TIMEOUT_MS, help="MSP write timeout in milliseconds")
    parser.add_argument("--joystick-index", type=int, default=0, help="Joystick index reported by pygame")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    period = 1.0 / args.rate
    next_send = time.monotonic()

    pygame.init()
    pygame.joystick.init()

    if pygame.joystick.get_count() == 0:
        raise RuntimeError("No joystick detected.")
    if args.joystick_index >= pygame.joystick.get_count():
        raise RuntimeError(f"Joystick index {args.joystick_index} not available.")

    joystick = pygame.joystick.Joystick(args.joystick_index)
    joystick.init()
    js_name = joystick.get_name()
    print(f"joystick_name={js_name} joystick_index={args.joystick_index}")

    is_tx12 = ("tx12" in js_name.lower()) or ("radiomaster" in js_name.lower())
    required_axes = 8 if is_tx12 else AXIS_COUNT

    channels = [MID_US] * 16
    channels[2] = MIN_US
    channels[4] = MIN_US
    armed = False
    last_arm_btn = 0
    btn_latched = [False] * 4
    last_btns = [0] * 4

    dbg_t = 0.0
    connection = f"tcp_endpoint={args.tcp}" if args.tcp else f"serial_port={args.port} baudrate={args.baudrate}"
    print(f"msp_connection={connection} rate_hz={args.rate} tx12_layout={is_tx12} read_timeout_ms={args.read_timeout_ms} write_timeout_ms={args.write_timeout_ms}")

    if args.tcp:
        api = MSPApi(
            tcp_endpoint=args.tcp,
            read_timeout_ms=args.read_timeout_ms,
            write_timeout_ms=args.write_timeout_ms,
        )
    else:
        api = MSPApi(
            port=args.port,
            baudrate=args.baudrate,
            read_timeout_ms=args.read_timeout_ms,
            write_timeout_ms=args.write_timeout_ms,
        )
    with api:
        try:
            while True:
                sleep_time = next_send - time.monotonic()
                if sleep_time > 0:
                    time.sleep(sleep_time)
                else:
                    time.sleep(0.001)
                    next_send = time.monotonic()
                next_send += period

                axes, buttons = get_joystick_state(joystick, required_axes)

                if is_tx12:
                    channels[0] = axis_to_us(axes[0])
                    channels[1] = axis_to_us(axes[1])
                    channels[2] = axis_to_us(axes[2])
                    channels[3] = axis_to_us(axes[3])
                    for i in range(4):
                        channels[4 + i] = axis_to_us(axes[4 + i])
                    for i in range(4):
                        channels[8 + i] = button_to_us(buttons[i])
                else:
                    channels[0] = axis_to_us(axes[2])
                    channels[1] = axis_to_us(axes[3])
                    channels[2] = axis_to_us(-axes[1])
                    channels[3] = axis_to_us(axes[0])

                    arm_btn = buttons[9]
                    if arm_btn and not last_arm_btn:
                        armed = not armed
                    last_arm_btn = arm_btn
                    channels[4] = MAX_US if armed else MIN_US

                    for i in range(4):
                        if buttons[i] and not last_btns[i]:
                            btn_latched[i] = not btn_latched[i]
                        last_btns[i] = buttons[i]
                        channels[5 + i] = MAX_US if btn_latched[i] else MIN_US

                payload = struct.pack("<16H", *channels)
                api._serial.send(int(InavMSP.MSP_SET_RAW_RC), payload)

                now = time.time()
                if now - dbg_t >= 1.0 / CHANNEL_PRINT_HZ:
                    print(f"channels={channels}")
                    dbg_t = now
        finally:
            pygame.quit()


if __name__ == "__main__":
    main()
