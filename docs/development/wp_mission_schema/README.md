# XML Mission File Definition

## Overview

Historically, mission planners interoperating with INAV (and multiwii) missions have used the XML mission file format defined by EOSBandi for MultiWii 2.3 (2013).

The format is defined the XSD schema here.

* Lower case tags are preferred by INAV. Older tools may prefer uppercase (the original MW usage).
* For INAV 4.0 and later, the `missionitem/flags` attribute is required for "fly-by home" waypoints and multi-mission files.
* For INAV 4.0 and later, multi-mission files; mission segments are delimited by a flag value of `165` (the MSP protocol 'last WP' value).
* For multi-mission files, the waypoints may be numbered either sequentially across the whole file, or "reset-numbering" within each mission segment. The latter may (or not) be considered to be more "human readable", particularly where `JUMP` is used.
* The `mwp` tag was introduced by the eponymous mission planner. Other mission planners may consider that reusing some of the tags (`cx`, `cy` - centre location, `zoom` TMS zoom level, `home-x`, `home-y` - home location) is useful.
* `meta` may be used as a synonym for `mwp`.
* The `version` tag may be intepreted by mission planners as they see fit. For example, the (obsolete) Android 'ez-gui' application requires '2.3-pre8'. For multi-mission files it is recommended to use another `version`.
* the `mwp` / `meta` element may be interleaved with `missionitem` in a multi-mission file to provide mission segment specific home, centre locations and zoom.

## Examples

### Multi-mission file with sequential numbering

```
<?xml version="1.0" encoding="UTF-8"?>
<mission>
 <version value="2.3-pre8"></version>
 <mwp zoom="14" cx="-3.2632398333333335" cy="54.570950466666666" home-x="0" home-y="0" save-date="2021-11-12T14:07:03Z" generator="impload"></mwp>
 <missionitem no="1" action="WAYPOINT" lat="54.5722109" lon="-3.2869291" alt="50" parameter1="0" parameter2="0" parameter3="0"></missionitem>
 <missionitem no="2" action="WAYPOINT" lat="54.5708178" lon="-3.2642698" alt="50" parameter1="0" parameter2="0" parameter3="0"></missionitem>
 <missionitem no="3" action="WAYPOINT" lat="54.5698227" lon="-3.2385206" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="165"></missionitem>
 <missionitem no="4" action="WAYPOINT" lat="54.5599696" lon="-3.2958555" alt="50" parameter1="0" parameter2="0" parameter3="0"></missionitem>
 <missionitem no="5" action="WAYPOINT" lat="54.5537978" lon="-3.2958555" alt="50" parameter1="0" parameter2="0" parameter3="0"></missionitem>
 <missionitem no="6" action="WAYPOINT" lat="54.5547933" lon="-3.2864141" alt="50" parameter1="0" parameter2="0" parameter3="0"></missionitem>
 <missionitem no="7" action="WAYPOINT" lat="54.5597705" lon="-3.2695913" alt="50" parameter1="0" parameter2="0" parameter3="0"></missionitem>
 <missionitem no="8" action="WAYPOINT" lat="54.555291" lon="-3.2598066" alt="50" parameter1="0" parameter2="0" parameter3="0"></missionitem>
 <missionitem no="9" action="JUMP" lat="0" lon="0" alt="0" parameter1="1" parameter2="0" parameter3="0" flag="165"></missionitem>
 <missionitem no="10" action="WAYPOINT" lat="54.5714148" lon="-3.2501936" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="165"></missionitem>
</mission>
```

### Multi-mission file with "reset" numbering and per-segment metadata with `meta` tag

```
<?xml version="1.0" encoding="utf-8"?>
<mission>
  <!--mw planner 0.01-->
  <version value="42"></version>
  <meta save-date="2021-11-12T14:22:05+0000" zoom="14" cx="-3.2627249" cy="54.5710168" home-x="-3.2989342" home-y="54.5707123" generator="mwp (mwptools)"><details><distance units="m" value="3130"></distance></details></meta>
  <missionitem no="1" action="WAYPOINT" lat="54.5722109" lon="-3.2869291" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="2" action="WAYPOINT" lat="54.5708178" lon="-3.2642698" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="3" action="WAYPOINT" lat="54.5698227" lon="-3.2385206" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="165"></missionitem>
  <meta save-date="2021-11-12T14:22:05+0000" zoom="15" cx="-3.2778311" cy="54.5568837" home-x="-3.2983737" home-y="54.5622331" generator="mwp (mwptools)"><details><distance units="m" value="9029"></distance><nav-speed units="m/s" value="10"></nav-speed><fly-time units="s" value="929"></fly-time><loiter-time units="s" value="0"></loiter-time></details></meta>
  <missionitem no="1" action="WAYPOINT" lat="54.5599696" lon="-3.2958555" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="2" action="WAYPOINT" lat="54.5537978" lon="-3.2958555" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="3" action="WAYPOINT" lat="54.5547933" lon="-3.2864141" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="4" action="WAYPOINT" lat="54.5597705" lon="-3.2695913" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="5" action="WAYPOINT" lat="54.5552910" lon="-3.2598066" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="0"></missionitem>
  <missionitem no="6" action="JUMP" lat="0.0000000" lon="0.0000000" alt="0" parameter1="1" parameter2="1" parameter3="0" flag="165"></missionitem>
  <meta save-date="2021-11-12T14:22:05+0000" zoom="20" cx="-3.2501936" cy="54.5714148" generator="mwp (mwptools)"><details><distance units="m" value="0"></distance></details></meta>
  <missionitem no="1" action="WAYPOINT" lat="54.5714148" lon="-3.2501936" alt="50" parameter1="0" parameter2="0" parameter3="0" flag="165"></missionitem>
</mission>
```
