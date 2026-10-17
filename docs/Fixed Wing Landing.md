# Fixed Wing Landing

## Introducion

INAV supports advanced automatic landings for fixed wing aircraft from version 7.1.
The procedure is based on landings for man-carrying aircraft, so that safe landings at a specific location are possible.
Supported are landings at Safehome after "Return to Home" or at a defined LAND waypoint for missions. 
Every landing locations can be defined with a target point and 2 different approach headings (colinear to the landing strips) with exclusive direction or opposite directions allowed. 
This enables up to 4 different approach directions, based on the landing site and surrounding area. 

## General procedure:

1. After reaching Safehome/LAND Waypoint the altitude is corrected to "Approach Altitude".
2. The aircraft circles for at least 30 seconds to determine the wind direction and strength.
3. The landing direction and the approach waypoints are calculated on the basis of the measured wind parameters. If no headwind landing is possible or the wind strength is greater than "Max. tailwind" (see Global Parameters), return to point 2.
4. The landing is initiated. The aircraft flies the downwind course, "Approach Altitude" is held.
5. Base Leg: the altitude is reduced from 2/3 of "Approach Altitude".
6. Final Approach: The engine power is reduced using "Pitch2throttle modifier" to reduce speed and the altitude is gradually reduced towards "Land Altitude" while approaching the Safehome coordinates.
7. Glide: When "Glide Altitude" is reached, the motor is switched off and the pitch angle of "Glide Pitch" is held.
7. Flare: Only if a LIDAR/Rangefinder sensor is present: the motor remains switched off and the pitch angle of "Flare Pitch" is held
   From the glide on, the rudder gently holds the landing direction (2 s time constant, at most 8 degrees per second), so a tail or nose wheel steered by the rudder keeps the roll-out straight.
   With a rangefinder, the glide and flare pitch are held on the measured attitude: after the throttle cut the pitch trim of the powered approach is wrong for the slowing aircraft, so the pitch I-term is re-trimmed on the attitude error until the nose holds the commanded pitch (within `pid_iterm_limit_percent`): the set one, or with `nav_fw_land_flare_sink` the one the sink rate asks for. Nothing changes once the wheels are on the runway.
   During the whole landing the pitch I-term lock (`fw_iterm_lock_*`) is off: there is no stick input, and the trim the power change needs would wait for it.
8. Landing: As soon as INAV has detected the landing, it is automatically disarmed, see setting `nav_disarm_on_landing`.

All turns between the approach legs are flown as coordinated FLY_BY corner cuts: the turn start is anticipated from the aircraft's actual turn radius (speed and `nav_fw_bank_angle`) so the plane rolls out already aligned on the next leg. This applies during the landing approach regardless of the configured `nav_fw_wp_turn_mode`.

To activate the automatic landing, the parameter `nav_rth_allow_landing` must be set to `ALWAYS` or `FAILSAFE`. 

> [!WARNING]
> If landing is activated and no parameters are set for the landing site (Safehome and/or landing waypoint), the old landing procedure (circling until close to the ground, then hovering out) is performed. 
> This is probably not what you want. 
 
The following graphics illustrate the process:

![Approach Drawing Up](/docs/assets/images/Approach-Drawing-Up.png  "Approach Drawing Up")

![Approach Drawing Side](/docs/assets/images/Approach-Drawing-Side.png  "Approach Drawing Side")

## Landing site parameters

### The following parameters are set for each landing site (Safefome/LAND waypoint):

All settings can also be conveniently made in the Configurator via Mission Control.

CLI command `fwapproach`:
`fwapproach <index> <Approach altitude> <Land altitude> <Approach direction> <approach heading 1> <approach heading 2> <sea level> [<runway start 1> <1 reverse> <2> <2 reverse> <height 1> <1 reverse> <2> <2 reverse>]`

`fwapproach` has 17 slots in which landing parameters can be stored. In slot 0-7 the landing parameters for Safehome are stored, in 8 - 16 the parameters for waypoint missions. Only one landing point per mission can be saved. 

