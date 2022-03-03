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


## CLI Command Reference

| `Command` | Description |
|-----------| ----------- |
| `adjrange` | Configure adjustment ranges |
| `assert` |  |
| `aux` | Configure modes |
| `batch` | Start or end a batch of commands |
| `battery_profile` | Change battery profile |
| `beeper` | Show/set beeper (buzzer) [usage](Buzzer.md) |
| `bind_rx` | Initiate binding for RX SPI or SRXL2 |
| `blackbox` | Configure blackbox fields |
| `bootlog` | Show boot events |
| `color` | Configure colors |
| `defaults` | Reset to defaults and reboot |
| `dfu` | DFU mode on reboot |
| `diff` | List configuration changes from default |
| `dump` | Dump configuration |
| `eleres_bind` |  |
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
| `imu2` | Secondary IMU |
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
| `serialpassthrough` | Passthrough serial data to port, with `<id> <baud> <mode>`, where `id` is the zero based port index, `baud` is a standard baud rate, and mode is `rx`, `tx`, or both (`rxtx`) |
| `servo` | Configure servos |
| `set` | Change setting with name=value or blank or * for list |
| `smix` | Custom servo mixer |
| `status` | Show status |
| `tasks` | Show task stats |
| `temp_sensor` | List or configure temperature sensor(s). See [temperature sensors documentation](Temperature-sensors.md) for more information. |
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
| IMU2                  | 24            | 16777216 |
| HDZERO                | 25            | 33554432 |

Thus, to enable MSP and LTM on a port, one would use the function **value** of 17 (1 << 0)+(1<<4), aka 1+16, aka 17.

```
serial 0 17 57600 57600 57600 57600
```
but to remove LTM using the +/- shorthand, use the **bit Id** (4, TELEMETRY_LTM):

```
serial 0 -4
```

`serial` can also be used without any argument to print the current configuration of all the serial ports.

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
