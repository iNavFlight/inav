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
| Dynamic Node Assignment | Planned | Manage node IDs dynamically to minimize first time configuration |

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

All peripherals need to have the node ID and the bitrate set manually through the dronecan_gui for now.  You can use your flight controller as a CAN interface by loading an Ardupilot image on it.  Once the set up is complete, you can reflash it to Inav.

### GPS via DroneCAN

To use a DroneCAN GPS receiver:

```
set gps_provider = DRONECAN
save
```

The flight controller will automatically receive GPS data from any DroneCAN GNSS device on the bus.  GPS configuration needs to be set using dronecan_gui to enable additional satellite networks.

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

## Configuration Examples

### Example 1: GPS Only Setup

Use a DroneCAN GPS without battery monitoring:

```
# Enable DroneCAN interface
set dronecan_node_id = 10
set dronecan_bitrate = 1000KBPS

# Configure GPS
set gps_provider = DRONECAN

# Disable battery monitoring from DroneCAN
set bat_voltage_src = ADC
set current_meter_type = NONE

save
```

**Result:** Flight controller receives GPS data from DroneCAN GPS receiver on node ID 1-127 (auto-detected).

---

### Example 2: Battery Monitoring Only

Use DroneCAN for battery voltage and current without GPS:

```
# Enable DroneCAN interface
set dronecan_node_id = 10
set dronecan_bitrate = 1000KBPS

# Configure battery monitoring
set bat_voltage_src = CAN
feature CURRENT_METER
set current_meter_type = CAN

# Keep GPS on other source
set gps_provider = UBLOX

save
```

**Result:** Flight controller receives battery voltage and current from DroneCAN BatteryInfo messages.

---

### Example 3: GPS + Battery Combined

Use DroneCAN for both GPS and battery monitoring:

```
# Enable DroneCAN interface
set dronecan_node_id = 10
set dronecan_bitrate = 1000KBPS

# Configure GPS
set gps_provider = DRONECAN

# Configure battery monitoring
set bat_voltage_src = CAN
feature CURRENT_METER
set current_meter_type = CAN

save
```

**Result:** Single CAN bus provides both GPS and battery data. Simplifies hardware setup significantly.

**Note:** The GPS and battery monitor must be configured as separate DroneCAN nodes with different node IDs (e.g., GPS on node 1, battery monitor on node 2).

---

### Example 4: Multi-Node DroneCAN Network

Setting up multiple DroneCAN peripherals on a single CAN bus:

```
# Flight Controller Configuration
set dronecan_node_id = 10           # Flight controller = node 10
set dronecan_bitrate = 1000KBPS

# Configure GPS (from node 1)
set gps_provider = DRONECAN

# Configure battery monitor (from node 2)
set bat_voltage_src = CAN
feature CURRENT_METER
set current_meter_type = CAN

save
```

**Peripheral Configuration (using dronecan_gui or similar tool):**
- **GPS Receiver:** Node ID = 1, Bitrate = 1000 KBPS
- **Battery Monitor:** Node ID = 2, Bitrate = 1000 KBPS
- **Potential Future Peripheral:** Node ID = 3, etc.

**CAN Bus Layout:**
```
   [FC: Node 10]
        |
        +-- CAN_H/CAN_L
        |
   [GPS: Node 1] ----+---- [Battery: Node 2]
        |                       |
       120R                     120R
   (termination)           (termination)
```

**Important:** Each node must have a unique ID (1-127). Don't assign two devices the same node ID.

---

### Example 5: SITL Simulation

Testing DroneCAN configuration without hardware:

```bash
# Build and run SITL
make SITL_TARGET=F405_OHMINIV2

# In SITL console, configure DroneCAN:
set dronecan_node_id = 10
set dronecan_bitrate = 1000KBPS
set gps_provider = DRONECAN
set bat_voltage_src = CAN
save
```

**Note:** SITL includes a virtual CAN bus for testing. To send simulated DroneCAN messages:

1. Connect SITL to external DroneCAN simulator or
2. Use `candump` and `cansend` tools to inject test messages
3. Monitor responses with `candump can0`

---

### Example 6: Hardware-Specific Setup for MATEKH743

The MATEKH743 has integrated FDCAN peripheral:

```
# MATEKH743 with DroneCAN
set dronecan_node_id = 10
set dronecan_bitrate = 1000KBPS

# GPS configuration
set gps_provider = DRONECAN

# Battery monitoring (if using DroneCAN BMS)
set bat_voltage_src = CAN
feature CURRENT_METER
set current_meter_type = CAN

# Save and reboot
save
```

**Wiring for MATEKH743:**
- Use the **CAN 1** port on the board
- Connect to DroneCAN GPS on CAN_H/CAN_L
- Connect to DroneCAN battery monitor on same CAN bus
- Add 120Ω termination resistors at both ends

---

### Example 7: Hardware-Specific Setup for MATEKF765SE

The MATEKF765SE has STM32F765 with bxCAN:

```
# MATEKF765SE with DroneCAN
set dronecan_node_id = 10
set dronecan_bitrate = 1000KBPS

# Note: F7 boards may have bitrate limitations
# If you experience issues, try 500KBPS
# set dronecan_bitrate = 500KBPS

# GPS configuration
set gps_provider = DRONECAN

# Battery monitoring
set bat_voltage_src = CAN
feature CURRENT_METER
set current_meter_type = CAN

save
```

**Wiring for MATEKF765SE:**
- Use the **CAN 1** port on the board
- F7 boards support both 500 KBPS and 1000 KBPS
- If you have range issues, try lower bitrate

---

### Configuration Verification

After setting up DroneCAN, verify configuration:

```
# Check DroneCAN settings
set dronecan_node_id
set dronecan_bitrate
set gps_provider
set bat_voltage_src
set current_meter_type
```

**Expected Output for GPS + Battery:**
```
dronecan_node_id = 10
dronecan_bitrate = 1000KBPS
gps_provider = DRONECAN
bat_voltage_src = CAN
current_meter_type = CAN
```

## Hardware Setup

### Wiring

Connect the CAN bus lines between your flight controller and DroneCAN peripherals:

| Signal | Description |
|--------|-------------|
| 4v5 | Power supply for peripheral (see dronecan specification for limitations) |
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

- **GPS:** (not tested) ARK GPS, Holybro DroneCAN GPS, mRo Location One
- **Battery Monitors:** Matek CAN-L4-BM, Various DroneCAN-compatible power modules

For the latest compatibility information, check the INAV Discord or GitHub discussions.

## Resources

- [DroneCAN Specification](https://dronecan.github.io/)
- [libcanard Library](https://github.com/dronecan/libcanard)
- [INAV Discord](https://discord.gg/peg2hhbYwN)
