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
- Add additional Zones as you like, Zones can be separated but also overlapping (See [Limitations]( ) for details

## Functions and Behaviors
- 
