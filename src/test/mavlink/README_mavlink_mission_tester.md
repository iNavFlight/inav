# MAVLink mission tester

This is a non-interactive mission upload/regression harness for MAVLink mission translation.

It uses:

- `pymavlink` for raw MAVLink mission upload through MAVProxy UDP `14550`
- `mspapi2` for MSP mission readback through SITL UART1 TCP `5760`

It does not start SITL or MAVProxy by default.

For a self-contained run that starts SITL, starts MAVProxy, runs the mission test, prints log errors, and shuts everything down:

```sh
src/test/mavlink/run_mavlink_mission_tester.sh
```

## Manual setup

From `inav`:

```sh
./cmake/build_SITL/inav_9.1.0_SITL --serialport=/dev/ttyUSB0 --serialuart=3 --baudrate=460800 --path="../mydev/branch/mavlink_multiport2/eeprom.bin" --chanmap=M01-01,S02-02,S01-03,S04-04
```

From `inav/src/test/mavlink/results`:

```sh
mavproxy.py --master=tcp:127.0.0.1:5763 --force-connected --nowait --daemon --out=udp:127.0.0.1:14550
```

Then from the INAV repo root:

```sh
conda run -n drone python src/test/mavlink/mavlink_mission_tester.py
```

List cases without connecting:

```sh
conda run -n drone python src/test/mavlink/mavlink_mission_tester.py --list-cases
```

Run one case:

```sh
conda run -n drone python src/test/mavlink/mavlink_mission_tester.py --case qgc_planned_home_waypoints_land
```

The JSON report is written to:

```text
src/test/mavlink/results/mission_tester_report.json
```

The default mission coordinates are centered near Sneedville, Tennessee (`36.530440`, `-83.216383`).

## Current cases

- QGC planned home + waypoints + land
- waypoint hold time plus `MAV_CMD_CONDITION_DELAY`
- `MAV_CMD_DO_CHANGE_SPEED` applying to following legs
- altitude modifier commands preserving INAV P3 bits
- `MAV_CMD_DO_JUMP` remap after QGC planned-home skip
- unsupported `MAV_CMD_NAV_TAKEOFF` rejection without MSP verification
