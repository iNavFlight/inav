# INAV Programming Framework

INAV Programming Framework (IPF) is a mechanism that allows you to to create
custom functionality in INAV. You can choose for certain actions to be done,
based on custom conditions you select.

Logic conditions can be based on things such as RC channel values, switches, altitude,
distance, timers, etc. The conditions you create  can also make use of other conditions
you've entered previously.
The results can be used in:

* [Servo mixer](Mixer.md) to activate/deactivate certain servo mix rulers
* To activate/deactivate system overrides

INAV Programming Framework consists of:

* Logic Conditions - each Logic Condition can be understood as a single command, a single line of code. Each logic condition consists of:
	* an operator (action), such as "plus" or "set vtx power"
	* one or two operands (nouns), which the action acts upon. Operands are often numbers, such as a channel value or the distance to home.
	* "activator" condition - optional. This condition is only active when another condition is true
* Global Variables - variables that can store values from and for Logic Conditions and servo mixer
* Programming PID - general purpose, user configurable PID controllers

IPF can be edited using INAV Configurator user interface, or via CLI. To use COnfigurator, click the tab labeled
"Programming". The various options shown in Configurator are described below.

**Note:** IPF uses integer math. If your programming line returns a decimal, it will be truncated to an integer.  So if your math is `1` / `3` = , IPF will truncate the decimal and return `0`.

## Logic Conditions

### CLI

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

