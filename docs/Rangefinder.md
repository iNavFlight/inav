# Rangefinder

Rangefinders are devices used to measure Above Ground Altitude.

## Current state

Current support of rangefinders in INAV is very limited. They are used only to:

* landing detection for multirotors
* automated landing support for fixed wings
* _Experimental_ terrain following (Surface) flight mode activated with _SURFACE_ and _ALTHOLD_ flight mode

## Hardware

Following rangefinders are supported:

* HC-SR04 - ***DO NOT USE*** HC-SR04 while most popular is not suited to use in noise reach environments (like multirotors). It is proven that this sonar rangefinder start to report random values already at 1.5m above solid concrete surface. Reported altitude is valid up to only 75cm above concrete. ***DO NOT USE***
* US-100 in trigger-echo mode - Can be used as direct replacement of _HC-SR04_ when `rangefinder_hardware` is set to _HCSR04_. Useful up to 2m over concrete and correctly reporting _out of range_ when out of range
* SRF10 - experimental
* INAV_I2C - is a simple [DIY rangefinder interface with Arduino Pro Mini 3.3V](https://github.com/iNavFlight/inav-rangefinder). Can be used to connect when flight controller has no Trigger-Echo ports. 
* VL53L0X - simple laser rangefinder usable up to 75cm
* UIB - experimental
* MSP - experimental

## Connections

Target dependent in case of Trigger/Echo solutions like `HC-SR04` and `US-100`.
I2C solutions like `VL53L0X` or `INAV_I2C` can be connected to I2C port and used as any other I2C device.

#### Constraints

Target dependent in case of Trigger/Echo solutions like `HC-SR04` and `US-100`. No constrains for I2C like `VL53L0X` or `INAV_I2C`.