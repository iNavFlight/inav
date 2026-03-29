# Rangefinder

Rangefinders are devices used to measure altitude Above Ground Level (AGL), the distance to the ground directly beneath the aircraft.

## Features

Rangefinders in INAV are used for:

* **Landing detection** for multirotors - precise touchdown detection
* **Automated landing support** for fixed wings - smooth descent control
* **Terrain following (Surface mode)** - maintain constant height above ground (multirotors only)
* **Position estimation** - when combined with optical flow sensors, enables GPS-free navigation

## Hardware

INAV supports the following rangefinder types:

### Time-of-Flight (ToF) Laser Sensors

* **VL53L0X** - STMicroelectronics laser rangefinder, 0-75cm range (I2C)
* **VL53L1X** - STMicroelectronics laser rangefinder, 0-400cm range (I2C)
* **TOF10120** - Small & lightweight laser sensor, 0-200cm range (I2C)
* **TERARANGER_EVO** - TeraRanger Evo series, 30-600cm range depending on model (I2C/UART)
  - https://www.terabee.com/sensors-modules/lidar-tof-range-finders/#individual-distance-measurement-sensors

### Ultrasonic Sensors

* **SRF10** - Devantech SRF10, 0-600cm range (I2C)
* **US42** - Maxbotix US-42, 0-645cm range (I2C)
* **USD1_V0** - USD1 ultrasonic sensor variant (UART)

### Radar Sensors

* **NRA** - NanoRadar NRA15/NRA24 millimeter-wave radar (UART)

NRA15/NRA24 sensors can use US-D1_V0 or NRA protocol depending on firmware configuration:

| Radar | Protocol | Resolution      | Name in configurator |
|-------|----------|-----------------|----------------------|
| NRA15 | US-D1_V0 | 0-30m (±4cm)    | USD1_V0              |
| NRA15 | NRA      | 0-30m (±4cm)    | NRA                  |
| NRA15 | NRA      | 0-100m (±10cm)  | NRA                  |
| NRA24 | US-D1_V0 | 0-50m (±4cm)    | USD1_V0              |
| NRA24 | US-D1_V0 | 0-200m (±10cm)  | USD1_V0              |
| NRA24 | NRA      | 0-50m (±4cm)    | NRA                  |
| NRA24 | NRA      | 0-200m (±10cm)  | NRA                  |

### Serial/External Sensors

* **MSP** - External rangefinder via MSP protocol (UART/MSP)
* **BENEWAKE** - Benewake LIDAR modules (TFmini, etc.) via serial protocol (UART)

### Development/Testing

* **FAKE** - Simulated rangefinder for testing without hardware

## Configuration

Enable rangefinder in CLI:

```
set rangefinder_hardware = VL53L0X  # or your sensor type
save
```

Optional median filtering for smoother readings:

```
set rangefinder_median_filter = ON
save
```

### Position Estimation Weights

Control how rangefinder data is fused into altitude estimates:

```
set inav_max_surface_altitude = 200      # Max rangefinder range to use (cm)
set inav_w_z_surface_p = 3.5             # Position weight
set inav_w_z_surface_v = 6.1             # Velocity weight
```

## Surface Mode (Terrain Following)

Surface mode enables terrain following on **multirotors only** by maintaining constant altitude above ground instead of absolute altitude.

**Activation:** Enable SURFACE mode via RC switch (requires rangefinder configured)

**Compatible with:**
- ALTHOLD - Maintains height above ground
- POSHOLD - Holds position with terrain-relative altitude
- CRUISE - Velocity control with terrain following

**Settings:**

```
set nav_max_terrain_follow_alt = 100    # Max altitude in Surface mode (cm)
```

**Important:** Always set `nav_max_terrain_follow_alt` **less than** `inav_max_surface_altitude` to prevent attempting to use the sensor beyond its reliable range. For example, if your rangefinder is reliable up to 200cm, set `inav_max_surface_altitude = 200` and `nav_max_terrain_follow_alt = 100` (or lower).

**Note:** Surface mode is NOT available on fixed wing aircraft.

## Connections

### I2C Rangefinders

I2C sensors (VL53L0X, VL53L1X, TOF10120, SRF10, US42, TERARANGER_EVO) connect to the flight controller's I2C port and are auto-detected when configured.

### Serial Rangefinders

UART-based sensors (MSP, BENEWAKE, NRA, USD1_V0) require:
1. Assign UART port in Ports tab
2. Configure `rangefinder_hardware` setting
3. Set appropriate baud rate

## Optical Flow Integration

When a rangefinder is combined with an optical flow sensor:
- Rangefinder provides altitude (Z-axis)
- Optical flow provides horizontal movement (X/Y axes)
- Together they enable GPS-free position hold indoors

See wiki for optical flow setup: https://github.com/iNavFlight/inav/wiki/Optic-Flow-and-Rangefinder

## Constraints

**Not Supported:**
- HC-SR04 ultrasonic sensors
- US-100 ultrasonic sensors

**Limitations:**
- Surface mode: Multirotors only
- Tilt angle: Rangefinder disabled if aircraft tilt exceeds sensor detection cone
- Range: Set `inav_max_surface_altitude` to sensor's reliable range, not maximum range

## Troubleshooting

**No rangefinder readings:**
- Verify correct `rangefinder_hardware` selection
- Check wiring and power (3.3V or 5V depending on sensor)
- Ensure I2C address is unique (no conflicts)
- Check sensor has clear line of sight to ground

**Erratic readings:**
- Enable `rangefinder_median_filter = ON`
- Check for vibration - mount sensor rigidly
- Verify sensor is not affected by propeller wash
- Ensure ground surface has sufficient texture (not smooth/reflective)

**Surface mode not working:**
- Verify SURFACE mode is enabled via switch
- Check rangefinder is providing valid data
- Ensure altitude is below `nav_max_terrain_follow_alt` (operational limit)
- Ensure altitude is within sensor's reliable range (set by `inav_max_surface_altitude`)
- Confirm platform is multirotor (fixed wings not supported)

## References

- [INAV Wiki: Optic Flow and Rangefinder Setup](https://github.com/iNavFlight/inav/wiki/Optic-Flow-and-Rangefinder)
- [Settings Reference](Settings.md)
