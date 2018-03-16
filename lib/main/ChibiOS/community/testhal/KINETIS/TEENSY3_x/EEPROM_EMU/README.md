# Teensy 3.0 EEPROM via FlexRAM example

FlexRAM, which is present on K20x MCUs, is configured to EEPROM mode, so it behaves like EEPROM. The maximum available size is 2K, but explicitly using a smaller chunk enables wear-levelling and increases write endurance.


## Credits

Most of the actual EEPROM code is from [PJRC/Teensyduino](https://www.pjrc.com/teensy/teensyduino.html).