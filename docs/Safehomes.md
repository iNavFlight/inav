# INav - Safehomes

## Introduction

The "Home" position is used for the landing point when landing is enabled or in an emergency situation.  It is usually determined by the GPS location where the aircraft is armed. 

For airplanes, the landing procedure is explained very well by Pawel Spychalski [here.](https://quadmeup.com/inav-1-8-automated-landing-for-fixed-wings/)

<img src="https://quadmeup.com/wp-content/uploads/2017/06/fixed-wing-landing-1024x683.png" width="600">

One potential risk when landing is that there might be buildings, trees and other obstacles in the way as the airplance circles lower toward the ground at the arming point.  Most people don't go the middle of the field when arming their airplanes.

## Safehome

Safehomes are a list of GPS coordinates that identify safe landing points.  When the flight controller is armed, it checks the list of safehomes.  The first one that is enabled and within 200m of the current position will be selected.  Otherwise, it reverts to the old behaviour of using your current GPS position as home.  

You can define up to 8 safehomes for different locations you fly at.

When you are choosing safehome locations, ensure that the location is clear of obstructions for a radius more than 50m (`nav_fw_loiter_radius`).  As the plane descends, the circles aren't always symmetrical, as wind direction could result in some wider or tighter turns.  Also, the direction and length of the final landing stage is also unknown.  You want to choose a point that has provides a margin for variation and the final landing.

## OSD Message when Armed

When the aircraft is armed, the OSD briefly shows `ARMED` and the current GPS position and current date and time.

If a safehome is selected, an additional message appears:
```
     H - DIST -> SAFEHOME n  <- New message
                                n   is the Safehome index (0-7)
            ARMED               DIST is the distance from   
        GPS LATITUDE                 your current position to this safehome
        GPS LONGITUDE
        GPS PLUS CODE
        
         CURRENT DATE
         CURRENT TIME
```
The GPS details are those of the selected safehome.
To draw your attention to "HOME" being replaced, the message flashes and stays visible longer.

## CLI command `safehome` to manage safehomes

`safehome` - List all safehomes

`safehome reset` - Clears all safehomes.

`safehome <n> <enabled> <lat> <lon>` - Set the parameters of a safehome with index `<n>`.

Parameters:

  * `<enabled>` - 0 is disabled, 1 is enabled.
  * `<lat>` - Latitude (WGS84), in degrees * 1E7 (for example 123456789 means 12.3456789).
  * `<lon>` - Longitude.

Safehomes are saved along with your regular settings and will appear in `diff` and `dump` output.  Use `save` to save any changes, as with other settings.

### `safehome` example

```
# safehome
safehome 0 1 543533193 -45179273
safehome 1 1 435464846 -78654544
safehome 2 0 0 0 
safehome 3 0 0 0 
safehome 4 0 0 0 
safehome 5 0 0 0 
safehome 6 0 0 0 
safehome 7 0 0 0 

```

