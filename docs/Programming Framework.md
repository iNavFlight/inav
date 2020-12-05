# INAV Programming Framework

INAV Programming Framework (abbr. IPF) is a mechanism that allows to evaluate cenrtain flight parameters (RC channels, switches, altitude, distance, timers, other logic conditions) and use the value of evaluated expression in different places of INAV. Currently, the result of LCs can be used in:

* [Servo mixer](Mixer.md) to activate/deactivate certain servo mix rulers
* To activate/deactivate system overrides

INAV Programming Framework coinsists of:

* Logic Conditions - each Logic Condition can be understood as a single command, a single line of code
* Global Variables - variables that can store values from and for LogiC Conditions and servo mixer

IPF can be edited using INAV Configurator user interface, of via CLI

## CLI

`logic <rule> <enabled> <activatorId> <operation> <operand A type> <operand A value> <operand B type> <operand B value> <flags>`

* `<rule>` - ID of Logic Condition rule
* `<enabled>` - `0` evaluates as disabled, `1` evaluates as enabled
* `<activatorId>` - the ID of _LogicCondition_ used to activate this _Condition_. _Logic Condition_ will be evaluated only then Activator evaluates as `true`. `-1` evaluates as `true`
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
| 14            | ADD           | Add `Operand A` to `Operand B` and returns the result |
| 15            | SUB           | Substract `Operand B` from `Operand A` and returns the result |
| 16            | MUL           | Multiply `Operand A` by `Operand B` and returns the result |
| 17            | DIV           | Divide `Operand A` by `Operand B` and returns the result |
| 18            | GVAR SET      | Store value from `Operand B` into the Global Variable addressed by `Operand B`. Bear in mind, that operand `Global Variable` means: Value stored in Global Variable of an index! To store in GVAR 1 use `Value 1` not `Global Variable 1` |
| 19            | GVAR INC      | Increase the GVAR indexed by `Operand A` with value from `Operand B`  |
| 20            | GVAR DEC      | Decrease the GVAR indexed by `Operand A` with value from `Operand B`  |
| 21            | IO PORT SET   | Set I2C IO Expander pin `Operand A` to value of `Operand B`. `Operand A` accepts values `0-7` and `Operand B` accepts `0` and `1` |
| 22            | OVERRIDE_ARMING_SAFETY        | Allows to arm on any angle even without GPS fix              |
| 23            | OVERRIDE_THROTTLE_SCALE       | Override throttle scale to the value defined by operand. Operand type `0` and value `50` means throttle will be scaled by 50%. |
| 24            | SWAP_ROLL_YAW                 | basically, when activated, yaw stick will control roll and roll stick will control yaw. Required for tail-sitters VTOL during vertical-horizonral transition when body frame changes |
| 25            | SET_VTX_POWER_LEVEL           | Sets VTX power level. Accepted values are `0-3` for SmartAudio and `0-4` for Tramp protocol |
| 26            | INVERT_ROLL                   | Inverts ROLL axis input for PID/PIFF controller |
| 27            | INVERT_PITCH                  | Inverts PITCH axis input for PID/PIFF controller  |
| 28            | INVERT_YAW                    | Inverts YAW axis input for PID/PIFF controller |
| 29            | OVERRIDE_THROTTLE             | Override throttle value that is fed to the motors by mixer. Operand is scaled in us. `1000` means throttle cut, `1500` means half throttle |
| 30            | SET_VTX_BAND                  | Sets VTX band. Accepted values are `1-5` |
| 31            | SET_VTX_CHANNEL               | Sets VTX channel. Accepted values are `1-8` |
| 32            | SET_OSD_LAYOUT                | Sets OSD layout. Accepted values are `0-3` |
| 33            | SIN                | Computes SIN of `Operand A` value in degrees. Output is multiplied by `Operand B` value. If `Operand B` is `0`, result is multiplied by `500` |
| 34            | COS                | Computes COS of `Operand A` value in degrees. Output is multiplied by `Operand B` value. If `Operand B` is `0`, result is multiplied by `500` |
| 35            | TAN                | Computes TAN of `Operand A` value in degrees. Output is multiplied by `Operand B` value. If `Operand B` is `0`, result is multiplied by `500` |
| 36            | MAP_INPUT          | Scales `Operand A` from [`0` : `Operand B`] to [`0` : `1000`]. Note: input will be constrained and then scaled |
| 37            | MAP_OUTPUT         | Scales `Operand A` from [`0` : `1000`] to [`0` : `Operand B`]. Note: input will be constrained and then scaled |


### Operands