| Operation ID  | Name                          | Notes |
|---------------|-------------------------------|-------|
| 0             | True                          | Always evaluates as true |
| 1             | Equal (A=B)                   | Evaluates `false` if `false` or `0` |
| 2             | Greater Than (A>B)            | `true` if `Operand A` is a higher value than `Operand B` |
| 3             | Lower Than (A<B)              | `true` if `Operand A` is a lower value than `Operand B` |
| 4             | Low                           | `true` if `<1333` |
| 5             | Mid                           | `true` if `>=1333 and <=1666` |
| 6             | High                          | `true` if `>1666` |
| 7             | AND                           | `true` if `Operand A` and `Operand B` are the same value or both `true` |
| 8             | OR                            | `true` if `Operand A` and/or `OperandB` is `true` |
| 9             | XOR                           | `true` if `Operand A` or `Operand B` is `true`, but not both |
| 10            | NAND                          | `false` if `Operand A` and `Operand B` are both `true`|
| 11            | NOR                           | `true` if `Operand A` and `Operand B` are both `false` |
| 12            | NOT                           | The boolean opposite to `Operand A` |
| 13            | Sticky                        | `Operand A` is the activation operator, `Operand B` is the deactivation operator. After the activation is `true`, the operator will return `true` until Operand B is evaluated as `true`|
| 14            | Basic: Add                    | Add `Operand A` to `Operand B` and returns the result |
| 15            | Basic: Subtract               | Substract `Operand B` from `Operand A` and returns the result |
| 16            | Basic: Multiply               | Multiply `Operand A` by `Operand B` and returns the result |
| 17            | Basic: Divide                 | Divide `Operand A` by `Operand B` and returns the result. NOTE: If `Operand B` = `0`, the `Divide` operation will simply return `Operand A`|
| 18            | Set GVAR                      | Store value from `Operand B` into the Global Variable addressed by `Operand A`. Bear in mind, that operand `Global Variable` means: Value stored in Global Variable of an index! To store in GVAR 1 use `Value 1` not `Global Variable 1` |
| 19            | Increase GVAR                 | Increase the GVAR indexed by `Operand A` (use `Value 1` for Global Variable 1) with value from `Operand B`  |
| 20            | Decrease GVAR                 | Decrease the GVAR indexed by `Operand A` (use `Value 1` for Global Variable 1) with value from `Operand B`  |
| 21            | Set IO Port                   | Set I2C IO Expander pin `Operand A` to value of `Operand B`. `Operand A` accepts values `0-7` and `Operand B` accepts `0` and `1` |
| 22            | Override Arming Safety        | Allows the craft to arm on any angle even without GPS fix. WARNING: This bypasses all safety checks, even that the throttle is low, so use with caution. If you only want to check for certain conditions, such as arm without GPS fix. You will need to add logic conditions to check the throttle is low. |
| 23            | Override Throttle Scale       | Override throttle scale to the value defined by operand. Operand type `0` and value `50` means throttle will be scaled by 50%. |
| 24            | Swap Roll & Yaw               | basically, when activated, yaw stick will control roll and roll stick will control yaw. Required for tail-sitters VTOL during vertical-horizonral transition when body frame changes |
| 25            | Set VTx Power Level           | Sets VTX power level. Accepted values are `0-3` for SmartAudio and `0-4` for Tramp protocol |
| 26            | Invert Roll                   | Inverts ROLL axis input for PID/PIFF controller |
| 27            | Invert Pitch                  | Inverts PITCH axis input for PID/PIFF controller  |
| 28            | Invert Yaw                    | Inverts YAW axis input for PID/PIFF controller |
| 29            | Override Throttlw             | Override throttle value that is fed to the motors by mixer. Operand is scaled in us. `1000` means throttle cut, `1500` means half throttle |
| 30            | Set VTx Band                  | Sets VTX band. Accepted values are `1-5` |
| 31            | Set VTx Channel               | Sets VTX channel. Accepted values are `1-8` |
| 32            | Set OSD Layout                | Sets OSD layout. Accepted values are `0-3` |
| 33            | Trigonometry: Sine            | Computes SIN of `Operand A` value in degrees. Output is multiplied by `Operand B` value. If `Operand B` is `0`, result is multiplied by `500` |
| 34            | Trigonometry: Cosine          | Computes COS of `Operand A` value in degrees. Output is multiplied by `Operand B` value. If `Operand B` is `0`, result is multiplied by `500` |
| 35            | Trigonometry: Tangent         | Computes TAN of `Operand A` value in degrees. Output is multiplied by `Operand B` value. If `Operand B` is `0`, result is multiplied by `500` |
| 36            | Map Input                     | Scales `Operand A` from [`0` : `Operand B`] to [`0` : `1000`]. Note: input will be constrained and then scaled |
| 37            | Map Output                    | Scales `Operand A` from [`0` : `1000`] to [`0` : `Operand B`]. Note: input will be constrained and then scaled |
| 38            | Override RC Channel           | Overrides channel set by `Operand A` to value of `Operand B`. Note operand A should normally be set as a "Value", NOT as "Get RC Channel"|
| 39            | Set Heading Target            | Sets heading-hold target to `Operand A`, in centidegrees. Value wraps-around. |
| 40            | Modulo                        | Modulo. Divide `Operand A` by `Operand B` and returns the remainder |
| 41            | Override Loiter Radius        | Sets the loiter radius to `Operand A` [`0` : `100000`] in cm. Must be larger than the loiter radius set in the **Advanced Tuning**. |
| 42            | Set Control Profile           | Sets the active config profile (PIDFF/Rates/Filters/etc) to `Operand A`. `Operand A` must be a valid profile number, currently from 1 to 3. If not, the profile will not change |
| 43            | Use Lowest Value              | Finds the lowest value of `Operand A` and `Operand B` |
| 44            | Use Highest Value             | Finds the highest value of `Operand A` and `Operand B` |
| 45			| Flight Axis Angle Override	| Sets the target attitude angle for axis. In other words, when active, it enforces Angle mode (Heading Hold for Yaw) on this axis (Angle mode does not have to be active). `Operand A` defines the axis: `0` - Roll, `1` - Pitch, `2` - Yaw. `Operand B` defines the angle in degrees |
| 46			| Flight Axis Rate Override	    | Sets the target rate (rotation speed) for axis. `Operand A` defines the axis: `0` - Roll, `1` - Pitch, `2` - Yaw. `Operand B` defines the rate in degrees per second |
| 47            | Edge                          | Momentarily true when triggered by `Operand A`. `Operand A` is the activation operator [`boolean`], `Operand B` _(Optional)_ is the time for the edge to stay active [ms]. After activation, operator will return `true` until the time in Operand B is reached. If a pure momentary edge is wanted. Just leave `Operand B` as the default `Value: 0` setting. |
| 48            | Delay                         | Delays activation after being triggered. This will return `true` when `Operand A` _is_ true, and the delay time in `Operand B` [ms] has been exceeded. |
| 49            | Timer                         | A simple on - off timer. `true` for the duration of `Operand A` [ms]. Then `false` for the duration of `Operand B` [ms]. |
| 50            | Delta             | This returns `true` when the value of `Operand A` has changed by the value of `Operand B` or greater within 100ms. ( \|ΔA\| >= B )  |
| 51            | Approx Equals (A ~ B)         | `true` if `Operand B` is within 1% of `Operand A`. |
| 52            | LED Pin PWM                   | Value `Operand A` from [`0` : `100`] starts PWM generation on LED Pin. See [LED pin PWM](LED%20pin%20PWM.md). Any other value stops PWM generation (stop to allow ws2812 LEDs updates in shared modes). |
| 53            | Disable GPS Sensor Fix        | Disables the GNSS sensor fix. For testing GNSS failure. |
| 54            | Mag calibration               | Trigger a magnetometer calibration. |

