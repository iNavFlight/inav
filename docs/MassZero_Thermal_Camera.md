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

### Serial Connection (UART Control)
- **TX (FC)** → **RX (Camera)**
- **RX (FC)** → **TX (Camera)**
- **GND** → **GND**
- **3.3V** → **VCC** (check camera power requirements)

### Analog Video Connection (Video Output)
- **Video Out (Camera)** → **Video In (FC)**
- **GND** → **GND** (shared ground with UART)

### Recommended Serial Ports
- **USART2** (default)
- **USART3**
- **USART6**

### Video Input Pins
The thermal camera's analog video output must be connected to the flight controller's video input pins. Common video input configurations:

#### For F4 Flight Controllers
- **Video In**: PA8 (TIM1_CH1) - Primary video input
- **Video In**: PA9 (TIM1_CH2) - Secondary video input
- **Video In**: PA10 (TIM1_CH3) - Tertiary video input

#### For F7 Flight Controllers
- **Video In**: PA8 (TIM1_CH1) - Primary video input
- **Video In**: PA9 (TIM1_CH2) - Secondary video input
- **Video In**: PA10 (TIM1_CH3) - Tertiary video input
- **Video In**: PC6 (TIM3_CH1) - Additional video input

#### For H7 Flight Controllers
- **Video In**: PA8 (TIM1_CH1) - Primary video input
- **Video In**: PA9 (TIM1_CH2) - Secondary video input
- **Video In**: PA10 (TIM1_CH3) - Tertiary video input
- **Video In**: PC6 (TIM3_CH1) - Additional video input

### Complete Wiring Example
```
MassZero Thermal Camera    Flight Controller
=======================    =================
VCC (3.3V)              →  3.3V
GND                     →  GND
TX                      →  RX (USART2)
RX                      →  TX (USART2)
Video Out               →  Video In (PA8)
GND (Video)             →  GND
```

## Quick Start with Application Presets

The easiest way to get started is to use one of the pre-configured application presets:

### 🔥 Fire Detection
```bash
mztc preset fire_detection
```
Optimized for wildfire monitoring and hot spot identification.

### 🚁 Search and Rescue
```bash
mztc preset search_rescue
```
Optimized for human detection and body heat signatures.

### 👁️ Surveillance
```bash
mztc preset surveillance
```
Optimized for long-range monitoring and perimeter security.

### ⚡ Rapid Environment Changes
```bash
mztc preset rapid_changes
```
Optimized for weather changes and dynamic environments.

### 🏭 Industrial Inspection
```bash
mztc preset industrial
```
Optimized for equipment monitoring and thermal analysis.

## Application-Specific Presets

### 🔥 Fire Detection Mode
**Use Case**: Wildfire monitoring, building fire detection, hot spot identification

```bash
mztc preset fire_detection
```

**Configuration Applied**:
- **Mode**: ALERT (only captures when alerts triggered)
- **Palette**: WHITE_HOT (maximum heat visibility)
- **Brightness**: 70% (enhanced visibility)
- **Contrast**: 80% (high contrast for hot spots)
- **Digital Enhancement**: 75% (enhanced detail)
- **Spatial Denoising**: 30% (preserve sharp edges)
- **Temporal Denoising**: 40% (reduce noise while maintaining detail)
- **Auto Shutter**: TIME_AND_TEMP (frequent calibration)
- **FFC Interval**: 3 minutes (frequent calibration for accuracy)
- **Temperature Alerts**: ON
- **Alert High Temp**: 60°C (fire detection threshold)
- **Alert Low Temp**: -10°C (environmental baseline)
- **Update Rate**: 15 Hz (fast response)

**Key Features**: High contrast, frequent calibration, temperature alerts, white-hot palette for maximum heat visibility

### 🚁 Search and Rescue Mode
**Use Case**: Human detection, body heat signatures, missing person searches

```bash
mztc preset search_rescue
```

