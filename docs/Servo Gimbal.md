# Servo Gimbal

Starting from INAV 2.0, _servo gimbal_ (aka _SERVO_TILT_) feature is removed. This functionality can be configured using _Mixer Configurator Tab_ instead.

Servo gimbal using mixer works for all flight controllers that supports servos in either multirotor or airplane configuration. Bear in mind, that some boards might not have any servo outputs in multirotor configuration. In this case, servo gimbal will just not work. Refer to the board documentation to find out if servos are available in multirotor configuration.

## How to setup _Servo Gimbal_

1. Open Mixer tab
1. Add new _servo rule_ for given servo index
1. Choose source as one of _Gimbal Pitch_ and _Gimbal Roll_
1. When required, apply scaling of the output by modyfing weight
1. When required, servo direction can be modified by applying negative weight

### MIXTILT option

In rare cases, when gimbal required pitch mixed with roll, it is possible to use mixer to do it. In this case, mix roll and pitch exis are required. Refer to gimbal documentation for more details.