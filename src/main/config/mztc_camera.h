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

// Purpose presets. A preset is a named bundle of the image settings below. It
// is applied on the flight controller and writes ordinary camera commands. The
// camera has no preset mechanism of its own.
//
// MZTC_PRESET_CUSTOM writes nothing, so a hand-tuned configuration survives.
// Changing any setting a preset owns switches the selection back to it.
typedef enum {
    MZTC_PRESET_CUSTOM = 0,            // Leave every setting as the user left it
    MZTC_PRESET_GENERAL,               // Balanced, for ordinary flying
    MZTC_PRESET_FIRE,                  // Flat mid-tones so hot spikes dominate
    MZTC_PRESET_SEARCH,                // Lift a small warm target out of ambient
    MZTC_PRESET_SURVEILLANCE,          // Loiter work, where frame averaging helps
    MZTC_PRESET_INSPECTION,            // Read gradients across a surface
    MZTC_PRESET_MARITIME               // Warm target on a large uniform cold field
} mztcPreset_e;

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

// MassZero Thermal Camera auto shutter modes.
//
// The camera expects 0x01 to 0x03 on the wire and rejects 0x00 as out of
// range, so the driver adds MZTC_SHUTTER_WIRE_OFFSET when it sends the value.
// These enum values stay zero-based because settings.yaml indexes its lookup
// table from zero.
typedef enum {
    MZTC_SHUTTER_TEMP_ONLY = 0,        // Camera wire value 0x01
    MZTC_SHUTTER_TIME_ONLY = 1,        // Camera wire value 0x02
    MZTC_SHUTTER_TIME_AND_TEMP = 2     // Camera wire value 0x03, the default
} mztcShutterMode_e;

#define MZTC_SHUTTER_WIRE_OFFSET        1

// MassZero Thermal Camera limits.
//
// These are the single source of truth for the valid ranges. settings.yaml
// references them for the CLI bounds. The MSP handlers validate against them.
// A value the CLI rejects cannot be smuggled in over MSP.
#define MZTC_MIN_FFC_INTERVAL           1       // Minimum 1 minute
#define MZTC_MAX_FFC_INTERVAL           60      // Maximum 60 minutes
#define MZTC_MIN_PERCENT                0
#define MZTC_MAX_PERCENT                100

// MassZero Thermal Camera configuration structure
// The serial port and its baud rate come from the Ports tab through
// findSerialPortConfig(FUNCTION_MZTC_CAMERA), the same way every other serial
// peripheral in INAV works. Assigning the function is what enables the camera,
// so there is no separate enable, port or baudrate setting to keep in step.
typedef struct mztcConfig_s {
    uint8_t preset;                     // Purpose preset, see mztcPreset_e
    uint8_t palette_mode;               // Color palette
    uint8_t auto_shutter;               // Auto shutter mode
    uint8_t digital_enhancement;        // Digital enhancement (0-100)
    uint8_t spatial_denoise;            // Spatial denoising (0-100)
    uint8_t temporal_denoise;           // Temporal denoising (0-100)
    uint8_t brightness;                 // Brightness (0-100)
    uint8_t contrast;                   // Contrast (0-100)
    uint8_t zoom_level;                 // Digital zoom level
    uint8_t mirror_mode;                // Image mirroring
    uint8_t ffc_interval;               // Automatic shutter interval, in minutes
} mztcConfig_t;

// MassZero Thermal Camera status structure
typedef struct mztcStatus_s {
    uint8_t status;                     // Camera status
    uint8_t preset;                     // Purpose preset in effect
    bool connected;                      // Connection status
    uint8_t connection_quality;         // Connection quality indicator
    uint16_t last_calibration;          // Minutes since last calibration (saturates at UINT16_MAX)
    uint8_t error_flags;                // Error status flags
} mztcStatus_t;

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

// Wire framing constants for the camera serial protocol.
// Layout: begin(1) size(1) addr(1) class(1) subclass(1) flags(1) data(N)
//         checksum(1) end(1). The size field is N+4 and covers addr through
//         checksum. The total byte count on the wire is size+4.
#define MZTC_PACKET_BEGIN               0xF0
#define MZTC_PACKET_END                 0xFF
#define MZTC_DEVICE_ADDR                0x36
#define MZTC_MAX_DATA_LEN               14
#define MZTC_PACKET_OVERHEAD            8       // Everything that is not payload
#define MZTC_MIN_PACKET_LEN             MZTC_PACKET_OVERHEAD
#define MZTC_MAX_PACKET_LEN             (MZTC_PACKET_OVERHEAD + MZTC_MAX_DATA_LEN)
#define MZTC_SIZE_FIELD_OFFSET          4       // size = data_len + 4

// Parameter group declaration
PG_DECLARE(mztcConfig_t, mztcConfig);

// Function declarations
void mztcInit(void);
void mztcUpdate(timeUs_t currentTimeUs);
bool mztcIsEnabled(void);
mztcStatus_t* mztcGetStatus(void);
bool mztcTriggerCalibration(void);
bool mztcSetPreset(mztcPreset_e preset);
bool mztcSetPalette(mztcPaletteMode_e palette);
bool mztcSetZoom(mztcZoomLevel_e zoom);
bool mztcSetImageParams(uint8_t brightness, uint8_t contrast, uint8_t enhancement);
bool mztcSetDenoising(uint8_t spatial, uint8_t temporal);
bool mztcIsConnected(void);
void mztcRequestReconnect(void);
bool mztcSaveConfiguration(void);
bool mztcRestoreDefaults(void);
bool mztcTriggerVignettingCorrection(void);
bool mztcConfigIsValid(const mztcConfig_t *cfg);

// Serial framing helpers. Exposed so the unit test can exercise the wire
// format without a serial port; see mztc_camera_unittest.cc.
uint8_t mztcBuildPacket(uint8_t *out, uint8_t class_cmd, uint8_t subclass_cmd,
                        uint8_t flags, const uint8_t *data, uint8_t data_len);
bool mztcPacketIsValid(const uint8_t *packet, uint8_t len);

#endif // USE_MZTC
