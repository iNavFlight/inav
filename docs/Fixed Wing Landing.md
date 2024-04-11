# Fixed Wing Landing

## Introducion

INAV supports advanced automatic landings for fixed wing aircraft from version 7.1.
The procedure is based on landings for man-carrying aircraft, so that safe landings at a specific location are possible.
Supported are landings at safehome after "Return to Home" or at a defined LAND waypoint for missions. 
Every landing locations can be defined with a target point and 2 different approach headings (colinear to the landing strips) with exclusive direction or opposite directions allowed. 
This enables up to 4 different approach directions, based on the landing site and surrounding area. 

## General procedure:

1. After reaching Safehome/LAND Waypoint the altitude is corrected to "Approach Altitude".
2. The aircraft circles for at least 30 seconds to determine the wind direction and strength.
3. The landing direction and the approach waypoints are calculated on the basis of the measured wind parameters. If no headwind landing is possible or the wind strength is greater than "Max. tailwind" (see Global Parameters), return to point 2.
4. The landing is initiated. The aircraft flies the downwind course, "Approach Altitude" is held.
5. Base Leg: the altitude is reduced from 2/3 of "Approach Altitude".
6. Final Appraoch: The engine power is reduced using "Pitch2throttle modifier" to reduce speed and the altitude is reduced to "Land Altitude".
7. Glide: When "Glide Altitude" is reached, the motor is switched off and the pitch angle of "Glide Pitch" is held.
7. Flare: Only if a LIDAR/Rangefinder sensor is present: the motor remains switched off and the pitch angle of "Flare Pitch" is held
8. Landing: As soon as INAV has detected the landing, it is automatically disarmed, see setting `nav_disarm_on_landing`.

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
`fwapproach <index> <Approach altitude> <Land altitude> <Approach direction> <approach heading 1> <approach heading 2> <sea level>`

`fwapproach` has 17 slots in which landing parameters can be stored. In slot 0-7 the landing parameters for Safehome are stored, in 8 - 16 the parameters for waypoint missions. Only one landing point per mission can be saved. 

* index: 0 - 17, 0 - 7 Safehome, 8 - 16 Mission
* Approach direction: 0 - Left, 1 - Right. Always seen from the primary landing direction (positive value), i.e. whether the aircraft flies left or right turns on approach.
* Approach Altitude: Initial altitude of the approach, the altitude at which the wind direction is determined and the downwind approach, in cm
* Land Altitude: Altitude of the landing site, in cm
* Approach heading 1 and 2: Two landing directions can be set, values: 0 - +/-360. 0 = landing direction is deactivated. 
A positive value means that you can approach in both directions, a negative value means that this direction is exclusive.
Example: 90 degrees: It is possible to land in 90 degrees as well as in 270 degrees. -90 means that you can only land in a 90 degree direction.
This means that practically 4 landing directions can be saved.
* Sea Level: 0 - Deactivated, 1 - Activated. If activated, approach and land altitude refer to normal zero (sea level), otherwise relative altitude to the altitude during first GPS fix.

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

* `nav_fw_land_max_tailwind`: Max. tailwind if no landing direction with downwind is available. Wind strengths below this value are ignored (error tolerance in the wind measurement). Landing then takes place in the main direction. If, for example, 90 degrees is configured, landing takes place in this direction, NOT in 270 degrees (see above).
In cm/s. Min: 0; Max: 3000, Default: 140

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
