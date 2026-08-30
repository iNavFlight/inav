# Futaba SBUS2 Telemetry

Basic experimental support for SBUS2 telemetry has been added to INAV 8.0.0. Currently it is limited to F7 and H7 mcus only. The main reason it is limited to those MCUs is due to the requirement for an inverted UART signal and the SBUS pads in F405 usually are not bi-directional.

The basic sensors have been tested with a Futaba T16IZ running software version 6.0E.

An alternative to using INAV's SBUS2 support is to use SBS-01ML MAVlink Telemetry Drone Sensor instead. (not tested and not supported with older futaba radios, including my 16IZ).

# Wiring
The SBUS2 signal should be connected to the TX PIN, not the RX PIN, like on a traditional SBUS setup.

# Sensor mapping

The following fixed sensor mapping is used:

| Slot | Sensort Type | Info |
| --- | --- | --- |
| 1 | Voltage | Pack voltage and cell voltage |
| 3 | Current | Capacity = used mAh |
| 6 | rpm sensor | motor rpm. Need to set geat ratio to 1.0 |
| 7 | Temperature | ESC Temperature |
| 8 | GPS | |
| 16 | Temperature | IMU Temperature |
| 17 | Temperature | Baro Temperature |
| 18-25 | Temperature | Temperature sensor 0-7 |
