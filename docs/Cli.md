# Command Line Interface (CLI)

INAV has a command line interface (CLI) that can be used to change settings and configure the FC.

## Accessing the CLI.

The CLI can be accessed via the GUI tool or via a terminal emulator connected to the CLI serial port.

1. Connect your terminal emulator to the CLI serial port (which, by default, is the same as the MSP serial port)
2. Use the baudrate specified by msp_baudrate (115200 by default).
3. Send a `#` character.

To save your settings type in 'save', saving will reboot the flight controller.

To exit the CLI without saving power off the flight controller or type in 'exit'.

To see a list of other commands type in 'help' and press return.

To dump your configuration (including the current profile), use the 'dump' or 'diff' command.

See the other documentation sections for details of the cli commands and settings that are available.

## Backup via CLI

Disconnect main power, connect to cli via USB/FTDI.

dump using cli

```
profile 0
dump
```

dump profiles using cli if you use them

```
profile 1
dump profile
profile 2
dump profile
```

copy screen output to a file and save it.

Alternatively, use the `diff` command to dump only those settings that differ from their default values (those that have been changed).


## Restore via CLI.

Use the cli `defaults` command first.

When restoring from backup it's a good idea to do a dump of the latest defaults so you know what has changed - if you do this each time a firmware release is created you will be able to see the cli changes between firmware versions. If you blindly restore your backup you would not benefit from these new defaults or may even end up with completely wrong settings in case some parameters changed semantics and/or value ranges.

It may be good idea to restore settings using the `diff` output rather than complete `dump`. This way you can have more control on what is restored and the risk of mistakenly restoring bad values if the semantics changes is minimised.

To perform the restore simply paste the saved commands in the Configurator CLI tab and then type `save`.

After restoring it's always a good idea to `dump` or `diff` the settings once again and compare the output with previous one to verify if everything is set as it should be.

## Flight Controller opereration while connected to the CLI

While connected to the CLI, all Logical Switches are temporarily disabled (5.1.0 onwards).

## CLI Command Reference

