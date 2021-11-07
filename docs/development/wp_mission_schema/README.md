# XML Mission File Definition

## Overview

Historically, mission planners interoperating with inav (and multiwii) missions have used the XML mission file format defined by EOSBandi for MultiWii 2.3 (2013).

The format is defined the XSD schema here.

* Lower case tags are preferred by inav. Older tools may prefer uppercase (the original MW usage).
* For inav 4.0 and later, the `missionitem/flags` attribute is required for "fly-by home" waypoints and multi-mission files.
* For inav 4.0 and later, multi-mission files; mission segments are delimited by a flag value of `165` (the MSP protocol value).
* For multi-mission files, the waypoints may be numbered either sequentially across the whole file, or "reset-numbering" within each mission segment. The latter may (or not) be considered to be more "human readable".
* The `mwp` tag was introduced by the eponymous mission planner. Other mission planners may consider that reusing some of the tags (`cx`, `cy` - centre location, `zoom` TMS zoom level) is useful.
* The `version` tag may be intepreted by mission planners as they see fit. For example, the (obsolete) Android 'ez-gui' application requires '2.3-pre8'. For multi-mission files it is recommended to use another `version`.

## Examples

### Multi-mission file with sequential numbering

```
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<mission>
	<version value="2.3-pre8"/>
	<mwp cx="-3.2869291" cy="54.5722109" home-x="0" home-y="0" zoom="14"/>
	<missionitem no="1" action="WAYPOINT" lat="54.5722109" lon="-3.2869291" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"/>
	<missionitem no="2" action="WAYPOINT" lat="54.5708178" lon="-3.2642698" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"/>
	<missionitem no="3" action="WAYPOINT" lat="54.5698227" lon="-3.2385206" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="165"/>
	<missionitem no="4" action="WAYPOINT" lat="54.5599696" lon="-3.2958555" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"/>
	<missionitem no="5" action="WAYPOINT" lat="54.5537978" lon="-3.2958555" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"/>
	<missionitem no="6" action="WAYPOINT" lat="54.5547933" lon="-3.2864141" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"/>
	<missionitem no="7" action="WAYPOINT" lat="54.5597705" lon="-3.2695913" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"/>
	<missionitem no="8" action="WAYPOINT" lat="54.555291" lon="-3.2598066" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"/>
	<missionitem no="9" action="JUMP" lat="0" lon="0" alt="0" parameter1="1" parameter2="0" parameter3="0" flag="165"/>
	<missionitem no="10" action="WAYPOINT" lat="54.5714148" lon="-3.2501936" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="165"/>
</mission>
```

### Multi-mission file with "reset" numbering and per-segment metadata

```
<?xml version="1.0" encoding="utf-8"?>
<mission>
  <!--mw planner 0.01-->
  <version value="42"></version>
  <mwp save-date="2021-11-05T11:02:39+0000" zoom="14" cx="-3.2627249" cy="54.5710168" generator="mwp (mwptools)"><details><distance units="m" value="3130"></distance></details></mwp>
  <missionitem no="1" action="WAYPOINT" lat="54.5722109" lon="-3.2869291" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="2" action="WAYPOINT" lat="54.5708178" lon="-3.2642698" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="3" action="WAYPOINT" lat="54.5698227" lon="-3.2385206" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="165"></missionitem>
  <mwp save-date="2021-11-05T11:02:39+0000" zoom="15" cx="-3.2778311" cy="54.5568837" generator="mwp (mwptools)"><details><distance units="m" value="3324"></distance><nav-speed units="m/s" value="10"></nav-speed><fly-time units="s" value="344"></fly-time><loiter-time units="s" value="0"></loiter-time></details></mwp>
  <missionitem no="1" action="WAYPOINT" lat="54.5599696" lon="-3.2958555" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="2" action="WAYPOINT" lat="54.5537978" lon="-3.2958555" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="3" action="WAYPOINT" lat="54.5547933" lon="-3.2864141" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="4" action="WAYPOINT" lat="54.5597705" lon="-3.2695913" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="5" action="WAYPOINT" lat="54.5552910" lon="-3.2598066" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="6" action="JUMP" lat="0.0000000" lon="0.0000000" alt="0" parameter1="1" parameter2="0" parameter3="0" flag="165"></missionitem>
  <mwp save-date="2021-11-05T11:02:39+0000" zoom="20" cx="-3.2501936" cy="54.5714148" generator="mwp (mwptools)"><details><distance units="m" value="0"></distance></details></mwp>
  <missionitem no="1" action="WAYPOINT" lat="54.5714148" lon="-3.2501936" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="165"></missionitem>
</mission>
```
