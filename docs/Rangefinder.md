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

## Connections

I2C solutions like `VL53L0X` or `INAV_I2C` can be connected to I2C port and used as any other I2C device.

#### Constraints

iNav does not support `HC-SR04` and `US-100`. No constrains for I2C like `VL53L0X` or `INAV_I2C`.