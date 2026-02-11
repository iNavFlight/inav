# DroneCAN

DroneCAN (formerly UAVCAN v0) is a lightweight protocol designed for reliable communication in aerospace and robotic applications over CAN bus. INAV supports DroneCAN for connecting external sensors and peripherals.

## Supported Features

| Feature | Status | Description |
|---------|--------|-------------|
| GPS | Supported | GNSS receivers via DroneCAN |
| Battery Voltage | Supported | Voltage sensing from DroneCAN battery monitors |
| Battery Current | Supported | Current sensing from DroneCAN battery monitors |
| Parameter Get/Set | Planned | Remote parameter configuration |
| ESC Control | Planned | Motor control via DroneCAN ESCs |

## Supported Hardware

DroneCAN requires a flight controller with CAN bus hardware:

- **STM32H7** boards with FDCAN peripheral
- **STM32F7** boards with bxCAN peripheral

Check your board's documentation to confirm CAN bus availability.

## Configuration

### Enabling DroneCAN

DroneCAN settings are configured via CLI:

```
set dronecan_node_id = 10
set dronecan_bitrate = 1000KBPS
save
```

### Settings

| Setting | Values | Default | Description |
|---------|--------|---------|-------------|
| `dronecan_node_id` | 1-127 | 10 | CAN node ID for the flight controller |
| `dronecan_bitrate` | 125KBPS, 250KBPS, 500KBPS, 1000KBPS | 1000KBPS | CAN bus bitrate |

### GPS via DroneCAN

To use a DroneCAN GPS receiver:

```
set gps_provider = DRONECAN
save
```

The flight controller will automatically receive GPS data from any DroneCAN GNSS device on the bus.

### Battery Monitoring via DroneCAN

DroneCAN battery monitors send both voltage and current data using the `BatteryInfo` message type.

**Voltage sensing:**
```
set bat_voltage_src = CAN
save
```

**Current sensing:**
```
feature CURRENT_METER
set current_meter_type = CAN
save
```

Both voltage and current come from the same DroneCAN BatteryInfo message, so they update together when using a DroneCAN battery monitor.

## Hardware Setup

### Wiring

Connect the CAN bus lines between your flight controller and DroneCAN peripherals:

| Signal | Description |
|--------|-------------|
| CAN_H | CAN High (dominant high) |
| CAN_L | CAN Low (dominant low) |
| GND | Ground reference |

**Important:** CAN bus requires 120 ohm termination resistors at each end of the bus. Many DroneCAN devices include built-in termination that can be enabled via jumper or configuration.

### Typical Setup

```
[FC CAN Port] ----CAN_H/CAN_L---- [DroneCAN GPS] ----CAN_H/CAN_L---- [DroneCAN Battery Monitor]
     |                                  |                                      |
    120R                          (pass-through)                              120R
```

## Troubleshooting

### No data from DroneCAN devices

1. **Check wiring** - Ensure CAN_H and CAN_L are not swapped
2. **Check termination** - Bus requires 120 ohm termination at both ends
3. **Check bitrate** - All devices must use the same bitrate
4. **Check node IDs** - Each device must have a unique node ID

### GPS not detected

1. Verify `gps_provider = DRONECAN` is set
2. Check that the GPS is broadcasting Fix2 or Fix messages
3. Some GPS modules need external power - check power supply

### Battery readings incorrect

1. Verify the DroneCAN battery monitor is configured correctly
2. Check that `bat_voltage_src = CAN` and/or `current_meter_type = CAN` are set
3. Some battery monitors need calibration - consult device documentation

## DroneCAN Message Types

INAV processes the following DroneCAN message types:

| Message Type | Data Type ID | Description |
|--------------|--------------|-------------|
| Fix2 | 1063 | GNSS fix data (preferred) |
| Fix | 1060 | GNSS fix data (legacy) |
| Auxiliary | 1061 | GNSS auxiliary data (HDOP, VDOP) |
| BatteryInfo | 1092 | Battery voltage, current, temperature, SoC |
| NodeStatus | 341 | Node health and status |

## Compatibility

DroneCAN devices compatible with INAV include:

- **GPS:** ARK GPS, Holybro DroneCAN GPS, mRo Location One
- **Battery Monitors:** Various DroneCAN-compatible power modules

For the latest compatibility information, check the INAV Discord or GitHub discussions.

## Resources

- [DroneCAN Specification](https://dronecan.github.io/)
- [libcanard Library](https://github.com/dronecan/libcanard)
- [INAV Discord](https://discord.gg/peg2hhbYwN)
