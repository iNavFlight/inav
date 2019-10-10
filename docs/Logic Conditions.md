# Logic Conditions

Logic Conditions (abbr. LC) is a mechanism that allows to evaluate cenrtain flight parameters (RC channels, switches, altitude, distance, timers, other logic conditions) and use the value of evaluated expression in different places of INAV. Currently, the result of LCs can be used in:

* [Servo mixer](Mixer.md) to activate/deactivate certain servo mix rulers
* [Global functions](Global%20Functions.md) to activate/deactivate system overrides

Logic conditions can be edited using INAV Configurator user interface, of via CLI

## CLI

`logic <rule> <enabled> <operation> <operand A type> <operand A value> <operand B type> <operand B value> <flags>`

* `<rule>` - ID of Logic Condition rule
* `<enabled>` - `0` evaluates as disabled, `1` evaluates as enabled
* `<operation>` - See `Operations` paragraph
* `<operand A type>` - See `Operands` paragraph
* `<operand A value>` - See `Operands` paragraph
* `<operand B type>` - See `Operands` paragraph
* `<operand B value>` - See `Operands` paragraph
* `<flags>` - See `Flags` paragraph

### Operations

| Operation ID  | Name      | Notes                                 |
|----           |----       |----                                   |
| 0             | TRUE      | Always evaluates as true              |
| 1             | EQUAL     | Evaluates `false` if `false` or `0`   |
| 2             | GREATER_THAN  |                                   |
| 3             | LOWER_THAN    |                                   |
| 4             | LOW           | `true` if `<1333`                 |
| 5             | MID           | `true` if `>=1333 and <=1666`     |
| 6             | HIGH          | `true` if `>1666`                 |
| 7             | AND           |                                   |
| 8             | OR            |                                   |
| 9             | XOR           |                                   |
| 10            | NAND          |                                   |
| 11            | NOR           |                                   |
| 12            | NOT           |                                   |         
| 13            | STICKY        | `Operand A` is activation operator, `Operand B` is deactivation operator. After activation, operator will return `true` until Operand B is evaluated as `true`|         

### Operands

| Operand Type  | Name      | Notes                                 |
|----           |----       |----                                   |
| 0             | VALUE     | Value derived from `value` field      |
| 1             | RC_CHANNEL    | `value` points to RC channel number, indexed from 1   |
| 2             | FLIGHT        | `value` points to flight parameter table              |
| 3             | FLIGHT_MODE   | `value` points to flight modes table                  |
| 4             | LC            | `value` points to other logic condition ID            |

#### FLIGHT

| Operand Value | Name          | Notes                                 |
|----           |----           |----                                   |
| 0             | ARM_TIMER     | in `seconds`                          |
| 1             | HOME_DISTANCE | in `meters`                           |
| 2             | TRIP_DISTANCE | in `meters`                           |
| 3             | RSSI          |                                       |
| 4             | VBAT          | in `Volts * 10`, eg. `12.1V` is `121` |
| 5             | CELL_VOLTAGE  | in `Volts * 10`, eg. `12.1V` is `121` |
| 6             | CURRENT       | in `Amps * 100`, eg. `9A` is `900`    |
| 7             | MAH_DRAWN     | in `mAh`                              |
| 8             | GPS_SATS      |                                       |
| 9             | GROUD_SPEED   | in `cm/s`                             |
| 10            | 3D_SPEED      | in `cm/s`                             |
| 11            | AIR_SPEED     | in `cm/s`                             |
| 12            | ALTITUDE      | in `cm`                               |
| 13            | VERTICAL_SPEED |  in `cm/s`                           |
| 14            | TROTTLE_POS   | in `%`                                |
| 15            | ATTITUDE_ROLL | in `degrees`                          |
| 16            | ATTITUDE_PITCH | in `degrees`                         |

#### FLIGHT_MODE

| Operand Value | Name          | Notes                                 |
|----           |----           |----                                   |
| 0             | FAILSAFE      |                                       |
| 1             | MANUAL        |                                       |
| 2             | RTH           |                                       |
| 3             | POSHOLD       |                                       |
| 4             | CRUISE        |                                       |
| 5             | ALTHOLD       |                                       |
| 6             | ANGLE         |                                       |
| 7             | HORIZON       |                                       |
| 8             | AIR           |                                       |

    
### Flags

All flags are reseted on ARM and DISARM event.

| bit   | Decimal   | Function              |
|----   |----       |----                   |
| 0     | 1         | Latch - after activation LC will stay active until LATCH flag is reseted |