from __future__ import annotations

import argparse

from inav_msp.lib import InavEnums
from inav_msp.msp_api import MSPApi
import time
from statistics import mean

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="MSP API demo")
    parser.add_argument("--port", default="/dev/ttyACM0", help="Serial device path (ignored if --tcp or --udp is used)")
    parser.add_argument("--baudrate", type=int, default=115200, help="Serial baud rate")
    parser.add_argument("--tcp", metavar="HOST:PORT", help="Connect using TCP socket instead of serial, e.g. localhost:5760")
    parser.add_argument("--udp", metavar="HOST:PORT", help="Connect using UDP socket instead of serial, e.g. localhost:27072")
    parser.add_argument("--force-mspv2", action="store_true", help="Force MSP v2 framing")
    parser.add_argument("--read-timeout", type=float, default=1, help="Read timeout in miliseconds")
    parser.add_argument("--write-timeout", type=float, default=1, help="Write timeout in miliseconds")
    return parser.parse_args()


def data(result):
    if isinstance(result, tuple) and len(result) == 2:
        return result[1]
    return result


def main() -> None:
    args = parse_args()
    if args.tcp and args.udp:
        raise ValueError("Provide only one of --tcp or --udp")
    port = None if (args.tcp or args.udp) else args.port
    with MSPApi(
        port=port,
        baudrate=args.baudrate,
        read_timeout=args.read_timeout,
        write_timeout=args.write_timeout,
        tcp_endpoint=args.tcp,
        udp_endpoint=args.udp,
        force_msp_v2=args.force_mspv2,
    ) as api:
        #print()
        #print("MSP API version:", api.get_api_version())

        #print()
        #print("Flight controller variant:", api.get_fc_variant())

        #print()
        #print("Board info:", api.get_board_info())

        #print()
        #print("Sensor configuration:", api.get_sensor_config())

        print()
        print("Mode ranges:")
        for entry in data(api.get_mode_ranges()):
            print(entry)

        print()
        status = data(api.get_inav_status())
        print("INAV status:", status)

        print()
        analog = data(api.get_inav_analog())
        print("Analog readings:", analog)

        print()
        print("RX config:", data(api.get_rx_config()))

        #print()
        #print("Logic conditions:")
        #for condition in api.get_logic_conditions():
        #    if condition["enabled"]:
        #        print(condition)

        print()
        print("Attitude:", data(api.get_attitude()))

        print()
        print("Altitude:", data(api.get_altitude()))

        print()
        print("IMU summary:", data(api.get_imu()))

        print()
        rc_channels = data(api.get_rc_channels())
        print("RC channels:", rc_channels)
        target_channels = rc_channels[:] if rc_channels else [1500, 1500, 1500, 1500]
        ack = data(api.set_rc_channels(target_channels))
        print("SET_RAW_RC ack:", ack)

        print()
        print("Battery config:", api.get_battery_config())

        print()
        print("GPS statistics:", api.get_gps_statistics())

        #print()
        #print("Waypoint info:", api.get_waypoint_info())

        print()
        print("Raw GPS:", api.get_raw_gps())

        """print()
        set_wp_ack = api.set_waypoint(
            waypointIndex=1,
            action=InavEnums.navWaypointActions_e.NAV_WP_ACTION_WAYPOINT,
            latitude=1.234,
            longitude=2.345,
            altitude=15.0,
        )
        print("SET_WP ack:", set_wp_ack)
        """

        print()
        print("Waypoint:", api.get_waypoint(1))

        print()
        print("Navigation status:", api.get_nav_status())

        chorder = [ # ?
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
        nchan = len(chorder)
        defval = 1500
        board_channels = [900] * nchan  # RC as it is on the flight controller
        channels = [900] * nchan # RC as we modify them
        pwm_range = [900, 2100]
        pwm_midpoint = mean(pwm_range)
        channels[0] = defval
        channels[1] = defval
        channels[3] = defval
        pt = time.time()
        starttime = time.time()
        stage = 0

        while 1:
            ack = api.set_rc_channels(channels)
            time.sleep(0.1)
            t = time.time()-starttime

            if stage==0:
                if t<3:
                    pass
                elif t>=3 and t<=5:
                    channels[4] = 2090
                elif t>=5 and t<=10:
                    channels[2] = 2090
                elif t>=10 and t<=13:
                    channels[1] = 1000
                elif t>=13:
                    stage+=1

            if stage==1:
                channels[1] = pwm_midpoint
                channels[6] = pwm_midpoint

            if time.time()-pt > 1:
                inavstatus = api.get_inav_status()
                rc_channels = api.get_rc_channels()[:4]
                analog = api.get_inav_analog()['vbat']
                gps = api.get_raw_gps()
                print()
                print("Altitude:", api.get_altitude())
                print("RC channels:", rc_channels)
                print("Modes:",inavstatus['activeModes'])
                print("VBATT:", analog)
                print("GPS:", gps)
                pt = time.time()



if __name__ == "__main__":
    main()
