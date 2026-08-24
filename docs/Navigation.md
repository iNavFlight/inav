# Navigation

The navigation system in INAV is responsible for assisting the pilot allowing altitude and position hold, return-to-home and waypoint flight.

## NAV ALTHOLD mode - altitude hold

Altitude hold requires a valid source of altitude - barometer, GPS or rangefinder. The best source is chosen automatically.
In this mode THROTTLE stick controls climb rate (vertical velocity). When pilot moves stick up - aircraft goes up, pilot moves stick down -
aircraft descends, you keep stick at neutral position - aircraft maintains current altitude.


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

Position hold requires GPS, accelerometer and compass sensors. Multirotor requires barometer, unless is enabled. Flight modes that require a compass (POSHOLD, RTH) are locked until compass is properly calibrated.
When activated, this mode will attempt to keep copter where it is (based on GPS coordinates). From INAV 2.0, POSHOLD is a full 3D position hold. Heading hold in this mode is assumed and activated automatically.

### CLI parameters affecting POSHOLD mode:
* *nav_user_control_mode* - can be set to "0" (GPS_ATTI) or "1" (GPS_CRUISE), controls how firmware will respond to roll/pitch stick movement. When in GPS_ATTI mode, right stick controls attitude, when it is released, new position is recorded and held. When in GPS_CRUISE mode right stick controls velocity and firmware calculates required attitude on its own.


### Related PIDs
PIDs affecting position hold: POS, POSR
PID meaning:
* POS - translated position error to desired velocity, uses P term only
* POSR - translates velocity error to desired acceleration

## Marker Guidance Target Consumer (MSP)

INAV can consume externally computed marker offsets over MSP and use them for:
1. precision landing alignment
2. marker-relative position hold in POSHOLD
3. marker-relative containment (indoor limiter behavior)

### Build-time availability
This feature is compiled only when `USE_MARKER_GUIDANCE` is enabled for the target.
On flash-constrained targets, it can be excluded at build time to preserve headroom.

### MSP payload
`MSP2_INAV_SET_MARKER_GUIDANCE_TARGET` (`8754 / 0x2232`) has one fixed 8-byte little-endian request:

| Bytes | Type | Field | Meaning |
|---|---|---|---|
| 0..1 | `int16_t` | `offsetForwardCm` | Levelled horizontal offset from the vehicle to touchdown, forward in the yaw-only body frame |
| 2..3 | `int16_t` | `offsetRightCm` | Levelled horizontal offset from the vehicle to touchdown, right in the yaw-only body frame |
| 4..5 | `int16_t` | `yawErrorDeciDeg` | Signed shortest turn from current heading to landing heading, from `-1800` to `1800` |
| 6..7 | `uint16_t` | `markerAglCm` | Positive distance from vehicle body origin to the landing reference plane |

The old 4-byte request and all other request sizes are rejected. There is no version, confidence, frame, timestamp, validity flag or marker identity field. Each accepted packet replaces the complete XY, heading and marker-relative height sample. Freshness is based on FC receive time and `nav_marker_guidance_max_target_age_ms`.

Example: forward `-123 cm`, right `456 cm`, yaw error `-90.0 deg` and marker AGL `321 cm` are encoded as `85 FF C8 01 7C FC 41 01`.

On receipt, INAV converts forward/right into local North/East using the current vehicle yaw and stores that result. The stored target does not rotate if the vehicle turns before the next packet. INAV also converts the relative yaw error into one absolute heading target at receipt time. The same cached North/East reference is used by precision landing and containment.

The five-byte reply is `accepted`, `used_now`, `nav_guidance_state`, `reason`, `retry_count`. `accepted = 1` means the complete sample passed validation and was atomically stored. `used_now = 1` means it can currently affect an allowed navigation controller; a valid sample can be accepted with `used_now = 0` outside an allowed mode.

ACK `reason` values are:

| Value | Reason |
|---|---|
| 0 | `OK` |
| 1 | `NOT_ENABLED` |
| 2 | `STALE` |
| 3 | `OFFSET_TOO_LARGE` |
| 4 | `NOT_MC_PROFILE` |
| 5 | `NOT_IN_POSHOLD_OR_LAND` |
| 6 | `FAILSAFE` |
| 7 | `INVALID_TARGET` |
| 8 | `NOT_ARMED` |

