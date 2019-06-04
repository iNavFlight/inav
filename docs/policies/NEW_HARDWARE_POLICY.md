# INAV New Hardware policies

## General

To prevent explosive growth of different target and feature count and ensure reasonable quality and usability of the firmware the following policy is mandatory for all new hardware additions (including variants of the same target).

## Definitions

"Target" - part of the source code responsible for providing a new build artifact (hex-file).

"Hardware" - physical hardware requiring support from firmware side.

## New target additions

New targets are accepted into INAV code if any of the following conditions is satisfied:

1. Board manufacturer provides specs, schematics and production samples are provided to at least two core developers. In this case the new target becomes part of official INAV release.

2. Community member or board manufacturer provides board samples to at least one core developer and the target already exists in official Cleanflight or Betaflight firmware. In this case the new target may or may not become part of official release based on the decision made by core developers.

# New hardware support

For the hardware which does not require a new target, but still require support from the firmware side the following rules apply:

1. Hardware manufacturer provides specs and production samples for the unsupported hardware to at least two core developers.

2. Community member or hardware manufacturer provides hardware samples to at least one core developer and the firmware support already exists in official Cleanflight or Betaflight firmware.

# Using own hardware exception

If one of the core developers has the hardware in possession they may opt in and submit support for it anyway. In this case the support is not official and is generally not included in official releases.

# Providing samples to developers

1. Hardware provided to the developers would be considered a donation to the INAV project. Under no circumstances developer will be obliged to return the hardware.

2. Manufacturer or community member providing the hardware bears all the costs of the hardware, delivery and VAT/customs duties. Hardware manufacturer also assumes the responsibility to provide up to date specs, documentation and firmware (if applicable).

3. Before sending samples the providing party should reach out to developers and agree on specific terms of implementing support for the unsupported hardware. Developers may place additional requirements on a case by case basis and at their sole discretion.

4. The new target and new hardware policies do not apply in the following cases. Developers may still chose to apply the "own hardware exception" at their own discretion.

  * if the receiving developer has to bear part of the costs (and is not reimbursed by the sender)
  * if the hardware provided does not behave according to specs (different communication protocol, undocumented or incorrect CPU pin mappings, damaged or dead on arrival) and the sender doesn't provide a way to resolve the problem (up to date docs, new firmware etc).
  * if the hardware was sent without reaching out to developers and agreeing on the terms first.

5. It's advised to provide more than one sample to avoid issues with damaged or dead on arrival hardware.