**Configuration Applied**:
- **Mode**: CONTINUOUS (continuous monitoring)
- **Palette**: FUSION_1 (better contrast for human detection)
- **Brightness**: 60% (balanced visibility)
- **Contrast**: 70% (good contrast for body heat)
- **Digital Enhancement**: 60% (enhanced detail)
- **Spatial Denoising**: 50% (balanced noise reduction)
- **Temporal Denoising**: 60% (smooth video)
- **Auto Shutter**: TIME_AND_TEMP (balanced calibration)
- **FFC Interval**: 5 minutes (regular calibration)
- **Temperature Alerts**: ON
- **Alert High Temp**: 45°C (human body temperature)
- **Alert Low Temp**: 15°C (environmental baseline)
- **Update Rate**: 9 Hz (good balance)
- **Zoom Level**: 2X (enhanced detail for human detection)

**Key Features**: Balanced settings, human body temperature range, 2X zoom for detail, fusion palette for better contrast

### 👁️ Surveillance Mode
**Use Case**: Long-range monitoring, perimeter security, wildlife observation

```bash
mztc preset surveillance
```

**Configuration Applied**:
- **Mode**: SURVEILLANCE (optimized for long-range)
- **Palette**: BLACK_HOT (stealth and contrast)
- **Brightness**: 50% (standard visibility)
- **Contrast**: 60% (good contrast)
- **Digital Enhancement**: 50% (standard enhancement)
- **Spatial Denoising**: 60% (good noise reduction)
- **Temporal Denoising**: 70% (smooth video)
- **Auto Shutter**: TIME_ONLY (less frequent calibration)
- **FFC Interval**: 10 minutes (battery conservation)
- **Temperature Alerts**: ON
- **Alert High Temp**: 50°C (intrusion detection)
- **Alert Low Temp**: 0°C (environmental baseline)
- **Update Rate**: 5 Hz (battery conservation)
- **Zoom Level**: 4X (long-range monitoring)

**Key Features**: High zoom, low update rate for battery life, black-hot for stealth, longer calibration intervals

### ⚡ Rapid Environment Changes Mode
**Use Case**: Weather changes, altitude changes, temperature fluctuations, dynamic environments

```bash
mztc preset rapid_changes
```

**Configuration Applied**:
- **Mode**: CONTINUOUS (continuous monitoring)
- **Palette**: RAINBOW (maximum contrast and detail)
- **Brightness**: 55% (balanced visibility)
- **Contrast**: 65% (good contrast)
- **Digital Enhancement**: 70% (enhanced detail for changing conditions)
- **Spatial Denoising**: 40% (preserve detail)
- **Temporal Denoising**: 30% (maintain responsiveness)
- **Auto Shutter**: TEMP_ONLY (temperature-based calibration)
- **FFC Interval**: 2 minutes (very frequent calibration)
- **Temperature Alerts**: ON
- **Alert High Temp**: 80°C (wide temperature range)
- **Alert Low Temp**: -20°C (wide temperature range)
- **Update Rate**: 20 Hz (high responsiveness)
- **Bad Pixel Removal**: ON (maintain image quality)
- **Vignetting Correction**: ON (compensate for environmental changes)

**Key Features**: Very frequent calibration, temperature-based shutter, high update rate, rainbow palette for maximum contrast, bad pixel removal enabled

### 🏭 Industrial Inspection Mode
**Use Case**: Equipment monitoring, thermal analysis, machinery inspection

```bash
mztc preset industrial
```

**Configuration Applied**:
- **Mode**: CONTINUOUS (continuous monitoring)
- **Palette**: IRON_RED_1 (heat visualization)
- **Brightness**: 60% (good visibility)
- **Contrast**: 75% (high contrast for equipment)
- **Digital Enhancement**: 65% (enhanced detail)
- **Spatial Denoising**: 45% (balanced noise reduction)
- **Temporal Denoising**: 55% (smooth video)
- **Auto Shutter**: TIME_AND_TEMP (balanced calibration)
- **FFC Interval**: 7 minutes (regular calibration)
- **Temperature Alerts**: ON
- **Alert High Temp**: 100°C (equipment overheating)
- **Alert Low Temp**: -30°C (wide temperature range)
- **Update Rate**: 12 Hz (good balance)
- **Zoom Level**: 1X (standard view)

**Key Features**: Wide temperature range, iron-red palette for heat visualization, balanced settings for equipment monitoring

## Temperature Range Guidelines

