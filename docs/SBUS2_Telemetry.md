# Futaba SBUS2 Telemetry

Basic support for SBUS2 telemetry has been added to INAV 8.0.0. Currently it is somewhat unreliable and slow to update.

The basic sensors have been tested with a Futaba T16IZ running software version 6.0E.

# Sensor mapping

The follow fixed sensor mapping is used:
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
