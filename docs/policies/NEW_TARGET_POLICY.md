# INAV Board Target policy

## General

To prevent explosive growth of different target count and ensure quality of new target definitions the following policy is mandatory for all new target additions (including variants of the same target). For the purpose of this document "target" is defined as part of the source code responsible for providing a new build artifact (hex-file).

## New target additions

New targets are accepted into INAV code if any of the following conditions is satisfied:

1. Board specs, schematics and samples (production, not prototype) are provided to at least two core developers by the board manufacturer. In this case the new target becomes part of official INAV release.

2. Board samples are provided to at least one core developer by a community member or board manufacturer and the target already exists in official Cleanflight or Betaflight firmware. In this case the new target may or may not become part of official release based on the decision made by core developers.

3. If one of the core developers has the unsupported board in posession they may opt in and submit the new target anyway. In this case the new target does *not* become part of official release.
