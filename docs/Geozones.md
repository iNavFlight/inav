# Geozones

## Introduction
The Geozone feature allows pilots to define one or multiple areas on the map in Mission Control, to prevent accidental flying outside of allowed zones (Flight-Zones, FZ) or to avoid certain areas they are not allowed or not willing to fly in (No-Flight-Zone, NFZ). 
This type of feature might be known to many pilots as a "Geofence" and despite providing the same core functionality, INAV Geozones are significantly more versatile and provide more safety and features. 

Geozones can not only inform the Pilot on the OSD, if the aircraft is approaching a geozone border, it also gives the distance and direction to the closest border and the remaining flight distance to the breach point. Additionally, it provides autonomous avoidance features, if the Aircraft is in any kind of self-leveling flight mode. 
The most important feature for safety is the automatic path planning for RTH (Start Return To Home), that automatically avoids NFZ areas if possible. 

![image](https://github.com/user-attachments/assets/48e3e5cc-8517-4096-8497-399cf00ee541)


## Compatibility
- INAV Version: 8.0 or later
- INAV Configurator: 8.0 or Later
- [MWPTools:](https://github.com/stronnag/mwptools) Snapshot 2024-11-15 or later
- Only flight controller with more than 512k of Flash (STM32F405, STM32F765, STM32H743, etc.)
- Plane, Multirotor (Rover and Boat are untested at time of writing)

## Setup Procedure
- In the INAV Configurator, switch to the Configuration Panel and enable "Geozone" in the features. 
- Switch to the Mission Control Panel and you will see a new Icon to call up the Geozone Editor. If Zones are already set up, they will be loaded automatically, as soon as you enter Mission Control.
  ![image](https://github.com/user-attachments/assets/23cd2149-b6a2-4945-b349-ee99863e74f0)
- Click on the "+" Symbol to add a new zone and define its parameters to your desire.
- The following Options are available:
  - Shape: Configures a Zone as a Circle or a Polygon
  - Type: Inclusive (FZ, green) or Exclusive (NFZ, red)
  - Min. Alt (cm): lower ceiling of the Zone (0 represents the ground relative from the launch location or AMSL. No action will be taken at a minimum altitude of 0, so the aircraft can "dive out" of an Inclusive FZ on a hill. To have a Minimum Altitude action, set a negative altitude of -1 or lower)
  - Max. Alt (cm): upper ceiling of the Zone (A value if 0 means no upper altitude limit)
  - Action: Action to execute if an aircraft approaches the border of that Zone
  - Radius: Circular Zone only, Radius of the Circle
- Move the Zone-Markers to the desired locations, to create a bordered area with the shape and size needed (Or change the radius in case of a Circular Zone)
- To add additional vertices, click on the borderline of the zone you are editing. This will add a new vertex to that line to move around.
  ![image](https://github.com/user-attachments/assets/eacb6d3c-62d3-4bab-8874-1543c0a6b06d)
- Add additional Zones as you like, Zones can be separated but also overlapping (See [Limitations]( ) for details)
- After finishing the zone setup, click the "Store in EEPROM" Button to save the zones on the Flight Controller. It is important that the FC reboots after storing, as the Zones can only be used after a fresh boot process.
  ![image](https://github.com/user-attachments/assets/4b278dd0-aa65-45f6-b914-22bdd753feaf)

## Global Settings
- In the Advanced Tuning Panel, you will find additional global settings for Geozones
  ![image](https://github.com/user-attachments/assets/db567521-e256-4fb6-8ca6-6e6b8b57d7a9)
  - Detection Distance `geozone_detection_distance`: Defines at what distance a Geozone will be shown as a System Message if a breach is imminent.
  - Avoid Altitude Range `geozone_avoid_altitude_range`: When the Aircraft approaches an NFZ that has a upper limit (can be overflown at higher altitude), INAV will climb above the Zone automatically if the altitude difference between Zone ceiling and current Aircraft altitude is less, than this value. For fixed wing, you need to consider how steep the possible climb angle is.
  - Safe Altitude Distance `geozone_safe_altitude_distance`: Vertical safety margin to avoid a ceiling or floor altitude breach at high vertical speed. If your FZ Ceiling is at 100m and this value set to 10m, the aircraft will not allow you to fly above 90m and descents if the Aircraft overshoots.
  - Safehome as Inclusive `geozone_safehome_as_inclusive`: Defines every Safehome location as a circular Inclusive zone with the radius of `safehome_max_distance` to allow a FZ at ground altitude (For Landings) if the general FZ around it might have a minimum altitude.
  - Safehome Zone Action `geozone_safehome_zone_action`: Defines the action on zone breach if Safehome is enabled as inclusive. This is helpful for flying fields with distance or altitude restrictions for LOS Pilots.
  - Multirotor Stop Distance `geozone_mr_stop_distance:`: Distance from the Border a Multirotor will stop, if the Fence Action is Avoid or Loiter (For fixed wings, this will be calculated from the Loiter-Radius of the Plane).
  - No Way Home Action `geozone_no_way_home_action`: If RTH cannot find a possible route in FS or RTH modes, the Aircraft will either emergency land or fly straight back home and ignores NFZ. 

## Functions and Behaviors
- Zone Type: Inclusive
  - If craft is armed inside the Inclusive FZ, everything outside that zone is considered an NFZ.
  - Inclusive FZ can be combined if they overlap and will be handled as one zone.
  - Overlapping multiple FZ allows different upper and lower altitude limits for each zone, as long as they still overlap in 3D Space (Both overlapping zones have to have a overlapping altitude range as well).
  - Arming the aircraft outside of an Inclusive Zone is prohibited within a 2km distance to the next vertex (Distance to a border between two vertex is not checked). Arming override can be used. Arming at a distance bigger than 2km is possible. 
  - Arming a craft outside of an Inclusive FZ will disable all Inclusive zones. 
- Zone Type: Exclusive
  - Arming inside an Exclusive NFZ is prohibited. Arming override can be used but not recommended. If armed inside an NFZ the Pilot keeps control until the NFZ is left, then any configured Action will be executed on re-enter.
  - Exclusive Zones can be combined and overlapped as needed.
  - Exclusive NFZ with an upper or lower limit other than 0 can be overflown and underflown. The Automatic avoidance will only fly above NFZ if possible and never below.
- Actions:
  - Avoid: Also called “Bounce” (only airplanes): The aircraft flies away from the boundary at the same angle it approached it, like a pool ball bouncing off the table border. Multirotor will switch into "Position Hold".
  - Hold: Position in front of the boundary is held. Airplances will adjust their loiter center according to the loider radius, to stay away from the border while circling.
  - RTH: Triggers Return To Home. The Failsafe RTH Procedure is executed, so RTH Trackback is also used if enabled for Failsafe situations.
  - None: No action (only info in OSD).
- RTH:
  - If RTH is enabled by Failsafe, User Command or Zone Action, INAV will calculate a path to the Home Location that automatically avoids NFZ and tries to stay inside the current FZ.
  - If no Path can be calculated (Not able to climb over a blocking NFZ, No Intersection between FZ, too tight gaps between blocking NFZ) a configurable alternative action will be executed.
    - Direct RTH: Ignores Flight zones and comes back in a direct path.
    - Emergency Land: Executes a GPS enabled Emergency Landing (Circle down with horizontal position hold active on Planes).
  - When direct "Line of sight" with the Home location is reached (No zones blocking path), regular RTH settings are executed.
  - To abort the Smart-RTH feature and come back on a direct way, disable and Re-Enable RTH within 1 Second. This temporarily ignores all FZ and NFZ borders.
- Return to Zone:
  - If the Aircraft breaches into an NFZ or out of a FZ (by avoiding tight angled Zones or flown in Acro mode and then the mode switches to any Self-Level mode), RTZ is initiated and the aircraft flies back into the last permitted zone on the shortest possible course.

## OSD Elements
- Three dedicated OSD Elements have been added:
  - Fence-Distance Horizontal shows the distance to the nearest Fence Border and the heading to that border. (ID 145)
  - Fence-Distance Vertical shows the distance to the nearest ceiling or floor of a zone. (ID 146)
  - Fence-Direction Vertical is an optional element to show if the nearest vertical border is above or below the aircraft. (ID 144)
  ![image](https://github.com/user-attachments/assets/87dd3c5a-1046-4bd4-93af-5f8c9078b868)
- The Flight-Mode will show AUTO if the Aircraft executes any kind of Fence-Action.
- The System-Message shows the distance to a potential fence breach point, based on the current aircraft Attitude and Heading.
- Additionally, the System Message shows the current Fence Action that is Executed.

  
## Limitations
- The maximum number of dedicated zones of any type is 63.
- The maximum number of vertices of all zones combined is 127. Every circular zone uses 2 vertices while every polygon has to consist of at least 3 vertices.
- INAV can only execute one border-breach action at a time. This is especially important to consider for Airplanes that cannot hover. 
  - Complicated Zone setups with overlaps and tight areas can cause a loiter or "bounce" into another NFZ that was not considered before.
  - This can lead to a "Return to FZ" action that tries to find the shortest path into an allowed area.
- All Geozone Actions are disabled when in Waypoint Mode. The Pilot is responsible for planning his mission accordingly, to not create a path that crosses NFZ areas. If a mission leads to such an area and the pilot disables WP mode, a "Return to FZ" action will be executed.
- All Geozone Actions are disabled in ACRO and MANUAL Mode. INAV will not take over control in these modes and only OSD Warnings are shown.
- Planning the Geozone as a polygon, needs to have the vertex numbering to be counter clockwise in ascending order. One vertex must not be dragged over another border to create crossing borders within one zone. INAV Configurator and MWP will check for that before uploading.
  - Examples of Zones that are not allowed:
    ![image](https://github.com/user-attachments/assets/50f1a441-39da-4f1c-9128-7375bc593fa5)
- To properly combine multiple Inclusion FZ into one area, the Zones need to overlap at 2 borders and the points where the borders touch, must be at least 2.5x Loiter-Radius apart from Airplanes at least 2.5x Multirotor Stop Distance apart for Multirotor.
  - Example:
    ![image](https://github.com/user-attachments/assets/cc50e24b-dc83-4408-bcba-90d6da33eb63)
- If multiple zones with different minimum and maximum altitudes are combined, they need to vertically overlap at least 50m.
- There is a chance that Smart RTH cannot find a path around NFZ areas, if there are multiple very big zones blocking the path. Due to hardware limitations, the amount of waypoints that Smart RTH can create are limited. Many Zones with very long border lines (>500m) cause additional waypoints.
- It is not recommended to edit geozones in CLI by hand as this bypasses a lot of sanity checks. Potential errors in zones will disable them or can lead to unexpected behaviors. Transferring Geozones with a DIFF between aircraft is fine.

## CLI
The Geozone Information are stored in two separate data arrays. The first array holds the main Geozone Information and settings. The second array holds the Geozone vertices. 
The following commands are available for users: 

- `geozone` without argument lists the current settings
- `geozone reset <id>` resets a specific geozone and all related vertices. If no ID proveded, all geozones and vertices will be deleted.
- `geozone vertex` - lists all vertices.
- `geozone vertex reset` - deletes all vertices.
- `geozone vertex reset <zone id>` - Deletes all vertices of the zone.
- `geozone vertex reset <zone id> <vertex id>` - Deletes the vertex with the corresponding id from a zone.

The following information are for app-developers. _DO NOT EDIT GEOZONES MANUALLY CLI_!

`geozone <id> <shape> <type> <minimum altitude> <maximum altitude> <is_amsl> <fence action> <vertices count>`

- id: 0 - 63
- shape: 0 = Circular, 1 = Polygonal
- type: 0 = Exclusive, 1 = Inclusive
- minimum altitude: In centimetres, 0 = ground
- maximum altitude: In centimetres, 0 = infinity
- is_amsl: 0 = relative, 1 = AMSL
- fence action: 0 = None, 1 = Avoid, 2 = Position hold, 3 = Return To Home
- vertices count: 0-126 - Sanity check if number of vertices matches with configured zones

`geozone vertex <zone id> <vertex idx> <latitude> <logitude>`

- zone id: (0-63) The zone id to which this vertex belongs
- vertex idx: Index of the vertex (0-126)
- latitude/ logitude: Longitude and latitude of the vertex. Values in decimal degrees * 1e7. Example:the value 47.562004o becomes 475620040