### Operands

| Operand Type  | Name                  | Notes |
|---------------|-----------------------|-------|
| 0             | Value                 | Value derived from `value` field |
| 1             | Get RC Channel        | `value` points to RC channel number, indexed from 1 |
| 2             | Flight                | `value` points to **Flight** Parameters table |
| 3             | Flight Mode           | `value` points to **Flight_Mode** table |
| 4             | Logic Condition       | `value` points to other logic condition ID |
| 5             | Get Global Variable   | Value stored in Global Variable indexed by `value`. `GVAR 1` means: value in GVAR 1 |
| 5             | Programming PID       | Output of a Programming PID indexed by `value`. `PID 1` means: value in PID 1 |
| 6             | Waypoints             | `value` points to the **Waypoint** parameter table |

#### Flight Parameters

| Operand Value | Name                                  | Notes |
|---------------|---------------------------------------|-------|
| 0             | ARM Timer [s]                         | Time since armed in `seconds` |
| 1             | Home Distance [m]                     | distance from home in `meters` |
| 2             | Trip distance [m]                     | Trip distance in `meters` |
| 3             | RSSI                                  |  |
| 4             | Vbat [centi-Volt] [1V = 100]          | VBAT Voltage in `Volts * 100`, eg. `12.1V` is `1210` |
| 5             | Cell voltage [centi-Volt] [1V = 100]  | Average cell voltage in `Volts * 100`, eg. `12.1V` is `1210` |
| 6             | Current [centi-Amp] [1A = 100]        | Current in `Amps * 100`, eg. `9A` is `900` |
| 7             | Current drawn [mAh]                   | Total used current in `mAh` |
| 8             | GPS Sats                              |  |
| 9             | Ground speed [cm/s]                   | Ground speed in `cm/s` |
| 10            | 3D speed [cm/s]                       | 3D speed in `cm/s` |
| 11            | Air speed [cm/s]                      | Air speed in `cm/s` |
| 12            | Altitude [cm]                         | Altitude in `cm` |
| 13            | Vertical speed [cm/s]                 | Vertical speed in `cm/s` |
| 14            | Throttle position [%]                 | Throttle position in `%` |
| 15            | Roll [deg]                            | Roll attitude in `degrees` |
| 16            | Pitch [deg]                           | Pitch attitude in `degrees` |
| 17            | Is Armed                              | Is the system armed? boolean `0`/`1` |
| 18            | Is Autolaunch                         | Is auto launch active? boolean `0`/`1` |
| 19            | Is Controlling Altitude               | Is altitude being controlled? boolean `0`/`1` |
| 20            | Is Controlling Position               | Is the position being controlled? boolean `0`/`1` |
| 21            | Is Emergency Landing                  | Is the aircraft emergency landing? boolean `0`/`1` |
| 22            | Is RTH                                | Is RTH active? boolean `0`/`1` |
| 23            | Is Landing                            | Is the aircaft automatically landing? boolean `0`/`1` |
| 24            | Is Failsafe                           | Is the flight controller in a failsafe? boolean `0`/`1` |
| 25            | Stabilized Roll                       | Roll PID controller output `[-500:500]` |
| 26            | Stabilized Pitch                      | Pitch PID controller output `[-500:500]` |
| 27            | Stabilized Yaw                        | Yaw PID controller output `[-500:500]` |
| 28            | 3D home distance [m]                  | 3D distance to home in `meters`. Calculated from Home distance and Altitude using Pythagorean theorem |
| 29            | CRSF LQ                               | Link quality as returned by the CRSF protocol |
| 30            | CRSF SNR                              | SNR as returned by the CRSF protocol |
| 31            | GPS Valid Fix                         | Boolean `0`/`1`. True when the GPS has a valid 3D Fix |
| 32            | Loiter Radius [cm]                    | The current loiter radius in cm. |
| 33            | Active Control Profile                | Integer for the active config profile `[1..MAX_PROFILE_COUNT]` |
| 34            | Battery cells                         | Number of battery cells detected |
| 35            | AGL status [0/1]                      | Boolean `1` when AGL can be trusted, `0` when AGL estimate can not be trusted |
| 36            | AGL [cm]                              | Integer altitude above The Groud Altitude in `cm` |
| 37            | Rangefinder [cm]                      | Integer raw distance provided by the rangefinder in `cm` |
| 38            | Active Mixer Profile                  | Which mixer is currently active (for vtol etc) |
| 39            | Mixer Transition Active               | Boolean `0`/`1`. Are we currently switching between mixers (quad to plane etc) |
| 40            | Yaw [deg]                             | Current heading (yaw) in `degrees` |
| 41            | FW Land Sate                          | Integer `1` - `5`, indicates the status of the FW landing, 0 Idle, 1 Downwind, 2 Base Leg, 3 Final Approach, 4 Glide, 5 Flare |
| 42            | Current battery profile               | The active battery profile. Integer `[1..MAX_PROFILE_COUNT]` |
| 43            | Flown Loiter Radius [m]               | The actual loiter radius flown by a fixed wing during hold modes, in `meters` |

