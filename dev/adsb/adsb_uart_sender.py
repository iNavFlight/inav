import json
import time
import argparse
import serial
from pymavlink.dialects.v20 import common as mavlink2

def load_aircraft(json_file):
    with open(json_file, "r") as f:
        return json.load(f)

def create_mavlink(serial_port):
    mav = mavlink2.MAVLink(serial_port)
    mav.srcSystem = 1
    mav.srcComponent = mavlink2.MAV_COMP_ID_ADSB
    return mav

def send_heartbeat(mav):
    mav.heartbeat_send(
        mavlink2.MAV_TYPE_ADSB,
        mavlink2.MAV_AUTOPILOT_INVALID,
        0,
        0,
        0
    )

def send_adsb(mav, aircraft):
    for ac in aircraft:
        icao = int(ac["icao_address"])
        lat = int(ac["lat"] * 1e7)
        lon = int(ac["lon"] * 1e7)
        alt_mm = int(ac["altitude"] * 1000)
        heading_cdeg = int(ac["heading"] * 100)
        hor_vel_cms = int(ac["hor_velocity"] * 100)
        ver_vel_cms = int(ac["ver_velocity"] * 100)
        callsign = ac["callsign"].encode("ascii").ljust(9, b'\x00')

        msg = mavlink2.MAVLink_adsb_vehicle_message(
            ICAO_address=icao,
            lat=lat,
            lon=lon,
            altitude_type=0,
            altitude=alt_mm,
            heading=heading_cdeg,
            hor_velocity=hor_vel_cms,
            ver_velocity=ver_vel_cms,
            callsign=callsign,
            emitter_type=0,
            tslc=1,
            flags=3,
            squawk=0
        )

        mav.send(msg)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("com_port")
    parser.add_argument("json_file")
    parser.add_argument("--baud", default=115200, type=int)
    parser.add_argument("--rate", default=1.0, type=float)
    args = parser.parse_args()

    ser = serial.Serial(args.com_port, args.baud)
    mav = create_mavlink(ser)

    aircraft = load_aircraft(args.json_file)

    period = 1.0 / args.rate
    last_hb = 0

    while True:
        now = time.time()

        if now - last_hb >= 1.0:
            send_heartbeat(mav)
            last_hb = now

        send_adsb(mav, aircraft)
        time.sleep(period)

if __name__ == "__main__":
    main()
