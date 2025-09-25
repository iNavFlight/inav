# MassZero Thermal Camera Integration

## Overview

The MassZero Thermal Camera (MZTC) integration provides real-time thermal imaging capabilities for INAV flight controllers. This integration allows pilots to see thermal data overlaid on their OSD, enabling applications such as search and rescue, wildlife monitoring, and industrial inspection.

## Purchase Information

**MassZero Thermal Camera**  
Website: [https://masszerofpv.com](https://masszerofpv.com)  
Model: MassZero Thermal Camera Core Component  
Price: Contact MassZero for current pricing and availability

## Hardware Requirements

- INAV-compatible flight controller
- MassZero Thermal Camera Core Component
- Serial port (UART) connection
- 3.3V power supply (check camera specifications)

## Wiring

### Serial Connection
- **TX (FC)** → **RX (Camera)**
- **RX (FC)** → **TX (Camera)**
- **GND** → **GND**
- **3.3V** → **VCC** (check camera power requirements)

### Recommended Serial Ports
- **USART2** (default)
- **USART3**
- **USART6**

## Configuration

### CLI Commands

#### Basic Configuration
```
# Enable MassZero Thermal Camera
set mztc_enabled = ON

# Set serial port (USART2, USART3, USART6, etc.)
set mztc_port = USART2

# Set baud rate (115200 is standard)
set mztc_baudrate = 115200

# Set operating mode
set mztc_mode = CONTINUOUS
```

#### Operating Modes
- `DISABLED` - Camera disabled
- `STANDBY` - Low power, periodic updates
- `CONTINUOUS` - Continuous frame capture
- `TRIGGERED` - Capture on demand
- `ALERT` - Only capture when alerts triggered
- `RECORDING` - High-speed recording mode
- `CALIBRATION` - Calibration mode
- `SURVEILLANCE` - Long-range surveillance mode

#### Image Parameters
```
# Set brightness (0-100, default 50)
set mztc_brightness = 50

# Set contrast (0-100, default 50)
set mztc_contrast = 50

# Set digital enhancement (0-100, default 50)
set mztc_digital_enhancement = 50

# Set spatial denoising (0-100, default 50)
set mztc_spatial_denoise = 50

# Set temporal denoising (0-100, default 50)
set mztc_temporal_denoise = 50
```

#### Color Palettes
```
# Set color palette
set mztc_palette_mode = WHITE_HOT
```

Available palettes:
- `WHITE_HOT` - White hot (default)
- `BLACK_HOT` - Black hot
- `FUSION_1` - Fusion 1
- `RAINBOW` - Rainbow
- `FUSION_2` - Fusion 2
- `IRON_RED_1` - Iron red 1
- `IRON_RED_2` - Iron red 2
- `SEPIA` - Sepia
- `COLOR_1` - Color 1
- `COLOR_2` - Color 2
- `ICE_FIRE` - Ice fire
- `RAIN` - Rain
- `GREEN_HOT` - Green hot
- `RED_HOT` - Red hot

#### Zoom and Mirror
```
# Set zoom level (1X, 2X, 4X, 8X)
set mztc_zoom_level = 1X

# Set mirror mode (NONE, HORIZONTAL, VERTICAL, CENTRAL)
set mztc_mirror_mode = NONE
```

#### Auto Shutter
```
# Set auto shutter mode
set mztc_auto_shutter = TIME_AND_TEMP
```

Available modes:
- `TEMP_ONLY` - Based on temperature difference only
- `TIME_ONLY` - Based on time interval only
- `TIME_AND_TEMP` - Both time and temperature (default)

#### Temperature Alerts
```
# Enable temperature alerts
set mztc_temperature_alerts = ON

# Set high temperature alert (Celsius)
set mztc_alert_high_temp = 80.0

# Set low temperature alert (Celsius)
set mztc_alert_low_temp = -10.0
```

#### Calibration
```
# Set FFC interval (minutes)
set mztc_ffc_interval = 5

# Enable bad pixel removal
set mztc_bad_pixel_removal = ON

# Enable vignetting correction
set mztc_vignetting_correction = ON
```

#### Channel Control
```
# Set RC channels for camera control
set mztc_zoom_channel = AUX1
set mztc_palette_channel = AUX2
set mztc_ffc_channel = AUX3
set mztc_brightness_channel = AUX4
set mztc_contrast_channel = AUX5
```

#### Camera Management
```
# Get camera initialization status
mztc init_status

# Save configuration to camera flash
mztc save_config

# Restore camera to factory defaults
mztc restore_defaults

# Trigger manual calibration (FFC)
mztc calibrate

# Reconnect to camera
mztc reconnect
```

### MSP Commands

The thermal camera supports MSP V2 commands for remote control:

- `MSP2_MZTC_CONFIG` (0x3000) - Get configuration
- `MSP2_SET_MZTC_CONFIG` (0x3001) - Set configuration
- `MSP2_MZTC_STATUS` (0x3002) - Get status
- `MSP2_MZTC_FRAME_DATA` (0x3003) - Get thermal frame data
- `MSP2_MZTC_CALIBRATE` (0x3004) - Trigger calibration
- `MSP2_MZTC_MODE` (0x3005) - Set operating mode
- `MSP2_MZTC_PALETTE` (0x3006) - Set color palette
- `MSP2_MZTC_ZOOM` (0x3007) - Set zoom level
- `MSP2_MZTC_SHUTTER` (0x3008) - Trigger manual shutter

## OSD Integration

### Thermal Data Display
The thermal camera data can be displayed on the OSD using the following elements:

- **Thermal Temperature** - Current temperature reading
- **Thermal Min/Max** - Temperature range in frame
- **Thermal Center** - Center point temperature
- **Thermal Hot Spot** - Hottest point coordinates and temperature
- **Thermal Cold Spot** - Coldest point coordinates and temperature

### OSD Configuration
```
# Enable thermal camera OSD elements
set osd_thermal_temp = ON
set osd_thermal_minmax = ON
set osd_thermal_center = ON
set osd_thermal_hotspot = ON
set osd_thermal_coldspot = ON
```

## Troubleshooting

### Common Issues

#### Camera Not Connecting
1. Check wiring connections
2. Verify serial port configuration
3. Check baud rate (should be 115200)
4. Ensure camera is powered on
5. Check for loose connections

#### No Thermal Data
1. Verify camera is in correct mode
2. Check if calibration is needed
3. Verify frame data is being received
4. Check error flags in status

#### Poor Image Quality
1. Adjust brightness and contrast
2. Modify denoising settings
3. Check if calibration is needed
4. Verify camera temperature

#### Calibration Issues
1. Ensure camera is at stable temperature
2. Point camera at uniform surface
3. Wait for calibration to complete
4. Check calibration interval settings

### Debug Information

#### Status Monitoring
```
# Check camera status
status

# Check thermal camera specific status
mztc status
```

#### Error Codes
- `COMMUNICATION` - Serial communication error
- `CALIBRATION` - Calibration failed
- `TEMPERATURE` - Temperature sensor error
- `MEMORY` - Memory allocation error
- `TIMEOUT` - Command timeout
- `INVALID_CONFIG` - Invalid configuration

## Technical Specifications

### Camera Specifications
- **Resolution**: 160x120 pixels (typical)
- **Temperature Range**: -20°C to +100°C
- **Accuracy**: ±2°C or ±2%
- **Update Rate**: 1-30 Hz (configurable)
- **Communication**: UART 115200 bps
- **Power**: 3.3V (check specifications)

### Protocol Details
- **Packet Format**: Custom UART protocol
- **Device Address**: 0x36
- **Checksum**: Sum of address + command + data
- **Error Handling**: Comprehensive error reporting

## Safety Considerations

### Important Safety Notes
1. **DO NOT** aim the detector at strong light sources (sun, fire, lasers)
2. **DO NOT** touch the detector directly - use anti-static gloves
3. **Avoid** corrosive gas environments
4. **Ensure** proper ventilation - camera generates heat
5. **Use** original packaging for transport/storage

### Operational Considerations
1. **Boot Time**: Camera needs ~2-3 seconds to initialize
2. **FFC Timing**: Default auto-shutter every 5 minutes
3. **Bad Pixels**: May appear over time - use bad pixel removal
4. **Temperature Drift**: Perform background correction after environment changes
5. **Power Consumption**: Camera may draw significant current during FFC

## Support

For technical support and questions:
- **MassZero Website**: [https://masszerofpv.com](https://masszerofpv.com)
- **INAV Discord**: [https://discord.gg/peg2hhbYwN](https://discord.gg/peg2hhbYwN)
- **INAV GitHub**: [https://github.com/iNavFlight/inav](https://github.com/iNavFlight/inav)

## Changelog

### Version 1.0.0
- Initial MassZero Thermal Camera integration
- Basic UART protocol implementation
- MSP V2 command support
- OSD integration
- CLI configuration support
- SITL simulation support
- Camera management functions (init status, save config, restore defaults)
- Comprehensive test coverage
