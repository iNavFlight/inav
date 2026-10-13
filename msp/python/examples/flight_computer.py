#!/usr/bin/env python3
"""
Flight Computer Example

Demonstrates using inav_msp on a companion computer (Raspberry Pi) to:
- Monitor telemetry
- Make autonomous decisions
- Control navigation via waypoints
- Handle errors gracefully

This is a complete example suitable for running on a flight computer
connected to an INAV flight controller via UART.

Hardware Setup:
- Raspberry Pi connected to FC UART port
- /dev/ttyAMA0 at 115200 baud (configure in INAV)

Usage:
    python3 flight_computer.py --port /dev/ttyAMA0
"""

import argparse
import time
import logging
from datetime import datetime
from inav_msp import MSPApi
from inav_msp.lib import InavEnums
from functools import partial
from inav_msp.utils import format_nested_dict


# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    handlers=[
        logging.FileHandler('flight_computer.log'),
        logging.StreamHandler()
    ]
)


class FlightComputer:
    """
    Flight computer that monitors telemetry and can control navigation.
    """

    def __init__(self, port="/dev/ttyAMA0", baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.api = None

        # Thresholds (adjust for your aircraft)
        self.LOW_BATTERY_VOLTAGE = 10.5  # Volts
        self.CRITICAL_BATTERY_VOLTAGE = 10.0
        self.MAX_ALTITUDE = 120  # meters
        self.MIN_GPS_SATELLITES = 6

        # State tracking
        self.last_telemetry = {}
        self.warnings_issued = set()

    def connect(self):
        """Connect to flight controller."""
        logging.info(f"Connecting to FC on {self.port} @ {self.baudrate}")
        self.api = MSPApi(port=self.port, baudrate=self.baudrate)
        self.api.open()

        # Verify connection
        version = self.api.get_api_version()
        logging.info(f"Connected! API v{version['apiVersionMajor']}.{version['apiVersionMinor']}")

        variant = self.api.get_fc_variant()
        logging.info(f"FC Variant: {variant['fcVariantIdentifier']}")

        board = self.api.get_board_info()
        logging.info(f"Board: {board['boardIdentifier']}")

    def disconnect(self):
        """Disconnect from flight controller."""
        if self.api:
            self.api.close()
            logging.info("Disconnected from FC")

    def read_telemetry(self):
        """
        Read all telemetry data.

        Returns dict with:
            - attitude: roll, pitch, yaw
            - gps: position, fix, satellites
            - battery: voltage, current, mAh
            - nav: navigation state
            - altitude: estimated altitude
        """
        pp = partial(format_nested_dict, start_indent=1)
        try:
            attitude = self.api.get_attitude()
            gps = self.api.get_raw_gps()
            battery = self.api.get_inav_analog()
            nav = self.api.get_nav_status()
            alt = self.api.get_altitude()
            analog = self.api.get_inav_analog()
            print("Analog readings:\n" + pp(analog))
            print()
            rc_channels = self.api.get_rc_channels()
            print("RC channels:\n" + pp(rc_channels[:6] if rc_channels else []))

            telemetry = {
                'timestamp': datetime.now(),
                'attitude': attitude,
                'gps': gps,
                'battery': battery,
                'nav': nav,
                'altitude': alt,
            }

            self.last_telemetry = telemetry
            return telemetry

        except Exception as e:
            logging.error(f"Failed to read telemetry: {e}")
            return None

    def check_safety(self, telemetry):
        """
        Check safety conditions and issue warnings.

        Returns True if safe to continue, False if critical issue.
        """
        if not telemetry:
            return False

        battery = telemetry['battery']
        gps = telemetry['gps']
        alt = telemetry['altitude']

        # Battery checks
        voltage = battery['vbat']  # Already in volts
        if voltage < self.CRITICAL_BATTERY_VOLTAGE:
            logging.critical(f"CRITICAL: Battery voltage {voltage:.2f}V - LAND NOW!")
            return False

        if voltage < self.LOW_BATTERY_VOLTAGE:
            if 'low_battery' not in self.warnings_issued:
                logging.warning(f"LOW BATTERY: {voltage:.2f}V")
                self.warnings_issued.add('low_battery')

        # GPS checks
        if gps['fixType'] == 0:
            if 'no_gps' not in self.warnings_issued:
                logging.warning("NO GPS FIX - Navigation disabled")
                self.warnings_issued.add('no_gps')
            return True  # Not critical, but can't navigate

        if gps['numSat'] < self.MIN_GPS_SATELLITES:
            if 'few_sats' not in self.warnings_issued:
                logging.warning(f"Only {gps['numSat']} satellites")
                self.warnings_issued.add('few_sats')

        # Altitude check
        altitude_m = alt['estimatedAltitude']  # Already in meters
        if altitude_m > self.MAX_ALTITUDE:
            if 'high_alt' not in self.warnings_issued:
                logging.warning(f"Altitude {altitude_m:.1f}m exceeds max {self.MAX_ALTITUDE}m")
                self.warnings_issued.add('high_alt')

        return True

    def log_telemetry(self, telemetry):
        """Log telemetry to console and file."""
        if not telemetry:
            return

        attitude = telemetry['attitude']
        gps = telemetry['gps']
        battery = telemetry['battery']
        nav = telemetry['nav']

        # Navigation state name
        try:
            nav_state = InavEnums.navigationFSMState_t(nav['navState']).name
        except:
            nav_state = f"Unknown({nav['navState']})"

        logging.info(
            f"Attitude: R={attitude['roll']:6.1f}° P={attitude['pitch']:6.1f}° Y={attitude['yaw']:6.1f}° | "
            f"GPS: {gps['fixType']} fix, {gps['numSat']} sats | "
            f"Battery: {battery['vbat']:.2f}V | "
            f"Nav: {nav_state}"
        )

    def set_waypoint_safe(self, index, latitude, longitude, altitude):
        """
        Set a waypoint with safety checks.

        Args:
            index: Waypoint number (0-based)
            latitude: Target latitude
            longitude: Target longitude
            altitude: Target altitude in meters

        Returns:
            True if waypoint set successfully, False otherwise
        """
        # Check GPS fix
        if not self.last_telemetry or self.last_telemetry['gps']['fixType'] == 0:
            logging.error("Cannot set waypoint - no GPS fix")
            return False

        # Check altitude limit
        if altitude > self.MAX_ALTITUDE:
            logging.error(f"Cannot set waypoint - altitude {altitude}m exceeds max {self.MAX_ALTITUDE}m")
            return False

        try:
            self.api.set_waypoint(
                waypointIndex=index,
                action=InavEnums.navWaypointActions_e.NAV_WP_ACTION_WAYPOINT,
                latitude=latitude,
                longitude=longitude,
                altitude=altitude,
                flag=0
            )
            logging.info(f"Waypoint {index} set: {latitude}, {longitude} @ {altitude}m")
            return True

        except Exception as e:
            logging.error(f"Failed to set waypoint: {e}")
            return False

    def run(self, update_interval=0.1):
        """
        Main loop - read telemetry and monitor.

        Args:
            update_interval: Seconds between telemetry reads
        """
        logging.info("Flight computer started")
        logging.info(f"Update interval: {update_interval}s ({1/update_interval:.0f} Hz)")

        loop_count = 0

        try:
            while True:
                # Read telemetry
                telemetry = self.read_telemetry()

                # Check safety
                #if not self.check_safety(telemetry):
                #    logging.critical("CRITICAL SAFETY ISSUE - Stopping autonomous control")
                #    #break

                # Log telemetry every 10 loops (reduce spam)
                if loop_count % 10 == 0:
                    self.log_telemetry(telemetry)

                # Your autonomous logic here
                # Example: simple altitude hold at 50m
                # if telemetry and telemetry['gps']['fixType'] > 0:
                #     current_alt = telemetry['altitude']['estimatedAltitude'] / 100.0  # cm to m
                #     if current_alt < 45:
                #         # Set waypoint above current position
                #         self.set_waypoint_safe(
                #             0,
                #             telemetry['gps']['lat'],
                #             telemetry['gps']['lon'],
                #             50.0
                #         )

                loop_count += 1
                time.sleep(update_interval)

        except KeyboardInterrupt:
            logging.info("Interrupted by user")

        except Exception as e:
            logging.error(f"Error in main loop: {e}")
            import traceback
            traceback.print_exc()

        finally:
            logging.info("Flight computer stopped")


def parse_args():
    parser = argparse.ArgumentParser(description="Flight Computer Example")
    parser.add_argument(
        "--port",
        default="/dev/ttyAMA0",
        help="Serial port (default: /dev/ttyAMA0)"
    )
    parser.add_argument(
        "--baudrate",
        type=int,
        default=115200,
        help="Baudrate (default: 115200)"
    )
    parser.add_argument(
        "--rate",
        type=float,
        default=0.1,
        help="Update interval in seconds (default: 0.1 = 10Hz)"
    )
    return parser.parse_args()


def main():
    args = parse_args()

    # Create flight computer
    fc = FlightComputer(port=args.port, baudrate=args.baudrate)

    # Reconnection loop
    while True:
        try:
            # Connect
            fc.connect()

            # Run main loop
            fc.run(update_interval=args.rate)

        except Exception as e:
            logging.error(f"Connection error: {e}")

        finally:
            fc.disconnect()

        # Wait before reconnecting
        logging.info("Reconnecting in 5 seconds...")
        time.sleep(5)


if __name__ == "__main__":
    main()
