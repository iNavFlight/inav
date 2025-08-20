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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "platform.h"

#include "build/debug.h"
#include "build/build_config.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/serial.h"
#include "drivers/system.h"
#include "drivers/time.h"

#include "io/serial.h"
#include "io/mztc_camera.h"

#include "scheduler/scheduler.h"
#include <stdio.h>

// Parameter group ID for MassZero Thermal Camera (defined in parameter_group_ids.h)

// Default configuration values
#define MZTC_DEFAULT_ENABLED            0
#define MZTC_DEFAULT_PORT               SERIAL_PORT_USART2
#define MZTC_DEFAULT_BAUDRATE           BAUD_115200
#define MZTC_DEFAULT_MODE               MZTC_MODE_DISABLED
#define MZTC_DEFAULT_UPDATE_RATE        9
#define MZTC_DEFAULT_TEMPERATURE_UNIT   MZTC_UNIT_CELSIUS
#define MZTC_DEFAULT_PALETTE_MODE       MZTC_PALETTE_WHITE_HOT
#define MZTC_DEFAULT_AUTO_SHUTTER       MZTC_SHUTTER_TIME_AND_TEMP
#define MZTC_DEFAULT_DIGITAL_ENHANCEMENT 50
#define MZTC_DEFAULT_SPATIAL_DENOISE    50
#define MZTC_DEFAULT_TEMPORAL_DENOISE   50
#define MZTC_DEFAULT_BRIGHTNESS         50
#define MZTC_DEFAULT_CONTRAST           50
#define MZTC_DEFAULT_ZOOM_LEVEL         MZTC_ZOOM_1X
#define MZTC_DEFAULT_MIRROR_MODE        MZTC_MIRROR_NONE
#define MZTC_DEFAULT_CROSSHAIR_ENABLED  0
#define MZTC_DEFAULT_TEMPERATURE_ALERTS 0
#define MZTC_DEFAULT_ALERT_HIGH_TEMP    80.0f
#define MZTC_DEFAULT_ALERT_LOW_TEMP     -10.0f
#define MZTC_DEFAULT_FFC_INTERVAL       5
#define MZTC_DEFAULT_BAD_PIXEL_REMOVAL  1
#define MZTC_DEFAULT_VIGNETTING_CORRECTION 1

// Serial packet structure for thermal camera communication
typedef struct {
    uint8_t begin;          // 0xF0 - Start byte
    uint8_t size;           // N+4 (total packet length)
    uint8_t device_addr;    // 0x36 - Device address
    uint8_t class_cmd;      // Class command address
    uint8_t subclass_cmd;   // Subclass command address
    uint8_t flags;          // Read/Write flags
    uint8_t data[14];       // Data content (max 14 bytes)
    uint8_t checksum;       // Checksum
    uint8_t end;            // 0xFF - End byte
} mztcPacket_t;

// Flag definitions
#define MZTC_FLAG_WRITE      0x00
#define MZTC_FLAG_READ       0x01
#define MZTC_FLAG_SUCCESS    0x03
#define MZTC_FLAG_ERROR      0x04

// Error codes
#define MZTC_ERR_NO_COMMAND  0x00
#define MZTC_ERR_THRESHOLD   0x01

// Camera commands
#define MZTC_CMD_READ_MODEL          {0x74, 0x02}  // Read device model
#define MZTC_CMD_READ_FPGA_VER       {0x74, 0x03}  // Read FPGA version
#define MZTC_CMD_READ_SW_VER         {0x74, 0x05}  // Read software version
#define MZTC_CMD_MANUAL_SHUTTER      {0x7C, 0x02}  // Manual FFC
#define MZTC_CMD_AUTO_SHUTTER        {0x7C, 0x04}  // Auto shutter control
#define MZTC_CMD_DIGITAL_ENHANCE     {0x78, 0x10}  // Digital enhancement
#define MZTC_CMD_SPATIAL_DENOISE     {0x78, 0x15}  // Spatial denoising
#define MZTC_CMD_TEMPORAL_DENOISE    {0x78, 0x16}  // Temporal denoising
#define MZTC_CMD_BRIGHTNESS          {0x78, 0x02}  // Brightness
#define MZTC_CMD_CONTRAST            {0x78, 0x03}  // Contrast
#define MZTC_CMD_PSEUDO_COLOR        {0x78, 0x20}  // Pseudo color
#define MZTC_CMD_ZOOM                {0x70, 0x12}  // Digital zoom
#define MZTC_CMD_IMAGE_MIRROR        {0x70, 0x11}  // Mirror mode

