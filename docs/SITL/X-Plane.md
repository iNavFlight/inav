# X-Plane 

Tested on X-Plane 11, 12 should(!) work but not tested.

X-Plane is not a model flight simulator, but is based on real world data and is therefore suitable 
GPS missions with waypoints.

## General settings
In Settings / Network select "Accept incoming connections".
The port can be found under "UDP PORTS", "Port we receive on". If no connection is established, the port can be changed.
You may want to incease the "Flight model per frame" value under "General"

## Channelmap:
The assignment of the "virtual receiver" is fixed: 
1 - Throttle
2 - Roll
3 - Pitch
4 - Yaw

The internal mixer (e.g. for wings only) cannot be deactivated without further ado, therefore always select "Aircraft with tail" in INAV. 
For the standard Aircraft preset the channelmap is:
```--chanmap=M01-01,S01-03,S03-02,S04-04```
