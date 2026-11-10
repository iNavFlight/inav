#!/usr/bin/env python3
"""Bench smoke test for a MassZero thermal camera on a local serial port.

Sends a handful of read commands and one write command directly to the
camera and prints the raw replies. Use it to confirm the camera and the
wiring before involving the flight controller. thermal_bridge.py is the
companion that connects the camera to a SITL instance.
"""

import argparse
import sys
import time

import serial  # pyserial

START_BYTE = 0xF0
END_BYTE = 0xFF
DEVICE_ADDRESS = 0x36
FLAG_WRITE = 0x00
FLAG_READ = 0x01


def hexdump(data):
    return ' '.join('%02X' % b for b in data)


def send_command(ser, class_cmd, subclass_cmd, flags=FLAG_READ, data=None):
    """Send one command packet and print whatever comes back."""
    if data is None:
        data = []

    packet = bytearray()
    packet.append(START_BYTE)
    packet.append(len(data) + 4)
    packet.append(DEVICE_ADDRESS)
    packet.append(class_cmd)
    packet.append(subclass_cmd)
    packet.append(flags)
    packet.extend(data)
    checksum = DEVICE_ADDRESS + class_cmd + subclass_cmd + flags + sum(data)
    packet.append(checksum & 0xFF)
    packet.append(END_BYTE)

    print("Sending:  %s" % hexdump(packet))
    ser.write(packet)

    time.sleep(0.1)
    response = ser.read(256)
    if response:
        print("Response: %s" % hexdump(response))
        return response
    print("No response")
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('port', help="serial port the camera is on, for example COM13 or /dev/ttyUSB0")
    parser.add_argument('--baud', type=int, default=115200, help="baud rate (default 115200)")
    args = parser.parse_args()

    try:
        ser = serial.Serial(args.port, args.baud, timeout=1.0)
    except serial.SerialException as e:
        print("Error: %s" % e, file=sys.stderr)
        return 1
    print("Opened %s at %d baud" % (args.port, args.baud))

    with ser:
        # Drop anything already buffered so the first reply lines up.
        ser.read_all()

        print("\n1. Reading device model...")
        send_command(ser, 0x74, 0x02)

        print("\n2. Reading FPGA version...")
        send_command(ser, 0x74, 0x03)

        print("\n3. Reading software version...")
        send_command(ser, 0x74, 0x05)

        print("\n4. Reading initialization status...")
        send_command(ser, 0x7C, 0x14)

        print("\n5. Setting brightness to 50...")
        send_command(ser, 0x78, 0x02, flags=FLAG_WRITE, data=[50])

    return 0


if __name__ == "__main__":
    sys.exit(main())