// Internal state
mztcStatus_t mztcStatus;
serialPort_t *mztcSerialPort = NULL;
static uint32_t mztcLastUpdateTime = 0;
static uint32_t mztcLastCalibrationTime = 0;
static bool mztcInitialized = false;
static uint8_t mztcRxBuffer[256];
static uint16_t mztcRxBufferIndex = 0;

// SITL detection and connection state
static bool mztcSitlMode = false;
static uint32_t mztcLastDataReceived = 0;

// Parameter group for MassZero Thermal Camera configuration
PG_REGISTER_WITH_RESET_TEMPLATE(mztcConfig_t, mztcConfig, PG_MZTC_CAMERA_CONFIG, 0);

PG_RESET_TEMPLATE(mztcConfig_t, mztcConfig,
    .enabled = MZTC_DEFAULT_ENABLED,
    .port = MZTC_DEFAULT_PORT,
    .baudrate = MZTC_DEFAULT_BAUDRATE,
    .mode = MZTC_DEFAULT_MODE,
    .update_rate = MZTC_DEFAULT_UPDATE_RATE,
    .temperature_unit = MZTC_DEFAULT_TEMPERATURE_UNIT,
    .palette_mode = MZTC_DEFAULT_PALETTE_MODE,
    .auto_shutter = MZTC_DEFAULT_AUTO_SHUTTER,
    .digital_enhancement = MZTC_DEFAULT_DIGITAL_ENHANCEMENT,
    .spatial_denoise = MZTC_DEFAULT_SPATIAL_DENOISE,
    .temporal_denoise = MZTC_DEFAULT_TEMPORAL_DENOISE,
    .brightness = MZTC_DEFAULT_BRIGHTNESS,
    .contrast = MZTC_DEFAULT_CONTRAST,
    .zoom_level = MZTC_DEFAULT_ZOOM_LEVEL,
    .mirror_mode = MZTC_DEFAULT_MIRROR_MODE,
    .crosshair_enabled = MZTC_DEFAULT_CROSSHAIR_ENABLED,
    .temperature_alerts = MZTC_DEFAULT_TEMPERATURE_ALERTS,
    .alert_high_temp = MZTC_DEFAULT_ALERT_HIGH_TEMP,
    .alert_low_temp = MZTC_DEFAULT_ALERT_LOW_TEMP,
    .ffc_interval = MZTC_DEFAULT_FFC_INTERVAL,
    .bad_pixel_removal = MZTC_DEFAULT_BAD_PIXEL_REMOVAL,
    .vignetting_correction = MZTC_DEFAULT_VIGNETTING_CORRECTION,
);

// Forward declarations
static void mztcSerialReceiveCallback(uint16_t c, void *rxCallbackData);
static bool mztcSendPacket(uint8_t class_cmd, uint8_t subclass_cmd, uint8_t flags, const uint8_t *data, uint8_t data_len);
static bool mztcProcessResponse(const uint8_t *data, uint8_t len);
static void mztcUpdateStatus(void);
static void mztcCheckCalibration(void);
static void mztcSendConfiguration(void); // Forward declaration for new function

