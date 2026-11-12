"""
Basic inav_msp usage examples.

This script demonstrates the fundamental operations:
- Connecting to a flight controller
- Reading basic information
- Reading sensor data
- Error handling
"""

import argparse
from inav_msp import MSPApi


def parse_args():
    parser = argparse.ArgumentParser(description="Basic inav_msp usage examples")
    parser.add_argument("--port", default="/dev/ttyACM0", help="Serial port")
    parser.add_argument("--baudrate", type=int, default=115200, help="Baudrate")
    parser.add_argument("--tcp", help="TCP endpoint (e.g., localhost:9000)")
    return parser.parse_args()


def main():
    args = parse_args()

    # Determine connection type
    if args.tcp:
        print(f"Connecting via TCP: {args.tcp}")
        api = MSPApi(tcp_endpoint=args.tcp)
    else:
        print(f"Connecting via serial: {args.port} @ {args.baudrate}")
        api = MSPApi(port=args.port, baudrate=args.baudrate)

    # Use context manager to auto-open/close connection
    with api:
        print("\n" + "="*70)
        print("FLIGHT CONTROLLER INFORMATION")
        print("="*70)

        # Get API version
        version = api.get_api_version()
        print(f"\nAPI Version: {version['apiVersionMajor']}.{version['apiVersionMinor']}")
        print(f"MSP Protocol: {version['mspProtocolVersion']}")
        print(f"Request latency: {api.info['latency_ms']:.1f}ms")

        # Get FC variant
        variant = api.get_fc_variant()
        print(f"\nFC Variant: {variant['fcVariantIdentifier']}")

        # Get board info
        board = api.get_board_info()
        print(f"Board: {board['boardIdentifier']}")
        print(f"Target: {board['targetName']}")
        print(f"Hardware Rev: {board['hardwareRevision']}")

        print("\n" + "="*70)
        print("SENSOR DATA")
        print("="*70)

        # Get attitude
        attitude = api.get_attitude()
        print(f"\nAttitude:")
        print(f"  Roll:    {attitude['roll']:7.1f}°")
        print(f"  Pitch:   {attitude['pitch']:7.1f}°")
        print(f"  Heading: {attitude['yaw']:7.1f}°")

        # Get altitude
        altitude = api.get_altitude()
        print(f"\nAltitude:")
        print(f"  Estimated: {altitude['estimatedAltitude']:6.1f}cm")
        print(f"  Variometer: {altitude['variometer']:6.1f}cm/s")

        # Get battery/power
        analog = api.get_inav_analog()
        print(f"\nPower:")
        print(f"  Battery:  {analog['vbat']:5.2f}V")  # Already converted to volts
        print(f"  Current:  {analog['amperage']:5.2f}A")  # Already converted to amps
        print(f"  Consumed: {analog['mAhDrawn']:5.0f}mAh")

        # Get GPS (may not have fix)
        gps = api.get_raw_gps()
        print(f"\nGPS:")
        if gps['fixType'] > 0:
            print(f"  Fix Type:    {gps['fixType']}")
            print(f"  Satellites:  {gps['numSat']}")
            print(f"  Position:    {gps['latitude']}, {gps['longitude']}")
            print(f"  Altitude:    {gps['altitude']}m")
            print(f"  Speed:       {gps['speed']} cm/s")
            print(f"  Ground Course: {gps['groundCourse']}°")
        else:
            print(f"  No GPS fix (fixType = {gps['fixType']})")

        print("\n" + "="*70)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nInterrupted by user")
    except Exception as e:
        print(f"\n\nError: {e}")
        import traceback
        traceback.print_exc()
