"""
Usage:
    python example_api.py --tcp 127.0.0.1:5760
"""

from __future__ import annotations

import argparse
from functools import partial

from inav_msp.lib import InavEnums, InavMSP
from inav_msp.msp_api import MSPApi
from inav_msp.msp_serial import MSPUnsupportedError
from inav_msp.utils import format_nested_dict
from inav_msp.lib.inav_version import MAJOR as INAV_VERSION_MAJOR
import time
from typing import Any, Dict

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="MSP API demo")
    parser.add_argument("--port", default="/dev/ttyACM0", help="Serial device path (ignored if --tcp or --udp is used)")
    parser.add_argument("--baudrate", type=int, default=115200, help="Serial baud rate")
    parser.add_argument("--tcp", metavar="HOST:PORT", help="Connect using TCP socket instead of serial, e.g. localhost:5760")
    parser.add_argument("--udp", metavar="HOST:PORT", help="Connect using UDP socket instead of serial, e.g. localhost:27072")
    parser.add_argument("--force-mspv2", action="store_true", help="Force all requests to MSPv2 framing")
    parser.add_argument("--read-timeout", type=float, default=10.0, help="Read timeout in milliseconds")
    parser.add_argument("--write-timeout", type=float, default=250.0, help="Write timeout in milliseconds")
    return parser.parse_args()


def show_info(info: Dict[str, Any]) -> None:
    parts = []
    latency = info.get("latency_ms")
    if isinstance(latency, (int, float)):
        parts.append(f"latency={latency:.2f}ms")
    if info.get("cached") is not None:
        parts.append(f"cached={info.get('cached')}")
    cache_age = info.get("cache_age_ms")
    if isinstance(cache_age, (int, float)):
        parts.append(f"cache_age={cache_age:.0f}ms")
    attempt = info.get("attempt")
    if attempt is not None:
        parts.append(f"attempt={attempt}")
    transport = info.get("transport")
    if transport:
        parts.append(f"transport={transport}")
    if parts:
        print(f", ".join(parts))