Disabled guidance, zero marker AGL, yaw error outside `[-1800, 1800]`, or an excessive horizontal offset returns `accepted = 0` and does not change the previous cache or its receive timestamp.

### Mode gating
Marker guidance can influence navigation only when:
* active profile is MC/VTOL-hover-capable
* `nav_marker_guidance_mode` is not `OFF`

Outside those contexts, updates may still be cached but do not affect navigation loops.

### POSHOLD behavior
When `nav_marker_guidance_mode = PL`:
* FC uses marker offsets to center above the target in POSHOLD.
* while the target is fresh, FC uses the marker heading immediately before the MC heading controller
* marker heading never overrides disarmed, failsafe, fixed-wing or manual yaw control

When `nav_marker_guidance_mode = CONTAINMENT`:
* FC uses marker-relative hold target:
  * `nav_marker_containment_hold_north_cm`
  * `nav_marker_containment_hold_east_cm`
* FC applies containment behavior with `nav_marker_guidance_radius_cm`:
  * inside radius: no correction
  * outside radius: FC corrects back toward allowed boundary
* containment does not take yaw control

### LAND behavior
When `nav_marker_guidance_mode = PL` and target is fresh:
* FC performs precision horizontal alignment to marker center during LAND
* FC uses the absolute marker heading calculated when the latest packet was accepted
* vertical descent profile remains normal LAND behavior (`nav_land_*`)

With stale/lost target:
* FC stops marker XY correction
* after a marker heading was acquired in this LAND context, FC keeps that last heading through lost hold, climb-and-retry and normal-landing fallback
* at or below `nav_marker_guidance_retry_min_alt_cm`, either usable INAV AGL or the last fresh marker AGL suppresses retry and continues normal LAND behavior
* if `nav_marker_guidance_low_alt_lock_xy = ON`, FC locks the current XY position when entering low-altitude fallback
* above that altitude, FC enters hold for `nav_marker_guidance_lost_hold_time_ms`
* optionally performs climb-and-retry up to `nav_marker_guidance_retry_count`
* then falls back to normal LAND behavior

Retry safety rule:
* retry is only entered if target was acquired at least once in the current LAND context
* if no target was ever acquired in that LAND context, no retry is performed
* set `nav_marker_guidance_retry_min_alt_cm = 0` to disable the low-altitude retry suppression
* set `nav_marker_guidance_low_alt_lock_xy = OFF` to keep the normal LAND XY target during low-altitude fallback
* marker AGL is only an additional reason to suppress a climb; it never starts a retry and never replaces INAV altitude, AGL or rangefinder estimates

### Shared radius setting
`nav_marker_guidance_radius_cm` is used by both modes:
* `PL`: center-alignment deadband around marker center
* `CONTAINMENT`: allowed radius around marker-containment hold target
* `0`: continuous correction (no deadband/boundary allowance)

### Core safety semantics
* new packet == fresh target sample
* no packet inside timeout window == target lost
* horizontal marker-guidance correction is capped by current active navigation speed limit (`getActiveSpeed()`)
* no dynamic allocation in the runtime path

## NAV RTH - return to home mode

Home for RTH is the position where vehicle was first armed. This position may be offset by the CLI settings `nav_rth_home_offset_distance` and `nav_rth_home_offset_direction`. This position may also be overridden with Safehomes. RTH requires accelerometer, compass and GPS sensors.

RTH requires barometer for multirotor.

RTH will maintain altitude during the return. When home is reached, a copter will attempt automated landing. An airplane will either loiter around the home position, or attempt an automated landing, depending on your settings.
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

`wp <n> <action> <lat> <lon> <alt> <p1> <p2> <p3> <flag>` - Set parameters of waypoint with index `<n>`. Note that prior to INAV 2.5, the `p2` and `p3` parameters were not required. From 2.5, INAV will accept either version but always saves and lists the later full version.

