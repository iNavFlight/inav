# Navigation

The navigation system in INAV is responsible for assisting the pilot allowing altitude and position hold, return-to-home and waypoint flight.

## NAV ALTHOLD mode - altitude hold

Altitude hold requires a valid source of altitude - barometer, GPS or rangefinder. The best source is chosen automatically. GPS is available as an altitude source for airplanes only.
In this mode THROTTLE stick controls climb rate (vertical velocity). When pilot moves stick up - quad goes up, pilot moves stick down - quad descends, you keep stick at neutral position - quad hovers.

### CLI parameters affecting ALTHOLD mode:
* *nav_use_midthr_for_althold* - when set to "0", firmware will remember where your throttle stick was when ALTHOLD was activated - this will be considered neutral position. When set to "1" - 50% throttle will be considered neutral position.

### Related PIDs
PIDs affecting altitude hold: ALT & VEL
PID meaning:
* ALT - translates altitude error to desired climb rate and acceleration. Tune P for altitude-to-velocity regulator and I for velocity-to-acceleration regulator
* VEL - translated Z-acceleration error to throttle adjustment

## Throttle tilt compensation

Throttle tilt compensation attempts to maintain constant vertical thrust when copter is tilted giving additional throttle if tilt angle (pitch/roll) is not zero. Controlled by *throttle_tilt_comp_str* CLI variable.

## NAV POSHOLD mode - position hold

Position hold requires GPS, accelerometer and compass sensors. Flight modes that require a compass (POSHOLD, RTH) are locked until compass is properly calibrated.
When activated, this mode will attempt to keep copter where it is (based on GPS coordinates). From inav 2.0, POSHOLD is a full 3D position hold. Heading hold in this mode is assumed and activated automatically.

### CLI parameters affecting POSHOLD mode:
* *nav_user_control_mode* - can be set to "0" (GPS_ATTI) or "1" (GPS_CRUISE), controls how firmware will respond to roll/pitch stick movement. When in GPS_ATTI mode, right stick controls attitude, when it is released, new position is recorded and held. When in GPS_CRUISE mode right stick controls velocity and firmware calculates required attitude on its own.

### Related PIDs
PIDs affecting position hold: POS, POSR
PID meaning:
* POS - translated position error to desired velocity, uses P term only
* POSR - translates velocity error to desired acceleration

## NAV RTH - return to home mode

Home for RTH is the position where vehicle was first armed. This position may be offset by the CLI settings `nav_rth_home_offset_distance` and `nav_rth_home_offset_direction`. This position may also be overridden with Safehomes. RTH requires accelerometer, compass and GPS sensors.

If barometer is NOT present, RTH will fly directly to home, altitude control here is up to pilot.

If barometer is present, RTH will maintain altitude during the return. When home is reached, a copter will attempt automated landing. An airplane will either loiter around the home position, or attempt an automated landing, depending on your settings.
When deciding what altitude to maintain, RTH has 6 different modes of operation (controlled by *nav_rth_alt_mode* and *nav_rth_altitude* cli variables):
* 0 (NAV_RTH_NO_ALT) - keep current altitude during whole RTH sequence (*nav_rth_altitude* is ignored)
* 1 (NAV_RTH_EXTRA_ALT) - climb to current altitude plus extra margin prior to heading home (*nav_rth_altitude* defines the extra altitude (cm))
* 2 (NAV_RTH_CONST_ALT) - climb/descend to predefined altitude before heading home (*nav_rth_altitude* defined altitude above launch point (cm))
* 3 (NAV_RTH_MAX_ALT) - track maximum altitude of the whole flight, climb to that altitude prior to the return (*nav_rth_altitude* is ignored)
* 4 (NAV_RTH_AT_LEAST_ALT) - same as 2 (NAV_RTH_CONST_ALT), but only climb, do not descend
* 5 (NAV_RTH_AT_LEAST_ALT_LINEAR_DESCENT) - Same as 4 (NAV_RTH_AT_LEAST_ALT). But, if above the RTH Altitude, the aircraft will gradually descend to the RTH Altitude. The target is to reach the RTH Altitude as it arrives at the home point. This is to save energy during the RTH.

## NAV WP - Waypoint mode

NAV WP allows the craft to autonomously navigate a set route defined by waypoints that are loaded into the FC as a predefined mission.

## CLI command `wp` to manage waypoints

`wp` - List all waypoints.

`wp load` - Load list of waypoints from EEPROM to FC.

`wp <n> <action> <lat> <lon> <alt> <p1> <p2> <p3> <flag>` - Set parameters of waypoint with index `<n>`. Note that prior to inav 2.5, the `p2` and `p3` parameters were not required. From 2.5, inav will accept either version but always saves and lists the later full version.