def main() -> None:
    args = parse_args()
    if args.tcp and args.udp:
        raise ValueError("Provide only one of --tcp or --udp")
    pp = partial(format_nested_dict, start_indent=1)
    port = None if (args.tcp or args.udp) else args.port
    with MSPApi(
        port=port,
        baudrate=args.baudrate,
        read_timeout_ms=args.read_timeout,
        write_timeout_ms=args.write_timeout,
        tcp_endpoint=args.tcp,
        udp_endpoint=args.udp,
        force_msp_v2=args.force_mspv2,
    ) as api:
        print()
        api_version = api.get_api_version()
        print("MSP API version:\n" + pp(api_version))
        show_info(api.info)

        print()
        fc_variant = api.get_fc_variant()
        print("Flight controller variant:\n" + pp(fc_variant))
        show_info(api.info)

        print()
        board_info = api.get_board_info()
        print("Board info:\n" + pp(board_info))
        show_info(api.info)

        print()
        sensor_cfg = api.get_sensor_config()
        print("Sensor configuration:\n" + pp(sensor_cfg))
        show_info(api.info)

        print()
        mode_ranges = api.get_mode_ranges()
        print("Mode ranges:\n" + pp(mode_ranges))
        show_info(api.info)

        print()
        status = api.get_inav_status()
        print("INAV status:\n" + pp(status))
        show_info(api.info)

        print()
        analog = api.get_inav_analog()
        print("Analog readings:\n" + pp(analog))
        show_info(api.info)

        print()
        rx_config = api.get_rx_config()
        print("RX config:\n" + pp(rx_config))
        show_info(api.info)

        print()
        link_stats = api.get_link_stats()
        print("Link stats:\n" + pp(link_stats))
        show_info(api.info)

        try:
            print()
            dronecan_nodes = api.get_dronecan_nodes()
            print("DroneCAN nodes:\n" + pp(dronecan_nodes))
            show_info(api.info)
            for dronecan_node in dronecan_nodes:
                node_info = api.get_dronecan_node_info(dronecan_node["nodeID"])
                print(f"DroneCAN node {dronecan_node['nodeID']} info:\n" + pp(node_info))
                show_info(api.info)
        except MSPUnsupportedError as exc:
            print(f"DroneCAN node queries unavailable in this firmware build: {exc}")

        print()
        boot_time_ns = api.get_timesync_ns()
        print(f"FC boot time: {boot_time_ns}ns")
        show_info(api.info)

        print()
        disarm_ack = api.set_armed(False)
        print("ARM_DISARM disarm ack:\n" + pp(disarm_ack))
        show_info(api.info)

        print()
        logic_condition = api.get_logic_condition(0)
        print("Logic condition[0]:\n" + pp(logic_condition))
        show_info(api.info)

        print()
        try:
            api._request_raw(InavMSP.MSP_SET_ACC_TRIM, b"")
        except Exception as exc:
            print(f"Deprecated/invalid MSP example (MSP_SET_ACC_TRIM): {exc}")

        try:
            api._request_raw(InavMSP.MSP2_INAV_GLOBAL_FUNCTIONS, b"")
        except Exception as exc:
            print(f"Unimplemented MSP example (MSP2_INAV_GLOBAL_FUNCTIONS): {exc}")

        print()
        rx_map = api.get_rx_map()
        print("RX map:\n" + pp(rx_map))
        show_info(api.info)

        print()
        attitude = api.get_attitude()
        print("Attitude:\n" + pp(attitude))
        show_info(api.info)

        print()
        altitude = api.get_altitude()
        print("Altitude:\n" + pp(altitude))
        show_info(api.info)

        print()
        imu = api.get_imu()
        print("IMU summary:\n" + pp(imu))
        show_info(api.info)

        print()
        rc_channels = api.get_rc_channels()
        print("RC channels:\n" + pp(rc_channels[:6] if rc_channels else []))
        show_info(api.info)

        print()
        target_channels = rc_channels[:] if rc_channels else [1500, 1500, 1500, 1500]
        ack = api.set_rc_channels(target_channels)
        print("SET_RAW_RC ack:\n" + pp(ack))
        show_info(api.info)

        print()
        aux_rc_ack = api.set_aux_rc(12, [1600, 1400], resolution_bits=16)
        print("SET_AUX_RC CH13-CH14 ack:\n" + pp(aux_rc_ack))
        show_info(api.info)
        aux_rc_channels = api.get_rc_channels()
        print("RC channels CH13-CH14 after SET_AUX_RC:\n" + pp(aux_rc_channels[12:14]))
        show_info(api.info)

        print()
        bat_cfg = api.get_battery_config()
        print("Battery config:\n" + pp(bat_cfg))
        show_info(api.info)

        print()
        gps_stats = api.get_gps_statistics()
        print("GPS statistics:\n" + pp(gps_stats))
        show_info(api.info)

        print()
        wp_info = api.get_waypoint_info()
        print("Waypoint info:\n" + pp(wp_info))
        show_info(api.info)

        print()
        raw_gps = api.get_raw_gps()
        print("Raw GPS:\n" + pp(raw_gps))
        show_info(api.info)

        print()
        set_wp_ack = api.set_waypoint(
            waypointIndex=1,
            action=InavEnums.navWaypointActions_e.NAV_WP_ACTION_WAYPOINT,
            latitude=1.234,
            longitude=2.345,
            altitude=15.0,
            param1 = 0,
            param2 = 0,
            param3 = 0,
            flag = 0
        )
        print("SET_WP ack:\n" + pp(set_wp_ack))
        show_info(api.info)

        print()
        waypoint = api.get_waypoint(1)
        print("Waypoint:\n" + pp(waypoint))
        show_info(api.info)

        print()
        nav_status = api.get_nav_status()
        print("Navigation status:\n" + pp(nav_status))
        show_info(api.info)

        print()
        try:
            waypoint_index_ack = api.set_waypoint_index(0)
            print("SET_WP_INDEX ack:\n" + pp(waypoint_index_ack))
            show_info(api.info)
        except MSPUnsupportedError as exc:
            print(f"SET_WP_INDEX rejected while disarmed/outside waypoint mode: {exc}")

        print()
        try:
            cruise_heading_ack = api.set_cruise_heading(90.0)
            print("SET_CRUISE_HEADING ack:\n" + pp(cruise_heading_ack))
            show_info(api.info)
        except MSPUnsupportedError as exc:
            print(f"SET_CRUISE_HEADING rejected while disarmed/outside Course Hold mode: {exc}")

        print()
        try:
            rth_ack = api.activate_rth()
            print("ACTIVATE_RTH ack:\n" + pp(rth_ack))
            show_info(api.info)
        except MSPUnsupportedError as exc:
            print(f"ACTIVATE_RTH rejected while disarmed: {exc}")

        print()
        try:
            landing_ack = api.activate_landing()
            print("ACTIVATE_LANDING ack:\n" + pp(landing_ack))
            show_info(api.info)
        except MSPUnsupportedError as exc:
            print(f"ACTIVATE_LANDING rejected while disarmed: {exc}")

        print()
        try:
            ack_heading = api.set_heading(heading_deg=90)
            print("SET_HEAD ack:\n" + pp(ack_heading))
            show_info(api.info)
        except Exception as exc:
            print(f"SET_HEAD failed: {exc}")

        print()
        active_modes = api.get_active_modes()
        print("Active modes:\n" + pp(active_modes))
        show_info(api.info)

        if INAV_VERSION_MAJOR >= 10:
            print()
            local_target = api.get_local_target()
            print("Local target (NEU offsets, cm):\n" + pp(local_target))
            show_info(api.info)

            print()
            try:
                ack_local = api.set_local_target(x_cm=100.0, y_cm=0.0, z_cm=0.0)
                print("SET_LOCAL_TARGET ack:\n" + pp(ack_local))
                show_info(api.info)
            except Exception as exc:
                print(f"SET_LOCAL_TARGET failed (expected if GCSNAV/offboard not active): {exc}")

            print()
            try:
                ack_alt = api.set_altitude_target(
                    altitude_m=30.0,
                    #altitude_datum=InavEnums.geoAltitudeDatumFlag_e.NAV_WP_TAKEOFF_DATUM, # Takeoff by default
                )
                print("SET_ALT_TARGET ack:\n" + pp(ack_alt))
                show_info(api.info)
            except Exception as exc:
                print(f"SET_ALT_TARGET failed (expected if GCSNAV/offboard not active): {exc}")

            print()
            nav_target = api.get_nav_target()
            print("NAV target (global):\n" + pp(nav_target))
            show_info(api.info)

            print()
            try:
                ack_global = api.set_global_target(
                    latitude_deg=raw_gps["latitude"],
                    longitude_deg=raw_gps["longitude"],
                    altitude_m=None,  # keep current altitude
                    #altitude_datum=InavEnums.geoAltitudeDatumFlag_e.NAV_WP_TAKEOFF_DATUM, # Takeoff by default
                )
                print("SET_GLOBAL_TARGET ack:\n" + pp(ack_global))
                show_info(api.info)
            except Exception as exc:
                print(f"SET_GLOBAL_TARGET failed (expected if GCSNAV/offboard not active): {exc}")


if __name__ == "__main__": 
    main()
