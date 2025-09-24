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

#include "msp/msp.h"
#include "config/mztc_camera.h"

#ifdef USE_MZTC

// MassZero Thermal Camera MSP V2 commands
// Using INAV-specific range 0x2000-0x2FFF as per MSP V2 standards
#define MSP2_MZTC_CONFIG                0x2000    // Get MassZero Thermal Camera configuration
#define MSP2_SET_MZTC_CONFIG            0x2001    // Set MassZero Thermal Camera configuration
#define MSP2_MZTC_STATUS                0x2002    // Get MassZero Thermal Camera status
#define MSP2_MZTC_FRAME_DATA            0x2003    // Get thermal frame data
#define MSP2_MZTC_CALIBRATE            0x2004    // Trigger calibration (FFC)
#define MSP2_MZTC_MODE                 0x2005    // Set operating mode
#define MSP2_MZTC_PALETTE              0x2006    // Set color palette
#define MSP2_MZTC_ZOOM                 0x2007    // Set zoom level
#define MSP2_MZTC_SHUTTER              0x2008    // Trigger manual shutter
#define MSP2_SET_MZTC_SHUTTER          0x200C    // Set manual shutter trigger
#define MSP2_MZTC_ALERTS               0x2009    // Configure temperature alerts
#define MSP2_SET_MZTC_ALERTS           0x200D    // Set temperature alerts configuration
#define MSP2_MZTC_IMAGE_PARAMS         0x200A    // Set image parameters (brightness, contrast, etc.)
#define MSP2_SET_MZTC_IMAGE_PARAMS    0x200E    // Set image parameters
#define MSP2_MZTC_CORRECTION           0x200B    // Set correction parameters (denoising, enhancement)
#define MSP2_SET_MZTC_CORRECTION      0x200F    // Set correction parameters

// MSP message structures for MassZero Thermal Camera

// MSP_MZTC_CONFIG (2000) - Get configuration
typedef struct {
    uint8_t enabled;
    uint8_t port;
    uint8_t baudrate;
    uint8_t mode;
    uint8_t update_rate;
    uint8_t temperature_unit;
    uint8_t palette_mode;
    uint8_t auto_shutter;
    uint8_t digital_enhancement;
    uint8_t spatial_denoise;
    uint8_t temporal_denoise;
    uint8_t brightness;
    uint8_t contrast;
    uint8_t zoom_level;
    uint8_t mirror_mode;
    uint8_t crosshair_enabled;
    uint8_t temperature_alerts;
    float alert_high_temp;
    float alert_low_temp;
    uint8_t ffc_interval;
    uint8_t bad_pixel_removal;
    uint8_t vignetting_correction;
} msp_mztc_config_t;

// MSP_MZTC_STATUS (2002) - Get status
typedef struct {
    uint8_t status;                     // Camera status
    uint8_t mode;                       // Current mode
    uint8_t connection_quality;         // Connection quality indicator
    uint8_t last_calibration;           // Minutes since last calibration
    float camera_temperature;           // Camera internal temperature
    float ambient_temperature;          // Ambient temperature
    uint32_t frame_count;               // Frame counter
    uint8_t error_flags;                // Error status flags
    uint32_t last_frame_time;           // Last frame timestamp
} msp_mztc_status_t;

// MSP_MZTC_FRAME_DATA (2003) - Get frame data
typedef struct {
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
} msp_mztc_frame_t;

// MSP_MZTC_MODE (2005) - Set mode
typedef struct {
    uint8_t mode;                       // Operating mode
} msp_mztc_mode_t;

// MSP_MZTC_PALETTE (2006) - Set palette
typedef struct {
    uint8_t palette;                    // Color palette
} msp_mztc_palette_t;

// MSP_MZTC_ZOOM (2007) - Set zoom
typedef struct {
    uint8_t zoom;                       // Zoom level
} msp_mztc_zoom_t;

// MSP_MZTC_SHUTTER (2008) - Trigger manual shutter
typedef struct {
    uint8_t shutter_trigger;            // Shutter trigger command
} msp_mztc_shutter_t;

// MSP_MZTC_IMAGE_PARAMS (2010) - Set image parameters
typedef struct {
    uint8_t brightness;                 // Brightness (0-100)
    uint8_t contrast;                   // Contrast (0-100)
    uint8_t enhancement;                // Digital enhancement (0-100)
} msp_mztc_image_params_t;

// MSP_MZTC_CORRECTION (2011) - Set correction parameters
typedef struct {
    uint8_t spatial_denoise;            // Spatial denoising (0-100)
    uint8_t temporal_denoise;           // Temporal denoising (0-100)
} msp_mztc_correction_t;

// MSP_MZTC_ALERTS (2009) - Configure alerts
typedef struct {
    uint8_t enabled;                    // Enable/disable alerts
    float high_temp;                    // High temperature threshold
    float low_temp;                     // Low temperature threshold
} msp_mztc_alerts_t;

// Function declarations
mspResult_e mspMztcProcessCommand(mspPacket_t *cmd, mspPacket_t *reply);
void mspMztcInit(void);

#endif // USE_MZTC
