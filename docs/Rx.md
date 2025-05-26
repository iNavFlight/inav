# Receivers (RX)

A receiver is used to receive radio control signals from your transmitter and convert them into signals that the flight controller can understand.

There are a number of types of receivers:

* PPM Receivers (obsolete)
* Serial Receivers
* MSP RX

## PPM Receivers

**Only supported in INAV 3.x and below**

PPM is sometimes known as PPM SUM or CPPM.

12 channels via a single input pin, not as accurate or jitter free as methods that use serial communications, but readily available.

These receivers are reported working:

FrSky D4R-II
http://www.frsky-rc.com/product/pro.php?pro_id=24

Graupner GR24
http://www.graupner.de/en/products/33512/product.aspx

R615X Spektrum/JR DSM2/DSMX Compatible 6Ch 2.4GHz Receiver w/CPPM
http://orangerx.com/2014/05/20/r615x-spektrumjr-dsm2dsmx-compatible-6ch-2-4ghz-receiver-wcppm-2/

FrSky D8R-XP 8ch telemetry receiver, or CPPM and RSSI enabled receiver
http://www.frsky-rc.com/product/pro.php?pro_id=21

## Serial Receivers

*Connect the receivers to UARTs and not to Software Serial ports. Using software serial for RX input can cause unexpected behaviours beacause the port cannot handle reliably the bit rate needed by the most common protocols*

### Spektrum

