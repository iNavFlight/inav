# GPS Fix estimation (dead reconing, RTH without GPS) for fixed wing

Video demonstration

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/wzvgRpXCS4U/0.jpg)](https://www.youtube.com/watch?v=wzvgRpXCS4U)

There is possibility to allow plane to estimate it's position when GPS fix is lost.
The main purpose is RTH without GPS.
It works on fixed wing only.

Plane should have the following sensors:
- acceleromenter, gyroscope
- barometer
- GPS
- magnethometer
- pitot (optional)

By befault, all navigation modes are disabled when GPS fix is lost. If RC signal is lost also, plane will not be able to enable RTH. Plane will switch to LANDING instead. When flying above inreachable spaces, plane will be lost.

GPS fix estimation allows to recover plane using magnetometer and baromener only.

Note, that GPS fix estimation is not a solution for navigation without GPS. Without GPS fix, position error accumulates quickly. But it is acceptable for RTH.

# How it works ?

In normal situation, plane is receiving it's position from GPS sensor. This way it is able to hold course, RTH or navigate by waypoints.


Without GPS fix, plane has nose heading from magnetometer only.

To navigate without GPS fix, we make the following assumptions:
- plane is flying in the direction where nose is pointing
- plane is flying with constant speed, specified in settings

It is posible to roughtly estimate position using these assumptions. To increase heading accuracy, plane will use information about wind direction and speed, estimated before GPS fix was lost. To increase speed estimation accuracy, plane will use pitot tube (if available).

From estimated heading direction and speed, plane is able to **roughtly** estimate it's position.

It is assumed, that plane will fly in roughtly estimated direction to home position untill either GPS fix or RC signal is recovered.

*Plane has to aquire GPS fix and store home position before takeoff. Estimation completely without GPS fix will not work*.

# Settings

GPS Fix estimation is enabled with CLI command:

```set inav_allow_gps_fix_estimation=ON```

Also you have to specify criuse airspeed of the plane.

The get cruise airspeed, make test flight. Enable ground speed display on OSD. Flight in CRUISE mode in two opposite directions. Take average speed.

Cruise airspeed is specified in m/s.

To convert km/h to m/s, multiply by 27.77.


Example: 100 km/h = 100 * 27.77 = 2777 m/s

```set fw_reference_airspeed=2777```

*It is important, that plane fly with specified speed in CRUISE mode. If you have set option "Icrease cruise speed with throttle"  - do not use it without GPS Fix.*

*If pitot is available, pitot sensor data will be used instead of constant.*

*Note related command: to continue mission without RC signal, see command ```set failsafe_mission=OFF```.*

**After entering CLI command, make sure that settings are saved:**

```save```

# Disabling GPS sensor from RC controller

![](Screenshots/gps_off_box.png) 

For testing purpoces, it is now possible to disable GPS sensor from RC controller:

*GPS can be disabled only after 1) initial GPS fix is acquired 2) in ARMED mode.*

# Is it possible to implement this for multirotor ?

There are some ideas, but there is no solution now. We can not make assumptions with multirotor which we can make with a fixed wing.


# Links

INAV HITL  https://github.com/RomanLut/INAV-X-Plane-HITL