// Initialize MassZero Thermal Camera
void mztcInit(void)
{
    // DEBUG: Print initialization message
    printf("MZTC: Initializing MassZero Thermal Camera\n");
    
    if (mztcInitialized) {
        return;
    }

    // Initialize status structure
    memset(&mztcStatus, 0, sizeof(mztcStatus));
    mztcStatus.status = MZTC_STATUS_OFFLINE;
    mztcStatus.mode = mztcConfig()->mode;
    mztcStatus.last_frame_time = 0;
    mztcStatus.frame_count = 0;
    mztcStatus.connected = false;  // Explicitly set to false

    // Check if enabled
    if (!mztcConfig()->enabled) {
        mztcStatus.status = MZTC_STATUS_OFFLINE;
        mztcInitialized = true;
        return;
    }

    // Detect SITL mode (check if we're running in simulation)
    #ifdef USE_SIMULATOR
        mztcSitlMode = true;
        printf("MZTC: Running in SITL mode\n");
    #else
        mztcSitlMode = false;
        printf("MZTC: Running on real hardware\n");
    #endif

    // Don't try to open port immediately - let the update loop handle it
    mztcStatus.status = MZTC_STATUS_INITIALIZING;
    mztcInitialized = true;
    mztcLastUpdateTime = millis();
    mztcLastCalibrationTime = millis();

    debug[0] = 0xAA; // Debug indicator
}

// Update MassZero Thermal Camera (called from scheduler)
void mztcUpdate(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);
    if (!mztcInitialized || !mztcConfig()->enabled) {
        return;
    }

    uint32_t now = millis();

    // Check if we need to retry connection
    if (!mztcStatus.connected && mztcSerialPort == NULL) {
        if ((now - mztcLastUpdateTime) > 1000) { // Try every second
            mztcLastUpdateTime = now;
            
            // Try to open serial port (works for both SITL TCP ports and real hardware)
            mztcSerialPort = openSerialPort(mztcConfig()->port,
                                           FUNCTION_MZTC_CAMERA,
                                           mztcSerialReceiveCallback,
                                           NULL,
                                           baudRates[mztcConfig()->baudrate],
                                           MODE_RXTX,
                                           SERIAL_NOT_INVERTED);
            
            if (mztcSerialPort != NULL) {
                // Successfully opened port
                mztcStatus.connected = true;
                mztcStatus.status = MZTC_STATUS_READY;
                mztcStatus.error_flags = 0;
                mztcStatus.last_frame_time = now;
                mztcStatus.frame_count = 0;
                
                // Send initial configuration commands
                printf("MZTC: Sending initial configuration...\n");
                mztcSendConfiguration();
                
                // Log successful connection
                if (mztcSitlMode) {
                    printf("MZTC: Connected via SITL TCP bridge on Serial %d\n", mztcConfig()->port);
                } else {
                    printf("MZTC: Connected to thermal camera on Serial %d\n", mztcConfig()->port);
                }
            } else {
                // Failed to open port
                mztcStatus.status = MZTC_STATUS_ERROR;
                mztcStatus.error_flags |= MZTC_ERROR_COMMUNICATION;
                printf("MZTC: Failed to open Serial %d, will retry...\n", mztcConfig()->port);
            }
        }
        return; // Don't process further until connected
    }

    // Check if it's time for an update
    if ((now - mztcLastUpdateTime) < (1000 / mztcConfig()->update_rate)) {
        return;
    }

    // Update status
    mztcUpdateStatus();

    // Check calibration timing
    mztcCheckCalibration();

    // Process camera data based on mode
    switch (mztcConfig()->mode) {
        case MZTC_MODE_CONTINUOUS:
            // Request frame data
            // This would typically request the latest thermal frame
            break;
            
        case MZTC_MODE_STANDBY:
            // Periodic status check only
            break;
            
        case MZTC_MODE_ALERT:
            // Check for temperature alerts
            if (mztcStatus.ambient_temperature > mztcConfig()->alert_high_temp ||
                mztcStatus.ambient_temperature < mztcConfig()->alert_low_temp) {
                mztcStatus.status = MZTC_STATUS_ALERT;
            }
            break;
            
        default:
            break;
    }

    mztcLastUpdateTime = now;
}

// Check if MassZero Thermal Camera is connected
bool mztcIsConnected(void)
{
    // For both SITL and real hardware, check if we have a valid serial port
    return mztcStatus.connected && (mztcSerialPort != NULL);
}