Parameters:

  * `<action>` - The action to be taken at the WP. The following are enumerations are available in inav 2.6 and later:
      *  0 - Unused / Unassigned
      *  1 - WAYPOINT
      *  3 - POSHOLD_TIME
      *  4 - RTH
	  *  5 - SET_POI
      *  6 - JUMP
      *  7 - SET_HEAD
      *  8 - LAND

  * `<lat>` - Latitude (WGS84), in degrees * 1E7 (for example 123456789 means 12.3456789).

  * `<lon>` - Longitude.

  * `<alt>` - Altitude in cm. See `p3` bit 0 for datum definition.

  * `<p1>` - For a RTH waypoint, p1 > 0 enables landing. For a normal waypoint it is the speed to this waypoint (cm/s), it is taken into account only for multicopters and when > 50 and < nav_auto_speed. For POSHOLD TIME waypoint it is time to loiter in seconds. For JUMP it is the target WP **index** (not number). For SET_HEAD, it is the desired heading (0-359) or -1 to cancel a previous SET_HEAD or SET_POI.

  * `<p2>` - For a POSHOLD TIME it is the speed to this waypoint (cm/s), it is taken into account only for multicopters and when > 50 and < nav_auto_speed. For JUMP it is the number of iterations of the JUMP.

  * `<p3>` - A  bitfield with four bits reserved for user specified actions. It is anticipated that these actions will be exposed through the logic conditions.
      * Bit 0 - Altitude (`alt`) : Relative (to home altitude) (0) or Absolute (AMSL) (1).
	  * Bit 1 - WP Action 1
	  * Bit 2 - WP Action 2
      * Bit 3 - WP Action 3
      * Bit 4 - WP Action 4
	  * Bits 5 - 15 : undefined / reserved.

      Note:

	  * If `p2` is specified, then `p3` is also required.
	  * `p3` is only defined for navigable WP types (WAYPOINT, POSHOLD_TIME, LAND). The affect of specifying a non-zero `p3` for other WP types is undefined.

  * `<flag>` - Last waypoint must have `flag` set to 165 (0xA5).

`wp save` - Checks list of waypoints and save from FC to EEPROM (warning: it also saves all unsaved CLI settings like normal `save`).

`wp reset` - Resets the list, sets the number of waypoints to 0 and marks the list as invalid (but doesn't delete the waypoint definitions).

### `wp` example

```
# wp load

# wp
# wp 11 valid
wp 0 1 543533193 -45179273 3500 0 0 0 0
wp 1 1 543535723 -45193913 3500 0 0 0 0
wp 2 1 543544541 -45196617 5000 0 0 0 0
wp 3 1 543546578 -45186895 5000 0 0 0 0
wp 4 6 0 0 0 1 2 0 0
wp 5 1 543546688 -45176009 3500 0 0 0 0
wp 6 1 543541225 -45172673 3500 0 0 0 0
wp 7 6 0 0 0 0 1 0 0
wp 8 3 543531383 -45190405 3500 45 0 0 0
wp 9 1 543548470 -45182104 3500 0 0 0 0
wp 10 8 543540521 -45178091 6000 0 0 0 165
wp 11 0 0 0 0 0 0 0 0
...
wp 59 0 0 0 0 0 0 0 0
```

Note that the `wp` CLI command shows waypoint list indices, while the MW-XML definition used by mwp, ezgui and the configurator use WP numbers.

## Multi-missions

Multi-missions allows up to 9 missions to be stored in the FC at the same time. It is possible to load them into the FC using the CLI. This is acheived by entering single missions into the CLI followed by `wp save` **after** the final mission has been entered (the single missions can be entered one after the other or as a single block entry, it doesn't matter). All missions will then be saved as a Multi Mission in the FC. Saved multi missions display consecutive WP indices from 0 to the last WP in the last mission when displayed using the `wp` command.

E.g. to enter 3 missions in the CLI enter each mission as a single mission (start WP index for each mission must be 0).
```
wp 0 1 545722109 -32869291 5000 0 0 0 0
wp 1 1 545708178 -32642698 5000 0 0 0 0
wp 2 1 545698227 -32385206 5000 0 0 0 165
...
wp 0 1 545599696 -32958555 5000 0 0 0 0
wp 1 1 545537978 -32958555 5000 0 0 0 0
wp 2 1 545547933 -32864141 5000 0 0 0 0
wp 3 1 545597705 -32695913 5000 0 0 0 0
wp 4 1 545552910 -32598066 5000 0 0 0 0
wp 5 6 0 0 0 0 0 0 165
...
wp 0 1 545714148 -32501936 5000 0 0 0 165

# wp save
```

Multi Mission after saving:
```
# wp
# wp 10 valid
wp 0 1 545722109 -32869291 5000 0 0 0 0
wp 1 1 545708178 -32642698 5000 0 0 0 0
wp 2 1 545698227 -32385206 5000 0 0 0 165
wp 3 1 545599696 -32958555 5000 0 0 0 0
wp 4 1 545537978 -32958555 5000 0 0 0 0
wp 5 1 545547933 -32864141 5000 0 0 0 0
wp 6 1 545597705 -32695913 5000 0 0 0 0
wp 7 1 545552910 -32598066 5000 0 0 0 0
wp 8 6 0 0 0 0 0 0 165
wp 9 1 545714148 -32501936 5000 0 0 0 165
wp 10 0 0 0 0 0 0 0 0
wp 11 0 0 0 0 0 0 0 0
wp 12 0 0 0 0 0 0 0 0
...
wp 59 0 0 0 0 0 0 0 0
```