| Operand Type  | Name      | Notes                                 |
|----           |----       |----                                   |
| 0             | VALUE     | Value derived from `value` field      |
| 1             | RC_CHANNEL    | `value` points to RC channel number, indexed from 1   |
| 2             | FLIGHT        | `value` points to flight parameter table              |
| 3             | FLIGHT_MODE   | `value` points to flight modes table                  |
| 4             | LC            | `value` points to other logic condition ID            |
| 5             | GVAR          | Value stored in Global Variable indexed by `value`. `GVAR 1` means: value in GVAR 1 |

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
| 17            | IS_ARMED              | boolean `0`/`1`               |
| 18            | IS_AUTOLAUNCH         | boolean `0`/`1`               |
| 19            | IS_ALTITUDE_CONTROL   | boolean `0`/`1`               |
| 20            | IS_POSITION_CONTROL   | boolean `0`/`1`               |
| 21            | IS_EMERGENCY_LANDING  | boolean `0`/`1`               |
| 22            | IS_RTH                | boolean `0`/`1`               |
| 23            | IS_WP                 | boolean `0`/`1`               |
| 24            | IS_LANDING            | boolean `0`/`1`               |
| 25            | IS_FAILSAFE           | boolean `0`/`1`               |
| 26            | STABILIZED_ROLL       | Roll PID controller output `[-500:500]`   |
| 27            | STABILIZED_PITCH      | Pitch PID controller output `[-500:500]`  |
| 28            | STABILIZED_YAW        | Yaw PID controller output `[-500:500]`    |
| 29            | ACTIVE_WAYPOINT_INDEX | Indexed from `1`. To verify WP is in progress, use `IS_WP` |
| 30            | ACTIVE_WAYPOINT_ACTION | See ACTIVE_WAYPOINT_ACTION paragraph |
| 31            | 3D HOME_DISTANCE      | in `meters`, calculated from HOME_DISTANCE and ALTITUDE using Pythagorean theorem |
| 32            | CROSSFIRE LQ          | Crossfire Link quality as returned by the CRSF protocol   | 
| 33            | CROSSFIRE SNR          | Crossfire SNR as returned by the CRSF protocol   | 

##### ACTIVE_WAYPOINT_ACTION

| Action        |  Value   |
|----           |----      |
| WAYPOINT      | 1      |
| HOLD_TIME     | 3      |
| RTH           | 4      |
| SET_POI       | 5      |
| JUMP          | 6      |
| SET_HEAD      | 7      |
| LAND          | 8      |


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
| 9             | USER1         |                                       |
| 10            | USER2         |                                       |

    
### Flags

All flags are reseted on ARM and DISARM event.

| bit   | Decimal   | Function              |
|----   |----       |----                   |
| 0     | 1         | Latch - after activation LC will stay active until LATCH flag is reseted |

## Examples

### Dynamic THROTTLE scale

`logic 0 1 0 23 0 50 0 0 0`

Limits the THROTTLE output to 50% when Logic Condition `0` evaluates as `true`

### Set VTX power level via Smart Audio

`logic 0 1 0 25 0 3 0 0 0`

Sets VTX power level to `3` when Logic Condition `0` evaluates as `true`

### Invert ROLL and PITCH when rear facing camera FPV is used

Solves the problem from [https://github.com/iNavFlight/inav/issues/4439](https://github.com/iNavFlight/inav/issues/4439)

```
logic 0 1 0 26 0 0 0 0 0
logic 1 1 0 27 0 0 0 0 0
```

Inverts ROLL and PITCH input when Logic Condition `0` evaluates as `true`. Moving Pitch stick up will cause pitch down (up for rear facing camera). Moving Roll stick right will cause roll left of a quad (right in rear facing camera)

### Cut motors but keep other throttle bindings active

`logic 0 1 0 29 0 1000 0 0 0`

Sets Thhrottle output to `0%` when Logic Condition `0` evaluates as `true`

### Set throttle to 50% and keep other throttle bindings active

`logic 0 1 0 29 0 1500 0 0 0`

Sets Thhrottle output to about `50%` when Logic Condition `0` evaluates as `true`

### Set throttle control to different RC channel

`logic 0 1 0 29 1 7 0 0 0`

If Logic Condition `0` evaluates as `true`, motor throttle control is bound to RC channel 7 instead of throttle channel

### Set VTX channel with a POT

Set VTX channel with a POT on the radio assigned to RC channel 6

```
logic 0 1 -1 15 1 6 0 1000 0
logic 1 1 -1 37 4 0 0 7 0
logic 2 1 -1 14 4 1 0 1 0
logic 3 1 -1 31 4 2 0 0 0
```

Steps:
1. Normalize range `[1000:2000]` to `[0:1000]` by substracting `1000`
2. Scale range `[0:1000]` to `[0:7]`
3. Increase range by `1` to have the range of `[1:8]`
4. Assign LC#2 to VTX channel function

### Set VTX power with a POT

Set VTX power with a POT on the radio assigned to RC channel 6. In this example we scale POT to 4 power level `[1:4]`

```
logic 0 1 -1 15 1 6 0 1000 0
logic 1 1 -1 37 4 0 0 3 0
logic 2 1 -1 14 4 1 0 1 0
logic 3 1 -1 25 4 2 0 0 0
```

Steps:
1. Normalize range [1000:2000] to [0:1000] by substracting `1000`
2. Scale range [0:1000] to [0:3]
3. Increase range by `1` to have the range of [1:4]
4. Assign LC#2 to VTX power function