// Check if MassZero Thermal Camera is enabled
bool mztcIsEnabled(void)
{
    return mztcConfig()->enabled && mztcInitialized;
}

// Simulate data reception in SITL mode (for testing)
void mztcSimulateDataReception(void)
{
    if (mztcSitlMode) {
        mztcLastDataReceived = millis();
        printf("MZTC SITL: Simulated data reception\n");
    }
}

// Get MassZero Thermal Camera status
mztcStatus_t* mztcGetStatus(void)
{
    return &mztcStatus;
}

// Trigger calibration (FFC)
bool mztcTriggerCalibration(void)
{
    if (!mztcIsEnabled()) {
        return false;
    }

    if (mztcSendPacket(0x7C, 0x02, MZTC_FLAG_WRITE, NULL, 0)) {
        mztcStatus.status = MZTC_STATUS_CALIBRATING;
        mztcLastCalibrationTime = millis();
        return true;
    }

    return false;
}

// Set operating mode
bool mztcSetMode(mztcMode_e mode)
{
    if (!mztcIsEnabled() || mode >= MZTC_MODE_SURVEILLANCE + 1) {
        return false;
    }

    mztcConfigMutable()->mode = mode;
    mztcStatus.mode = mode;
    return true;
}

// Set color palette
bool mztcSetPalette(mztcPaletteMode_e palette)
{
    if (!mztcIsEnabled() || palette >= MZTC_PALETTE_RED_HOT + 1) {
        return false;
    }

    if (mztcSendPacket(0x78, 0x20, MZTC_FLAG_WRITE, (uint8_t*)&palette, 1)) {
        mztcConfigMutable()->palette_mode = palette;
        return true;
    }

    return false;
}

// Set zoom level
bool mztcSetZoom(mztcZoomLevel_e zoom)
{
    if (!mztcIsEnabled() || zoom >= MZTC_ZOOM_8X + 1) {
        return false;
    }

    if (mztcSendPacket(0x70, 0x12, MZTC_FLAG_WRITE, (uint8_t*)&zoom, 1)) {
        mztcConfigMutable()->zoom_level = zoom;
        return true;
    }

    return false;
}

// Set image parameters
bool mztcSetImageParams(uint8_t brightness, uint8_t contrast, uint8_t enhancement)
{
    if (!mztcIsEnabled()) {
        return false;
    }

    bool success = true;
    
    if (brightness <= 100) {
        success &= mztcSendPacket(0x78, 0x02, MZTC_FLAG_WRITE, &brightness, 1);
        if (success) mztcConfigMutable()->brightness = brightness;
    }
    
    if (contrast <= 100) {
        success &= mztcSendPacket(0x78, 0x03, MZTC_FLAG_WRITE, &contrast, 1);
        if (success) mztcConfigMutable()->contrast = contrast;
    }
    
    if (enhancement <= 100) {
        success &= mztcSendPacket(0x78, 0x10, MZTC_FLAG_WRITE, &enhancement, 1);
        if (success) mztcConfigMutable()->digital_enhancement = enhancement;
    }

    return success;
}

// Set denoising parameters
bool mztcSetDenoising(uint8_t spatial, uint8_t temporal)
{
    if (!mztcIsEnabled()) {
        return false;
    }

    bool success = true;
    
    if (spatial <= 100) {
        success &= mztcSendPacket(0x78, 0x15, MZTC_FLAG_WRITE, &spatial, 1);
        if (success) mztcConfigMutable()->spatial_denoise = spatial;
    }
    
    if (temporal <= 100) {
        success &= mztcSendPacket(0x78, 0x16, MZTC_FLAG_WRITE, &temporal, 1);
        if (success) mztcConfigMutable()->temporal_denoise = temporal;
    }

    return success;
}

