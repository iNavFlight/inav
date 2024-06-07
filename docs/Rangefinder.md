# Rangefinder

Rangefinders are devices used to measure Above Ground Altitude.

## Current state

Current support of rangefinders in INAV is very limited. They are used only to:

* landing detection for multirotors
* automated landing support for fixed wings
* _Experimental_ terrain following (Surface) flight mode activated with _SURFACE_ and _ALTHOLD_ flight mode

## Hardware

Following rangefinders are supported:

* SRF10 - experimental
* INAV_I2C - is a simple [DIY rangefinder interface with Arduino Pro Mini 3.3V](https://github.com/iNavFlight/inav-rangefinder). Can be used to connect when flight controller has no Trigger-Echo ports. 
* VL53L0X - simple laser rangefinder usable up to 75cm
* UIB - experimental
* MSP - experimental
* TOF10120 - small & lightweight laser range sensor, usable up to 200cm
* TERARANGER EVO - 30cm to 600cm, depends on version https://www.terabee.com/sensors-modules/lidar-tof-range-finders/#individual-distance-measurement-sensors
* NRA15/NRA24 - experimental, UART version

#### NRA15/NRA24
NRA15/NRA24 from nanoradar use US-D1_V0 or NRA protocol, it depends which firmware you use. Radar can be set by firmware
to two different resolutions. See table below.

| Radar | Protocol | Resolution      | Name in configurator |
|-------|----------|-----------------|----------------------|
| NRA15 | US-D1_V0 | 0-30m (+-4cm)   | USD1_V0              |
| NRA15 | NRA      | 0-30m (+-4cm)   | NRA                  | 
| NRA15 | NRA      | 0-100m (+-10cm) | NRA                  | 
| NRA24 | US-D1_V0 | 0-50m (+-4cm)   | USD1_V0              |
| NRA24 | US-D1_V0 | 0-200m (+-10cm) | USD1_V0              |
| NRA24 | NRA      | 0-50m (+-4cm)   | NRA                  | 
| NRA24 | NRA      | 0-200m (+-10cm) | NRA                  | 


## Connections

I2C solutions like `VL53L0X` or `INAV_I2C` can be connected to I2C port and used as any other I2C device.

#### Constraints

INAV does not support `HC-SR04` and `US-100`. No constrains for I2C like `VL53L0X` or `INAV_I2C`.