Parameters:

  * `<action>` - The action to be taken at the WP. The following are enumerations are available in INAV 2.6 and later:
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

  * `<p1>` - For a RTH waypoint, p1 > 0 enables landing. For a normal waypoint it is the speed to this waypoint (cm/s). For multicopters it works for speeds > 0.5 m/s and < nav_auto_speed. The speed setting also applies for fixed wing from V10.0, where a non-zero value requests fixed-wing Auto Speed for that waypoint. Auto Speed remains disabled while the VTOL transition controller owns a transition, including the short completion phase after the mixer profile changes. For POSHOLD TIME waypoint it is time to loiter in seconds. For JUMP it is the target WP **index** (not number). For SET_HEAD, it is the desired heading (0-359) or -1 to cancel a previous SET_HEAD or SET_POI.

  * `<p2>` - For a POSHOLD TIME it is the speed to this waypoint (cm/s). For multicopters it works for speeds > 0.5 m/s and < nav_auto_speed. For fixed wing from V10.0, a non-zero value requests fixed-wing Auto Speed for that hold waypoint, with the same VTOL transition exclusion described for `p1`. For JUMP it is the number of iterations of the JUMP.

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

### Mission VTOL transition using existing User Actions

Mission VTOL transition can be requested.
This is available only on targets with more than 512 KB flash, compiled with `USE_AUTO_TRANSITION`.
Targets with 512 KB flash do not include these mission VTOL transition settings.

Configuration:

- `nav_vtol_mission_transition_user_action` selects which waypoint User Action (`USER1..USER4`) is used as the mission VTOL target selector.
- `nav_vtol_mission_transition_min_altitude_cm` optionally enforces a minimum altitude before transition start (`0` disables check).
- During MC->FW mission transition, INAV uses a built-in straight run-up target to help the model build speed before switching to fixed-wing.
- VTOL transition completion logic is shared with manual MIXER TRANSITION and uses mixer transition settings:
  - preferred MC->FW threshold: `vtol_transition_to_fw_min_airspeed_cm_s`
  - FW->MC threshold: `vtol_transition_to_mc_max_airspeed_cm_s`

Behavior on each navigable mission waypoint (`WAYPOINT`, `POSHOLD_TIME`, `LAND`):

- The configured USER bit is an **absolute target selector**:
  - `0`: transition to MC / MULTIROTOR profile
  - `1`: transition to FW / AIRPLANE profile
- When `nav_vtol_mission_transition_user_action != OFF`, each navigable waypoint always encodes target state via that selected USER bit.
- This means every navigable waypoint implicitly declares desired VTOL platform state when this feature is enabled; users must intentionally set/clear that bit on each waypoint.
- This command is **not** a toggle.
- The command is idempotent: if already in the requested target profile type, the mission continues immediately.
- If a transition is needed, mission progression pauses while automated transition runs, then resumes only after completion.

Transition behavior in this MVP:

- MC -> FW: straight-line acceleration segment (no loiter), heading from the next waypoint bearing when available, otherwise current heading.
- MC -> FW and FW -> MC completion uses pitot airspeed thresholds when healthy/available (`vtol_transition_to_fw_min_airspeed_cm_s`, `vtol_transition_to_mc_max_airspeed_cm_s`).
- If pitot is unavailable/unhealthy (or threshold is `0`), timer fallback (`mixer_switch_trans_timer`) is used.
- Ground speed is not used for transition progress/completion.
- FW -> MC: mission pauses during automated transition, then resumes after switching back to MC profile.
- Strict altitude hold is not enforced during MC -> FW transition; natural climb is allowed.
- If an airspeed-controlled MC -> FW transition times out, `nav_vtol_transition_retry_on_airspeed_timeout` can run one heading scan/retry before the configured fail action is used.

Safety and scope:

- This path uses authorized automated transition state handling; it does not permit manual mixer profile switching during normal waypoint navigation.
- It still depends on valid mixer profile switching infrastructure (two configured mixer profiles and a valid `MIXER PROFILE 2` mode activation condition).

RTH and failsafe VTOL transitions:

- RTH may request MC -> FW before flying home if the aircraft is in MC and far enough from home.
- RTH landing may request FW -> MC before using the MC landing controller.
- Failsafe RTH/LAND is allowed to continue those navigation-owned `RTH` and `LAND` transition requests.
- `vtol_fw_to_mc_auto_switch_airspeed_cm_s` can also request a navigation-owned FW -> MC safety transition during mission, RTH, or failsafe RTH when trusted pitot airspeed falls too low.
- This low-speed safety transition requires `mixer_automated_switch = ON` and a valid MC target profile.
- After the low-speed safety transition switches to MC, INAV keeps the current navigation task in MC and blocks automatic MC -> FW RTH or mission re-entry for that navigation session.
- Manual and mission transition requests are not allowed to continue just because failsafe became active; they are aborted unless the target profile has already been selected and INAV is only finishing the remaining safe output movement.
- `vtol_transition_to_mc_max_airspeed_cm_s` controls when an already-requested FW -> MC transition is considered safe to complete.