// Set temperature alerts
bool mztcSetTemperatureAlerts(bool enabled, float high_temp, float low_temp)
{
    if (!mztcIsEnabled()) {
        return false;
    }

    mztcConfigMutable()->temperature_alerts = enabled ? 1 : 0;
    mztcConfigMutable()->alert_high_temp = high_temp;
    mztcConfigMutable()->alert_low_temp = low_temp;

    return true;
}

// Serial receive callback
static void mztcSerialReceiveCallback(uint16_t c, void *rxCallbackData)
{
    UNUSED(rxCallbackData);

    mztcLastDataReceived = millis();

    if (c == 0xF0) {
        mztcRxBufferIndex = 0;
        mztcRxBuffer[mztcRxBufferIndex++] = c;
        return;
    }

    if (mztcRxBufferIndex > 0 && mztcRxBufferIndex < sizeof(mztcRxBuffer)) {
        mztcRxBuffer[mztcRxBufferIndex++] = c;

        // Minimum packet: begin,size,addr,class,subclass,flags,checksum,end => 8 bytes
        if (mztcRxBufferIndex >= 8 && c == 0xFF) {
            const uint8_t *buf = mztcRxBuffer;
            uint8_t declaredSize = buf[1]; // N+4 total length excluding begin/end
            uint16_t totalLen = declaredSize + 4; // matches sender usage
            
            // Verify total length matches received length
            if (mztcRxBufferIndex == totalLen && buf[0] == 0xF0 && buf[mztcRxBufferIndex - 1] == 0xFF) {
                // Verify checksum
                uint8_t calc = buf[2] + buf[3] + buf[4] + buf[5];
                for (uint16_t i = 0; i < (uint16_t)(declaredSize - 4); i++) {
                    calc += buf[6 + i];
                }
                uint8_t recvCks = buf[mztcRxBufferIndex - 2];
                if (calc == recvCks) {
                    mztcProcessResponse(buf, mztcRxBufferIndex);
                }
            }
            mztcRxBufferIndex = 0;
        }
    } else {
        // Overflow or out-of-sync, reset parser
        mztcRxBufferIndex = 0;
    }
}

// Send packet to thermal camera
static bool mztcSendPacket(uint8_t class_cmd, uint8_t subclass_cmd, uint8_t flags, const uint8_t *data, uint8_t data_len)
{
    if (!mztcSerialPort) {
        return false;
    }
    
    // Validate data length to prevent buffer overflow
    if (data_len > sizeof(((mztcPacket_t *)0)->data)) {
        return false;
    }

    mztcPacket_t packet;
    packet.begin = 0xF0;
    packet.device_addr = 0x36;
    packet.class_cmd = class_cmd;
    packet.subclass_cmd = subclass_cmd;
    packet.flags = flags;

    // Copy data
    if (data && data_len > 0) {
        memcpy(packet.data, data, data_len);
    }
    
    // Calculate checksum over addr, class, subclass, flags and data
    uint8_t checksum = packet.device_addr + packet.class_cmd + packet.subclass_cmd + packet.flags;
    for (uint8_t i = 0; i < data_len; i++) {
        checksum += packet.data[i];
    }
    packet.checksum = checksum;
    packet.end = 0xFF;

    // Size field is N+4 per protocol (addr..data..checksum), where N = 3(command bytes)+1(flags)+data_len
    packet.size = (uint8_t)(4 + 3 + 1 + data_len);

    // Total bytes on wire = 1(begin) + 1(size) + (size) + 1(end)
    const uint8_t totalLen = (uint8_t)(1 + 1 + packet.size + 1);
    serialWriteBufShim(mztcSerialPort, (const uint8_t *)&packet, totalLen);
    
    // Debug log
    printf("MZTC: Sent packet - cmd:0x%02X/0x%02X size:%u\n", class_cmd, subclass_cmd, totalLen);
    
    return true;
}

