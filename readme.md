These are unofficial targets for [INAV](https://github.com/iNavFlight/inav).
Some of the targets provide support for flight controller boards that do not have official INAV support.
Others add features to boards that are supported, such as adding PINIO or extra servo outputs.

Targets can be found in [src/main/target/](src/main/target/)

Targets available here include:
GEPRCF722
GRAVITYF7
IFLIGHTF7_SXEMINI
MATEKF405SE_PINIO
MATEKF405SE_PINIO2
OMNIBUSF4_PINIO:
OMNIBUSF4_PINIOPRO
OMNIBUSF4_PINIOV3
OMNIBUSF4_PINIOV3_S5_S6_2SS
OMNIBUSF4_PINIOV3_S5S6_SS
OMNIBUSF4_PINIOV3_S6_SS
SKYSTARSF405hd
SKYSTARSF722HDPRO
and more

New additions are very much welcomed. If you've made your own target, please
send a pull request or send me a zip of your files and I'll add it here.
Improvements are also very much welcomed. Any additions or improvements you
make, please send a PR or let me know.

Here is some information about how you can make your own target, either to support
an FC that isn't already supported, or to remap resources:
[Building Custom Firmware](https://github.com/iNavFlight/inav/wiki/Building-custom-firmware)

Getting a new target added to the *offical* inav distribution has certain
[requirements](https://github.com/iNavFlight/inav/blob/master/docs/policies/NEW_HARDWARE_POLICY.md).
On the other hand, all targets are welcome here.

Please add in your target folder a readme.txt file saying what your target is for and how you've tested it.

# INAV 6 Horizon Hawk feature freeze

INAV 6 feature freeze will happen on 29th of January 2023. No new features for INAv 6 will be accepted after that date.

# INAV Community


* Runs on the most popular F4, F7 and H7 flight controllers
* On Screen Display (OSD) - both character and pixel style
* DJI OSD integration: all elements, system messages and warnings
* Outstanding performance out of the box
* Position Hold, Altitude Hold, Return To Home and Missions
* Excellent support for fixed wing UAVs: airplanes, flying wings 
* Fully configurable mixer that allows to run any hardware you want: multirotor, fixed wing, rovers, boats and other experimental devices
* Multiple sensor support: GPS, Pitot tube, sonar, lidar, temperature, ESC with BlHeli_32 telemetry
* SmartAudio and IRC Tramp VTX support
* Blackbox flight recorder logging
* Telemetry: SmartPort, FPort, MAVlink, LTM
* Multi-color RGB LED Strip support
* Advanced gyro filtering
* Logic Conditions, Global Functions and Global Variables: you can program INAV with a GUI
* And many more!


Note we cannot generally provide support for the code here. This is just a 
collection of targets that have been contributed by people like you. The maintainer
of this repository (Sensei) didn't write this code and may not own the hardware.
The only exception is Airbot Omnibus boards. The maintainer of this repo
does own Omnibus flight controllers and is familiar with ONLY those boards.

These files are not a product of the official inav project and are not endorsed
by the project or affiliated with it. These files may or may not work well for you.
They have been provided by people who made them for their own use and report that the files work for them.


Official tool for INAV can be downloaded [here](https://github.com/iNavFlight/inav-configurator/releases). It can be run on Windows, MacOS and Linux machines and standalone application.  

--------------------------------------------------------

### INAV Blackbox Explorer

Tool for Blackbox logs analysis is available [here](https://github.com/iNavFlight/blackbox-log-viewer/releases)

### Telemetry screen for OpenTX

Users of OpenTX radios (Taranis, Horus, Jumper, Radiomaster, Nirvana) can use INAV OpenTX Telemetry Widget screen. Software and installation instruction are available here: [https://github.com/iNavFlight/OpenTX-Telemetry-Widget](https://github.com/iNavFlight/OpenTX-Telemetry-Widget)

### INAV magnetometer alignment helper

[INAV Magnetometer Alignment helper](https://kernel-machine.github.io/INavMagAlignHelper/) allows to align INAV magnetometer despite position and orientation. This simplifies the process of INAV setup on multirotors with tilted GPS modules.

### OSD layout Copy, Move, or Replace helper tool

[Easy INAV OSD switcher tool](https://www.mrd-rc.com/tutorials-tools-and-testing/useful-tools/inav-osd-switcher-tool/) allows you to easily switch your OSD layouts around in INAV. Choose the from and to OSD layouts, and the method of transfering the layouts.

## Installation

See: https://github.com/iNavFlight/inav/blob/master/docs/Installation.md

## Documentation, support and learning resources
* [INAV 5 on a flying wing full tutorial](https://www.youtube.com/playlist?list=PLOUQ8o2_nCLkZlulvqsX_vRMfXd5zM7Ha)
* [INAV on a multirotor drone tutorial](https://www.youtube.com/playlist?list=PLOUQ8o2_nCLkfcKsWobDLtBNIBzwlwRC8)
* [Fixed Wing Guide](docs/INAV_Fixed_Wing_Setup_Guide.pdf)
* [Autolaunch Guide](docs/INAV_Autolaunch.pdf)
* [Modes Guide](docs/INAV_Modes.pdf)
* [Wing Tuning Masterclass](docs/INAV_Wing_Tuning_Masterclass.pdf)
* [Official documentation](https://github.com/iNavFlight/inav/tree/master/docs)
* [Official Wiki](https://github.com/iNavFlight/inav/wiki)
* [Video series by Paweł Spychalski](https://www.youtube.com/playlist?list=PLOUQ8o2_nCLloACrA6f1_daCjhqY2x0fB)
* [Target documentation](https://github.com/iNavFlight/inav/tree/master/docs/boards)

## Contributing

Contributions are welcome and encouraged.  You can contribute in many ways:

* Documentation updates and corrections.
* How-To guides - received help?  help others!
* Bug fixes.
* New features.
* Telling us your ideas and suggestions.
* Buying your hardware from this [link](https://inavflight.com/shop/u/bg/)

A good place to start is Telegram channel or Facebook group. Drop in, say hi.

Github issue tracker is a good place to search for existing issues or report a new bug/feature request:

https://github.com/iNavFlight/inav/issues

https://github.com/iNavFlight/inav-configurator/issues

Before creating new issues please check to see if there is an existing one, search first otherwise you waste peoples time when they could be coding instead!

## Developers

Please refer to the development section in the [docs/development](https://github.com/iNavFlight/inav/tree/master/docs/development) folder.


## INAV Releases
https://github.com/iNavFlight/inav/releases