* index: 0 - 17, 0 - 7 Safehome, 8 - 16 Mission
* Approach direction: 0 - Left, 1 - Right. Always seen from the primary landing direction (positive value), i.e. whether the aircraft flies left or right turns on approach. With a landing circuit (`nav_fw_land_downwind_offset` set) it is the side of the runway the circuit is flown on, seen from approach heading 1 whatever the landing direction: the free side, away from trees and buildings.
* Approach Altitude: Initial altitude of the approach, the altitude at which the wind direction is determined and the downwind approach, in cm
* Land Altitude: Altitude of the landing site, in cm
* Approach heading 1 and 2: Two landing directions can be set, values: 0 - +/-360. 0 = landing direction is deactivated. 
A positive value means that you can approach in both directions, a negative value means that this direction is exclusive.
Example: 90 degrees: It is possible to land in 90 degrees as well as in 270 degrees. -90 means that you can only land in a 90 degree direction.
This means that practically 4 landing directions can be saved.
* Sea Level: 0 - Deactivated, 1 - Activated. If activated, approach and land altitude refer to normal zero (sea level), otherwise relative altitude to the altitude during first GPS fix.
* Runway starts and heights: optional, for each landing direction (heading 1, its reverse, heading 2, its reverse) where the runway begins before the landing site, in metres (0 - 255), then the height the final crosses it at, in decimetres (0 - 255, 20 = 2 m). The final is placed so that it passes over the runway start at that height, on the round-out of the sink-rate flare when that is lower, and the wheels touch where the flare ends: the ground before a runway (a ploughed field, obstacles) is crossed high enough and the flare is flown over the runway. 0 as runway start uses `nav_fw_land_aim_offset` as the touchdown point. With a runway start set the landing checks the approach (a stabilised approach gate): before the runway start the aircraft must not stay over half a second more than 1 m below the set height, and at the runway start it must be no more than 2 m above it, within 2 m/s under and 4 m/s over the approach speed. Otherwise it goes around: it climbs straight ahead to 10 m, then back to the wind circle, and flies the circuit again; at the third time the landing is aborted. Since INAV 11.0; the slots return to their defaults once on the update.

> [!CAUTION]
> The Configuator automatically determines the ground altitude based on databases on the Internet, which may be inaccurate. Please always compare with the measured GPS altitude at the landing site to avoid crashes.

### Global parameters

All settings are available via “Advanced Tuning” in the Configurator.

* `nav_fw_land_approach_length`: Length of the final approach, measured from the land site (Safehome/Waypoint) to the last turning point.
In cm. Max: 100000, Min: 100, Default: 35000

* `nav_fw_land_final_approach_pitch2throttle_mod`: Modifier for pitch to throttle ratio at final approach. This parameter can be used to reduce speed during the final approach. 
Example: If the parameter is set to 200% and Pitch To Throttle Ratio is set to 15, Pitch To Throttle Ratio is set to 30 on the final approach. This causes a reduction in engine power on approach when the nose is pointing downwards.
In Percent. Min: 100, Max: 400, Default: 100

* `nav_fw_land_glide_alt`: Initial altitude of the glide phase. The altitude refers to "Landing Altitude", see above under "Landing site parameters"
In cm. Min: 100, Max: 5000, Default: 200

* `nav_fw_land_flare_alt`: Initial altitude of the flare phase. The altitude refers to "Landing Altitude", see above under "Landing site parameters"
In cm. Min: 0, Max: 5000, Default: 200

* `nav_fw_land_glide_pitch`: Pitch value for glide phase. 
In degrees. Min: 0, Max: 45, Default: 0

* `nav_fw_land_flare_pitch`: Pitch value for flare phase. 
  In degrees. Min: 0, Max: 45, Default: 8

