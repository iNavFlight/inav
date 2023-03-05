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
| Throttle | Throttle |
| Yaw | Yaw | 
| Channel 5 | Prop | 
| Channel 6 | Mixture | 
| Channel 7 | Collective | 
| Channel 8 | Thrust vector | 

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