| `Command` | Description |
|-----------| ----------- |
| `adjrange` | Configure adjustment ranges |
| `assert` |  |
| `aux` | Configure modes |
| `batch` | Start or end a batch of commands |
| `battery_profile` | Change battery profile |
| `beeper` | Show/set beeper (buzzer) [usage](Buzzer.md) |
| `bind_rx` | Initiate binding for SRXL2 or CRSF receivers |
| `blackbox` | Configure blackbox fields |
| `bootlog` | Show boot events |
| `color` | Configure colors |
| `defaults` | Reset to defaults and reboot |
| `dmesg` | Show init logs from [serial_printf_debugging](./development/serial_printf_debugging.md) |
| `dfu` | DFU mode on reboot |
| `diff` | List configuration changes from default |
| `dump` | Dump configuration |
| `exit` |  |
| `feature` | List or enable <val> or disable <-val> |
| `flash_erase` | Erase flash chip |
| `flash_info` | Show flash chip info |
| `flash_read` |  |
| `flash_write` |  |
| `get` | Get variable value |
| `gpspassthrough` | Passthrough gps to serial |
| `gvar` | Configure global variables |
| `help` | Displays CLI help and command parameters / options |
| `led` | Configure leds |
| `logic` | Configure logic conditions |
| `map` | Configure rc channel order |
| `memory` | View memory usage |
| `mmix` | Custom motor mixer |
| `mode_color` | Configure mode and special colors |
| `motor` | Get/set motor |
| `msc` | Enter USB Mass storage mode. See [USB MSC documentation](USB_Mass_Storage_(MSC)_mode.md) for usage information. |
| `osd_layout` | Get or set the layout of OSD items |
| `pid` | Configurable PID controllers |
| `play_sound` | `<index>`, or none for next item |
| `profile` | Change profile |
| `resource` | View currently used resources |
| `rxrange` | Configure rx channel ranges |
| `safehome` | Define safe home locations. See the [safehome documentation](Safehomes.md) for usage information. |
| `save` | Save and reboot |
| `sd_info` | Sdcard info |
| `serial` | Configure serial ports. [Usage](Serial.md) |
| `serialpassthrough` | Passthrough serial data to port, with `<id> <baud> <mode> <options>`, where `id` is the zero based port index, `baud` is a standard baud rate, mode is `rx`, `tx`, or both (`rxtx`), and options is a short string like `8N1` or `8E2` |
| `servo` | Configure servos |
| `set` | Change setting with name=value or blank or * for list |
| `smix` | Custom servo mixer |
| `status` | Show status. Error codes can be looked up [here](https://github.com/iNavFlight/inav/wiki/%22Something%22-is-disabled----Reasons) |
| `tasks` | Show task stats |
| `temp_sensor` | List or configure temperature sensor(s). See [temperature sensors documentation](Temperature-sensors.md) for more information. |
|  `timer_output_mode`  | Override automatic timer /  pwm function allocation. [Additional Information](#timer_outout_mode)|
| `version` | Show version |
| `wp` | List or configure waypoints. See the [navigation documentation](Navigation.md#cli-command-wp-to-manage-waypoints). |

Notes:

* Available commands depend upon hardware specific and debug build options. Not all commands are available in every FC.
* The above list shows the output available in the CLI `help` command. This may also show additional information.

### serial

The syntax of the `serial` command is `serial <id>  <function_value> <msp-baudrate> <gps-baudrate> <telemetry-baudate> <peripheral-baudrate>`.

A shorter form is also supported to enable and disable a single function using `serial <id> +n` and `serial <id> -n`, where n is the a serial function identifier. The following values are available:

| Function              | Bit Identifier | Numeric value |
|-----------------------|---------------|----------------|
| MSP                   | 0             | 1 |
| GPS                   | 1             | 2 |
| TELEMETRY_FRSKY       | 2             | 4 |
| TELEMETRY_HOTT        | 3             | 8 |
| TELEMETRY_LTM         | 4             | 16 |
| TELEMETRY_SMARTPORT   | 5             | 32 |
| RX_SERIAL             | 6             | 64 |
| BLACKBOX              | 7             | 128 |
| TELEMETRY_MAVLINK     | 8             | 256 |
| TELEMETRY_IBUS        | 9             | 512 |
| RCDEVICE              | 10            | 1024 |
| VTX_SMARTAUDIO        | 11            | 2048 |
| VTX_TRAMP             | 12            | 4096 |
| UAV_INTERCONNECT      | 13            | 8192 |
| OPTICAL_FLOW          | 14            | 16384 |
| LOG                   | 15            | 32768 |
| RANGEFINDER           | 16            | 65536 |
| VTX_FFPV              | 17            | 131072 |
| ESCSERIAL             | 18            | 262144 |
| TELEMETRY_SIM         | 19            | 524288 |
| FRSKY_OSD             | 20            | 1048576 |
| DJI_HD_OSD            | 21            | 2097152 |
| SERVO_SERIAL          | 22            | 4194304 |
| TELEMETRY_SMARTPORT_MASTER | 23       | 8388608 |
| UNUSED                | 24            | 16777216 |
| MSP_DISPLAYPORT       | 25            | 33554432 |
| GIMBAL_SERIAL         | 26            | 67108864 |
| HEADTRACKER_SERIAL    | 27            | 134217728 |

Thus, to enable MSP and LTM on a port, one would use the function **value** of 17 (1 << 0)+(1<<4), aka 1+16, aka 17.

```
serial 0 17 57600 57600 57600 57600
```
but to remove LTM using the +/- shorthand, use the **bit Id** (4, TELEMETRY_LTM):

```
serial 0 -4
```

`serial` can also be used without any argument to print the current configuration of all the serial ports.

### `timer_output_mode`

Since INAV 7, the firmware can dynamically allocate servo and motor outputs. This removes the need for bespoke targets for special cases (e.g. `MATEKF405` and `MATEKF405_SERVOS6`).

#### Syntax

```
timer_output_mode [timer [function]]
```
where:
* Without parameters, lists the current timers and modes
* With just a `timer` lists the mode for that timer
* With both `timer` and `function`, sets the function for that timers

Note:

* `timer` identifies the timer **index** (from 0); thus is one less than the corresponding `TIMn` definition in a target's `target.c`.
* The function is one of `AUTO` (the default), `MOTORS` or `SERVOS`.

Motors are allocated first, hence having a servo before a motor may require use of `timer_output_mode`.

#### Example

The original `MATEKF405` target defined a multi-rotor (MR) servo on output S1. The later `MATEKF405_SERVOS6` target defined (for MR) S1 as a motor and S6 as a servo. This was more logical, but annoying for anyone who had a legacy `MATEKF405` tricopter with the servo on S1.

#### Solution

There is now a single `MATEKF405` target. The `target.c` sets the relevant  outputs as:

```
DEF_TIM(TIM3, CH1, PC6,  TIM_USE_OUTPUT_AUTO, 0, 0), // S1
DEF_TIM(TIM8, CH2, PC7,  TIM_USE_OUTPUT_AUTO, 0, 1), // S2  UP(2,1)
DEF_TIM(TIM8, CH3, PC8,  TIM_USE_OUTPUT_AUTO, 0, 1), // S3  UP(2,1)
DEF_TIM(TIM8, CH4, PC9,  TIM_USE_OUTPUT_AUTO, 0, 0), // S4  UP(2,1)
DEF_TIM(TIM2, CH1, PA15, TIM_USE_MC_MOTOR | TIM_USE_LED, 0, 0), // S5  UP(1,7)
DEF_TIM(TIM1, CH1, PA8,  TIM_USE_OUTPUT_AUTO, 0, 0), // S6  UP(2,5)
DEF_TIM(TIM4, CH3, PB8,  TIM_USE_OUTPUT_AUTO, 0, 0), // S7  D(1,7)!S5 UP(2,6)
```

Using the "motors first" allocation, the servo would end up on S6, which in the legacy "tricopter servo on S1" case is not desired.

Forcing the S1 output (`TIM3`) to servo is achieved by:

```
timer_output_mode 2 SERVOS
```

with resulting `resource` output:

```
C06: SERVO4 OUT
C07: MOTOR1 OUT
C08: MOTOR2 OUT
C09: MOTOR3 OUT
```

Note that the `timer` **index** in the `timer_output_mode` line is one less than the mnemonic in `target.c`, `timer` of 2 for `TIM3`.

Note that the usual caveat that one should not share a timer with both a motor and a servo still apply.

## Flash chip management

For targets that have a flash data chip, typically used for blackbox logs, the following additional comamnds are provided.

| Command | Effect |
| ------- | ------ |
| `flash_erase` | Erases the  flash chip |
| `flash_info` | Displays flash chip information (used, free etc.) |
| `flash_read <length> <address>` | Reads `length` bytes from `address` |
| `flash_write <address> <data>` | Writes `data` to `address` |

## CLI Variable Reference
See [Settings.md](Settings.md).