#### FLIGHT_MODE

The flight mode operands return `true` when the mode is active. These are modes that you will see in the **Modes** tab. Note: the `USER*` modes are used by camera switchers, PINIO etc. They are not the Waypoint User Actions. See the [Waypoints](#waypoints) section to access those.

| Operand Value | Name              | Notes |
|---------------|-------------------|-------|
| 0             | Failsafe          | `true` when a **Failsafe** state has been triggered. |
| 1             | Manual            | `true` when you are in the **Manual** flight mode. |
| 2             | RTH               | `true` when you are in the **Return to Home** flight mode. |
| 3             | Position Hold     | `true` when you are in the **Position Hold** or **Loiter** flight modes. |
| 4             | Cruise            | `true` when you are in the **Cruise** flight mode. |
| 5             | Altitude Hold     | `true` when you the **Altitude Hold** flight mode modifier is active. |
| 6             | Angle             | `true` when you are in the **Angle** flight mode. |
| 7             | Horizon           | `true` when you are in the **Horizon** flight mode. |
| 8             | Air               | `true` when you the **Airmode** flight mode modifier is active. |
| 9             | USER 1            | `true` when the **USER 1** mode is active. |
| 10            | USER 2            | `true` when the **USER 2** mode is active. |
| 11            | Course Hold       | `true` when you are in the **Course Hold** flight mode. |
| 12            | USER 3            | `true` when the **USER 3** mode is active. |
| 13            | USER 4            | `true` when the **USER 4** mode is active. |
| 14            | Acro              | `true` when you are in the **Acro** flight mode. |
| 15            | Waypoint Mission  | `true` when you are in the **WP Mission** flight mode. |

#### WAYPOINTS

| Operand Value | Name                          | Notes |
|---------------|-------------------------------|-------|
| 0             | Is WP                         | Boolean `0`/`1` |
| 1             | Current Waypoint Index        | Current waypoint leg. Indexed from `1`. To verify WP is in progress, use `Is WP` |
| 2             | Current Waypoint Action       | `true` when Action active in current leg. See ACTIVE_WAYPOINT_ACTION table |
| 3             | Next Waypoint Action          | `true` when Action active in next leg. See ACTIVE_WAYPOINT_ACTION table |
| 4             | Distance to next Waypoint     | Distance to next WP in metres |
| 5             | Distance from Waypoint        | Distance from the last WP in metres |
| 6             | User Action 1                 | `true` when User Action 1 is active on this waypoint leg [boolean `0`/`1`] |
| 7             | User Action 2                 | `true` when User Action 2 is active on this waypoint leg [boolean `0`/`1`] |
| 8             | User Action 3                 | `true` when User Action 3 is active on this waypoint leg [boolean `0`/`1`] |
| 9             | User Action 4                 | `true` when User Action 4 is active on this waypoint leg [boolean `0`/`1`] |
| 10            | Next Waypoint User Action 1   | `true` when User Action 1 is active on the next waypoint leg [boolean `0`/`1`] |
| 11            | Next Waypoint User Action 2   | `true` when User Action 2 is active on the next waypoint leg [boolean `0`/`1`] |
| 12            | Next Waypoint User Action 3   | `true` when User Action 3 is active on the next waypoint leg [boolean `0`/`1`] |
| 13            | Next Waypoint User Action 4   | `true` when User Action 4 is active on the next waypoint leg [boolean `0`/`1`] |


#### ACTIVE_WAYPOINT_ACTION

| Action        | Value |
|---------------|-------|
| WAYPOINT      | 1     |
| HOLD_TIME     | 3     |
| RTH           | 4     |
| SET_POI       | 5     |
| JUMP          | 6     |
| SET_HEAD      | 7     |
| LAND          | 8     |

### Flags

All flags are reseted on ARM and DISARM event.

| bit   | Decimal   | Function  |
|-------|-----------|-----------|
| 0     | 1         | Latch - after activation LC will stay active until LATCH flag is reset |
| 1     | 2         | Timeout satisfied - Used in timed operands to determine if the timeout has been met |

## Global variables

### CLI

`gvar <index> <default value> <min> <max>`

**Note:**  Global Variables (GVARs) are limited to integers between negative `-32768` and positive `32767`.

## Programming PID

IPF makes a set of general user PIDFF controllers avaliable for use in your program.  These PIDFF controllers are not tied to any roll/pitch/yaw profiles or other controls.
The output of these controllers can be used in an IPF program by using the `Programming PID` operand.
The `<setpoint value>` of the controller is the target value for the controller to hit.  The `<measurement value>` is the measurement of the current value.  For instance, `<setpoint value>` could be the speed you want to go, and `<measurement value>` is the current speed.
P, I, D, and FF values will need to be manually adjusted to determine the appropriate value for the program and controller.

`pid <index> <enabled> <setpoint type> <setpoint value> <measurement type> <measurement value> <P gain> <I gain> <D gain> <FF gain>`

* `<index>` - ID of PID Controller, starting from `0`
* `<enabled>` - `0` evaluates as disabled, `1` evaluates as enabled
* `<setpoint type>` - See `Operands` paragraph
* `<setpoint value>` - See `Operands` paragraph
* `<measurement type>` - See `Operands` paragraph
* `<measurement value>` - See `Operands` paragraph
* `<P gain>` - P-gain, scaled to `1/1000`
* `<I gain>` - I-gain, scaled to `1/1000`
* `<D gain>` - D-gain, scaled to `1/1000`
* `<FF gain>` - FF-gain, scaled to `1/1000`

## Examples

### When more than 100 meters away, increase VTX power
![screenshot of vtx home distance](./assets/images/vtx_home_distance.png)

### When more than 600 meters away, engage return-to-home by setting the matching RC channel
![screenshot of rth home distance](./assets/images/rth_home_distance.jpg)


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

Sets throttle output to `0%` when Logic Condition `0` evaluates as `true`

### Set throttle to 50% and keep other throttle bindings active

`logic 0 1 0 29 0 1500 0 0 0`

Sets throttle output to about `50%` when Logic Condition `0` evaluates as `true`

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

## Common issues / questions about IPF

One common mistake involves setting RC channel values. To override (set) the
value of a specific RC channel, choose "Override RC value", then for operand A
choose *value* and enter the channel number. Choosing "get RC value" is a common mistake,
which does something other than what you probably want.

![screenshot of override an RC channel with a value](./assets/images/ipf_set_get_rc_channel.png)
