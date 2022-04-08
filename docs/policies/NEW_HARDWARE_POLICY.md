# INAV Hardware Policy

## General

To prevent explosive growth of different target and feature count and ensure reasonable quality and usability of the firmware the following policy is mandatory for all new hardware additions (including variants of the same target).

## Definitions

"Target" - part of the source code responsible for providing a new build artifact (hex-file).

"Hardware" - physical hardware requiring support from firmware side.

"Requester" - manufacturer or community member seeking addition of new hardware or target

## New target additions

New targets are accepted into INAV code if any of the following conditions is satisfied:

1. Requester is a manufacturer of the hardware or an affiliated community member. Requester provides specs, schematics and production samples are provided to at least two core developers. In this case the new target becomes part of official INAV release.

2. Requester is a community member not affiliated with a manufacturer of the hardware. Requester provides board samples to at least one core developer and the target is already supported in official Cleanflight or Betaflight firmware. In this case the new target may or may not become part of official release based on the decision made by core developers.

3. The new target must meet the following minimal requirements:

  * On-board sensors include at least the IMU (gyroscope + accelerometer)
  * At least 2 hardware serial ports are available with both TX and RX pins
  * At least 512K of firmware flash memory and at least of 64K of RAM available
  * At least one I2C bus broken out (SCL and SDA pins) and not shared with other functions 

## New hardware support

For the hardware which does not require a new target, but still require support from the firmware side the following rules apply:

1. Requester is a manufacturer of the hardware or an affiliated community member. Requester provides specs and production samples for the unsupported hardware to at least two core developers.

2. Requester is a community member not affiliated with a manufacturer of the hardware. Requester provides hardware samples to at least one core developer and the firmware support already exists in official Cleanflight or Betaflight firmware.

## Using own hardware exception

If one of the core developers has the hardware in possession they may opt in and submit support for it anyway. In this case the support is not official and may not be included in official releases.

## Providing samples to developers

1. Hardware provided to the developers would be considered a donation to the INAV project. Under no circumstances developer will be obliged to return the hardware or pay for it.

2. Requester bears all the costs of the hardware, delivery and VAT/customs duties. Hardware manufacturer also assumes the responsibility to provide up to date specs, documentation and firmware (if applicable).

3. Before sending samples the Requester should reach out to developers and agree on specific terms of implementing support for the unsupported hardware. Developers may place additional requirements on a case by case basis and at their sole discretion.

4. The new target and new hardware policies do not apply in the following cases. Developers may still chose to apply the "own hardware exception" at their own discretion.

  * if the receiving developer has to bear part of the costs (and is not reimbursed by the sender)
  * if the hardware provided does not behave according to specs (different communication protocol, undocumented or incorrect CPU pin mappings, damaged or dead on arrival) and the sender doesn't provide a way to resolve the problem (up to date docs, new firmware etc).
  * if the hardware was sent without reaching out to developers and agreeing on the terms first.

5. It's advised to provide more than one sample to avoid issues with damaged or dead on arrival hardware.

## Implementing support for the new target or hardware

1. Pull request to add the new target or hardware may be authored by a contributor outside INAV team or by one of the core developers. 

2. There is no obligation to accept a pull request made by an outside contributor. INAV team reserves the right to reject that pull request and re-implement the support or take that pull request as a baseline and amend it.

3. INAV team reserves the right to reject the new target or hardware or remove the support for an unforeseen reason including, but not limited to violation of [INAV Code of Conduct](CODE_OF_CONDUCT.md) by the manufacturer or an affiliated outside contributor.

## Guidelines on contacting the team

1. Requester is advised to open a feature request to add support for certain hardware to INAV by following [this link](https://github.com/iNavFlight/inav/issues/new/choose)

2. After opening a feature request, Requester is advised to contact the core development team by [email](mailto:coredev@inavflight.com) mentioning the open feature request and communicate with developer team via email to arrange hardware and specifications delivery.
