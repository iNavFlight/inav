# Pixracer

* Target owner: @DigitalEntity
* The PixRacer is the first autopilot of the new FMUv4 Pixhawk generation. It has Wifi built-in, comes with upgraded sensors and more flash.
* Website: https://store.mrobotics.io/mRo-PixRacer-R14-Official-p/auav-pxrcr-r14-mr.htm

## HW info

* STM32F427VIT6
* MPU9250 SPI
* MS5611 baro
* 6 pwm outputs + PPM/SBus input

## R14

* ICM-20608 SPI
* HMC5983 compass

## R15

* ICM-20608 or ICM-20602 SPI
* LIS3MDL compass

## Warnings

* PixRacers native Flight controller (PX4 & ArduPilot) motor layout is different to INAV
  * Either Swap your ESC PWM cables to match INAV
  * Or make a custom mix
