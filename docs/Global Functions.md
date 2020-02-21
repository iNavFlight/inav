# Global Functions

_Global Functions_ (abbr. GF) are a mechanism allowing to override certain flight parameters (during flight). Global Functions are activated by [Logic Conditions](Logic%20Conditions.md)

## CLI

`gf <rule> <enabled> <logic condition> <action> <operand type> <operand value> <flags>`

* `<rule>` - GF ID, indexed from `0`
* `<enabled>` - `0` evaluates as disabled, `1` evaluates as enabled. Only enabled GFs are executed
* `<logic condition>` - the ID of _LogicCondition_ used to trigger GF On/Off. Then LC evaluates `true`, GlobalFunction will be come active
* `<action>` - action to execute when GF is active
* `<operand type>` - allows to pass arguments into Global Function action. Syntax is the same as in case of Logic Conditions operands. Used only when `action` requires it. Should be kept at `0` in other case. See [Logic Conditions](Logic%20Conditions.md)
* `<operand value>` - allows to pass arguments into Global Function action. Syntax is the same as in case of Logic Conditions operands. Used only when `action` requires it. Should be kept at `0` in other case. See [Logic Conditions](Logic%20Conditions.md)
* `<flags>` - allows to pass arguments into Global Function action. Syntax is the same as in case of Logic Conditions operands

## Actions

| Action ID     | Name                          | Notes                                 |
|----           |----                           |----                                   |
| 0             | OVERRIDE_ARMING_SAFETY        | Allows to arm on any angle even without GPS fix              |
| 1             | OVERRIDE_THROTTLE_SCALE       | Override throttle scale to the value defined by operand. Operand type `0` and value `50` means throttle will be scaled by 50%. |
| 2             | SWAP_ROLL_YAW                 | basically, when activated, yaw stick will control roll and roll stick will control yaw. Required for tail-sitters VTOL during vertical-horizonral transition when body frame changes |
| 3             | SET_VTX_POWER_LEVEL           | Sets VTX power level. Accepted values are `0-3` for SmartAudio and `0-4` for Tramp protocol |
| 4             | INVERT_ROLL                   | Inverts ROLL axis input for PID/PIFF controller |
| 5             | INVERT_PITCH                  | Inverts PITCH axis input for PID/PIFF controller  |
| 6             | INVERT_YAW                    | Inverts YAW axis input for PID/PIFF controller |
| 7             | OVERRIDE_THROTTLE             | Override throttle value that is fed to the motors by mixer. Operand is scaled in us. `1000` means throttle cut, `1500` means half throttle |

## Flags

Current no flags are implemented

## Example

### Dynamic THROTTLE scale

`gf 0 1 0 1 0 50 0`

Limits the THROTTLE output to 50% when Logic Condition `0` evaluates as `true`

### Set VTX power level via Smart Audio

`gf 0 1 0 3 0 3 0`

Sets VTX power level to `3` when Logic Condition `0` evaluates as `true`

### Invert ROLL and PITCH when rear facing camera FPV is used

Solves the problem from [https://github.com/iNavFlight/inav/issues/4439](https://github.com/iNavFlight/inav/issues/4439)

```
gf 0 1 0 4 0 0 0
gf 1 1 0 5 0 0 0
```

Inverts ROLL and PITCH input when Logic Condition `0` evaluates as `true`. Moving Pitch stick up will cause pitch down (up for rear facing camera). Moving Roll stick right will cause roll left of a quad (right in rear facing camera)

### Cut motors but keep other throttle bindings active

`gf 0 1 0 7 0 1000 0`

Sets Thhrottle output to `0%` when Logic Condition `0` evaluates as `true`

### Set throttle to 50% and keep other throttle bindings active

`gf 0 1 0 7 0 1500 0`

Sets Thhrottle output to about `50%` when Logic Condition `0` evaluates as `true`

### Set throttle control to different RC channel

`gf 0 1 0 7 1 7 0`

If Logic Condition `0` evaluates as `true`, motor throttle control is bound to RC channel 7 instead of throttle channel

