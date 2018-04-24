# Sonar

A sonar sensor can be used to measure altitude for use with BARO and RANGEFINDER altitude
hold modes.

The sonar sensor is used instead of the pressure sensor (barometer) at low altitudes.
The sonar sensor is only used when the aircraft inclination angle (attitude) is small.

## Hardware

Currently the only supported sensor is the HCSR04 sensor.

## Connections

Target dependent

#### Constraints

Current meter cannot be used in conjunction with Parallel PWM and Sonar.

### Olimexino

| Trigger       | Echo          | Inline 1k resistors |
| ------------- | ------------- | ------------------- |
| PB0 / RC7     | PB1 / RC8     | YES (3.3v input)    |

#### Constraints

Current meter cannot be used in conjunction with Sonar.