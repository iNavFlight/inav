# GPS Fix estimation (dead reconing, RTH without GPS) for fixed wing

Video demonstration

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/wzvgRpXCS4U/0.jpg)](https://www.youtube.com/watch?v=wzvgRpXCS4U)

There is possibility to allow plane to estimate it's position when GPS fix is lost.
The main purpose is RTH without GPS.
It works for fixed wing only.

Plane should have the following sensors:
- acceleromenter, gyroscope
- barometer
- GPS
- magnethometer
- pitot (optional)

By befault, all navigation modes are disabled when GPS fix is lost. If RC signal is lost also, plane will not be able to enable RTH. Plane will switch to LANDING instead. When flying above inreachable spaces, plane will be lost.

GPS fix estimation allows to recover plane using magnetometer and baromener only.

Note, that GPS fix estimation is not a solution for navigation without GPS. Without GPS fix, position error accumulates quickly. But it is acceptable for RTH. 

GPS Fix is also estimated on GPS Sensor timeouts (hardware failures).

# How it works ?

In normal situation, plane is receiving it's position from GPS sensor. This way it is able to hold course, RTH or navigate by waypoints.

Without GPS fix, plane has nose heading from magnetometer and height from barometer only.

To navigate without GPS fix, we make the following assumptions:
- plane is flying in the direction where nose is pointing
- (if pitot tube is not installed) plane is flying with constant airspeed, specified in settings

It is posible to roughtly estimate position using these assumptions. To increase accuracy, plane will use information about wind direction and speed, estimated before GPS fix was lost. To increase groundspeed estimation accuracy, plane will use pitot tube data(if available).

From estimated heading direction and speed, plane is able to **roughtly** estimate it's position.

It is assumed, that plane will fly in roughtly estimated direction to home position untill either GPS fix or RC signal is recovered.

*Plane has to aquire GPS fix and store home position before takeoff. Estimation completely without GPS fix will not work*.


# Settings

GPS Fix estimation is enabled with CLI command:

```set inav_allow_gps_fix_estimation=ON```

Also you have to specify cruise airspeed of the plane.

To find out cruise airspeed, make a test flight. Enable ground speed display on OSD. Flight in CRUISE mode in two opposite directions. Take average speed.

Cruise airspeed is specified in cm/s.

To convert km/h to m/s, multiply by 27.77.


Example: 100 km/h = 100 * 27.77 = 2777 cm/s

```set fw_reference_airspeed=2777```

*It is important, that plane fly with specified speed in CRUISE mode. If you have set option "Increase cruise speed with throttle"  - do not use it without GPS Fix.*

*If pitot is available, pitot sensor data will be used instead of constant. It is not necessary to specify fw_reference_airspeed. However, it is still adviced to specify for the case of pitot failure.*

*Note related command: to continue mission without RC signal, see command ```set failsafe_mission_delay=-1```.*

**After entering CLI command, make sure that settings are saved:**

```save```

# Disabling GPS sensor from RC controller

![](Screenshots/programming_disable_gps_sensor_fix.png) 

For testing purpoces, it is possible to disable GPS sensor fix from RC controller in programming tab:

*GPS can be disabled only after: 1) initial GPS fix is acquired 2) in ARMED mode.*

# Allowing wp missions with GPS Fix estimation

```failsafe_gps_fix_estimation_delay```

Controls whether waypoint mission is allowed to proceed with gps fix estimation. Sets the time delay in seconds between gps fix lost event and RTH activation. Minimum delay is 7 seconds. If set to -1 the mission will continue until the end. With default setting(7), waypoint mission is aborted and switched to RTH with 7 seconds delay. RTH is done with GPS Fix estimation. RTH is trigerred regradless of failsafe procedure selected in configurator.

# Expected error

Relistic expected error is up to 20%. In tests, 500m drift per 5km path was seen. 

To dicrease drift:
- fly one large circle with GPS available to get good wind estimation
- use airspeed sensor
- do large turns
- make sure compass is pointing nose direction precicely
- calibrate compass correctly


# Is it possible to implement this for multirotor ?

There are some ideas, but there is no solution now. We can not make assumptions with multirotor which we can make with a fixed wing.


# Links

INAV HITL  https://github.com/RomanLut/INAV-X-Plane-HITL
