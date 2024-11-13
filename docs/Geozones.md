# Geozones

## Introduction
The Geozone feature allows pilots to define one or multiple areas on the map in Mission Control, to prevent accidental flying outside of allowed zones (Flight-Zones, FZ) or to avoid certain areas they are not allowed or not willing to fly in (No-Flight-Zone, NFZ). 
This type of feature might be known to many pilots as a "Geofence" and despite providing the same core functionality, INAV Geozones are significantly more versatile and provide more safety and features. 

Geozones can not only inform the Pilot on the OSD, if the aircraft is approaching a geozone border, it also gives the distance and direction to the closest border and the remaining flight distance to the breach point. Additionally it provides autonomous avoidance features, if the Aircraft is in any kind of self-leveling flight mode. 
The most important feature for safety, is the automatic path planning for RTH (Return To Home), that automatically avoids NFZ areas if possible. 

![image](https://github.com/user-attachments/assets/48e3e5cc-8517-4096-8497-399cf00ee541)


## Compatibility
- INAV Version: 8.0 or later
- INAV Configurator: 8.0 or Later
- [MWPTools:](https://github.com/stronnag/mwptools) Snapshot 2024-11-xx or later
- Only flight controller with more than 512k of Flash (STM32F405, STM32F765, STM32H743, etc.)
- Plane, Multirotor (Rover and Boat are untested at time of writing)

## Setup Procedure
- In the INAV Configurator, switch to the Configuration Panel and enable "Geozone" in the features. 
- Switch to the Mission Control Panel and you will see a new Icon to call up the Geozone Editor. If Zones are already set up, they will be loaded automatically, as soon as the you enter Mission Control. 
- Click on the "+" Symbol to add a new zone and define its parameters to your desire.
- The following Options are available:
  - Shape: Configures a Zone as a Circle or a Polygon
  - Type: Inclusive (FZ, green) or Exclusive (NFZ, red)
  - Min. Alt (cm): lower ceiling of the Zone
  - Max. Alt (cm): upper ceiling of the Zone
  - Action: Ection to execute if an aircraft approaches the border of that Zone
  - Radius: Circular Zone only, Radius of the Circle
- Move the Zone-Markers to the desired locations, to create a bordered area with the needed shape and size (Or change the radius in case of a Circular Zone)
- To add additional vertices, click on the border-line of the zone you are editing. This will add a new vertex on that line to move around.
- Add additional Zones as you like, Zones can be separated but also overlapping (See [Limitations]( ) for details)

## Additional Settings
- 

## Functions and Behaviors
- Zone Type: Inclusive
  - If craft is armed inside the Inclusive FZ, everything outside that zone is considered a NFZ.
  - Inclusive FZ can be combined if they overlap and will be handled as one zone.
  - Overlapping multiple FZ allows different upper and lower altitude limits for each zone, as long as they still overlap in 3D Space (Both overlapping zones have to have a overlapping altitude range as well).
  - Arming the aircraft outside of an Inclusive Zone is prohibited within a 2km distance to the next vertex (Distance to a border between two vertex is not checked). Arming override can be used. Arming at a distance bigger than 2km is possible. 
  - Arming a craft outside of an Inclusive FZ will disable all Inclusive zones. 
- Zone Type: Exclusive
  - Arming inside an Exclusive NFZ is prohibited. Arming override can be used but not recommended. If armed inside a NFZ the Pilot keeps control until the NFZ is left, then any configured Action will be executed on re-enter.
  - Exclusive Zones can be combined and overlapped as needed
  - Exclusive NFZ with an upper or lower limit other than 0 can be overflown and underflown. The Automatic avoidance will only fly above NFZ if possible and never below.
- Actions:
  - Avoid: Also called “Bounce” (only airplanes): The aircraft flies away from the boundary “billiard ball effect”
  - Hold: Position in front of the boundary is held. Airplances will adjust their loiter center according to the loider radius, to stay away from the border while circling.
  - RTH: Triggers return to home. The Failsafe RTH Procedure is executed, so RTH Trackback is also used if enabled for Failsafe situations.
  - None: No action (only info in OSD)
- RTH:
  - If RTH is enabled by Failsafe, User Command or Zone Action, INAV will calculate a path to the Home Location that automatically avoids NFZ and tries to stay inside the current FZ.
  - If no Path can be calculated (Not able to climb over a blocking NFZ, No Intersection between FZ, too tight gaps between blocking NFZ) a configurable alternative action will be executed
    - Direct RTH: Ignores Flight zones and comes back in a direct path
    - Emergency Land: Executes a GPS enabled Emergency Landing (Circle down with horizontal position hold active on Planes)
  - When direct "Line of sight" with the Home location is reached (No zones blocking path), regular RTH settings are executed

## Limitations
- The maximum amount of dedicated zones of any type is 63.
- The maximum amount of vertices of all zones combined is 127. Every circular zone uses 2 vertices while every polygon has to consist of at least 3 vertices.
- INAV can only execute one border-breach action at a time. This is especially important to consider for Airplanes that can not hover. 
  - Complicated Zone setups with overlaps and tight areas can cause a Loiter or "bounce" into another NFZ that was not considered before.
  - This can lead to a "Return to FZ" action that tries to find the shortest path into a legal area.
- All Geozone Actions are disabled when in Waypoint Mode. The Pilot is responsible to plan his mission accordingly, to not create a path that crosses NFZ areas. If a mission leads to such area and the pilot disables WP mode, a "Return to FZ" action will be executed.
- All Geozone Actions are disabled in ACRO and MANUAL Mode. INAV will not take over control in these modes and only OSD Warnings are shown.
- Planning the Geozone as a polygon, needs to have the vertex numbering to be counter clock wise in ascending order. One vertex must not be dragged over another border to create crossing borders within one zone. INAV Configurator and MWP will check for that before upload. 
