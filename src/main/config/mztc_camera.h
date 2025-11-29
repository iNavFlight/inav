/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR ANY PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef USE_MZTC

#include "config/parameter_group.h"
#include "drivers/time.h"

// MassZero Thermal Camera operating modes
typedef enum {
    MZTC_MODE_DISABLED = 0,
    MZTC_MODE_STANDBY,                 // Low power, periodic updates
    MZTC_MODE_CONTINUOUS,              // Continuous frame capture
    MZTC_MODE_TRIGGERED,               // Capture on demand
    MZTC_MODE_ALERT,                   // Only capture when alerts triggered
    MZTC_MODE_RECORDING,               // High-speed recording mode
    MZTC_MODE_CALIBRATION,             // Calibration mode
    MZTC_MODE_SURVEILLANCE             // Long-range surveillance mode
} mztcMode_e;

// MassZero Thermal Camera temperature units
typedef enum {
    MZTC_UNIT_CELSIUS = 0,
    MZTC_UNIT_FAHRENHEIT = 1,
    MZTC_UNIT_KELVIN = 2
} mztcTemperatureUnit_e;

// MassZero Thermal Camera color palettes
typedef enum {
    MZTC_PALETTE_WHITE_HOT = 0,        // Default white hot
    MZTC_PALETTE_BLACK_HOT = 1,        // Black hot
    MZTC_PALETTE_FUSION_1 = 2,         // Fusion 1
    MZTC_PALETTE_RAINBOW = 3,          // Rainbow
    MZTC_PALETTE_FUSION_2 = 4,         // Fusion 2
    MZTC_PALETTE_IRON_RED_1 = 5,       // Iron red 1
    MZTC_PALETTE_IRON_RED_2 = 6,       // Iron red 2
    MZTC_PALETTE_SEPIA = 7,            // Sepia
    MZTC_PALETTE_COLOR_1 = 8,          // Color 1
    MZTC_PALETTE_COLOR_2 = 9,          // Color 2
    MZTC_PALETTE_ICE_FIRE = 10,        // Ice fire
    MZTC_PALETTE_RAIN = 11,            // Rain
    MZTC_PALETTE_GREEN_HOT = 12,       // Green hot
    MZTC_PALETTE_RED_HOT = 13          // Red hot
} mztcPaletteMode_e;

// MassZero Thermal Camera zoom levels
typedef enum {
    MZTC_ZOOM_1X = 0,                  // 1x zoom (default)
    MZTC_ZOOM_2X = 1,                  // 2x zoom
    MZTC_ZOOM_4X = 2,                  // 4x zoom (per thermal camera docs)
    MZTC_ZOOM_8X = 3                   // 8x zoom (per thermal camera docs)
} mztcZoomLevel_e;

// MassZero Thermal Camera mirror modes
typedef enum {
    MZTC_MIRROR_NONE = 0,              // No mirroring (default)
    MZTC_MIRROR_HORIZONTAL = 1,        // Horizontal mirror
    MZTC_MIRROR_VERTICAL = 2,          // Vertical mirror
    MZTC_MIRROR_CENTRAL = 3            // Both horizontal and vertical
} mztcMirrorMode_e;

// MassZero Thermal Camera auto shutter modes
typedef enum {
    MZTC_SHUTTER_TEMP_ONLY = 0,        // Based on temperature difference only
    MZTC_SHUTTER_TIME_ONLY = 1,        // Based on time interval only
    MZTC_SHUTTER_TIME_AND_TEMP = 2     // Both time and temperature (default)
} mztcShutterMode_e;

// MassZero Thermal Camera configuration structure
typedef struct mztcConfig_s {
    uint8_t enabled;                    // Enable/disable MassZero Thermal Camera
    uint8_t port;                       // Serial port (USART1-8, etc.)
    uint8_t baudrate;                   // Baud rate index
    uint8_t mode;                       // Operating mode
    uint8_t update_rate;                // Frame update rate (Hz)
    uint8_t temperature_unit;           // Temperature unit
    uint8_t palette_mode;               // Color palette
    uint8_t auto_shutter;               // Auto shutter mode
    uint8_t digital_enhancement;        // Digital enhancement (0-100)
    uint8_t spatial_denoise;            // Spatial denoising (0-100)
    uint8_t temporal_denoise;           // Temporal denoising (0-100)
    uint8_t brightness;                 // Brightness (0-100)
    uint8_t contrast;                   // Contrast (0-100)
    uint8_t zoom_level;                 // Digital zoom level
    uint8_t mirror_mode;                // Image mirroring
    uint8_t crosshair_enabled;          // Enable crosshair overlay
    uint8_t temperature_alerts;         // Enable temperature alerts
    float alert_high_temp;              // High temperature alert threshold
    float alert_low_temp;               // Low temperature alert threshold
    uint8_t ffc_interval;               // Flat Field Calibration interval (minutes)
    uint8_t bad_pixel_removal;          // Enable bad pixel removal
    uint8_t vignetting_correction;      // Enable vignetting correction
    // Channel control configuration
    uint8_t zoom_channel;               // 0 = disabled, 1-18 = AUX1-18
    uint8_t palette_channel;            // 0 = disabled, 1-18 = AUX1-18
    uint8_t ffc_channel;                // 0 = disabled, 1-18 = AUX1-18
    uint8_t brightness_channel;         // 0 = disabled, 1-18 = AUX1-18
    uint8_t contrast_channel;           // 0 = disabled, 1-18 = AUX1-18
} mztcConfig_t;

