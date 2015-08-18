# Navigation

Navigation system in Cleanflight is responsible for assisting the pilot allowing altitude and position hold, return-to-home and waypoint flight.

## Altitude hold

Altitude hold requires a valid source of altitude - barometer or sonar. The best source is chosen automatically

PIDs to tune: ALT & VEL
PID meaning:
    ALT - translates altitude error to desired climb rate and acceleration. Tune P for altitude-to-velocity regulator and I for velocity-to-acceleration regulator
    VEL - translated Z-acceleration error to throttle adjustment, full PID is used

## Throttle tilt compensation

Throttle tilt compensation attempts to maintain constant vertical thrust when copter is tilted giving additional throttle if tilt angle (pitch/roll) is not zero. Controlled by `nav_throttle_tilt_comp` CLI variable

## Position hold

Position hold required GPS and compass sensors. Flight modes that require a compass (POSHOLD, RTH) are locked until compass is properly calibrated

PIDs to tune: POS, POSR
PID meaning:
    POS - translated position error to desired velocity, uses P term only
    POSR - translates velocity error to desired acceleration, is a full PID-regulator

## Return to home

Home for RTH is position, where copter was armed. RTH requires accelerometer, compass and GPS sensors.

If barometer is NOT present, RTH will fly directly to home, altitude control here is up to pilot.

If barometer is present, RTH will maintain altitude during the return and when home is reached copter will attempt automated landing.
When deciding what altitude to maintain, RTH has 4 different modes of operation (controlled by `nav_rth_alt_mode` and `nav_rth_altitude` cli variables):
0 - maintain constant altitude during whole RTH sequence (`nav_rth_altitude` is ignored)
1 - climb to current altitude plus extra margin prior to heading home (`nav_rth_altitude` defines the extra altitude (cm))
2 - climb to predefined altitude before heading home (`nav_rth_altitude` defined altitude above launch point (cm))
3 - track maximum altitude of the whole flight, climb to that altitude prior to the return (`nav_rth_altitude` is ignored)
