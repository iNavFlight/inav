# Buzzer

INAV supports a buzzer which is used for the following purposes:

 * Low and critical battery alarms (when battery monitoring enabled)
 * Arm/disarm tones (and warning beeps while armed)
 * Notification of calibration complete status
 * TX-AUX operated beeping - useful for locating your aircraft after a crash
 * Failsafe status
 * Flight mode change
 * Rate profile change (via TX-AUX switch)

Three beeps immediately after powering the board means that the gyroscope calibration has completed successfully. INAV calibrates the gyro automatically upon every power-up. It is important that the copter stay still on the ground until the three beeps sound, so that gyro calibration isn't thrown off. If you move the copter significantly during calibration, INAV will detect this, and will automatically re-start the calibration once the copter is still again. This will delay the "three beeps" tone. If you move the copter just a little bit, the gyro calibration may be incorrect, and the copter may not fly correctly. In this case, the gyro calibration can be performed manually via [stick command](Controls.md), or you may simply power cycle the board.

There is a special arming tone used if a GPS fix has been attained, and there's a "ready" tone sounded after a GPS fix has been attained (only happens once).  The tone sounded via the TX-AUX-switch will count out the number of satellites (if GPS fix).

The CLI command `play_sound` is useful for demonstrating the buzzer tones. Repeatedly entering the command will play the various tones in turn. Entering the command with a numeric-index parameter (see below) will play the associated tone.

Buzzer is enabled by default on platforms that have buzzer connections.

## Tone sequences

Buzzer tone sequences (square wave generation) are made so that : 1st, 3rd, 5th, .. are the delays how long the beeper is on and 2nd, 4th, 6th, .. are the delays how long beeper is off. Delays are in milliseconds/10 (i.e., 5 => 50ms).

Sequences:

    0    RUNTIME_CALIBRATION       20, 10, 20, 10, 20, 10     Sensor calibration finished while disarmed (at power-up,
                                                              or e.g. after a gyro calibration by sticks)
    1    HW_FAILURE                10, 10                     A hardware failure was detected (repeats)
    2    RX_LOST                   50, 50                     RX signal lost or FAILSAFE mode on, only after the first
                                                              arm since power-up (repeats while it lasts)
    3    RX_LOST_LANDING           10, 10, 10, 10, 10, 40,    SOS morse code
                                   40, 10, 40, 10, 40, 40,
                                   10, 10, 10, 10, 10, 70
    4    DISARMING                 15, 5, 15, 5               Disarming the board
    5    ARMING                    30, 5, 5, 5                Arming the board
    6    ARMING_GPS_FIX            5, 5, 15, 5, 5, 5, 15, 30  Arming and GPS has fix
    7    BAT_CRIT_LOW              50, 2                      Battery is critically low (repeats)
    8    BAT_LOW                   25, 50                     Battery is getting low (repeats)
    9    GPS_STATUS                multi beeps                Never sounds in normal use (only via play_sound); the
                                                              satellite count plays as MULTI_BEEPS
    10   RX_SET                    10, 10                     BEEPER mode on; with GPS enabled only while there is no
                                                              GPS fix or fewer than 5 satellites
    11   ACTION_SUCCESS            5, 5, 5, 5                 Waypoint list saved or loaded by sticks; compass
                                                              calibration started (without a compass: heading set to
                                                              north); temperature auto-calibration ended with a
                                                              correction
    12   ACTION_FAIL               20, 15, 35, 5              Waypoint list save or load by sticks failed, or list
                                                              erased by sticks; temperature auto-calibration ended
                                                              without a correction
    13   READY_BEEP                4, 5, 4, 5, 8, 5, 15, 5,   GPS locked and craft ready
                                   8, 5, 4, 5, 4, 5
    14   MULTI_BEEPS               multi beeps                Short beeps: flight mode change or arming blocked (1),
                                                              profile change (profile number), in-flight adjustment
                                                              (1 down, 2 up), accelerometer calibration position
                                                              done (2), settings saved (1), satellite count while BEEPER
                                                              mode is on (GPS fix and at least 5 satellites)
    15   DISARM_REPEAT             0, 100, 10                 Never sounds in normal use (only via play_sound)
    16   ARMED                     0, 245, 10, 5              Armed with throttle low and motorstop_on_low on, not on
                                                              airplanes, rovers or boats (repeats until disarmed or
                                                              throttle raised)
    17   SYSTEM_INIT               none                       Not played from this table; disabling it mutes the ten
                                                              short power-up beeps
    18   ON_USB                    none                       Not a sequence: when disabled, all tones except the
                                                              power-up beeps are muted while no battery is detected
                                                              (needs VBAT)
    19   LAUNCH_MODE               5, 5, 5, 100               Fixed wing launch mode active
    20   LAUNCH_MODE_LOW_THROTTLE  5, 5, 5, 5, 3, 100         Launch mode active, throttle below the launch threshold
    21   LAUNCH_MODE_IDLE_START    5, 5, 5, 5, 5, 5, 5, 80    Launch mode: motor starts at idle throttle within 5 s
                                                              (nav_fw_launch_idle_motor_delay)
    22   CAM_CONNECTION_OPEN       5, 15, 10, 15, 20          Camera control connection opened
    23   CAM_CONNECTION_CLOSED     10, 8, 5                   Camera control connection closed
    24   ALL                       none                       Not a sequence: beeper ALL / beeper -ALL enables or
                                                              disables every tone
    25   PREFERED                  none                       Not a sequence: beeper PREFERED stores the current set,
                                                              beeper -PREFERED restores it