// Process response from thermal camera
static bool mztcProcessResponse(const uint8_t *data, uint8_t len)
{
    if (len < 8 || data[0] != 0xF0 || data[len-1] != 0xFF) {
        return false;
    }

    // Check device address
    if (data[2] != 0x36) {
        return false;
    }

    // Check flags
    uint8_t flags = data[5];
    if (flags == MZTC_FLAG_SUCCESS) {
        // Command executed successfully
        mztcStatus.error_flags &= ~MZTC_ERROR_COMMUNICATION;
    } else if (flags == MZTC_FLAG_ERROR) {
        // Handle error
        mztcStatus.error_flags |= MZTC_ERROR_COMMUNICATION;
        if (len > 6) {
            uint8_t error_code = data[6];
            switch (error_code) {
                case MZTC_ERR_NO_COMMAND:
                    debug[1] = 0x01;
                    break;
                case MZTC_ERR_THRESHOLD:
                    debug[1] = 0x02;
                    break;
                default:
                    debug[1] = 0x03;
                    break;
            }
        }
    }

    return true;
}

// Update camera status
static void mztcUpdateStatus(void)
{
    if (!mztcSerialPort) {
        mztcStatus.status = MZTC_STATUS_ERROR;
        mztcStatus.error_flags |= MZTC_ERROR_COMMUNICATION;
        return;
    }

    // Update connection quality (simple check)
    mztcStatus.connection_quality = 100; // Assume good for now
    
    // Update frame count
    if (mztcConfig()->mode == MZTC_MODE_CONTINUOUS) {
        mztcStatus.frame_count++;
    }
    
    // Update last frame time
    mztcStatus.last_frame_time = millis();
}

// Check calibration timing
static void mztcCheckCalibration(void)
{
    uint32_t now = millis();
    uint32_t minutes_since_calibration = (now - mztcLastCalibrationTime) / (60 * 1000);
    
    mztcStatus.last_calibration = minutes_since_calibration;
    
    // Auto-calibration if enabled and interval reached
    if (mztcConfig()->ffc_interval > 0 && 
        minutes_since_calibration >= mztcConfig()->ffc_interval &&
        mztcConfig()->auto_shutter != MZTC_SHUTTER_TIME_ONLY) {
        
        mztcTriggerCalibration();
    }
}

// Send initial configuration commands to the camera
static void mztcSendConfiguration(void)
{
    if (!mztcIsEnabled() || !mztcIsConnected()) {
        return;
    }

    // Send auto shutter command
    if (mztcSendPacket(0x7C, 0x04, MZTC_FLAG_WRITE, &mztcConfig()->auto_shutter, 1)) {
        // Send digital enhancement command
        mztcSendPacket(0x78, 0x10, MZTC_FLAG_WRITE, &mztcConfig()->digital_enhancement, 1);
        // Send spatial denoising command
        mztcSendPacket(0x78, 0x15, MZTC_FLAG_WRITE, &mztcConfig()->spatial_denoise, 1);
        // Send temporal denoising command
        mztcSendPacket(0x78, 0x16, MZTC_FLAG_WRITE, &mztcConfig()->temporal_denoise, 1);
        // Send brightness command
        mztcSendPacket(0x78, 0x02, MZTC_FLAG_WRITE, &mztcConfig()->brightness, 1);
        // Send contrast command
        mztcSendPacket(0x78, 0x03, MZTC_FLAG_WRITE, &mztcConfig()->contrast, 1);
        // Send pseudo color command
        mztcSendPacket(0x78, 0x20, MZTC_FLAG_WRITE, &mztcConfig()->palette_mode, 1);
        // Send zoom level command
        mztcSendPacket(0x70, 0x12, MZTC_FLAG_WRITE, &mztcConfig()->zoom_level, 1);
        // Send mirror mode command
        mztcSendPacket(0x70, 0x11, MZTC_FLAG_WRITE, &mztcConfig()->mirror_mode, 1);
    }
}

// Safe reconnection API for CLI use
void mztcRequestReconnect(void)
{
    if (mztcSerialPort != NULL) {
        closeSerialPort(mztcSerialPort);
        mztcSerialPort = NULL;
    }
    mztcStatus.connected = false;
    mztcStatus.error_flags = 0;
    mztcLastUpdateTime = 0; // force immediate retry in update loop
}