| Application | Min Temp | Max Temp | Palette | Update Rate | FFC Interval | Zoom |
|-------------|----------|----------|---------|-------------|--------------|------|
| Fire Detection | -10°C | 60°C+ | White Hot | 15 Hz | 3 min | 1X |
| Search & Rescue | 15°C | 45°C | Fusion 1 | 9 Hz | 5 min | 2X |
| Surveillance | 0°C | 50°C | Black Hot | 5 Hz | 10 min | 4X |
| Rapid Changes | -20°C | 80°C | Rainbow | 20 Hz | 2 min | 1X |
| Industrial | -30°C | 100°C | Iron Red | 12 Hz | 7 min | 1X |

## Customizing Presets

After applying a preset, you can fine-tune individual parameters:

```bash
# Apply a preset
mztc preset fire_detection

# Fine-tune specific parameters
set mztc_brightness = 75
set mztc_contrast = 85
set mztc_alert_high_temp = 65.0

# Save the customized configuration
mztc save_config
```

## Preset Selection Guide

### Choose Fire Detection When:
- Monitoring for wildfires or building fires
- Detecting hot spots or overheating equipment
- Need maximum sensitivity to heat sources
- Working in outdoor environments

### Choose Search and Rescue When:
- Looking for people in search and rescue operations
- Detecting human body heat signatures
- Need good detail and contrast for human detection
- Working in moderate temperature environments

### Choose Surveillance When:
- Long-range monitoring or perimeter security
- Need to conserve battery power
- Working in stealth mode (black-hot palette)
- Monitoring large areas with minimal detail requirements

### Choose Rapid Changes When:
- Weather conditions are changing rapidly
- Altitude changes during flight
- Temperature fluctuations in the environment
- Need maximum responsiveness and detail

### Choose Industrial When:
- Monitoring equipment or machinery
- Thermal analysis of industrial processes
- Need wide temperature range monitoring
- Working in controlled industrial environments

## Troubleshooting Presets

### If a preset doesn't work well:
1. Check your environment conditions
2. Adjust brightness and contrast manually
3. Try a different palette mode
4. Modify the temperature alert thresholds
5. Adjust the FFC interval based on stability

### For extreme environments:
- **Very cold**: Use Industrial preset with lower alert thresholds
- **Very hot**: Use Fire Detection preset with higher alert thresholds
- **High altitude**: Use Rapid Changes preset for frequent calibration
- **Long missions**: Use Surveillance preset for battery conservation

## Advanced Usage

### Creating Custom Presets
While the CLI doesn't support custom preset creation, you can create scripts or use the configuration system to quickly apply your preferred settings:

```bash
# Create a custom configuration script
# Save your preferred settings to a file
# Apply them when needed
```

### Preset Combinations
You can mix and match features from different presets:

```bash
# Start with a base preset
mztc preset search_rescue

# Modify specific features
set mztc_palette_mode = RAINBOW  # Use rainbow instead of fusion
set mztc_zoom_level = 4X         # Use higher zoom
set mztc_update_rate = 20        # Use higher update rate
```

This gives you the flexibility to start with a proven configuration and customize it for your specific needs.

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

#### Video Configuration
```
# Enable video input
set video_input = ON

# Set video input source (check your FC's video input pins)
set video_input_pin = PA8

# Set video input mode
set video_input_mode = PAL  # or NTSC

# Enable thermal camera video overlay
set video_thermal_overlay = ON

# Set thermal video transparency (0-100)
set video_thermal_alpha = 50
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

#### Video Issues
1. Check video input pin configuration
2. Verify video input mode (PAL/NTSC)
3. Check video input signal strength
4. Ensure proper grounding of video signal
5. Verify video input is enabled in configuration
6. Check for video input conflicts with other cameras

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
- **Video Output**: Analog composite video (PAL/NTSC)
- **Video Resolution**: 640x480 (typical)
- **Video Frame Rate**: 25/30 fps (PAL/NTSC)

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

### Version 1.1.0
- **Application Presets**: Added complete preset system with 5 optimized configurations
- **Enhanced Documentation**: Added detailed preset descriptions, troubleshooting guides, and selection criteria
- **Temperature Guidelines**: Added comprehensive temperature range table for all applications
- **Advanced Usage**: Added custom preset creation and combination guidance

### Version 1.0.0
- Initial MassZero Thermal Camera integration
- Basic UART protocol implementation
- MSP V2 command support
- OSD integration
- CLI configuration support
- SITL simulation support
- Camera management functions (init status, save config, restore defaults)
- Comprehensive test coverage
