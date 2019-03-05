# Temperature sensors

It is now possible to measure temperatures with the help of the I²C LM75 and 1-Wire DS18B20 chips. The temperature values can be displayed on the OSD and they are also logged. A total maximum of 8 temperature sensors can be connected. The support is enabled by default on F4 and F7 FCs. To use with F3 FCs you need to build a custom firmware.

## LM75

Up to 8 can be connected to the flight controller.

* Package: SOP-8, breakout boards can be found easily
* Interface: I²C
* Supply: 2.7 to 5.5V
* Temperature range: -55 to +125°C

## DS18B20

* Package: TO-92, SO-8, µSOP-8
* Interface: 1-Wire
* Supply: 3.0 to 5.5V (parasitic power not supported)
* Temperature range: -55 to +125°C

None of the flight controllers on the market at the time this documentation is written supports 1-Wire directly. To use these sensors a I²C to 1-Wire interface chip needs to be used, the DS2482. Connect the DS2482 SCL and SDA lines to your FC, add a 4.7kohm pull-up resistor between VCC and the DQ pin then connect all the sensors DQ pin to the DS2482 DQ pin.

## Configuring temperature sensors

The `temp_sensor` CLI command can be used to display and change the temperature sensors configuration. When a new temperature sensor is connected it is automatically detected and will appear in the output of the `temp_sensor` command.

* `temp_sensor` without any argument displays all the sensors configuration
* `temp_sensor reset` deletes all the sensors
* `temp_sensor index type address alarm_min alarm_max osd_symbol label` to configure a new sensor or modify the configuration of an existing one.

### Parameters description

* `index` is the index of the configuration slot you want to change
* `type` can be `1` for LM75 or `2` for DS18B20
* `address` is the address of the device on the bus. 0 to 7 for a LM75 or the full 64bit ROM in hex format for a 18B20
* `alarm_min` is the temperature under which the corresponding OSD element will start blinking (decidegrees centigrade)
* `alarm_max` is the temperature above which the corresponding OSD element will start blinking (decidegrees centigrade)
* `osd_symbol` is the ID of a symbol to display on the OSD to the left of the temperature value. Use 0 to display a label instead (see next parameter). See the table bellow for the available IDs
* `label` is a 4 characters maximum label that is displayed on the OSD next to the temperature value

| Symbol ID | Description                 |
|-----------|-----------------------------|
| 1         | Generic temperature symbol  |
| 2         | ESC temperature symbol      |
| 3         | VTX temperature symbol      |
| 4         | Motor temperature symbol    |
| 5         | Battery temperature symbol  |
| 6         | Exterior temperature symbol |

### Example output

Example output of the `temp_sensor` command on a system with two LM75 and four DS18B20 sensors connected

```
temp_sensor 0 1 0 -200 600 0
temp_sensor 1 1 1 -200 600 0
temp_sensor 2 2 7c0118681e1cff28 -200 600 0
temp_sensor 3 2 7d01186838f2ff28 -200 600 0
temp_sensor 4 2 210118684001ff28 -200 600 0
temp_sensor 5 2 f801186750c7ff28 -200 600 0
temp_sensor 6 0 0 0 0 
temp_sensor 7 0 0 0 0 
```

To set for example the OSD symbol of the first temperature sensor to the ESC symbol:

`temp_sensor 0 1 0 -200 600 2`

To change for example the configuration of the fourth sensor to label `BATT`, minimum value alarm 0.5°C and maximum value alarm 45°C

`temp_sensor 3 2 7d01186838f2ff28 5 450 0 BATT`

## Building a custom firmware with temperature sensor support (F3 only)

This needs to be added in the `target.h` file:

```
#define USE_TEMPERATURE_SENSOR
#define TEMPERATURE_I2C_BUS BUS_I2Cx // replace x with the index of the I²C bus the temperature sensors will be connected to

// for LM75 sensors support
#define USE_TEMPERATURE_LM75

// for DS18B20 sensors
#define USE_1WIRE
#define USE_1WIRE_DS2482
#define USE_TEMPERATURE_DS18B20
```

## Configuring the way OSD temperature labels are displayed

You can use the `osd_temp_label_align` setting to chose how the labels for the temperature sensor's values are displayed. Possible alignment values are `LEFT` and `RIGHT`.

### Example

```
LEFT alignment:
T1   xxx°C
ESC  xxx°C

RIGHT alignment:
 T1 xxx°C
ESC xxx°C
```
