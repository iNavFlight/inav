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
- magnethometer (optional, highly recommended)
- pitot (optional)

By befault, all navigation modes are disabled when GPS fix is lost. If RC signal is lost also, plane will not be able to enable RTH. Plane will switch to LANDING instead. When flying above unreachable spaces, plane will be lost.

GPS fix estimation allows to recover plane using magnetometer and baromener only.

GPS Fix is also estimated on GPS Sensor timeouts (hardware failures).

Note, that GPS fix estimation is not a solution for navigation without GPS. Without GPS fix, position error accumulates quickly. But it is acceptable for RTH. This is not a solution for flying under spoofing also. GPS is the most trusted sensor in Inav. It's output is not validated.

# How it works ?

In normal situation, plane is receiving it's position from GPS sensor. This way it is able to hold course, RTH or navigate by waypoints.

Without GPS fix, plane has nose heading from magnetometer and height from barometer only.

To navigate without GPS fix, we make the following assumptions:
- plane is flying in the direction where nose is pointing
- (if pitot tube is not installed) plane is flying with constant airspeed, specified in settings

It is possible to roughly estimate position using theese assumptions. To increase accuracy, plane will use information about wind direction and speed, estimated before GPS fix was lost. To increase groundspeed estimation accuracy, plane will use pitot tube data(if available).

From estimated heading direction and speed, plane is able to **roughty** estimate it's position.

It is assumed, that plane will fly in roughly estimated direction to home position untill either GPS fix or RC signal is recovered.

*Plane has to aquire GPS fix and store home position before takeoff. Estimation completely without GPS fix will not work*.

# Estimation without magnethometer

Without magnethometer, navigation accuracy is very poor. The problem is heading drift. 

The longer plane flies without magnethometer or GPS, the bigger is course estimation error.

After few minutes and few turns, "North" direction estimation can be completely broken.
In general, accuracy is enough to perform RTH U-turn when both RC controls and GPS are lost, and roughtly keep RTH direction in areas with occasional GPS outages.

![image](https://github.com/RomanLut/inav/assets/11955117/3d5c5d10-f43a-45f9-a647-af3cca87c4da)

(purple line - estimated position, black line - real position).

It is recommened to use GPS fix estimation without magnethometer as last resort only. For example, if plane is flying above lake, landing means loss of plane. With GPS Fix estimation, plane will try to do RTH in very rought direction, instead of landing.

It is up to user to estimate the risk of fly-away.


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

For testing purposes, it is possible to disable GPS sensor fix from RC controller in programming tab:

*GPS can be disabled only after: 1) initial GPS fix is acquired 2) in ARMED mode.*

# Allowing wp missions with GPS Fix estimation

```failsafe_gps_fix_estimation_delay```

Controls whether waypoint mission is allowed to proceed with gps fix estimation. Sets the time delay in seconds between gps fix lost event and RTH activation. Minimum delay is 7 seconds. If set to -1 the mission will continue until the end. With default setting(7), waypoint mission is aborted and switched to RTH with 7 seconds delay. RTH is done with GPS Fix estimation. RTH is trigerred regradless of failsafe procedure selected in configurator.

# Expected error (mag + baro)

Realistic expected error is up to 200m per 1km of flight path. In tests, 500m drift per 5km path was seen. 

To dicrease drift:
- fly one large circle with GPS available to get good wind estimation
- use airspeed sensor. If airspeed sensor is not installed, fly in cruise mode without throttle override.
- do smooth, large turns
- make sure compass is pointing in nose direction precicely
- calibrate compass correctly

This video shows real world test where GPS was disabled occasionally. Wind is 10km/h south-west:


https://github.com/RomanLut/inav/assets/11955117/0599a3c3-df06-4d40-a32a-4d8f96140592


Purple line shows estimated position. Black line shows real position. "EST ERR" sensor shows estimation error in metters. Estimation is running when satellite icon displays "ES". Estimated position snaps to real position when GPS fix is reaquired.


# Is it possible to implement this for multirotor ?

There are some ideas, but there is no solution now. We can not make assumptions with multirotor which we can make with a fixed wing.


# Links

INAV HITL  https://github.com/RomanLut/INAV-X-Plane-HITL