* `nav_fw_land_flare_sink`: Sink rate the landing touches down with, 10 to 20 cm/s for a soft landing on delicate gear. Set, the landing flares on its sink rate instead of a fixed pitch. The flare asks the height / `nav_fw_land_flare_time`, down to this sink, held from there to the ground, and it starts where that equals the sink rate the aircraft has (between `nav_fw_land_flare_alt` and twice `nav_fw_land_glide_alt`): higher at more speed or on a steeper final, whatever the aircraft or the wind. Over the first half of the height it blends from the sink measured at its start. The pitch is the flight path angle of that sink plus the angle of attack: the one at the flare start, raised as the lift must hold the weight with the square of the airspeed lost since then, and corrected by an integral of the path error; it starts from the pitch the approach was commanding, moves no faster than 4 m/s2 of vertical acceleration allow at the speed, and stays below `nav_fw_land_flare_pitch`. Near the ground (below this sink x `nav_fw_land_flare_time`) the lowest pitch rises to `nav_fw_land_glide_pitch`, so the main wheels touch first. The power comes off over the time the flare takes and the bank is limited to 5 degrees. The heights are the rangefinder's while it is healthy. The final's straight line is aimed short by the part of the flare's run past where the line meets the ground, at the groundspeed the wind leaves, so the wheels touch at `nav_fw_land_aim_offset`.
  In cm/s. Min: 0, Max: 500, Default: 0 (pitch flare)

* `nav_fw_land_flare_time`: Time constant of the sink-rate flare, in ds (20 = 2 s). Longer is gentler and takes longer over the ground; it should be at least twice the time the aircraft takes to answer a pitch command with its flight path, about a second on a small model.
  Min: 5, Max: 60, Default: 20

* `nav_fw_land_approach_speed`: Airspeed the landing holds with the throttle, through the Auto Speed controller, from the wind circle to the throttle cut (kept within `fw_auto_speed_min_speed` and `fw_auto_speed_max_speed`). The wind circle is planned for it instead of the speed the RTH arrives with, so with flaps a lower approach speed gives a smaller circle, gentler turns and a slower touchdown. Half the headwind along the final is added to it, as pilots add, and the final's groundspeed planned with what is left. On the circuit's descending legs the throttle may go below `nav_fw_min_thr`, down to idle, to hold it: with flaps on the final the aircraft can be faster than the approach speed at the cruise minimum. It needs a valid airspeed: a pitot, or `pitot_hardware` VIRTUAL, which estimates it from the GPS and the wind estimate. It is never held on ground speed: downwind, ground speed above the airspeed would fly the aircraft slower than set. Without a valid airspeed, or at 0, the throttle follows the pitch as in the rest of the navigation.
  In cm/s. Min: 0, Max: 5000, Default: 0

* `nav_fw_land_max_tailwind`: Max. tailwind if no landing direction with downwind is available. Wind strengths below this value are ignored (error tolerance in the wind measurement). Landing then takes place in the main direction. If, for example, 90 degrees is configured, landing takes place in this direction, NOT in 270 degrees (see above).
In cm/s. Min: 0; Max: 3000, Default: 140

### Landing circuit

By default the wind circle is flown over the landing site and the turning point before the final is placed 4 loiter radii beside the runway (or half the approach length, whichever is longer). Small fields with trees or buildings on one side want the whole pattern on the other side, at a known distance, with a final that is really as long as configured. The three settings below do that; all keep the default behaviour at 0.

* `nav_fw_land_downwind_offset`: Distance between the edge of the wind circle and the landing site. The wind circle is flown abeam the landing site on the side given by the approach direction (seen from approach heading 1, whatever the landing direction, so always the same physical side of the runway), its centre one radius further out. The downwind leg is the circle's tangent parallel to the runway on its far side, two radii further out. The aircraft leaves the circle heading down it; when `fw_loiter_direction` turns the circle towards the runway for the landing direction the wind chose (right turns with the circle on the left of the landing direction, left turns on the right), that heading comes on the circle's near side, so the aircraft leaves heading straight out instead, on the side of the final, flies straight for 1.5 s to roll level and turns 90 degrees onto the downwind, which then lies further out by that straight and the turn. Then a base leg towards the runway and the final.
In cm. Min: 0, Max: 50000, Default: 0 (classic approach)

* `nav_fw_land_loiter_radius`: Radius of the wind circle. The aircraft cannot fly a circle tighter than its coordinated turn at the approach speed and `nav_fw_bank_angle` (V^2 / (g tan bank), 37 m at 16 m/s and 35 degrees), so the circle grows to that, at the speed the landing starts with. The radius is then fixed and the loiter controller holds exactly it; if the aircraft flies it wider (faster than planned), the circle grows to the radius flown after each full turn and moves out with it, so its edge stays at the offset. The circle altitude is reached slowly (0.5 m/s), so the descent does not build speed. The circuit turns onto the final one turn radius before the start of the final, so the straight final is `nav_fw_land_approach_length` long.
In cm. Min: 0, Max: 30000, Default: 0 (uses `nav_fw_loiter_radius`)