### VTOL MC navigation protection and landing detection

Targets with more than 512 KB flash can enable extra protection for VTOL aircraft flying in MC mode:

- `vtol_mc_protection_mode = OFF`: NAV capture, throttle reserve, landing settle, bailout, and command shaping are disabled. The independent VTOL MC touchdown confirmation described below remains active.
- `vtol_mc_protection_mode = NAV`: protects VTOL MC navigation and altitude-control behavior.
- `vtol_mc_protection_mode = NAV_AND_STABILIZED`: also shapes ANGLE/HORIZON roll, pitch, and yaw commands at higher horizontal speed.

The protection only activates when the current mixer profile is multicopter-like and another configured mixer profile is fixed-wing. Normal multirotors and fixed-wing mode are not changed.

In NAV modes, VTOL MC protection adds:

- throttle reserve before altitude PID anti-windup, controlled by `vtol_mc_thr_reserve_percent`,
- capture/settle when entering position-holding navigation with horizontal speed,
- soft altitude capture while horizontal speed is being bled off,
- a stricter RTH/WP landing settle gate before descent starts,
- a conservative bailout path if attitude becomes excessive while automatic throttle is active.

The landing settle gate uses `min(nav_wp_radius, 100 cm)` as the capture radius for the landing point. It also requires low horizontal speed, low vertical speed, and safe attitude to be held for the internal settle time. This prevents a large `nav_wp_radius` from starting landing descent after only briefly touching the waypoint radius while still moving.

ANGLE/HORIZON shaping in `NAV_AND_STABILIZED` only runs when armed, VTOL MC mode is detected, velocity estimate is trusted, and horizontal speed is above the shaping threshold. It continuously scales roll, pitch, and yaw commands as speed increases, preserving command sign and small deadband behavior.

#### Landing detector sensitivity

`nav_land_detect_sensitivity` scales the generic landing detector velocity and gyro thresholds. The default `5` is nominal sensitivity. For multirotors, this corresponds to about:

- `100 cm/s` horizontal speed,
- `100 cm/s` vertical speed,
- `4 deg/s` average pitch/roll gyro rate.

Higher values relax these thresholds and can make landing detection faster, but also increase false-detect risk. VTOL MC landing detection adds additional safety gates that `nav_land_detect_sensitivity` does not bypass:

- vertical speed must be near zero when altitude/vertical-speed estimate is available,
- NAV landing must be in the final slow-descent context,
- trusted surface/AGL data, if available, must show near-ground,
- all VTOL MC landing candidates must pass touchdown confirmation before `LANDING_DETECTED`.

When automatic throttle control is active, the VTOL MC throttle probe gently reduces lift throttle for a short confirmation window. If the aircraft starts descending, shows unloading acceleration, or trusted AGL drops, the candidate is rejected and the detector waits again. In a manual-throttle mode INAV does not alter pilot throttle for this test, so touchdown cannot be confirmed from a passive timeout unless trusted AGL also shows near-ground. This avoids false disarm while still airborne without adding an unexpected motor command to ANGLE, HORIZON, or manual flight.

`nav_landing_bump_detection = ON` allows G-bump touchdown detection to create a landing candidate. For VTOL MC it is not an immediate disarm shortcut: trusted high AGL blocks it, and accepted candidates still go through touchdown confirmation. For non-VTOL multirotors it keeps the existing landing detector behavior.

Automatic disarm still requires `nav_disarm_on_landing = ON`. `nav_auto_disarm_delay` is applied after a landing candidate is detected; in VTOL MC mode the additional touchdown confirmation must also pass before the global `LANDING_DETECTED` state is set.

Debugging:

- `debug_mode = VTOL_MC_PROTECT` shows protection flags, safe throttle range, protected throttle, speed, attitude, and settle/command-scale progress.
- `debug_mode = LANDING` shows normal landing detector status and candidate state.

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
### Changing Mission-Index in flight
The MISSION CHANGE mode allows to switch between multiple stored missions in flight. With mode active the required mission index can be selected by cycling through missions using the WP mode switch. Selected mission is loaded when mission change mode is switched off. Mission index can also be changed through addition of a new Mission Index adjustment function which should be useful for DJI users unable to use the normal OSD mission related fields.