// MassZero Thermal Camera status structure
typedef struct mztcStatus_s {
    uint8_t status;                     // Camera status
    uint8_t mode;                       // Current mode
    bool connected;                      // Connection status
    uint8_t connection_quality;         // Connection quality indicator
    uint8_t last_calibration;           // Minutes since last calibration
    float camera_temperature;           // Camera internal temperature
    float ambient_temperature;          // Ambient temperature
    uint32_t frame_count;               // Frame counter
    uint8_t error_flags;                // Error status flags
    uint32_t last_frame_time;           // Last frame timestamp
} mztcStatus_t;

// MassZero Thermal Camera frame data structure
typedef struct mztcFrameData_s {
    uint16_t width;                     // Frame width
    uint16_t height;                    // Frame height
    float min_temp;                     // Minimum temperature in frame
    float max_temp;                     // Maximum temperature in frame
    float center_temp;                  // Center point temperature
    float hottest_temp;                 // Hottest point temperature
    float coldest_temp;                 // Coldest point temperature
    uint16_t hottest_x;                // Hottest point X coordinate
    uint16_t hottest_y;                // Hottest point Y coordinate
    uint16_t coldest_x;                // Coldest point X coordinate
    uint16_t coldest_y;                // Coldest point Y coordinate
    uint8_t data[256];                 // Raw thermal data (reduced size for MSP)
} mztcFrameData_t;

// MassZero Thermal Camera status values
#define MZTC_STATUS_OFFLINE             0x00
#define MZTC_STATUS_INITIALIZING        0x01
#define MZTC_STATUS_READY               0x02
#define MZTC_STATUS_CAPTURING           0x03
#define MZTC_STATUS_CALIBRATING         0x04
#define MZTC_STATUS_ERROR               0x05
#define MZTC_STATUS_ALERT               0x06
#define MZTC_STATUS_RECORDING           0x07

// MassZero Thermal Camera error flags
#define MZTC_ERROR_COMMUNICATION        0x01
#define MZTC_ERROR_CALIBRATION          0x02
#define MZTC_ERROR_TEMPERATURE          0x04
#define MZTC_ERROR_MEMORY               0x08
#define MZTC_ERROR_TIMEOUT              0x10
#define MZTC_ERROR_INVALID_CONFIG       0x20

// MassZero Thermal Camera limits
#define MZTC_MAX_UPDATE_RATE            30      // Maximum 30 Hz
#define MZTC_MIN_UPDATE_RATE            1       // Minimum 1 Hz
#define MZTC_MAX_FFC_INTERVAL           60      // Maximum 60 minutes
#define MZTC_MIN_FFC_INTERVAL           1       // Minimum 1 minute

// Parameter group declaration
PG_DECLARE(mztcConfig_t, mztcConfig);

// Function declarations
void mztcInit(void);
void mztcUpdate(timeUs_t currentTimeUs);
bool mztcIsEnabled(void);
mztcStatus_t* mztcGetStatus(void);
bool mztcTriggerCalibration(void);
bool mztcSetMode(mztcMode_e mode);
bool mztcSetPalette(mztcPaletteMode_e palette);
bool mztcSetZoom(mztcZoomLevel_e zoom);
bool mztcSetImageParams(uint8_t brightness, uint8_t contrast, uint8_t enhancement);
bool mztcSetDenoising(uint8_t spatial, uint8_t temporal);
bool mztcSetTemperatureAlerts(bool enabled, float high_temp, float low_temp);
bool mztcIsConnected(void);
void mztcSimulateDataReception(void);

#endif // USE_MZTC