You can use [this tool](https://www.mrd-rc.com/tutorials-tools-and-testing/useful-tools/helpful-inav-buzzer-code-checker/) to hear current buzzer sequences or enter custom sequences.

## Controlling buzzer usage

The usage of the buzzer can be controlled by the CLI `beeper` command.

### List current usage

```
beeper 
```
### List all buzzer setting options

```
beeper list
```
giving:

```
Available:  RUNTIME_CALIBRATION  HW_FAILURE  RX_LOST  RX_LOST_LANDING  DISARMING  ARMING  ARMING_GPS_FIX  BAT_CRIT_LOW  BAT_LOW  GPS_STATUS  RX_SET  ACTION_SUCCESS  ACTION_FAIL  READY_BEEP  MULTI_BEEPS  DISARM_REPEAT  ARMED  SYSTEM_INIT  ON_USB  LAUNCH_MODE  LAUNCH_MODE_LOW_THROTTLE  LAUNCH_MODE_IDLE_START  CAM_CONNECTION_OPEN  CAM_CONNECTION_CLOSED  ALL  PREFERED
```

The `beeper` command  syntax follows that of the `feature` command; a minus (`-`) in front of a name disables that function.

So to disable the beeper / buzzer when 	powered by USB (may enhance domestic harmony):

```
beeper -ON_USB
```

Now the `beeper` command will show:

```
# beeper
Disabled:  ON_USB
```

*Note: SYSTEM_INIT sequence is not affected by ON_USB setting and will still be played on USB connection. Disable both ON_USB and SYSTEM_INIT to disable buzzer completely when FC is powered from USB.*

*Note: ON_USB setting requires present and configured battery voltage metter.*

To disable all features use:

```
beeper -ALL
```

To store current set to preferences use (preferences also require ```save```):

```
beeper PREFERED
```

To restore set from preferences use:

```
beeper -PREFERED
```

To activate an external beeper via aux channel switch, assign aux channel and set both:

```
beeper RX_SET
beeper MULTI_BEEPS
```
If MULTI_BEEPS is not set, the beeper will not sound after GPS lock.

As with other CLI commands, the `save` command is needed to save the new settings.

## Types of buzzer supported

Most FCs require ACTIVE buzzers. Active buzzers are enabled/disabled by simply enabling or disabling a GPIO output pin on the board.
This means the buzzer must be able to generate its own tone simply by having power applied to it.

Passive buzzers that need an analog or PWM signal do not work and will make clicking noises or no sound at all.

Passive buzzers are supported on FCs which are designed to work with passive buzzers only (so far there is no available, except rare cases like Matek F765-WSE where passive buzzer is preinstalled).

Examples of a known-working buzzers.

 * [Hcm1205x Miniature Buzzer 5v](http://www.rapidonline.com/Audio-Visual/Hcm1205x-Miniature-Buzzer-5v-35-0055)
 * [MultiComp MCKPX-G1205A-3700 TRANSDUCER, THRU-HOLE, 4V, 30MA](http://uk.farnell.com/multicomp/mckpx-g1205a-3700/transducer-thru-hole-4v-30ma/dp/2135914?CMP=i-bf9f-00001000)
 * [3-24V Piezo Electronic Tone Buzzer Alarm 95DB](https://inavflight.com/shop/s/bg/919348)