This section describes the legacy Spektrum satellite capability; the newer SRXL2 protocol is described [later in this document](#srxl2) .

8 channels via serial currently supported.

These receivers are reported working:

Lemon Rx DSMX Compatible PPM 8-Channel Receiver + Lemon DSMX Compatible Satellite with Failsafe
http://www.lemon-rx.com/shop/index.php?route=product/product&product_id=118


#### Spektrum pesudo RSSI

As of INAV 1.6, a pseudo RSSI, based on satellite fade count is supported and reported as normal INAV RSSI (0-1023 range). In order to use this feature, the following is necessary:

* Bind the satellite receiver using a physical RX; the bind function provided by the flight controller is not sufficient.
* The CLI variable `rssi_channel` is set to channel 9:
```
set rssi_channel = 9
```
This pseudo-RSSI should work on all makes of Spektrum satellite RX; it is tested as working on [Lemon RX satellites](http://www.lemon-rx.com/index.php?route=product/product&path=72&product_id=109 and http://www.lemon-rx.com/index.php?route=product/product&path=72&product_id=135) (recommended).

### S.BUS

16 channels via serial currently supported.  See below how to set up your transmitter.

* You probably need an inverter between the receiver output and the flight controller. However, some flight controllers have this built in and doesn't need one.
* Some OpenLRS receivers produce a non-inverted SBUS signal. It is possible to switch SBUS inversion off using CLI command `set sbus_inversion = OFF` when using an F3 based flight controller.
* Softserial ports cannot be used with SBUS because it runs at too high of a bitrate (1Mbps).  Refer to the chapter specific to your board to determine which port(s) may be used.
* You will need to configure the channel mapping in the GUI (Receiver tab) or CLI (`map` command). Note that channels above 8 are mapped "straight", with no remapping.

These receivers are reported working:

FrSky X4RSB 3/16ch Telemetry Receiver
http://www.frsky-rc.com/product/pro.php?pro_id=135

FrSky X8R 8/16ch Telemetry Receiver
http://www.frsky-rc.com/product/pro.php?pro_id=105

Futaba R2008SB 2.4GHz S-FHSS
http://www.futaba-rc.com/systems/futk8100-8j/


#### OpenTX S.BUS configuration

If using OpenTX set the transmitter module to D16 mode and ALSO select CH1-16 on the transmitter before binding to allow reception
of all 16 channels.

OpenTX 2.09, which is shipped on some Taranis X9D Plus transmitters, has a bug - [issue:1701](https://github.com/opentx/opentx/issues/1701).
The bug prevents use of all 16 channels.  Upgrade to the latest OpenTX version to allow correct reception of all 16 channels,
without the fix you are limited to 8 channels regardless of the CH1-16/D16 settings.


### F.Port

F.Port is a protocol running on async serial allowing 16 controls channels and telemetry on a single UART.

Supported receivers include FrSky R-XSR, X4R, X4R-SB, XSR, XSR-M, R9M Slim, R9M Slim+, R9 Mini. For ACCST receivers you need to flash the corresponding firmware for it to output F.Port. For ACCESS receivers the protocol output from the receiver can be switched between S.Bus and F.Port from the model's setup page in the RX options.

#### Connection

Just connect the S.Port wire from the receiver to the TX pad of a free UART on your flight controller

#### Configuration

For INAV 2.6 and newer versions, the default configuration should just work. However, if you're
upgrading from a previous version you might need to set the following settings to their
default values:

```
set serialrx_inverted = OFF
set serialrx_halfduplex = AUTO
```

For INAV versions prior to 2.6, you need to change the following settings:

```
set serialrx_inverted = ON
set serialrx_halfduplex = ON
```

### SUMD

16 channels via serial currently supported.

These receivers are reported working:

GR-24 receiver HoTT
http://www.graupner.de/en/products/33512/product.aspx

Graupner receiver GR-12SH+ HoTT
http://www.graupner.de/en/products/870ade17-ace8-427f-943b-657040579906/33565/product.aspx

### IBUS

10 channels via serial currently supported.

IBUS is the FlySky digital serial protocol and is available with the FS-IA6B, FS-X6B and FS-IA10 receivers.
The Turnigy TGY-IA6B and TGY-IA10 are the same devices with a different label, therefore they also work.

IBUS can provide up to 120Hz refresh rate, more than double compared to standard 50Hz of PPM.

FlySky FS-I6X TX natively supports 10ch.

If you are using a 6ch TX such as the FS-I6 or TGY-I6 then you must flash a 10ch
firmware on the TX to make use of these extra channels.
The flash is avaliable here: https://github.com/benb0jangles/FlySky-i6-Mod-
```
     _______
    /       \                               /------------\
    | STM32 |-->UART RX-->[115200 baud]---->| Flysky RX  |
    |  uC   |-  UART TX--x[not connected]   | IBUS-Servo |
    \_______/                               \------------/
```
After flash "10ch Timer Mod i6 Updater", it is passible to get RSSI signal on selected Aux channel from FS-i6 Err sensor.

It is possible to use IBUS RX and IBUS telemetry on only one port of the hardware UART. More information in Telemetry.md.

### SRXL2

SRXL2 is a newer Spektrum protocol that provides a bidirectional link between the FC and the receiver, allowing the user to get FC telemetry data and basic settings on Spektrum Gen 2 airware TX. SRXL2 is supported in INAV 2.6 and later. It offers improved performance and features compared to earlier Spektrum RX.

#### Wiring

Signal pin on receiver (labeled "S") must be wired to a **UART TX** pin on the FC. Voltage can be 3.3V (4.0V for SPM4651T) to 8.4V. On some F4 FCs, the TX pin may have a signal inverter (such as for S.Port). Make sure this isn't the case for the pin you intend to use.

#### Configuration

Selection of SXRL2 is provided in the INAV 2.6 and later configurators. It is necessary to complete the configuration via the CLI; the following settings are recommended:

```
feature TELEMETRY
feature -RSSI_ADC
map TAER
set receiver_type = SERIAL
set serialrx_provider = SRXL2
set serialrx_inverted = OFF
set srxl2_unit_id = 1
set srxl2_baud_fast = ON
set rssi_source = PROTOCOL
set rssi_channel = 0
```

#### Notes:

* RSSI_ADC is disabled, as this would override the value provided through SRXL2
* `rssi_channel = 0` is required, unlike earlier Spektrum devices (e.g. SPM4649T).

Setting these values differently may have an adverse effects on RSSI readings.

#### CLI Bind Command

This command will put the receiver into bind mode without the need to reboot the FC as it was required with the older `spektrum_sat_bind` command.

```
bind_rx
```

## MultiWii serial protocol (MSP RX)

Allows you to use MSP commands as the RC input. Up to 18 channels are supported.
Note:
* It is necessary to update `MSP_SET_RAW_RC` at 5Hz or faster.
* `MSP_SET_RAW_RC` uses the defined RC channel map
* `MSP_RC` returns `AERT` regardless of channel map
* You can combine "real" RC radio and MSP RX by using `msp_override_channels` to set the channels to be overridden.
* The [wiki Remote Control and Management article](https://github.com/iNavFlight/inav/wiki/INAV-Remote-Management,-Control-and-Telemetry) provides more information, including links to 3rd party projects that exercise `MSP_SET_RAW_RC` and `USE_MSP_RC_OVERRIDE`

## SIM (SITL) Joystick

Enables the use of a joystick in the INAV SITL with a flight simulator. See the [SITL documentation](SITL/SITL.md).

## Configuration

The receiver type can be set from the configurator or CLI.

```
# get receiver_type
receiver_type = NONE
Allowed values: NONE, SERIAL, MSP, SIM (SITL)
```

### RX signal-loss detection

The software has signal loss detection which is always enabled.  Signal loss detection is used for safety and failsafe reasons.

The `rx_min_usec` and `rx_max_usec` settings helps detect when your RX stops sending any data, enters failsafe mode or when the RX loses signal.

By default, when the signal loss is detected the FC will set pitch/roll/yaw to the value configured for `mid_rc`. The throttle will be set to the value configured for `rx_min_usec` or `mid_rc` if using 3D feature.

Signal loss can be detected when:

1. no rx data is received (due to radio reception, recevier configuration or cabling issues).
2. using Serial RX and receiver indicates failsafe condition.
3. using any of the first 4 stick channels do not have a value in the range specified by `rx_min_usec` and `rx_max_usec`.

#### `rx_min_usec`

The lowest channel value considered valid.  e.g. PWM/PPM pulse length

#### `rx_max_usec`

The highest channel value considered valid.  e.g. PWM/PPM pulse length

### Serial RX

See the Serial chapter for some some RX configuration examples.

To setup spectrum in the GUI:
1. Start on the "Ports" tab make sure that teh required has serial RX.  If not set the checkbox, save and reboot.
2. Move to the "Configuration" page and in the upper lefthand corner choose Serial RX as the receiver type.
3. Below that choose the type of serial receiver that you are using.  Save and reboot.

#### Using CLI:

For Serial RX set the `receiver_type` and `serialrx_provider` setting as appropriate for your RX.

```
# get rec
receiver_type = SERIAL
Allowed values: NONE, SERIAL, MSP, SIM (SITL)

# get serialrx
serialrx_provider = SBUS
Allowed values: SPEK1024, SPEK2048, SBUS, SUMD, IBUS, JETIEXBUS, CRSF, FPORT, SBUS_FAST, FPORT2, SRXL2, GHST, MAVLINK, FBUS

```

## Receiver configuration.

### FrSky D4R-II

Set the RX for 'No Pulses'.  Turn OFF TX and RX, Turn ON RX.  Press and release F/S button on RX.  Turn off RX.

### Graupner GR-24 PWM

Set failsafe on the throttle channel in the receiver settings (via transmitter menu) to a value below `rx_min_usec` using channel mode FAILSAFE.
This is the prefered way, since this is *much faster* detected by the FC then a channel that sends no pulses (OFF).

__NOTE:__
One or more control channels may be set to OFF to signal a failsafe condition to the FC, all other channels *must* be set to either HOLD or OFF.
Do __NOT USE__ the mode indicated with FAILSAFE instead, as this combination is NOT handled correctly by the FC.

## Receiver Channel Range Configuration.

If you have a transmitter/receiver, that output a non-standard pulse range (i.e. 1070-1930 as some Spektrum receivers)
you could use rx channel range configuration to map actual range of your transmitter to 1000-2000 as expected by INAV.

The low and high value of a channel range are often referred to as 'End-points'.  e.g. 'End-point adjustments / EPA'.

All attempts should be made to configure your transmitter/receiver to use the range 1000-2000 *before* using this feature
as you will have less preceise control if it is used.

To do this you should figure out what range your transmitter outputs and use these values for rx range configuration.
You can do this in a few simple steps:

If you have used rc range configuration previously you should reset it to prevent it from altering rc input. Do so
by entering the following command in CLI:
```
rxrange reset
save
```

Now reboot your FC, connect the configurator, go to the `Receiver` tab move sticks on your transmitter and note min and
max values of first 4 channels. Take caution as you can accidentally arm your craft. Best way is to move one channel at
a time.

Go to CLI and set the min and max values with the following command:
```
rxrange <channel_number> <min> <max>
```

For example, if you have the range 1070-1930 for the first channel you should use `rxrange 0 1070 1930` in
the CLI. Be sure to enter the `save` command to save the settings.

After configuring channel ranges use the sub-trim on your transmitter to set the middle point of pitch, roll, yaw and throttle.


You can also use rxrange to reverse the direction of an input channel, e.g. `rxrange 0 2000 1000`.