* `nav_fw_land_final_alt`: Altitude above the landing site at the start of the straight final. The downwind and base legs descend evenly to it from the altitude the circle is left at (the approach altitude give or take the loiter's error, so the path has no step there) with the distance flown since the circle, the straight final from it to the ground at the aim point. The turns cut the corners, so the final altitude is reached a little before the straight final and the turn onto it is about level. The altitude target leads along the path by the altitude controller's lag on a slope (100 / `nav_fw_alt_control_response` seconds), so the aircraft stays on the path. The circuit's turns are flown at the wind circle's radius, not tighter.
In cm. Min: 0, Max: 30000, Default: 0 (two thirds of the approach altitude)

* `nav_fw_land_aim_offset`: The glide path meets the ground this far before the landing site, in either landing direction (with `nav_fw_land_flare_sink` in the landing circuit, the wheels touch here). After the throttle cut at `nav_fw_land_glide_alt` the aircraft floats while it slows down (a 1.2 m model at 14 m/s floats 50 to 60 m), so aiming short puts the touchdown on the site. Works with the classic approach too.
In cm. Min: 0, Max: 10000, Default: 0

An RTH that will land with the circuit flies towards the far side of the wind circle instead of the landing site, and the landing takes over where the circle's own approach starts (3.7 radii from its centre), so the aircraft joins the circle tangentially and never circles over the runway.

### General paramters and tuning tips

* `nav_fw_wp_tracking_accuracy`: Its highly recommended that this parameter is used and tuned well. Only with WP-Tracking enabled, the Aircraft will try to precisely align with the runway during approach. 
If WP-Tracking is not used, the Plane will head straight to the landiung location without flying in line with the intended landing strip. Wind can intensively alter the final landing heading.

* `nav_fw_pitch2thr`: The navigation throttle modifier has to be tuned well to allow stable navigation during climbs and descents to prevent a stall. Make sure your plane maintains Ground or Airspeed, when climbing in any navigation mode. 
The Craft should not get slower and not speed ub significantly during a navigation climb, if P2T is tuned properly. See `Fixed Wing Pitch To Throttle Tuning.md` for a full tuning procedure.

* `nav_wp_radius`: The turning points for the Crosswind-Leg and Final Approach are calculated automatically from the aircraft's coordinated turn radius; `nav_wp_radius` does not shape the approach corners. 
Make sure this parameter is not set greater than 1000 (cm) so approach waypoints are not detected as reached too early.

* Test your Navigation-Tuning: A better Navigation-Tune will reward you with smoother and more reliable landings. To test your nav systems limit, we recommend to create a waypoint missions with many 90° turn angles with shorter and shorter tracks. 
With this Method, you can find out how well your plane can follow a navigation path and how long it takes to align to a waypoint track. A well tuned plane should be able to pull of a WP Mission that looks like this, where the distance between WP6 and WP7 si recommended to be the minimum approach length: 

![Test Waypoint Track](https://github.com/iNavFlight/inav/assets/33039058/a929cd0c-80b1-42d6-815d-89a90e9daa1b)


## Waypoint missions

Only one landing waypoint per mission can be active and saved and the landing waypoint must be the last waypoint of the mission. 
If the altitude of the waypoint and the "Approach Altitude" are different, the altitude of the waypoint is approached first and then the altitude is corrected to "Approach Altitude".

## Logic Conditions

The current landing state can be retrieved via ID 41 in "Flight" (FW Land State). This allows additional actions to be executed according to the landing phases, e.g. deployment of the landing flaps.

| Returned value | State |
| --- | --- |
| 0 | Idle/Inactive |
| 1 | Loiter |
| 2 | Downwind |
| 3 | Base Leg |
| 4 | Final Approach |
| 5 | Glide |
| 6 | Flare |
