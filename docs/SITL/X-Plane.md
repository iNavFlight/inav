# X-Plane

Tested on X-Plane 11, 12 should(!) work but not tested.

X-Plane is not a model flight simulator, but is based on real world data and is therefore suitable for GPS missions with waypoints.

## Aircraft
It is recommended to use the "AR Wing" of the INAV HITL project: https://github.com/RomanLut/INAV-X-Plane-HITL

## General settings
In Settings / Network select "Accept incoming connections".
The port can be found under "UDP PORTS", "Port we receive on". If no connection is established, the port can be changed.
You may want to incease the "Flight model per frame" value under "General"

## Joystick
In the settings, calibrate the joystick, set it up and assign the axes as follows:

| INAV | X-Plane |
|------|---------|
| Roll | Roll |
| Pitch | Pitch |
| Throttle | Cowl Flap 1 |
| Yaw | Yaw |
| Channel 5 | Cowl Flap 2 |
| Channel 6 | Cowl Flap 3 |
| Channel 7 | Cowl Flap 4 |
| Channel 8 | Cowl Flap 5 |

Reverse axis in X-Plane if necessary.

## Channelmap:
The assignment of the "virtual receiver" is fixed:
1 - Throttle
2 - Roll
3 - Pitch
4 - Yaw

The internal mixer (e.g. for flying wings) cannot be deactivated without further ado, therefore always select "Aircraft with tail" in INAV.
For the standard Aircraft preset the channelmap is:
```--chanmap=M01-01,S01-03,S03-02,S04-04```

## Other applications

[fl2sitl](https://github.com/stronnag/bbl2kml/wiki/fl2sitl) is an open source application to replay an INAV Blackbox log through the INAV SITL via `blackbox_decode`. The output may be visualised in any MSP capable application, such as the INAV Configurator or [mwp](https://github.com/stronnag/mwptools). fl2sitl uses the X-plane protocol.
